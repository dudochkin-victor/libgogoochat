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

#include "meegochatmessage.h"

namespace MeeGoChat {

    Message::Message(ChatContact *contact, QString text, Status status,
                     QDateTime dateTime, QObject *parent) :
        QObject(parent),
        mContact(contact),
        mText(text),
        mDateTime(dateTime),
        mStatus(status)
    {
    }

    QString Message::getText() const
    {
        return mText;
    }

    void Message::setText(QString text)
    {
        mText = text;
        emit MessageChanged(this);
    }

    QDateTime Message::getDateTime() const
    {
        return mDateTime;
    }

    void Message::setDateTime(QDateTime dateTime)
    {
        mDateTime = dateTime;
        emit MessageChanged(this);
    }

    Message::Status Message::getStatus() const
    {
        return mStatus;
    }

    void Message::setStatus(Message::Status status)
    {
        if ((status == Message::SENT) && (mStatus == Message::PENDINGSEND)) {
            emit MessageSent(this);
        }
        mStatus = status;
        emit MessageChanged(this);
    }

    QString Message::getTpMessageToken() const
    {
        return mTpMessageToken;
    }

    void Message::setTpMessageToken(QString token)
    {
        mTpMessageToken = token;
        emit MessageChanged(this);
    }

    //Convenience function for a quick incoming/outgoing bool test
    bool Message::getIncoming() const
    {
        return (mStatus == Message::RECEIVED);
    }

    ChatContact * Message::getContact() const
    {
        return mContact;
    }

} // namespace MeeGoChat
