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

#ifndef SHORTCUTSVIEW_H
#define SHORTCUTSVIEW_H

#include <QTreeView>

class ShortcutItem;
class ShortcutsModel;

class ShortcutsView : public QTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY(ShortcutsView)

public:
    explicit ShortcutsView(QWidget *parent = nullptr);

    ShortcutsModel *model() const;
    ShortcutItem *currentItem() const;

signals:
    void currentItemChanged(ShortcutItem *shortcut);
};

#endif // SHORTCUTSVIEW_H
