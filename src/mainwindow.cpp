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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "popupwindow.h"
#include "settingsdialog.h"
#include "addlangdialog.h"
#include "appsettings.h"
#include "singleapplication.h"
#if defined(Q_OS_WIN)
#include "updaterwindow.h"

#include <QMimeData>
#include <QTimer>
#include <QThread>
#include <Windows.h>
#endif
#include <QClipboard>
#include <QNetworkProxy>
#include <QMessageBox>

constexpr int AUTOTRANSLATE_DELAY = 500; // Used when changing text
constexpr int SHORT_AUTOTRANSLATE_DELAY = 300; // Used when changing language

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Show a message that the application is already running
    connect(dynamic_cast<SingleApplication*>(SingleApplication::instance()), &SingleApplication::instanceStarted, this, &MainWindow::showAppRunningMessage);

    // Shortcuts
    connect(&m_translateSelectionHotkey, &QHotkey::activated, this, &MainWindow::translateSelectedText);
    connect(&m_playSelectionHotkey, &QHotkey::activated, this, &MainWindow::playSelection);
    connect(&m_playTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::playTranslatedSelection);
    connect(&m_stopSelectionHotkey, &QHotkey::activated, &m_selectionPlayer, &QMediaPlayer::stop);
    connect(&m_showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::showMainWindow);
    connect(&m_copyTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::copyTranslatedSelection);
    connect(&m_closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);

    // Text speaking
    m_sourcePlayer.setPlaylist(&m_sourcePlaylist); // Use playlist to split long queries due Google limit
    m_translationPlayer.setPlaylist(&m_translationPlaylist);
    m_selectionPlayer.setPlaylist(&m_selectionPlaylist);
    connect(ui->sourceEdit, &QPlainTextEdit::textChanged, &m_sourcePlayer, &QMediaPlayer::stop);
    connect(&m_sourcePlayer, &QMediaPlayer::stateChanged, this, &MainWindow::changeSourcePlayerState);
    connect(&m_translationPlayer, &QMediaPlayer::stateChanged, this, &MainWindow::changeTranslationPlayerState);
    connect(&m_selectionPlayer, &QMediaPlayer::stateChanged, this, &MainWindow::changeSelectionPlayerState);

    // Source button group
    m_sourceButtons.addButton(ui->autoSourceButton);
    m_sourceButtons.addButton(ui->firstSourceButton);
    m_sourceButtons.addButton(ui->secondSourceButton);
    m_sourceButtons.addButton(ui->thirdSourceButton);
    m_sourceButtons.setName("Source");
    m_sourceButtons.loadLanguages();

    // Translation button group
    m_translationButtons.addButton(ui->autoTranslationButton);
    m_translationButtons.addButton(ui->firstTranslationButton);
    m_translationButtons.addButton(ui->secondTranslationButton);
    m_translationButtons.addButton(ui->thirdTranslationButton);
    m_translationButtons.setName("Translation");
    m_translationButtons.loadLanguages();

    // Toggle language logic
    connect(&m_translationButtons, &LangButtonGroup::buttonChecked, [&](int id) {
        checkLanguageButton(&m_translationButtons, &m_sourceButtons, id);
    });
    connect(&m_sourceButtons, &LangButtonGroup::buttonChecked, [&](int id) {
        checkLanguageButton(&m_sourceButtons, &m_translationButtons, id);
    });

    // System tray icon
    m_trayMenu.addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    m_trayMenu.addAction(QIcon::fromTheme("dialog-object-properties"), tr("Settings"), this, &MainWindow::on_settingsButton_clicked);
    m_trayMenu.addAction(QIcon::fromTheme("application-exit"), tr("Exit"), SingleApplication::instance(), &SingleApplication::quit);
    m_trayIcon.setContextMenu(&m_trayMenu);
    connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::activateTray);

    // Timer for automatic translation
    m_translateTimer.setSingleShot(true);
    connect(&m_translateTimer, &QTimer::timeout, this, &MainWindow::translateTimerExpires);

    // Get UI language for translation
    m_uiLang = QOnlineTranslator::language(QLocale());

    // Load app settings
    loadSettings();

    // Load main window settings
    AppSettings settings;
    restoreGeometry(settings.mainWindowGeometry());
    ui->autoTranslateCheckBox->setChecked(settings.isAutoTranslateEnabled());
    ui->engineComboBox->setCurrentIndex(settings.currentEngine());

