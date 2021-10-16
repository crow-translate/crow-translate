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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "appsettings.h"

#include "cmake.h"
#include "languagebuttonswidget.h"
#include "trayicon.h"

#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QGuiApplication>
#include <QHotkey>
#include <QKeySequence>
#include <QLibraryInfo>
#include <QMetaEnum>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QTranslator>

QTranslator AppSettings::s_appTranslator;
QTranslator AppSettings::s_qtTranslator;
#ifdef WITH_PORTABLE_MODE
const QString AppSettings::s_portableConfigName = QStringLiteral("settings.ini");
#endif

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
#ifndef WITH_PORTABLE_MODE
    , m_settings(new QSettings(this))
#else
    , m_settings(QFile::exists(s_portableConfigName) ? new QSettings(s_portableConfigName, QSettings::IniFormat, this) : new QSettings(this))
#endif
{
}

void AppSettings::setupLocalization() const
{
    applyLocale(locale());
    QCoreApplication::installTranslator(&s_appTranslator);
    QCoreApplication::installTranslator(&s_qtTranslator);
}

QLocale AppSettings::locale() const
{
    return m_settings->value(QStringLiteral("Locale"), defaultLocale()).value<QLocale>();
}

void AppSettings::setLocale(const QLocale &locale)
{
    if (locale != this->locale()) {
        m_settings->setValue(QStringLiteral("Locale"), locale);
        applyLocale(locale);
    }
}

