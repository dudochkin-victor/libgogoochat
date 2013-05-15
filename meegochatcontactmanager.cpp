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

#include "meegochatcontactmanager.h"
#include "meegochatcontact.h"
#include "meegochatcontactmodel.h"

#include <TelepathyQt/ConnectionInterfaceAvatarsInterface>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Feature>
#include <TelepathyQt/PendingOperation>

namespace MeeGoChat {

    ContactManager::ContactManager(Acct *acct, QObject *parent) :
        QObject(parent),
        mAccount(acct),
        mTpAI(0),
        mAccountStatus(mAccount->getStatus()),
        mReady(false)
    {
        connect(mAccount,
                SIGNAL(StatusChanged(Acct *, Acct::AccountStatus)),
                this,
                SLOT(onAccountStatusChanged(Acct *, Acct::AccountStatus)));
        setupTpCm();
    }

    void ContactManager::setAvatarInterface(Tp::Client::ConnectionInterfaceAvatarsInterface *tpAI)
    {
        if (tpAI->isValid())
        {
            mTpAI = tpAI;
            connect(mTpAI,
                    SIGNAL(AvatarRetrieved(uint,QString,QByteArray,QString)),
                    this,
                    SLOT(onAvatarRetrieved(uint,QString,QByteArray,QString)));
            connect(mTpAI,
                    SIGNAL(AvatarUpdated(uint,QString)),
                    this,
                    SLOT(onAvatarUpdated(uint,QString)));
        }

    }

    ChatContact * ContactManager::getContactByTpContactHandle(uint contactHandle)
    {
        foreach (ChatContact *contact, mContacts) {
            if (contact->getContactHandle() == contactHandle)
                return contact;
        }

        return 0;
    }

    ChatContact * ContactManager::getContactByTpID(QString id)
    {
        foreach (ChatContact *contact, mContacts) {
            if (contact->getTpID() == id)
                return contact;
        }

        return 0;
    }

    const QList<ChatContact *> ContactManager::getContactList()
    {
        return mContacts;
    }

    const QList<ChatContact *> ContactManager::getPresPubReqContactList()
    {
        return mPresPubReqContacts;
    }

    ContactModel * ContactManager::getContactModel(QObject *parent)
    {
        return new ContactModel(this, mContacts, parent);
    }

    bool ContactManager::getAccountOnline()
    {
        return (mAccount->getStatus() != Acct::OFFLINE);
    }

    void ContactManager::requestAvatar(ChatContact *contact)
    {
        requestAvatar(contact->getContactHandle());
    }

    void ContactManager::removeContact(ChatContact *contact)
    {
        contact->removeContact();
    }

    void ContactManager::addContact(QString contactID)
    {
        if (!mReady)
            return;
        //If we don't already have this contact...
        if (!getContactByTpID(contactID))
            connect(mTpCM->contactsForIdentifiers(QStringList() << contactID),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    this,
                    SLOT(onContactsForIdentifiersFinished(Tp::PendingOperation*)));

    }

    Acct * ContactManager::getAccount()
    {
        return mAccount;
    }

    void ContactManager::approveContact(ChatContact *contact)
    {
        //If we're already subscribed, don't attempt to dupe the contact...
        Tp::ContactPtr tpC = contact->getTpContact();
        if ((tpC->subscriptionState() != Tp::Contact::PresenceStateYes)
            && ((tpC->subscriptionState() != Tp::Contact::PresenceStateAsk)))
            tpC->requestPresenceSubscription();
        tpC->authorizePresencePublication();
        addContact(contact);
        if (mPresPubReqContacts.contains(contact))
            mPresPubReqContacts.removeAll(contact);
    }

    void ContactManager::denyContact(ChatContact *contact)
    {
        contact->getTpContact()->removePresencePublication();
        if (mPresPubReqContacts.contains(contact))
            mPresPubReqContacts.removeAll(contact);
    }

    //Private functions

    void ContactManager::requestAvatar(uint contactHandle)
    {
        if (mTpAI)
            mTpAI->RequestAvatars(Tp::UIntList() << contactHandle);
    }

    void ContactManager::setupTpCm()
    {
        if (!mAccount->getTpConn().isNull() && mAccount->getTpConn()->isValid()
            && mAccount->getTpConn()->isReady()) {
            mTpCM = mAccount->getTpConn()->contactManager().data();

            connect(mTpCM,
                    SIGNAL(presencePublicationRequested(Tp::Contacts)),
                    this,
                    SLOT(onTpCmPresencePublicationRequested(Tp::Contacts)));

            upgradeContacts(mTpCM->allKnownContacts().toList());
        }
    }

