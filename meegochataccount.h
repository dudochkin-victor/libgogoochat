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

#ifndef MEEGOCHATACCOUNT_H
#define MEEGOCHATACCOUNT_H

#include <QObject>
#include <QString>
#include <QMetaType>
#include <QVariant>
#include <TelepathyQt/Account>
#include <TelepathyQt/Channel>
#include <TelepathyQt/Connection>
#include <TelepathyQt/PendingChannelRequest>


namespace MeeGoChat
{

    class ContactManager;
    class ChatContact;

    class Acct : public QObject
    {
        Q_OBJECT
    public:

        enum AccountStatus {
            AVAILABLE,
            AWAY,
            BUSY,
            OFFLINE,
            STATUSCOUNT
        };

        explicit Acct(Tp::AccountPtr tpAccount, QObject *parent = 0);

        QVariant getDataByRole(int role);
        QString getDisplayName();
        QString getIcon();
        QString getTpProto();
        QString getTpAcctPath();
        AccountStatus getStatus();
        QString getStatusIcon();
        bool isReady();
        Tp::ConnectionPtr getTpConn();
        void setStatus(AccountStatus newStatus);
        void handleChannel(Tp::ChannelPtr tpChannel);
        void setEnabled(bool enabled);
        bool getEnabled();
        MeeGoChat::ContactManager * getContactManager();
        Tp::PendingChannelRequest * ensureTextChat(ChatContact *contact);
        QVariantMap getTpParameters();
        void setDisplayName(QString newName);
        void updateTpParameters(QVariantMap set, QStringList unset);

        void remove();

        static AccountStatus mapTpStatusToAccountStatus(Tp::ConnectionPresenceType tpStatus);
        static Tp::ConnectionPresenceType mapAccountStatusToTpStatus(AccountStatus status);
        static QString mapAccountStatusToIconName(AccountStatus status);

    public slots:
        void onChannelReady(Tp::PendingOperation *po);

    signals:
        void Ready(Acct *);
        void ReadyError(Acct *, QString errMsg);
        void Updated(Acct *);
        void Offline(Acct *);
        void Reconnect(Acct *);
        void Online(Acct *);
        void StatusChanged(Acct *, Acct::AccountStatus);
        void Disconnected(Acct *);
        void Connected(Acct *);
        void Reconnected(Acct *);
        void ConnectionError(Acct *, QString errMsg);
        void Removed(Acct *);
        void TpParameterUpdateSuccess(Acct *);
        void TpParameterUpdateFailure(Acct *, QString errMsg);

    private slots:
        void onTpAccountReady(Tp::PendingOperation *po);
        void onSetRequestedPresenceFinished(Tp::PendingOperation *po);
        void onTpConnReady(Tp::PendingOperation *po);
        void onHaveConnectionChanged(bool haveConn); //TODO
        void onUpdateTpParametersFinished(Tp::PendingOperation *po);

    private:
        void setStatus(Tp::ConnectionPresenceType type);

        Tp::AccountPtr mTpAccount;
        Tp::ConnectionPtr mTpConn;
        ContactManager *mContactMgr;
        Acct::AccountStatus mPrevStatus, mCurStatus;
        bool mReady, mReadyError;

    };

}

Q_DECLARE_METATYPE(MeeGoChat::Acct *);

#endif // MEEGOCHATACCOUNT_H
