/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include <QFont>
#include <QMetaEnum>

ShortcutsModel::ShortcutsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new ShortcutItem(this))
{
    // Global shortcuts
    auto *globalShortcuts = new ShortcutItem(tr("Global"), m_rootItem);

    auto *translateSelectionShortcut = new ShortcutItem(tr("Translate selected text"), QStringLiteral("go-next"), globalShortcuts);
    translateSelectionShortcut->setDefaultShortcut(AppSettings::defaultTranslateSelectionShortcut());

    auto *speakSelection = new ShortcutItem(tr("Speak selected text"), QStringLiteral("media-playback-start"), globalShortcuts);
    speakSelection->setDefaultShortcut(AppSettings::defaultSpeakSelectionShortcut());

    auto *speakTranslatedSelectionShortcut = new ShortcutItem(tr("Speak translation of selected text"), QStringLiteral("media-playback-start"), globalShortcuts);
    speakTranslatedSelectionShortcut->setDefaultShortcut(AppSettings::defaultSpeakTranslatedSelectionShortcut());

    auto *stopSpeakingShortcut = new ShortcutItem(tr("Stop text speaking"), QStringLiteral("media-playback-stop"), globalShortcuts);
    stopSpeakingShortcut->setDefaultShortcut(AppSettings::defaultStopSpeakingShortcut());

    auto *showMainWindowShortcut = new ShortcutItem(tr("Show main window"), QStringLiteral("window"), globalShortcuts);
    showMainWindowShortcut->setDefaultShortcut(AppSettings::defaultShowMainWindowShortcut());

    auto *copyTranslatedSelectionShortcut = new ShortcutItem(tr("Translate selected text and copy to clipboard"), QStringLiteral("edit-copy"), globalShortcuts);
    copyTranslatedSelectionShortcut->setDefaultShortcut(AppSettings::defaultCopyTranslatedSelectionShortcut());

    auto *recognizeScreenAreaShortcut = new ShortcutItem(tr("Recognize text in screen area"), QStringLiteral("scanner"), globalShortcuts);
    recognizeScreenAreaShortcut->setDefaultShortcut(AppSettings::defaultRecognizeScreenAreaShortcut());

    auto *translateScreenAreaShortcut = new ShortcutItem(tr("Translate text in screen area"), QStringLiteral("scanner"), globalShortcuts);
    translateScreenAreaShortcut->setDefaultShortcut(AppSettings::defaultTranslateScreenAreaShortcut());

    // Window shortcuts
    auto *windowShortcuts = new ShortcutItem(tr("Main window"), m_rootItem);

    auto *translateShortcut = new ShortcutItem(tr("Translate"), QStringLiteral("go-next"), windowShortcuts);
    translateShortcut->setDefaultShortcut(AppSettings::defaultTranslateShortcut());

    auto *swapShortcut = new ShortcutItem(tr("Swap languages"), QStringLiteral("object-flip-horizontal"), windowShortcuts);
    swapShortcut->setDefaultShortcut(AppSettings::defaultSwapShortcut());

    auto *closeWindowShortcut = new ShortcutItem(tr("Close window"), QStringLiteral("application-exit"), windowShortcuts);
    closeWindowShortcut->setDefaultShortcut(AppSettings::defaultCloseWindowShortcut());

    // Source text shortcuts
    auto *sourceText = new ShortcutItem(tr("Source text"), windowShortcuts);

    auto *speakSourceShortcut = new ShortcutItem(tr("Speak / pause text speaking"), QStringLiteral("media-playback-start"), sourceText);
    speakSourceShortcut->setDefaultShortcut(AppSettings::defaultSpeakSourceShortcut());

    // Translation text shortcuts
    auto *translationText = new ShortcutItem(tr("Translation"), windowShortcuts);

    auto *speakTranslationShortcut = new ShortcutItem(tr("Speak / pause speaking"), QStringLiteral("media-playback-start"), translationText);
    speakTranslationShortcut->setDefaultShortcut(AppSettings::defaultSpeakTranslationShortcut());

    auto *copyTranslationShortcut = new ShortcutItem(tr("Copy to clipboard"), QStringLiteral("edit-copy"), translationText);
    copyTranslationShortcut->setDefaultShortcut(AppSettings::defaultCopyTranslationShortcut());
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
    if (childItem == nullptr)
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
    const auto *item = static_cast<const ShortcutItem *>(index.internalPointer());
    itemFlags.setFlag(Qt::ItemIsEnabled, item->isEnabled());

    return itemFlags;
}

