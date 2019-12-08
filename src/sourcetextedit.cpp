/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sourcetextedit.h"

#include <QTimer>

SourceTextEdit::SourceTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_textEditedTimer(new QTimer(this))
{
    m_textEditedTimer->setSingleShot(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    m_textEditedTimer->callOnTimeout(this, &SourceTextEdit::translationRequested);
#else
    connect(m_textEditedTimer, &QTimer::timeout, this, &SourceTextEdit::translationRequested);
#endif
    connect(this, &SourceTextEdit::textChanged, this, &SourceTextEdit::checkSourceEmptyChanged);
}

void SourceTextEdit::setRequestTranlationOnEdit(bool listen)
{
    m_requestTranslationOnEdit = listen;

    if (m_requestTranslationOnEdit) {
        connect(this, &SourceTextEdit::textChanged, this, &SourceTextEdit::startTimerDelay);
    } else {
        m_textEditedTimer->stop();
        disconnect(this, &SourceTextEdit::textChanged, this, &SourceTextEdit::startTimerDelay);
    }
}

void SourceTextEdit::markSourceAsChanged()
{
    if (m_requestTranslationOnEdit) {
        m_textEditedTimer->stop();
        emit translationRequested();
    }
}

void SourceTextEdit::stopChangedTimer()
{
    m_textEditedTimer->stop();
}

void SourceTextEdit::startTimerDelay()
{
    m_textEditedTimer->start(s_delay);
}

void SourceTextEdit::checkSourceEmptyChanged()
{
    if (toPlainText().isEmpty() != m_sourceEmpty) {
        m_sourceEmpty = toPlainText().isEmpty();
        emit sourceEmpty(m_sourceEmpty);
    }
}
