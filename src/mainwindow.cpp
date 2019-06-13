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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "popupwindow.h"
#include "addlangdialog.h"
#include "langbuttongroup.h"
#include "playerbuttons.h"
#include "qhotkey.h"
#include "singleapplication.h"
#include "settings/settingsdialog.h"
#include "settings/appsettings.h"
#if defined(Q_OS_WIN)
#include "updaterwindow.h"

#include <QMimeData>
#include <QThread>
#include <Windows.h>
#endif
#include <QClipboard>
#include <QShortcut>
#include <QNetworkProxy>
#include <QMessageBox>
#include <QTimer>
#include <QMenu>
#include <QMediaPlaylist>

constexpr int autotranslateDelay = 500; // Automatic translation delay when changing source text

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Show a message that the application is already running
    connect(qobject_cast<SingleApplication*>(SingleApplication::instance()), &SingleApplication::instanceStarted, this, &MainWindow::showAppRunningMessage);

    // Translator
    m_translator = new QOnlineTranslator(this);

    // Text speaking
    m_sourcePlayerButtons = new PlayerButtons(ui->playSourceButton, ui->stopSourceButton, this);
    m_translationPlayerButtons = new PlayerButtons(ui->playTranslationButton, ui->stopTranslationButton, this);
    m_sourcePlayerButtons->setMediaPlayer(new QMediaPlayer);
    m_translationPlayerButtons->setMediaPlayer(new QMediaPlayer);
    connect(ui->sourceEdit, &QPlainTextEdit::textChanged, m_sourcePlayerButtons, &PlayerButtons::stop);
    connect(m_sourcePlayerButtons, &PlayerButtons::playerDataRequested, [&](QMediaPlaylist *playlist) {
       setPlayingText(playlist, ui->sourceEdit->toPlainText(), m_sourceLangButtons->checkedLanguage());
    });
    connect(m_translationPlayerButtons, &PlayerButtons::playerDataRequested, [&](QMediaPlaylist *playlist) {
       setPlayingText(playlist, m_translator->translation(), m_translationLangButtons->checkedLanguage());
    });

    // Shortcuts
    m_translateSelectionHotkey = new QHotkey(this);
    m_playSelectionHotkey = new QHotkey(this);
    m_playTranslatedSelectionHotkey = new QHotkey(this);
    m_stopSpeakingHotkey = new QHotkey(this);
    m_showMainWindowHotkey = new QHotkey(this);
    m_copyTranslatedSelectionHotkey = new QHotkey(this);
    m_closeWindowsShortcut = new QShortcut(this);
    connect(m_translateSelectionHotkey, &QHotkey::activated, this, &MainWindow::translateSelectedText);
    connect(m_playSelectionHotkey, &QHotkey::activated, this, &MainWindow::playSelection);
    connect(m_playTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::playTranslatedSelection);
    connect(m_showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::activate);
    connect(m_copyTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::copyTranslatedSelection);
    connect(m_closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);
    connect(m_stopSpeakingHotkey, &QHotkey::activated, m_sourcePlayerButtons, &PlayerButtons::stop);
    connect(m_stopSpeakingHotkey, &QHotkey::activated, m_translationPlayerButtons, &PlayerButtons::stop);

    // Source button group
    const AppSettings settings;
    m_sourceLangButtons = new LangButtonGroup(LangButtonGroup::Source, this);
    m_sourceLangButtons->addButton(ui->autoSourceButton);
    m_sourceLangButtons->addButton(ui->firstSourceButton);
    m_sourceLangButtons->addButton(ui->secondSourceButton);
    m_sourceLangButtons->addButton(ui->thirdSourceButton);
    m_sourceLangButtons->loadLanguages(settings);

    // Translation button group
    m_translationLangButtons = new LangButtonGroup(LangButtonGroup::Translation, this);
    m_translationLangButtons->addButton(ui->autoTranslationButton);
    m_translationLangButtons->addButton(ui->firstTranslationButton);
    m_translationLangButtons->addButton(ui->secondTranslationButton);
    m_translationLangButtons->addButton(ui->thirdTranslationButton);
    m_translationLangButtons->loadLanguages(settings);

    // Toggle language logic
    connect(m_translationLangButtons, &LangButtonGroup::buttonChecked, [&](int id) {
        checkLanguageButton(m_translationLangButtons, m_sourceLangButtons, id);
    });
    connect(m_sourceLangButtons, &LangButtonGroup::buttonChecked, [&](int id) {
        checkLanguageButton(m_sourceLangButtons, m_translationLangButtons, id);
    });

    // System tray icon
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    m_trayMenu->addAction(QIcon::fromTheme("dialog-object-properties"), tr("Settings"), this, &MainWindow::on_settingsButton_clicked);
    m_trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), SingleApplication::instance(), &SingleApplication::quit);
    m_trayIcon = new TrayIcon(this);
    m_trayIcon->setContextMenu(m_trayMenu);

    // Timer for automatic translation
    m_translateTimer = new QTimer(this);
    m_translateTimer->setSingleShot(true);
    connect(m_translateTimer, &QTimer::timeout, this, &MainWindow::on_translateButton_clicked);

    // Get UI language for translation
    m_uiLang = QOnlineTranslator::language(settings.locale());

    // Load app settings
    loadSettings(settings);

    // Load main window settings
    ui->autoTranslateCheckBox->setChecked(settings.isAutoTranslateEnabled());
    ui->engineComboBox->setCurrentIndex(settings.currentEngine());
    restoreGeometry(settings.mainWindowGeometry());

