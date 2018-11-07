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

#include <QClipboard>
#include <QNetworkProxy>
#include <QMessageBox>

#if defined(Q_OS_WIN)
#include <QMimeData>
#include <QTimer>
#include <windows.h>

#include "updaterwindow.h"
#endif

#include "ui_mainwindow.h"
#include "appsettings.h"
#include "popupwindow.h"
#include "settingsdialog.h"

constexpr int AUTOTRANSLATE_DELAY = 500; // Used when changing text
constexpr int SHORT_AUTOTRANSLATE_DELAY = 300; // Used when changing language

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow),
    onlineTranslator (new QOnlineTranslator(this)),
    sourcePlayer (new QMediaPlayer(this)),
    translationPlayer (new QMediaPlayer(this)),
    selectionPlayer (new QMediaPlayer(this)),
    sourcePlaylist (new QMediaPlaylist(this)),
    translationPlaylist (new QMediaPlaylist(this)),
    selectionPlaylist (new QMediaPlaylist(this)),
    closeWindowsShortcut (new QShortcut(this)),
    translateSelectionHotkey (new QHotkey(this)),
    playSelectionHotkey (new QHotkey(this)),
    playTranslatedSelectionHotkey (new QHotkey(this)),
    stopSelectionHotkey (new QHotkey(this)),
    showMainWindowHotkey (new QHotkey(this)),
    copyTranslatedSelectionHotkey (new QHotkey(this)),
    sourceButtons (new LangButtonGroup(this)),
    translationButtons (new LangButtonGroup(this)),
    trayMenu (new QMenu(this)),
    trayIcon (new QSystemTrayIcon(this)),
    languagesMenu (new QMenu(this)),
    translateTimer (new QTimer(this))
{
    ui->setupUi(this);

    // Shortcuts
    connect(translateSelectionHotkey, &QHotkey::activated, this, &MainWindow::translateSelectedText);
    connect(playSelectionHotkey, &QHotkey::activated, this, &MainWindow::playSelection);
    connect(playTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::playTranslatedSelection);
    connect(stopSelectionHotkey, &QHotkey::activated, selectionPlayer, &QMediaPlayer::stop);
    connect(showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::showMainWindow);
    connect(copyTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::copyTranslatedSelection);
    connect(closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);

    // Text speaking
    sourcePlayer->setPlaylist(sourcePlaylist); // Use playlist to split long queries due Google limit
    translationPlayer->setPlaylist(translationPlaylist);
    selectionPlayer->setPlaylist(selectionPlaylist);
    connect(ui->sourceEdit, &QPlainTextEdit::textChanged, sourcePlayer, &QMediaPlayer::stop);
    connect(sourcePlayer, &QMediaPlayer::stateChanged, this, &MainWindow::changeSourcePlayerState);
    connect(translationPlayer, &QMediaPlayer::stateChanged, this, &MainWindow::changeTranslationPlayerState);
    connect(selectionPlayer, &QMediaPlayer::stateChanged, this, &MainWindow::changeSelectionPlayerState);

    // Source button group
    sourceButtons->addButton(ui->autoSourceButton);
    sourceButtons->addButton(ui->firstSourceButton);
    sourceButtons->addButton(ui->secondSourceButton);
    sourceButtons->addButton(ui->thirdSourceButton);
    sourceButtons->setName("Source");
    sourceButtons->loadLanguages();

    // Translation button group
    translationButtons->addButton(ui->autoTranslationButton);
    translationButtons->addButton(ui->firstTranslationButton);
    translationButtons->addButton(ui->secondTranslationButton);
    translationButtons->addButton(ui->thirdTranslationButton);
    translationButtons->setName("Translation");
    translationButtons->loadLanguages();

    // Setup toggle logic
    connect(translationButtons, &LangButtonGroup::buttonChecked, [&](int id) {
        checkLanguageButton(translationButtons, sourceButtons, id);
    });
    connect(sourceButtons, &LangButtonGroup::buttonChecked, [&](int id) {
        checkLanguageButton(sourceButtons, translationButtons, id);
    });

    // System tray icon
    trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    trayMenu->addAction(QIcon::fromTheme("dialog-object-properties"), tr("Settings"), this, &MainWindow::on_settingsButton_clicked);
    trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), qApp, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::activateTray);

    // Timer for automatic translation
    translateTimer->setSingleShot(true);
    connect(translateTimer, &QTimer::timeout, this, &MainWindow::translateTimerExpires);

    // Add languages menu to auto-language buttons
    languagesMenu->addActions(languagesList());
    ui->autoSourceButton->setMenu(languagesMenu);
    ui->autoTranslationButton->setMenu(languagesMenu);

    // Get UI language for translation
    uiLang = QOnlineTranslator::language(QLocale());

    // Load settings
    loadSettings();
    AppSettings settings;
    restoreGeometry(settings.mainWindowGeometry());

    // Check for updates
