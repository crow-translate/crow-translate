/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "xdgdesktopportal.h"

#ifdef WITH_KWAYLAND
#include "waylandhelper.h"
#endif

#include <QDebug>
#include <QWindow>
#include <QX11Info>

QString XdgDesktopPortal::parentWindow(QWindow *activeWindow)
{
    if (!QX11Info::isPlatformX11()) {
#ifdef WITH_KWAYLAND
        WaylandHelper wayland(activeWindow);
        QEventLoop loop;
        QObject::connect(&wayland, &WaylandHelper::xdgExportDone, &loop, &QEventLoop::quit);
        loop.exec();
        const QString handle = wayland.exportedHandle();
        if (handle.isEmpty()) {
            return {};
        }
        return QStringLiteral("wayland:%1").arg(handle);
#else
        return {};
#endif
    }

    return QStringLiteral("x11:%1").arg(activeWindow->winId(), 0, 16);
}
