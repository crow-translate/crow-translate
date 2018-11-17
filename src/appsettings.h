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

#include "qonlinetranslator.h"
#include "langbuttongroup.h"

#include <QSettings>
#include <QLocale>
#include <QNetworkProxy>
#include <QTranslator>

class AppSettings : private QSettings
{
    Q_OBJECT
public:
    enum WindowMode {
        PopupWindow,
        MainWindow
    };
    Q_ENUM(WindowMode)
    enum Engine {
        Google,
        Yandex
    };
    Q_ENUM(Engine)
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
    void setupLocale();
    QLocale::Language locale();
    void setLocale(QLocale::Language lang);
    void loadLocale(QLocale::Language lang);

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

    bool showSourceTranscription();
    void setShowSourceTranscription(bool show);

    bool showTranslationOptions();
    void setShowTranslationOptions(bool show);

    bool showDefinitions();
    void setShowDefinitions(bool show);

    QOnlineTranslator::Language primaryLanguage();
    void setPrimaryLanguage(QOnlineTranslator::Language lang);

    QOnlineTranslator::Language secondaryLanguage();
    void setSecondaryLanguage(QOnlineTranslator::Language lang);

    // Speech synthesis settings
    QOnlineTranslator::Speaker speaker();
    void setSpeaker(QOnlineTranslator::Speaker speaker);

    QOnlineTranslator::Emotion emotion();
    void setEmotion(QOnlineTranslator::Emotion emotion);

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
    constexpr const char *defaultTranslateSelectionHotkey()
    { return "Ctrl+Alt+E"; }

    QString playSelectionHotkey();
    void setPlaySelectionHotkey(const QString &hotkey);
    constexpr const char *defaultPlaySelectionHotkey()
    { return "Ctrl+Alt+S"; }

    QString playTranslatedSelectionHotkey();
    void setPlayTranslatedSelectionHotkey(const QString &hotkey);
    constexpr const char *defaultPlayTranslatedSelectionHotkey()
    { return "Ctrl+Alt+F"; }

    QString stopSelectionHotkey();
    void setStopSelectionHotkey(const QString &hotkey);
    constexpr const char *defaultStopSelectionHotkey()
    { return "Ctrl+Alt+G"; }

    QString showMainWindowHotkey();
    void setShowMainWindowHotkey(const QString &hotkey);
    constexpr const char *defaultShowMainWindowHotkey()
    { return "Ctrl+Alt+C"; }

    QString copyTranslatedSelectionHotkey();
    void setCopyTranslatedSelectionHotkeyHotkey(const QString &hotkey);
    constexpr const char *defaultCopyTranslatedSelectionHotkey()
    { return ""; }

    // Window shortcuts
    QString translateHotkey();
    void setTranslateHotkey(const QString &hotkey);
    constexpr const char *defaultTranslateHotkey()
    { return "Ctrl+Return"; }

    QString closeWindowHotkey();
    void setCloseWindowHotkey(const QString &hotkey);
    constexpr const char *defaultCloseWindowHotkey()
    { return "Ctrl+Q"; }

    QString playSourceHotkey();
    void setPlaySourceHotkey(const QString &hotkey);
    constexpr const char *defaultPlaySourceHotkey()
    { return "Ctrl+S"; }

    QString stopSourceHotkey();
    void setStopSourceHotkey(const QString &hotkey);
    constexpr const char *defaultStopSourceHotkey()
    { return "Ctrl+G"; }

    QString playTranslationHotkey();
    void setPlayTranslationHotkey(const QString &hotkey);
    constexpr const char *defaultPlayTranslationHotkey()
    { return "Ctrl+Shift+S"; }

    QString stopTranslationHotkey();
    void setStopTranslationHotkey(const QString &hotkey);
    constexpr const char *defaultStopTranslationHotkey()
    { return "Ctrl+Shift+G"; }

    QString copyTranslationHotkey();
    void setCopyTranslationHotkey(const QString &hotkey);
    constexpr const char *defaultCopyTranslationHotkey()
    { return "Ctrl+Shift+C"; }

    // Buttons
    QOnlineTranslator::Language buttonLanguage(LangButtonGroup *group, int id);
    void setButtonLanguage(LangButtonGroup *group, int id, QOnlineTranslator::Language lang);

    int checkedButton(LangButtonGroup *group);
    void setCheckedButton(LangButtonGroup *group, int id);

    // Main window settings
    QByteArray mainWindowGeometry();
    void setMainWindowGeometry(const QByteArray &geometry);

    bool isAutoTranslateEnabled();
    void setAutoTranslateEnabled(bool enable);

    Engine currentEngine();
    void setCurrentEngine(Engine currentEngine);

private:
    static QTranslator m_appTranslator;
};

#endif // APPSETTINGS_H