#if defined(Q_OS_WIN)
    // Check date for updates
    const auto updateInterval = settings.checkForUpdatesInterval();
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
        auto release = new QGitTag(this);
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
    settings.setCurrentEngine(static_cast<AppSettings::Engine>(ui->engineComboBox->currentIndex()));
    delete ui;
}

void MainWindow::on_translateButton_clicked()
{
    if (ui->sourceEdit->toPlainText().isEmpty()) {
        ui->translationEdit->clear();
        m_translationButtons.setLanguage(0, QOnlineTranslator::Auto);
        return;
    }

    // Disable the translation button to prevent re-pressing
    ui->translateButton->setEnabled(false);

    // Stop translation playback
    m_translationPlayer.stop();

    // Source Language
    QOnlineTranslator::Language sourceLang;
    if (ui->autoSourceButton->isChecked())
        sourceLang = QOnlineTranslator::Auto;
    else
        sourceLang = m_sourceButtons.checkedLanguage();

    // Translation language
    AppSettings settings;
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
        translationLang = m_translationButtons.checkedLanguage();
    }

    // Get translation
    if (!translate(translationLang, sourceLang))
        return;

    // Re-translate to a secondary or a primary language if the autodetected source language and the source language are the same
    if (ui->autoTranslationButton->isChecked() && m_translator.sourceLanguage() == m_translator.translationLanguage()) {
        auto primaryLanguage = settings.primaryLanguage();
        auto secondaryLanguage = settings.secondaryLanguage();
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
        m_sourceButtons.setLanguage(0, m_translator.sourceLanguage());
        connect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    }

    if (ui->autoTranslationButton->isChecked())
        m_translationButtons.setLanguage(0, m_translator.translationLanguage());
    else
        m_translationButtons.setLanguage(0, QOnlineTranslator::Auto);

    // Show translation and transcription
    ui->translationEdit->setHtml(m_translator.translation().toHtmlEscaped().replace("\n", "<br>"));
    if (!m_translator.translationTranslit().isEmpty() && settings.showTranslationTranslit())
        ui->translationEdit->append("<font color=\"grey\"><i>/" + m_translator.translationTranslit().replace("\n", "/<br>/") + "/</i></font>");
    if (!m_translator.sourceTranslit().isEmpty() && settings.showSourceTranslit())
        ui->translationEdit->append("<font color=\"grey\"><i><b>(" + m_translator.sourceTranslit().replace("\n", "/<br>/") + ")</b></i></font>");

    // Show transcription
    if (!m_translator.sourceTranscription().isEmpty() && settings.showSourceTranscription())
        ui->translationEdit->append("<font color=\"grey\">[" + m_translator.sourceTranscription() + "]</font>");

    ui->translationEdit->append(""); // Add new line before translation options

    // Show translation options
    if (!m_translator.dictionaryList().isEmpty() && settings.showTranslationOptions()) {
        // Label
        ui->translationEdit->append("<font color=\"grey\"><i>" + m_translator.source() + "</i> – " + tr("translation options:") + "</font>");

        // Print words for each type of speech
        foreach (const QDictionary &dictionary, m_translator.dictionaryList()) {
            ui->translationEdit->append("<b>" + dictionary.typeOfSpeech() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            ui->translationEdit->textCursor().setBlockFormat(indent);

            for (int i = 0; i <  dictionary.count(); ++i) {
                // Show word gender
                QString wordLine;
                if (!dictionary.gender(i).isEmpty())
                    wordLine.append("<i>" + dictionary.gender(i) + "</i> ");

                // Show Word
                wordLine.append(dictionary.word(i));

                // Show word meaning
                if (!dictionary.translations(i).isEmpty()) {
                    wordLine.append(": ");
                    wordLine.append("<font color=\"grey\"><i>");
                    wordLine.append(dictionary.translations(i));
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

    // Show definitions
    if (!m_translator.definitionsList().isEmpty() && settings.showDefinitions()) {
        ui->translationEdit->append("<font color=\"grey\"><i>" + m_translator.source() + "</i> – " + tr("definitions:") + "</font>");
        foreach (const QDefinition &definition, m_translator.definitionsList()) {
            ui->translationEdit->append("<b>" + definition.typeOfSpeech() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            ui->translationEdit->textCursor().setBlockFormat(indent);
            ui->translationEdit->append(definition.description());
            ui->translationEdit->append("<font color=\"grey\"><i>" + definition.example()+ "</i></font>");
            indent.setTextIndent(0);
            ui->translationEdit->textCursor().setBlockFormat(indent);
            ui->translationEdit->append("");
        }
    }

    ui->translationEdit->moveCursor(QTextCursor::Start);
    emit translationTextChanged(ui->translationEdit->toHtml());
    ui->translateButton->setEnabled(true);
}

void MainWindow::on_swapButton_clicked()
{
    const int checkedSourceButton = m_sourceButtons.checkedId();
    const int checkedTranslationButton = m_translationButtons.checkedId();
    const QOnlineTranslator::Language sourceLang = m_sourceButtons.checkedLanguage();
    const QOnlineTranslator::Language translationLang = m_translationButtons.checkedLanguage();

    // Insert current translation language to source buttons
    if (checkedTranslationButton == 0)
        m_sourceButtons.checkButton(0); // Select "Auto" button
    else
        m_sourceButtons.insertLanguage(translationLang);

    // Insert current source language to translation buttons
    if (checkedSourceButton == 0)
        m_translationButtons.checkButton(0); // Select "Auto" button
    else
        m_translationButtons.insertLanguage(sourceLang);

    // Copy translation to source text
    ui->sourceEdit->setPlainText(m_translator.translation());
    ui->sourceEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(this);
    if (config.exec() == QDialog::Accepted)
        loadSettings();
}

void MainWindow::on_autoTranslateCheckBox_toggled(bool checked)
{
    if (checked) {
        connect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::startTranslateTimer);
        translateTimerExpires();
    } else {
        disconnect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::startTranslateTimer);
    }
}

void MainWindow::on_engineComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    if (ui->autoTranslateCheckBox->isChecked())
        m_translateTimer.start(SHORT_AUTOTRANSLATE_DELAY);
}

void MainWindow::on_playSourceButton_clicked()
{
    switch (m_sourcePlayer.state()) {
    case QMediaPlayer::PlayingState:
        m_sourcePlayer.pause();
        break;
    case QMediaPlayer::PausedState:
        m_sourcePlayer.play();
        break;
    case QMediaPlayer::StoppedState:
        play(&m_sourcePlayer, &m_sourcePlaylist, ui->sourceEdit->toPlainText(), m_sourceButtons.checkedLanguage());
        break;
    }
}

void MainWindow::on_playTranslationButton_clicked()
{
    switch (m_translationPlayer.state()) {
    case QMediaPlayer::PlayingState:
        m_translationPlayer.pause();
        break;
    case QMediaPlayer::PausedState:
        m_translationPlayer.play();
        break;
    case QMediaPlayer::StoppedState:
        play(&m_translationPlayer, &m_translationPlaylist, m_translator.translation(), m_translator.translationLanguage());
        break;
    }
}

void MainWindow::on_stopSourceButton_clicked()
{
    m_sourcePlayer.stop();
}

void MainWindow::on_stopTranslationButton_clicked()
{
    m_translationPlayer.stop();
}

void MainWindow::on_copySourceButton_clicked()
{
    if (!ui->sourceEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(m_translator.source());
    else
        qInfo() << tr("Text field is empty");
}

void MainWindow::on_copyTranslationButton_clicked()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(m_translator.translation());
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
    m_translateSelectionHotkey.blockSignals(true);

    AppSettings settings;
    if (this->isHidden() && settings.windowMode() == AppSettings::PopupWindow) {
        // Show popup
        auto popup = new PopupWindow(&m_sourceButtons, &m_translationButtons, ui->engineComboBox->currentIndex(), this);

        // Connect main window events to popup events
        connect(&m_sourceButtons, &LangButtonGroup::buttonChecked, popup->sourceButtons(), &LangButtonGroup::checkButton);
        connect(&m_sourceButtons, &LangButtonGroup::languageChanged, popup->sourceButtons(),  &LangButtonGroup::setLanguage);
        connect(&m_translationButtons, &LangButtonGroup::buttonChecked, popup->translationButtons(), &LangButtonGroup::checkButton);
        connect(&m_translationButtons, &LangButtonGroup::languageChanged, popup->translationButtons(),  &LangButtonGroup::setLanguage);
        connect(this, &MainWindow::translationTextChanged, popup->translationEdit(), &QTextEdit::setHtml);
        connect(this, &MainWindow::playSourceButtonIconChanged, popup->playSourceButton(), &QToolButton::setIcon);
        connect(this, &MainWindow::stopSourceButtonEnabled, popup->stopSourceButton(), &QToolButton::setEnabled);
        connect(this, &MainWindow::playTranslationButtonIconChanged, popup->playTranslationButton(), &QToolButton::setIcon);
        connect(this, &MainWindow::stopTranslationButtonEnabled, popup->stopTranslationButton(), &QToolButton::setEnabled);

        // Connect popup events
        connect(popup->engineCombobox(), qOverload<int>(&QComboBox::currentIndexChanged), ui->engineComboBox, &QComboBox::setCurrentIndex);
        connect(popup->sourceButtons(), &LangButtonGroup::buttonChecked, &m_sourceButtons, &LangButtonGroup::checkButton);
        connect(popup->translationButtons(), &LangButtonGroup::buttonChecked, &m_translationButtons, &LangButtonGroup::checkButton);
        connect(popup->addSourceLangButton(), &QToolButton::clicked, ui->addSourceLangButton, &QToolButton::click);
        connect(popup->addTranslationLangButton(), &QToolButton::clicked, ui->addTranslationLangButton, &QToolButton::click);
        connect(popup->swapButton(), &QToolButton::clicked, this, &MainWindow::on_swapButton_clicked);
        connect(popup->playSourceButton(), &QToolButton::clicked, this, &MainWindow::on_playSourceButton_clicked);
        connect(popup->stopSourceButton(), &QToolButton::clicked, &m_sourcePlayer, &QMediaPlayer::stop);
        connect(popup->copySourceButton(), &QToolButton::clicked, this, &MainWindow::on_copySourceButton_clicked);
        connect(popup->playTranslationButton(), &QToolButton::clicked, this, &MainWindow::on_playTranslationButton_clicked);
        connect(popup->stopTranslationButton(), &QToolButton::clicked, &m_translationPlayer, &QMediaPlayer::stop);
        connect(popup->copyTranslationButton(), &QToolButton::clicked, this, &MainWindow::on_copyTranslationButton_clicked);
        connect(popup->copyAllTranslationButton(), &QToolButton::clicked, this, &MainWindow::on_copyAllTranslationButton_clicked);
        connect(popup, &PopupWindow::destroyed, &m_sourcePlayer, &QMediaPlayer::stop);
        connect(popup, &PopupWindow::destroyed, &m_translationPlayer, &QMediaPlayer::stop);

        // Restore the keyboard shortcut
        connect(popup, &PopupWindow::destroyed, [this] {
            m_translateSelectionHotkey.blockSignals(false);
        });

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
        showMainWindow();

        // Restore the keyboard shortcut
        m_translateSelectionHotkey.blockSignals(false);
    }
}

void MainWindow::copyTranslatedSelection()
{
    ui->sourceEdit->setPlainText(selectedText());

    on_translateButton_clicked();

    if (m_translator.error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to translate text"), m_translator.errorString());
        errorMessage.exec();
        return;
    }

    SingleApplication::clipboard()->setText(m_translator.translation());
}

void MainWindow::playSelection()
{
    play(&m_sourcePlayer, &m_sourcePlaylist, selectedText());
}

void MainWindow::playTranslatedSelection()
{
    const QString selection = selectedText();

    // Detect languages
    AppSettings settings;
    QOnlineTranslator::Language primaryLanguage = settings.primaryLanguage();
    if (primaryLanguage == QOnlineTranslator::Auto)
        primaryLanguage = m_uiLang;

    // Translate text
    if (!translateOutside(selection, primaryLanguage))
        return;

    if (m_translator.sourceLanguage() == primaryLanguage) {
        QOnlineTranslator::Language secondaryLanguage = settings.secondaryLanguage();
        if (secondaryLanguage == QOnlineTranslator::Auto)
            secondaryLanguage = m_uiLang;

        if (!translateOutside(selection, secondaryLanguage))
            return;
    }

    play(&m_selectionPlayer, &m_selectionPlaylist, m_translator.translation(), m_translator.translationLanguage());
}

void MainWindow::checkLanguageButton(LangButtonGroup *checkedGroup, LangButtonGroup *anotherGroup, int id)
{
    // If the target and source languages are the same (and they are not automatic translation buttons), then source target language to previous target language
    AppSettings settings;
    if (id != 0
            && anotherGroup->checkedId() != 0
            && checkedGroup->language(id) == anotherGroup->checkedLanguage()) {
        const int previousCheckedButton = settings.checkedButton(checkedGroup);
        anotherGroup->insertLanguage(checkedGroup->language(previousCheckedButton));
        settings.setCheckedButton(anotherGroup, anotherGroup->checkedId());
    }

    // Translate the text automatically if "Automatically translate" is checked or if a pop-up window is open
    if (ui->autoTranslateCheckBox->isChecked() || this->isHidden())
        m_translateTimer.start(SHORT_AUTOTRANSLATE_DELAY);

    settings.setCheckedButton(checkedGroup, checkedGroup->checkedId());
}

void MainWindow::resetAutoSourceButtonText()
{
    disconnect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    m_sourceButtons.setLanguage(0, QOnlineTranslator::Auto);
}

void MainWindow::changeSourcePlayerState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        // Change icon
        ui->playSourceButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        emit playSourceButtonIconChanged(ui->playSourceButton->icon());

        // Disable stop button
        ui->stopSourceButton->setEnabled(true);
        emit stopSourceButtonEnabled(true);

        // Pause other players
        m_translationPlayer.stop();
        m_selectionPlayer.stop();
        break;
    case QMediaPlayer::PausedState:
        // Change icon
        ui->playSourceButton->setIcon(QIcon::fromTheme("media-playback-start"));
        emit playSourceButtonIconChanged(ui->playSourceButton->icon());
        break;
    case QMediaPlayer::StoppedState:
        // Change icon
        ui->playSourceButton->setIcon(QIcon::fromTheme("media-playback-start"));
        emit playSourceButtonIconChanged(ui->playSourceButton->icon());

        // Enable stop button
        ui->stopSourceButton->setEnabled(false);
        emit stopSourceButtonEnabled(false);
        break;
    }
}