    void ContactManager::upgradeContacts(QList<Tp::ContactPtr> cList)
    {
        QSet<Tp::Feature> cFeatures;
        cFeatures << Tp::Contact::FeatureAlias
                << Tp::Contact::FeatureAvatarToken
                << Tp::Contact::FeatureSimplePresence;
        connect(mTpCM->upgradeContacts(cList, cFeatures),
                SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onContactsFeaturesRequestFinished(Tp::PendingOperation*)));
    }

    void ContactManager::addContact(ChatContact *contact)
    {
        mContacts.append(contact);
        emit ContactAdded(contact);
        connect(contact,
                SIGNAL(ContactChanged(ChatContact*)),
                this,
                SIGNAL(ContactUpdated(ChatContact*)));
        connect(contact,
                SIGNAL(ContactRemoved(ChatContact*)),
                this,
                SLOT(onContactRemoved(ChatContact*)));
        connect(contact,
                SIGNAL(ContactRemoved(ChatContact*)),
                this,
                SIGNAL(ContactRemoved(ChatContact*)));
        connect(contact,
                SIGNAL(ReceivedMessage(ChatContact*,Message*)),
                this,
                SIGNAL(ContactReceivedMessage(ChatContact*,Message*)));
        connect(contact,
                SIGNAL(NewChannel(ChatContact*,QString)),
                this,
                SIGNAL(ContactNewChannel(ChatContact*,QString)));
    }

    //Private slots

    void ContactManager::onAvatarRetrieved(uint contactHandle, const QString &token,
                                 const QByteArray &avatar, const QString &type)
    {
        Q_UNUSED(type);
        QPixmap *pix = new QPixmap();
        if (!pix->loadFromData(avatar))
            pix = 0;
        if (mContactAvatars.contains(contactHandle))
        {
            mContactAvatars[contactHandle].first = token;
            mContactAvatars[contactHandle].second = pix;
        } else
        {
            mContactAvatars.insert(contactHandle,
                                  QPair<QString, QPixmap *>(token, pix));
        }
        ChatContact *cContact =
                getContactByTpContactHandle(contactHandle);
        if (cContact)
            cContact->setAvatar(pix);
    }

    void ContactManager::onAvatarUpdated(uint contactHandle, const QString &newAvatarToken)
    {
        if (mContactAvatars.contains(contactHandle)
            && (mContactAvatars[contactHandle].first != newAvatarToken))
            requestAvatar(contactHandle);
    }

    void ContactManager::onAccountStatusChanged(Acct *acct, Acct::AccountStatus status)
    {
        Q_UNUSED(acct);
        if ((status != Acct::OFFLINE) && (mAccountStatus == Acct::OFFLINE)) {
            //We're connecting for the first time, or reconnecting - set/replace our Tp::ContactManager...
            setupTpCm();
            emit AccountOnline();
        } else if ((status == Acct::OFFLINE) && (mAccountStatus != Acct::OFFLINE)) {
            //We're going offline from online
            emit AccountOffline();
        }
        mAccountStatus = status;
    }

    void ContactManager::onContactsFeaturesRequestFinished(Tp::PendingOperation *po)
    {
        if (po->isError())
        {
            qDebug() << QString("Error when upgrading Contacts features: %1").arg(po->errorMessage());
            return;
        }
        Tp::PendingContacts *pending = qobject_cast<Tp::PendingContacts *>(po);
        QList<Tp::ContactPtr> contactList = pending->contacts();

        QHash<QString, QPair<QString, QString> > invIDs = pending->invalidIdentifiers();
        ChatContact *contact;

        foreach (Tp::ContactPtr ptrCt, contactList)
        {
            //TODO - handle lack of existing contact in new contactList
            //i.e. the contact was removed from our server-side contacts
            //list while we were offline/disconnected...
            contact = getContactByTpID(ptrCt->id());
            if (contact)
            {
                contact->setTpContact(ptrCt);
                emit ContactUpdated(contact);
            } else
            {
                contact = new ChatContact(this, mAccount, ptrCt, this);
                if (ptrCt->publishState() == Tp::Contact::PresenceStateAsk)
                {
                    emit PresencePublicationRequested(contact);
                    mPresPubReqContacts.append(contact);
                } else
                {
                    addContact(contact);
                }
            }
        }
        if (!mReady) {
            mReady = true;
            emit Ready(this);
        }
    }

    void ContactManager::onContactRemoved(ChatContact *contact)
    {
        int idx;
        if ((idx = mContacts.indexOf(contact)) != -1) {
            mContacts.removeAt(idx);
        }
    }

    void ContactManager::onContactsForIdentifiersFinished(Tp::PendingOperation *po)
    {
        Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts *>(po);
        if (po->isError()) {
            foreach(Tp::ContactPtr ptrCt, pc->contacts()) {
                emit this->AddContactError(ptrCt->id(), po->errorMessage());
            }
        } else {
            foreach(Tp::ContactPtr ptrCt, pc->contacts()) {
                ptrCt->requestPresenceSubscription();
                emit this->AddContactSuccess(ptrCt->id());
            }
            this->upgradeContacts(pc->contacts());
        }
    }

    void ContactManager::onTpCmPresencePublicationRequested(Tp::Contacts contacts)
    {
        //Upgrade the contacts, then, when the upgraded
        //contacts are looped through in onContactsFeaturesRequestFinished,
        //any contact with a PublishState of PresenceStateAsk will cause
        //a PresencePublicationRequested signal to be emitted...
        upgradeContacts(contacts.toList());
    }

}
