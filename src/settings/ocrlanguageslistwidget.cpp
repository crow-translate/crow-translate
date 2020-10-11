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

#include "ocrlanguageslistwidget.h"

OcrLanguagesListWidget::OcrLanguagesListWidget(QWidget *parent)
    : QListWidget(parent)
{
}

void OcrLanguagesListWidget::addLanguages(const QStringList &labels) 
{
    for (const QString &label : labels) {
        auto *widgetItem = new QListWidgetItem(label, this);
        widgetItem->setCheckState(Qt::Unchecked);
    }
}

void OcrLanguagesListWidget::setCheckedLanguages(const QByteArray &languagesString)
{
    const QByteArrayList languages = languagesString.split('+');
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *widgetItem = item(i);
        if (languages.contains(widgetItem->text().toLocal8Bit()))
            widgetItem->setCheckState(Qt::Checked);
    }
}

QByteArray OcrLanguagesListWidget::checkedLanguagesString() const
{
    QByteArray languagesString;
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *widgetItem = item(i);
        if (widgetItem->checkState() == Qt::Checked) {
            if (!languagesString.isEmpty())
                languagesString += '+';
            languagesString += widgetItem->text().toLocal8Bit();
        }
    }
    return languagesString;
}