void AppSettings::applyLocale(const QLocale &locale)
{
    const QLocale newLocale = locale == defaultLocale() ? QLocale::system() : locale;
    QLocale::setDefault(newLocale);
    s_appTranslator.load(newLocale, QStringLiteral(PROJECT_NAME), QStringLiteral("_"), QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("translations"), QStandardPaths::LocateDirectory));
    s_qtTranslator.load(newLocale, QStringLiteral("qtbase"), QStringLiteral("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
}

QLocale AppSettings::defaultLocale()
{
    return QLocale::c(); // C locale is used as the system language on apply
}

Qt::ScreenOrientation AppSettings::mainWindowOrientation() const
{
    return m_settings->value(QStringLiteral("MainWindowOrientation"), defaultMainWindowOrientation()).value<Qt::ScreenOrientation>();
}

void AppSettings::setMainWindowOrientation(Qt::ScreenOrientation mode)
{
    m_settings->setValue(QStringLiteral("MainWindowOrientation"), mode);
}

Qt::ScreenOrientation AppSettings::defaultMainWindowOrientation()
{
    return Qt::PrimaryOrientation;
}

AppSettings::WindowMode AppSettings::windowMode() const
{
    return m_settings->value(QStringLiteral("WindowMode"), defaultWindowMode()).value<WindowMode>();
}

void AppSettings::setWindowMode(WindowMode mode)
{
    m_settings->setValue(QStringLiteral("WindowMode"), mode);
}

AppSettings::WindowMode AppSettings::defaultWindowMode()
{
    return PopupWindow;
}

int AppSettings::translationNotificationTimeout() const
{
    return m_settings->value(QStringLiteral("Interface/TranslationNotificationTimeout"), defaultTranslationNotificationTimeout()).toInt();
}

void AppSettings::setTranslationNotificationTimeout(int timeout)
{
    m_settings->setValue(QStringLiteral("Interface/TranslationNotificationTimeout"), timeout);
}

int AppSettings::defaultTranslationNotificationTimeout()
{
    return 3;
}

int AppSettings::popupWindowTimeout() const
{
    return m_settings->value(QStringLiteral("Interface/PopupWindowTimeout"), defaultPopupWindowTimeout()).toInt();
}

void AppSettings::setPopupWindowTimeout(int timeout)
{
    m_settings->setValue(QStringLiteral("Interface/PopupWindowTimeout"), timeout);
}

int AppSettings::defaultPopupWindowTimeout()
{
    return 0;
}

bool AppSettings::isShowTrayIcon() const
{
    return m_settings->value(QStringLiteral("TrayIconVisible"), defaultShowTrayIcon()).toBool();
}

void AppSettings::setShowTrayIcon(bool visible)
{
    m_settings->setValue(QStringLiteral("TrayIconVisible"), visible);
}

bool AppSettings::defaultShowTrayIcon()
{
#ifdef Q_OS_LINUX
    // Plasma Mobile currently says that system tray is available by mistake https://invent.kde.org/plasma/plasma-nano/-/issues/1
    if (const QByteArray plasmaPlatform = qgetenv("PLASMA_PLATFORM"); plasmaPlatform.contains("phone"))
        return false;
#endif
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool AppSettings::isStartMinimized() const
{
    return m_settings->value(QStringLiteral("StartMinimized"), defaultStartMinimized()).toBool();
}

void AppSettings::setStartMinimized(bool minimized)
{
    m_settings->setValue(QStringLiteral("StartMinimized"), minimized);
}

bool AppSettings::defaultStartMinimized()
{
    return true;
}

bool AppSettings::isAutostartEnabled()
{
#if defined(Q_OS_LINUX)
    return QFileInfo::exists(QStringLiteral("%1/autostart/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), QGuiApplication::desktopFileName()));
#elif defined(Q_OS_WIN)
    QSettings autostartSettings(QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat);
    return autostartSettings.contains(QStringLiteral("Crow Translate"));
#endif
}

void AppSettings::setAutostartEnabled(bool enabled)
{
#if defined(Q_OS_LINUX)
    QDir autostartDir(QStringLiteral("%1/autostart").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)));

    if (enabled) {
        // Create autorun file
        if (autostartDir.exists(QGuiApplication::desktopFileName()))
            return;

        if (!autostartDir.exists()) {
            if (!autostartDir.mkpath(QStringLiteral("."))) {
                qCritical() << tr("Unable to create %1").arg(autostartDir.path());
                return;
            }
        }

        const QString desktopFileName = QStringLiteral("/usr/share/applications/%1").arg(QGuiApplication::desktopFileName());
        if (!QFile::copy(desktopFileName, autostartDir.filePath(QGuiApplication::desktopFileName())))
            qCritical() << tr("Unable to copy %1 to %2").arg(desktopFileName, autostartDir.path());
    } else if (autostartDir.exists(QGuiApplication::desktopFileName())) {
        // Remove autorun file
        if (!autostartDir.remove(QGuiApplication::desktopFileName()))
            qCritical() << tr("Unable to remove %1 from %2").arg(QGuiApplication::desktopFileName(), autostartDir.path());
    }
#elif defined(Q_OS_WIN)
    QSettings autostartSettings(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);
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

#ifdef WITH_PORTABLE_MODE
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
    return m_settings->value(QStringLiteral("CheckForUpdatesInterval"), defaultCheckForUpdatesInterval()).value<Interval>();
}

void AppSettings::setCheckForUpdatesInterval(AppSettings::Interval interval)
{
    m_settings->setValue(QStringLiteral("CheckForUpdatesInterval"), interval);
}

AppSettings::Interval AppSettings::defaultCheckForUpdatesInterval()
{
    return Month;
}

QDate AppSettings::lastUpdateCheckDate() const
{
    return m_settings->value(QStringLiteral("LastUpdateCheckDate"), QDate::currentDate()).toDate();
}

void AppSettings::setLastUpdateCheckDate(const QDate &date)
{
    m_settings->setValue(QStringLiteral("LastUpdateCheckDate"), date);
}
#endif

QFont AppSettings::font() const
{
    return m_settings->value(QStringLiteral("Interface/Font"), QGuiApplication::font()).value<QFont>();
}

void AppSettings::setFont(const QFont &font)
{
    m_settings->setValue(QStringLiteral("Interface/Font"), font);
}

double AppSettings::popupOpacity() const
{
    return m_settings->value(QStringLiteral("Interface/PopupOpacity"), defaultPopupOpacity()).toDouble();
}

void AppSettings::setPopupOpacity(double opacity)
{
    m_settings->setValue(QStringLiteral("Interface/PopupOpacity"), opacity);
}

double AppSettings::defaultPopupOpacity()
{
    return 0.8;
}

int AppSettings::popupHeight() const
{
    return m_settings->value(QStringLiteral("Interface/PopupHeight"), defaultPopupHeight()).toInt();
}

void AppSettings::setPopupHeight(int height)
{
    m_settings->setValue(QStringLiteral("Interface/PopupHeight"), height);
}

int AppSettings::defaultPopupHeight()
{
    return 300;
}

int AppSettings::popupWidth() const
{
    return m_settings->value(QStringLiteral("Interface/PopupWidth"), defaultPopupWidth()).toInt();
}

void AppSettings::setPopupWidth(int width)
{
    m_settings->setValue(QStringLiteral("Interface/PopupWidth"), width);
}

int AppSettings::defaultPopupWidth()
{
    return 350;
}

AppSettings::LanguageFormat AppSettings::popupLanguageFormat() const
{
    return m_settings->value(QStringLiteral("Interface/PopupLanguageFormat"), defaultPopupLanguageFormat()).value<LanguageFormat>();
}

void AppSettings::setPopupLanguageFormat(LanguageFormat style)
{
    m_settings->setValue(QStringLiteral("Interface/PopupLanguageFormat"), style);
}

AppSettings::LanguageFormat AppSettings::defaultPopupLanguageFormat()
{
    return IsoCode;
}

AppSettings::LanguageFormat AppSettings::mainWindowLanguageFormat() const
{
    return m_settings->value(QStringLiteral("Interface/MainWindowLanguageFormat"), defaultMainWindowLanguageFormat()).value<LanguageFormat>();
}

void AppSettings::setMainWindowLanguageFormat(LanguageFormat style)
{
    m_settings->setValue(QStringLiteral("Interface/MainWindowLanguageFormat"), style);
}

AppSettings::LanguageFormat AppSettings::defaultMainWindowLanguageFormat()
{
    return FullName;
}

AppSettings::IconType AppSettings::trayIconType() const
{
    return m_settings->value(QStringLiteral("Interface/TrayIconName"), defaultTrayIconType()).value<IconType>();
}

void AppSettings::setTrayIconType(IconType type)
{
    m_settings->setValue(QStringLiteral("Interface/TrayIconName"), type);
}

AppSettings::IconType AppSettings::defaultTrayIconType()
{
    return DefaultIcon;
}

QString AppSettings::customIconPath() const
{
    return m_settings->value(QStringLiteral("Interface/CustomIconPath"), defaultCustomIconPath()).toString();
}

void AppSettings::setCustomIconPath(const QString &path)
{
    m_settings->setValue(QStringLiteral("Interface/CustomIconPath"), path);
}

QString AppSettings::defaultCustomIconPath()
{
    return TrayIcon::trayIconName(AppSettings::DefaultIcon);
}

bool AppSettings::isSourceTranslitEnabled() const
{
    return m_settings->value(QStringLiteral("Translation/SourceTranslitEnabled"), defaultSourceTranslitEnabled()).toBool();
}

void AppSettings::setSourceTranslitEnabled(bool enable)
{
    m_settings->setValue(QStringLiteral("Translation/SourceTranslitEnabled"), enable);
}

bool AppSettings::defaultSourceTranslitEnabled()
{
    return false;
}

bool AppSettings::isTranslationTranslitEnabled() const
{
    return m_settings->value(QStringLiteral("Translation/TranslationTranslitEnabled"), defaultTranslationTranslitEnabled()).toBool();
}

void AppSettings::setTranslationTranslitEnabled(bool enable)
{
    m_settings->setValue(QStringLiteral("Translation/TranslationTranslitEnabled"), enable);
}

bool AppSettings::defaultTranslationTranslitEnabled()
{
    return false;
}

bool AppSettings::isSourceTranscriptionEnabled() const
{
    return m_settings->value(QStringLiteral("Translation/SourceTranscriptionEnabled"), defaultSourceTranscriptionEnabled()).toBool();
}

void AppSettings::setSourceTranscriptionEnabled(bool enable)
{
    m_settings->setValue(QStringLiteral("Translation/SourceTranscriptionEnabled"), enable);
}

bool AppSettings::defaultSourceTranscriptionEnabled()
{
    return true;
}

bool AppSettings::isTranslationOptionsEnabled() const
{
    return m_settings->value(QStringLiteral("Translation/TranslationOptionsEnabled"), defaultTranslationOptionsEnabled()).toBool();
}

void AppSettings::setTranslationOptionsEnabled(bool enable)
{
    m_settings->setValue(QStringLiteral("Translation/TranslationOptionsEnabled"), enable);
}

bool AppSettings::defaultTranslationOptionsEnabled()
{
    return true;
}

bool AppSettings::isExamplesEnabled() const
{
    return m_settings->value(QStringLiteral("Translation/ExamplesEnabled"), defaultExamplesEnabled()).toBool();
}

void AppSettings::setExamplesEnabled(bool enable)
{
    m_settings->setValue(QStringLiteral("Translation/ExamplesEnabled"), enable);
}

bool AppSettings::defaultExamplesEnabled()
{
    return true;
}

bool AppSettings::isSimplifySource() const
{
    return m_settings->value(QStringLiteral("Translation/SimplifySource"), defaultSimplifySource()).toBool();
}

void AppSettings::setSimplifySource(bool simplify)
{
    m_settings->setValue(QStringLiteral("Translation/SimplifySource"), simplify);
}

bool AppSettings::defaultSimplifySource()
{
    return false;
}

QOnlineTranslator::Language AppSettings::primaryLanguage() const
{
    return m_settings->value(QStringLiteral("Translation/PrimaryLanguage"), defaultPrimaryLanguage()).value<QOnlineTranslator::Language>();
}

void AppSettings::setPrimaryLanguage(QOnlineTranslator::Language lang)
{
    m_settings->setValue(QStringLiteral("Translation/PrimaryLanguage"), lang);
}

QOnlineTranslator::Language AppSettings::defaultPrimaryLanguage()
{
    return QOnlineTranslator::Auto;
}

QOnlineTranslator::Language AppSettings::secondaryLanguage() const
{
    return m_settings->value(QStringLiteral("Translation/SecondaryLanguage"), defaultSecondaryLanguage()).value<QOnlineTranslator::Language>();
}

void AppSettings::setSecondaryLanguage(QOnlineTranslator::Language lang)
{
    m_settings->setValue(QStringLiteral("Translation/SecondaryLanguage"), lang);
}

QOnlineTranslator::Language AppSettings::defaultSecondaryLanguage()
{
    return QOnlineTranslator::English;
}

bool AppSettings::isForceSourceAutodetect() const
{
    return m_settings->value(QStringLiteral("Translation/ForceSourceAutodetect"), defaultForceSourceAutodetect()).toBool();
}

void AppSettings::setForceSourceAutodetect(bool force)
{
    m_settings->setValue(QStringLiteral("Translation/ForceSourceAutodetect"), force);
}

bool AppSettings::defaultForceSourceAutodetect()
{
    return true;
}

bool AppSettings::isForceTranslationAutodetect() const
{
    return m_settings->value(QStringLiteral("Translation/ForceTranslationAutodetect"), defaultForceTranslationAutodetect()).toBool();
}

void AppSettings::setForceTranslationAutodetect(bool force)
{
    m_settings->setValue(QStringLiteral("Translation/ForceTranslationAutodetect"), force);
}

bool AppSettings::defaultForceTranslationAutodetect()
{
    return true;
}

QString AppSettings::engineUrl(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::LibreTranslate:
        return m_settings->value(QStringLiteral("Translation/LibreTranslateUrl"), defaultEngineUrl(engine)).toString();
    case QOnlineTranslator::Lingva:
        return m_settings->value(QStringLiteral("Translation/LingvaUrl"), defaultEngineUrl(engine)).toString();
    default:
        Q_UNREACHABLE();
    }
}

void AppSettings::setEngineUrl(QOnlineTranslator::Engine engine, const QString &url)
{
    switch (engine) {
    case QOnlineTranslator::LibreTranslate:
        m_settings->setValue(QStringLiteral("Translation/LibreTranslateUrl"), url);
        break;
    case QOnlineTranslator::Lingva:
        m_settings->setValue(QStringLiteral("Translation/LingvaUrl"), url);
        break;
    default:
        Q_UNREACHABLE();
    }
}

QString AppSettings::defaultEngineUrl(QOnlineTranslator::Engine engine)
{
    switch (engine) {
    case QOnlineTranslator::LibreTranslate:
        return QStringLiteral("https://translate.argosopentech.com");
    case QOnlineTranslator::Lingva:
        return QStringLiteral("https://lingva.ml");
    default:
        Q_UNREACHABLE();
    }
}

QByteArray AppSettings::engineApiKey(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::LibreTranslate:
        return m_settings->value(QStringLiteral("Translation/LibreTranslateApiKey"), defaultEngineApiKey(engine)).toByteArray();
    default:
        Q_UNREACHABLE();
    }
}

