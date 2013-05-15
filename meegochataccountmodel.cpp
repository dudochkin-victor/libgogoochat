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

#include "meegochataccountmodel.h"
#include "meegochataccount.h"
#include "meegochataccountmanager.h"

namespace MeeGoChat {

    AccountModel::AccountModel(AccountManager *acctMgr, QObject *parent) :
        QAbstractListModel(parent),
        mAccountMgr(acctMgr)
    {
        if (!mAccountMgr->isReady())
            connect(mAccountMgr,
                    SIGNAL(Ready()),
                    this,
                    SLOT(onAccountMgrReady()));
        else
            onAccountMgrReady();
    }

    int AccountModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mAccountList.size();
    }

    QVariant AccountModel::data(const QModelIndex &index, int role) const
    {
        if (index.isValid())
        {
            Acct *acct = mAccountList.at(index.row());
            return acct->getDataByRole(role);
        }
        return QVariant();
    }

    QModelIndex AccountModel::getIndexForAccount(Acct *acct) const
    {
        int idx;
        if ((idx = mAccountList.indexOf(acct)) != -1)
            return index(idx, 0);
        else
            return QModelIndex();
    }

    //Private functions

    void AccountModel::addAccount(Acct *acct)
    {
        connect(acct,
                SIGNAL(Updated(Acct *)),
                this,
                SLOT(onAccountUpdated(Acct *)));
        emit this->beginInsertRows(QModelIndex(), mAccountList.count(), mAccountList.count());
        mAccountList.append(acct);
        emit this->endInsertRows();
    }

    void AccountModel::delAccount(Acct *acct)
    {
        if (mAccountList.contains(acct))
        {
            int idx = mAccountList.indexOf(acct);
            emit this->beginRemoveRows(QModelIndex(), idx, idx);
            mAccountList.removeAt(idx);
            emit this->endRemoveRows();
        }
    }

    Acct * AccountModel::findAccountByPath(QString acctPath) const
    {
        foreach (Acct *acct, mAccountList) {
            if (acct->getTpAcctPath() == acctPath)
                return acct;
        }
        return 0;
    }

    // Private Slots

    void AccountModel::onAccountMgrReady()
    {
        connect(mAccountMgr,
                SIGNAL(AccountCreated(QString)),
                this,
                SLOT(onAccountCreated(QString)));
        connect(mAccountMgr,
                SIGNAL(AccountRemoved(QString)),
                this,
                SLOT(onAccountRemoved(QString)));
        foreach (const QString &acctPath, mAccountMgr->getAllAccountPaths())
        {
            onAccountCreated(acctPath);
        }
    }

    void AccountModel::onAccountCreated(QString acctPath)
    {
        Acct *acct = mAccountMgr->getAccountByPath(acctPath);
        //We don't deal with IRC connections (yet)
        //TODO: Please add!!!
        if (acct->getTpProto() != "irc")
        {
            connect(acct,
                    SIGNAL(Updated(Acct *)),
                    this,
                    SLOT(onAccountUpdated(Acct *)));
            addAccount(acct);
        }
    }

    void AccountModel::onAccountRemoved(QString acctPath)
    {
        Acct *acct;

        if ((acct = findAccountByPath(acctPath)))
            delAccount(acct);
    }

    void AccountModel::onAccountUpdated(Acct *acct)
    {
        int idx = mAccountList.indexOf(acct);

        if (idx == -1)
            return;
        QModelIndex qmi = createIndex(idx, 0, 0);
        emit dataChanged(qmi, qmi);
    }

    void AccountModel::onAccountStatusChanged(MeeGoChat::Acct *acct)
    {
        Q_UNUSED(acct);
        //TODO - see if we still need this...
    }

}