#if defined(Q_OS_WIN)
    auto updateInterval = settings.checkForUpdatesInterval();
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
        QGitRelease release("Shatur95", "Crow-Translate");
        if (!release.error() && qApp->applicationVersion() < release.tagName()) {
            UpdaterWindow *updaterWindow = new UpdaterWindow(release, this);
            updaterWindow->show();
        }
    }
#endif
}

MainWindow::~MainWindow()
{
    AppSettings settings;
    settings.setMainWindowGeometry(saveGeometry());
    settings.setAutoTranslateEnabled(ui->autoTranslateCheckBox->isChecked());
    delete ui;
}

void MainWindow::on_translateButton_clicked()
{
    if (ui->sourceEdit->toPlainText().isEmpty()) {
        ui->translationEdit->clear();
        translationButtons->setLanguage(0, QOnlineTranslator::Auto);
        return;
    }

    // Disable the translation button to prevent re-pressing
    ui->translateButton->setEnabled(false);

    // Stop translation playback
    translationPlayer->stop();

    // Source Language
    QOnlineTranslator::Language sourceLang;
    if (ui->autoSourceButton->isChecked())
        sourceLang = QOnlineTranslator::Auto;
    else
        sourceLang = sourceButtons->checkedLanguage();

    // Translation language
    AppSettings settings;
    QOnlineTranslator::Language translationLang;
    if (ui->autoTranslationButton->isChecked()) {
        // Use primary target language from settings
        translationLang = settings.primaryLanguage();
        if (translationLang == QOnlineTranslator::Auto)
            translationLang = uiLang;
        if (translationLang == sourceLang) {
            // If primary language is equal to source language, than use secondary language
            translationLang = settings.secondaryLanguage();
            if (translationLang == QOnlineTranslator::Auto)
                translationLang = uiLang;
        }
    } else {
        translationLang = translationButtons->checkedLanguage();
    }

    // Get translation
    if (!translate(translationLang, sourceLang))
        return;

    // Re-translate to a secondary or a primary language if the autodetected source language and the source language are the same
    if (ui->autoTranslationButton->isChecked() && onlineTranslator->sourceLanguage() == onlineTranslator->translationLanguage()) {
        auto primaryLanguage = settings.primaryLanguage();
        auto secondaryLanguage = settings.secondaryLanguage();
        if (primaryLanguage == QOnlineTranslator::Auto)
            primaryLanguage = uiLang;
        if (secondaryLanguage == QOnlineTranslator::Auto)
            secondaryLanguage = uiLang;

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
        sourceButtons->setLanguage(0, onlineTranslator->sourceLanguage());
        connect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    }

    if (ui->autoTranslationButton->isChecked())
        translationButtons->setLanguage(0, onlineTranslator->translationLanguage());
    else
        translationButtons->setLanguage(0, QOnlineTranslator::Auto);

    // Show translation and transcription
    ui->translationEdit->setHtml(onlineTranslator->translation().toHtmlEscaped().replace("\n", "<br>"));
    if (!onlineTranslator->translationTranslit().isEmpty() && settings.showTranslationTranslit())
        ui->translationEdit->append("<font color=\"grey\"><i>/" + onlineTranslator->translationTranslit().replace("\n", "/<br>/") + "/</i></font>");

    if (!onlineTranslator->sourceTranslit().isEmpty() && settings.showSourceTranslit())
        ui->translationEdit->append("<font color=\"grey\"><i><b>(" + onlineTranslator->sourceTranslit().replace("\n", "/<br>/") + ")</b></i></font>");

    ui->translationEdit->append("");

    // Show translation options
    if (!onlineTranslator->dictionaryList().isEmpty() && settings.showTranslationOptions()) {
        ui->translationEdit->append("<font color=\"grey\"><i>" + onlineTranslator->source() + "</i> – " + tr("translation options:") + "</font>");
        foreach (const auto translationOptions, onlineTranslator->dictionaryList()) {
            ui->translationEdit->append("<b>" + translationOptions.typeOfSpeech() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            ui->translationEdit->textCursor().setBlockFormat(indent);

            for (int i = 0; i <  translationOptions.count(); ++i) {
                QString wordLine;
                if (!translationOptions.gender(i).isEmpty())
                    wordLine.append("<i>" + translationOptions.gender(i) + "</i> ");
                wordLine.append(translationOptions.word(i) + ": ");

                wordLine.append("<font color=\"grey\"><i>");
                wordLine.append(translationOptions.translations(i));
                wordLine.append("</i></font>");
                ui->translationEdit->append(wordLine);
            }

            indent.setTextIndent(0);
            ui->translationEdit->textCursor().setBlockFormat(indent);
            ui->translationEdit->append("");
        }
    }

    // Show definitions
    if (!onlineTranslator->definitionsList().isEmpty() && settings.showDefinitions()) {
        ui->translationEdit->append("<font color=\"grey\"><i>" + onlineTranslator->source() + "</i> – " + tr("definitions:") + "</font>");
        foreach (const auto definition, onlineTranslator->definitionsList()) {
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
    auto sourceLang = sourceButtons->checkedLanguage();
    auto translationLang = translationButtons->checkedLanguage();
    int checkedSourceButton = sourceButtons->checkedId();
    int checkedTranslationButton = translationButtons->checkedId();

    // Insert current translation language to source buttons
    if (checkedTranslationButton == 0)
        sourceButtons->checkButton(0); // Select "Auto" button
    else
        sourceButtons->insertLanguage(translationLang);

    // Insert current source language to translation buttons
    if (checkedSourceButton == 0)
        translationButtons->checkButton(0); // Select "Auto" button
    else
        translationButtons->insertLanguage(sourceLang);

    // Copy translation to source text
    ui->sourceEdit->setPlainText(onlineTranslator->translation());
    ui->sourceEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(languagesMenu, this);
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

void MainWindow::on_playSourceButton_clicked()
{

    switch (sourcePlayer->state()) {
    case QMediaPlayer::PlayingState:
        sourcePlayer->pause();
        break;
    case QMediaPlayer::PausedState:
        sourcePlayer->play();
        break;
    case QMediaPlayer::StoppedState:
        play(sourcePlayer, sourcePlaylist, ui->sourceEdit->toPlainText(), sourceButtons->checkedLanguage());
        break;
    }
}

void MainWindow::on_playTranslationButton_clicked()
{
    switch (translationPlayer->state()) {
    case QMediaPlayer::PlayingState:
        translationPlayer->pause();
        break;
    case QMediaPlayer::PausedState:
        translationPlayer->play();
        break;
    case QMediaPlayer::StoppedState:
        play(translationPlayer, translationPlaylist, onlineTranslator->translation(), onlineTranslator->translationLanguage());
        break;
    }
}

void MainWindow::on_stopSourceButton_clicked()
{
    sourcePlayer->stop();
}

void MainWindow::on_stopTranslationButton_clicked()
{
    translationPlayer->stop();
}

void MainWindow::on_copySourceButton_clicked()
{
    if (!ui->sourceEdit->toPlainText().isEmpty())
        QApplication::clipboard()->setText(onlineTranslator->source());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_copyTranslationButton_clicked()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        QApplication::clipboard()->setText(onlineTranslator->translation());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_copyAllTranslationButton_clicked()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        QApplication::clipboard()->setText(ui->translationEdit->toPlainText());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_autoSourceButton_triggered(QAction *language)
{
    sourceButtons->insertLanguage(language->data().value<QOnlineTranslator::Language>());
}

void MainWindow::on_autoTranslationButton_triggered(QAction *language)
{
    translationButtons->insertLanguage(language->data().value<QOnlineTranslator::Language>());
}

void MainWindow::translateSelectedText()
{
    // Prevent pressing the translation hotkey again
    translateSelectionHotkey->blockSignals(true);

    // Stop previous playback
    sourcePlayer->stop();
    translationPlayer->stop();

    AppSettings settings;
    if (this->isHidden() && settings.windowMode() == AppSettings::PopupWindow) {
        // Show popup
        PopupWindow *popup = new PopupWindow(languagesMenu, sourceButtons, translationButtons, this);

        // Connect main window events to popup events
        connect(sourceButtons, &LangButtonGroup::buttonChecked, popup->sourceButtons(), &LangButtonGroup::checkButton);
        connect(sourceButtons, &LangButtonGroup::languageChanged, popup->sourceButtons(),  &LangButtonGroup::setLanguage);
        connect(translationButtons, &LangButtonGroup::buttonChecked, popup->translationButtons(), &LangButtonGroup::checkButton);
        connect(translationButtons, &LangButtonGroup::languageChanged, popup->translationButtons(),  &LangButtonGroup::setLanguage);
        connect(this, &MainWindow::translationTextChanged, popup->translationEdit(), &QTextEdit::setHtml);
        connect(this, &MainWindow::playSourceButtonIconChanged, popup->playSourceButton(), &QToolButton::setIcon);
        connect(this, &MainWindow::stopSourceButtonEnabled, popup->stopSourceButton(), &QToolButton::setEnabled);
        connect(this, &MainWindow::playTranslationButtonIconChanged, popup->playTranslationButton(), &QToolButton::setIcon);
        connect(this, &MainWindow::stopTranslationButtonEnabled, popup->stopTranslationButton(), &QToolButton::setEnabled);

        // Connect popup events
        connect(popup->sourceButtons(), &LangButtonGroup::buttonChecked, sourceButtons, &LangButtonGroup::checkButton);
        connect(popup->translationButtons(), &LangButtonGroup::buttonChecked, translationButtons, &LangButtonGroup::checkButton);
        connect(popup->autoSourceButton(), &QToolButton::triggered, this, &MainWindow::on_autoSourceButton_triggered);
        connect(popup->autoTranslationButton(), &QToolButton::triggered, this, &MainWindow::on_autoTranslationButton_triggered);
        connect(popup->swapButton(), &QToolButton::clicked, this, &MainWindow::on_swapButton_clicked);
        connect(popup->playSourceButton(), &QToolButton::clicked, this, &MainWindow::on_playSourceButton_clicked);
        connect(popup->stopSourceButton(), &QToolButton::clicked, sourcePlayer, &QMediaPlayer::stop);
        connect(popup->copySourceButton(), &QToolButton::clicked, this, &MainWindow::on_copySourceButton_clicked);
        connect(popup->playTranslationButton(), &QToolButton::clicked, this, &MainWindow::on_playTranslationButton_clicked);
        connect(popup->stopTranslationButton(), &QToolButton::clicked, translationPlayer, &QMediaPlayer::stop);
        connect(popup->copyTranslationButton(), &QToolButton::clicked, this, &MainWindow::on_copyTranslationButton_clicked);
        connect(popup->copyAllTranslationButton(), &QToolButton::clicked, this, &MainWindow::on_copyAllTranslationButton_clicked);
        connect(popup, &PopupWindow::destroyed, sourcePlayer, &QMediaPlayer::stop);
        connect(popup, &PopupWindow::destroyed, translationPlayer, &QMediaPlayer::stop);

        // Restore the keyboard shortcut
        connect(popup, &PopupWindow::destroyed, [this] {
            translateSelectionHotkey->blockSignals(false);
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
        translateSelectionHotkey->blockSignals(false);
    }
}

void MainWindow::copyTranslatedSelection()
{
    ui->sourceEdit->setPlainText(selectedText());

    on_translateButton_clicked();

    if (onlineTranslator->error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to translate text"), onlineTranslator->errorString());
        errorMessage.exec();
        return;
    }

    QApplication::clipboard()->setText(onlineTranslator->translation());
}

void MainWindow::playSelection()
{
    play(sourcePlayer, sourcePlaylist, selectedText());
}

void MainWindow::playTranslatedSelection()
{
    const QString selection = selectedText();

    // Detect languages
    AppSettings settings;
    QOnlineTranslator::Language primaryLanguage = settings.primaryLanguage();
    if (primaryLanguage == QOnlineTranslator::Auto)
        primaryLanguage = uiLang;

    // Translate text
    if (!translateOutside(selection, primaryLanguage))
        return;

    if (onlineTranslator->sourceLanguage() == primaryLanguage) {
        QOnlineTranslator::Language secondaryLanguage = settings.secondaryLanguage();
        if (secondaryLanguage == QOnlineTranslator::Auto)
            secondaryLanguage = uiLang;

        if (!translateOutside(selection, secondaryLanguage))
            return;
    }

    play(selectionPlayer, selectionPlaylist, onlineTranslator->translation(), onlineTranslator->translationLanguage());
}

void MainWindow::checkLanguageButton(LangButtonGroup *checkedGroup, LangButtonGroup *anotherGroup, int id)
{
    // If the target and source languages are the same (and they are not automatic translation buttons), then source target language to previous target language
    AppSettings settings;
    if (id != 0
            && anotherGroup->checkedId() != 0
            && checkedGroup->language(id) == anotherGroup->checkedLanguage()) {
        int previousCheckedButton = settings.checkedButton(checkedGroup);
        anotherGroup->insertLanguage(checkedGroup->language(previousCheckedButton));
        settings.setCheckedButton(anotherGroup, anotherGroup->checkedId());
    }

    // Translate the text automatically if "Automatically translate" is checked or if a pop-up window is open
    if (ui->autoTranslateCheckBox->isChecked() || this->isHidden())
        translateTimer->start(SHORT_AUTOTRANSLATE_DELAY);

    settings.setCheckedButton(checkedGroup, checkedGroup->checkedId());
}

void MainWindow::resetAutoSourceButtonText()
{
    disconnect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    sourceButtons->setLanguage(0, QOnlineTranslator::Auto);
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
        translationPlayer->stop();
        selectionPlayer->stop();
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
        sourcePlayer->stop();
        selectionPlayer->stop();
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
        sourcePlayer->stop();
        translationPlayer->stop();
    }
}

void MainWindow::startTranslateTimer()
{
    translateTimer->start(AUTOTRANSLATE_DELAY);
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
    showNormal();
    activateWindow();
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

void MainWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LocaleChange:
    {
        // System language chaged
        AppSettings settings;
        QLocale::Language lang = settings.locale();
        if (lang == QLocale::AnyLanguage)
            settings.loadLocale(lang); // Reload language if application use system language
        break;
    }
    case QEvent::LanguageChange:
        // Reload UI if application language changed
        ui->retranslateUi(this);

        sourceButtons->retranslate();
        translationButtons->retranslate();

        trayMenu->actions().at(0)->setText(tr("Show window"));
        trayMenu->actions().at(1)->setText(tr("Settings"));
        trayMenu->actions().at(2)->setText(tr("Exit"));

        languagesMenu->clear();
        languagesMenu->addActions(languagesList());
        break;
    default:
        QMainWindow::changeEvent(event);
    }
}

// Translate text in window
bool MainWindow::translate(QOnlineTranslator::Language translationLang, QOnlineTranslator::Language sourceLang)
{
    onlineTranslator->translate(ui->sourceEdit->toPlainText(), QOnlineTranslator::Google, translationLang, sourceLang, uiLang);

    // Check for error
    if (onlineTranslator->error()) {
        ui->translationEdit->setHtml(onlineTranslator->errorString());
        ui->translateButton->setEnabled(true);
        sourceButtons->setLanguage(0, QOnlineTranslator::Auto);
        emit translationTextChanged(onlineTranslator->errorString());
        return false;
    } else {
        return true;
    }
}

// Translate text outside the window
bool MainWindow::translateOutside(const QString &text, QOnlineTranslator::Language translationLang)
{
    onlineTranslator->translate(text, QOnlineTranslator::Google, translationLang);

    if (onlineTranslator->error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to translate text"), onlineTranslator->errorString());
        errorMessage.exec();
        return false;
    } else {
        return true;
    }
}

void MainWindow::loadSettings()
{
    AppSettings settings;

    // Autotranslation
    ui->autoTranslateCheckBox->setChecked(settings.isAutoTranslateEnabled());

    // System tray icon
    QString iconName = settings.trayIconName();
    if (iconName == "custom") {
        QIcon customIcon(settings.customIconPath());
        if (customIcon.isNull())
            trayIcon->setIcon(QIcon::fromTheme("dialog-error"));
        else
            trayIcon->setIcon(customIcon);
    } else {
        QIcon crowIcon = QIcon::fromTheme(iconName);
        if (crowIcon.isNull())
            trayIcon->setIcon(QIcon::fromTheme("dialog-error"));
        else
            trayIcon->setIcon(crowIcon);
    }

    bool traIconVisible = settings.isTrayIconVisible();
    trayIcon->setVisible(traIconVisible);
    QApplication::setQuitOnLastWindowClosed(!traIconVisible);

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
    Qt::ToolButtonStyle languagesStyle = settings.windowLanguagesStyle();
    ui->firstSourceButton->setToolButtonStyle(languagesStyle);
    ui->secondSourceButton->setToolButtonStyle(languagesStyle);
    ui->thirdSourceButton->setToolButtonStyle(languagesStyle);
    ui->firstTranslationButton->setToolButtonStyle(languagesStyle);
    ui->secondTranslationButton->setToolButtonStyle(languagesStyle);
    ui->thirdTranslationButton->setToolButtonStyle(languagesStyle);

    // Control buttons style
    Qt::ToolButtonStyle controlsStyle = settings.windowControlsStyle();
    ui->playSourceButton->setToolButtonStyle(controlsStyle);
    ui->stopSourceButton->setToolButtonStyle(controlsStyle);
    ui->copySourceButton->setToolButtonStyle(controlsStyle);
    ui->playTranslationButton->setToolButtonStyle(controlsStyle);
    ui->stopTranslationButton->setToolButtonStyle(controlsStyle);
    ui->copyTranslationButton->setToolButtonStyle(controlsStyle);
    ui->copyAllTranslationButton->setToolButtonStyle(controlsStyle);
    ui->settingsButton->setToolButtonStyle(controlsStyle);

    // Global shortcuts
    translateSelectionHotkey->setShortcut(settings.translateSelectionHotkey(), true);
    playSelectionHotkey->setShortcut(settings.playSelectionHotkey(), true);
    stopSelectionHotkey->setShortcut(settings.stopSelectionHotkey(), true);
    playTranslatedSelectionHotkey->setShortcut(settings.playTranslatedSelectionHotkey(), true);
    showMainWindowHotkey->setShortcut(settings.showMainWindowHotkey(), true);
    copyTranslatedSelectionHotkey->setShortcut(settings.copyTranslatedSelectionHotkey(), true);

    // Window shortcuts
    ui->translateButton->setShortcut(settings.translateHotkey());
    ui->playSourceButton->setShortcut(settings.playSourceHotkey());
    ui->stopSourceButton->setShortcut(settings.stopSourceHotkey());
    ui->playTranslationButton->setShortcut(settings.playTranslationHotkey());
    ui->stopTranslationButton->setShortcut(settings.stopTranslationHotkey());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationHotkey());
    closeWindowsShortcut->setKey(settings.closeWindowHotkey());
}

void MainWindow::play(QMediaPlayer *player, QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang)
{
    if (text.isEmpty()) {
        QMessageBox errorMessage(QMessageBox::Information, tr("Nothing to play"), tr("Playback text is empty"));
        errorMessage.exec();
        return;
    }

    playlist->clear();
    auto media = onlineTranslator->media(text, QOnlineTranslator::Google, lang);
    if (onlineTranslator->error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to play text"), onlineTranslator->errorString());
        errorMessage.exec();
        return;
    }

    playlist->addMedia(media);
    player->play();
}

QList<QAction *> MainWindow::languagesList()
{
    // Load all languages and codes from QOnlineTranslator
    QList<QAction *> languagesList;
    for (int i = 1; i != QOnlineTranslator::Zulu; ++i) {
        auto Language = static_cast<QOnlineTranslator::Language>(i);
        QAction *action = new QAction();
        action->setText(QOnlineTranslator::languageString(Language));
        action->setIcon(QIcon(":/icons/flags/" + onlineTranslator->languageCode(Language) + ".svg"));
        action->setData(Language);
        languagesList.append(action);
    }

    // Sort alphabetically
    std::sort(languagesList.begin() + 1, languagesList.end(), [](QAction *first, QAction *second) {
        return first->text() < second->text();
    });

    return languagesList;
}

QString MainWindow::selectedText()
{
    QString selectedText;
#if defined(Q_OS_LINUX)
    selectedText = QApplication::clipboard()->text(QClipboard::Selection);
#elif defined(Q_OS_WIN) // Send Ctrl + C to get selected text
    // Save original clipboard data
    QVariant originalClipboard;
    if (QApplication::clipboard()->mimeData()->hasImage())
        originalClipboard = QApplication::clipboard()->image();
    else
        originalClipboard = QApplication::clipboard()->text();

    // Wait until the hot key is pressed
    while (GetAsyncKeyState(translateSelectionHotkey->currentNativeShortcut().key) || GetAsyncKeyState(VK_CONTROL)
           || GetAsyncKeyState(VK_MENU) || GetAsyncKeyState(VK_SHIFT))
        Sleep(50);

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

    // Wait for the clipboard to change
    QEventLoop loop;
    QTimer timer; // Add a timer for the case where the text is not selected
    loop.connect(QApplication::clipboard(), &QClipboard::changed, &loop, &QEventLoop::quit);
    loop.connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(1000);
    loop.exec();

    // Translate the text from the clipboard if the selected text was not copied
    if (timer.isActive())
        return QApplication::clipboard()->text();
    else
        timer.stop();

    // Get clipboard data
    selectedText = QApplication::clipboard()->text();

    // Return original clipboard
    if (originalClipboard.type() == QVariant::Image)
        QApplication::clipboard()->setImage(originalClipboard.value<QImage>());
    else
        QApplication::clipboard()->setText(originalClipboard.toString());
#endif
    return selectedText;
}
