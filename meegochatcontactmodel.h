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

#ifndef MEEGOCHATCONTACTMODEL_H
#define MEEGOCHATCONTACTMODEL_H

#include <QAbstractListModel>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>

namespace MeeGoChat
{
    class ChatContact;
    class ContactManager;

    class ContactModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        explicit ContactModel(ContactManager *contactMgr,
                              const QList<ChatContact *> &contactList,
                              QObject *parent = 0);
        int rowCount(const QModelIndex &) const;
        QVariant data(const QModelIndex &, int) const;

    signals:

    public slots:

    private slots:
        void onContactAdded(ChatContact *contact);
        void onContactRemoved(ChatContact *contact);
        void onContactUpdated(ChatContact *contact);

    private:
        ContactManager *mContactMgr;
        QList<ChatContact *> mContactList;

    };

}

Q_DECLARE_METATYPE(MeeGoChat::ContactModel *);

#endif // MEEGOCHATCONTACTMODEL_H
