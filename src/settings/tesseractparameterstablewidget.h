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

#ifndef TESSERACTPARAMETERSTABLEWIDGT_H
#define TESSERACTPARAMETERSTABLEWIDGT_H

#include <QTableWidget>

class TesseractParametersTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    TesseractParametersTableWidget(QWidget *parent = nullptr);

    void setParameters(const QMap<QString, QVariant> &parameters);
    QMap<QString, QVariant> parameters() const;
    bool validateParameters();
    void removeInvalidParameters();

public slots:
    void addParameter(const QString &key = {}, const QVariant &value = {}, bool edit = true);
    void removeCurrent();
};

#endif // TESSERACTPARAMETERSTABLEWIDGT_H
