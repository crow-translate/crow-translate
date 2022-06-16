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

#ifndef SHORTCUTITEM_H
#define SHORTCUTITEM_H

#include <QIcon>
#include <QKeySequence>

class ShortcutsModel;

class ShortcutItem
{
    Q_DISABLE_COPY(ShortcutItem)

public:
    explicit ShortcutItem(ShortcutsModel *model);
    ShortcutItem(QString description, ShortcutItem *parent);
    ShortcutItem(QString description, const QString &iconName, ShortcutItem *parent);
    ~ShortcutItem();

    ShortcutItem *child(int row);
    int childCount() const;
    int row() const;
    ShortcutItem *parentItem();

    QString description() const;
    QIcon icon() const;

    QKeySequence defaultShortcut() const;
    void setDefaultShortcut(const QKeySequence &shortcut);

    QKeySequence shortcut() const;
    void setShortcut(const QKeySequence &shortcut);
    void resetShortcut();
    void resetAllShortucts();

    bool isEnabled() const;
    void setEnabled(bool enabled);

private:
    QString m_description;
    QIcon m_icon;
    QKeySequence m_shortcut;
    QKeySequence m_defaultShortcut;
    bool m_enabled = true;

    QVector<ShortcutItem *> m_childItems;
    ShortcutItem *m_parentItem = nullptr;
    ShortcutsModel *m_model;
};

#endif // SHORTCUTITEM_H
