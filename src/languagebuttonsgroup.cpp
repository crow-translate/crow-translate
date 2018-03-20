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

#include "languagebuttonsgroup.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDebug>

#include "qonlinetranslator.h"

LanguageButtonsGroup::LanguageButtonsGroup(QObject *parent, const QString &name) :
    QButtonGroup(parent)
{
    connect(this, static_cast<void (LanguageButtonsGroup::*)(int)>(&LanguageButtonsGroup::buttonReleased), &LanguageButtonsGroup::savePressedButton);
    m_name = name;
}

void LanguageButtonsGroup::swapChecked(LanguageButtonsGroup *first, LanguageButtonsGroup *second)
{
    // Save input language name
    QString inputLanguageName = first->checkedButton()->toolTip();

    // Insert new buttons
    first->insertLanguage(second->checkedButton()->toolTip());
    second->insertLanguage(inputLanguageName);
}

// Load buttons from settings
void LanguageButtonsGroup::loadSettings()
{
    // Load buttons text and tooltip
    QSettings settings;
    for (auto i=1; i<this->buttons().size(); i++)
    {
        QString languageCode = settings.value("Buttons/" + m_name + QString::number(i), "").toString();
        // Check if the code is set
        if (languageCode != "") {
            this->buttons().at(i)->setToolTip(languageCode);
            int index = QOnlineTranslator::LANGUAGE_SHORT_CODES.indexOf(languageCode);
            this->buttons().at(i)->setText(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(index)))); // Set name of the button from QStringList LANGUAGE_NAMES by index
            this->buttons().at(i)->setVisible(true);
        }
        else this->buttons().at(i)->setVisible(false);
    }

    // Load selected button
    this->buttons().at(settings.value("Buttons/" + m_name + "Pressed", 0).toInt())->setChecked(true);
}

// Shift text and tooltips of language buttons and insert new one
void LanguageButtonsGroup::insertLanguage(const QString &languageCode)
{
    // Exit the function if the current language already has a button
    for (auto i = 0; i<this->buttons().size(); i++) {
        if (languageCode == this->buttons().at(i)->toolTip()) {
            this->buttons().at(i)->setChecked(true);

            QSettings settings;
            settings.setValue("Buttons/" + m_name + "Pressed", i);
            return;
        }
    }

    // Shift buttons (... 2 -> 3, 1 -> 2)
    for (auto i = this->buttons().size()-1; i > 1; i--) {
        // Skip iteration, if previous button is not specified
        if (this->buttons().at(i-1)->toolTip() == "") continue;

        // Set values
        this->buttons().at(i)->setText(this->buttons().at(i-1)->text());
        this->buttons().at(i)->setToolTip(this->buttons().at(i-1)->toolTip());
        this->buttons().at(i)->setVisible(true);

        // Save settings
        QSettings settings;
        settings.setValue("Buttons/" + m_name + QString::number(i), this->buttons().at(i)->toolTip());
    }

    // Insert new language to first button
    int index = QOnlineTranslator::LANGUAGE_SHORT_CODES.indexOf(languageCode);
    this->buttons().at(1)->setText(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(index))));
    this->buttons().at(1)->setToolTip(languageCode);
    this->buttons().at(1)->setVisible(true);
    this->buttons().at(1)->setChecked(true);

    // Save first button and pressed button settings
    QSettings settings;
    settings.setValue("Buttons/" + m_name + QString::number(1), this->buttons().at(1)->toolTip());
    settings.setValue("Buttons/" + m_name + "Pressed", 1);
}

void LanguageButtonsGroup::setName(const QString &name)
{
    m_name = name;
}

void LanguageButtonsGroup::savePressedButton(const short &buttonIndex)
{
    QSettings settings;
    settings.setValue("Buttons/" + m_name + "Pressed", buttonIndex);
}
