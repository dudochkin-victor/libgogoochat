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

#include "meegochatcontactsortfilterproxymodel.h"
#include "meegochatcontact.h"
#include "meegochatmessage.h"
#include "meegochatcontactmodel.h"

namespace MeeGoChat
{

    ContactSortFilterProxyModel::ContactSortFilterProxyModel(QObject *parent) :
        QSortFilterProxyModel(parent),
        mfCrit(FILTER_NONE),
        msCrit(SORT_STATUS)
    {
    }


    bool ContactSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        QVariant leftData = left.data(Qt::UserRole);//ChatContact *
        QVariant rightData = right.data(Qt::UserRole);//ChatContact*

        ChatContact * leftContact = leftData.value<ChatContact *>();
        ChatContact * rightContact = rightData.value<ChatContact *>();

        if (!leftContact || !rightContact)
            return false;


        switch (msCrit) {
        case ContactSortFilterProxyModel::SORT_STATUS:
            if (leftContact->getStatus() == rightContact->getStatus()) {
                return QString::localeAwareCompare(leftContact->getName(),
                                                   rightContact->getName()) < 0;
            } else {
                return leftContact->getStatus() < rightContact->getStatus();
            }
            break;
        case ContactSortFilterProxyModel::SORT_LASTMSGRCVD:
        default:
            Message *leftMsg = leftContact->getLastMessage(Message::RECEIVED);
            Message *rightMsg = rightContact->getLastMessage(Message::RECEIVED);
            if (leftMsg && rightMsg)
            {
                return leftMsg->getDateTime() < rightMsg->getDateTime();
            } else
            {
                if (leftMsg)
                    return true;
                else if (rightMsg)
                    return false;
                else
                    return QString::localeAwareCompare(leftContact->getName(),
                                                   rightContact->getName()) < 0;
            }
            break;
        }
        //GCC mistakenly throws a "control reaches end of non-void function"
        //warning without the following...
        return false;
    }

    bool ContactSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
    {
        ContactModel *model =
                qobject_cast<ContactModel *>(this->sourceModel());
        if (!model)
            return false;
        ChatContact *contact =
                model->data(
                        model->index(source_row, 0, source_parent), Qt::UserRole
                            ).value<ChatContact *>();
        if (!contact)
            return false;
        switch (mfCrit)
        {
        case ContactSortFilterProxyModel::FILTER_INBOX:
            return (contact->getLastMessage() != 0);
            break;
        case ContactSortFilterProxyModel::FILTER_NONE:
        default:
            return true;
            break;
        }
    }

    void ContactSortFilterProxyModel::setFilter(FilterCriteria fCrit)
    {
        mfCrit = fCrit;
        this->filterChanged();
    }


    void ContactSortFilterProxyModel::setSort(SortCriteria sCrit)
    {
        msCrit = sCrit;
    }

}
