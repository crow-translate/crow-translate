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

#include "appsettings.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QTextStream>
#include <QLibraryInfo>
#if defined(Q_OS_WIN)
#include <QDir>
#endif

#include "singleapplication.h"

QTranslator AppSettings::m_appTranslator;
#if defined(Q_OS_WIN)
QTranslator AppSettings::m_qtTranslator;
#endif

AppSettings::AppSettings(QObject *parent) :
    QSettings(parent)
{
}

void AppSettings::setupLocale()
{
    loadLocale(locale());
    SingleApplication::installTranslator(&m_appTranslator);
#if defined(Q_OS_WIN)
    SingleApplication::installTranslator(&m_qtTranslator);
#endif
}

QLocale::Language AppSettings::locale()
{
    return value("Locale", QLocale::AnyLanguage).value<QLocale::Language>();
}

void AppSettings::setLocale(QLocale::Language lang)
{
    if (lang != locale()) {
        setValue("Locale", lang);
        loadLocale(lang);
    }
}

void AppSettings::loadLocale(QLocale::Language lang)
{
    if (lang == QLocale::AnyLanguage)
        QLocale::setDefault(QLocale::system());
    else
        QLocale::setDefault(QLocale(lang));

    m_appTranslator.load(QLocale(), "crow", "_", ":/translations");
#if defined(Q_OS_WIN)
    m_qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#endif
}

AppSettings::WindowMode AppSettings::windowMode()
{
    return value("WindowMode", PopupWindow).value<WindowMode>();
}

void AppSettings::setWindowMode(WindowMode mode)
{
    setValue("WindowMode", mode);
}

bool AppSettings::isTrayIconVisible()
{
    return value("TrayIconVisible", true).toBool();
}

void AppSettings::setTrayIconVisible(bool visible)
{
    setValue("TrayIconVisible", visible);
}

bool AppSettings::isStartMinimized()
{
    return value("StartMinimized", true).toBool();
}

void AppSettings::setStartMinimized(bool minimized)
{
    setValue("StartMinimized", minimized);
}

bool AppSettings::isAutostartEnabled()
{
#if defined(Q_OS_LINUX)
    return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/crow-translate.desktop").exists();
#elif defined(Q_OS_WIN)
    QSettings autostartSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return autostartSettings.contains("Crow Translate");
#endif
}

