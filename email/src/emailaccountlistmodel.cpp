/*
 * Copyright 2011 Intel Corporation.
 * Copyright (C) 2012 Jolla Ltd.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */


#include <qmailstore.h>
#include <qmailnamespace.h>

#include "emailaccountlistmodel.h"

EmailAccountListModel::EmailAccountListModel(QObject *parent) :
    QMailAccountListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(DisplayName, "displayName");
    roles.insert(EmailAddress, "emailAddress");
    roles.insert(MailServer, "mailServer");
    roles.insert(UnreadCount, "unreadCount");
    roles.insert(MailAccountId, "mailAccountId");
    roles.insert(LastSynchronized, "lastSynchronized");
    setRoleNames(roles);

    connect (QMailStore::instance(), SIGNAL(accountsAdded(const QMailAccountIdList &)), this,
             SLOT(onAccountsAdded (const QMailAccountIdList &)));
    connect (QMailStore::instance(), SIGNAL(accountsRemoved(const QMailAccountIdList &)), this,
             SLOT(onAccountsRemoved(const QMailAccountIdList &)));
    connect (QMailStore::instance(), SIGNAL(accountsUpdated(const QMailAccountIdList &)), this,
             SLOT(onAccountsUpdated(const QMailAccountIdList &)));

    QMailAccountListModel::setSynchronizeEnabled(true);
    QMailAccountListModel::setKey(QMailAccountKey::messageType(QMailMessage::Email));
}

EmailAccountListModel::~EmailAccountListModel()
{
}

int EmailAccountListModel::rowCount(const QModelIndex &parent) const
{
    return QMailAccountListModel::rowCount(parent);
}

QVariant EmailAccountListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

     QMailAccountId accountId = QMailAccountListModel::idFromIndex(index);
    
     QMailAccount account(accountId);
    if (role == DisplayName) {
        return QMailAccountListModel::data(index, QMailAccountListModel::NameTextRole);
    }

    if (role == EmailAddress) {
        return account.fromAddress().address();
    }

    if (role == MailServer) {
        QString address = account.fromAddress().address();
        int index = address.indexOf("@");
        QString server = address.right(address.size() - index - 1);
        index = server.indexOf(".com", Qt::CaseInsensitive);
        return server.left(index);
    }

    if (role == UnreadCount) {
        QMailFolderKey key = QMailFolderKey::parentAccountId(accountId);
        QMailFolderSortKey sortKey = QMailFolderSortKey::serverCount(Qt::DescendingOrder);
        QMailFolderIdList folderIds = QMailStore::instance()->queryFolders(key, sortKey);

        QMailMessageKey accountKey(QMailMessageKey::parentAccountId(accountId));
        QMailMessageKey folderKey(QMailMessageKey::parentFolderId(folderIds));
        QMailMessageKey unreadKey(QMailMessageKey::status(QMailMessage::Read, QMailDataComparator::Excludes));
        return (QMailStore::instance()->countMessages(accountKey & folderKey & unreadKey));
    }

    if (role == MailAccountId) {
        return accountId;
    }

    if (role == LastSynchronized) {
        if (account.lastSynchronized().isValid()) {
            return account.lastSynchronized().toLocalTime();
        }
        else {
            //Account was never synced, return zero
            return 0;
        }
    }

    return QVariant();
}

void EmailAccountListModel::onAccountsAdded(const QMailAccountIdList &ids)
{
    QMailAccountListModel::reset();
    QVariantList accountIds;
    foreach (QMailAccountId accountId, ids) {
        accountIds.append(accountId.toULongLong());
    }
    emit accountsAdded(accountIds);
}

void EmailAccountListModel::onAccountsRemoved(const QMailAccountIdList &ids)
{
    QMailAccountListModel::reset();
    QVariantList accountIds;
    foreach (QMailAccountId accountId, ids) {
        accountIds.append(accountId.toULongLong());
    }
    emit accountsRemoved(accountIds);
}

void EmailAccountListModel::onAccountsUpdated(const QMailAccountIdList &ids)
{
    QMailAccountListModel::reset();
    QVariantList accountIds;
    foreach (QMailAccountId accountId, ids) {
        accountIds.append(accountId.toULongLong());
    }
    emit accountsUpdated(accountIds);
}

int EmailAccountListModel::indexFromAccountId(QVariant id)
{ 
    QMailAccountId accountId = id.value<QMailAccountId>();
    if (!accountId.isValid())
        return -1;

    for (int row = 0; row < rowCount(); row++) {
        if (accountId == QMailAccountListModel::idFromIndex(index(row)))
            return row;
    }
    return -1;
}

QVariant EmailAccountListModel::getDisplayNameByIndex(int idx)
{
    return data(index(idx), EmailAccountListModel::DisplayName);
}

QVariant EmailAccountListModel::getEmailAddressByIndex(int idx)
{
    return data(index(idx), EmailAccountListModel::EmailAddress);
}

int EmailAccountListModel::getRowCount()
{
    return rowCount();
}

QStringList EmailAccountListModel::getAllDisplayNames()
{
    QStringList displayNameList;
    for (int row = 0; row < rowCount(); row++) {
        QString displayName = data(index(row), EmailAccountListModel::DisplayName).toString();
        displayNameList << displayName;
    }
    return displayNameList;
}

QStringList EmailAccountListModel::getAllEmailAddresses()
{
    QStringList emailAddressList;
    for (int row = 0; row < rowCount(); row++) {
        QString emailAddress = data(index(row), EmailAccountListModel::EmailAddress).toString();
        emailAddressList << emailAddress;
    }
    return emailAddressList;
}

QVariant EmailAccountListModel::getAccountIdByIndex(int idx)
{
    return data(index(idx), EmailAccountListModel::MailAccountId);
}

QString EmailAccountListModel::addressFromAccountId(QVariant accountId)
{
    int accountIndex = indexFromAccountId(accountId);

    if (accountIndex < 0)
        return "";

    return data(index(accountIndex), EmailAccountListModel::EmailAddress).toString();
}

QString EmailAccountListModel::displayNameFromAccountId(QVariant accountId)
{
    int accountIndex = indexFromAccountId(accountId);

    if (accountIndex < 0)
        return "";

    return data(index(accountIndex), EmailAccountListModel::DisplayName).toString();
}

QDateTime EmailAccountListModel::lastUpdatedAccountTime()
{
    QDateTime lastUpdatedAccTime;
    for (int row = 0; row < rowCount(); row++) {
        if ((data(index(row), EmailAccountListModel::LastSynchronized)).toDateTime() > lastUpdatedAccTime)
            lastUpdatedAccTime = (data(index(row), EmailAccountListModel::LastSynchronized)).toDateTime();
    }
    return lastUpdatedAccTime;
}