#if defined(Q_OS_WIN)
    // Check date for updates
    const AppSettings::Interval updateInterval = settings.checkForUpdatesInterval();
    QDate checkDate = settings.lastUpdateCheckDate();
    switch (updateInterval) {
    case AppSettings::Day:
        checkDate = checkDate.addDays(1);
        break;
    case AppSettings::Week:
        checkDate = checkDate.addDays(7);
        break;
    case AppSettings::Month:
        checkDate = checkDate.addMonths(1);
        break;
    case AppSettings::Never:
        return;
    }

    if (QDate::currentDate() >= checkDate) {
        auto *release = new QGitTag(this);
        connect(release, &QGitTag::requestFinished, this, &MainWindow::checkForUpdates);
        release->get("Shatur95", "crow-translate");
    }
#endif
}

MainWindow::~MainWindow()
{
    AppSettings settings;
    settings.setMainWindowGeometry(saveGeometry());
    settings.setAutoTranslateEnabled(ui->autoTranslateCheckBox->isChecked());
    settings.setCurrentEngine(static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex()));

    m_sourceLangButtons->saveLanguages(settings);
    m_translationLangButtons->saveLanguages(settings);
    delete ui;
}

void MainWindow::activate()
{
    show();
    activateWindow();
    raise();
}

QComboBox *MainWindow::engineCombobox()
{
    return ui->engineComboBox;
}

QTextEdit *MainWindow::translationEdit()
{
    return ui->translationEdit;
}

QToolButton *MainWindow::addSourceLangButton()
{
    return ui->addSourceLangButton;
}

QToolButton *MainWindow::addTranslationLangButton()
{
    return ui->addTranslationLangButton;
}

QToolButton *MainWindow::swapButton()
{
    return ui->swapButton;
}

QToolButton *MainWindow::copySourceButton()
{
    return ui->copySourceButton;
}

QToolButton *MainWindow::copyTranslationButton()
{
    return ui->copyTranslationButton;
}

QToolButton *MainWindow::copyAllTranslationButton()
{
    return ui->copyAllTranslationButton;
}

LangButtonGroup *MainWindow::sourceLangButtons()
{
    return m_sourceLangButtons;
}

LangButtonGroup *MainWindow::translationLangButtons()
{
    return m_translationLangButtons;
}

PlayerButtons *MainWindow::sourcePlayerButtons()
{
    return m_sourcePlayerButtons;
}

PlayerButtons *MainWindow::translationPlayerButtons()
{
    return m_translationPlayerButtons;
}

