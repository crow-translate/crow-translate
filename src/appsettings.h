/*
 *  Copyright © 2018 Gennady Chernyshchuk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QLocale>
#include <QNetworkProxy>
#include <QButtonGroup>

#include "qonlinetranslator.h"

class AppSettings : private QSettings
{
    Q_OBJECT
public:
    enum WindowMode {
        PopupWindow,
        MainWindow
    };
    Q_ENUM(WindowMode)
#ifdef Q_OS_WIN
    enum Interval {
        Day,
        Week,
        Month,
        Never
    };
    Q_ENUM(Interval)
#endif

    AppSettings(QObject *parent = nullptr);

    // General settings
    QLocale::Language locale();
    void setLocale(QLocale::Language language);

    WindowMode windowMode();
    void setWindowMode(WindowMode mode);

    bool isTrayIconVisible();
    void setTrayIconVisible(bool visible);

    bool isStartMinimized();
    void setStartMinimized(bool minimized);

    bool isAutostartEnabled();
    void setAutostartEnabled(bool enabled);

#ifdef Q_OS_WIN
    Interval checkForUpdatesInterval();
    void setCheckForUpdatesInterval(Interval interval);

    QDate lastUpdateCheckDate();
    void setLastUpdateCheckDate(const QDate &date);
#endif

    // Interface settings
    double popupOpacity();
    void setPopupOpacity(double opacity);

    int popupHeight();
    void setPopupHeight(int height);

    int popupWidth();
    void setPopupWidth(int width);

    Qt::ToolButtonStyle popupLanguagesStyle();
    void setPopupLanguagesStyle(Qt::ToolButtonStyle style);

    Qt::ToolButtonStyle popupControlsStyle();
    void setPopupControlsStyle(Qt::ToolButtonStyle style);

    Qt::ToolButtonStyle windowLanguagesStyle();
    void setWindowLanguagesStyle(Qt::ToolButtonStyle style);

    Qt::ToolButtonStyle windowControlsStyle();
    void setWindowControlsStyle(Qt::ToolButtonStyle style);

    QString trayIconName();
    void setTrayIconName(const QString &name);

    QString customIconPath();
    void setCustomIconPath(const QString &path);

    // Translation settings
    bool showSourceTranslit();
    void setShowSourceTranslit(bool show);

    bool showTranslationTranslit();
    void setShowTranslationTranslit(bool show);

    bool showTranslationOptions();
    void setShowTranslationOptions(bool show);

    bool showDefinitions();
    void setShowDefinitions(bool show);

    QOnlineTranslator::Language primaryLanguage();
    void setPrimaryLanguage(QOnlineTranslator::Language lang);

    QOnlineTranslator::Language secondaryLanguage();
    void setSecondaryLanguage(QOnlineTranslator::Language lang);

    // Connection settings
    QNetworkProxy::ProxyType proxyType();
    void setProxyType(QNetworkProxy::ProxyType type);

    QString proxyHost();
    void setProxyHost(const QString &hostName);

    quint16 proxyPort();
    void setProxyPort(quint16 port);

    bool isProxyAuthEnabled();
    void setProxyAuthEnabled(bool enabled);

    QString proxyUsername();
    void setProxyUsername(const QString &username);

    QString proxyPassword();
    void setProxyPassword(const QString &password);

    // Global shortcuts
    QString translateSelectionHotkey();
    void setTranslateSelectionHotkey(const QString &hotkey);

    QString playSelectionHotkey();
    void setPlaySelectionHotkey(const QString &hotkey);

    QString playTranslatedSelectionHotkey();
    void setPlayTranslatedSelectionHotkey(const QString &hotkey);

    QString stopSelectionHotkey();
    void setStopSelectionHotkey(const QString &hotkey);

    QString showMainWindowHotkey();
    void setShowMainWindowHotkey(const QString &hotkey);

    QString copyTranslatedSelectionHotkey();
    void setCopyTranslatedSelectionHotkeyHotkey(const QString &hotkey);

    // Window shortcuts
    QString translateHotkey();
    void setTranslateHotkey(const QString &hotkey);

    QString closeWindowHotkey();
    void setCloseWindowHotkey(const QString &hotkey);

    QString playSourceHotkey();
    void setPlaySourceHotkey(const QString &hotkey);

    QString stopSourceHotkey();
    void setStopSourceHotkey(const QString &hotkey);

    QString playTranslationHotkey();
    void setPlayTranslationHotkey(const QString &hotkey);

    QString stopTranslationHotkey();
    void setStopTranslationHotkey(const QString &hotkey);

    QString copyTranslationHotkey();
    void setCopyTranslationHotkey(const QString &hotkey);

    // Buttons
    QOnlineTranslator::Language buttonLanguage(QButtonGroup *group, int id);
    void setButtonLanguage(QButtonGroup *group, int id, QOnlineTranslator::Language lang);

    int checkedButton(QButtonGroup *group);
    void setCheckedButton(QButtonGroup *group, int id);

    // Other settings
    QByteArray mainWindowGeometry();
    void setMainWindowGeometry(const QByteArray &geometry);

    bool isAutoTranslateEnabled();
    void setAutoTranslateEnabled(bool enable);
};

#endif // APPSETTINGS_H
