/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "trayicon.h"

#include "mainwindow.h"
#include "settings/appsettings.h"

#include <QFileInfo>
#include <QGuiApplication>

TrayIcon::TrayIcon(MainWindow *parent)
    : QSystemTrayIcon(parent)
{
    connect(this, &TrayIcon::activated, this, &TrayIcon::processTrayActivated);
}

void TrayIcon::loadSettings(const AppSettings &settings)
{
    const IconType iconType = settings.trayIconType();
    if (iconType == CustomIcon) {
        const QString customIconName = settings.customIconPath();
        setIcon(customTrayIcon(customIconName));
        if (icon().isNull()) {
            const QString defaultIconName = trayIconName(DefaultIcon);
            showMessage(tr("Invalid tray icon"), tr("The specified icon '%1' is invalid. The default icon will be used.").arg(customIconName));
            setIcon(QIcon::fromTheme(defaultIconName));
        }
    } else {
        setIcon(QIcon::fromTheme(trayIconName(iconType)));
    }

    const bool trayIconVisible = settings.isShowTrayIcon();
    setVisible(trayIconVisible);
    QGuiApplication::setQuitOnLastWindowClosed(!trayIconVisible);
}

QIcon TrayIcon::customTrayIcon(const QString &customName)
{
    if (QIcon::hasThemeIcon(customName))
        return QIcon::fromTheme(customName);
    if (QFileInfo::exists(customName))
        return QIcon(customName);

    return QIcon();
}

QString TrayIcon::trayIconName(TrayIcon::IconType type)
{
    switch (type) {
    case DefaultIcon:
        return QStringLiteral("crow-translate-tray");
    case DarkIcon:
        return QStringLiteral("crow-translate-tray-dark");
    case LightIcon:
        return QStringLiteral("crow-translate-tray-light");
    default:
        return QString();
    }
}

void TrayIcon::processTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Trigger)
        return;

    auto *mainWindow = qobject_cast<MainWindow *>(parent());
    if (mainWindow->isActiveWindow())
        mainWindow->hide();
    else
        mainWindow->open();
}
