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

#ifndef PORTALAUTOSTARTMGR_H
#define PORTALAUTOSTARTMGR_H

#include "abstractautostartmgr.h"

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <qvariant.h>

class QDBusPendingCallWatcher;

class PortalAutostartMgr : public AbstractAutostartMgr
{
    Q_OBJECT
    Q_DISABLE_COPY(PortalAutostartMgr)

public:
    explicit PortalAutostartMgr(QObject *parent = nullptr);

    static bool isAvailable();

    bool canCheckEnabled() override;

    bool isAutostartEnabled() override;

public slots:
    void setAutostartEnabled(bool enabled) override;
    void onHandleResponse(uint, const QVariantMap &results);

private:
    static QDBusInterface s_interface;
};

#endif // PORTALAUTOSTARTMGR_H