void MainWindow::on_translateButton_clicked()
{
    m_translateTimer->stop();

    if (ui->sourceEdit->toPlainText().isEmpty()) {
        ui->translationEdit->clear();
        m_translationLangButtons->setLanguage(0, QOnlineTranslator::Auto);
        return;
    }

    // Disable the translation button to prevent re-pressing
    ui->translateButton->setEnabled(false);

    // Stop translation speaking
    m_translationPlayerButtons->stop();

    // Source Language
    QOnlineTranslator::Language sourceLang;
    if (ui->autoSourceButton->isChecked())
        sourceLang = QOnlineTranslator::Auto;
    else
        sourceLang = m_sourceLangButtons->checkedLanguage();

    // Translation language
    const AppSettings settings;
    QOnlineTranslator::Language translationLang;
    if (ui->autoTranslationButton->isChecked()) {
        // Use primary target language from settings
        translationLang = settings.primaryLanguage();
        if (translationLang == QOnlineTranslator::Auto)
            translationLang = m_uiLang;
        if (translationLang == sourceLang) {
            // If primary language is equal to source language, than use secondary language
            translationLang = settings.secondaryLanguage();
            if (translationLang == QOnlineTranslator::Auto)
                translationLang = m_uiLang;
        }
    } else {
        translationLang = m_translationLangButtons->checkedLanguage();
    }

    // Get translation
    if (!translate(translationLang, sourceLang))
        return;

    // Re-translate to a secondary or a primary language if the autodetected source language and the source language are the same
    if (ui->autoTranslationButton->isChecked() && m_translator->sourceLanguage() == m_translator->translationLanguage()) {
        QOnlineTranslator::Language primaryLanguage = settings.primaryLanguage();
        QOnlineTranslator::Language secondaryLanguage = settings.secondaryLanguage();
        if (primaryLanguage == QOnlineTranslator::Auto)
            primaryLanguage = m_uiLang;
        if (secondaryLanguage == QOnlineTranslator::Auto)
            secondaryLanguage = m_uiLang;

        // Select primary or secondary language
        if (translationLang == primaryLanguage)
            translationLang = secondaryLanguage;
        else
            translationLang = primaryLanguage;

        // Get translation
        if (!translate(translationLang, sourceLang))
            return;
    }

    // Display languages on "Auto" buttons.
    if (ui->autoSourceButton->isChecked()) {
        m_sourceLangButtons->setLanguage(0, m_translator->sourceLanguage());
        connect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    }

    if (ui->autoTranslationButton->isChecked())
        m_translationLangButtons->setLanguage(0, m_translator->translationLanguage());
    else
        m_translationLangButtons->setLanguage(0, QOnlineTranslator::Auto);

    // Translation
    ui->translationEdit->setHtml(m_translator->translation().toHtmlEscaped().replace("\n", "<br>"));

    // Translit
    if (!m_translator->translationTranslit().isEmpty())
        ui->translationEdit->append("<font color=\"grey\"><i>/" + m_translator->translationTranslit().replace("\n", "/<br>/") + "/</i></font>");
    if (!m_translator->sourceTranslit().isEmpty())
        ui->translationEdit->append("<font color=\"grey\"><i><b>(" + m_translator->sourceTranslit().replace("\n", "/<br>/") + ")</b></i></font>");

    // Transcription
    if (!m_translator->sourceTranscription().isEmpty())
        ui->translationEdit->append("<font color=\"grey\">[" + m_translator->sourceTranscription() + "]</font>");

    ui->translationEdit->append(""); // Add new line before translation options

    // Translation options
    if (!m_translator->translationOptions().isEmpty()) {
        ui->translationEdit->append("<font color=\"grey\"><i>" + m_translator->source() + "</i> – " + tr("translation options:") + "</font>");

        // Print words for each type of speech
        foreach (const QOption &option, m_translator->translationOptions()) {
            ui->translationEdit->append("<b>" + option.typeOfSpeech() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            ui->translationEdit->textCursor().setBlockFormat(indent);

            for (int i = 0; i <  option.count(); ++i) {
                // Show word gender
                QString wordLine;
                if (!option.gender(i).isEmpty())
                    wordLine.append("<i>" + option.gender(i) + "</i> ");

                // Show Word
                wordLine.append(option.word(i));

                // Show word meaning
                if (!option.translations(i).isEmpty()) {
                    wordLine.append(": ");
                    wordLine.append("<font color=\"grey\"><i>");
                    wordLine.append(option.translations(i));
                    wordLine.append("</i></font>");
                }

                // Add generated line to edit
                ui->translationEdit->append(wordLine);
            }

            indent.setTextIndent(0);
            ui->translationEdit->textCursor().setBlockFormat(indent);
            ui->translationEdit->append(""); // Add a new line before the next type of speech
        }
    }

    // Examples
    if (!m_translator->examples().isEmpty()) {
        ui->translationEdit->append("<font color=\"grey\"><i>" + m_translator->source() + "</i> – " + tr("examples:") + "</font>");
        foreach (const QExample &example, m_translator->examples()) {
            ui->translationEdit->append("<b>" + example.typeOfSpeech() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            ui->translationEdit->textCursor().setBlockFormat(indent);
            for (int i = 0; i < example.count(); ++i) {
                ui->translationEdit->append(example.description(i));
                ui->translationEdit->append("<font color=\"grey\"><i>" + example.example(i) + "</i></font>");
                ui->translationEdit->append("");
            }
            indent.setTextIndent(0);
            ui->translationEdit->textCursor().setBlockFormat(indent);
        }
    }

    ui->translationEdit->moveCursor(QTextCursor::Start);
    emit translationTextChanged(ui->translationEdit->toHtml());
    ui->translateButton->setEnabled(true);
}

void MainWindow::on_swapButton_clicked()
{
    const QOnlineTranslator::Language sourceLang = m_sourceLangButtons->checkedLanguage();
    const QOnlineTranslator::Language translationLang = m_translationLangButtons->checkedLanguage();

    // Insert current translation language to source buttons
    if (m_translationLangButtons->checkedId() == 0)
        m_sourceLangButtons->checkButton(0); // Select "Auto" button
    else
        m_sourceLangButtons->insertLanguage(translationLang);

    // Insert current source language to translation buttons
    if (m_sourceLangButtons->checkedId() == 0)
        m_translationLangButtons->checkButton(0); // Select "Auto" button
    else
        m_translationLangButtons->insertLanguage(sourceLang);

    // Copy translation to source text
    ui->sourceEdit->setPlainText(m_translator->translation());
    ui->sourceEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(this);
    if (config.exec() == QDialog::Accepted) {
        const AppSettings settings;
        loadSettings(settings);
    }
}

void MainWindow::on_autoTranslateCheckBox_toggled(bool checked)
{
    if (checked) {
        connect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::startTranslateTimer);
        on_translateButton_clicked();
    } else {
        disconnect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::startTranslateTimer);
    }
}

