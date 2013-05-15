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

#ifndef MEEGOCHATCONTACTMANAGER_H
#define MEEGOCHATCONTACTMANAGER_H

#include <QObject>
#include <TelepathyQt4/ContactManager>
#include "meegochataccount.h"

namespace Tp {
    namespace Client {
        class ConnectionInterfaceAvatarsInterface;
    }
}

namespace MeeGoChat
{
    class ChatContact;
    class Acct;
    class ContactModel;
    class Message;

    class ContactManager : public QObject
    {
        Q_OBJECT
    public:
        explicit ContactManager(Acct *acct, QObject *parent = 0);

        void setAvatarInterface(Tp::Client::ConnectionInterfaceAvatarsInterface *tpAI);
        ChatContact * getContactByTpContactHandle(uint contactHandle);
        ChatContact * getContactByTpID(QString id);
        const QList<ChatContact *> getContactList();
        const QList<ChatContact *> getPresPubReqContactList();
        ContactModel * getContactModel(QObject *parent = 0);
        bool getAccountOnline();
        void requestAvatar(ChatContact *contact);
        void removeContact(ChatContact *contact);
        void addContact(QString contactID);
        Acct * getAccount();
        void approveContact(ChatContact *contact);
        void denyContact(ChatContact *contact);
        bool isReady() { return mReady; }

    signals:
        void PresencePublicationRequested(ChatContact *);
        void ContactAdded(ChatContact *);
        void ContactRemoved(ChatContact *);
        void ContactUpdated(ChatContact *);
        void AccountOffline();
        void AccountOnline();
        void Ready(ContactManager *);
        void AddContactError(QString contactID, QString errMsg);
        void AddContactSuccess(QString contactID);
        void ContactReceivedMessage(ChatContact *contact, Message *msg);
        void ContactNewChannel(ChatContact *contact, QString channelType);

    public slots:

    private slots:
        void onAvatarRetrieved(uint contactHandle, const QString &token,
                               const QByteArray &avatar, const QString &type);
        void onAvatarUpdated(uint contactHandle, const QString &newAvatarToken);
        void onAccountStatusChanged(Acct *acct, Acct::AccountStatus status); //TODO
        void onContactsFeaturesRequestFinished(Tp::PendingOperation *po);
        void onContactRemoved(ChatContact*);
        void onContactsForIdentifiersFinished(Tp::PendingOperation*);
        void onTpCmPresencePublicationRequested(Tp::Contacts);

    private:
        void requestAvatar(uint contactHandle);
        void setupTpCm();
        void upgradeContacts(QList<Tp::ContactPtr> cList);
        void addContact(ChatContact *contact);

        Acct *mAccount;
        Tp::ContactManager *mTpCM;
        Tp::Client::ConnectionInterfaceAvatarsInterface *mTpAI;
        QList<ChatContact *> mContacts;
        QList<ChatContact *> mPresPubReqContacts;
        QHash<uint, QPair<QString, QPixmap*> > mContactAvatars;
        Acct::AccountStatus mAccountStatus;
        bool mReady;

    };

}

Q_DECLARE_METATYPE(MeeGoChat::ContactManager *);

#endif // MEEGOCHATCONTACTMANAGER_H
