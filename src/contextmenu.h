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

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include "translationedit.h"

#include <QContextMenuEvent>
#include <QMenu>

class QContextMenuEvent;

class ContextMenu : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContextMenu)

public:
    template<class TextEdit>
    ContextMenu(TextEdit *edit, const QContextMenuEvent *event)
        : QObject(edit)
    {
        m_text = edit->textCursor().selectedText();
        if (m_text.isEmpty()) {
            if constexpr (std::is_same_v<TextEdit, TranslationEdit>)
                m_text = edit->translation();
            else
                m_text = edit->toPlainText();
        }

        m_menu = edit->createStandardContextMenu(event->globalPos());
        m_menu->move(event->globalPos());
        m_menu->addSeparator();
        QAction *searchOnForvoAction = m_menu->addAction(QIcon::fromTheme("text-speak"), tr("Search on Forvo.com"), this, &ContextMenu::searchOnForvo);
        if (m_text.isEmpty())
            searchOnForvoAction->setEnabled(false);
    }

    ~ContextMenu() override;

    void popup();

private slots:
    void searchOnForvo();

private:
    QMenu *m_menu;
    QString m_text;
};

#endif // CONTEXTMENU_H
