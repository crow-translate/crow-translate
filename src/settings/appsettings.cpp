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

#include "appsettings.h"
#include "singleapplication.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QTextStream>
#include <QLibraryInfo>
#include <QMetaEnum>
#include <QKeySequence>
#include <QSettings>
#include <QTranslator>
#ifdef Q_OS_WIN
#include <QDir>
#endif

QTranslator AppSettings::s_appTranslator;
QTranslator AppSettings::s_qtTranslator;
#ifdef Q_OS_LINUX
const QString AppSettings::s_autostartFileName = QStringLiteral("%1/autostart/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), SingleApplication::desktopFileName());
#endif
#ifdef PORTABLE_MODE
const QString AppSettings::s_portableConfigName = QStringLiteral("settings.ini");
#endif

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
#ifndef PORTABLE_MODE
    , m_settings(new QSettings(this))
{
}
#else
{
    m_settings = QFile::exists(s_portableConfigName) ? new QSettings(s_portableConfigName, QSettings::IniFormat, this) : new QSettings(this);
}
#endif

void AppSettings::setupLocalization() const
{
    applyLanguage(language());
    SingleApplication::installTranslator(&s_appTranslator);
    SingleApplication::installTranslator(&s_qtTranslator);
}

QLocale::Language AppSettings::language() const
{
    return m_settings->value("Language", defaultLanguage()).value<QLocale::Language>();
}

void AppSettings::setLanguage(QLocale::Language lang)
{
    if (lang != language()) {
        m_settings->setValue("Locale", lang);
        applyLanguage(lang);
    }
}

void AppSettings::applyLanguage(QLocale::Language lang)
{
    if (lang == QLocale::AnyLanguage)
        QLocale::setDefault(QLocale::system());
    else
        QLocale::setDefault(QLocale(lang));

    s_appTranslator.load(QLocale(), "crow", "_", ":/i18n");
    s_qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
}

QLocale::Language AppSettings::defaultLanguage()
{
    return QLocale::AnyLanguage;
}

AppSettings::WindowMode AppSettings::windowMode() const
{
    return m_settings->value("WindowMode", defaultWindowMode()).value<WindowMode>();
}

void AppSettings::setWindowMode(WindowMode mode)
{
     m_settings->setValue("WindowMode", mode);
}

AppSettings::WindowMode AppSettings::defaultWindowMode()
{
    return PopupWindow;
}

bool AppSettings::isShowTrayIcon() const
{
    return m_settings->value("TrayIconVisible", defaultShowTrayIcon()).toBool();
}

void AppSettings::setShowTrayIcon(bool visible)
{
     m_settings->setValue("TrayIconVisible", visible);
}

bool AppSettings::defaultShowTrayIcon()
{
    return true;
}

bool AppSettings::isStartMinimized() const
{
    return m_settings->value("StartMinimized", defaultStartMinimized()).toBool();
}

void AppSettings::setStartMinimized(bool minimized)
{
     m_settings->setValue("StartMinimized", minimized);
}

bool AppSettings::defaultStartMinimized()
{
    return true;
}

bool AppSettings::isAutostartEnabled() const
{
#if defined(Q_OS_LINUX)
    return QFileInfo::exists(s_autostartFileName);
#elif defined(Q_OS_WIN)
    QSettings autostartSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return autostartSettings.contains("Crow Translate");
#endif
}

void AppSettings::setAutostartEnabled(bool enabled)
{
#if defined(Q_OS_LINUX)
    QFile autorunFile(s_autostartFileName);

    if (enabled) {
        // Create autorun file
        if (!autorunFile.exists()) {
            const QString desktopFileName = QStringLiteral("/usr/share/applications/%1").arg(SingleApplication::desktopFileName());

            if (!QFile::copy(desktopFileName, autorunFile.fileName()))
                qCritical() << tr("Unable to create autorun file from %1").arg(desktopFileName);
        }
    } else {
        // Remove autorun file
        if(autorunFile.exists())
            autorunFile.remove();
    }
#elif defined(Q_OS_WIN)
    QSettings autostartSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (enabled)
        autostartSettings.setValue("Crow Translate", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    else
        autostartSettings.remove("Crow Translate");
#endif
}

bool AppSettings::defaultAutostartEnabled()
{
    return false;
}

#ifdef PORTABLE_MODE
bool AppSettings::isPortableModeEnabled() const
{
    return m_settings->format() == QSettings::IniFormat;
}

void AppSettings::setPortableModeEnabled(bool enabled)
{
    if (enabled) {
        QFile configFile(s_portableConfigName);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        configFile.open(QIODevice::NewOnly);
#else
        if (!configFile.exists())
            configFile.open(QIODevice::WriteOnly);
#endif
    } else {
        QFile::remove(s_portableConfigName);
    }
}

QString AppSettings::portableConfigName()
{
    return s_portableConfigName;
}
#endif

#ifdef Q_OS_WIN
AppSettings::Interval AppSettings::checkForUpdatesInterval() const
{
    return m_settings->value("CheckForUpdatesInterval", defaultCheckForUpdatesInterval()).value<Interval>();
}

void AppSettings::setCheckForUpdatesInterval(AppSettings::Interval interval)
{
     m_settings->setValue("CheckForUpdatesInterval", interval);
}

AppSettings::Interval AppSettings::defaultCheckForUpdatesInterval()
{
    return Month;
}

QDate AppSettings::lastUpdateCheckDate() const
{
    return m_settings->value("LastUpdateCheckDate", QDate::currentDate()).toDate();
}

void AppSettings::setLastUpdateCheckDate(const QDate &date)
{
     m_settings->setValue("LastUpdateCheckDate", date);
}
#endif

double AppSettings::popupOpacity() const
{
    return m_settings->value("PopupOpacity", defaultPopupOpacity()).toDouble();
}

void AppSettings::setPopupOpacity(double opacity)
{
     m_settings->setValue("PopupOpacity", opacity);
}

double AppSettings::defaultPopupOpacity()
{
    return 0.8;
}

int AppSettings::popupHeight() const
{
    return m_settings->value("PopupHeight", defaultPopupHeight()).toInt();
}

void AppSettings::setPopupHeight(int height)
{
     m_settings->setValue("PopupHeight", height);
}

int AppSettings::defaultPopupHeight()
{
    return 300;
}

int AppSettings::popupWidth() const
{
    return m_settings->value("PopupWidth", defaultPopupWidth()).toInt();
}

void AppSettings::setPopupWidth(int width)
{
     m_settings->setValue("PopupWidth", width);
}

int AppSettings::defaultPopupWidth()
{
    return 350;
}

Qt::ToolButtonStyle AppSettings::popupLanguagesStyle() const
{
    return m_settings->value("PopupLanguagesStyle", defaultPopupLanguagesStyle()).value<Qt::ToolButtonStyle>();
}

void AppSettings::setPopupLanguagesStyle(Qt::ToolButtonStyle style)
{
     m_settings->setValue("PopupLanguagesStyle", style);
}

Qt::ToolButtonStyle AppSettings::defaultPopupLanguagesStyle()
{
    return Qt::ToolButtonIconOnly;
}

Qt::ToolButtonStyle AppSettings::popupControlsStyle() const
{
    return m_settings->value("PopupControlsStyle", defaultPopupControlsStyle()).value<Qt::ToolButtonStyle>();
}

void AppSettings::setPopupControlsStyle(Qt::ToolButtonStyle style)
{
     m_settings->setValue("PopupControlsStyle", style);
}

Qt::ToolButtonStyle AppSettings::defaultPopupControlsStyle()
{
    return Qt::ToolButtonIconOnly;
}

Qt::ToolButtonStyle AppSettings::windowLanguagesStyle() const
{
    return m_settings->value("WindowLanguagesStyle", defaultWindowLanguagesStyle()).value<Qt::ToolButtonStyle>();
}

void AppSettings::setWindowLanguagesStyle(Qt::ToolButtonStyle style)
{
     m_settings->setValue("WindowLanguagesStyle", style);
}

Qt::ToolButtonStyle AppSettings::defaultWindowLanguagesStyle()
{
    return Qt::ToolButtonTextBesideIcon;
}

Qt::ToolButtonStyle AppSettings::windowControlsStyle() const
{
    return m_settings->value("WindowControlsStyle", defaultWindowControlsStyle()).value<Qt::ToolButtonStyle>();
}

void AppSettings::setWindowControlsStyle(Qt::ToolButtonStyle style)
{
     m_settings->setValue("WindowControlsStyle", style);
}

Qt::ToolButtonStyle AppSettings::defaultWindowControlsStyle()
{
    return Qt::ToolButtonIconOnly;
}

TrayIcon::IconType AppSettings::trayIconType() const
{
    return m_settings->value("TrayIconName", defaultTrayIconType()).value<TrayIcon::IconType>();
}

void AppSettings::setTrayIconType(TrayIcon::IconType type)
{
     m_settings->setValue("TrayIconName", type);
}

TrayIcon::IconType AppSettings::defaultTrayIconType()
{
    return TrayIcon::DefaultIcon;
}

QString AppSettings::customIconPath() const
{
    return m_settings->value("CustomIconPath", defaultCustomIconPath()).toString();
}

void AppSettings::setCustomIconPath(const QString &path)
{
     m_settings->setValue("CustomIconPath", path);
}

QString AppSettings::defaultCustomIconPath()
{
    return TrayIcon::trayIconName(TrayIcon::DefaultIcon);
}

bool AppSettings::isSourceTranslitEnabled() const
{
    return m_settings->value("Translation/SourceTranslitEnabled", defaultSourceTranslitEnabled()).toBool();
}

void AppSettings::setSourceTranslitEnabled(bool enable)
{
     m_settings->setValue("Translation/SourceTranslitEnabled", enable);
}

bool AppSettings::defaultSourceTranslitEnabled()
{
    return true;
}

bool AppSettings::isTranslationTranslitEnabled() const
{
    return m_settings->value("Translation/TranslationTranslitEnabled", defaultTranslationTranslitEnabled()).toBool();
}

void AppSettings::setTranslationTranslitEnabled(bool enable)
{
     m_settings->setValue("Translation/TranslationTranslitEnabled", enable);
}

bool AppSettings::defaultTranslationTranslitEnabled()
{
    return true;
}

bool AppSettings::isSourceTranscriptionEnabled() const
{
    return m_settings->value("Translation/SourceTranscriptionEnabled", defaultSourceTranscriptionEnabled()).toBool();
}

void AppSettings::setSourceTranscriptionEnabled(bool enable)
{
     m_settings->setValue("Translation/SourceTranscriptionEnabled", enable);
}

bool AppSettings::defaultSourceTranscriptionEnabled()
{
    return true;
}

bool AppSettings::isTranslationOptionsEnabled() const
{
    return m_settings->value("Translation/TranslationOptionsEnabled", defaultTranslationOptionsEnabled()).toBool();
}

void AppSettings::setTranslationOptionsEnabled(bool enable)
{
     m_settings->setValue("Translation/TranslationOptionsEnabled", enable);
}

bool AppSettings::defaultTranslationOptionsEnabled()
{
    return true;
}

bool AppSettings::isExamplesEnabled() const
{
    return m_settings->value("Translation/ExamplesEnabled", defaultExamplesEnabled()).toBool();
}

void AppSettings::setExamplesEnabled(bool enable)
{
     m_settings->setValue("Translation/ExamplesEnabled", enable);
}

bool AppSettings::defaultExamplesEnabled()
{
    return true;
}

QOnlineTranslator::Language AppSettings::primaryLanguage() const
{
    return m_settings->value("Translation/PrimaryLanguage", defaultPrimaryLanguage()).value<QOnlineTranslator::Language>();
}

void AppSettings::setPrimaryLanguage(QOnlineTranslator::Language lang)
{
     m_settings->setValue("Translation/PrimaryLanguage", lang);
}

QOnlineTranslator::Language AppSettings::defaultPrimaryLanguage()
{
    return QOnlineTranslator::Auto;
}

QOnlineTranslator::Language AppSettings::secondaryLanguage() const
{
    return m_settings->value("Translation/SecondaryLanguage", defaultSecondaryLanguage()).value<QOnlineTranslator::Language>();
}

void AppSettings::setSecondaryLanguage(QOnlineTranslator::Language lang)
{
     m_settings->setValue("Translation/SecondaryLanguage", lang);
}

QOnlineTranslator::Language AppSettings::defaultSecondaryLanguage()
{
    return QOnlineTranslator::English;
}

// Selected primary or secondary language depends on sourceLang
QOnlineTranslator::Language AppSettings::preferredTranslationLanguage(QOnlineTranslator::Language sourceLang) const
{
    QOnlineTranslator::Language translationLang = primaryLanguage();
    if (translationLang == QOnlineTranslator::Auto)
        translationLang = QOnlineTranslator::language(QLocale());

    if (translationLang != sourceLang)
        return translationLang;

    return secondaryLanguage();
}

bool AppSettings::isForceSourceAutodetect() const
{
    return m_settings->value("Translation/ForceSourceAutodetect", defaultForceSourceAutodetect()).toBool();
}

void AppSettings::setForceSourceAutodetect(bool force)
{
     m_settings->setValue("Translation/ForceSourceAutodetect", force);
}

bool AppSettings::defaultForceSourceAutodetect()
{
    return true;
}

bool AppSettings::isForceTranslationAutodetect() const
{
    return m_settings->value("Translation/ForceTranslationAutodetect", defaultForceTranslationAutodetect()).toBool();
}

void AppSettings::setForceTranslationAutodetect(bool force)
{
     m_settings->setValue("Translation/ForceTranslationAutodetect", force);
}

bool AppSettings::defaultForceTranslationAutodetect()
{
    return true;
}

QOnlineTts::Voice AppSettings::voice(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
        return QOnlineTts::NoVoice;
    case QOnlineTranslator::Yandex:
        return m_settings->value("Translation/YandexVoice", defaultVoice(engine)).value<QOnlineTts::Voice>();
    }

    qFatal("Unknown engine");
}

void AppSettings::setVoice(QOnlineTranslator::Engine engine, QOnlineTts::Voice voice)
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
        qFatal("Currently only Yandex have voice settings");
    case QOnlineTranslator::Yandex:
         m_settings->setValue("Translation/YandexVoice", voice);
        return;
    }

    qFatal("Unknown engine");
}

