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

#include "langbuttongroup.h"
#include "settings/appsettings.h"

#include <QAbstractButton>
#include <QButtonGroup>

LangButtonGroup::LangButtonGroup(GroupType type, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_group(new QButtonGroup(this))
{
    connect(m_group, qOverload<int, bool>(&QButtonGroup::buttonToggled), this, &LangButtonGroup::processButtonToggled);
}

void LangButtonGroup::loadLanguages(const LangButtonGroup *group)
{
    Q_ASSERT(group->m_group->buttons().size() == m_group->buttons().size());

    // Copy all languages from buttons
    for (int i = 0; i < m_group->buttons().size(); ++i)
        setLanguage(i, group->language(i));

    // Copy checked button
    m_group->button(group->m_group->checkedId())->setChecked(true);
}

void LangButtonGroup::loadLanguages(const AppSettings &settings)
{
    for (int i = 1; i < m_group->buttons().size(); ++i)
        setLanguage(i, settings.buttonLanguage(m_type, i));

    m_group->button(settings.checkedButton(m_type))->setChecked(true);
}

void LangButtonGroup::saveLanguages(AppSettings &settings)
{
    for (int i = 1; i < m_group->buttons().size(); ++i)
        settings.setButtonLanguage(m_type, i, language(i));

    settings.setCheckedButton(m_type, m_group->checkedId());
}

void LangButtonGroup::addButton(QAbstractButton *button)
{
    const int buttonId = m_group->buttons().count();

    m_group->addButton(button, m_group->buttons().count());
    if (buttonId == 0) {
       button->setText(tr("Auto"));
       button->setToolTip(tr("Auto"));
       button->setProperty(s_languageProperty, QOnlineTranslator::Auto); // Save language id in property
    }
}

void LangButtonGroup::addLanguage(QOnlineTranslator::Language lang)
{
    // Select auto button
    if (lang == QOnlineTranslator::Auto) {
        m_group->button(0)->setChecked(true);
        return;
    }

    // Exit the function if the current language already has a button
    for (int i = 1; i < m_group->buttons().size(); ++i) {
        if (lang == language(i)) {
            m_group->button(i)->setChecked(true);
            return;
        }
    }

    // Shift buttons (..., 3 <- 2, 2 <- 1)
    for (int i = m_group->buttons().size() - 1; i > 1; --i)
        setLanguage(i, language(i - 1));

    // Insert new language to first button
    setLanguage(1, lang);

    if (m_group->button(1)->isChecked())
        emit buttonChecked(1); // Emit signal, because first button has shifted to second
    else
        m_group->button(1)->setChecked(true);
}

QOnlineTranslator::Language LangButtonGroup::checkedLanguage() const
{
    return m_group->checkedButton()->property(s_languageProperty).value<QOnlineTranslator::Language>();
}

QOnlineTranslator::Language LangButtonGroup::previousCheckedLanguage() const
{
    return m_group->button(m_previousCheckedId)->property(s_languageProperty).value<QOnlineTranslator::Language>();
}

QOnlineTranslator::Language LangButtonGroup::language(int id) const
{
    return m_group->button(id)->property(s_languageProperty).value<QOnlineTranslator::Language>();
}

bool LangButtonGroup::isAutoButtonChecked()
{
    return m_group->checkedId() == 0;
}

void LangButtonGroup::retranslate()
{
    for (int i = 0; i < m_group->buttons().size(); ++i) {
        const QOnlineTranslator::Language lang = language(i);
        if (lang == QOnlineTranslator::NoLanguage)
            continue;

        const QString langName = QOnlineTranslator::languageString(lang);
        m_group->button(i)->setToolTip(langName);
        if (i != 0) {
            // Language button
            m_group->button(i)->setText(langName);
        } else {
            // Auto language button
            if (lang == QOnlineTranslator::Auto)
                m_group->button(i)->setText(tr("Auto"));
            else
                m_group->button(i)->setText(tr("Auto") + " (" + langName + ")");
        }
    }
}

QIcon LangButtonGroup::countryIcon(QOnlineTranslator::Language lang)
{
    switch (lang) {
    case QOnlineTranslator::Afrikaans:
    case QOnlineTranslator::Xhosa:
    case QOnlineTranslator::Zulu:
        return QIcon(":/icons/flags/za.svg");
    case QOnlineTranslator::Albanian:
        return QIcon(":/icons/flags/al.svg");
    case QOnlineTranslator::Amharic:
        return QIcon(":/icons/flags/et.svg");
    case QOnlineTranslator::Arabic:
    case QOnlineTranslator::LevantineArabic:
        return QIcon(":/icons/flags/eg.svg");
    case QOnlineTranslator::Armenian:
        return QIcon(":/icons/flags/am.svg");
    case QOnlineTranslator::Azerbaijani:
        return QIcon(":/icons/flags/az.svg");
    case QOnlineTranslator::Basque:
        return QIcon(":/icons/flags/eu.svg");
    case QOnlineTranslator::Belarusian:
        return QIcon(":/icons/flags/by.svg");
    case QOnlineTranslator::Bengali:
        return QIcon(":/icons/flags/bd.svg");
    case QOnlineTranslator::Bosnian:
    case QOnlineTranslator::Yiddish:
        return QIcon(":/icons/flags/ba.svg");
    case QOnlineTranslator::Bulgarian:
        return QIcon(":/icons/flags/bg.svg");
    case QOnlineTranslator::Catalan:
        return QIcon(":/icons/flags/ad.svg");
    case QOnlineTranslator::SimplifiedChinese:
    case QOnlineTranslator::TraditionalChinese:
    case QOnlineTranslator::Hmong:
    case QOnlineTranslator::Cantonese:
        return QIcon(":/icons/flags/cn.svg");
    case QOnlineTranslator::Croatian:
        return QIcon(":/icons/flags/hr.svg");
    case QOnlineTranslator::Czech:
        return QIcon(":/icons/flags/cz.svg");
    case QOnlineTranslator::Danish:
        return QIcon(":/icons/flags/dk.svg");
    case QOnlineTranslator::Dutch:
    case QOnlineTranslator::Frisian:
        return QIcon(":/icons/flags/nl.svg");
    case QOnlineTranslator::English:
        return QIcon(":/icons/flags/gb.svg");
    case QOnlineTranslator::Esperanto:
        return QIcon(":/icons/flags/eo.svg");
    case QOnlineTranslator::Estonian:
        return QIcon(":/icons/flags/ee.svg");
    case QOnlineTranslator::Fijian:
        return QIcon(":/icons/flags/fj.svg");
    case QOnlineTranslator::Filipino:
    case QOnlineTranslator::Cebuano:
    case QOnlineTranslator::Tagalog:
        return QIcon(":/icons/flags/ph.svg");
    case QOnlineTranslator::Finnish:
        return QIcon(":/icons/flags/fi.svg");
    case QOnlineTranslator::French:
    case QOnlineTranslator::Corsican:
        return QIcon(":/icons/flags/fr.svg");
    case QOnlineTranslator::Galician:
        return QIcon(":/icons/flags/es-ga.svg");
    case QOnlineTranslator::Georgian:
        return QIcon(":/icons/flags/ge.svg");
    case QOnlineTranslator::German:
        return QIcon(":/icons/flags/de.svg");
    case QOnlineTranslator::Greek:
        return QIcon(":/icons/flags/gr.svg");
    case QOnlineTranslator::HaitianCreole:
        return QIcon(":/icons/flags/ht.svg");
    case QOnlineTranslator::Hausa:
        return QIcon(":/icons/flags/ne.svg");
    case QOnlineTranslator::Hawaiian:
        return QIcon(":/icons/flags/es.svg");
    case QOnlineTranslator::Hebrew:
        return QIcon(":/icons/flags/il.svg");
    case QOnlineTranslator::Hindi:
    case QOnlineTranslator::Kannada:
    case QOnlineTranslator::Gujarati:
    case QOnlineTranslator::Malayalam:
    case QOnlineTranslator::Marathi:
    case QOnlineTranslator::Punjabi:
    case QOnlineTranslator::Telugu:
        return QIcon(":/icons/flags/in.svg");
    case QOnlineTranslator::Hungarian:
        return QIcon(":/icons/flags/hu.svg");
    case QOnlineTranslator::Icelandic:
        return QIcon(":/icons/flags/is.svg");
    case QOnlineTranslator::Igbo:
    case QOnlineTranslator::Yoruba:
        return QIcon(":/icons/flags/ng.svg");
    case QOnlineTranslator::Indonesian:
        return QIcon(":/icons/flags/id.svg");
    case QOnlineTranslator::Irish:
        return QIcon(":/icons/flags/ie.svg");
    case QOnlineTranslator::Italian:
        return QIcon(":/icons/flags/it.svg");
    case QOnlineTranslator::Japanese:
        return QIcon(":/icons/flags/jp.svg");
    case QOnlineTranslator::Javanese:
    case QOnlineTranslator::Sundanese:
        return QIcon(":/icons/flags/jw.svg");
    case QOnlineTranslator::Kazakh:
        return QIcon(":/icons/flags/kz.svg");
    case QOnlineTranslator::Khmer:
        return QIcon(":/icons/flags/kh.svg");
    case QOnlineTranslator::Klingon:
    case QOnlineTranslator::KlingonPlqaD:
        return QIcon(":/icons/flags/tlh.svg");
    case QOnlineTranslator::Korean:
        return QIcon(":/icons/flags/kr.svg");
    case QOnlineTranslator::Kurdish:
        return QIcon(":/icons/flags/iq.svg");
    case QOnlineTranslator::Kyrgyz:
        return QIcon(":/icons/flags/kg.svg");
    case QOnlineTranslator::Lao:
        return QIcon(":/icons/flags/la.svg");
    case QOnlineTranslator::Latin:
        return QIcon(":/icons/flags/va.svg");
    case QOnlineTranslator::Latvian:
        return QIcon(":/icons/flags/lv.svg");
    case QOnlineTranslator::Lithuanian:
        return QIcon(":/icons/flags/lt.svg");
    case QOnlineTranslator::Luxembourgish:
        return QIcon(":/icons/flags/lu.svg");
    case QOnlineTranslator::Macedonian:
        return QIcon(":/icons/flags/mk.svg");
    case QOnlineTranslator::Malagasy:
        return QIcon(":/icons/flags/mg.svg");
    case QOnlineTranslator::Malay:
        return QIcon(":/icons/flags/my.svg");
    case QOnlineTranslator::Maltese:
        return QIcon(":/icons/flags/mt.svg");
    case QOnlineTranslator::Maori:
        return QIcon(":/icons/flags/nz.svg");
    case QOnlineTranslator::Mongolian:
        return QIcon(":/icons/flags/mn.svg");
    case QOnlineTranslator::Myanmar:
        return QIcon(":/icons/flags/mm.svg");
    case QOnlineTranslator::Nepali:
        return QIcon(":/icons/flags/np.svg");
    case QOnlineTranslator::Norwegian:
        return QIcon(":/icons/flags/no.svg");
    case QOnlineTranslator::Chichewa:
        return QIcon(":/icons/flags/mw.svg");
    case QOnlineTranslator::Papiamento:
        return QIcon(":/icons/flags/aw.svg");
    case QOnlineTranslator::Pashto:
        return QIcon(":/icons/flags/af.svg");
    case QOnlineTranslator::Persian:
        return QIcon(":/icons/flags/ir.svg");
    case QOnlineTranslator::Polish:
        return QIcon(":/icons/flags/pl.svg");
    case QOnlineTranslator::Portuguese:
        return QIcon(":/icons/flags/pt.svg");
    case QOnlineTranslator::QueretaroOtomi:
    case QOnlineTranslator::YucatecMaya:
        return QIcon(":/icons/flags/mx.svg");
    case QOnlineTranslator::Romanian:
        return QIcon(":/icons/flags/ro.svg");
    case QOnlineTranslator::Russian:
    case QOnlineTranslator::Bashkir:
    case QOnlineTranslator::HillMari:
    case QOnlineTranslator::Mari:
    case QOnlineTranslator::Tatar:
    case QOnlineTranslator::Udmurt:
        return QIcon(":/icons/flags/ru.svg");
    case QOnlineTranslator::Samoan:
        return QIcon(":/icons/flags/ws.svg");
    case QOnlineTranslator::ScotsGaelic:
        return QIcon(":/icons/flags/gb-sct.svg");
    case QOnlineTranslator::SerbianCyrillic:
    case QOnlineTranslator::SerbianLatin:
        return QIcon(":/icons/flags/rs.svg");
    case QOnlineTranslator::Sesotho:
        return QIcon(":/icons/flags/ls.svg");
    case QOnlineTranslator::Shona:
        return QIcon(":/icons/flags/zw.svg");
    case QOnlineTranslator::Sindhi:
    case QOnlineTranslator::Urdu:
        return QIcon(":/icons/flags/pk.svg");
    case QOnlineTranslator::Sinhala:
        return QIcon(":/icons/flags/lk.svg");
    case QOnlineTranslator::Slovak:
        return QIcon(":/icons/flags/sk.svg");
    case QOnlineTranslator::Slovenian:
        return QIcon(":/icons/flags/sl.svg");
    case QOnlineTranslator::Somali:
        return QIcon(":/icons/flags/so.svg");
    case QOnlineTranslator::Spanish:
        return QIcon(":/icons/flags/es.svg");
    case QOnlineTranslator::Swahili:
        return QIcon(":/icons/flags/ke.svg");
    case QOnlineTranslator::Swedish:
        return QIcon(":/icons/flags/se.svg");
    case QOnlineTranslator::Tahitian:
        return QIcon(":/icons/flags/pf.svg");
    case QOnlineTranslator::Tajik:
        return QIcon(":/icons/flags/tj.svg");
    case QOnlineTranslator::Tamil:
        return QIcon(":/icons/flags/tk.svg");
    case QOnlineTranslator::Thai:
        return QIcon(":/icons/flags/th.svg");
    case QOnlineTranslator::Tongan:
        return QIcon(":/icons/flags/to.svg");
    case QOnlineTranslator::Turkish:
        return QIcon(":/icons/flags/tr.svg");
    case QOnlineTranslator::Ukrainian:
        return QIcon(":/icons/flags/ua.svg");
    case QOnlineTranslator::Uzbek:
        return QIcon(":/icons/flags/uz.svg");
    case QOnlineTranslator::Vietnamese:
        return QIcon(":/icons/flags/vn.svg");
    case QOnlineTranslator::Welsh:
        return QIcon(":/icons/flags/gb-wls.svg");
    default:
        return QIcon();
    }
}

void LangButtonGroup::checkAutoButton()
{
    checkButton(0);
}

void LangButtonGroup::checkButton(int id)
{
    m_group->button(id)->setChecked(true);
}

void LangButtonGroup::setLanguage(int id, QOnlineTranslator::Language lang)
{
    if (lang == language(id))
        return;

    m_group->button(id)->setProperty(s_languageProperty, lang); // Save language id in property

    if (lang != QOnlineTranslator::NoLanguage) {
        const QString languageName = QOnlineTranslator::languageString(lang);

        if (id != 0) {
            // Language button
            m_group->button(id)->setText(languageName);
            m_group->button(id)->setIcon(countryIcon(lang));
        } else {
            // Auto language button
            if (lang == QOnlineTranslator::Auto)
                m_group->button(id)->setText(tr("Auto"));
            else
                m_group->button(id)->setText(tr("Auto") + " (" + languageName + ")");
        }

        m_group->button(id)->setToolTip(languageName);
        m_group->button(id)->setVisible(true);
    } else {
        m_group->button(id)->setVisible(false);
    }

    emit languageChanged(id, lang);
}

void LangButtonGroup::processButtonToggled(int id, bool checked)
{
    if (!checked)
        m_previousCheckedId = id;
    else
        emit buttonChecked(id);
}
