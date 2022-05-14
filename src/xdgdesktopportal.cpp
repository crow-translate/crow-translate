
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

#include "xdgdesktopportal.h"

#include <QDebug>
#include <QWindow>
#include <QX11Info>

QString XdgDesktopPortal::parentWindow(const QWindow *activeWindow)
{
    if (!QX11Info::isPlatformX11()) {
        // TODO Implement Wayland window ID using https://wayland.app/protocols/xdg-foreign-unstable-v2
        qWarning() << "Retrieving XDP window ID on Wayland not implemented";
        return {};
    }

    return QStringLiteral("x11:%1").arg(activeWindow->winId(), 0, 16);
}
