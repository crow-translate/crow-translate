 #include "languagebuttonswidget.h"
#include "ui_languagebuttonswidget.h"

#include "addlanguagedialog.h"

#include <QButtonGroup>
#include <QTimer>
#include <QToolButton>
#include <QScreen>
#include <QMessageBox>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QWindow>
#endif

using namespace std::chrono_literals;

LanguageButtonsWidget::LanguageButtonsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LanguageButtonsWidget)
    , m_buttonGroup(new QButtonGroup)
{
    ui->setupUi(this);
    addButton(QOnlineTranslator::Auto);
    m_buttonGroup->button(s_autoButtonId)->setChecked(true);
    setWindowWidthCheckEnabled(true);
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
    if (m_languages == languages)
        return;

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

QOnlineTranslator::Language LanguageButtonsWidget::language(int id) const
{
    if (id == s_autoButtonId)
        return m_autoLang;

    return m_languages[id];
}

bool LanguageButtonsWidget::checkLanguage(QOnlineTranslator::Language lang)
{
    // Select auto button
    if (lang == QOnlineTranslator::Auto) {
        checkAutoButton();
        return true;
    }

    // Exit the function if the current language already has a button
    for (int i = 0; i < m_languages.size(); ++i) {
        if (lang == m_languages[i]) {
            checkButton(i);
            return true;
        }
    }

    return false;
}

void LanguageButtonsWidget::setLanguageFormat(AppSettings::LanguageFormat languageFormat)
{
    if (m_languageFormat == languageFormat)
        return;

    m_languageFormat = languageFormat;
    retranslate();
}

int LanguageButtonsWidget::checkedId() const
{
    return m_buttonGroup->checkedId();
}

bool LanguageButtonsWidget::isAutoButtonChecked() const
{
    return m_buttonGroup->checkedId() == s_autoButtonId;
}

void LanguageButtonsWidget::retranslate()
{
    for (int i = 0; i < m_languages.size(); ++i)
        setButtonLanguage(m_buttonGroup->button(i), m_languages[i]);
    setButtonLanguage(m_buttonGroup->button(s_autoButtonId), m_autoLang);
}

QIcon LanguageButtonsWidget::countryIcon(QOnlineTranslator::Language lang)
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
        return QIcon(":/icons/flags/es-pv.svg");
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
    case QOnlineTranslator::Cantonese:
    case QOnlineTranslator::Hmong:
    case QOnlineTranslator::SimplifiedChinese:
    case QOnlineTranslator::TraditionalChinese:
    case QOnlineTranslator::Uighur:
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
        return QIcon(":/icons/flags/esperanto.svg");
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
        return QIcon(":/icons/flags/us.svg");
    case QOnlineTranslator::Hebrew:
        return QIcon(":/icons/flags/il.svg");
    case QOnlineTranslator::Gujarati:
    case QOnlineTranslator::Hindi:
    case QOnlineTranslator::Kannada:
    case QOnlineTranslator::Malayalam:
    case QOnlineTranslator::Marathi:
    case QOnlineTranslator::Oriya:
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
    case QOnlineTranslator::Javanese:
        return QIcon(":/icons/flags/id.svg");
    case QOnlineTranslator::Irish:
        return QIcon(":/icons/flags/ie.svg");
    case QOnlineTranslator::Italian:
        return QIcon(":/icons/flags/it.svg");
    case QOnlineTranslator::Japanese:
        return QIcon(":/icons/flags/jp.svg");
    case QOnlineTranslator::Sundanese:
        return QIcon(":/icons/flags/sd.svg");
    case QOnlineTranslator::Kazakh:
        return QIcon(":/icons/flags/kz.svg");
    case QOnlineTranslator::Khmer:
        return QIcon(":/icons/flags/kh.svg");
    case QOnlineTranslator::Kinyarwanda:
        return QIcon(":/icons/flags/rw.svg");
    case QOnlineTranslator::Klingon:
    case QOnlineTranslator::KlingonPlqaD:
        return QIcon(":/icons/flags/fiction/klingon.svg");
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
    case QOnlineTranslator::Turkmen:
        return QIcon(":/icons/flags/tm.svg");
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
    const QOnlineTranslator::Language sourceLang = first->checkedLanguage();
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
        second->addOrCheckLanguage(sourceLang);
}

void LanguageButtonsWidget::checkAutoButton()
{
    checkButton(s_autoButtonId);
}

void LanguageButtonsWidget::checkButton(int id)
{
    m_buttonGroup->button(id)->setChecked(true);
}

void LanguageButtonsWidget::addLanguage(QOnlineTranslator::Language lang)
{
    Q_ASSERT_X(!m_languages.contains(lang), "addLanguage", "Language already exists");

    m_languages.append(lang);
    addButton(lang);
    emit languageAdded(lang);
}

