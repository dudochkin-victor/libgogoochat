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

#ifndef MEEGOCHATMESSAGE_H
#define MEEGOCHATMESSAGE_H

#include <QObject>
#include <QDateTime>
#include <QMetaType>

namespace MeeGoChat {

    class ChatContact;

    class Message : public QObject
    {
    Q_OBJECT
    public:
        enum Status {
            RECEIVED,
            SENT,
            PENDINGSEND,
            NONE
        };

        explicit Message(ChatContact *contact, QString text, Status status,
                         QDateTime dateTime = QDateTime::currentDateTime(),
                         QObject *parent = 0);

        QString getText() const;
        void setText(QString text);
        QDateTime getDateTime() const;
        void setDateTime(QDateTime dateTime);
        Message::Status getStatus() const;
        void setStatus(Message::Status status);
        QString getTpMessageToken() const;
        void setTpMessageToken(QString token);
        bool getIncoming() const;
        ChatContact *getContact() const;

    signals:
        void MessageSent(Message *);
        void MessageChanged(Message *);

    public slots:

    private:
        ChatContact *mContact;
        QString mText;
        QDateTime mDateTime;
        Status mStatus;
        QString mTpMessageToken;


    };

} // namespace MeeGoChat

Q_DECLARE_METATYPE(MeeGoChat::Message *);

#endif // MEEGOCHATMESSAGE_H
