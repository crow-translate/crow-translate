/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "unixautostartmanager.h"

#include <QDir>
#include <QGuiApplication>
#include <QStandardPaths>

UnixAutostartManager::UnixAutostartManager(QObject *parent)
    : AbstractAutostartManager(parent)
{
}

bool UnixAutostartManager::isAutostartEnabled() const
{
    return QFileInfo::exists(QStringLiteral("%1/autostart/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), QGuiApplication::desktopFileName()));
}

void UnixAutostartManager::setAutostartEnabled(bool enabled)
{
    QDir autostartDir(QStringLiteral("%1/autostart").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)));

    if (enabled) {
        // Create autorun file
        if (autostartDir.exists(QGuiApplication::desktopFileName()))
            return;

        if (!autostartDir.exists()) {
            if (!autostartDir.mkpath(QStringLiteral("."))) {
                showError(tr("Unable to create %1").arg(autostartDir.path()));
                return;
            }
        }

        const QString desktopFileName = QStringLiteral("/usr/share/applications/%1").arg(QGuiApplication::desktopFileName());
        if (!QFile::copy(desktopFileName, autostartDir.filePath(QGuiApplication::desktopFileName())))
            showError(tr("Unable to copy %1 to %2").arg(desktopFileName, autostartDir.path()));

    } else if (autostartDir.exists(QGuiApplication::desktopFileName())) {
        // Remove autorun file
        if (!autostartDir.remove(QGuiApplication::desktopFileName()))
            showError(tr("Unable to remove %1 from %2").arg(QGuiApplication::desktopFileName(), autostartDir.path()));
    }
}