void AppSettings::setAutostartEnabled(bool enabled)
{
#if defined(Q_OS_LINUX)
    QFile autorunFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/crow-translate.desktop");
    if (enabled) {
        // Create autorun file if checked
        if (!autorunFile.exists()) {
            if (autorunFile.open(QFile::WriteOnly)) {
                QString autorunContent("[Desktop Entry]\n"
                                       "Type=Application\n"
                                       "Exec=crow\n"
                                       "Hidden=false\n"
                                       "NoDisplay=false\n"
                                       "Icon=crow-translate\n"
                                       "Name=Crow Translate\n"
                                       "Comment=A simple and lightweight translator that allows to translate and say selected text using the Google Translate API and much more\n"
                                       "Comment[ru]=Простой и легковесный переводчик, который позволяет переводить и озвучивать выделенный текст с помощью Google Translate API, а также многое другое\n");
                QTextStream outStream(&autorunFile);
                outStream << autorunContent;
                autorunFile.close();
            } else {
                qDebug() << tr("Unable to create autorun file");
            }
        }
    } else {
        // Remove autorun file if box unchecked
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

#ifdef Q_OS_WIN
QDate AppSettings::lastUpdateCheckDate()
{
    return value("LastUpdateCheckDate", QDate::currentDate()).toDate();
}

void AppSettings::setLastUpdateCheckDate(const QDate &date)
{
    setValue("LastUpdateCheckDate", date);
}

AppSettings::Interval AppSettings::checkForUpdatesInterval()
{
    return value("CheckForUpdatesInterval", Month).value<Interval>();
}

void AppSettings::setCheckForUpdatesInterval(AppSettings::Interval interval)
{
    setValue("CheckForUpdatesInterval", interval);
}
#endif

double AppSettings::popupOpacity()
{
    return value("PopupOpacity", 0.8).toDouble();
}

void AppSettings::setPopupOpacity(double opacity)
{
    setValue("PopupOpacity", opacity);
}

int AppSettings::popupHeight()
{
    return value("PopupHeight", 300).toInt();
}

void AppSettings::setPopupHeight(int height)
{
    setValue("PopupHeight", height);
}

int AppSettings::popupWidth()
{
    return value("PopupWidth", 350).toInt();
}

void AppSettings::setPopupWidth(int width)
{
    setValue("PopupWidth", width);
}

Qt::ToolButtonStyle AppSettings::popupLanguagesStyle()
{
    return value("PopupLanguagesStyle", Qt::ToolButtonIconOnly).value<Qt::ToolButtonStyle>();
}

void AppSettings::setPopupLanguagesStyle(Qt::ToolButtonStyle style)
{
    setValue("PopupLanguagesStyle", style);
}

Qt::ToolButtonStyle AppSettings::popupControlsStyle()
{
    return value("PopupControlsStyle", Qt::ToolButtonIconOnly).value<Qt::ToolButtonStyle>();
}

void AppSettings::setPopupControlsStyle(Qt::ToolButtonStyle style)
{
    setValue("PopupControlsStyle", style);
}

Qt::ToolButtonStyle AppSettings::windowLanguagesStyle()
{
    return value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon).value<Qt::ToolButtonStyle>();
}

void AppSettings::setWindowLanguagesStyle(Qt::ToolButtonStyle style)
{
    setValue("WindowLanguagesStyle", style);
}

Qt::ToolButtonStyle AppSettings::windowControlsStyle()
{
    return value("WindowControlsStyle", Qt::ToolButtonIconOnly).value<Qt::ToolButtonStyle>();
}

void AppSettings::setWindowControlsStyle(Qt::ToolButtonStyle style)
{
    setValue("WindowControlsStyle", style);
}

QString AppSettings::trayIconName()
{
    return value("TrayIconName", "crow-translate-tray").toString();
}

void AppSettings::setTrayIconName(const QString &name)
{
    setValue("TrayIconName", name);
}

QString AppSettings::customIconPath()
{
    return value("CustomIconPath").toString();
}

void AppSettings::setCustomIconPath(const QString &path)
{
    setValue("CustomIconPath", path);
}

bool AppSettings::showSourceTranslit()
{
    return value("Translation/ShowSourceTranslit", true).toBool();
}

void AppSettings::setShowSourceTranslit(bool show)
{
    setValue("Translation/ShowSourceTranslit", show);
}

bool AppSettings::showTranslationTranslit()
{
    return value("Translation/ShowTranslationTranslit", true).toBool();
}

void AppSettings::setShowTranslationTranslit(bool show)
{
    setValue("Translation/ShowTranslationTranslit", show);
}

bool AppSettings::showSourceTranscription()
{
    return value("Translation/ShowSourceTranscription", true).toBool();
}

void AppSettings::setShowSourceTranscription(bool show)
{
    setValue("Translation/ShowSourceTranscription", show);
}

bool AppSettings::showTranslationOptions()
{
    return value("Translation/ShowTranslationOptions", true).toBool();
}

void AppSettings::setShowTranslationOptions(bool show)
{
    setValue("Translation/ShowTranslationOptions", show);
}

bool AppSettings::showDefinitions()
{
    return value("Translation/ShowDefinitions", true).toBool();
}

void AppSettings::setShowDefinitions(bool show)
{
    setValue("Translation/ShowDefinitions", show);
}

QOnlineTranslator::Language AppSettings::primaryLanguage()
{
    return value("Translation/PrimaryLanguage", QOnlineTranslator::Auto).value<QOnlineTranslator::Language>();
}

void AppSettings::setPrimaryLanguage(QOnlineTranslator::Language lang)
{
    setValue("Translation/PrimaryLanguage", lang);
}

QOnlineTranslator::Language AppSettings::secondaryLanguage()
{
    return value("Translation/SecondaryLanguage", QOnlineTranslator::English).value<QOnlineTranslator::Language>();
}

void AppSettings::setSecondaryLanguage(QOnlineTranslator::Language lang)
{
    setValue("Translation/SecondaryLanguage", lang);
}

QOnlineTranslator::Speaker AppSettings::speaker()
{
    return value("Translation/Speaker", QOnlineTranslator::Zahar).value<QOnlineTranslator::Speaker>();
}

void AppSettings::setSpeaker(QOnlineTranslator::Speaker speaker)
{
    setValue("Translation/Speaker", speaker);
}

QOnlineTranslator::Emotion AppSettings::emotion()
{
    return value("Translation/Emotion", QOnlineTranslator::Neutral).value<QOnlineTranslator::Emotion>();
}

void AppSettings::setEmotion(QOnlineTranslator::Emotion emotion)
{
    setValue("Translation/Emotion", emotion);
}

QNetworkProxy::ProxyType AppSettings::proxyType()
{
    return static_cast<QNetworkProxy::ProxyType>(value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt());
}

void AppSettings::setProxyType(QNetworkProxy::ProxyType type)
{
    setValue("Connection/ProxyType", type);
}

QString AppSettings::proxyHost()
{
    return value("Connection/ProxyHost").toString();
}

void AppSettings::setProxyHost(const QString &hostName)
{
    setValue("Connection/ProxyHost", hostName);
}

quint16 AppSettings::proxyPort()
{
    return value("Connection/ProxyPort", 8080).value<quint16>();
}

void AppSettings::setProxyPort(quint16 port)
{
    setValue("Connection/ProxyPort", port);
}

bool AppSettings::isProxyAuthEnabled()
{
    return value("Connection/ProxyAuthEnabled", false).toBool();
}

void AppSettings::setProxyAuthEnabled(bool enabled)
{
    setValue("Connection/ProxyAuthEnabled", enabled);
}

QString AppSettings::proxyUsername()
{
    return value("Connection/ProxyUsername").toString();
}

void AppSettings::setProxyUsername(const QString &username)
{
    setValue("Connection/ProxyUsername", username);
}

QString AppSettings::proxyPassword()
{
    return value("Connection/ProxyPassword").toString();
}

void AppSettings::setProxyPassword(const QString &password)
{
    setValue("Connection/ProxyPassword", password);
}

QString AppSettings::translateSelectionHotkey()
{
    return value("Hotkeys/TranslateSelection", defaultTranslateSelectionHotkey()).toString();
}

void AppSettings::setTranslateSelectionHotkey(const QString &hotkey)
{
    setValue("Hotkeys/TranslateSelection", hotkey);
}

QString AppSettings::playSelectionHotkey()
{
    return value("Hotkeys/PlaySelection", defaultPlaySelectionHotkey()).toString();
}

void AppSettings::setPlaySelectionHotkey(const QString &hotkey)
{
    setValue("Hotkeys/PlaySelection", hotkey);
}

QString AppSettings::playTranslatedSelectionHotkey()
{
    return value("Hotkeys/PlayTranslatedSelection", defaultPlayTranslatedSelectionHotkey()).toString();
}

void AppSettings::setPlayTranslatedSelectionHotkey(const QString &hotkey)
{
    setValue("Hotkeys/PlayTranslatedSelection", hotkey);
}

QString AppSettings::stopSelectionHotkey()
{
    return value("Hotkeys/StopSelection", defaultStopSelectionHotkey()).toString();
}

void AppSettings::setStopSelectionHotkey(const QString &hotkey)
{
    setValue("Hotkeys/StopSelection", hotkey);
}

QString AppSettings::showMainWindowHotkey()
{
    return value("Hotkeys/ShowMainWindow", defaultShowMainWindowHotkey()).toString();
}

void AppSettings::setShowMainWindowHotkey(const QString &hotkey)
{
    setValue("Hotkeys/ShowMainWindow", hotkey);
}

QString AppSettings::copyTranslatedSelectionHotkey()
{
    return value("Hotkeys/CopyTranslatedSelection", defaultCopyTranslatedSelectionHotkey()).toString();
}

void AppSettings::setCopyTranslatedSelectionHotkeyHotkey(const QString &hotkey)
{
    setValue("Hotkeys/CopyTranslatedSelection", hotkey);
}

QString AppSettings::translateHotkey()
{
    return value("Hotkeys/Translate", defaultTranslateHotkey()).toString();
}

void AppSettings::setTranslateHotkey(const QString &hotkey)
{
    setValue("Hotkeys/Translate", hotkey);
}

QString AppSettings::closeWindowHotkey()
{
    return value("Hotkeys/CloseWindow", defaultCloseWindowHotkey()).toString();
}

void AppSettings::setCloseWindowHotkey(const QString &hotkey)
{
    setValue("Hotkeys/CloseWindow", hotkey);
}

QString AppSettings::playSourceHotkey()
{
    return value("Hotkeys/PlaySource", defaultPlaySourceHotkey()).toString();
}

void AppSettings::setPlaySourceHotkey(const QString &hotkey)
{
    setValue("Hotkeys/PlaySource", hotkey);
}

QString AppSettings::stopSourceHotkey()
{
    return value("Hotkeys/StopSource", defaultStopSourceHotkey()).toString();
}

void AppSettings::setStopSourceHotkey(const QString &hotkey)
{
    setValue("Hotkeys/StopSource", hotkey);
}

QString AppSettings::playTranslationHotkey()
{
    return value("Hotkeys/PlayTranslation", defaultPlayTranslationHotkey()).toString();
}

void AppSettings::setPlayTranslationHotkey(const QString &hotkey)
{
    setValue("Hotkeys/PlayTranslation", hotkey);
}

QString AppSettings::stopTranslationHotkey()
{
    return value("Hotkeys/StopTranslation", defaultStopTranslationHotkey()).toString();
}

void AppSettings::setStopTranslationHotkey(const QString &hotkey)
{
    setValue("Hotkeys/StopTranslation", hotkey);
}

QString AppSettings::copyTranslationHotkey()
{
    return value("Hotkeys/CopyTranslation", defaultCopyTranslationHotkey()).toString();
}

void AppSettings::setCopyTranslationHotkey(const QString &hotkey)
{
    setValue("Hotkeys/CopyTranslation", hotkey);
}

QOnlineTranslator::Language AppSettings::buttonLanguage(LangButtonGroup *group, int id)
{
    return value("Buttons/" + group->name() + "Button" + QString::number(id), QOnlineTranslator::NoLanguage).value<QOnlineTranslator::Language>();
}

void AppSettings::setButtonLanguage(LangButtonGroup *group, int id, QOnlineTranslator::Language lang)
{
    setValue("Buttons/"  + group->name() + "Button" + QString::number(id), lang);
}

int AppSettings::checkedButton(LangButtonGroup *group)
{
    return value("Buttons/Checked" + group->name() + "Button", 0).toInt();
}

void AppSettings::setCheckedButton(LangButtonGroup *group, int id)
{
    setValue("Buttons/Checked" + group->name() + "Button", id);
}

QByteArray AppSettings::mainWindowGeometry()
{
    return value("WindowGeometry").toByteArray();
}

void AppSettings::setMainWindowGeometry(const QByteArray &geometry)
{
    setValue("WindowGeometry", geometry);
}

bool AppSettings::isAutoTranslateEnabled()
{
    return value("AutoTranslate", true).toBool();
}

void AppSettings::setAutoTranslateEnabled(bool enable)
{
    setValue("AutoTranslate", enable);
}

AppSettings::Engine AppSettings::currentEngine()
{
    return value("CurrentEngine", Google).value<Engine>();
}

void AppSettings::setCurrentEngine(AppSettings::Engine currentEngine)
{
    setValue("CurrentEngine", currentEngine);
}