void ShortcutsModel::loadShortcuts(const AppSettings &settings)
{
    // Global shortcuts
    m_rootItem->child(0)->child(0)->setShortcut(settings.translateSelectionShortcut());
    m_rootItem->child(0)->child(1)->setShortcut(settings.speakSelectionShortcut());
    m_rootItem->child(0)->child(2)->setShortcut(settings.speakTranslatedSelectionShortcut());
    m_rootItem->child(0)->child(3)->setShortcut(settings.stopSpeakingShortcut());
    m_rootItem->child(0)->child(4)->setShortcut(settings.showMainWindowShortcut());
    m_rootItem->child(0)->child(5)->setShortcut(settings.copyTranslatedSelectionShortcut());
    m_rootItem->child(0)->child(6)->setShortcut(settings.recognizeScreenAreaShortcut());
    m_rootItem->child(0)->child(7)->setShortcut(settings.translateScreenAreaShortcut());

    // Window shortcuts
    m_rootItem->child(1)->child(0)->setShortcut(settings.translateShortcut());
    m_rootItem->child(1)->child(1)->setShortcut(settings.swapShortcut());
    m_rootItem->child(1)->child(2)->setShortcut(settings.closeWindowShortcut());

    // Source text shortcuts
    m_rootItem->child(1)->child(3)->child(0)->setShortcut(settings.speakSourceShortcut());

    // Translation text shortcuts
    m_rootItem->child(1)->child(4)->child(0)->setShortcut(settings.speakTranslationShortcut());
    m_rootItem->child(1)->child(4)->child(1)->setShortcut(settings.copyTranslationShortcut());
}

void ShortcutsModel::saveShortcuts(AppSettings &settings) const
{
    // Global shortcuts
    settings.setTranslateSelectionShortcut(m_rootItem->child(0)->child(0)->shortcut());
    settings.setSpeakSelectionShortcut(m_rootItem->child(0)->child(1)->shortcut());
    settings.setSpeakTranslatedSelectionShortcut(m_rootItem->child(0)->child(2)->shortcut());
    settings.setStopSpeakingShortcut(m_rootItem->child(0)->child(3)->shortcut());
    settings.setShowMainWindowShortcut(m_rootItem->child(0)->child(4)->shortcut());
    settings.setCopyTranslatedSelectionShortcut(m_rootItem->child(0)->child(5)->shortcut());
    settings.setRecognizeScreenAreaShortcut(m_rootItem->child(0)->child(6)->shortcut());
    settings.setTranslateScreenAreaShortcut(m_rootItem->child(0)->child(7)->shortcut());

    // Window shortcuts
    settings.setTranslateShortcut(m_rootItem->child(1)->child(0)->shortcut());
    settings.setSwapShortcut(m_rootItem->child(1)->child(1)->shortcut());
    settings.setCloseWindowShortcut(m_rootItem->child(1)->child(2)->shortcut());

    // Source text shortcuts
    settings.setSpeakSourceShortcut(m_rootItem->child(1)->child(3)->child(0)->shortcut());

    // Translation text shortcuts
    settings.setSpeakTranslationShortcut(m_rootItem->child(1)->child(4)->child(0)->shortcut());
    settings.setCopyTranslationShortcut(m_rootItem->child(1)->child(4)->child(1)->shortcut());
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

void ShortcutsModel::updateItem(ShortcutItem *item)
{
    const QModelIndex modelIndexLeft = index(item, ShortcutColumn);
    const QModelIndex modelIndexRight = index(item, DescriptionColumn);
    emit dataChanged(modelIndexLeft, modelIndexRight, {Qt::DisplayRole});
}

QModelIndex ShortcutsModel::index(ShortcutItem *item, int column) const
{
    if (item == m_rootItem)
        return QModelIndex();

    return index(item->row(), column, index(item->parentItem(), column));
}