void AppSettings::setEngineApiKey(QOnlineTranslator::Engine engine, const QByteArray &apiKey)
{
    switch (engine) {
    case QOnlineTranslator::LibreTranslate:
        m_settings->setValue(QStringLiteral("Translation/LibreTranslateApiKey"), apiKey);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

QByteArray AppSettings::defaultEngineApiKey(QOnlineTranslator::Engine engine)
{
    switch (engine) {
    case QOnlineTranslator::LibreTranslate:
        return {};
    default:
        Q_UNREACHABLE();
    }
}

QOnlineTts::Voice AppSettings::voice(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::LibreTranslate:
    case QOnlineTranslator::Lingva:
        return QOnlineTts::NoVoice;
    case QOnlineTranslator::Yandex:
        return m_settings->value(QStringLiteral("Translation/YandexVoice"), defaultVoice(engine)).value<QOnlineTts::Voice>();
    default:
        Q_UNREACHABLE();
    }
}

void AppSettings::setVoice(QOnlineTranslator::Engine engine, QOnlineTts::Voice voice)
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        m_settings->setValue(QStringLiteral("Translation/YandexVoice"), voice);
        return;
    default:
        // Currently only Yandex have voice settings
        Q_UNREACHABLE();
    }
}

QOnlineTts::Voice AppSettings::defaultVoice(QOnlineTranslator::Engine engine)
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::LibreTranslate:
    case QOnlineTranslator::Lingva:
        return QOnlineTts::NoVoice;
    case QOnlineTranslator::Yandex:
        return QOnlineTts::Zahar;
    default:
        Q_UNREACHABLE();
    }
}

