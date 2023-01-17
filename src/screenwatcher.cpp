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

#include "screenwatcher.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QWindow>
#endif

ScreenWatcher::ScreenWatcher(QWidget *parent)
    : QObject(parent)
{
    for (QScreen *screen : QGuiApplication::screens())
        listenForOrientationChange(screen);
    connect(qobject_cast<QGuiApplication *>(QCoreApplication::instance()), &QGuiApplication::screenAdded, this, &ScreenWatcher::listenForOrientationChange);
}

bool ScreenWatcher::isWidthFitScreen(QWidget *widget)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return widget->frameGeometry().width() <= widget->screen()->availableGeometry().width();
#else
    if (!widget->windowHandle())
        widget->winId(); // Call to create handle
    return widget->frameGeometry().width() <= widget->windowHandle()->screen()->availableGeometry().width();
#endif
}

void ScreenWatcher::listenForOrientationChange(QScreen *screen)
{
    connect(screen, &QScreen::orientationChanged, [this, screen](Qt::ScreenOrientation orientation) {
        auto *widget = qobject_cast<QWidget *>(parent());
        // clang-format off
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        if (widget->screen() == screen) {
#else
        if (widget->windowHandle() && widget->windowHandle()->screen() == screen) {
#endif
            emit screenOrientationChanged(orientation);
        }
        // clang-format on
    });
    screen->setOrientationUpdateMask(Qt::LandscapeOrientation
                                     | Qt::PortraitOrientation
                                     | Qt::InvertedLandscapeOrientation
                                     | Qt::InvertedPortraitOrientation);
}
