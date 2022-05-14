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

#include "portalautostartmanager.h"

#include "settings/appsettings.h"

#include <QDBusReply>
#include <QtCore>

QDBusInterface PortalAutostartManager::s_interface(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                   QStringLiteral("/org/freedesktop/portal/desktop"),
                                                   QStringLiteral("org.freedesktop.portal.Background"));

PortalAutostartManager::PortalAutostartManager(QObject *parent)
    : AbstractAutostartManager(parent)
{
}

bool PortalAutostartManager::isAutostartEnabled() const
{
    return AppSettings().isAutostartEnabled();
}

void PortalAutostartManager::setAutostartEnabled(bool enabled)
{
    const QVariantMap options{
        {QStringLiteral("reason"), tr("Allow %1 to manage autostart setting for itself.").arg(QCoreApplication::applicationName())},
        {QStringLiteral("autostart"), enabled},
        {QStringLiteral("commandline"), QStringList{QCoreApplication::applicationFilePath()}},
        {QStringLiteral("dbus-activatable"), false},
    };
    // TODO: Retrieve parent window in string form
    // as a second argument according to https://flatpak.github.io/xdg-desktop-portal/#parent_window
    const QDBusReply<QDBusObjectPath> reply = s_interface.call(QStringLiteral("RequestBackground"), QString(), options);

    if (!reply.isValid()) {
        showError(reply.error().message());
        return;
    }

    const bool connected = s_interface.connection().connect(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                            reply.value().path(),
                                                            QStringLiteral("org.freedesktop.portal.Request"),
                                                            QStringLiteral("Response"),
                                                            this,
                                                            SLOT(parsePortalResponse(quint32, QVariantMap)));
    if (!connected) {
        showError(tr("Unable to subscribe to response from xdg-desktop-portal."));
        return;
    }

    QEventLoop loop;
    connect(this, &PortalAutostartManager::responseParsed, &loop, &QEventLoop::quit);
    loop.exec();
}

bool PortalAutostartManager::isAvailable()
{
    return QFile::exists(QStringLiteral("/.flatpak-info")) && s_interface.isValid();
}

void PortalAutostartManager::parsePortalResponse(quint32, const QVariantMap &results)
{
    AppSettings().setAutostartEnabled(results.value(QStringLiteral("autostart")).toBool());
    emit responseParsed();
}
