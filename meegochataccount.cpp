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

#include "meegochataccount.h"
#include "meegochatcontactmanager.h"
#include "meegochatcontact.h"
#include "meegochataccountmanager.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ConnectionLowlevel>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/ConnectionInterfaceAvatarsInterface>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingStringList>

using namespace QtMobility;

namespace MeeGoChat {

    Acct::Acct(Tp::AccountPtr tpAccount, QObject *parent) :
        QObject(parent),
        mTpAccount(tpAccount),
        mTpConn(0),
        mContactMgr(new ContactManager(this, parent)),
        mPrevStatus(Acct::OFFLINE),
        mCurStatus(Acct::OFFLINE),
        mReady(false),
        mReadyError(false)
    {
        connect(mTpAccount->becomeReady(),
                SIGNAL(finished(Tp::PendingOperation*)),
                this,
                SLOT(onTpAccountReady(Tp::PendingOperation*)));
    }

    QVariant Acct::getDataByRole(int role)
    {
        if (!mReady)
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
            return QVariant::fromValue<QString>(getDisplayName());
            break;
        case Qt::UserRole:
            return QVariant::fromValue<Acct *>(this);
            break;
        case Qt::DecorationRole:
            return QVariant::fromValue<QString>(getIcon());
            break;
        default:
            return QVariant();
            break;
        }
    }

    QString Acct::getDisplayName()
    {
        if (!mReady || mTpAccount.isNull())
            return QString();
        return mTpAccount->displayName();
    }

    QString Acct::getIcon()
    {
        if (!mReady)
            return QString();
        return mTpAccount->iconName();
    }

    QString Acct::getTpProto()
    {
        if (!mReady)
            return QString();
        return mTpAccount->protocolName();
    }

    QString Acct::getTpAcctPath()
    {
/*        if (!mReady)
            return QString();*/
        return mTpAccount->objectPath();
    }

    Acct::AccountStatus Acct::getStatus()
    {
        if (!mReady)
            return Acct::OFFLINE;
        return mapTpStatusToAccountStatus((Tp::ConnectionPresenceType)mTpAccount->currentPresence().type());
    }

    QString Acct::getStatusIcon()
    {
        return mapAccountStatusToIconName(getStatus());
    }

    bool Acct::isReady()
    {
        return mReady;
    }

    Tp::ConnectionPtr Acct::getTpConn()
    {
        return mTpConn;
    }

    void Acct::setStatus(Acct::AccountStatus newStatus)
    {
        setStatus(mapAccountStatusToTpStatus(newStatus));
    }

    void Acct::handleChannel(Tp::ChannelPtr tpChannel)
    {
        if (!tpChannel->isReady()) {
            qDebug("Got to Acct::handleChannel with a not ready tpChannel! This shouldn't have been able to happen!");
            return;
        }
        ChatContact *contact = mContactMgr->getContactByTpContactHandle(tpChannel->targetHandle());
        contact->handleChannel(tpChannel);
    }

    void Acct::setEnabled(bool enabled)
    {
        if (enabled)
            this->setStatus(AccountManager::getInstance()->getGlobalStatus());
        else
            this->setStatus(Acct::OFFLINE);
        mTpAccount->setEnabled(enabled);
    }

    bool Acct::getEnabled()
    {
        return mTpAccount->isEnabled();
    }

    ContactManager * Acct::getContactManager()
    {
        return mContactMgr;
    }

    Tp::PendingChannelRequest * Acct::ensureTextChat(ChatContact *contact)
    {
        return mTpAccount->ensureTextChat(contact->getTpID());
    }

    QVariantMap Acct::getTpParameters()
    {
        return mTpAccount->parameters();
    }

    //TODO: handle pending Tp operation - emit success/failure
    void Acct::setDisplayName(QString newName)
    {
        mTpAccount->setDisplayName(newName);
        emit Updated(this);
    }

    void Acct::updateTpParameters(QVariantMap set, QStringList unset)
    {
        connect(mTpAccount->updateParameters(set, unset),
                SIGNAL(finished(Tp::PendingOperation*)),
                this,
                SLOT(onUpdateTpParametersFinished(Tp::PendingOperation*)));
    }

    void Acct::remove()
    {
        if (mTpAccount.isNull())
            return;
        mTpAccount->remove();
        emit Removed(this);
    }

    //Static public methods

    Acct::AccountStatus Acct::mapTpStatusToAccountStatus(Tp::ConnectionPresenceType tpStatus)
    {
        switch (tpStatus)
        {
        case Tp::ConnectionPresenceTypeAvailable:
            return Acct::AVAILABLE;
            break;
        case Tp::ConnectionPresenceTypeAway:
        case Tp::ConnectionPresenceTypeExtendedAway:
            return Acct::AWAY;
            break;
        case Tp::ConnectionPresenceTypeBusy:
            return Acct::BUSY;
            break;
        case Tp::ConnectionPresenceTypeError:
            qDebug("Tp::ConnectionPresenceTypeError!");
            return Acct::OFFLINE;
            break;
        case Tp::ConnectionPresenceTypeHidden:
            qDebug("Tp::ConnectionPresenceTypeHidden!");
            return Acct::OFFLINE;
            break;
        case Tp::ConnectionPresenceTypeUnknown:
            qDebug("Tp::ConnectionPresenceTypeUnknown!");
            return Acct::OFFLINE;
            break;
        case Tp::ConnectionPresenceTypeUnset:
            qDebug("Tp::ConnectionPresenceTypeUnset!");
            return Acct::OFFLINE;
            break;
        case Tp::ConnectionPresenceTypeOffline:
            return Acct::OFFLINE;
            break;
        default:
            qDebug("Unhandled Tp::ConnectionPresentType!");
            return Acct::OFFLINE;
            break;
        }

    }

    Tp::ConnectionPresenceType Acct::mapAccountStatusToTpStatus(AccountStatus status)
    {
        switch (status)
        {
        case Acct::AVAILABLE:
            return Tp::ConnectionPresenceTypeAvailable;
            break;
        case Acct::AWAY:
            return Tp::ConnectionPresenceTypeAway;
            break;
        case Acct::BUSY:
            return Tp::ConnectionPresenceTypeBusy;
            break;
        case Acct::OFFLINE:
        default:
            return Tp::ConnectionPresenceTypeOffline;
            break;
        }
    }

    QString Acct::mapAccountStatusToIconName(AccountStatus status)
    {
        switch (status)
        {
        case AVAILABLE:
            return QString("icon-m-common-presence-online");
            break;
        case AWAY:
            return QString("icon-m-common-presence-away");
            break;
        case BUSY:
            return QString("icon-m-common-presence-busy");
            break;
        case OFFLINE:
        default:
            return QString("icon-m-common-presence-offline");
            break;
        }
    }

    //Public slots

    void Acct::onChannelReady(Tp::PendingOperation *po)
    {
        if (po->isFinished()) {
            Tp::ChannelPtr cPtr = /*Tp::ChannelPtr(qobject_cast<Tp::Channel *>(*/dynamic_cast<Tp::PendingChannel *>(po)->channel()/*))*/;
            if (!cPtr->isValid() || !cPtr->isReady()) {
                qDebug() << QString("Got an invalid/not ready ChannelPtr in Acct::onChannelReady!");
                return;
            }
            handleChannel(cPtr);
        } else {
            qDebug() << QString("Got an PendingOperation error in Acct::onChannelReady: ") << po->errorMessage();
        }
    }

    //Private functions

    void Acct::setStatus(Tp::ConnectionPresenceType type)
    {
        Tp::SimplePresence sp;
        sp.type = type;
        connect(mTpAccount->setRequestedPresence(sp), SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onSetRequestedPresenceFinished(Tp::PendingOperation*)));
    }

    //Private Slots

    void Acct::onTpAccountReady(Tp::PendingOperation *po)
    {
        if (po->isFinished()) {
            connect (mTpAccount.data(),
                     SIGNAL(haveConnectionChanged(bool)),
                     this,
                     SLOT(onHaveConnectionChanged(bool)));
            /*connect(tpAcct.data(),
                    SIGNAL(validityChanged(bool)),
                    this,
                    SLOT(onAcctValidityChanged(bool)));*/
            if (mTpAccount->connectionStatus() == Tp::ConnectionStatusConnected)
                onHaveConnectionChanged(true);

            mReady = true;
            mReadyError = false;
            mCurStatus = getStatus();
            emit Ready(this);
        } else {
            mReady = false;
            mReadyError = true;
            emit ReadyError(this, po->errorMessage());
        }
        emit Updated(this);
    }

    void Acct::onHaveConnectionChanged(bool haveConn)
    {
        if (haveConn)
        {
            mTpConn = mTpAccount->connection();
            Tp::Features feats = Tp::Features();
            feats << Tp::Connection::FeatureCore;
            feats << Tp::Connection::FeatureRoster;
            feats << Tp::Connection::FeatureRosterGroups;
    //        feats << Tp::Connection::FeatureSelfContact;
            feats << Tp::Connection::FeatureSimplePresence;
//            connect(mTpConn->requestConnect(feats),
//                    SIGNAL(finished(Tp::PendingOperation*)),
//                    this, SLOT(onTpConnReady(Tp::PendingOperation*))); //DV
        } else if (mTpAccount->connectionStatusReason() != Tp::ConnectionStatusReasonRequested)
        {
            //Try again
            setStatus((Tp::ConnectionPresenceType)mTpAccount->requestedPresence().type());
        }

    }

    void Acct::onSetRequestedPresenceFinished(Tp::PendingOperation *po)
    {
        if (po->isFinished())
        {
            mPrevStatus = mCurStatus;
            mCurStatus = this->mapTpStatusToAccountStatus((Tp::ConnectionPresenceType)mTpAccount->requestedPresence().type());

            if (mCurStatus == OFFLINE)
                emit Offline(this);

            if (mPrevStatus == Acct::OFFLINE) {

                if (mTpAccount->connectionStatus() == Tp::ConnectionStatusConnected)
                {
                    mTpConn = mTpAccount->connection();
                    Tp::Features feats = Tp::Features();
                    feats << Tp::Connection::FeatureCore;
                    feats << Tp::Connection::FeatureRoster;
                    feats << Tp::Connection::FeatureRosterGroups;
                    //feats << Tp::Connection::FeatureSelfContact;
                    feats << Tp::Connection::FeatureSimplePresence;

//                    connect(mTpConn->requestConnect(feats),
//                            SIGNAL(finished(Tp::PendingOperation*)),
//                            this,
//                            SLOT(onTpConnReady(Tp::PendingOperation*))); //DV
                }
            }
        } else if (po->isError())
        {
            emit ReadyError(this, po->errorMessage());
        }

    }

    void Acct::onTpConnReady(Tp::PendingOperation *po)
    {
        if (po->isFinished() && mTpConn->isValid())
        {
            //if (mTpAccount->avatarRequirements().isValid())
            //    mContactMgr->setAvatarInterface(mTpConn->avatarsInterface());

            //TODO: figure out how this will work with ContactManager...
            //ContactManager will need to connect to online/offline signals (or accountstatuschanged signal)
            //and Do the Right Thing...

            //If we've previously become ready, delete our old contact list and
            //add the new one back - easier/quicker than looping through all the
            //existing ChatContact *'s and replacing the referenced Tp::ContactPtr...
            if ((mReady) && (mPrevStatus == OFFLINE) && (mCurStatus != OFFLINE))
            {
                emit Reconnected(this);
//                contactAvatars.clear();
//                contactListModel->updateContactManager(this, tpConn->contactManager());
                //TODO: move the above to ContactManager
            } else if (mCurStatus == Acct::OFFLINE)
                emit Disconnected(this);
            else
                emit Connected(this);

            //TODO: See if this needs to be different

/*
            //If !mReady, then this is the first time we've become ready - add our ContactManager...
            if (!mReady) {
//                contactListModel->addContactManager(this, tpConn->contactManager());
                //TODO: move the above to ContactManager
//                mReady = true;
                emit Ready(this);
            }
*/

            emit StatusChanged(this, mCurStatus);

        } else if (po->isError())
        {
            //TODO: Handle error here - attempt reconnect, based on network status?
            emit ConnectionError(this, po->errorMessage());
        }
    }

    void Acct::onUpdateTpParametersFinished(Tp::PendingOperation *po)
    {
        if (po->isFinished())
        {
            mTpAccount->reconnect();
            emit TpParameterUpdateSuccess(this);
        } else
        {
            emit TpParameterUpdateFailure(this, po->errorMessage());
            //TODO - handle error
        }
        emit Updated(this);
    }

}