QOnlineTts::Voice AppSettings::defaultVoice(QOnlineTranslator::Engine engine)
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
        return QOnlineTts::NoVoice;
    case QOnlineTranslator::Yandex:
        return QOnlineTts::Zahar;
    }

    qFatal("Unknown engine");
}

QOnlineTts::Emotion AppSettings::emotion(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::Google:
        return QOnlineTts::NoEmotion;
    case QOnlineTranslator::Yandex:
        return m_settings->value("Translation/YandexEmotion", defaultEmotion(engine)).value<QOnlineTts::Emotion>();
    }

    qFatal("Unknown engine");
}

void AppSettings::setEmotion(QOnlineTranslator::Engine engine, QOnlineTts::Emotion emotion)
{
    switch (engine) {
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::Google:
        qFatal("Currently only Yandex have emotion settings");
    case QOnlineTranslator::Yandex:
         m_settings->setValue("Translation/YandexEmotion", emotion);
        return;
    }

    qFatal("Unknown engine");
}

QOnlineTts::Emotion AppSettings::defaultEmotion(QOnlineTranslator::Engine engine)
{
    switch (engine) {
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::Google:
        return QOnlineTts::NoEmotion;
    case QOnlineTranslator::Yandex:
        return QOnlineTts::Neutral;
    }

    qFatal("Unknown engine");
}

