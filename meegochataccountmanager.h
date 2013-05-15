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

#ifndef MEEGOCHATACCOUNTMANAGER_H
#define MEEGOCHATACCOUNTMANAGER_H

#include <QObject>
#include <QContactManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Channel>


#include "meegochataccount.h"

namespace MeeGoChat
{

    class Acct;
    class AccountModel;
    class ChatContact;

    class AccountManager : public QObject
    {
        Q_OBJECT
    public:
        explicit AccountManager(QObject *parent = 0);
        ~AccountManager();

        static AccountManager * getInstance(bool enableTpWarnings = false, bool enableTpDebug = false);
        static AccountModel * getAccountModel();
        static void tpSetup(bool enableTpWarnings, bool enableTpDebug);

        bool isReady();
        bool areAccountsReady();
        Acct * getAccountByPath(QString AccountPath, QObject *parent = 0);
        ChatContact * getContactByPathAndID(QString acctPath, QString chatID);
        QStringList getAllAccountPaths() const;
        Acct::AccountStatus getGlobalStatus();
        void setGlobalStatus(Acct::AccountStatus);
        const QList<Acct *> getAccountList();
        void createAccount(QString tpConnectionManager, QString tpProtocol,
                           QString displayName, QVariantMap tpParameters,
                           QVariantMap tpProperties);
        void removeAccount(Acct *acct);
        QtMobility::QContactManager * getQContactManager() const;
        QtMobility::QContact * getSelfQContact() const;

    signals:
        void AccountCreated(QString AccountPath);
        void AccountCreated(Acct *acct);
        void AccountRemoved(QString AccountPath);
        void AccountValidityChanged(QString AccountPath, bool isValid);
        void Ready();
        void ReadyError(QString errMsg);
        void NoAccounts();
        void AccountsReady();
        void CreateAccountSuccess(QString acctName);
        void CreateAccountFailed(QString acctName, QString errMsg);

    public slots:
        bool onChannelToBeHandled(Tp::AccountPtr tpAcct, Tp::ChannelPtr tpChannel);

    private slots:
        void onTpAMReady(Tp::PendingOperation *po);
        void onTpAccountCreated(QString acctPath);
        void onAccountStatusChanged(Acct *acct, Acct::AccountStatus status);
        void onFirstAccountReady(Acct *);
        void onFirstAccountReadyError(Acct *, QString);
        void onAccountReady(Acct *);
        void onAccountReadyError(Acct *, QString errMsg);
        void onAccountRemoved(Acct *);
        void onCreateAccountFinished(Tp::PendingOperation *);

    private:
        Tp::AccountManagerPtr mTpAM;
        QtMobility::QContactManager *mQContactManager;
        QtMobility::QContact *mSelfQContact;
        QList<Acct *> mAccounts;
        Acct::AccountStatus mGlobalStatus;
        bool mReady, mReadyError, mFirstAccount;

    };

}

Q_DECLARE_METATYPE(MeeGoChat::AccountManager *);

#endif // MEEGOCHATACCOUNTMANAGER_H
