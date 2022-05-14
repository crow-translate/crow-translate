/*
 *  Copyright © 2018-2021 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "waylandportalscreengrabber.h"

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QFile>
#include <QPixmap>
#include <QUrl>

// https://github.com/flatpak/xdg-desktop-portal/blob/89d2197002f164d02d891c530dcaa2808f27f593/data/org.freedesktop.portal.Screenshot.xml
QDBusInterface WaylandPortalScreenGrabber::s_interface(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                       QStringLiteral("/org/freedesktop/portal/desktop"),
                                                       QStringLiteral("org.freedesktop.portal.Screenshot"));

WaylandPortalScreenGrabber::WaylandPortalScreenGrabber(QObject *parent)
    : DBusScreenGrabber(parent)
{
}

bool WaylandPortalScreenGrabber::ignoreDevicePixelRatio() const
{
    return qgetenv("XDG_CURRENT_DESKTOP") == "KDE";
}

bool WaylandPortalScreenGrabber::isAvailable()
{
    return s_interface.isValid();
}

void WaylandPortalScreenGrabber::grab()
{
    // TODO: Retrieve parent window in string form
    // as a second argument according to https://flatpak.github.io/xdg-desktop-portal/#parent_window
    const QDBusPendingReply<QDBusObjectPath> reply = s_interface.asyncCall(QStringLiteral("Screenshot"), QString(), QVariantMap());
    m_callWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(m_callWatcher, &QDBusPendingCallWatcher::finished, [this] {
        const QDBusPendingReply<QDBusObjectPath> reply = readReply<QDBusObjectPath>();

        if (reply.isError()) {
            emit showError(reply.error().message());
            return;
        }

        m_responseServicePath = reply.value().path();
        bool success = QDBusConnection::sessionBus().connect({},
                                                             m_responseServicePath,
                                                             QLatin1String("org.freedesktop.portal.Request"),
                                                             QLatin1String("Response"),
                                                             this,
                                                             SLOT(parsePortalResponse(uint, QVariantMap)));
        if (!success)
            emit showError(tr("Unable to subscribe to response from xdg-desktop-portal."));
    });
}

void WaylandPortalScreenGrabber::cancel()
{
    DBusScreenGrabber::cancel();

    QDBusConnection::sessionBus().disconnect({},
                                             m_responseServicePath,
                                             QLatin1String("org.freedesktop.portal.Request"),
                                             QLatin1String("Response"),
                                             this,
                                             SLOT(parsePortalResponse(uint, QVariantMap)));
}

void WaylandPortalScreenGrabber::parsePortalResponse(uint, const QVariantMap &response)
{
    QString path = response.value(QLatin1String("uri")).toUrl().toLocalFile();
    if (path.isEmpty()) {
        emit showError(tr("Received an empty path from xdg-desktop-portal."));
        return;
    }

    emit grabbed(splitScreenImages(path));
    QFile::remove(path);
}
