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

#include "meegochataccountmanager.h"
#include "meegochataccount.h"
#include "meegochataccountmodel.h"
#include "meegochatcontact.h"

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Debug>
#include <TelepathyQt/PendingAccount>

using namespace QtMobility;

namespace MeeGoChat {
    static AccountManager *gAccountManager = 0;


    AccountManager::AccountManager(QObject *parent) :
        QObject(parent),
        mTpAM(Tp::AccountManager::create()),
        mQContactManager(new QContactManager("tracker")),
        mSelfQContact(0),
        mGlobalStatus(Acct::OFFLINE),
        mReady(false),
        mReadyError(false),
        mFirstAccount(false)
    {
        connect(mTpAM->becomeReady(),
                SIGNAL(finished(Tp::PendingOperation*)),
                this,
                SLOT(onTpAMReady(Tp::PendingOperation *)));

        QContactLocalId selfID = mQContactManager->selfContactId();
        if (selfID) {
            QContact selfContact = mQContactManager->contact(selfID);
            if (selfContact.localId() != selfID) {
                qWarning("Creating self contact!");
                QContactId cID;
                cID.setLocalId(selfID);
                cID.setManagerUri(mQContactManager->managerUri());
                selfContact.setId(cID);
                mQContactManager->saveContact(&selfContact);
            }
            mSelfQContact = new QContact(selfContact);
        } else {
            qWarning("Invalid selfID!");
        }

    }

    AccountManager::~AccountManager()
    {
        if (gAccountManager)
            gAccountManager = 0;
    }

    //Public member functions

    //Static:
    //There should be only 1...
    AccountManager * AccountManager::getInstance(bool enableTpWarnings, bool enableTpDebug)
    {
        if (!gAccountManager) {
            tpSetup(enableTpWarnings, enableTpDebug);
            gAccountManager = new AccountManager();
        }
        return gAccountManager;
    }

    AccountModel * AccountManager::getAccountModel()
    {
        return new AccountModel(AccountManager::getInstance());
    }

    void AccountManager::tpSetup(bool enableTpWarnings, bool enableTpDebug)
    {
        Tp::registerTypes();
        Tp::enableWarnings(enableTpWarnings);
        Tp::enableDebug(enableTpDebug);
    }

    //Non-static

    bool AccountManager::isReady()
    {
        return mReady;
    }

    bool AccountManager::areAccountsReady()
    {
        if (!mReady)
            return false;
        foreach(Acct *acct, mAccounts) {
            if (!acct->isReady())
                return false;
        }
        return true;
    }

    Acct * AccountManager::getAccountByPath(QString AccountPath, QObject *parent)
    {
        if (mReady) {
            //First try to find it in our account list
            foreach (Acct *acct, mAccounts) {
                if (acct->getTpAcctPath() == AccountPath)
                    return acct;
            }
            //If that doesn't work, create a new one and append it to the account list
            Acct *acct = new Acct(mTpAM->accountForPath(AccountPath), parent);
            mAccounts.append(acct);
            return acct;
        }
        return 0;
    }

    ChatContact * AccountManager::getContactByPathAndID(QString acctPath, QString chatID)
    {
        Acct *acct = getAccountByPath(acctPath);
        if (!acct || !acct->isReady())
            return 0;
        return acct->getContactManager()->getContactByTpID(chatID);
    }

    QStringList AccountManager::getAllAccountPaths() const
    {
//        if (mReady)
//            return mTpAM->allAccounts(); //DV
        return QStringList();
    }

    Acct::AccountStatus AccountManager::getGlobalStatus()
    {
        return mGlobalStatus;
    }

    void AccountManager::setGlobalStatus(Acct::AccountStatus status)
    {
        mGlobalStatus = status;
        foreach (Acct *acct, mAccounts) {
            if (acct->getEnabled())
                acct->setStatus(status);
        }
    }

    const QList<Acct *> AccountManager::getAccountList()
    {
        return mAccounts;
    }

    void AccountManager::createAccount(QString tpConnectionManager,
                                       QString tpProtocol, QString displayName,
                                       QVariantMap tpParameters,
                                       QVariantMap tpProperties)
    {
        Tp::PendingAccount *pa = mTpAM->createAccount(tpConnectionManager, tpProtocol, displayName, tpParameters, tpProperties);
        connect(pa,
                SIGNAL(finished(Tp::PendingOperation*)),
                this,
                SLOT(onCreateAccountFinished(Tp::PendingOperation *)));
    }

    void AccountManager::removeAccount(Acct *acct)
    {
        acct->remove();
    }

    QContactManager * AccountManager::getQContactManager() const
    {
        return mQContactManager;
    }

    QContact * AccountManager::getSelfQContact() const
    {
        return mSelfQContact;
    }

    //Public slots

    bool AccountManager::onChannelToBeHandled(Tp::AccountPtr tpAcct, Tp::ChannelPtr tpChannel)
    {
        Acct *acct = getAccountByPath(tpAcct->objectPath(), this);
        if (!acct) {
            qDebug() << QString("Couldn't get MeeGoChat::Acct for path %1 in AccountManager::onChannelToBeHandled!").arg(tpAcct->objectPath());
            return false;
        }
        if (!tpChannel->isReady()) {
            qDebug() << QString("Got an unready TP channel in AM::onChannelToBeHandled!")
                     << QString ("Calling becomeReady() on the channel...");
            connect(tpChannel->becomeReady(),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    acct,
                    SLOT(onChannelReady(Tp::PendingOperation*)));
        } else {
            acct->handleChannel(tpChannel);
        }
        return true;

    }