QNetworkProxy::ProxyType AppSettings::proxyType() const
{
    return static_cast<QNetworkProxy::ProxyType>(m_settings->value("Connection/ProxyType", defaultProxyType()).toInt());
}

void AppSettings::setProxyType(QNetworkProxy::ProxyType type)
{
     m_settings->setValue("Connection/ProxyType", type);
}

QNetworkProxy::ProxyType AppSettings::defaultProxyType()
{
    return QNetworkProxy::DefaultProxy;
}

QString AppSettings::proxyHost() const
{
    return m_settings->value("Connection/ProxyHost", defaultProxyHost()).toString();
}

void AppSettings::setProxyHost(const QString &hostName)
{
     m_settings->setValue("Connection/ProxyHost", hostName);
}

QString AppSettings::defaultProxyHost()
{
    return {};
}

quint16 AppSettings::proxyPort() const
{
    return m_settings->value("Connection/ProxyPort", defaultProxyPort()).value<quint16>();
}

void AppSettings::setProxyPort(quint16 port)
{
     m_settings->setValue("Connection/ProxyPort", port);
}

quint16 AppSettings::defaultProxyPort()
{
    return 8080;
}

bool AppSettings::isProxyAuthEnabled() const
{
    return m_settings->value("Connection/ProxyAuthEnabled", defaultProxyAuthEnabled()).toBool();
}

