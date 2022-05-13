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

#ifndef ABSTRACTAUTOSTARTMGR_H
#define ABSTRACTAUTOSTARTMGR_H

#include <QObject>

class QScreen;

class AbstractAutostartMgr : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractAutostartMgr)

public:
    static AbstractAutostartMgr *createAutostartMgr(QObject *parent = nullptr);

    virtual bool canCheckEnabled();

    virtual bool isAutostartEnabled() = 0;

public slots:
    virtual void setAutostartEnabled(bool enabled) = 0;

signals:
    void autostartEnabled(bool enabled);
    void autostartToggleFailed();

protected:
    explicit AbstractAutostartMgr(QObject *parent = nullptr);

protected slots:
    void showError(const QString &errorString);
};

#endif // ABSTRACTAUTOSTARTMGR_H
