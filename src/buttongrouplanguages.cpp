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

#include "buttongrouplanguages.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDebug>

#include "qonlinetranslator.h"

ButtonGroupLanguages::ButtonGroupLanguages(QObject *parent) :
    QButtonGroup(parent)
{
    connect(this, static_cast<void (ButtonGroupLanguages::*)(int)>(&ButtonGroupLanguages::buttonReleased), &ButtonGroupLanguages::savePressedId);
}

void ButtonGroupLanguages::swapChecked(ButtonGroupLanguages *first, ButtonGroupLanguages *second)
{
    // Save first checked button id
    short firstId = first->checkedId();

    // Insert new buttons
    first->insertLanguage(second->checkedId());
    second->insertLanguage(firstId);
}

// Load buttons from settings
void ButtonGroupLanguages::loadSettings()
{
    for (auto i=1; i<this->buttons().size(); i++)
    {
        const short id = loadId(i);
        // Check if the id is set
        if (id > 1) {
            this->buttons().at(i)->setText(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(id)))); // Set name of the button from QStringList LANGUAGE_NAMES by index
            this->setId(this->buttons().at(i), id);
            this->buttons().at(i)->setVisible(true);
        }
        else this->buttons().at(i)->setVisible(false);
    }
    // Get selected button id
    this->button(loadPressedId())->setChecked(true);
}

// Shift names and ids of language buttons and insert new one
void ButtonGroupLanguages::insertLanguage(const short &languageIndex)
{
    // Exit the function if the current language already has a button
    for (auto i = 0; i<this->buttons().size(); i++) {
        if (languageIndex == loadId(i)) {
            this->buttons().at(i)->setChecked(true);
            savePressedId(languageIndex);
            return;
        }
    }

    // Shift buttons (... 2 -> 3, 1 -> 2)
    for (auto i = this->buttons().size()-1; i > 1; i--) {
        // Skip iteration, if the index key is not specified
        if (loadId(i-1) <= 0) continue;

        // Set values
        this->buttons().at(i)->setText(this->buttons().at(i-1)->text());
        this->buttons().at(i)->setVisible(true);
        this->setId(this->buttons().at(i), this->id(this->buttons().at(i-1)));

        // Save settings
        saveId(i, this->id(this->buttons().at(i)));
    }

    // Insert new language to first button
    this->buttons().at(1)->setText(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(languageIndex))));
    this->buttons().at(1)->setVisible(true);
    this->buttons().at(1)->setChecked(true);
    this->setId(this->buttons().at(1), languageIndex);

    // Save first button and pressed button
    saveId(1, languageIndex);
    savePressedId(languageIndex);
}

//
// Functions to store settings via QSettings
//
void ButtonGroupLanguages::saveId(const short &buttonIndex, const short &buttonId)
{
    QString name = this->buttons().at(buttonIndex)->objectName();
    name.prepend("Buttons/");
    QSettings settings;
    settings.setValue(name, buttonId);
}

short ButtonGroupLanguages::loadId(const short &buttonIndex)
{
    QString name = this->buttons().at(buttonIndex)->objectName();
    name.prepend("Buttons/");
    QSettings settings;
    return settings.value(name, 0).toInt();
}

void ButtonGroupLanguages::savePressedId(const short &buttonIndex)
{
    QString name = this->objectName();
    name.prepend("Buttons/");
    QSettings settings;
    settings.setValue(name, buttonIndex);
}

short ButtonGroupLanguages::loadPressedId()
{
    QString name = this->objectName();
    name.prepend("Buttons/");
    QSettings settings;
    return settings.value(name, 0).toInt();
}