    //Private slots

    void AccountManager::onTpAMReady(Tp::PendingOperation *po)
    {
        if (po->isFinished()) {
            connect(mTpAM.data(),
                    SIGNAL(accountCreated(QString)),
                    this,
                    SIGNAL(AccountCreated(QString)));
            connect(mTpAM.data(),
                    SIGNAL(accountCreated(QString)),
                    this,
                    SLOT(onTpAccountCreated(QString)));
            connect(mTpAM.data(),
                    SIGNAL(accountRemoved(QString)),
                    this,
                    SIGNAL(AccountRemoved(QString)));
            connect(mTpAM.data(),
                    SIGNAL(accountValidityChanged(QString,bool)),
                    this,
                    SIGNAL(AccountValidityChanged(QString,bool)));

            if (mTpAM->allAccounts().count() == 0)
            {
                emit NoAccounts();
            } else {
//                foreach (Tp::AccountPtr tpAcct, mTpAM->validAccounts()) {
//                    Acct *acct = new Acct(tpAcct, this);
//                    if (!mFirstAccount && tpAcct->isEnabled()) {
//                        connect(acct,
//                                SIGNAL(Ready(Acct *)),
//                                this,
//                                SLOT(onFirstAccountReady(Acct *)));
//                        connect(acct,
//                                SIGNAL(ReadyError(Acct *,QString)),
//                                this,
//                                SLOT(onFirstAccountReadyError(Acct *, QString)));
//                        mFirstAccount = true;
//                    }
//                    connect(acct,
//                            SIGNAL(Ready(Acct *)),
//                            this,
//                            SLOT(onAccountReady(Acct *)));
//                    connect(acct,
//                            SIGNAL(ReadyError(Acct *,QString)),
//                            this,
//                            SLOT(onAccountReadyError(Acct *,QString)));
//                    connect(acct,
//                            SIGNAL(Removed(Acct*)),
//                            this,
//                            SLOT(onAccountRemoved(Acct*)));
//                    mAccounts.append(acct);
//                } //DV
            }

            mReady = true;
            mReadyError = false;
            emit Ready();
        } else {
            mReady = false;
            mReadyError = true;
            emit ReadyError(po->errorMessage());
        }

    }

    void AccountManager::onTpAccountCreated(QString acctPath)
    {
        Acct *acct = new Acct(mTpAM->accountForPath(acctPath));
        connect(acct,
                SIGNAL(Ready(Acct *)),
                this,
                SLOT(onAccountReady(Acct *)));
        connect(acct,
                SIGNAL(ReadyError(Acct *,QString)),
                this,
                SLOT(onAccountReadyError(Acct *,QString)));
        connect(acct,
                SIGNAL(Removed(Acct*)),
                this,
                SLOT(onAccountRemoved(Acct*)));
        mAccounts.append(acct);
        emit AccountCreated(acct);
    }

    void AccountManager::onAccountStatusChanged(MeeGoChat::Acct *acct, Acct::AccountStatus status)
    {
        //TODO: Have to figure out how non-requested, non-universal status changes affect our overall status, if at all...
        //Also, on a UX note, how should "Chat status is avail, but network just dropped out so we're really offline,
        //but we'll be avail again as soon as we have network again" be presented to user?
        qDebug() << QString("Account status changed for path %1, new status %2"
                    ).arg(acct->getTpAcctPath(), QString((uint)status));
    }

    //This is really ugly, but basically we're just grabbing the status of the
    //first account we run across, and setting that as our global status.
    void AccountManager::onFirstAccountReady(Acct *acct)
    {
        mGlobalStatus = acct->getStatus();
        disconnect(acct, SIGNAL(Ready(Acct*)), this, SLOT(onFirstAccountReady(Acct*)));
    }

    void AccountManager::onFirstAccountReadyError(Acct *acct, QString errMsg)
    {
        Q_UNUSED(acct);
        Q_UNUSED(errMsg);
        //TODO - gather our hacky global status some other way - try second account?
    }

    void AccountManager::onAccountReady(Acct *acct)
    {
        //For now, we're not going to store a separate list of
        //QPair(Acct *, bool isReady), in order to save memory
        //I don't anticipate having so many accounts that
        //this loop becomes overly CPU intensive. However, if I'm
        //wrong, we can always trade CPU for memory...
        Q_UNUSED(acct);
        foreach(Acct *acct, mAccounts)
        {
            if (!acct->isReady())
                return;
        }
        emit AccountsReady();
    }

    void AccountManager::onAccountReadyError(Acct *acct, QString errMsg)
    {
        //TODO: Handle this somehow. ;)
        //In more seriousness, we need to come up with a strategy
        //for dealing with some accounts becoming ready and others going
        //into ReadyError state...
        qDebug() <<
            QString(
                "Got Account::ReadyError for account path %1, Tp error message %2"
                ).arg(acct->getTpAcctPath(), errMsg);
    }

    void AccountManager::onAccountRemoved(Acct *acct)
    {
        mAccounts.removeAll(acct);
    }

    void AccountManager::onCreateAccountFinished(Tp::PendingOperation *po)
    {
        if (po->isError())
        {
            emit CreateAccountFailed(dynamic_cast<Tp::PendingAccount *>(po)->account()->displayName(), po->errorMessage());
        } else
        {
            Tp::AccountPtr tpAcct = dynamic_cast<Tp::PendingAccount *>(po)->account();
            tpAcct->reconnect();
            emit CreateAccountSuccess(tpAcct->displayName());
        }
    }

}
