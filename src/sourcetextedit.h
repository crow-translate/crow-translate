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

#ifndef SOURCETEXTEDIT_H
#define SOURCETEXTEDIT_H

#include <QPlainTextEdit>

class QTimer;

class SourceTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(SourceTextEdit)

public:
    explicit SourceTextEdit(QWidget *parent = nullptr);

    void setListenForEdits(bool listen);
    void setSimplifySource(bool enabled);
    QString toSourceText();

    // Text manipulation that preserves undo / redo history
    void replaceText(const QString &text);
    void removeText();

public slots:
    void stopEditTimer();

signals:
    void textEdited();
    void sourceEmpty(bool empty);

private slots:
    void startTimerDelay();
    void checkSourceEmptyChanged();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QTimer *m_textEditedTimer;
    bool m_listenForEdits = false;
    bool m_sourceEmpty = true;
    bool m_simplifySource = false;
};

#endif // SOURCETEXTEDIT_H
