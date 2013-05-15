/*
 * libmeegochat - Qt4 API that wraps Telepathy-Qt4 for easier MeeGo integration
 *
 * Copyright (c) 2010, Intel Corporation.
 *
 * Author: James Ausmus <james.ausmus@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef MEEGOCHATACCOUNTMODEL_H
#define MEEGOCHATACCOUNTMODEL_H

#include <QObject>
#include <QVariant>
#include <QAbstractListModel>
#include <QModelIndex>

namespace MeeGoChat
{
    class AccountManager;
    class Acct;

    class AccountModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        explicit AccountModel(AccountManager *acctMgr, QObject *parent = 0);

        int rowCount(const QModelIndex &) const;
        QVariant data(const QModelIndex &, int) const;
        QModelIndex getIndexForAccount(Acct *acct) const;

    signals:

    public slots:

    private slots:
        void onAccountMgrReady();
        void onAccountCreated(QString acctPath);
        void onAccountRemoved(QString acctPath);
        void onAccountUpdated(Acct *acct);
        void onAccountStatusChanged(Acct *acct);

    private:
        void addAccount(MeeGoChat::Acct *);
        void delAccount(MeeGoChat::Acct *);
        Acct * findAccountByPath(QString acctPath) const;

        AccountManager *mAccountMgr;
        QList<Acct *> mAccountList;

    };

}

Q_DECLARE_METATYPE(MeeGoChat::AccountModel *);

#endif // MEEGOCHATACCOUNTMODEL_H