void MainWindow::on_engineComboBox_currentIndexChanged(int)
{
    if (ui->autoTranslateCheckBox->isChecked())
        on_translateButton_clicked();
}

void MainWindow::on_copySourceButton_clicked()
{
    if (!ui->sourceEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(m_translator->source());
    else
        qInfo() << tr("Text field is empty");
}

void MainWindow::on_copyTranslationButton_clicked()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(m_translator->translation());
    else
        qInfo() << tr("Text field is empty");
}

void MainWindow::on_copyAllTranslationButton_clicked()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(ui->translationEdit->toPlainText());
    else
        qInfo() << tr("Text field is empty");
}

void MainWindow::translateSelectedText()
{
    // Prevent pressing the translation hotkey again
    m_translateSelectionHotkey->blockSignals(true);

    const AppSettings settings;
    if (this->isHidden() && settings.windowMode() == AppSettings::PopupWindow) {
        // Show popup
        auto *popup = new PopupWindow(this);

        // Send selected text to source field and translate it
        if (ui->autoTranslateCheckBox->isChecked()) {
            // Block signals and translate text without delay
            ui->sourceEdit->blockSignals(true);
            ui->sourceEdit->setPlainText(selectedText());
            ui->sourceEdit->blockSignals(false);
        } else {
            ui->sourceEdit->setPlainText(selectedText());
        }

        on_translateButton_clicked();
        popup->show();
        popup->activateWindow();

        // Restore the keyboard shortcut
        connect(popup, &PopupWindow::destroyed, [this] {
            m_translateSelectionHotkey->blockSignals(false);
        });
    } else {
        // Send selected text to source field and translate it
        if (ui->autoTranslateCheckBox->isChecked()) {
            ui->sourceEdit->blockSignals(true);
            ui->sourceEdit->setPlainText(selectedText());
            ui->sourceEdit->blockSignals(false);
        } else {
            ui->sourceEdit->setPlainText(selectedText());
        }

        on_translateButton_clicked();
        activate();

        // Restore the keyboard shortcut
        m_translateSelectionHotkey->blockSignals(false);
    }
}

