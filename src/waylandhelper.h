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

#ifndef WAYLANDHELPER_H
#define WAYLANDHELPER_H

#include <QtCore>

#include <KWayland/Client/registry.h>

namespace KWayland::Client
{
class Surface;
class ConnectionThread;
class XdgExporter;
class XdgExported;
}
class QWindow;

class WaylandHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WaylandHelper)

public:
    explicit WaylandHelper(QWindow *parent);

    QString exportedHandle() const;

signals:
    void xdgExportDone();

private:
    KWayland::Client::Surface *surface;
    KWayland::Client::Registry registry;
    KWayland::Client::ConnectionThread *connection;
    KWayland::Client::XdgExported *xdgExportedTopLevel = nullptr;

private slots:
    void onXdgExporterAnnounced(quint32 name, quint32 version);
    void onInterfacesAnnounced();
};

#endif // WAYLANDHELPER_H