QOnlineTts::Emotion AppSettings::emotion(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::LibreTranslate:
    case QOnlineTranslator::Lingva:
        return QOnlineTts::NoEmotion;
    case QOnlineTranslator::Yandex:
        return m_settings->value(QStringLiteral("Translation/YandexEmotion"), defaultEmotion(engine)).value<QOnlineTts::Emotion>();
    default:
        Q_UNREACHABLE();
    }
}

void AppSettings::setEmotion(QOnlineTranslator::Engine engine, QOnlineTts::Emotion emotion)
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        m_settings->setValue(QStringLiteral("Translation/YandexEmotion"), emotion);
        return;
    default:
        // Currently only Yandex have emotion settings
        Q_UNREACHABLE();
    }
}

QOnlineTts::Emotion AppSettings::defaultEmotion(QOnlineTranslator::Engine engine)
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
    case QOnlineTranslator::LibreTranslate:
    case QOnlineTranslator::Lingva:
        return QOnlineTts::NoEmotion;
    case QOnlineTranslator::Yandex:
        return QOnlineTts::Neutral;
    default:
        Q_UNREACHABLE();
    }
}

QNetworkProxy::ProxyType AppSettings::proxyType() const
{
    return static_cast<QNetworkProxy::ProxyType>(m_settings->value(QStringLiteral("Connection/ProxyType"), defaultProxyType()).toInt());
}

