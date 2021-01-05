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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QAbstractItemModel>

class ShortcutItem;
class AppSettings;

class ShortcutsModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ShortcutsModel)

public:
    enum Column {
        DescriptionColumn,
        ShortcutColumn
    };
    Q_ENUM(Column)

    explicit ShortcutsModel(QObject *parent = nullptr);
    ~ShortcutsModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void loadShortcuts(const AppSettings &settings);
    void saveShortcuts(AppSettings &settings) const;
    void resetAllShortcuts();

    void updateShortcut(ShortcutItem *item);
    void updateItem(ShortcutItem *item);

public slots:
    void setGlobalShortuctsEnabled(bool enabled);

private:
    QModelIndex index(ShortcutItem *item, int column) const;

    ShortcutItem *m_rootItem;
    ShortcutItem *m_globalShortcuts;

    // Global shortcuts
    ShortcutItem *m_translateSelectionShortcut;
    ShortcutItem *m_speakSelectionShortcut;
    ShortcutItem *m_speakTranslatedSelectionShortcut;
    ShortcutItem *m_stopSpeakingShortcut;
    ShortcutItem *m_showMainWindowShortcut;
    ShortcutItem *m_copyTranslatedSelectionShortcut;
    ShortcutItem *m_recognizeScreenAreaShortcut;
    ShortcutItem *m_translateScreenAreaShortcut;

    // Window shortcuts
    ShortcutItem *m_translateShortcut;
    ShortcutItem *m_swapShortcut;
    ShortcutItem *m_closeWindowShortcut;

    // Source text shortcuts
    ShortcutItem *m_speakSourceShortcut;

    // Translation text shortcuts
    ShortcutItem *m_speakTranslationShortcut;
    ShortcutItem *m_copyTranslationShortcut;
};

#endif // SHORTCUTSMODEL_H
