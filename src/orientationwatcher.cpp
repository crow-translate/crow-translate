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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "orientationwatcher.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QWindow>
#endif

OrientationWatcher::OrientationWatcher(QWidget *parent)
    : QObject(parent)
{
    for (QScreen *screen : QGuiApplication::screens())
        listenForOrientationChange(screen);
    connect(qobject_cast<QGuiApplication *>(QCoreApplication::instance()), &QGuiApplication::screenAdded, this, &OrientationWatcher::listenForOrientationChange);
}

void OrientationWatcher::listenForOrientationChange(QScreen *screen)
{
    connect(screen, &QScreen::orientationChanged, [this, screen] (Qt::ScreenOrientation orientation) {
        auto *widget = qobject_cast<QWidget *>(parent());
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        if (widget->screen() == screen)
#else
        if (widget->windowHandle() && widget->windowHandle()->screen() == screen)
#endif
            emit screenOrientationChanged(orientation);
    });
    screen->setOrientationUpdateMask(Qt::LandscapeOrientation
                                     | Qt::PortraitOrientation
                                     | Qt::InvertedLandscapeOrientation
                                     | Qt::InvertedPortraitOrientation);
}
