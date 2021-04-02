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

#ifdef Q_OS_UNIX
#include "ocr/ocr.h"

#include <QDBusConnection>
#include <QDBusError>
#endif

int launchGui(int argc, char *argv[]);
int launchCli(int argc, char *argv[]);
#ifdef Q_OS_UNIX
void registerDBusObject(QObject *object);
#endif

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
#if defined(Q_OS_LINUX)
    QGuiApplication::setDesktopFileName(QStringLiteral(DESKTOP_FILE));
#elif defined(Q_OS_WIN)
    QIcon::setThemeName("hicolor");
#endif

    SingleApplication app(argc, argv);

    AppSettings().setupLocalization();

    MainWindow window;

#ifdef Q_OS_UNIX
    if (QDBusConnection::sessionBus().isConnected()) {
        const QString service = QStringLiteral("io.crow_translate.CrowTranslate");
        if (QDBusConnection::sessionBus().registerService(service)) {
            registerDBusObject(&window);
            registerDBusObject(window.ocr());
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

#ifdef Q_OS_UNIX
void registerDBusObject(QObject *object)
{
    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/io/crow_translate/CrowTranslate/") + object->metaObject()->className(), object, QDBusConnection::ExportScriptableSlots))
        qWarning() << QCoreApplication::translate("D-Bus", "Unable to register D-Bus object for %1").arg(object->metaObject()->className());
}
#endif
