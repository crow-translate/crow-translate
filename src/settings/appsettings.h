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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "qonlinetts.h"

#include <QLocale>
#include <QNetworkProxy>

class QTranslator;
class QSettings;

class AppSettings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AppSettings)

public:
    enum LanguageFormat {
        FullName,
        IsoCode
    };
    Q_ENUM(LanguageFormat)

    enum LanguageButtonsType {
        Source,
        Translation
    };
    Q_ENUM(LanguageButtonsType)

    enum WindowMode {
        PopupWindow,
        MainWindow,
        Notification
    };
    Q_ENUM(WindowMode)

    enum RegionRememberType {
        NeverRemember,
        RememberLast,
        RememberAlways
    };
    Q_ENUM(RegionRememberType)

    enum IconType {
        DefaultIcon,
        LightIcon,
        DarkIcon,
        CustomIcon
    };
    Q_ENUM(IconType)

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
    void setupLocalization() const;
    QLocale::Language language() const;
    void setLanguage(QLocale::Language lang);
    static void applyLanguage(QLocale::Language lang);
    static QLocale::Language defaultLanguage();

    WindowMode windowMode() const;
    void setWindowMode(WindowMode mode);
    static WindowMode defaultWindowMode();

    bool isShowTrayIcon() const;
    void setShowTrayIcon(bool visible);
    static bool defaultShowTrayIcon();

    bool isStartMinimized() const;
    void setStartMinimized(bool minimized);
    static bool defaultStartMinimized();

    static bool isAutostartEnabled();
    static void setAutostartEnabled(bool enabled);
    static bool defaultAutostartEnabled();

#ifdef WITH_PORTABLE_MODE
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
    QFont font() const;
    void setFont(const QFont &font);

    double popupOpacity() const;
    void setPopupOpacity(double opacity);
    static double defaultPopupOpacity();

    int popupHeight() const;
    void setPopupHeight(int height);
    static int defaultPopupHeight();

    int popupWidth() const;
    void setPopupWidth(int width);
    static int defaultPopupWidth();

    int translationNotificationTimeout() const;
    void setTranslationNotificationTimeout(int timeout);
    static int defaultTranslationNotificationTimeout();

    LanguageFormat popupLanguageFormat() const;
    void setPopupLanguageFormat(LanguageFormat style);
    static LanguageFormat defaultPopupLanguageFormat();

    LanguageFormat mainWindowLanguageFormat() const;
    void setMainWindowLanguageFormat(LanguageFormat style);
    static LanguageFormat defaultMainWindowLanguageFormat();

    IconType trayIconType() const;
    void setTrayIconType(IconType type);
    static IconType defaultTrayIconType();

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

    bool isSimplifySource() const;
    void setSimplifySource(bool simplify);
    static bool defaultSimplifySource();

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
    bool isGlobalShortuctsEnabled() const;
    void setGlobalShortcutsEnabled(bool enabled);
    static bool defaultGlobalShortcutsEnabled();

    QKeySequence translateSelectionShortcut() const;
    void setTranslateSelectionShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultTranslateSelectionShortcut();

    QKeySequence speakSelectionShortcut() const;
    void setSpeakSelectionShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultSpeakSelectionShortcut();

    QKeySequence speakTranslatedSelectionShortcut() const;
    void setSpeakTranslatedSelectionShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultSpeakTranslatedSelectionShortcut();

    QKeySequence stopSpeakingShortcut() const;
    void setStopSpeakingShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultStopSpeakingShortcut();

    QKeySequence showMainWindowShortcut() const;
    void setShowMainWindowShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultShowMainWindowShortcut();

    QKeySequence copyTranslatedSelectionShortcut() const;
    void setCopyTranslatedSelectionShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultCopyTranslatedSelectionShortcut();

    QKeySequence recognizeScreenAreaShortcut() const;
    void setRecognizeScreenAreaShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultRecognizeScreenAreaShortcut();

    QKeySequence translateScreenAreaShortcut() const;
    void setTranslateScreenAreaShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultTranslateScreenAreaShortcut();

    // Window shortcuts
    QKeySequence translateShortcut() const;
    void setTranslateShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultTranslateShortcut();

    QKeySequence swapShortcut() const;
    void setSwapShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultSwapShortcut();

    QKeySequence closeWindowShortcut() const;
    void setCloseWindowShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultCloseWindowShortcut();

    QKeySequence speakSourceShortcut() const;
    void setSpeakSourceShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultSpeakSourceShortcut();

    QKeySequence speakTranslationShortcut() const;
    void setSpeakTranslationShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultSpeakTranslationShortcut();

    QKeySequence copyTranslationShortcut() const;
    void setCopyTranslationShortcut(const QKeySequence &shortcut);
    static QKeySequence defaultCopyTranslationShortcut();

    // OCR settings
    bool isConvertLineBreaks() const;
    void setConvertLineBreaks(bool convert);
    static bool defaultConvertLineBreaks();

    QByteArray ocrLanguagesPath() const;
    void setOcrLanguagesPath(const QByteArray &path);
    static QByteArray defaultOcrLanguagesPath();

    QByteArray ocrLanguagesString() const;
    void setOcrLanguagesString(const QByteArray &string);
    static QByteArray defaultOcrLanguagesString();

    QMap<QString, QVariant> tesseractParameters() const;
    void setTesseractParameters(const QMap<QString, QVariant> &parameters);
    static QMap<QString, QVariant> defaultTesseractParameters();

    bool isShowMagnifier() const;
    void setShowMagnifier(bool show);
    static bool defaultShowMagnifier();

    bool isCaptureOnRelease() const;
    void setCaptureOnRelease(bool capture);
    static bool defaultCaptureOnRelease();

    bool isApplyLightMask() const;
    void setApplyLightMask(bool use);
    static bool defaultApplyLightMask();

    RegionRememberType regionRememberType() const;
    void setRegionRememberType(RegionRememberType type);
    static RegionRememberType defaultRegionRememberType();

    QRect cropRegion() const;
    void setCropRegion(QRect rect);


    // Buttons
    QVector<QOnlineTranslator::Language> languages(LanguageButtonsType type) const;
    void setLanguages(LanguageButtonsType type, const QVector<QOnlineTranslator::Language> &languages);

    int checkedButton(LanguageButtonsType type) const;
    void setCheckedButton(LanguageButtonsType type, int id);

    // Main window settings
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);

    bool isAutoTranslateEnabled() const;
    void setAutoTranslateEnabled(bool enable);

    QOnlineTranslator::Engine currentEngine() const;
    void setCurrentEngine(QOnlineTranslator::Engine currentEngine);

private:
    static QTranslator s_appTranslator;
#ifdef Q_OS_WIN
    static QTranslator s_qtTranslator; // Qt library translations
#endif
#ifdef WITH_PORTABLE_MODE
    static const QString s_portableConfigName;
#endif

    QSettings *m_settings;
};

#endif // APPSETTINGS_H
