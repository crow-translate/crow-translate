/*
 *  Copyright © 2018-2023 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#include "portalautostartmanager.h"

#include "xdgdesktopportal.h"
#include "settings/appsettings.h"

#include <QDBusReply>
#include <QWidget>
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
    auto *window = qobject_cast<QWidget *>(parent())->windowHandle();
    const QVariantMap options{
        {QStringLiteral("reason"), tr("Allow %1 to manage autostart setting for itself.").arg(QCoreApplication::applicationName())},
        {QStringLiteral("autostart"), enabled},
        {QStringLiteral("commandline"), QStringList{QCoreApplication::applicationFilePath()}},
        {QStringLiteral("dbus-activatable"), false},
    };
    const QDBusReply<QDBusObjectPath> reply = s_interface.call(QStringLiteral("RequestBackground"), XdgDesktopPortal::parentWindow(window), options);

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