void AppSettings::setProxyAuthEnabled(bool enabled)
{
     m_settings->setValue("Connection/ProxyAuthEnabled", enabled);
}

bool AppSettings::defaultProxyAuthEnabled()
{
    return false;
}

QString AppSettings::proxyUsername() const
{
    return m_settings->value("Connection/ProxyUsername", defaultProxyUsername()).toString();
}

void AppSettings::setProxyUsername(const QString &username)
{
     m_settings->setValue("Connection/ProxyUsername", username);
}

QString AppSettings::defaultProxyUsername()
{
    return {};
}

QString AppSettings::proxyPassword() const
{
    return m_settings->value("Connection/ProxyPassword", defaultProxyPassword()).toString();
}

void AppSettings::setProxyPassword(const QString &password)
{
     m_settings->setValue("Connection/ProxyPassword", password);
}

QString AppSettings::defaultProxyPassword()
{
    return {};
}

bool AppSettings::isGlobalShortuctsEnabled() const
{
    return m_settings->value("Hotkeys/GlobalShortcutsEnabled", defaultGlobalShortcutsEnabled()).toBool();
}

void AppSettings::setGlobalShortcutsEnabled(bool enabled)
{
    m_settings->setValue("Hotkeys/GlobalShortcutsEnabled", enabled);
}