void MainWindow::copyTranslatedSelection()
{
    ui->sourceEdit->setPlainText(selectedText());

    on_translateButton_clicked();

    if (m_translator->error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to translate text"), m_translator->errorString());
        errorMessage.exec();
        return;
    }

    SingleApplication::clipboard()->setText(m_translator->translation());
}

void MainWindow::playSelection()
{
    ui->sourceEdit->setPlainText(selectedText());
    m_sourcePlayerButtons->play();
}

void MainWindow::playTranslatedSelection()
{
    ui->sourceEdit->setPlainText(selectedText());
    on_translateButton_clicked();
    m_translationPlayerButtons->play();
}

void MainWindow::checkLanguageButton(LangButtonGroup *checkedGroup, LangButtonGroup *anotherGroup, int id)
{
    /* If the target and source languages are the same (and they are not autodetect buttons),
     * then insert previous checked language from just checked language group to another group */
    if (checkedGroup->language(id) == anotherGroup->checkedLanguage() && anotherGroup->checkedId() != 0 && id != 0)
        anotherGroup->insertLanguage(checkedGroup->previousCheckedLanguage());

    // Check if selected language is supported by engine
    if (!QOnlineTranslator::isSupportTranslation(static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex()),
                                                 checkedGroup->language(id))) {
        for (int i = 0; i < ui->engineComboBox->count(); ++i) {
            if (QOnlineTranslator::isSupportTranslation(static_cast<QOnlineTranslator::Engine>(i), checkedGroup->language(id))) {
                ui->engineComboBox->setCurrentIndex(i); // Check first supported language
                break;
            }
        }
        return;
    }

    // Translate the text automatically if "Automatically translate" is checked or if a pop-up window is open
    if (ui->autoTranslateCheckBox->isChecked() || isHidden())
        on_translateButton_clicked();
}

void MainWindow::resetAutoSourceButtonText()
{
    disconnect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    m_sourceLangButtons->setLanguage(0, QOnlineTranslator::Auto);
}

void MainWindow::startTranslateTimer()
{
    m_translateTimer->start(autotranslateDelay);
}

void MainWindow::showAppRunningMessage()
{
    auto *message = new QMessageBox(QMessageBox::Information, SingleApplication::applicationName(), tr("The application is already running"));
    message->setAttribute(Qt::WA_DeleteOnClose); // Need to allocate on heap to avoid crash!
    activate();
    message->show();
}

#if defined(Q_OS_WIN)
void MainWindow::checkForUpdates()
{
    auto *release = qobject_cast<QGitTag *>(sender());
    if (release->error()) {
        delete release;
        return;
    }

    const int installer = release->assetId(".exe");
    if (SingleApplication::applicationVersion() < release->tagName() && installer != -1) {
        auto *updater = new UpdaterWindow(release, installer, this);
        updater->show();
    }

    delete release;
    AppSettings settings;
    settings.setLastUpdateCheckDate(QDate::currentDate());
}
#endif

void MainWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LocaleChange:
    {
        // System language chaged
        AppSettings settings;
        const QLocale::Language lang = settings.locale();
        if (lang == QLocale::AnyLanguage)
            settings.loadLocale(lang); // Reload language if application use system language
        m_uiLang = QOnlineTranslator::language(lang);
        break;
    }
    case QEvent::LanguageChange:
        // Reload UI if application language changed
        ui->retranslateUi(this);

        m_sourceLangButtons->retranslate();
        m_translationLangButtons->retranslate();

        m_trayMenu->actions().at(0)->setText(tr("Show window"));
        m_trayMenu->actions().at(1)->setText(tr("Settings"));
        m_trayMenu->actions().at(2)->setText(tr("Exit"));
        break;
    default:
        QMainWindow::changeEvent(event);
    }
}