void AppSettings::setProxyType(QNetworkProxy::ProxyType type)
{
    m_settings->setValue(QStringLiteral("Connection/ProxyType"), type);
}

QNetworkProxy::ProxyType AppSettings::defaultProxyType()
{
    return QNetworkProxy::DefaultProxy;
}

QString AppSettings::proxyHost() const
{
    return m_settings->value(QStringLiteral("Connection/ProxyHost"), defaultProxyHost()).toString();
}

void AppSettings::setProxyHost(const QString &hostName)
{
    m_settings->setValue(QStringLiteral("Connection/ProxyHost"), hostName);
}

QString AppSettings::defaultProxyHost()
{
    return {};
}

quint16 AppSettings::proxyPort() const
{
    return m_settings->value(QStringLiteral("Connection/ProxyPort"), defaultProxyPort()).value<quint16>();
}

void AppSettings::setProxyPort(quint16 port)
{
    m_settings->setValue(QStringLiteral("Connection/ProxyPort"), port);
}

quint16 AppSettings::defaultProxyPort()
{
    return 8080;
}

bool AppSettings::isProxyAuthEnabled() const
{
    return m_settings->value(QStringLiteral("Connection/ProxyAuthEnabled"), defaultProxyAuthEnabled()).toBool();
}

void AppSettings::setProxyAuthEnabled(bool enabled)
{
    m_settings->setValue(QStringLiteral("Connection/ProxyAuthEnabled"), enabled);
}

bool AppSettings::defaultProxyAuthEnabled()
{
    return false;
}

QString AppSettings::proxyUsername() const
{
    return m_settings->value(QStringLiteral("Connection/ProxyUsername"), defaultProxyUsername()).toString();
}

void AppSettings::setProxyUsername(const QString &username)
{
    m_settings->setValue(QStringLiteral("Connection/ProxyUsername"), username);
}

QString AppSettings::defaultProxyUsername()
{
    return {};
}

QString AppSettings::proxyPassword() const
{
    return m_settings->value(QStringLiteral("Connection/ProxyPassword"), defaultProxyPassword()).toString();
}

void AppSettings::setProxyPassword(const QString &password)
{
    m_settings->setValue(QStringLiteral("Connection/ProxyPassword"), password);
}

QString AppSettings::defaultProxyPassword()
{
    return {};
}

bool AppSettings::isGlobalShortuctsEnabled() const
{
    return m_settings->value(QStringLiteral("Shortcuts/GlobalShortcutsEnabled"), defaultGlobalShortcutsEnabled()).toBool();
}

void AppSettings::setGlobalShortcutsEnabled(bool enabled)
{
    m_settings->setValue(QStringLiteral("Shortcuts/GlobalShortcutsEnabled"), enabled);
}

