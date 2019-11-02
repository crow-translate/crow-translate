/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
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
#include "qonlinetts.h"
#include "langbuttongroup.h"
#include "trayicon.h"

#include <QLocale>
#include <QNetworkProxy>
#include <QTranslator>

class QSettings;

class AppSettings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AppSettings)

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
    void setupLocale() const;
    QLocale::Language locale() const;
    void setLocale(QLocale::Language lang);
    void loadLocale(QLocale::Language lang) const;
    static QLocale::Language defaultLocale();

    WindowMode windowMode() const;
    void setWindowMode(WindowMode mode);
    static WindowMode defaultWindowMode();

    bool isShowTrayIcon() const;
    void setShowTrayIcon(bool visible);
    static bool defaultShowTrayIcon();

    bool isStartMinimized() const;
    void setStartMinimized(bool minimized);
    static bool defaultStartMinimized();

    bool isAutostartEnabled() const;
    void setAutostartEnabled(bool enabled);
    static bool defaultAutostartEnabled();

#ifdef PORTABLE_MODE
    bool isPortableModeEnabled() const;
    static void setPortableModeEnabled(bool enabled);
    static QString portableConfigName();
#endif

#ifdef Q_OS_WIN
    Interval checkForUpdatesInterval() const;
    void setCheckForUpdatesInterval(Interval interval);
    static Interval defaultCheckForUpdatesInterval();

    QDate lastUpdateCheckDate() const;
    void setLastUpdateCheckDate(const QDate &date);
