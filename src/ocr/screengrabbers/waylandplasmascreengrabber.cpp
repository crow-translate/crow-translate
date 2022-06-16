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

#include "waylandplasmascreengrabber.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QPixmap>
#include <QScreen>
#include <QtConcurrent>

#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

using ScreenImagesMap = QMap<const QScreen *, QImage>;

QDBusInterface WaylandPlasmaScreenGrabber::s_interface(QStringLiteral("org.kde.KWin"),
                                                       QStringLiteral("/Screenshot"),
                                                       QStringLiteral("org.kde.kwin.Screenshot"));

WaylandPlasmaScreenGrabber::WaylandPlasmaScreenGrabber(QObject *parent)
    : DBusScreenGrabber(parent)
{
    static int id = qRegisterMetaType<ScreenImagesMap>("ScreenImagesMap");
    Q_UNUSED(id)
}

bool WaylandPlasmaScreenGrabber::ignoreDevicePixelRatio() const
{
    return true;
}

bool WaylandPlasmaScreenGrabber::isAvailable()
{
    return s_interface.isValid();
}

void WaylandPlasmaScreenGrabber::grab()
{
    QVarLengthArray<int, 2> pipe;
    if (pipe2(pipe.data(), O_CLOEXEC | O_NONBLOCK) != 0) {
        showError(tr("Unable to create pipe: %1.").arg(strerror(errno)));
        return;
    }

    const QDBusPendingReply<void> reply = s_interface.asyncCall(QStringLiteral("screenshotFullscreen"), QVariant::fromValue(QDBusUnixFileDescriptor(pipe[1])), false, true);
    m_callWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(m_callWatcher, &QDBusPendingCallWatcher::finished, [this, sockedDescriptor = pipe[0]] {
        const QDBusPendingReply<void> reply = readReply<void>();

        if (!reply.isValid()) {
            showError(reply.error().message());
            return;
        }

        m_readImageFuture = QtConcurrent::run([this, sockedDescriptor] {
            readPixmapFromSocket(sockedDescriptor);
            close(sockedDescriptor);
        });
    });
    close(pipe[1]);
}

void WaylandPlasmaScreenGrabber::cancel()
{
    DBusScreenGrabber::cancel();

    m_readImageFuture.cancel();
}

void WaylandPlasmaScreenGrabber::readPixmapFromSocket(int socketDescriptor)
{
    QByteArray data;
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(socketDescriptor, &readset);
    timeval timeout{30, 0};
    QVarLengthArray<char, 4096 * 16> buffer;

    forever {
        if (m_readImageFuture.isCanceled())
            return;

        const int ready = select(FD_SETSIZE, &readset, nullptr, nullptr, &timeout);
        if (ready < 0) {
            QMetaObject::invokeMethod(this, "showError", Q_ARG(QString, tr("Unable to wait for socket readiness: %1.").arg(strerror(errno))));
            return;
        }

        if (ready == 0) {
            QMetaObject::invokeMethod(this, "showError", Q_ARG(QString, tr("Timeout reading from pipe.")));
            return;
        }

        const ssize_t bytesRead = read(socketDescriptor, buffer.data(), buffer.capacity());
        if (bytesRead < 0) {
            QMetaObject::invokeMethod(this, "showError", Q_ARG(QString, tr("Unable to read data from socket: %1.").arg(strerror(errno))));
            return;
        }
        if (bytesRead == 0) {
            QDataStream lDataStream(data);
            QPixmap pixmap;
            lDataStream >> pixmap;
            const QMetaMethod grabbed = QMetaMethod::fromSignal(&WaylandPlasmaScreenGrabber::grabbed);
            grabbed.invoke(this, Q_ARG(ScreenImagesMap, splitScreenImages(pixmap)));
            return;
        }

        data.append(buffer.data(), static_cast<int>(bytesRead));
    }
}
