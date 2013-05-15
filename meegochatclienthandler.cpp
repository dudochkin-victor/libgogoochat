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

#include "meegochatclienthandler.h"
#include "meegochataccountmanager.h"

#include <TelepathyQt/Debug>
#include <TelepathyQt/Channel>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/Constants>

namespace MeeGoChat {


    ClientHandler::ClientHandler(
                const Tp::ChannelClassSpecList &channelFilter,
                const Capabilities &capabilities,
                QString clientName,
                bool wantsRequestNotification,
                bool enableTpWarnings,
                bool enableTpDebug,
                QObject *parent) :
            QObject(parent),
            Tp::AbstractClientHandler(channelFilter, capabilities,
                                      wantsRequestNotification),
            mAccountManager(AccountManager::getInstance(enableTpWarnings, enableTpDebug))
    {
        mClientRegistrar = Tp::ClientRegistrar::create();
        mACP = Tp::AbstractClientPtr::dynamicCast(Tp::SharedPtr<ClientHandler>(this));
        mClientRegistrar->registerClient(mACP, clientName);
    }

    ClientHandler::~ClientHandler()
    {
        mClientRegistrar->unregisterClient(Tp::AbstractClientPtr::dynamicCast(Tp::SharedPtr<ClientHandler>(this)));
    }


    ClientHandler * ClientHandler::createClient(QString clientName,
                                                bool enableTpWarnings,
                                                bool enableTpDebug,
                                                QStringList cCaps,
                                                Tp::ChannelClassList cFilters,
                                                bool wantReqNot,
                                                QObject *parent)
    {
        Tp::registerTypes();
        Tp::enableWarnings(enableTpWarnings);
        Tp::enableDebug(enableTpDebug);

        //If client capabilities aren't specified, assume just Text Channels
        if (cCaps.count() == 0) {
            cCaps.append(
                TP_QT_IFACE_CHANNEL + QLatin1String(".TextChannel"));
        }

        if (cFilters.count() == 0) {
            QMap<QString, QDBusVariant> filter;

            filter.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"),
                          QDBusVariant(QLatin1String(TP_QT_IFACE_CHANNEL_TYPE_TEXT)));
            filter.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                          QDBusVariant((uint) Tp::HandleTypeContact));
            cFilters.append(filter);
        }
        /*ClientHandler *clientHandler = new ClientHandler(cFilters, cCaps, clientName, wantReqNot, enableTpWarnings, enableTpDebug, parent);

        return clientHandler;*/
        return NULL; //DV
    }

    void ClientHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                       const Tp::AccountPtr &account,
                                       const Tp::ConnectionPtr &connection,
                                       const QList<Tp::ChannelPtr> &channels,
                                       const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                       const QDateTime &userActionTime,
                                       const QVariantMap &handlerInfo)
    {
        Q_UNUSED(connection);
        Q_UNUSED(requestsSatisfied);
        Q_UNUSED(userActionTime);
        Q_UNUSED(handlerInfo);

        foreach (const Tp::ChannelPtr &channel, channels)
        {
            //If we are trying to use an invalid account (aka not found or creatable in MeeGoChat::AccountManager::mAccounts)
            //then just drop the whole set of channels, since they are all for the same account...
            //The only way we should really ever hit this (at least, at this point), is if
            //we hit this code path before our AccountManager is ready - aka before the Ready() signal comes through...
            if (!mAccountManager->onChannelToBeHandled(account, channel)) {
                context->setFinishedWithError(QString("Invalid Account"), QString("The account specified in the channel to be handled was not found in the Account Manager!"));
                return;
            }
        }
        context->setFinished();
    }

    AccountManager * ClientHandler::getAccountManager()
    {
        return mAccountManager;
    }

    Tp::AbstractClientPtr ClientHandler::getACP()
    {
        return mACP;
    }

} // namespace MeeGoChat