bool AppSettings::defaultGlobalShortcutsEnabled()
{
    return true;
}

QKeySequence AppSettings::translateSelectionHotkey() const
{
    return m_settings->value("Hotkeys/TranslateSelection", defaultTranslateSelectionHotkey()).value<QKeySequence>();
}

void AppSettings::setTranslateSelectionHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/TranslateSelection", hotkey);
}

QKeySequence AppSettings::defaultTranslateSelectionHotkey()
{
    return QKeySequence("Ctrl+Alt+E");
}

QKeySequence AppSettings::speakSelectionHotkey() const
{
    return m_settings->value("Hotkeys/PlaySelection", defaultSpeakSelectionHotkey()).value<QKeySequence>();
}

void AppSettings::setSpeakSelectionHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/PlaySelection", hotkey);
}

QKeySequence AppSettings::defaultSpeakSelectionHotkey()
{
    return QKeySequence("Ctrl+Alt+S");
}

QKeySequence AppSettings::speakTranslatedSelectionHotkey() const
{
    return m_settings->value("Hotkeys/PlayTranslatedSelection", defaultSpeakTranslatedSelectionHotkey()).value<QKeySequence>();
}

void AppSettings::setSpeakTranslatedSelectionHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/PlayTranslatedSelection", hotkey);
}

QKeySequence AppSettings::defaultSpeakTranslatedSelectionHotkey()
{
    return QKeySequence("Ctrl+Alt+F");
}

QKeySequence AppSettings::stopSpeakingHotkey() const
{
    return m_settings->value("Hotkeys/StopSelection", defaultStopSpeakingHotkey()).value<QKeySequence>();
}

void AppSettings::setStopSpeakingHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/StopSelection", hotkey);
}

QKeySequence AppSettings::defaultStopSpeakingHotkey()
{
    return QKeySequence("Ctrl+Alt+G");
}

QKeySequence AppSettings::showMainWindowHotkey() const
{
    return m_settings->value("Hotkeys/ShowMainWindow", defaultShowMainWindowHotkey()).value<QKeySequence>();
}

void AppSettings::setShowMainWindowHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/ShowMainWindow", hotkey);
}

QKeySequence AppSettings::defaultShowMainWindowHotkey()
{
    return QKeySequence("Ctrl+Alt+C");
}

QKeySequence AppSettings::copyTranslatedSelectionHotkey() const
{
    return m_settings->value("Hotkeys/CopyTranslatedSelection", defaultCopyTranslatedSelectionHotkey()).toString();
}

void AppSettings::setCopyTranslatedSelectionHotkeyHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/CopyTranslatedSelection", hotkey);
}

QKeySequence AppSettings::defaultCopyTranslatedSelectionHotkey()
{
    return QKeySequence();
}

QKeySequence AppSettings::translateHotkey() const
{
    return m_settings->value("Hotkeys/Translate", defaultTranslateHotkey()).value<QKeySequence>();
}

