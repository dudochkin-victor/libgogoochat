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

#ifndef MEEGOCHATCONTACTSORTFILTERPROXYMODEL_H
#define MEEGOCHATCONTACTSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace MeeGoChat
{

    class ContactSortFilterProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT
    public:
        explicit ContactSortFilterProxyModel(QObject *parent = 0);

        enum SortCriteria {
            SORT_STATUS,
            SORT_LASTMSGRCVD
        };

        enum FilterCriteria {
            FILTER_NONE,
            FILTER_INBOX
        };

        void setFilter(FilterCriteria fCrit);
        void setSort(SortCriteria sCrit);

    protected:
        virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
        virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    signals:

    public slots:

    private:
        FilterCriteria mfCrit;
        SortCriteria msCrit;

    };

}

Q_DECLARE_METATYPE(MeeGoChat::ContactSortFilterProxyModel *);

#endif // MEEGOCHATCONTACTSORTFILTERPROXYMODEL_H
