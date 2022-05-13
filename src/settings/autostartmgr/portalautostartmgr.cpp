/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "portalautostartmgr.h"

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QtCore>
#include <qdbusinterface.h>

QDBusInterface PortalAutostartMgr::s_interface(QStringLiteral("org.freedesktop.portal.Desktop"),
                                               QStringLiteral("/org/freedesktop/portal/desktop"),
                                               QStringLiteral("org.freedesktop.portal.Background"));

PortalAutostartMgr::PortalAutostartMgr(QObject *parent)
    : AbstractAutostartMgr(parent)
{
}

bool PortalAutostartMgr::isAvailable()
{
    return QFile::exists("/.flatpak-info");
}

bool PortalAutostartMgr::canCheckEnabled()
{
    return false;
}

bool PortalAutostartMgr::isAutostartEnabled()
{
    qFatal("XDP autostart manager cannot determine current autostart state");
}

void PortalAutostartMgr::onHandleResponse(uint, const QVariantMap &results)
{
    qInfo() << QString("Portal response: background %1, autostart %2").arg(results.value("background").toString(), results.value("autostart").toString());
    emit autostartEnabled(results.value("autostart").toBool());
}

void PortalAutostartMgr::setAutostartEnabled(bool enabled)
{
    QString parent_window = ""; // TODO: get the actual window id
    QVariantMap options{
        {"reason", tr("Allow Crow Translate to manage autostart setting for itself.")},
        {"autostart", enabled},
        {"commandline", QStringList{QCoreApplication::applicationFilePath()}},
        {"dbus-activatable", false},
    };
    const QDBusPendingReply<QDBusObjectPath> reply = s_interface.call(QDBus::Block, QStringLiteral("RequestBackground"), parent_window, options);

    if (reply.isError()) {
        emit showError(reply.error().message());
        return;
    }

    bool connected = s_interface.connection().connect(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                      reply.value().path(),
                                                      QStringLiteral("org.freedesktop.portal.Request"),
                                                      QStringLiteral("Response"),
                                                      this,
                                                      SLOT(onHandleResponse(uint, QVariantMap)));
    if (!connected)
        emit showError(tr("Unable to subscribe to response from xdg-desktop-portal."));

    auto *loop = new QEventLoop(this);
    connect(this, &PortalAutostartMgr::autostartEnabled, loop, &QEventLoop::quit);
    loop->exec();
}
