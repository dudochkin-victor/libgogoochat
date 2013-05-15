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

#ifndef MEEGOCHATCLIENTHANDLER_H
#define MEEGOCHATCLIENTHANDLER_H

#include <TelepathyQt/Channel>
#include <TelepathyQt/ChannelRequest>
#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/ClientRegistrar>


namespace MeeGoChat {

    class AccountManager;

    class ClientHandler : public QObject, public Tp::AbstractClientHandler
    {
        Q_OBJECT
    public:

        ClientHandler(
                const Tp::ChannelClassSpecList &channelFilter,
                const Capabilities &capabilities,
                QString clientName,
                bool wantsRequestNotification = false,
                bool enableTpWarnings = false,
                bool enableTpDebug = false,
                QObject *parent = 0);
        virtual ~ClientHandler();

        static ClientHandler * createClient(QString clientName,
                                            bool enableTpWarnings = false,
                                            bool enableTpDebug = false,
                                            QStringList cCaps = QStringList(),
                                            Tp::ChannelClassList cFilters = QList<Tp::ChannelClass>(),
                                            bool wantReqNot = false,
                                            QObject *parent = 0);

        virtual void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                            const Tp::AccountPtr &account,
                            const Tp::ConnectionPtr &connection,
                            const QList<Tp::ChannelPtr> &channels,
                            const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                            const QDateTime &userActionTime,
                            const QVariantMap &handlerInfo);

        virtual bool bypassApproval() const { return false; }

        AccountManager * getAccountManager();
        Tp::AbstractClientPtr getACP();

    signals:

    public slots:

    private:
        AccountManager *mAccountManager;
        Tp::ClientRegistrarPtr mClientRegistrar;
        Tp::AbstractClientPtr mACP;

    };

} // namespace MeeGoChat

Q_DECLARE_METATYPE(MeeGoChat::ClientHandler *);

#endif // MEEGOCHATCLIENTHANDLER_H
