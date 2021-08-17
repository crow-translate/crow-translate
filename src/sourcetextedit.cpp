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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sourcetextedit.h"

#include "contextmenu.h"

#include <QTimer>

using namespace std::chrono_literals;

SourceTextEdit::SourceTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_textEditedTimer(new QTimer(this))
{
    m_textEditedTimer->setSingleShot(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    m_textEditedTimer->callOnTimeout(this, &SourceTextEdit::textEdited);
#else
    connect(m_textEditedTimer, &QTimer::timeout, this, &SourceTextEdit::textEdited);
#endif
    connect(this, &SourceTextEdit::textChanged, this, &SourceTextEdit::checkSourceEmptyChanged);
}

void SourceTextEdit::setListenForEdits(bool listen)
{
    m_listenForEdits = listen;

    if (m_listenForEdits) {
        connect(this, &SourceTextEdit::textChanged, this, &SourceTextEdit::startTimerDelay);
    } else {
        m_textEditedTimer->stop();
        disconnect(this, &SourceTextEdit::textChanged, this, &SourceTextEdit::startTimerDelay);
    }
}

void SourceTextEdit::setSimplifySource(bool enabled)
{
    m_simplifySource = enabled;
}

QString SourceTextEdit::toSourceText()
{
    return m_simplifySource ? toPlainText().simplified() : toPlainText().trimmed();
}

void SourceTextEdit::replaceText(const QString &text)
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::Document);
    if (text.isEmpty())
        cursor.removeSelectedText();
    else
        cursor.insertText(text);
    setTextCursor(cursor);

    // To avoid emitting textEdited signal
    m_textEditedTimer->stop();
}

void SourceTextEdit::removeText()
{
    replaceText({});
}

void SourceTextEdit::stopEditTimer()
{
    m_textEditedTimer->stop();
}

void SourceTextEdit::startTimerDelay()
{
    m_textEditedTimer->start(500ms);
}

void SourceTextEdit::checkSourceEmptyChanged()
{
    if (toPlainText().isEmpty() != m_sourceEmpty) {
        m_sourceEmpty = toPlainText().isEmpty();
        emit sourceEmpty(m_sourceEmpty);
    }
}

void SourceTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    auto *contextMenu = new ContextMenu(this, event);
    contextMenu->popup();
}
