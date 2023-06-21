/*
 *  Copyright © 2018-2023 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#include "macosautostartmanager.h"

#include "cmake.h"

#include <QApplication>
#include <QtCore>

macOSAutostartManager::macOSAutostartManager(QObject *parent)
    : AbstractAutostartManager(parent)
{
}

QString macOSAutostartManager::getLaunchAgentFilename()
{
    QDir launchAgentDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/../LaunchAgents"));
    return QFile(launchAgentDir.absoluteFilePath(QStringLiteral(APPLICATION_ID).append(".plist"))).fileName();
}

bool macOSAutostartManager::isAutostartEnabled() const
{
    return QFile(getLaunchAgentFilename()).exists();
}

void macOSAutostartManager::setAutostartEnabled(bool enabled)
{
    const bool autostartEnabled = isAutostartEnabled();
    if (enabled && !autostartEnabled) {
        QSettings launchAgent(getLaunchAgentFilename(), QSettings::NativeFormat);
        launchAgent.setValue("Label", QStringLiteral(APPLICATION_ID));
        launchAgent.setValue("ProgramArguments", QStringList() << QApplication::applicationFilePath());
        launchAgent.setValue("RunAtLoad", true);
        launchAgent.setValue("StandardErrorPath", "/dev/null");
        launchAgent.setValue("StandardOutPath", "/dev/null");
    } else if (autostartEnabled) {
        QFile::remove(getLaunchAgentFilename());
    }
}
