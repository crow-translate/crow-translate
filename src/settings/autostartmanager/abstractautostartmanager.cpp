/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "abstractautostartmanager.h"
#if defined(Q_OS_LINUX)
#include "portalautostartmanager.h"
#include "unixautostartmanager.h"
#elif defined(Q_OS_WIN)
#include "windowsautostartmanager.h"
#endif

#include <QMessageBox>

AbstractAutostartManager::AbstractAutostartManager(QObject *parent)
    : QObject(parent)
{
}

AbstractAutostartManager *AbstractAutostartManager::createAutostartManager(QObject *parent)
{
#if defined(Q_OS_LINUX)
    if (PortalAutostartManager::isAvailable())
        return new PortalAutostartManager(parent);
    return new UnixAutostartManager(parent);
#elif defined(Q_OS_WIN)
    return new WindowsAutostartManager(parent);
#else
    qFatal("No autostart provider implemented");
#endif
}

void AbstractAutostartManager::showError(const QString &informativeText)
{
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("Unable to apply autostart settings"));
    message.setInformativeText(informativeText);
    message.exec();
}