void MainWindow::changeTranslationPlayerState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        // Change icon
        ui->playTranslationButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        emit playTranslationButtonIconChanged(ui->playTranslationButton->icon());

        // Disable stop button
        ui->stopTranslationButton->setEnabled(true);
        emit stopTranslationButtonEnabled(true);

        // Pause other players
        m_sourcePlayer.stop();
        m_selectionPlayer.stop();
        break;
    case QMediaPlayer::PausedState:
        // Change icon
        ui->playTranslationButton->setIcon(QIcon::fromTheme("media-playback-start"));
        emit playTranslationButtonIconChanged(ui->playTranslationButton->icon());
        break;
    case QMediaPlayer::StoppedState:
        // Change icon
        ui->playTranslationButton->setIcon(QIcon::fromTheme("media-playback-start"));
        emit playTranslationButtonIconChanged(ui->playTranslationButton->icon());

        // Enable stop button
        ui->stopTranslationButton->setEnabled(false);
        emit stopTranslationButtonEnabled(false);
        break;
    }
}

void MainWindow::changeSelectionPlayerState(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PlayingState) {
        // Pause other players
        m_sourcePlayer.stop();
        m_translationPlayer.stop();
    }
}

void MainWindow::startTranslateTimer()
{
    m_translateTimer.start(AUTOTRANSLATE_DELAY);
}

