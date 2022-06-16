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

#include "waylandhelper.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/xdgforeign.h>

using namespace KWayland::Client;

WaylandHelper::WaylandHelper(QWindow *parent)
    : QObject(parent)
    , surface(Surface::fromWindow(parent))
    , registry(new Registry(this))
    , connection(ConnectionThread::fromApplication(this))
{
    if (!surface->isValid())
        qFatal("Invalid Wayland surface");
    registry.create(connection);
    registry.setup();
    if (!registry.isValid())
        qFatal("Invalid Wayland registry");
    connect(connection, &ConnectionThread::connectionDied, &registry, &Registry::destroy);
    connect(&registry, &Registry::exporterUnstableV2Announced, this, &WaylandHelper::onXdgExporterAnnounced);
    connect(&registry, &Registry::interfacesAnnounced, this, &WaylandHelper::onInterfacesAnnounced);
}

void WaylandHelper::onXdgExporterAnnounced(quint32 name, quint32 version)
{
    auto *xdgExporter = registry.createXdgExporter(name, version, this);
    if (!xdgExporter->isValid())
        qFatal("Invalid Wayland XdgExporter");
    xdgExportedTopLevel = xdgExporter->exportTopLevel(surface, this);
    if (!xdgExportedTopLevel->isValid())
        qFatal("Invalid Wayland toplevel export");
    connect(xdgExportedTopLevel, &XdgExported::done, this, &WaylandHelper::xdgExportDone);
}

void WaylandHelper::onInterfacesAnnounced()
{
    if (!xdgExportedTopLevel) {
        qWarning("Wayland compositor didn't announce XdgExporter, can't get export handle");
        emit xdgExportDone();
    }
}

QString WaylandHelper::exportedHandle() const
{
    if (!xdgExportedTopLevel)
        return {};
    return xdgExportedTopLevel->handle();
}
