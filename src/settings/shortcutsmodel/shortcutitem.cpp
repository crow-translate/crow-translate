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

#include "shortcutitem.h"

#include "shortcutsmodel.h"

ShortcutItem::ShortcutItem(ShortcutsModel *model)
    : m_model(model)
{
}

ShortcutItem::ShortcutItem(QString description, ShortcutItem *parent)
    : m_description(qMove(description))
    , m_parentItem(parent)
    , m_model(parent->m_model)
{
    m_parentItem->m_childItems.append(this);
}

ShortcutItem::ShortcutItem(QString description, const QString &iconName, ShortcutItem *parent)
    : m_description(qMove(description))
    , m_icon(QIcon::fromTheme(iconName))
    , m_parentItem(parent)
    , m_model(parent->m_model)
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
    if (m_parentItem != nullptr)
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

void ShortcutItem::setDefaultShortcut(const QKeySequence &shortcut)
{
    m_defaultShortcut = shortcut;
}

void ShortcutItem::setShortcut(const QKeySequence &shortcut)
{
    if (shortcut == m_shortcut)
        return;

    m_shortcut = shortcut;
    m_model->updateShortcut(this);
}

void ShortcutItem::resetShortcut()
{
    setShortcut(m_defaultShortcut);
}

void ShortcutItem::resetAllShortucts()
{
    m_shortcut = m_defaultShortcut;
    for (ShortcutItem *item : qAsConst(m_childItems))
        item->resetAllShortucts();
}

bool ShortcutItem::isEnabled() const
{
    return m_enabled;
}

void ShortcutItem::setEnabled(bool enabled)
{
    m_enabled = enabled;
    m_model->updateItem(this);
    for (ShortcutItem *item : qAsConst(m_childItems))
        item->setEnabled(enabled);
}
