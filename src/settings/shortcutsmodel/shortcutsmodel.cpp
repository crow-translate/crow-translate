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
#include "shortcutitem.h"
#include "settings/appsettings.h"

#include <QStringList>
#include <QMetaEnum>
#include <QFont>

ShortcutsModel::ShortcutsModel(QObject *parent) :
    QAbstractItemModel(parent)
{
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
        ShortcutItem *parentItem = static_cast<ShortcutItem *>(parent.internalPointer());
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

    ShortcutItem *childItem = static_cast<ShortcutItem *>(index.internalPointer());
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
    return QMetaEnum::fromType<Columns>().keyCount();
}

void ShortcutsModel::loadShortcuts(const AppSettings &settings)
{
    beginResetModel();
    qDeleteAll(m_rootItems);
    m_rootItems.clear();

    // Global shortcuts
    auto *globalShortcuts = new ShortcutItem(this);
    globalShortcuts->setDescription(tr("Global"));

    auto *translateSelectedText = new ShortcutItem(globalShortcuts);
    translateSelectedText->setDescription(tr("Translate selected text"));
    translateSelectedText->setIconName("preferences-desktop-locale");
    translateSelectedText->setShortcut(settings.translateSelectionHotkey());
    translateSelectedText->setDefaultShortcut(AppSettings::defaultTranslateSelectionHotkey());

    auto *speakSelectedText = new ShortcutItem(globalShortcuts);
    speakSelectedText->setDescription(tr("Speak selected text"));
    speakSelectedText->setIconName("media-playback-start");
    speakSelectedText->setShortcut(settings.speakSelectionHotkey());
    speakSelectedText->setDefaultShortcut(AppSettings::defaultSpeakSelectionHotkey());

    auto *speakTranslatedSelection = new ShortcutItem(globalShortcuts);
    speakTranslatedSelection->setDescription(tr("Speak translation of selected text"));
    speakTranslatedSelection->setIconName("media-playback-start");
    speakTranslatedSelection->setShortcut(settings.speakTranslatedSelectionHotkey());
    speakTranslatedSelection->setDefaultShortcut(AppSettings::defaultSpeakTranslatedSelectionHotkey());

    auto *stopSpeaking = new ShortcutItem(globalShortcuts);
    stopSpeaking->setDescription(tr("Stop text speaking"));
    stopSpeaking->setIconName("media-playback-stop");
    stopSpeaking->setShortcut(settings.stopSpeakingHotkey());
    stopSpeaking->setDefaultShortcut(AppSettings::defaultStopSpeakingHotkey());

    auto *showMainWindow = new ShortcutItem(globalShortcuts);
    showMainWindow->setDescription(tr("Stop text speaking"));
    showMainWindow->setIconName("media-playback-stop");
    showMainWindow->setShortcut(settings.showMainWindowHotkey());
    showMainWindow->setDefaultShortcut(AppSettings::defaultShowMainWindowHotkey());

    auto *copyTranslatedSelection = new ShortcutItem(globalShortcuts);
    copyTranslatedSelection->setDescription(tr("Translate selected text and copy to clipboard"));
    copyTranslatedSelection->setIconName("edit-copy");
    copyTranslatedSelection->setShortcut(settings.copyTranslatedSelectionHotkey());
    copyTranslatedSelection->setDefaultShortcut(AppSettings::defaultCopyTranslatedSelectionHotkey());

    // Window shortcuts
    auto *windowShortcuts = new ShortcutItem(this);
    windowShortcuts->setDescription("Main window");

    auto *translate = new ShortcutItem(windowShortcuts);
    translate->setDescription(tr("Translate"));
    translate->setIconName("go-next");
    translate->setShortcut(settings.translateHotkey());
    translate->setDefaultShortcut(AppSettings::defaultTranslateHotkey());

    auto *closeWindow = new ShortcutItem(windowShortcuts);
    closeWindow->setDescription(tr("Close window"));
    closeWindow->setIconName("application-exit");
    closeWindow->setShortcut(settings.closeWindowHotkey());
    closeWindow->setDefaultShortcut(AppSettings::defaultCloseWindowHotkey());

    // Source text shortcuts
    auto *sourceText = new ShortcutItem(windowShortcuts);
    sourceText->setDescription("Source text");

    auto *speakSource = new ShortcutItem(sourceText);
    speakSource->setDescription(tr("Play / pause text speaking"));
    speakSource->setIconName("media-playback-start");
    speakSource->setShortcut(settings.speakSourceHotkey());
    speakSource->setDefaultShortcut(AppSettings::defaultSpeakSourceHotkey());

    // Translation text shortcuts
    auto *translationText = new ShortcutItem(windowShortcuts);
    translationText->setDescription("Translation");

    auto *speakTranslation = new ShortcutItem(translationText);
    speakTranslation->setDescription(tr("Play / pause text speaking"));
    speakTranslation->setIconName("media-playback-start");
    speakTranslation->setShortcut(settings.speakTranslationHotkey());
    speakTranslation->setDefaultShortcut(AppSettings::defaultSpeakTranslationHotkey());

    auto *copyTranslation = new ShortcutItem(translationText);
    copyTranslation->setDescription(tr("Play / pause text speaking"));
    copyTranslation->setIconName("media-playback-start");
    copyTranslation->setShortcut(settings.copyTranslationHotkey());
    copyTranslation->setDefaultShortcut(AppSettings::defaultCopyTranslationHotkey());

    endResetModel();
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
    for (ShortcutItem *rootItem : m_rootItems)
        resetAllShortcuts(rootItem);
}

void ShortcutsModel::resetAllShortcuts(ShortcutItem *parent)
{
    parent->resetShortcut();
    for (int i = 0; i < parent->childCount(); ++i)
        resetAllShortcuts(parent->child(i));
}

void ShortcutsModel::updateShortcutText(ShortcutItem *item)
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
