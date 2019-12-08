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

#ifndef SOURCETEXTEDIT_H
#define SOURCETEXTEDIT_H

#include <QPlainTextEdit>

class QTimer;

class SourceTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(SourceTextEdit)

public:
    SourceTextEdit(QWidget *parent = nullptr);

    void setRequestTranlationOnEdit(bool listen);

public slots:
    void markSourceAsChanged();
    void stopChangedTimer();

signals:
    void translationRequested();
    void sourceEmpty(bool empty);

private slots:
    void startTimerDelay();
    void checkSourceEmptyChanged();

private:
    QTimer *m_textEditedTimer;
    bool m_requestTranslationOnEdit = false;
    bool m_sourceEmpty = true;

    static constexpr int s_delay = 500;
};

#endif // SOURCETEXTEDIT_H
