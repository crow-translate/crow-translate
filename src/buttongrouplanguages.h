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

#ifndef BUTTONGROUPLANGUAGES_H
#define BUTTONGROUPLANGUAGES_H

#include <QButtonGroup>
#include <QPushButton>
#include <QToolButton>

class ButtonGroupLanguages : public QButtonGroup
{
    Q_OBJECT

public:
    explicit ButtonGroupLanguages(QObject *parent = Q_NULLPTR);

    static void swapChecked(ButtonGroupLanguages *first, ButtonGroupLanguages *second);

    void loadSettings();

    void insertLanguage(const short &languageIndex);

private:
    void saveId(const short &buttonIndex, const short &buttonId);

    short loadId(const short &buttonIndex);

    void savePressedId(const short &index);

    short loadPressedId();
};

#endif // BUTTONGROUPLANGUAGES_H
