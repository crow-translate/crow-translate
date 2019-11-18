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
    , m_rootItem(new ShortcutItem(this))
{
    // Global shortcuts
    auto *globalShortcuts = new ShortcutItem(tr("Global"), m_rootItem);

    auto *translateSelectionShortcut = new ShortcutItem(tr("Translate selected text"), "preferences-desktop-locale", globalShortcuts);
    translateSelectionShortcut->setDefaultShortcut(AppSettings::defaultTranslateSelectionHotkey());

    auto *speakSelection = new ShortcutItem(tr("Speak selected text"), "media-playback-start", globalShortcuts);
    speakSelection->setDefaultShortcut(AppSettings::defaultSpeakSelectionHotkey());

    auto *speakTranslatedSelectionShortcut = new ShortcutItem(tr("Speak translation of selected text"), "media-playback-start", globalShortcuts);
    speakTranslatedSelectionShortcut->setDefaultShortcut(AppSettings::defaultSpeakTranslatedSelectionHotkey());

    auto *stopSpeakingShortcut = new ShortcutItem(tr("Stop text speaking"), "media-playback-stop", globalShortcuts);
    stopSpeakingShortcut->setDefaultShortcut(AppSettings::defaultStopSpeakingHotkey());

    auto *showMainWindowShortcut = new ShortcutItem(tr("Show main window"), "window", globalShortcuts);
    showMainWindowShortcut->setDefaultShortcut(AppSettings::defaultShowMainWindowHotkey());

    auto *copyTranslatedSelectionShortcut = new ShortcutItem(tr("Translate selected text and copy to clipboard"), "edit-copy", globalShortcuts);
    copyTranslatedSelectionShortcut->setDefaultShortcut(AppSettings::defaultCopyTranslatedSelectionHotkey());

    // Window shortcuts
    auto *windowShortcuts = new ShortcutItem(tr("Main window"), m_rootItem);

    auto *translateShortcut = new ShortcutItem(tr("Translate"), "go-next", windowShortcuts);
    translateShortcut->setDefaultShortcut(AppSettings::defaultTranslateHotkey());

    auto *closeWindowShortcut = new ShortcutItem(tr("Close window"), "application-exit", windowShortcuts);
    closeWindowShortcut->setDefaultShortcut(AppSettings::defaultCloseWindowHotkey());

    // Source text shortcuts
    auto *sourceText = new ShortcutItem(tr("Source text"), windowShortcuts);

    auto *speakSourceShortcut = new ShortcutItem(tr("Play / pause text speaking"), "media-playback-start", sourceText);
    speakSourceShortcut->setDefaultShortcut(AppSettings::defaultSpeakSourceHotkey());

    // Translation text shortcuts
    auto *translationText = new ShortcutItem(tr("Translation"), windowShortcuts);

    auto *speakTranslationShortcut = new ShortcutItem(tr("Play / pause text speaking"), "media-playback-start", translationText);
    speakTranslationShortcut->setDefaultShortcut(AppSettings::defaultSpeakTranslationHotkey());

    auto *copyTranslationShortcut = new ShortcutItem(tr("Copy to clipboard"), "edit-copy", translationText);
    copyTranslationShortcut->setDefaultShortcut(AppSettings::defaultCopyTranslationHotkey());
}

ShortcutsModel::~ShortcutsModel()
{
    delete m_rootItem;
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

    ShortcutItem *parentItem = parent.isValid() ? static_cast<ShortcutItem *>(parent.internalPointer()) : m_rootItem;

    ShortcutItem *childItem = parentItem->child(row);
    if (!childItem)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex ShortcutsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto *item = static_cast<ShortcutItem *>(index.internalPointer());
    ShortcutItem *parentItem = item->parentItem();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_rootItem->childCount();

    return static_cast<ShortcutItem *>(parent.internalPointer())->childCount();
}

int ShortcutsModel::columnCount(const QModelIndex &) const
{
    return QMetaEnum::fromType<Column>().keyCount();
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index);
    auto *item = static_cast<const ShortcutItem *>(index.internalPointer());
    itemFlags.setFlag(Qt::ItemIsEnabled, item->isEnabled());

    return itemFlags;
}

void ShortcutsModel::loadShortcuts(const AppSettings &settings)
{
    // Global shortcuts
    m_rootItem->child(0)->child(0)->setShortcut(settings.translateSelectionHotkey());
    m_rootItem->child(0)->child(1)->setShortcut(settings.speakSelectionHotkey());
    m_rootItem->child(0)->child(2)->setShortcut(settings.speakTranslatedSelectionHotkey());
    m_rootItem->child(0)->child(3)->setShortcut(settings.stopSpeakingHotkey());
    m_rootItem->child(0)->child(4)->setShortcut(settings.showMainWindowHotkey());
    m_rootItem->child(0)->child(5)->setShortcut(settings.copyTranslatedSelectionHotkey());

    // Window shortcuts
    m_rootItem->child(1)->child(0)->setShortcut(settings.translateHotkey());
    m_rootItem->child(1)->child(1)->setShortcut(settings.closeWindowHotkey());

    // Source text shortcuts
    m_rootItem->child(1)->child(2)->child(0)->setShortcut(settings.speakSourceHotkey());

    // Translation text shortcuts
    m_rootItem->child(1)->child(3)->child(0)->setShortcut(settings.speakTranslationHotkey());
    m_rootItem->child(1)->child(3)->child(1)->setShortcut(settings.copyTranslationHotkey());
}

void ShortcutsModel::saveShortcuts(AppSettings &settings) const
{
    // Global shortcuts
    settings.setTranslateSelectionHotkey(m_rootItem->child(0)->child(0)->shortcut());
    settings.setSpeakSelectionHotkey(m_rootItem->child(0)->child(1)->shortcut());
    settings.setSpeakTranslatedSelectionHotkey(m_rootItem->child(0)->child(2)->shortcut());
    settings.setStopSpeakingHotkey(m_rootItem->child(0)->child(3)->shortcut());
    settings.setShowMainWindowHotkey(m_rootItem->child(0)->child(4)->shortcut());
    settings.setCopyTranslatedSelectionHotkeyHotkey(m_rootItem->child(0)->child(5)->shortcut());

    // Window shortcuts
    settings.setTranslateHotkey(m_rootItem->child(1)->child(0)->shortcut());
    settings.setCloseWindowHotkey(m_rootItem->child(1)->child(1)->shortcut());

    // Source text shortcuts
    settings.setSpeakSourceHotkey(m_rootItem->child(1)->child(2)->child(0)->shortcut());

    // Translation text shortcuts
    settings.setSpeakTranslationHotkey(m_rootItem->child(1)->child(3)->child(0)->shortcut());
    settings.setCopyTranslationHotkey(m_rootItem->child(1)->child(3)->child(1)->shortcut());
}

void ShortcutsModel::resetAllShortcuts()
{
    m_rootItem->resetAllShortucts();
}

void ShortcutsModel::setGlobalShortuctsEnabled(bool enabled)
{
    m_rootItem->child(0)->setEnabled(enabled);
}

void ShortcutsModel::updateShortcut(ShortcutItem *item)
{
    const QModelIndex modelIndex = index(item, ShortcutColumn);
    emit dataChanged(modelIndex, modelIndex, {Qt::DisplayRole});
}

QModelIndex ShortcutsModel::index(ShortcutItem *item, int column) const
{
    if (item == m_rootItem)
        return QModelIndex();

    return index(item->row(), column, index(item->parentItem(), column));
}
