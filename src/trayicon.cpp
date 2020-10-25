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

#include <QFileInfo>
#include <QGuiApplication>
#include <QMenu>

TrayIcon::TrayIcon(MainWindow *parent)
    : QSystemTrayIcon(parent)
    , m_trayMenu(new QMenu(parent))
    , m_showMainWindowAction(m_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("window")), tr("Show window"), parent, &MainWindow::open))
    , m_openSettingsAction(m_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("dialog-object-properties")), tr("Settings"), parent, &MainWindow::openSettings))
    , m_quitAction(m_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), tr("Exit"), parent, &MainWindow::quit))
{
    setContextMenu(m_trayMenu);

    connect(this, &TrayIcon::activated, [parent](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::Trigger)
            return;

        if (parent->isActiveWindow())
            parent->hide();
        else
            parent->open();
    });
}

void TrayIcon::setTranslationNotificationTimeout(int timeout) 
{
    m_translationNotificaitonTimeout = timeout;
}

void TrayIcon::retranslateMenu() 
{
    m_showMainWindowAction->setText(tr("Show window"));
    m_openSettingsAction->setText(tr("Settings"));
    m_quitAction->setText(tr("Quit"));
}

void TrayIcon::showTranslationMessage(const QString &message)
{
    showMessage(tr("Translation result"), message, QSystemTrayIcon::NoIcon, m_translationNotificaitonTimeout * 1000);
}

QIcon TrayIcon::customTrayIcon(const QString &customName)
{
    if (QFileInfo::exists(customName))
        return QIcon(customName);
    return QIcon::fromTheme(customName);
}

QString TrayIcon::trayIconName(AppSettings::IconType type)
{
    switch (type) {
    case AppSettings::DefaultIcon:
        return QStringLiteral("crow-translate-tray");
    case AppSettings::DarkIcon:
        return QStringLiteral("crow-translate-tray-dark");
    case AppSettings::LightIcon:
        return QStringLiteral("crow-translate-tray-light");
    default:
        return QString();
    }
}