// Translate text in window
bool MainWindow::translate(QOnlineTranslator::Language translationLang, QOnlineTranslator::Language sourceLang)
{
    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    m_translator->translate(ui->sourceEdit->toPlainText(), engine, translationLang, sourceLang, m_uiLang);

    // Check for error
    if (m_translator->error()) {
        ui->translationEdit->setHtml(m_translator->errorString());
        ui->translateButton->setEnabled(true);
        m_sourceLangButtons->setLanguage(0, QOnlineTranslator::Auto);
        emit translationTextChanged(m_translator->errorString());
        return false;
    }

    return true;
}

void MainWindow::loadSettings(const AppSettings &settings)
{
    m_trayIcon->loadSettings(settings);

    // Translation
    m_translator->setSourceTranslitEnabled(settings.isSourceTranslitEnabled());
    m_translator->setTranslationTranslitEnabled(settings.isTranslationTranslitEnabled());
    m_translator->setSourceTranscriptionEnabled(settings.isSourceTranscriptionEnabled());
    m_translator->setTranslationOptionsEnabled(settings.isTranslationOptionsEnabled());
    m_translator->setExamplesEnabled(settings.isExamplesEnabled());

    // Connection
    QNetworkProxy proxy;
    proxy.setType(settings.proxyType());
    if (proxy.type() == QNetworkProxy::HttpProxy || proxy.type() == QNetworkProxy::Socks5Proxy) {
        proxy.setHostName(settings.proxyHost());
        proxy.setPort(settings.proxyPort());
        if (settings.isProxyAuthEnabled()) {
            proxy.setUser(settings.proxyUsername());
            proxy.setPassword(settings.proxyPassword());
        }
    }
    QNetworkProxy::setApplicationProxy(proxy);

    // Language buttons style
    const Qt::ToolButtonStyle languagesStyle = settings.windowLanguagesStyle();
    ui->firstSourceButton->setToolButtonStyle(languagesStyle);
    ui->secondSourceButton->setToolButtonStyle(languagesStyle);
    ui->thirdSourceButton->setToolButtonStyle(languagesStyle);
    ui->firstTranslationButton->setToolButtonStyle(languagesStyle);
    ui->secondTranslationButton->setToolButtonStyle(languagesStyle);
    ui->thirdTranslationButton->setToolButtonStyle(languagesStyle);

    // Control buttons style
    const Qt::ToolButtonStyle controlsStyle = settings.windowControlsStyle();
    ui->playSourceButton->setToolButtonStyle(controlsStyle);
    ui->stopSourceButton->setToolButtonStyle(controlsStyle);
    ui->copySourceButton->setToolButtonStyle(controlsStyle);
    ui->playTranslationButton->setToolButtonStyle(controlsStyle);
    ui->stopTranslationButton->setToolButtonStyle(controlsStyle);
    ui->copyTranslationButton->setToolButtonStyle(controlsStyle);
    ui->copyAllTranslationButton->setToolButtonStyle(controlsStyle);
    ui->settingsButton->setToolButtonStyle(controlsStyle);

    // Global shortcuts
    m_translateSelectionHotkey->setShortcut(settings.translateSelectionHotkey(), true);
    m_playSelectionHotkey->setShortcut(settings.speakSelectionHotkey(), true);
    m_stopSpeakingHotkey->setShortcut(settings.stopSpeakingHotkey(), true);
    m_playTranslatedSelectionHotkey->setShortcut(settings.speakTranslatedSelectionHotkey(), true);
    m_showMainWindowHotkey->setShortcut(settings.showMainWindowHotkey(), true);
    m_copyTranslatedSelectionHotkey->setShortcut(settings.copyTranslatedSelectionHotkey(), true);

    // Window shortcuts
    ui->translateButton->setShortcut(settings.translateHotkey());
    ui->playSourceButton->setShortcut(settings.speakSourceHotkey());
    ui->playTranslationButton->setShortcut(settings.speakTranslationHotkey());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationHotkey());
    m_closeWindowsShortcut->setKey(settings.closeWindowHotkey());
}

