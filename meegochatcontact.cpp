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

#include "meegochatcontact.h"
#include "meegochatcontactmanager.h"
#include "meegochatmessage.h"
#include "meegochataccount.h"
#include "meegochataccountmanager.h"

#include <QContactDetail>
#include <QContactOnlineAccount>
#include <QContactDetailFilter>
#include <QContactIntersectionFilter>
#include <QContactFetchHint>
#include <QContactFetchRequest>

#include <QtDBus>

#include <TelepathyQt/Types>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ReferencedHandles>
#include <TelepathyQt/ReceivedMessage>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelRequest>

using namespace QtMobility;

namespace MeeGoChat {

    ChatContact::ChatContact(ContactManager *cMgr,
                             Acct *account,
                     Tp::ContactPtr tpContact,
                     QObject *parent) :
        QObject(parent),
        mContactMgr(cMgr),
        mAccount(account),
        mQContact(0),
        mAvatar(0),
        mLastIncomingMessage(0),
        mLastOutgoingMessage(0),
        mLastPendingSendMessage(0),
        mTextChan(0),
        mPeopleID(QString()),
        mAccountOnline(mContactMgr->getAccountOnline())
    {
        setTpContact(tpContact);
        mContactMgr->requestAvatar(this);

        QContactManager *qCM = AccountManager::getInstance()->getQContactManager();
        if (qCM) {
            //Whenever contacts are added, changed, or removed, just (try to)
            //refetch the QContact for this ChatContact
            connect(qCM,
                    SIGNAL(contactsAdded(QList<QContactLocalId>)),
                    this,
                    SLOT(associateQContact()));
            //QContactManager is continually outputting contactsChanged - remove this for now...
            /*
            connect(qCM,
                    SIGNAL(contactsChanged(QList<QContactLocalId>)),
                    this,
                    SLOT(associateQContact()));*/
            connect(qCM,
                    SIGNAL(contactsRemoved(QList<QContactLocalId>)),
                    this,
                    SLOT(associateQContact()));
        }
        associateQContact();
    }