#endif

    // Interface settings
    double popupOpacity() const;
    void setPopupOpacity(double opacity);
    static double defaultPopupOpacity();

    int popupHeight() const;
    void setPopupHeight(int height);
    static int defaultPopupHeight();

    int popupWidth() const;
    void setPopupWidth(int width);
    static int defaultPopupWidth();

    Qt::ToolButtonStyle popupLanguagesStyle() const;
    void setPopupLanguagesStyle(Qt::ToolButtonStyle style);
    static Qt::ToolButtonStyle defaultPopupLanguagesStyle();

    Qt::ToolButtonStyle popupControlsStyle() const;
    void setPopupControlsStyle(Qt::ToolButtonStyle style);
    static Qt::ToolButtonStyle defaultPopupControlsStyle();

    Qt::ToolButtonStyle windowLanguagesStyle() const;
    void setWindowLanguagesStyle(Qt::ToolButtonStyle style);
    static Qt::ToolButtonStyle defaultWindowLanguagesStyle();

    Qt::ToolButtonStyle windowControlsStyle() const;
    void setWindowControlsStyle(Qt::ToolButtonStyle style);
    static Qt::ToolButtonStyle defaultWindowControlsStyle();

    TrayIcon::IconType trayIconType() const;
    void setTrayIconType(TrayIcon::IconType icon);
    static TrayIcon::IconType defaultTrayIconType();

    QString customIconPath() const;
    void setCustomIconPath(const QString &path);
    static QString defaultCustomIconPath();

    // Translation settings
    bool isSourceTranslitEnabled() const;
    void setSourceTranslitEnabled(bool enable);
    static bool defaultSourceTranslitEnabled();

    bool isTranslationTranslitEnabled() const;
    void setTranslationTranslitEnabled(bool enable);
    static bool defaultTranslationTranslitEnabled();

    bool isSourceTranscriptionEnabled() const;
    void setSourceTranscriptionEnabled(bool enable);
    static bool defaultSourceTranscriptionEnabled();

    bool isTranslationOptionsEnabled() const;
    void setTranslationOptionsEnabled(bool enable);
    static bool defaultTranslationOptionsEnabled();

    bool isExamplesEnabled() const;
    void setExamplesEnabled(bool enable);
    static bool defaultExamplesEnabled();

    QOnlineTranslator::Language primaryLanguage() const;
    void setPrimaryLanguage(QOnlineTranslator::Language lang);
    static QOnlineTranslator::Language defaultPrimaryLanguage();

    QOnlineTranslator::Language secondaryLanguage() const;
    void setSecondaryLanguage(QOnlineTranslator::Language lang);
    static QOnlineTranslator::Language defaultSecondaryLanguage();

    QOnlineTranslator::Language preferredTranslationLanguage(QOnlineTranslator::Language sourceLang) const;

    bool isForceSourceAutodetect() const;
    void setForceSourceAutodetect(bool force);
    static bool defaultForceSourceAutodetect();

    bool isForceTranslationAutodetect() const;
    void setForceTranslationAutodetect(bool force);
    static bool defaultForceTranslationAutodetect();

    // Speech synthesis settings
    QOnlineTts::Voice voice(QOnlineTranslator::Engine engine) const;
    void setVoice(QOnlineTranslator::Engine engine, QOnlineTts::Voice voice);
    static QOnlineTts::Voice defaultVoice(QOnlineTranslator::Engine engine);

    QOnlineTts::Emotion emotion(QOnlineTranslator::Engine engine) const;
    void setEmotion(QOnlineTranslator::Engine engine, QOnlineTts::Emotion emotion);
    static QOnlineTts::Emotion defaultEmotion(QOnlineTranslator::Engine engine);

    // Connection settings
    QNetworkProxy::ProxyType proxyType() const;
    void setProxyType(QNetworkProxy::ProxyType type);
    static QNetworkProxy::ProxyType defaultProxyType();

    QString proxyHost() const;
    void setProxyHost(const QString &hostName);
    static QString defaultProxyHost();

    quint16 proxyPort() const;
    void setProxyPort(quint16 port);
    static quint16 defaultProxyPort();

    bool isProxyAuthEnabled() const;
    void setProxyAuthEnabled(bool enabled);
    static bool defaultProxyAuthEnabled();

    QString proxyUsername() const;
    void setProxyUsername(const QString &username);
    static QString defaultProxyUsername();

    QString proxyPassword() const;
    void setProxyPassword(const QString &password);
    static QString defaultProxyPassword();

    // Global shortcuts
    QKeySequence translateSelectionHotkey() const;
    void setTranslateSelectionHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultTranslateSelectionHotkey();

    QKeySequence speakSelectionHotkey() const;
    void setSpeakSelectionHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultSpeakSelectionHotkey();

    QKeySequence speakTranslatedSelectionHotkey() const;
    void setSpeakTranslatedSelectionHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultSpeakTranslatedSelectionHotkey();

    QKeySequence stopSpeakingHotkey() const;
    void setStopSpeakingHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultStopSpeakingHotkey();

    QKeySequence showMainWindowHotkey() const;
    void setShowMainWindowHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultShowMainWindowHotkey();

    QKeySequence copyTranslatedSelectionHotkey() const;
    void setCopyTranslatedSelectionHotkeyHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultCopyTranslatedSelectionHotkey();

    // Window shortcuts
    QKeySequence translateHotkey() const;
    void setTranslateHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultTranslateHotkey();

    QKeySequence closeWindowHotkey() const;
    void setCloseWindowHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultCloseWindowHotkey();

    QKeySequence speakSourceHotkey() const;
    void setSpeakSourceHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultSpeakSourceHotkey();

    QKeySequence speakTranslationHotkey() const;
    void setSpeakTranslationHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultSpeakTranslationHotkey();

    QKeySequence copyTranslationHotkey() const;
    void setCopyTranslationHotkey(const QKeySequence &hotkey);
    static QKeySequence defaultCopyTranslationHotkey();

    // Buttons
    QOnlineTranslator::Language buttonLanguage(LangButtonGroup::GroupType group, int id) const;
    void setButtonLanguage(LangButtonGroup::GroupType group, int id, QOnlineTranslator::Language lang);

    int checkedButton(LangButtonGroup::GroupType group) const;
    void setCheckedButton(LangButtonGroup::GroupType group, int id);

    // Main window settings
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);

    bool isAutoTranslateEnabled() const;
    void setAutoTranslateEnabled(bool enable);

    QOnlineTranslator::Engine currentEngine() const;
    void setCurrentEngine(QOnlineTranslator::Engine currentEngine);

private:
    QSettings *m_settings;

    static QTranslator m_appTranslator;
    static QTranslator m_qtTranslator; // Qt library translations
#ifdef PORTABLE_MODE
    const static QString s_portableConfigName;
#endif
};

#endif // APPSETTINGS_H
