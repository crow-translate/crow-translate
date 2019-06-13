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

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QAbstractItemModel>
#include <QVariant>

class ShortcutItem;
class AppSettings;

class ShortcutsModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ShortcutsModel)

    friend class ShortcutItem;

public:
    enum Columns {
        DescriptionColumn,
        ShortcutColumn
    };
    Q_ENUM(Columns)

    explicit ShortcutsModel(QObject *parent = nullptr);
    ~ShortcutsModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &) const override;

    void loadShortcuts(const AppSettings &settings);
    void saveShortcuts(AppSettings &settings) const;
    void resetAllShortcuts();

private:
    void resetAllShortcuts(ShortcutItem *parent);
    void updateShortcutText(ShortcutItem *item);
    QModelIndex index(ShortcutItem *item, int column) const;

    QVector<ShortcutItem *> m_rootItems;
};

#endif // SHORTCUTSMODEL_H