void AppSettings::setTranslateHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/Translate", hotkey);
}

QKeySequence AppSettings::defaultTranslateHotkey()
{
    return QKeySequence("Ctrl+Return");
}

QKeySequence AppSettings::closeWindowHotkey() const
{
    return m_settings->value("Hotkeys/CloseWindow", defaultCloseWindowHotkey()).value<QKeySequence>();
}

void AppSettings::setCloseWindowHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/CloseWindow", hotkey);
}

QKeySequence AppSettings::defaultCloseWindowHotkey()
{
    return QKeySequence("Ctrl+Q");
}

QKeySequence AppSettings::speakSourceHotkey() const
{
    return m_settings->value("Hotkeys/PlaySource", defaultSpeakSourceHotkey()).value<QKeySequence>();
}

void AppSettings::setSpeakSourceHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/PlaySource", hotkey);
}

QKeySequence AppSettings::defaultSpeakSourceHotkey()
{
    return QKeySequence("Ctrl+S");
}

QKeySequence AppSettings::speakTranslationHotkey() const
{
    return m_settings->value("Hotkeys/PlayTranslation", defaultSpeakTranslationHotkey()).value<QKeySequence>();
}

void AppSettings::setSpeakTranslationHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/PlayTranslation", hotkey);
}

QKeySequence AppSettings::defaultSpeakTranslationHotkey()
{
    return QKeySequence("Ctrl+Shift+S");
}

QKeySequence AppSettings::copyTranslationHotkey() const
{
    return m_settings->value("Hotkeys/CopyTranslation", defaultCopyTranslationHotkey()).value<QKeySequence>();
}

void AppSettings::setCopyTranslationHotkey(const QKeySequence &hotkey)
{
     m_settings->setValue("Hotkeys/CopyTranslation", hotkey);
}

QKeySequence AppSettings::defaultCopyTranslationHotkey()
{
    return QKeySequence("Ctrl+Shift+C");
}

QOnlineTranslator::Language AppSettings::buttonLanguage(LangButtonGroup::GroupType group, int id) const
{
    const QMetaEnum groupType = QMetaEnum::fromType<LangButtonGroup::GroupType>();

    return m_settings->value(QStringLiteral("Buttons/") + groupType.key(group) + QStringLiteral("Button") + QString::number(id), QOnlineTranslator::NoLanguage).value<QOnlineTranslator::Language>();
}

void AppSettings::setButtonLanguage(LangButtonGroup::GroupType group, int id, QOnlineTranslator::Language lang)
{
    const QMetaEnum groupType = QMetaEnum::fromType<LangButtonGroup::GroupType>();

     m_settings->setValue(QStringLiteral("Buttons/") + groupType.key(group) + QStringLiteral("Button") + QString::number(id), lang);
}

int AppSettings::checkedButton(LangButtonGroup::GroupType group) const
{
    const QMetaEnum groupType = QMetaEnum::fromType<LangButtonGroup::GroupType>();

    return m_settings->value(QStringLiteral("Buttons/Checked") + groupType.key(group) + QStringLiteral("Button"), 0).toInt();
}

void AppSettings::setCheckedButton(LangButtonGroup::GroupType group, int id)
{
    const QMetaEnum groupType = QMetaEnum::fromType<LangButtonGroup::GroupType>();

     m_settings->setValue(QStringLiteral("Buttons/Checked") + groupType.key(group) + QStringLiteral("Button"), id);
}

QByteArray AppSettings::mainWindowGeometry() const
{
    return m_settings->value("WindowGeometry").toByteArray();
}

void AppSettings::setMainWindowGeometry(const QByteArray &geometry)
{
     m_settings->setValue("WindowGeometry", geometry);
}

bool AppSettings::isAutoTranslateEnabled() const
{
    return m_settings->value("AutoTranslate", true).toBool();
}

void AppSettings::setAutoTranslateEnabled(bool enable)
{
     m_settings->setValue("AutoTranslate", enable);
}

QOnlineTranslator::Engine AppSettings::currentEngine() const
{
    return m_settings->value("CurrentEngine", QOnlineTranslator::Google).value<QOnlineTranslator::Engine>();
}

void AppSettings::setCurrentEngine(QOnlineTranslator::Engine currentEngine)
{
     m_settings->setValue("CurrentEngine", currentEngine);
}