    QVariant ChatContact::getDataByRole(int role)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return QVariant::fromValue<QString>(getName());
            break;
        case Qt::UserRole:
            return QVariant::fromValue<ChatContact *>(this);
            break;
        default:
            return QVariant();
            break;
        }

    }

    QPixmap * ChatContact::getAvatar() const
    {
        return mAvatar;
    }

    void ChatContact::setAvatar(QPixmap *avatar)
    {
        mAvatar = avatar;
    }

    uint ChatContact::getContactHandle() const
    {
        return mTpContact->handle()[0];
    }

    QString ChatContact::getName() const
    {
        return (mTpContact->alias().isEmpty() ? mTpContact->id() : mTpContact->alias());
    }

    Acct::AccountStatus ChatContact::getStatus() const
    {
        if (mAccountOnline) {
            return Acct::mapTpStatusToAccountStatus(
                    (Tp::ConnectionPresenceType)mTpContact->presence().type());
        } else {
            return Acct::OFFLINE;
        }

    }

    QString ChatContact::getStatusMsg() const
    {
        //TODO: Figure out why gchat gives Pending/Denied statuses for contacts
        //that are completely valid contacts...
        /*if (tpContact->subscriptionState() == Tp::Contact::PresenceStateAsk)
            return QString("Pending Chat Approval");
        else if (tpContact->subscriptionState() == Tp::Contact::PresenceStateNo)
            return QString("Chat Approval Denied");
        else*/
            return mTpContact->presence().statusMessage();
    }

    QString ChatContact::getTpID() const
    {
        return mTpID;
    }

    void ChatContact::removeContact()
    {
        mTpContact->removePresenceSubscription();
        mTpContact->deleteLater();
        emit ContactRemoved(this);
    }

    void ChatContact::requestChannel(QString channelType)
    {
        QVariantMap request;
        Tp::PendingChannelRequest *pcr;
        if (channelType == TP_QT_IFACE_CHANNEL_TYPE_TEXT) {
            pcr = mAccount->ensureTextChat(this);
        } else {
            qDebug() << QString("Requested unhandled channel type of %1 in ChatContact::requestChannel!").arg(channelType);
            return;
        }
        connect(pcr->channelRequest().data(),
                SIGNAL(succeeded()),
                this,
                SLOT(onChannelRequestSucceeded()));
        connect(pcr->channelRequest().data(),
                SIGNAL(failed(QString,QString)),
                this,
                SLOT(onChannelRequestFailed(QString, QString)));

    }

    Message * ChatContact::sendMessage(QString message)
    {
        if (!haveTextChannel())
            requestChannel();
        return sendMessage(new Message(this, message, Message::PENDINGSEND));
    }

    Message * ChatContact::getLastMessage(Message::Status dir) const
    {
        switch (dir) {
        case Message::RECEIVED:
            return mLastIncomingMessage;
            break;
        case Message::SENT:
            return mLastOutgoingMessage;
            break;
        case Message::PENDINGSEND:
            return mLastPendingSendMessage;
            break;
        default:
            if (!mLastIncomingMessage)
                return mLastOutgoingMessage;
            if (!mLastOutgoingMessage)
                return mLastIncomingMessage;
            if (mLastOutgoingMessage->getDateTime() > mLastIncomingMessage->getDateTime())
                return mLastOutgoingMessage;
            return mLastIncomingMessage;
            break;
        }
    }

    const QList<Message *> ChatContact::getMessageList() const
    {
        const QList<Message *> msgList = mMsgList;
        return msgList;
    }

    Acct * ChatContact::getAccount() const
    {
        return mAccount;
    }

    ContactManager * ChatContact::getContactManager() const
    {
        return mContactMgr;
    }

    QContact * ChatContact::getQContact() const
    {
        return mQContact;
    }

    bool ChatContact::openPeople() const
    {
        if (!mQContact)
            return false;

        QDBusInterface people("com.meego.people", "/com/meego/people", "com.meego.people");
        people.call(QDBus::NoBlock,
                            "showDetailsPage",
                            QVariant::fromValue<QContactLocalId>(mQContact->localId()));

        return true;
    }

    void ChatContact::handleChannel(Tp::ChannelPtr cPtr)
    {
/*        if (mChannels.contains(cPtr->channelType())
            && cPtr->isValid()) {
            mChannels.value(cPtr->channelType())->requestClose();
            mChannels.remove(cPtr->channelType());
            mChannels.insert(cPtr->channelType(), cPtr);
        }*/

        if (cPtr->channelType() == TP_QT_IFACE_CHANNEL_TYPE_TEXT) {
            //Using mTextChan for now, until I further think about
            //the right way to use the type->channel mapping of mChannels...
            if (haveTextChannel()) {
                mTextChan->requestClose();
                mTextChan->deleteLater();
            }
            mTextChan = Tp::TextChannelPtr::dynamicCast(cPtr);
            Tp::Features chanFeats = Tp::Features() << Tp::TextChannel::FeatureMessageQueue << Tp::TextChannel::FeatureMessageCapabilities << Tp::TextChannel::FeatureMessageSentSignal;
            connect(mTextChan.data(),
                    SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
                    SLOT(onTextChannelInvalidated()));
            connect(mTextChan->becomeReady(chanFeats),
                    SIGNAL(finished(Tp::PendingOperation *)),
                    SLOT(onTextChannelReady(Tp::PendingOperation *)));
            connect(mTextChan.data(),
                    SIGNAL(messageReceived(Tp::ReceivedMessage)),
                    this,
                    SLOT(onMessageReceived(Tp::ReceivedMessage)));
            //Not supporting any Delivery Reporting functionality ATM, so we don't need the messageSent signal...
/*            connect(mTextChan.data(),
                    SIGNAL(messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)),
                    this,
                    SLOT(onMessageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString)));
                    */
            //Add any additional required channel types here w/ else ifs
        } else {
            qDebug() << QString("Unhandled channel type %1 in MeeGoChat::ChatContact::handleChannel!").arg(cPtr->channelType());
        }

        emit NewChannel(this, cPtr->channelType());

    }

    void ChatContact::setTpContact(Tp::ContactPtr contactPtr)
    {
        mTpContact = contactPtr;
        mTpID = mTpContact->id();
        connect(mTpContact.data(),
                SIGNAL(simplePresenceChanged(const QString &, uint, const QString &)),
                SLOT(onContactChanged()));
        connect(mTpContact.data(),
                SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
                SLOT(onContactChanged()));
        connect(mTpContact.data(),
                SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
                SLOT(onContactChanged()));
        connect(mTpContact.data(),
                SIGNAL(blockStatusChanged(bool)),
                SLOT(onContactChanged()));
    }

    Tp::ContactPtr ChatContact::getTpContact() const
    {
        return mTpContact;
    }

    void ChatContact::triggerChanged()
    {
        emit ContactChanged(this);
    }

    //Private slots

    //These emit ContactChanged so that
    //models/lists can re-read the presence/status,
    //and show Offline/$RealStatus as appropriate
    void ChatContact::onAccountOffline()
    {
        mAccountOnline = false;
        emit ContactChanged(this);
    }

    void ChatContact::onAccountOnline()
    {
        mAccountOnline = true;
        emit ContactChanged(this);
    }

    void ChatContact::onContactChanged()
    {
        emit ContactChanged(this);
    }

    void ChatContact::onTextChannelReady(Tp::PendingOperation *po)
    {
        if (po->isError())
        {
            //TODO - handle gracefully
        } else
        {
            //Send queued messages...
            foreach(Message *msg, mMsgQueue)
            {
                mMsgQueue.removeOne(msg);
                sendMessage(msg);
            }
        }
    }

    void ChatContact::onTextChannelInvalidated()
    {
        mTextChan->deleteLater();
        mTextChan = Tp::TextChannelPtr(0);
        if (mMsgQueue.count()) {
            this->requestChannel();
        }
    }

    void ChatContact::onMessageReceived(Tp::ReceivedMessage rMsg)
    {
        mLastIncomingMessage = new Message(this, rMsg.text(), Message::RECEIVED);
        mMsgList.append(mLastIncomingMessage);
        emit ReceivedMessage(this, mLastIncomingMessage);
        emit ContactChanged(this);
        if (haveTextChannel())
            mTextChan->acknowledge(QList<Tp::ReceivedMessage>() << rMsg);
    }

    void ChatContact::onChannelRequestSucceeded()
    {
        emit ChannelRequestSuccess(this);
    }

    void ChatContact::onChannelRequestFailed(QString errName, QString errMsg)
    {
        Q_UNUSED(errName);
        emit ChannelRequestError(this, errMsg);
    }

    void ChatContact::associateQContact()
    {
        QContactManager *qCM;
        if ((qCM = AccountManager::getInstance()->getQContactManager())) {

            QContactIntersectionFilter qCIF;
            QContactDetailFilter qCDFAccount, qCDFContact;


            qCDFContact.setDetailDefinitionName(QContactOnlineAccount::DefinitionName,
                                                QContactOnlineAccount::FieldAccountUri);
            qCDFContact.setValue(QVariant::fromValue<QString>(getTpID()));

            //Note: QContactOnlineAccount doesn't currently have a ::FieldAccountPath, but that is where
            //contactsd is storing the associated Telepathy account path.
            qCDFAccount.setDetailDefinitionName(QContactOnlineAccount::DefinitionName,
                                                "AccountPath");
            qCDFAccount.setValue(QVariant::fromValue<QString>(mAccount->getTpAcctPath()));

            qCIF.append(qCDFContact);
            qCIF.append(qCDFAccount);

            QList<QContact> contacts = qCM->contacts(qCIF);
            if (contacts.count()) {
                if (contacts.count() > 1) {
                    qWarning() <<
                            QString("Error in ChatContact::associateQContact:"
                                    "%1 matching QContacts for TpContact %2!%3!"
                                    ).arg(QString::number(contacts.count()),
                                         mAccount->getTpAcctPath(),
                                         getTpID());
                }
                mQContact = new QContact(contacts[0]);
            } else {
                mQContact = 0;
                qDebug() << QString("No QContact for ChatContact %1!%2").arg(mAccount->getTpAcctPath(), getTpID());
            }

#if 0       //Debug - dumps all values for all details for all contacts
            contacts = qCM->contacts();
            foreach (QContact contact, contacts) {
                foreach (QContactDetail detail, contact.details()) {
                    qDebug() << QString("detail info for contact %1: detail %2").arg(QString::number(contact.localId()),
                            detail.definitionName());
                    qDebug() << detail.variantValues();

                }
            }
#endif
        }
        emit ContactChanged(this);

    }

    void ChatContact::associateQContactAdded()
    {
        qDebug("QContactAdded!");
    }

    void ChatContact::associateQContactChanged()
    {
        qDebug("QContactChanged!");
    }

    void ChatContact::associateQContactRemoved()
    {
        qDebug("QContactRemoved!");
    }

    //Private functions

    Message * ChatContact::sendMessage(Message *msg)
    {
        if (haveTextChannel()) {
            mTextChan->send(msg->getText());
            msg->setStatus(Message::SENT);
            mLastOutgoingMessage = msg;
        } else {
            mMsgQueue.append(msg);
            mLastPendingSendMessage = msg;
        }
        mMsgList.append(msg);
        emit ContactChanged(this);

        return msg;
    }

    bool ChatContact::haveTextChannel() const
    {
        return (mTextChan.data() && !mTextChan.isNull()
                && mTextChan->isValid() && mTextChan->isReady());
    }

}
