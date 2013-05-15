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

#ifndef MEEGOCHATCONTACT_H
#define MEEGOCHATCONTACT_H

#include <QObject>
#include <QPixmap>
#include <QContact>
#include <TelepathyQt/Contact>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "meegochatcontactmanager.h"
#include "meegochatmessage.h"

namespace MeeGoChat
{

    class Message;
    //Have to have the ChatContact aware of the Acct it is from,
    //as the Tp method for requesting a Channel is currently hooked
    //of the Tp::Account class... :(
    //Might rethink this later, but it's a convenience to just have the ChatContact
    //request it's own Channel when needed...
    class Acct;

    //Had to rename this from Contact to ChatContact, as
    //the typedef for a Tp::ContactPtr is Tp::SharedPtr<Contact> -
    //so we get namespace conflicts with their typedef...
    class ChatContact : public QObject
    {
        Q_OBJECT
    public:
    explicit ChatContact(MeeGoChat::ContactManager *cMgr,
                         MeeGoChat::Acct *account,
                     Tp::ContactPtr tpContact,
                         QObject *parent = 0);

        void setAvatar(QPixmap *avatar);

        QVariant getDataByRole(int role);
        QPixmap * getAvatar() const;
        uint getContactHandle() const;
        QString getName() const;
        Acct::AccountStatus getStatus() const;
        QString getStatusMsg() const;
        QString getTpID() const;
        void removeContact();
        void requestChannel(QString channelType = TP_QT_IFACE_CHANNEL_TYPE_TEXT);
        Message * sendMessage(QString message);
        Message * getLastMessage(Message::Status dir = Message::NONE) const;
        const QList<Message *> getMessageList() const;
        Acct * getAccount() const;
        ContactManager * getContactManager() const;
        QtMobility::QContact *getQContact() const;
        bool openPeople() const;
        //TODO, if we end up needing it...
        //Message * getPrevMsg(Message *curMsg);
        void handleChannel(Tp::ChannelPtr chanPtr);
        void setTpContact(Tp::ContactPtr contactPtr);
        Tp::ContactPtr getTpContact() const;
        //Ugly hack to support UI asking the ChatContact
        //to act like it was updated so we can do an easy
        //UI refresh in MLists, for such things as "fuzzy"
        //date/time stamps. Just causes a ContactChanged signal
        //to be emitted...
        void triggerChanged();

    signals:
        void ContactChanged(ChatContact *);
        void ContactRemoved(ChatContact *);
        void NewChannel(ChatContact *, QString channelType);
        void ReceivedMessage(ChatContact *, Message *);
        void ChannelRequestError(ChatContact *, QString errMsg);
        void ChannelRequestSuccess(ChatContact *);

    public slots:

    private slots:
        void onAccountOffline();
        void onAccountOnline();
        void onContactChanged();
        void onTextChannelReady(Tp::PendingOperation *po);
        void onTextChannelInvalidated();
        void onMessageReceived(Tp::ReceivedMessage rMsg);
        void onChannelRequestSucceeded();
        void onChannelRequestFailed(QString errName, QString errMsg);
        void associateQContact();
        void associateQContactAdded();
        void associateQContactChanged();
        void associateQContactRemoved();

    private:
        Message * sendMessage(Message *msg);
        bool haveTextChannel() const;

        MeeGoChat::ContactManager *mContactMgr;
        MeeGoChat::Acct *mAccount;
        Tp::ContactPtr mTpContact;
        QtMobility::QContact *mQContact;
        QPixmap *mAvatar;
        QList<Message *> mMsgQueue;
        QList<Message *> mMsgList;
        //A hack for now, until I get around to properly implementing a MessageModel class...
        Message *mLastIncomingMessage;
        Message *mLastOutgoingMessage;
        Message *mLastPendingSendMessage;
//        QHash<QString, Tp::ChannelPtr> mChannels;
        Tp::TextChannelPtr mTextChan;
        QString mTpID;
        QString mPeopleID;
        bool mAccountOnline;


    };

}

Q_DECLARE_METATYPE(MeeGoChat::ChatContact *);

#endif // MEEGOCHATCONTACT_H