bool AppSettings::defaultGlobalShortcutsEnabled()
{
    return QHotkey::isPlatformSupported();
}

QKeySequence AppSettings::translateSelectionShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/TranslateSelection"), defaultTranslateSelectionShortcut()).value<QKeySequence>();
}

void AppSettings::setTranslateSelectionShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/TranslateSelection"), shortcut);
}

QKeySequence AppSettings::defaultTranslateSelectionShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+E"));
}

QKeySequence AppSettings::speakSelectionShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/SpeakSelection"), defaultSpeakSelectionShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakSelectionShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/SpeakSelection"), shortcut);
}

QKeySequence AppSettings::defaultSpeakSelectionShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+S"));
}

QKeySequence AppSettings::speakTranslatedSelectionShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/SpeakTranslatedSelection"), defaultSpeakTranslatedSelectionShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakTranslatedSelectionShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/SpeakTranslatedSelection"), shortcut);
}

QKeySequence AppSettings::defaultSpeakTranslatedSelectionShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+F"));
}

QKeySequence AppSettings::stopSpeakingShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/StopSelection"), defaultStopSpeakingShortcut()).value<QKeySequence>();
}

void AppSettings::setStopSpeakingShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/StopSelection"), shortcut);
}

QKeySequence AppSettings::defaultStopSpeakingShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+G"));
}

QKeySequence AppSettings::playPauseSpeakingShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/PlayPauseSpeakingSelection"), defaultStopSpeakingShortcut()).value<QKeySequence>();
}

void AppSettings::setPlayPauseSpeakingShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/PlayPauseSpeakingSelection"), shortcut);
}

QKeySequence AppSettings::defaultPlayPauseSpeakingShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+D"));
}

QKeySequence AppSettings::showMainWindowShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/ShowMainWindow"), defaultShowMainWindowShortcut()).value<QKeySequence>();
}

void AppSettings::setShowMainWindowShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/ShowMainWindow"), shortcut);
}

QKeySequence AppSettings::defaultShowMainWindowShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+C"));
}

QKeySequence AppSettings::copyTranslatedSelectionShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/CopyTranslatedSelection"), defaultCopyTranslatedSelectionShortcut()).toString();
}

void AppSettings::setCopyTranslatedSelectionShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/CopyTranslatedSelection"), shortcut);
}

QKeySequence AppSettings::defaultCopyTranslatedSelectionShortcut()
{
    return {};
}

QKeySequence AppSettings::recognizeScreenAreaShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/RecognizeScreenAreaShortcut"), defaultRecognizeScreenAreaShortcut()).toString();
}

void AppSettings::setRecognizeScreenAreaShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/RecognizeScreenAreaShortcut"), shortcut);
}

QKeySequence AppSettings::defaultRecognizeScreenAreaShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+I"));
}

QKeySequence AppSettings::translateScreenAreaShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/TranslateScreenAreaShortcut"), defaultTranslateScreenAreaShortcut()).toString();
}

void AppSettings::setTranslateScreenAreaShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/TranslateScreenAreaShortcut"), shortcut);
}

QKeySequence AppSettings::defaultTranslateScreenAreaShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Alt+O"));
}

QKeySequence AppSettings::delayedRecognizeScreenAreaShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/DelayedRecognizeScreenAreaShortcut"), defaultDelayedRecognizeScreenAreaShortcut()).toString();
}

void AppSettings::setDelayedRecognizeScreenAreaShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/DelayedRecognizeScreenAreaShortcut"), shortcut);
}

QKeySequence AppSettings::defaultDelayedRecognizeScreenAreaShortcut()
{
    return {};
}

QKeySequence AppSettings::delayedTranslateScreenAreaShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/DelayedTranslateScreenAreaShortcut"), defaultDelayedTranslateScreenAreaShortcut()).toString();
}

void AppSettings::setDelayedTranslateScreenAreaShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/DelayedTranslateScreenAreaShortcut"), shortcut);
}

QKeySequence AppSettings::defaultDelayedTranslateScreenAreaShortcut()
{
    return {};
}

QKeySequence AppSettings::translateShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/Translate"), defaultTranslateShortcut()).value<QKeySequence>();
}

void AppSettings::setTranslateShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/Translate"), shortcut);
}

QKeySequence AppSettings::defaultTranslateShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Return"));
}

QKeySequence AppSettings::swapShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/Swap"), defaultSwapShortcut()).value<QKeySequence>();
}

void AppSettings::setSwapShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/Swap"), shortcut);
}

QKeySequence AppSettings::defaultSwapShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+R"));
}

