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

#include "abstractautostartmgr.h"
#ifdef Q_OS_LINUX
#include "portalautostartmgr.h"
#include "unixconfigautostartmgr.h"
#endif
#ifdef Q_OS_WIN
#include "windowsautostartmgr.h"
#endif

#include <QMessageBox>
#include <QtCore>

AbstractAutostartMgr::AbstractAutostartMgr(QObject *parent)
    : QObject(parent)
{
}

bool AbstractAutostartMgr::canCheckEnabled()
{
    return true;
}

void AbstractAutostartMgr::setAutostartEnabled(bool enabled)
{
    qDebug() << QString("Set autostart to %1").arg(enabled);
}

AbstractAutostartMgr *AbstractAutostartMgr::createAutostartMgr(QObject *parent)
{
#ifdef Q_OS_LINUX
    if (PortalAutostartMgr::isAvailable())
        return new PortalAutostartMgr(parent);
    return new UnixConfigAutostartMgr(parent);
#endif
#ifdef Q_OS_WIN
    return new WindowsAutostartMgr(parent);
#endif
    qFatal("No autostart provider implemented");
}

void AbstractAutostartMgr::showError(const QString &errorString)
{
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("Unable to apply autostart settings"));
    message.setInformativeText(errorString);
    message.exec();

    emit autostartToggleFailed();
}
