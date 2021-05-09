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

#ifndef WAYLANDPORTALSCREENGRABBER_H
#define WAYLANDPORTALSCREENGRABBER_H

#include "dbusscreengrabber.h"

#include <QDBusInterface>

class WaylandPortalScreenGrabber : public DBusScreenGrabber
{
    Q_OBJECT
    Q_DISABLE_COPY(WaylandPortalScreenGrabber)

public:
    explicit WaylandPortalScreenGrabber(QObject *parent = nullptr);

    bool ignoreDevicePixelRatio() const override;

    static bool isAvailable();

public slots:
    void grab() override;
    void cancel() override;

private slots:
    void parsePortalResponse(uint, const QVariantMap &response);

private:
    QString m_responseServicePath;

    static QDBusInterface s_interface;
};

#endif // WAYLANDPORTALSCREENGRABBER_H
