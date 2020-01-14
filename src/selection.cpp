/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "selection.h"
#include "singleapplication.h"

#include <QClipboard>
#ifdef Q_OS_WIN
#include <QTimer>
#include <QMimeData>

#include <windows.h>
#endif

Selection::~Selection() = default;

Selection *Selection::instance()
{
    static Selection singletone;
    return &singletone;
}

void Selection::requestSelection()
{
#if defined(Q_OS_LINUX)
    getSelection();
#elif defined(Q_OS_WIN) // Send Ctrl + C to get selected text
    // Save the original clipboard
    m_originalClipboardData.reset(new QMimeData);
    const QMimeData *clipboardData = SingleApplication::clipboard()->mimeData();
    for (const QString &format : clipboardData->formats())
        m_originalClipboardData->setData(format, clipboardData->data(format));

    // Wait until all modifiers will be unpressed (to avoid conflicts with the other shortcuts)
    while(GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN) || GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_MENU) || GetAsyncKeyState(VK_CONTROL));

    // Generate Ctrl + C input
    INPUT copyText[4];

    // Set the press of the "Ctrl" key
    copyText[0].ki.wVk = VK_CONTROL;
    copyText[0].ki.dwFlags = 0; // 0 for key press
    copyText[0].type = INPUT_KEYBOARD;

    // Set the press of the "C" key
    copyText[1].ki.wVk = 'C';
    copyText[1].ki.dwFlags = 0;
    copyText[1].type = INPUT_KEYBOARD;

    // Set the release of the "C" key
    copyText[2].ki.wVk = 'C';
    copyText[2].ki.dwFlags = KEYEVENTF_KEYUP;
    copyText[2].type = INPUT_KEYBOARD;

    // Set the release of the "Ctrl" key
    copyText[3].ki.wVk = VK_CONTROL;
    copyText[3].ki.dwFlags = KEYEVENTF_KEYUP;
    copyText[3].type = INPUT_KEYBOARD;

    // Send key sequence to system
    SendInput(static_cast<UINT>(std::size(copyText)), copyText, sizeof(INPUT));

    // Wait for the clipboard changes
    connect(SingleApplication::clipboard(), &QClipboard::dataChanged, this, &Selection::getSelection);
    m_maxSelectionDelay->start();
#endif
}

Selection::Selection()
{
#ifdef Q_OS_WIN
    m_maxSelectionDelay = new QTimer(this);
    m_maxSelectionDelay->setSingleShot(true);
    m_maxSelectionDelay->setInterval(1000);
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    connect(m_maxSelectionWaitDelay, &QTimer::timeout, this, &Selection::saveSelection);
#else
    m_maxSelectionDelay->callOnTimeout(this, &Selection::getSelection);
#endif
#endif
}

void Selection::getSelection()
{
#if defined(Q_OS_LINUX)
     emit requestedSelectionAvailable(SingleApplication::clipboard()->text(QClipboard::Selection));
#elif defined(Q_OS_WIN)
    const QString selection = SingleApplication::clipboard()->text();
    if (selection.isEmpty() && m_maxSelectionDelay->isActive())
        return;

    m_maxSelectionDelay->stop();
    disconnect(SingleApplication::clipboard(), &QClipboard::dataChanged, this, &Selection::getSelection);

    // Restore the clipboard data after exit to event loop
    QMetaObject::invokeMethod(this, [this] {
        SingleApplication::clipboard()->setMimeData(m_originalClipboardData.take());
    },  Qt::QueuedConnection);

    emit requestedSelectionAvailable(selection);
#endif
}
