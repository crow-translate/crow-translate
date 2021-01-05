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

#include "cli.h"
#include "cmake.h"
#include "mainwindow.h"
#include "singleapplication.h"
#include "settings/appsettings.h"

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusError>
#endif

int launchGui(int argc, char *argv[]);
int launchCli(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationVersion(QStringLiteral("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH));
    QCoreApplication::setApplicationName(QStringLiteral(APPLICATION_NAME));
    QCoreApplication::setOrganizationName(QStringLiteral(APPLICATION_NAME));

    if (argc == 1)
        return launchGui(argc, argv); // Launch GUI if there are no arguments

    return launchCli(argc, argv);
}

int launchGui(int argc, char *argv[])
{
    SingleApplication app(argc, argv);

#if defined(Q_OS_LINUX)
    QGuiApplication::setDesktopFileName(QStringLiteral(DESKTOP_FILE));
#elif defined(Q_OS_WIN)
    QIcon::setThemeName("We10X");
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/icons");
#endif

    AppSettings().setupLocalization();

    MainWindow window;

#ifdef Q_OS_LINUX
    if (QDBusConnection::sessionBus().isConnected()) {
        if (const QString service = QStringLiteral("io.crow_translate.CrowTranslate"); QDBusConnection::sessionBus().registerService(service)) {
            if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/io/crow_translate/CrowTranslate/MainWindow"), &window, QDBusConnection::ExportScriptableSlots))
                qWarning() << QCoreApplication::translate("D-Bus", "Unable to register D-Bus object for %1").arg(window.metaObject()->className());
        } else {
            qWarning() << QCoreApplication::translate("D-Bus", "D-Bus service %1 is already registered by another application").arg(service);
        }
    }
#endif

    return QCoreApplication::exec();
}

int launchCli(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const AppSettings settings;
    settings.setupLocalization();

    Cli cli;
    cli.process(app);

    return QCoreApplication::exec();
}
