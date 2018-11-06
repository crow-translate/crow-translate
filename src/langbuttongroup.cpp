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

#include "langbuttongroup.h"

#include <QAbstractButton>

#include "appsettings.h"

LangButtonGroup::LangButtonGroup(QObject *parent) :
    QButtonGroup (parent)
{
    // Setup "Checked" signal
    connect(this, qOverload<int, bool>(&LangButtonGroup::buttonToggled), [&](int id, bool checked) {
        if (checked)
            emit buttonChecked(id);
    });
}

void LangButtonGroup::addButton(QAbstractButton *button)
{
    QButtonGroup::addButton(button, buttons().count());
}

void LangButtonGroup::loadLanguages()
{
    // Load buttons data from settings
    AppSettings settings;
    setAtributes(0, QOnlineTranslator::Auto);
    for (int i = 1; i < buttons().size(); ++i)
        setAtributes(i, settings.buttonLanguage(this, i));

    // Load checked button
    button(settings.checkedButton(this))->setChecked(true);
}

void LangButtonGroup::loadLanguages(const LangButtonGroup *group)
{
    // Check group sizes
    if (group->buttons().size() != buttons().size()) {
        qDebug() << tr("Different number of buttons in copied groups");
        return;
    }

    // Copy all languages from buttons
    for (int i = 0; i < buttons().size(); ++i) {
        if (group->language(i) != QOnlineTranslator::NoLanguage) {
            button(i)->setText(group->button(i)->text());
            button(i)->setToolTip(group->button(i)->toolTip());
            button(i)->setIcon(group->button(i)->icon());
            button(i)->setProperty("Lang", group->language(i));
            button(i)->setVisible(true);
        } else {
            button(i)->setVisible(false);
        }
    }

    // Copy checked button
    button(group->checkedId())->setChecked(true);
}

void LangButtonGroup::insertLanguage(QOnlineTranslator::Language lang)
{
    // Exit the function if the current language already has a button
    for (int i = 0; i < buttons().size(); ++i) {
        if (lang == language(i)) {
            button(i)->setChecked(true);
            return;
        }
    }

    // Shift buttons (..., 3 <- 2, 2 <- 1)
    AppSettings settings;
    for (int i = buttons().size() - 1; i > 1; --i) {
        // Skip iteration, if previous button is not specified
        if (language(i - 1) == QOnlineTranslator::NoLanguage)
            continue;

        setAtributes(i, language(i - 1));

        // Save settings
        settings.setButtonLanguage(this, i, language(i));
    }

    // Shift checked button in the settings
    if (checkedId() != 0 && checkedId() != 3)
        settings.setCheckedButton(this, settings.checkedButton(this) + 1);

    // Insert new language to first button
    setAtributes(1, lang);

    // Save first button settings
    settings.setButtonLanguage(this, 1, lang);

    if (button(1)->isChecked())
        emit buttonChecked(1); // Emit signal, because first button has shifted to second
    else
        button(1)->setChecked(true);
}

QOnlineTranslator::Language LangButtonGroup::checkedLanguage() const
{
    return checkedButton()->property("Lang").value<QOnlineTranslator::Language>();
}

QOnlineTranslator::Language LangButtonGroup::language(int id) const
{
    return button(id)->property("Lang").value<QOnlineTranslator::Language>();
}

QString LangButtonGroup::name() const
{
    return m_name;
}

void LangButtonGroup::setName(const QString &name)
{
    m_name = name;
}

void LangButtonGroup::checkButton(int id)
{
    button(id)->setChecked(true);
}

void LangButtonGroup::setLanguage(int id, QOnlineTranslator::Language lang)
{
    // Do nothing if the language is already set
    if (language(id) != lang) {
        setAtributes(id, lang);
    }
}

void LangButtonGroup::setAtributes(int id, QOnlineTranslator::Language lang)
{
    button(id)->setProperty("Lang", lang);
    if (lang != QOnlineTranslator::NoLanguage) {
        // Set button appearance
        button(id)->setVisible(true);
        button(id)->setToolTip(QOnlineTranslator::languageString(lang));
        if (id != 0) {
            // Language buttomn
            button(id)->setText(QOnlineTranslator::languageString(lang));
            button(id)->setIcon(QIcon(":/icons/flags/" + QOnlineTranslator::languageCode(lang) + ".svg"));
        } else {
            // Auto language button
            if (lang == QOnlineTranslator::Auto)
                button(id)->setText(tr("Auto"));
            else
                button(id)->setText(tr("Auto") + " (" + QOnlineTranslator::languageString(lang) + ")");
        }
    } else {
        // Hide button
        button(id)->setVisible(false);
    }

    emit languageChanged(id, lang);
}
