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

#include "meegochatcontactmodel.h"
#include "meegochatcontact.h"
#include "meegochatcontactmanager.h"

#include <TelepathyQt/PendingContacts>

namespace MeeGoChat {

    ContactModel::ContactModel(ContactManager *contactMgr,
                               const QList <ChatContact *> &contactList,
                               QObject *parent) :
        QAbstractListModel(parent),
        mContactMgr(contactMgr)
    {
        mContactList.append(contactList);
        connect(mContactMgr,
                SIGNAL(ContactAdded(ChatContact*)),
                this,
                SLOT(onContactAdded(ChatContact*)));
        connect(mContactMgr,
                SIGNAL(ContactRemoved(ChatContact*)),
                this,
                SLOT(onContactRemoved(ChatContact*)));
        connect(mContactMgr,
                SIGNAL(ContactUpdated(ChatContact*)),
                this,
                SLOT(onContactUpdated(ChatContact*)));
    }

    int ContactModel::rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return mContactList.size();
    }

    QVariant ContactModel::data(const QModelIndex &index, int role) const
    {
        if (index.isValid())
        {
            return mContactList.at(index.row())->getDataByRole(role);
        }
        return QVariant();
    }

    //Private slots

    void ContactModel::onContactAdded(ChatContact *contact)
    {
        emit beginInsertRows(QModelIndex(),
                             mContactList.count(), mContactList.count());
        mContactList.append(contact);
        emit endInsertRows();
    }

    void ContactModel::onContactRemoved(ChatContact *contact)
    {
        int idx;
        if ((idx = mContactList.indexOf(contact)) != -1) {
            emit beginRemoveRows(QModelIndex(), idx, idx);
            mContactList.removeAt(idx);
            emit endRemoveRows();
        }
    }

    void ContactModel::onContactUpdated(ChatContact *contact)
    {
        int idx = mContactList.indexOf(contact);

        if (idx == -1)
            return;
        QModelIndex qmi = this->createIndex(idx, 0, 0);
        emit dataChanged(qmi, qmi);
    }
}
