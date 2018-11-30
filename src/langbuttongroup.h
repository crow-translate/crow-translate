/*
 *  Copyright © 2018 Gennady Chernyshchuk <genaloner@gmail.com>
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

#ifndef LANGBUTTONGROUP_H
#define LANGBUTTONGROUP_H

#include "qonlinetranslator.h"

#include <QButtonGroup>

class LangButtonGroup : public QButtonGroup
{
    Q_OBJECT

public:
    LangButtonGroup(QObject *parent = nullptr);

    void addButton(QAbstractButton *button);
    void loadLanguages();
    void loadLanguages(const LangButtonGroup *group);
    void insertLanguage(QOnlineTranslator::Language lang);
    void retranslate();

    QOnlineTranslator::Language checkedLanguage() const;
    QOnlineTranslator::Language language(int id) const;

    QString name() const;
    void setName(const QString &name);

signals:
    void buttonChecked(int id);
    void languageChanged(int id, QOnlineTranslator::Language lang);

public slots:
    void checkButton(int id);
    void setLanguage(int id, QOnlineTranslator::Language lang);

private:
    QString m_name;
};

#endif // LANGBUTTONGROUP_H