QKeySequence AppSettings::closeWindowShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/CloseWindow"), defaultCloseWindowShortcut()).value<QKeySequence>();
}

void AppSettings::setCloseWindowShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/CloseWindow"), shortcut);
}

QKeySequence AppSettings::defaultCloseWindowShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Q"));
}

QKeySequence AppSettings::speakSourceShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/SpeakSource"), defaultSpeakSourceShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakSourceShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/SpeakSource"), shortcut);
}

QKeySequence AppSettings::defaultSpeakSourceShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+S"));
}

QKeySequence AppSettings::speakTranslationShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/SpeakTranslation"), defaultSpeakTranslationShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakTranslationShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/SpeakTranslation"), shortcut);
}

QKeySequence AppSettings::defaultSpeakTranslationShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Shift+S"));
}

QKeySequence AppSettings::copyTranslationShortcut() const
{
    return m_settings->value(QStringLiteral("Shortcuts/CopyTranslation"), defaultCopyTranslationShortcut()).value<QKeySequence>();
}

void AppSettings::setCopyTranslationShortcut(const QKeySequence &shortcut)
{
    m_settings->setValue(QStringLiteral("Shortcuts/CopyTranslation"), shortcut);
}

QKeySequence AppSettings::defaultCopyTranslationShortcut()
{
    return QKeySequence(QStringLiteral("Ctrl+Shift+C"));
}

bool AppSettings::isConvertLineBreaks() const
{
    return m_settings->value(QStringLiteral("OCR/ConvertLineBreaks"), defaultConvertLineBreaks()).toBool();
}

void AppSettings::setConvertLineBreaks(bool convert)
{
    m_settings->setValue(QStringLiteral("OCR/ConvertLineBreaks"), convert);
}

bool AppSettings::defaultConvertLineBreaks()
{
    return true;
}

QByteArray AppSettings::ocrLanguagesPath() const
{
    return m_settings->value(QStringLiteral("OCR/LanguagesPath"), defaultOcrLanguagesPath()).toByteArray();
}

void AppSettings::setOcrLanguagesPath(const QByteArray &path)
{
    m_settings->setValue(QStringLiteral("OCR/LanguagesPath"), path);
}

QByteArray AppSettings::defaultOcrLanguagesPath()
{
    return {};
}

QByteArray AppSettings::ocrLanguagesString() const
{
    return m_settings->value(QStringLiteral("OCR/Languages"), defaultOcrLanguagesString()).toByteArray();
}

void AppSettings::setOcrLanguagesString(const QByteArray &string)
{
    m_settings->setValue(QStringLiteral("OCR/Languages"), string);
}

QByteArray AppSettings::defaultOcrLanguagesString()
{
    return {};
}

bool AppSettings::isShowMagnifier() const
{
    return m_settings->value(QStringLiteral("OCR/ShowMagnifier"), defaultShowMagnifier()).toBool();
}

void AppSettings::setShowMagnifier(bool show)
{
    m_settings->setValue(QStringLiteral("OCR/ShowMagnifier"), show);
}

bool AppSettings::defaultShowMagnifier()
{
    return false;
}

QMap<QString, QVariant> AppSettings::tesseractParameters() const
{
    QMap<QString, QVariant> parameters;
    m_settings->beginGroup("Tesseract");
    for (const QString &key : m_settings->childKeys())
        parameters.insert(key, m_settings->value(key));
    m_settings->endGroup();
    return parameters;
}

void AppSettings::setTesseractParameters(const QMap<QString, QVariant> &parameters)
{
    m_settings->beginGroup("Tesseract");
    m_settings->remove({}); // Remove all keys in current group
    for (auto it = parameters.cbegin(); it != parameters.cend(); ++it)
        m_settings->setValue(it.key(), it.value());
    m_settings->endGroup();
}

QMap<QString, QVariant> AppSettings::defaultTesseractParameters()
{
    return {};
}

AppSettings::RegionRememberType AppSettings::regionRememberType() const
{
    return m_settings->value(QStringLiteral("OCR/RegionRememberType"), defaultRegionRememberType()).value<RegionRememberType>();
}

void AppSettings::setRegionRememberType(RegionRememberType type)
{
    m_settings->setValue(QStringLiteral("OCR/RegionRememberType"), type);
    if (type != RememberAlways)
        m_settings->remove(QStringLiteral("OCR/RememberedCropRegion"));
}

AppSettings::RegionRememberType AppSettings::defaultRegionRememberType()
{
    return NeverRemember;
}