void LanguageButtonsWidget::setAutoLanguage(QOnlineTranslator::Language lang)
{
    if (m_autoLang == lang)
        return;

    m_autoLang = lang;
    setButtonLanguage(m_buttonGroup->button(s_autoButtonId), m_autoLang);
    emit autoLanguageChanged(m_autoLang);
}

void LanguageButtonsWidget::editLanguages()
{
    AddLanguageDialog langDialog(languages());
    if (langDialog.exec() == QDialog::Accepted)
        setLanguages(langDialog.languages());
}

void LanguageButtonsWidget::savePreviousToggledButton(int id, bool checked)
{
    if (!checked)
        m_previousCheckedId = id;
    else
        emit buttonChecked(id);
}

void LanguageButtonsWidget::checkAvailableScreenWidth()
{
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    if (isWindowWidthFitScreen())
        return;

    // Try resize first
    window()->resize(window()->minimumWidth(), window()->height());

    if (isWindowWidthFitScreen())
        return;

    QMessageBox message;
    message.setIcon(QMessageBox::Information);
    message.setText(tr("Window width is larger than screen due to the languages on the panel."));
    message.setInformativeText(tr("Please reduce added languages."));
    if (message.exec() == QMessageBox::Ok) {
        const int languagesCountBefore = m_languages.size();
        // Temporary disable connection to this slot to trigger it manually after resize
        setWindowWidthCheckEnabled(false);
        editLanguages();
        setWindowWidthCheckEnabled(true);

        if (m_languages.size() < languagesCountBefore)
            minimizeWindowWidth(); // For unknown reason QWindow::minimumWidthChanged is not emited in this case, so wait for changes manually
        else
            checkAvailableScreenWidth();
    }
}

void LanguageButtonsWidget::minimizeWindowWidth()
{
    if (window()->width() == window()->minimumWidth()) {
        QTimer::singleShot(100ms, this, &LanguageButtonsWidget::minimizeWindowWidth);
        return;
    }

    window()->resize(window()->minimumWidth(), window()->height());
    checkAvailableScreenWidth();
}

void LanguageButtonsWidget::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        if (m_languageFormat != AppSettings::IsoCode)
            retranslate();
        break;
    default:
        QWidget::changeEvent(event);
    }
}

void LanguageButtonsWidget::setWindowWidthCheckEnabled(bool enable) const
{
    if (enable)
        connect(this, &LanguageButtonsWidget::languagesChanged, this, &LanguageButtonsWidget::checkAvailableScreenWidth, Qt::QueuedConnection);
    else
        disconnect(this, &LanguageButtonsWidget::languagesChanged, this, &LanguageButtonsWidget::checkAvailableScreenWidth);
}

void LanguageButtonsWidget::addOrCheckLanguage(QOnlineTranslator::Language lang)
{
    if (checkLanguage(lang))
        return;

    addLanguage(lang);
    m_buttonGroup->buttons().last()->setChecked(true);
}

void LanguageButtonsWidget::addButton(QOnlineTranslator::Language lang)
{
    auto *button = new QToolButton;
    button->setCheckable(true);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); // To make the same size for all buttons (without it "Auto" button can look different)

    // Use special id for "Auto" button to count all other languages from 0
    m_buttonGroup->addButton(button, lang == QOnlineTranslator::Auto ? s_autoButtonId : m_buttonGroup->buttons().size() - 1);

    setButtonLanguage(button, lang);

    // Insert all languages after "Edit" button
    ui->languagesLayout->insertWidget(ui->languagesLayout->count() - 1, button);
}

void LanguageButtonsWidget::setButtonLanguage(QAbstractButton *button, QOnlineTranslator::Language lang)
{
    const QString langName = languageString(lang);
    if (button == m_buttonGroup->button(s_autoButtonId)) {
        if (lang == QOnlineTranslator::Auto)
            button->setText(tr("Auto"));
        else
            button->setText(tr("Auto") + " (" + langName + ")");
    } else {
        button->setText(langName);
        button->setIcon(countryIcon(lang));
    }

    button->setToolTip(langName);
}

bool LanguageButtonsWidget::isWindowWidthFitScreen()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return window()->frameGeometry().width() <= screen()->availableGeometry().width();
#else
    if (!window()->windowHandle())
        return true;
    return window()->frameGeometry().width() <= window()->windowHandle()->screen()->availableGeometry().width();
#endif
}

QString LanguageButtonsWidget::languageString(QOnlineTranslator::Language language)
{
    switch (m_languageFormat) {
    case AppSettings::FullName:
        return QOnlineTranslator::languageName(language);
    case AppSettings::IsoCode:
        return QOnlineTranslator::languageCode(language);
    default:
        Q_UNREACHABLE();
    }
}
