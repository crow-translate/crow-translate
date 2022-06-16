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

#ifndef TRAYICON_H
#define TRAYICON_H

#include "settings/appsettings.h"

#include <QSystemTrayIcon>

class QAction;
class MainWindow;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
    Q_DISABLE_COPY(TrayIcon)

public:
    explicit TrayIcon(MainWindow *parent = nullptr);

    void setTranslationNotificationTimeout(int timeout);
    void retranslateMenu();
    void showTranslationMessage(const QString &message);

    static QIcon customTrayIcon(const QString &customName);
    static QString trayIconName(AppSettings::IconType type);

private:
    QMenu *m_trayMenu;
    QAction *m_showMainWindowAction;
    QAction *m_openSettingsAction;
    QAction *m_quitAction;

    int m_translationNotificaitonTimeout = AppSettings::defaultTranslationNotificationTimeout();
};

#endif // TRAYICON_H
