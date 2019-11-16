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

#include "shortcutsmodel.h"
#include "shortcutitem.h"
#include "settings/appsettings.h"

#include <QMetaEnum>
#include <QFont>

ShortcutsModel::ShortcutsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // Global shortcuts
    auto *globalShortcuts = new ShortcutItem(tr("Global"), this);
    m_rootItems.append(globalShortcuts);

    globalShortcuts->addChild(new ShortcutItem(tr("Translate selected text"), "preferences-desktop-locale", AppSettings::defaultTranslateSelectionHotkey()));
    globalShortcuts->addChild(new ShortcutItem(tr("Speak selected text"), "media-playback-start", AppSettings::defaultSpeakSelectionHotkey()));
    globalShortcuts->addChild(new ShortcutItem(tr("Speak translation of selected text"), "media-playback-start", AppSettings::defaultSpeakTranslatedSelectionHotkey()));
    globalShortcuts->addChild(new ShortcutItem(tr("Stop text speaking"), "media-playback-stop", AppSettings::defaultStopSpeakingHotkey()));
    globalShortcuts->addChild(new ShortcutItem(tr("Show main window"), "window", AppSettings::defaultShowMainWindowHotkey()));
    globalShortcuts->addChild(new ShortcutItem(tr("Translate selected text and copy to clipboard"), "edit-copy", AppSettings::defaultCopyTranslatedSelectionHotkey()));

    // Window shortcuts
    auto *windowShortcuts = new ShortcutItem(tr("Main window"), this);
    m_rootItems.append(windowShortcuts);

    windowShortcuts->addChild(new ShortcutItem(tr("Translate"), "go-next", AppSettings::defaultTranslateHotkey()));
    windowShortcuts->addChild(new ShortcutItem(tr("Close window"), "application-exit", AppSettings::defaultCloseWindowHotkey()));

    // Source text shortcuts
    auto *sourceText = new ShortcutItem(tr("Source text"));
    windowShortcuts->addChild(sourceText);
    sourceText->addChild(new ShortcutItem(tr("Play / pause text speaking"), "media-playback-start", AppSettings::defaultSpeakSourceHotkey()));

    // Translation text shortcuts
    auto *translationText = new ShortcutItem(tr("Translation"));
    windowShortcuts->addChild(translationText);

    translationText->addChild(new ShortcutItem(tr("Play / pause text speaking"), "media-playback-start", AppSettings::defaultSpeakTranslationHotkey()));
    translationText->addChild(new ShortcutItem(tr("Copy to clipboard"), "edit-copy", AppSettings::defaultCopyTranslationHotkey()));
}

ShortcutsModel::~ShortcutsModel()
{
    qDeleteAll(m_rootItems);
}

QVariant ShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto *shortcut = static_cast<ShortcutItem *>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case DescriptionColumn:
            return shortcut->description();
        case ShortcutColumn:
            return shortcut->shortcut();
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == DescriptionColumn)
            return shortcut->icon();
        break;
    case Qt::FontRole:
        if (shortcut->childCount() > 0) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    return QVariant();
}

QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case DescriptionColumn:
            return tr("Description");
        case ShortcutColumn:
            return tr("Shortcut");
        }
    }

    return QVariant();
}

QModelIndex ShortcutsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ShortcutItem *childItem;
    if (!parent.isValid()) {
        childItem = m_rootItems.value(row);
    } else {
        auto *parentItem = static_cast<ShortcutItem *>(parent.internalPointer());
        childItem = parentItem->child(row);
    }

    if (childItem == nullptr)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex ShortcutsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto *childItem = static_cast<ShortcutItem *>(index.internalPointer());
    ShortcutItem *parentItem = childItem->parentItem();

    if (parentItem == nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const
{
    ShortcutItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_rootItems.size();

    parentItem = static_cast<ShortcutItem *>(parent.internalPointer());
    return parentItem->childCount();
}

int ShortcutsModel::columnCount(const QModelIndex &) const
{
    return QMetaEnum::fromType<Column>().keyCount();
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto *item = static_cast<const ShortcutItem *>(index.internalPointer());
    Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index);
    itemFlags.setFlag(Qt::ItemIsEnabled, item->isEnabled());

    return itemFlags;
}

void ShortcutsModel::loadShortcuts(const AppSettings &settings)
{
    // Global shortcuts
    m_rootItems.at(0)->child(0)->setShortcut(settings.translateSelectionHotkey());
    m_rootItems.at(0)->child(1)->setShortcut(settings.speakSelectionHotkey());
    m_rootItems.at(0)->child(2)->setShortcut(settings.speakTranslatedSelectionHotkey());
    m_rootItems.at(0)->child(3)->setShortcut(settings.stopSpeakingHotkey());
    m_rootItems.at(0)->child(4)->setShortcut(settings.showMainWindowHotkey());
    m_rootItems.at(0)->child(5)->setShortcut(settings.copyTranslatedSelectionHotkey());

    // Window shortcuts
    m_rootItems.at(1)->child(0)->setShortcut(settings.translateHotkey());
    m_rootItems.at(1)->child(1)->setShortcut(settings.closeWindowHotkey());

    // Source text shortcuts
    m_rootItems.at(1)->child(2)->child(0)->setShortcut(settings.speakSourceHotkey());

    // Translation text shortcuts
    m_rootItems.at(1)->child(3)->child(0)->setShortcut(settings.speakTranslationHotkey());
    m_rootItems.at(1)->child(3)->child(1)->setShortcut(settings.copyTranslationHotkey());
}

void ShortcutsModel::saveShortcuts(AppSettings &settings) const
{
    // Global shortcuts
    settings.setTranslateSelectionHotkey(m_rootItems.at(0)->child(0)->shortcut());
    settings.setSpeakSelectionHotkey(m_rootItems.at(0)->child(1)->shortcut());
    settings.setSpeakTranslatedSelectionHotkey(m_rootItems.at(0)->child(2)->shortcut());
    settings.setStopSpeakingHotkey(m_rootItems.at(0)->child(3)->shortcut());
    settings.setShowMainWindowHotkey(m_rootItems.at(0)->child(4)->shortcut());
    settings.setCopyTranslatedSelectionHotkeyHotkey(m_rootItems.at(0)->child(5)->shortcut());

    // Window shortcuts
    settings.setTranslateHotkey(m_rootItems.at(1)->child(0)->shortcut());
    settings.setCloseWindowHotkey(m_rootItems.at(1)->child(1)->shortcut());

    // Source text shortcuts
    settings.setSpeakSourceHotkey(m_rootItems.at(1)->child(2)->child(0)->shortcut());

    // Translation text shortcuts
    settings.setSpeakTranslationHotkey(m_rootItems.at(1)->child(3)->child(0)->shortcut());
    settings.setCopyTranslationHotkey(m_rootItems.at(1)->child(3)->child(1)->shortcut());
}

void ShortcutsModel::resetAllShortcuts()
{
    for (ShortcutItem *item : m_rootItems)
        item->resetAllShortucts();
}

void ShortcutsModel::setGlobalShortuctsEnabled(bool enabled)
{
    m_rootItems.at(0)->setEnabled(enabled);
}

void ShortcutsModel::updateShortcut(ShortcutItem *item)
{
    const QModelIndex modelIndex = index(item, ShortcutColumn);
    emit dataChanged(modelIndex, modelIndex, {Qt::DisplayRole});
}

QModelIndex ShortcutsModel::index(ShortcutItem *item, int column) const
{
    if (item == nullptr)
        return QModelIndex();

    return index(item->row(), column, index(item->parentItem(), column));
}
