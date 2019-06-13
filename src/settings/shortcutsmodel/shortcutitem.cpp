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

#include <QStringList>

#include "shortcutitem.h"
#include "shortcutsmodel.h"

ShortcutItem::ShortcutItem(ShortcutsModel *parent) :
    m_parentModel(parent)
{
    m_parentItem = nullptr;
    parent->m_rootItems.append(this);
}

ShortcutItem::ShortcutItem(ShortcutItem *parent) :
    m_parentItem(parent),
    m_parentModel(parent->m_parentModel)
{
    m_parentItem->m_childItems.append(this);
}

ShortcutItem::~ShortcutItem()
{
    qDeleteAll(m_childItems);
}

ShortcutItem *ShortcutItem::child(int row)
{
    return m_childItems.value(row);
}

int ShortcutItem::childCount() const
{
    return m_childItems.count();
}

int ShortcutItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ShortcutItem *>(this));

    return 0;
}

ShortcutItem *ShortcutItem::parentItem()
{
    return m_parentItem;
}

QString ShortcutItem::description() const
{
    return m_description;
}

QIcon ShortcutItem::icon() const
{
    return m_icon;
}

QKeySequence ShortcutItem::shortcut() const
{
    return m_shortcut;
}

QKeySequence ShortcutItem::defaultShortcut() const
{
    return m_defaultShortcut;
}

void ShortcutItem::setShortcut(const QKeySequence &shortcut)
{
    if (shortcut == m_shortcut)
        return;

    m_shortcut = shortcut;
    m_parentModel->updateShortcutText(this);
}

void ShortcutItem::resetShortcut()
{
    setShortcut(m_defaultShortcut);
}

void ShortcutItem::setIconName(const QString &iconName)
{
    m_icon = QIcon::fromTheme(iconName);
}

void ShortcutItem::setDefaultShortcut(const QKeySequence &defaultShortcut)
{
    m_defaultShortcut = defaultShortcut;
}

void ShortcutItem::setDescription(const QString &description)
{
    m_description = description;
}
