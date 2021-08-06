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

#include "dbusscreengrabber.h"

#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>

DBusScreenGrabber::DBusScreenGrabber(QObject *parent)
    : AbstractScreenGrabber(parent)
{
}

// Split to separate images per screen
QMap<const QScreen *, QImage> DBusScreenGrabber::splitScreenImages(const QPixmap &pixmap)
{
    QMap<const QScreen *, QImage> images;
    for (QScreen *screen : QGuiApplication::screens()) {
        QRect geom = screen->geometry();
        geom.setSize(screen->size() * screen->devicePixelRatio());
        images.insert(screen, pixmap.copy(geom).toImage());
    }
    return images;
}

void DBusScreenGrabber::cancel()
{
    if (m_callWatcher != nullptr) {
        m_callWatcher->deleteLater();
        m_callWatcher = nullptr;
    }
}
