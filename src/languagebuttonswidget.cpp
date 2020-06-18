 #include "languagebuttonswidget.h"
#include "ui_languagebuttonswidget.h"

#include "addlangdialog.h"

#include <QButtonGroup>
#include <QToolButton>

LanguageButtonsWidget::LanguageButtonsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LanguageButtonsWidget)
    , m_buttonGroup(new QButtonGroup)
{
    ui->setupUi(this);
    addButton(QOnlineTranslator::Auto);
    m_buttonGroup->button(s_autoButtonIndex)->setChecked(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_buttonGroup, &QButtonGroup::idToggled, this, &LanguageButtonsWidget::savePreviousToggledButton);
#else
    connect(m_buttonGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), this, &LanguageButtonsWidget::savePreviousToggledButton);
#endif
}

LanguageButtonsWidget::~LanguageButtonsWidget()
{
    delete ui;
}

QVector<QOnlineTranslator::Language> LanguageButtonsWidget::languages() const
{
    return m_languages;
}

void LanguageButtonsWidget::setLanguages(const QVector<QOnlineTranslator::Language> &languages)
{
    // Add or set new languages
    for (int i = 0; i < languages.size(); ++i) {
        // Use -1 to ignore "Auto" button
        if (i < m_buttonGroup->buttons().size() - 1) {
            if (m_languages[i] != languages[i])
                setButtonLanguage(m_buttonGroup->button(i), languages[i]);
        } else {
            addButton(languages[i]);
        }
    }

    m_languages = languages;

    // Delete extra buttons
    for (int i = languages.size(); languages.size() != m_buttonGroup->buttons().size() - 1; ++i) {
        QAbstractButton *button = m_buttonGroup->button(i);
        m_buttonGroup->removeButton(button);
        delete button;
    }

    emit languagesChanged(m_languages);
}

QOnlineTranslator::Language LanguageButtonsWidget::checkedLanguage() const
{
    return language(m_buttonGroup->checkedId());
}

QOnlineTranslator::Language LanguageButtonsWidget::previousCheckedLanguage() const
{
    return language(m_previousCheckedId);
}

QOnlineTranslator::Language LanguageButtonsWidget::language(int index) const
{
    if (index == s_autoButtonIndex)
        return m_autoLanguage;

    return m_languages[index];
}

bool LanguageButtonsWidget::checkLanguage(QOnlineTranslator::Language language)
{
    // Select auto button
    if (language == QOnlineTranslator::Auto) {
        checkAutoButton();
        return true;
    }

    // Exit the function if the current language already has a button
    for (int i = 0; i < m_languages.size(); ++i) {
        if (language == m_languages[i]) {
            checkButton(i);
            return true;
        }
    }

    return false;
}

bool LanguageButtonsWidget::isAutoButtonChecked() const
{
    return m_buttonGroup->checkedId() == s_autoButtonIndex;
}

void LanguageButtonsWidget::retranslate()
{
    for (int i = 0; i < m_languages.size(); ++i)
        setButtonLanguage(m_buttonGroup->button(i), m_languages[i]);
    setButtonLanguage(m_buttonGroup->button(s_autoButtonIndex), m_autoLanguage);
}

QIcon LanguageButtonsWidget::countryIcon(QOnlineTranslator::Language language)
{
    switch (language) {
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

void LanguageButtonsWidget::swapCurrentLanguages(LanguageButtonsWidget *first, LanguageButtonsWidget *second)
{
    // Backup first widget buttons properties
    const QOnlineTranslator::Language sourceLanguage = first->checkedLanguage();
    const bool isSourceAutoButtonChecked = first->isAutoButtonChecked();

    // Insert current translation language to the first widget
    if (second->isAutoButtonChecked())
        first->checkAutoButton();
    else
        first->addOrCheckLanguage(second->checkedLanguage());

    // Insert current source language to the second widget
    if (isSourceAutoButtonChecked)
        second->checkAutoButton();
    else
        second->addOrCheckLanguage(sourceLanguage);
}

void LanguageButtonsWidget::checkAutoButton()
{
    checkButton(s_autoButtonIndex);
}

void LanguageButtonsWidget::checkButton(int index)
{
    m_buttonGroup->button(index)->setChecked(true);
}

void LanguageButtonsWidget::addLanguage(QOnlineTranslator::Language language)
{
    Q_ASSERT_X(!m_languages.contains(language), "addLanguage", "Language already exists");

    m_languages.append(language);
    addButton(language);
    emit languageAdded(language);
}

void LanguageButtonsWidget::setAutoLanguage(QOnlineTranslator::Language language)
{
    if (m_autoLanguage == language)
        return;

    m_autoLanguage = language;
    setButtonLanguage(m_buttonGroup->button(s_autoButtonIndex), m_autoLanguage);
    emit autoLanguageChanged(m_autoLanguage);
}

void LanguageButtonsWidget::editLanguages()
{
    AddLangDialog langDialog(languages());
    if (langDialog.exec() == QDialog::Accepted)
        setLanguages(langDialog.languages());
}

void LanguageButtonsWidget::savePreviousToggledButton(int index, bool checked)
{
    if (!checked)
        m_previousCheckedId = index;
    else
        emit buttonChecked(index);
}

void LanguageButtonsWidget::addOrCheckLanguage(QOnlineTranslator::Language language)
{
    if (checkLanguage(language))
        return;

    addLanguage(language);
    m_buttonGroup->buttons().last()->setChecked(true);
}

void LanguageButtonsWidget::addButton(QOnlineTranslator::Language language)
{
    auto *button = new QToolButton;
    button->setCheckable(true);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Use special index for "Auto" button to count all other languages from 0
    m_buttonGroup->addButton(button, language == QOnlineTranslator::Auto ? s_autoButtonIndex : m_buttonGroup->buttons().size() - 1);

    setButtonLanguage(button, language);

    // Insert all languages after "Edit" button
    ui->languagesLayout->insertWidget(ui->languagesLayout->count() - 1, button);
}

void LanguageButtonsWidget::setButtonLanguage(QAbstractButton *button, QOnlineTranslator::Language language)
{
    const QString languageName = QOnlineTranslator::languageString(language);
    if (button == m_buttonGroup->button(s_autoButtonIndex)) {
        if (language == QOnlineTranslator::Auto)
            button->setText(tr("Auto"));
        else
            button->setText(tr("Auto") + " (" + languageName + ")");
    } else {
        button->setText(languageName);
        button->setIcon(countryIcon(language));
    }

    button->setToolTip(languageName);
}
