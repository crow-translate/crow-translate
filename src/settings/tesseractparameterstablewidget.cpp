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

#include "tesseractparameterstablewidget.h"

#include <QLineEdit>
#include <QHeaderView>

TesseractParametersTableWidget::TesseractParametersTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    setColumnCount(2);
    setHorizontalHeaderLabels({ tr("Property"), tr("Value") });
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setVisible(false);
}

void TesseractParametersTableWidget::addParameter(const QString &key, const QVariant &value)
{
    auto *keyWidget = new QTableWidgetItem(key);
    auto *valueWidget = new QTableWidgetItem(value.toString());
    insertRow(rowCount());
    setItem(rowCount() - 1, 0, keyWidget);
    setItem(rowCount() - 1, 1, valueWidget);
}

void TesseractParametersTableWidget::addParameter()
{
    addParameter("", "");
}

void TesseractParametersTableWidget::setParameters(const QMap<QString, QVariant> &parameters)
{
    clearContents();
    setRowCount(0);
    for (auto it = parameters.cbegin(); it != parameters.cend(); ++it) {
        addParameter(it.key(), it.value());
    }
}

QMap<QString, QVariant> TesseractParametersTableWidget::parameters()
{
    removeInvalidParameters();
    QMap<QString, QVariant> parameters;
    for (int i = 0; i < rowCount(); ++i) {
        QString key = item(i, 0)->text();
        auto value = QVariant(item(i, 1)->text());
        parameters.insert(key, value);
    }
    return parameters;
}

// Return false if any key is missing a value or vice versa. Also focus the empty cell.
bool TesseractParametersTableWidget::validateParameters()
{
    for (int i = 0; i < rowCount(); ++i)
    {
        QTableWidgetItem *key = item(i, 0);
        QTableWidgetItem *value = item(i, 1);
        if (key->text().isEmpty() && !value->text().isEmpty()) {
            setCurrentItem(key);
            editItem(key);
            return false;
        } 
        if (!key->text().isEmpty() && value->text().isEmpty()) {
            setCurrentItem(value);
            editItem(value);
            return false;
        }
    }
    return true;
}

// Remove all rows with empty cells
void TesseractParametersTableWidget::removeInvalidParameters()
{
    for (int i = rowCount() - 1; i >= 0; --i) {
        QTableWidgetItem *key = item(i, 0);
        QTableWidgetItem *value = item(i, 1);
        if (key->text().isEmpty() || value->text().isEmpty()) {
            removeRow(i);
        }
    }
}