// Use playlist to split long queries due Google limit
void MainWindow::setPlayingText(QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang)
{
    if (text.isEmpty()) {
        QMessageBox errorMessage(QMessageBox::Information, tr("Nothing to play"), tr("Playback text is empty"));
        errorMessage.exec();
        return;
    }

    playlist->clear();
    const AppSettings settings;
    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    QOnlineTts::Voice voice = QOnlineTts::DefaultVoice;
    QOnlineTts::Emotion emotion = QOnlineTts::DefaultEmotion;
    switch (engine) {
    case QOnlineTranslator::Yandex:
        voice = settings.yandexVoice();
        emotion = settings.yandexEmotion();
        break;
    case QOnlineTranslator::Bing:
        voice = settings.bingVoice();
        break;
    default:
        break;
    }

    if (lang == QOnlineTranslator::Auto) {
        m_translator->detectLanguage(text, engine);
        if (m_translator->error()) {
            QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to play text"), m_translator->errorString());
            errorMessage.exec();
            return;
        }
        lang = m_translator->sourceLanguage();
    }

    QOnlineTts tts;
    tts.generateUrls(text, engine, lang, voice, emotion);
    const QList<QMediaContent> media = tts.media();
    if (tts.error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to play text"), tts.errorString());
        errorMessage.exec();
        return;
    }

    playlist->addMedia(media);
}

QString MainWindow::selectedText()
{
    QString selectedText;
#if defined(Q_OS_LINUX)
    selectedText = SingleApplication::clipboard()->text(QClipboard::Selection);
#elif defined(Q_OS_WIN) // Send Ctrl + C to get selected text
    // Save original clipboard data
    QVariant originalClipboard;
    if (SingleApplication::clipboard()->mimeData()->hasImage())
        originalClipboard = SingleApplication::clipboard()->image();
    else
        originalClipboard = SingleApplication::clipboard()->text();

    // Wait until the hot key is pressed
    while (GetAsyncKeyState(static_cast<int>(m_translateSelectionHotkey.currentNativeShortcut().key))
           || GetAsyncKeyState(VK_CONTROL)
           || GetAsyncKeyState(VK_MENU)
           || GetAsyncKeyState(VK_SHIFT)) {
    }

    // Generate key sequence
    INPUT copyText[4];

    // Set the press of the "Ctrl" key
    copyText[0].ki.wVk = VK_CONTROL;
    copyText[0].ki.dwFlags = 0; // 0 for key press
    copyText[0].type = INPUT_KEYBOARD;

    // Set the press of the "C" key
    copyText[1].ki.wVk = 'C';
    copyText[1].ki.dwFlags = 0;
    copyText[1].type = INPUT_KEYBOARD;

    // Set the release of the "C" key
    copyText[2].ki.wVk = 'C';
    copyText[2].ki.dwFlags = KEYEVENTF_KEYUP;
    copyText[2].type = INPUT_KEYBOARD;

    // Set the release of the "Ctrl" key
    copyText[3].ki.wVk = VK_CONTROL;
    copyText[3].ki.dwFlags = KEYEVENTF_KEYUP;
    copyText[3].type = INPUT_KEYBOARD;

    // Send key sequence to system
    SendInput(4, copyText, sizeof(INPUT));

    // Wait for clipboard changes
    QEventLoop loop;
    loop.connect(SingleApplication::clipboard(), &QClipboard::changed, &loop, &QEventLoop::quit);

    // Set the timer to exit the loop if no text is selected for a second
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(1000);
    loop.connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Start waiting
    timer.start();
    loop.exec();

    // Check if timer is out
    if (!timer.isActive())
        return SingleApplication::clipboard()->text(); // No text selected, just return the clipboard
    else
        timer.stop();

    // Workaround for the case where the clipboard has changed but not yet available
    if (SingleApplication::clipboard()->text().isEmpty())
        QThread::msleep(1);

    // Get clipboard data
    selectedText = SingleApplication::clipboard()->text();

    // Restore original clipboard
    if (originalClipboard.type() == QVariant::Image)
        SingleApplication::clipboard()->setImage(originalClipboard.value<QImage>());
    else
        SingleApplication::clipboard()->setText(originalClipboard.toString());
#endif
    return selectedText;
}

void MainWindow::on_addSourceLangButton_clicked()
{
    AddLangDialog langDialog(this);
    if (langDialog.exec() == QDialog::Accepted)
        m_sourceLangButtons->insertLanguage(langDialog.language());
}

void MainWindow::on_addTranslationLangButton_clicked()
{
    AddLangDialog langDialog(this);
    if (langDialog.exec() == QDialog::Accepted)
        m_translationLangButtons->insertLanguage(langDialog.language());
}