void MainWindow::translateTimerExpires()
{
    if (ui->translateButton->isEnabled())
        on_translateButton_clicked();
    else
        startTranslateTimer();
}

void MainWindow::showMainWindow()
{
    show();
    activateWindow();
    raise();
}

void MainWindow::showAppRunningMessage()
{
    auto message = new QMessageBox(QMessageBox::Information, "Crow Translate", tr("The application is already running"));
    message->setAttribute(Qt::WA_DeleteOnClose); // Need to allocate on heap to avoid crash!
    showMainWindow();
    message->show();
}

void MainWindow::activateTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (!this->isVisible())
            showMainWindow();
        else
            hide();
    }
}

#if defined(Q_OS_WIN)
void MainWindow::checkForUpdates()
{
    auto release = qobject_cast<QGitTag *>(sender());
    if (release->error()) {
        delete release;
        return;
    }

    const int installer = release->assetId(".exe");
    if (SingleApplication::applicationVersion() < release->tagName() && installer != -1) {
        auto updater = new UpdaterWindow(release, installer, this);
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
        break;
    }
    case QEvent::LanguageChange:
        // Reload UI if application language changed
        ui->retranslateUi(this);

        m_sourceButtons.retranslate();
        m_translationButtons.retranslate();

        m_trayMenu.actions().at(0)->setText(tr("Show window"));
        m_trayMenu.actions().at(1)->setText(tr("Settings"));
        m_trayMenu.actions().at(2)->setText(tr("Exit"));
        break;
    default:
        QMainWindow::changeEvent(event);
    }
}

