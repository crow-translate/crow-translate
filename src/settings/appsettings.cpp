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
        autostartSettings.setValue("Crow Translate", QDir::toNativeSeparators(SingleApplication::applicationFilePath()));
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
    return m_settings->value("Shortcuts/GlobalShortcutsEnabled", defaultGlobalShortcutsEnabled()).toBool();
}

void AppSettings::setGlobalShortcutsEnabled(bool enabled)
{
    m_settings->setValue("Shortcuts/GlobalShortcutsEnabled", enabled);
}

bool AppSettings::defaultGlobalShortcutsEnabled()
{
    return true;
}

QKeySequence AppSettings::translateSelectionShortcut() const
{
    return m_settings->value("Shortcuts/TranslateSelection", defaultTranslateSelectionShortcut()).value<QKeySequence>();
}

void AppSettings::setTranslateSelectionShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/TranslateSelection", shortcut);
}

QKeySequence AppSettings::defaultTranslateSelectionShortcut()
{
    return QKeySequence("Ctrl+Alt+E");
}

QKeySequence AppSettings::speakSelectionShortcut() const
{
    return m_settings->value("Shortcuts/SpeakSelection", defaultSpeakSelectionShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakSelectionShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/SpeakSelection", shortcut);
}

QKeySequence AppSettings::defaultSpeakSelectionShortcut()
{
    return QKeySequence("Ctrl+Alt+S");
}

QKeySequence AppSettings::speakTranslatedSelectionShortcut() const
{
    return m_settings->value("Shortcuts/SpeakTranslatedSelection", defaultSpeakTranslatedSelectionShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakTranslatedSelectionShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/SpeakTranslatedSelection", shortcut);
}

QKeySequence AppSettings::defaultSpeakTranslatedSelectionShortcut()
{
    return QKeySequence("Ctrl+Alt+F");
}

QKeySequence AppSettings::stopSpeakingShortcut() const
{
    return m_settings->value("Shortcuts/StopSelection", defaultStopSpeakingShortcut()).value<QKeySequence>();
}

void AppSettings::setStopSpeakingShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/StopSelection", shortcut);
}

QKeySequence AppSettings::defaultStopSpeakingShortcut()
{
    return QKeySequence("Ctrl+Alt+G");
}

QKeySequence AppSettings::showMainWindowShortcut() const
{
    return m_settings->value("Shortcuts/ShowMainWindow", defaultShowMainWindowShortcut()).value<QKeySequence>();
}

void AppSettings::setShowMainWindowShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/ShowMainWindow", shortcut);
}

QKeySequence AppSettings::defaultShowMainWindowShortcut()
{
    return QKeySequence("Ctrl+Alt+C");
}

QKeySequence AppSettings::copyTranslatedSelectionShortcut() const
{
    return m_settings->value("Shortcuts/CopyTranslatedSelection", defaultCopyTranslatedSelectionShortcut()).toString();
}

void AppSettings::setCopyTranslatedSelectionShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/CopyTranslatedSelection", shortcut);
}

QKeySequence AppSettings::defaultCopyTranslatedSelectionShortcut()
{
    return QKeySequence();
}

QKeySequence AppSettings::translateShortcut() const
{
    return m_settings->value("Shortcuts/Translate", defaultTranslateShortcut()).value<QKeySequence>();
}

void AppSettings::setTranslateShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/Translate", shortcut);
}

QKeySequence AppSettings::defaultTranslateShortcut()
{
    return QKeySequence("Ctrl+Return");
}

QKeySequence AppSettings::closeWindowShortcut() const
{
    return m_settings->value("Shortcuts/CloseWindow", defaultCloseWindowShortcut()).value<QKeySequence>();
}

void AppSettings::setCloseWindowShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/CloseWindow", shortcut);
}

QKeySequence AppSettings::defaultCloseWindowShortcut()
{
    return QKeySequence("Ctrl+Q");
}

QKeySequence AppSettings::speakSourceShortcut() const
{
    return m_settings->value("Shortcuts/SpeakSource", defaultSpeakSourceShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakSourceShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/SpeakSource", shortcut);
}

QKeySequence AppSettings::defaultSpeakSourceShortcut()
{
    return QKeySequence("Ctrl+S");
}

QKeySequence AppSettings::speakTranslationShortcut() const
{
    return m_settings->value("Shortcuts/SpeakTranslation", defaultSpeakTranslationShortcut()).value<QKeySequence>();
}

void AppSettings::setSpeakTranslationShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/SpeakTranslation", shortcut);
}

QKeySequence AppSettings::defaultSpeakTranslationShortcut()
{
    return QKeySequence("Ctrl+Shift+S");
}

QKeySequence AppSettings::copyTranslationShortcut() const
{
    return m_settings->value("Shortcuts/CopyTranslation", defaultCopyTranslationShortcut()).value<QKeySequence>();
}

void AppSettings::setCopyTranslationShortcut(const QKeySequence &shortcut)
{
     m_settings->setValue("Shortcuts/CopyTranslation", shortcut);
}

QKeySequence AppSettings::defaultCopyTranslationShortcut()
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
