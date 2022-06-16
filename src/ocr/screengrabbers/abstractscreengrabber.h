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

#ifndef ABSTRACTSCREENGRABBER_H
#define ABSTRACTSCREENGRABBER_H

#include <QObject>

class QScreen;

class AbstractScreenGrabber : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScreenGrabber)

public:
    virtual bool ignoreDevicePixelRatio() const;

    static AbstractScreenGrabber *createScreenGrabber(QObject *parent = nullptr);

signals:
    void grabbed(const QMap<const QScreen *, QImage> &screenImages);
    void grabbingFailed();

public slots:
    virtual void grab() = 0;
    virtual void cancel() = 0;

protected:
    explicit AbstractScreenGrabber(QObject *parent = nullptr);

protected slots:
    void showError(const QString &errorString);
};

#endif // ABSTRACTSCREENGRABBER_H