int AppSettings::captureDelay() const
{
    return m_settings->value(QStringLiteral("OCR/CaptureDelay"), defaultCaptureDelay()).toInt();
}

void AppSettings::setCaptureDelay(int ms)
{
    m_settings->setValue(QStringLiteral("OCR/CaptureDelay"), ms);
}

int AppSettings::defaultCaptureDelay()
{
    return 1000;
}

bool AppSettings::isConfirmOnRelease() const
{
    return m_settings->value(QStringLiteral("OCR/ConfirmOnRelease"), defaultConfirmOnRelease()).toBool();
}

void AppSettings::setConfirmOnRelease(bool capture)
{
    m_settings->setValue(QStringLiteral("OCR/ConfirmOnRelease"), capture);
}

bool AppSettings::defaultConfirmOnRelease()
{
    return false;
}

bool AppSettings::isApplyLightMask() const
{
    return m_settings->value(QStringLiteral("OCR/ApplyLightMask"), defaultApplyLightMask()).toBool();
}

void AppSettings::setApplyLightMask(bool use)
{
    m_settings->setValue(QStringLiteral("OCR/ApplyLightMask"), use);
}

bool AppSettings::defaultApplyLightMask()
{
    return true;
}

QRect AppSettings::cropRegion() const
{
    return m_settings->value(QStringLiteral("OCR/CropRegion"), defaultRegionRememberType()).toRect();
}

void AppSettings::setCropRegion(QRect rect)
{
    m_settings->setValue(QStringLiteral("OCR/CropRegion"), rect);
}

QVector<QOnlineTranslator::Language> AppSettings::languages(LanguageButtonsType type) const
{
    const auto typeEnum = QMetaEnum::fromType<LanguageButtonsType>();
    const QStringList languageCodes = m_settings->value(QStringLiteral("Buttons/%1").arg(typeEnum.valueToKey(type))).toStringList();

    QVector<QOnlineTranslator::Language> languages;
    languages.reserve(languageCodes.size());
    for (const QString &langCode : languageCodes) {
        QOnlineTranslator::Language lang = QOnlineTranslator::language(langCode);
        if (lang != QOnlineTranslator::NoLanguage && lang != QOnlineTranslator::Auto)
            languages.append(lang);
        else
            qWarning() << tr("Unknown language code: %1").arg(langCode);
    }

    return languages;
}

void AppSettings::setLanguages(LanguageButtonsType type, const QVector<QOnlineTranslator::Language> &languages)
{
    QStringList langCodes;
    langCodes.reserve(languages.size());
    for (QOnlineTranslator::Language lang : languages)
        langCodes.append(QOnlineTranslator::languageCode(lang));

    const auto typeEnum = QMetaEnum::fromType<LanguageButtonsType>();
    m_settings->setValue(QStringLiteral("Buttons/%1").arg(typeEnum.valueToKey(type)), langCodes);
}

int AppSettings::checkedButton(LanguageButtonsType type) const
{
    const QMetaEnum typeEnum = QMetaEnum::fromType<LanguageButtonsType>();

    return m_settings->value(QStringLiteral("Buttons/Checked%1").arg(typeEnum.valueToKey(type)), LanguageButtonsWidget::autoButtonId()).toInt();
}

void AppSettings::setCheckedButton(LanguageButtonsType type, int id)
{
    const QMetaEnum typeEnum = QMetaEnum::fromType<LanguageButtonsType>();

    m_settings->setValue(QStringLiteral("Buttons/Checked%1").arg(typeEnum.valueToKey(type)), id);
}

QByteArray AppSettings::mainWindowGeometry() const
{
    return m_settings->value(QStringLiteral("MainWindow/WindowGeometry")).toByteArray();
}

void AppSettings::setMainWindowGeometry(const QByteArray &geometry)
{
    m_settings->setValue(QStringLiteral("MainWindow/WindowGeometry"), geometry);
}

bool AppSettings::isAutoTranslateEnabled() const
{
    return m_settings->value(QStringLiteral("MainWindow/AutoTranslate"), false).toBool();
}

void AppSettings::setAutoTranslateEnabled(bool enable)
{
    m_settings->setValue(QStringLiteral("MainWindow/AutoTranslate"), enable);
}

QOnlineTranslator::Engine AppSettings::currentEngine() const
{
    return m_settings->value(QStringLiteral("MainWindow/CurrentEngine"), QOnlineTranslator::Google).value<QOnlineTranslator::Engine>();
}

void AppSettings::setCurrentEngine(QOnlineTranslator::Engine currentEngine)
{
    m_settings->setValue(QStringLiteral("MainWindow/CurrentEngine"), currentEngine);
}
