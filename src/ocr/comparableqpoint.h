/*
 *  Copyright © 2020 Méven Car <meven.car@enioka.com>
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

#ifndef COMPARABLEQPOINT_H
#define COMPARABLEQPOINT_H

#include <QPoint>

// Utility class that allows using QMap to sort its keys when they are QPoint
class ComparableQPoint : public QPoint
{
public:
    ComparableQPoint() = default;
    ComparableQPoint(QPoint point)
        : QPoint(point)
    {
    }

    bool operator<(ComparableQPoint other) const
    {
        return x() < other.x() || (x() == other.x() && y() < other.y());
    }
};

#endif // COMPARABLEQPOINT_H