// Translate text in window
bool MainWindow::translate(QOnlineTranslator::Language translationLang, QOnlineTranslator::Language sourceLang)
{
    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    m_translator.translate(ui->sourceEdit->toPlainText(), engine, translationLang, sourceLang, m_uiLang);

    // Check for error
    if (m_translator.error()) {
        ui->translationEdit->setHtml(m_translator.errorString());
        ui->translateButton->setEnabled(true);
        m_sourceButtons.setLanguage(0, QOnlineTranslator::Auto);
        emit translationTextChanged(m_translator.errorString());
        return false;
    }

    return true;
}

// Translate text outside the window
bool MainWindow::translateOutside(const QString &text, QOnlineTranslator::Language translationLang)
{
    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    m_translator.translate(text, engine, translationLang);

    if (m_translator.error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to translate text"), m_translator.errorString());
        errorMessage.exec();
        return false;
    }

    return true;
}

void MainWindow::loadSettings()
{
    AppSettings settings;

    // System tray icon
    const QString iconName = settings.trayIconName();
    if (iconName == "custom") {
        QIcon customIcon(settings.customIconPath());
        if (customIcon.isNull())
            m_trayIcon.setIcon(QIcon::fromTheme("dialog-error"));
        else
            m_trayIcon.setIcon(customIcon);
    } else {
        QIcon crowIcon = QIcon::fromTheme(iconName);
        if (crowIcon.isNull())
            m_trayIcon.setIcon(QIcon::fromTheme("dialog-error"));
        else
            m_trayIcon.setIcon(crowIcon);
    }

    const bool trayIconVisible = settings.isTrayIconVisible();
    m_trayIcon.setVisible(trayIconVisible);
    SingleApplication::setQuitOnLastWindowClosed(!trayIconVisible);

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
    m_translateSelectionHotkey.setShortcut(settings.translateSelectionHotkey(), true);
    m_playSelectionHotkey.setShortcut(settings.playSelectionHotkey(), true);
    m_stopSelectionHotkey.setShortcut(settings.stopSelectionHotkey(), true);
    m_playTranslatedSelectionHotkey.setShortcut(settings.playTranslatedSelectionHotkey(), true);
    m_showMainWindowHotkey.setShortcut(settings.showMainWindowHotkey(), true);
    m_copyTranslatedSelectionHotkey.setShortcut(settings.copyTranslatedSelectionHotkey(), true);

    // Window shortcuts
    ui->translateButton->setShortcut(settings.translateHotkey());
    ui->playSourceButton->setShortcut(settings.playSourceHotkey());
    ui->stopSourceButton->setShortcut(settings.stopSourceHotkey());
    ui->playTranslationButton->setShortcut(settings.playTranslationHotkey());
    ui->stopTranslationButton->setShortcut(settings.stopTranslationHotkey());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationHotkey());
    m_closeWindowsShortcut.setKey(settings.closeWindowHotkey());
}

void MainWindow::play(QMediaPlayer *player, QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang)
{
    if (text.isEmpty()) {
        QMessageBox errorMessage(QMessageBox::Information, tr("Nothing to play"), tr("Playback text is empty"));
        errorMessage.exec();
        return;
    }

    playlist->clear();
    AppSettings settings;
    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    const QList<QMediaContent> media = m_translator.media(text, engine, lang, settings.speaker(), settings.emotion());
    if (m_translator.error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to play text"), m_translator.errorString());
        errorMessage.exec();
        return;
    }

    playlist->addMedia(media);
    player->play();
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
        m_sourceButtons.insertLanguage(langDialog.language());
}

void MainWindow::on_addTranslationLangButton_clicked()
{
    AddLangDialog langDialog(this);
    if (langDialog.exec() == QDialog::Accepted)
        m_translationButtons.insertLanguage(langDialog.language());
}
