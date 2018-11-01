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
#include <QSettings>
#include <QMessageBox>

#if defined(Q_OS_WIN)
#include <QMimeData>
#include <QTimer>
#include <windows.h>

#include "updaterwindow.h"
#endif

#include "ui_mainwindow.h"
#include "popupwindow.h"
#include "settingsdialog.h"

constexpr int AUTOTRANSLATE_TIMEOUT = 500;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow),
    interfaceTranslator (new QTranslator(this)),
    sourcePlayer (new QMediaPlayer(this)),
    translationPlayer (new QMediaPlayer(this)),
    selectionPlayer (new QMediaPlayer(this)),
    sourcePlaylist (new QMediaPlaylist(this)),
    translationPlaylist (new QMediaPlaylist(this)),
    selectionPlaylist (new QMediaPlaylist(this)),
    trayMenu (new QMenu(this)),
    trayIcon (new QSystemTrayIcon(this)),
    languagesMenu (new QMenu(this)),
    autoTranslateTimer (new QTimer(this)),
    closeWindowsShortcut (new QShortcut(this)),
    translateSelectionHotkey (new QHotkey(this)),
    playSelectionHotkey (new QHotkey(this)),
    playTranslatedSelectionHotkey (new QHotkey(this)),
    stopSelectionHotkey (new QHotkey(this)),
    showMainWindowHotkey (new QHotkey(this)),
    sourceButtonGroup (new QButtonGroup(this)),
    translationButtonGroup (new QButtonGroup(this))
{
    loadLocale(); // Load application locale

    onlineTranslator = new QOnlineTranslator(this); // Need to create after localization

    ui->setupUi(this);

    // Shortcuts
    connect(closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);
    connect(showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::showMainWindow);
    connect(playSelectionHotkey, &QHotkey::activated, this, &MainWindow::playSelection);
    connect(playTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::playTranslatedSelection);
    connect(stopSelectionHotkey, &QHotkey::activated, selectionPlayer, &QMediaPlayer::stop);
    connect(translateSelectionHotkey, &QHotkey::activated, this, &MainWindow::showPopupWindow);

    // Text speaking
    sourcePlayer->setPlaylist(sourcePlaylist); // Use playlist to split long queries due Google limit
    translationPlayer->setPlaylist(translationPlaylist);
    selectionPlayer->setPlaylist(selectionPlaylist);
    connect(ui->sourceEdit, &QPlainTextEdit::textChanged, sourcePlayer, &QMediaPlayer::stop);

    // Dynamic players buttons
    connect(sourcePlayer, &QMediaPlayer::stateChanged, [&](QMediaPlayer::State state) {
        switch (state) {
        case QMediaPlayer::PlayingState:
            ui->playSourceButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            emit playSourceButtonIconChanged(ui->playSourceButton->icon());
            ui->stopSourceButton->setEnabled(true);
            emit stopSourceButtonEnabled(true);
            break;
        case QMediaPlayer::PausedState:
            ui->playSourceButton->setIcon(QIcon::fromTheme("media-playback-start"));
            emit playSourceButtonIconChanged(ui->playSourceButton->icon());
            break;
        case QMediaPlayer::StoppedState:
            ui->playSourceButton->setIcon(QIcon::fromTheme("media-playback-start"));
            emit playSourceButtonIconChanged(ui->playSourceButton->icon());
            ui->stopSourceButton->setEnabled(false);
            emit stopSourceButtonEnabled(false);
            break;
        }
    });
    connect(translationPlayer, &QMediaPlayer::stateChanged, [&](QMediaPlayer::State state) {
        switch (state) {
        case QMediaPlayer::PlayingState:
            ui->playTranslationButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            emit playTranslationButtonIconChanged(ui->playTranslationButton->icon());
            ui->stopTranslationButton->setEnabled(true);
            emit stopTranslationButtonEnabled(true);
            break;
        case QMediaPlayer::PausedState:
            ui->playTranslationButton->setIcon(QIcon::fromTheme("media-playback-start"));
            emit playTranslationButtonIconChanged(ui->playTranslationButton->icon());
            break;
        case QMediaPlayer::StoppedState:
            ui->playTranslationButton->setIcon(QIcon::fromTheme("media-playback-start"));
            emit playTranslationButtonIconChanged(ui->playTranslationButton->icon());
            ui->stopTranslationButton->setEnabled(false);
            emit stopTranslationButtonEnabled(false);
            break;
        }
    });

    // Source button group
    sourceButtonGroup->addButton(ui->autoSourceButton, 0);
    sourceButtonGroup->addButton(ui->firstSourceButton, 1);
    sourceButtonGroup->addButton(ui->secondSourceButton, 2);
    sourceButtonGroup->addButton(ui->thirdSourceButton, 3);
    sourceButtonGroup->setProperty("GroupCategory", "Source");
    connect(sourceButtonGroup, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &MainWindow::toggleSourceButton);

    // Translation button group
    translationButtonGroup->addButton(ui->autoTranslationButton, 0);
    translationButtonGroup->addButton(ui->firstTranslationButton, 1);
    translationButtonGroup->addButton(ui->secondTranslationButton, 2);
    translationButtonGroup->addButton(ui->thirdTranslationButton, 3);
    translationButtonGroup->setProperty("GroupCategory", "Source");
    connect(translationButtonGroup, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &MainWindow::toggleTranslationButton);

    // System tray icon
    trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    trayMenu->addAction(QIcon::fromTheme("dialog-object-properties"), tr("Settings"), this, &MainWindow::on_settingsButton_clicked);
    trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), qApp, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (!this->isVisible())
                showMainWindow();
            else
                hide();
        }
    });

    // Timer for automatic translation
    autoTranslateTimer->setSingleShot(true);
    connect(ui->sourceEdit, &QPlainTextEdit::textChanged, [&]() {
        autoTranslateTimer->start(AUTOTRANSLATE_TIMEOUT);
    });
    connect(autoTranslateTimer, &QTimer::timeout, [&]() {
        if (ui->translateButton->isEnabled())
            on_translateButton_clicked();
        else
            autoTranslateTimer->start(AUTOTRANSLATE_TIMEOUT);
    });

    // Add languages menu to auto-language buttons
    languagesMenu->addActions(languagesList());
    ui->autoSourceButton->setMenu(languagesMenu);
    ui->autoTranslationButton->setMenu(languagesMenu);

    // Load settings
    loadLanguageButtons(sourceButtonGroup);
    loadLanguageButtons(translationButtonGroup);
    loadSettings();
    setProxy();
    QSettings settings;
    restoreGeometry(settings.value("MainWindowGeometry").toByteArray());

    // Check for updates
#if defined(Q_OS_WIN)
    int updateInterval = settings.value("CheckForUpdatesInterval", 1).toInt();
    if (updateInterval < Interval::Never) {
        QDate checkDate = settings.value("LastUpdateCheckDate", QDate::currentDate()).toDate();
        switch (updateInterval) {
        case Interval::Day:
        {
            checkDate = checkDate.addDays(1);
            break;
        }
        case Interval::Week:
        {
            checkDate = checkDate.addDays(7);
            break;
        }
        case Interval::Month:
        {
            checkDate = checkDate.addMonths(1);
            break;
        }
        }
        if (QDate::currentDate() >= checkDate) {
            QGitRelease release("Shatur95", "Crow-Translate");
            if (!release.error() && qApp->applicationVersion() == release.tagName()) {
                UpdaterWindow *updaterWindow = new UpdaterWindow(release, this);
                updaterWindow->show();
            }
        }
    }
#endif
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindowGeometry", saveGeometry());
    delete ui;
}

void MainWindow::on_translateButton_clicked()
{
    if (ui->sourceEdit->toPlainText().isEmpty()) {
        ui->translationEdit->clear();
        return;
    }

    // Disable the translation button to prevent re-pressing
    ui->translateButton->setEnabled(false);

    // Stop translation playback
    translationPlayer->stop();

    // Source Language
    QSettings settings;
    auto sourceLang = sourceButtonGroup->checkedButton()->property("Lang").value<QOnlineTranslator::Language>();

    // Translation language
    QOnlineTranslator::Language translationLang;
    if (ui->autoTranslationButton->isChecked()) {
        // Use primary target language from settings
        translationLang = settings.value("Translation/PrimaryLanguage", QOnlineTranslator::Auto).value<QOnlineTranslator::Language>();
        if (translationLang == QOnlineTranslator::Auto)
            translationLang = uiLang;
        if (translationLang == sourceLang) {
            // If primary language is equal to source language, than use secondary language
            translationLang = settings.value("Translation/SecondaryLanguage", QOnlineTranslator::Auto).value<QOnlineTranslator::Language>();
            if (translationLang == QOnlineTranslator::Auto)
                translationLang = uiLang;
        }
    } else {
        translationLang = translationButtonGroup->checkedButton()->property("Lang").value<QOnlineTranslator::Language>();
    }

    // Get translation
    if (!translate(translationLang, sourceLang))
        return;

    // Re-translate to a secondary or a primary language if the autodetected source language and the source language are the same
    if (ui->autoTranslationButton->isChecked() && onlineTranslator->sourceLanguage() == onlineTranslator->translationLanguage()) {
        auto primaryLanguage = settings.value("Translation/PrimaryLanguage", QOnlineTranslator::Auto).value<QOnlineTranslator::Language>();
        auto secondaryLanguage = settings.value("Translation/SecondaryLanguage", QOnlineTranslator::English).value<QOnlineTranslator::Language>();
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
        if (onlineTranslator->sourceLanguage() != sourceButtonGroup->checkedButton()->property("Lang").toInt()) {
            ui->autoSourceButton->setText(tr("Auto") + " (" + onlineTranslator->sourceLanguageString() + ")");
            ui->autoSourceButton->setProperty("Lang", onlineTranslator->sourceLanguage());
            emit sourceButtonChanged(ui->autoSourceButton, 0);
            connect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
        }
    }
    if (ui->autoTranslationButton->isChecked()) {
        if (onlineTranslator->translationLanguage() != translationButtonGroup->checkedButton()->property("Lang").toInt()) {
            ui->autoTranslationButton->setText(tr("Auto") + " (" + onlineTranslator->translationLanguageString() + ")");
            ui->autoTranslationButton->setProperty("Lang", onlineTranslator->translationLanguage());
            emit translationButtonChanged(ui->autoTranslationButton, 0);
        }
    } else {
        if (ui->autoTranslationButton->property("Lang").toInt() != QOnlineTranslator::Auto) {
            ui->autoTranslationButton->setText(tr("Auto"));
            ui->autoTranslationButton->setProperty("Lang", QOnlineTranslator::Auto);
            emit translationButtonChanged(ui->autoTranslationButton, 0);
        }
    }

    // Show translation and transcription
    ui->translationEdit->setHtml(onlineTranslator->translation().toHtmlEscaped().replace("\n", "<br>"));
    if (!onlineTranslator->translationTranslit().isEmpty() && settings.value("Translation/ShowTranslationTransliteration", true).toBool())
        ui->translationEdit->append("<font color=\"grey\"><i>/" + onlineTranslator->translationTranslit().replace("\n", "/<br>/") + "/</i></font>");

    if (!onlineTranslator->sourceTranslit().isEmpty() && settings.value("Translation/ShowSourceTransliteration", true).toBool())
        ui->translationEdit->append("<font color=\"grey\"><i><b>(" + onlineTranslator->sourceTranslit().replace("\n", "/<br>/") + ")</b></i></font>");

    ui->translationEdit->append("");

    // Show translation options
    if (!onlineTranslator->dictionaryList().isEmpty() && settings.value("Translation/TranslationOptions", true).toBool()) {
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
    if (!onlineTranslator->definitionsList().isEmpty() && settings.value("Translation/Definitions", true).toBool()) {
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

    // Insert current translation language to source buttons
    if (sourceButtonGroup->checkedId() == 0) {
        sourceButtonGroup->button(0)->setChecked(true); // Select "Auto" button
    } else {
        auto translationLang = translationButtonGroup->checkedButton()->property("Lang").value<QOnlineTranslator::Language>();
        insertLanguage(sourceButtonGroup, translationLang);
    }

    // Insert current source language to translation buttons
    if (translationButtonGroup->checkedId() == 0) {
        translationButtonGroup->button(0)->setChecked(true); // Select "Auto" button
    } else {
        auto sourceLang = sourceButtonGroup->checkedButton()->property("Lang").value<QOnlineTranslator::Language>();
        insertLanguage(translationButtonGroup, sourceLang);
    }

    // Copy translation to source text
    ui->sourceEdit->setPlainText(onlineTranslator->translation());
    ui->sourceEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(languagesMenu, this);
    if (config.exec()) {
        if (config.localizationChanged()) {
            loadLocale();

            // Reload UI
            ui->retranslateUi(this);

            delete onlineTranslator;
            onlineTranslator = new QOnlineTranslator(this);

            trayMenu->actions().at(0)->setText(tr("Show window"));
            trayMenu->actions().at(1)->setText(tr("Settings"));
            trayMenu->actions().at(2)->setText(tr("Exit"));

            languagesMenu->clear();
            languagesMenu->addActions(languagesList());

            loadLanguageButtons(sourceButtonGroup);
            loadLanguageButtons(translationButtonGroup);

            auto sourceLang = ui->autoSourceButton->property("Lang").value<QOnlineTranslator::Language>();
            auto translationLang = ui->autoTranslationButton->property("Lang").value<QOnlineTranslator::Language>();
            ui->autoSourceButton->setText(tr("Auto") + " (" + onlineTranslator->languageString(sourceLang) + ")");
            ui->autoTranslationButton->setText(tr("Auto") + " (" + onlineTranslator->languageString(translationLang) + ")");
        }

        if (config.proxyChanged())
            setProxy();

        loadSettings();
    }
}

void MainWindow::on_playSourceButton_clicked()
{
    if (ui->sourceEdit->toPlainText().isEmpty()) {
        qDebug() << tr("Text field is empty");
        return;
    }

    switch (sourcePlayer->state()) {
    case QMediaPlayer::PlayingState:
        sourcePlayer->pause();
        break;
    case QMediaPlayer::PausedState:
        // Pause other players
        if (translationPlayer->state() == QMediaPlayer::PlayingState)
            translationPlayer->pause();
        else if (selectionPlayer->state() == QMediaPlayer::PlayingState)
            selectionPlayer->pause();

        sourcePlayer->play();
        break;
    case QMediaPlayer::StoppedState:
        // Pause other players
        if (translationPlayer->state() == QMediaPlayer::PlayingState)
            translationPlayer->pause();
        else if (selectionPlayer->state() == QMediaPlayer::PlayingState)
            selectionPlayer->pause();

        play(sourcePlayer, sourcePlaylist, ui->sourceEdit->toPlainText(), sourceButtonGroup->checkedButton()->property("Lang").value<QOnlineTranslator::Language>());
        break;
    }
}

void MainWindow::on_playTranslationButton_clicked()
{
    if (ui->translationEdit->toPlainText().isEmpty()) {
        qDebug() << tr("Text field is empty");
        return;
    }

    switch (translationPlayer->state()) {
    case QMediaPlayer::PlayingState:
        translationPlayer->pause();
        break;
    case QMediaPlayer::PausedState:
        if (sourcePlayer->state() == QMediaPlayer::PlayingState)
            sourcePlayer->pause();
        else if (selectionPlayer->state() == QMediaPlayer::PlayingState)
            selectionPlayer->pause();

        translationPlayer->play();
        break;
    case QMediaPlayer::StoppedState:
        if (sourcePlayer->state() == QMediaPlayer::PlayingState)
            sourcePlayer->pause();
        else if (selectionPlayer->state() == QMediaPlayer::PlayingState)
            selectionPlayer->pause();

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
    insertLanguage(sourceButtonGroup, language->data().value<QOnlineTranslator::Language>());
}

void MainWindow::on_autoTranslationButton_triggered(QAction *language)
{
    insertLanguage(translationButtonGroup, language->data().value<QOnlineTranslator::Language>());
}

void MainWindow::on_autoTranslateCheckBox_toggled(bool checked)
{
    if (checked) {
        if (ui->translateButton->isEnabled())
            on_translateButton_clicked();
        else
            autoTranslateTimer->start(AUTOTRANSLATE_TIMEOUT);
    }

    autoTranslateTimer->blockSignals(!checked);

    QSettings settings;
    settings.setValue("AutoTranslate", checked);
}

void MainWindow::showMainWindow()
{
    showNormal();
    activateWindow();
}

void MainWindow::showPopupWindow()
{
    // Prevent pressing the translation hotkey again
    translateSelectionHotkey->blockSignals(true);

    // Stop previous playback
    sourcePlayer->stop();
    translationPlayer->stop();

    QSettings settings;
    if (this->isHidden() && settings.value("WindowMode", 0).toInt() == 0) {
        // Show popup
        PopupWindow *popup = new PopupWindow(languagesMenu, sourceButtonGroup, translationButtonGroup, this);
        connect(this, &MainWindow::translationTextChanged, popup->translationEdit(), &QTextEdit::setHtml);

        connect(this, &MainWindow::sourceButtonChanged, popup, &PopupWindow::loadSourceButton);
        connect(this, &MainWindow::translationButtonChanged, popup, &PopupWindow::loadTranslationButton);

        connect(this, &MainWindow::playSourceButtonIconChanged, popup->playSourceButton(), &QToolButton::setIcon);
        connect(this, &MainWindow::stopSourceButtonEnabled, popup->stopSourceButton(), &QToolButton::setEnabled);
        connect(this, &MainWindow::playTranslationButtonIconChanged, popup->playTranslationButton(), &QToolButton::setIcon);
        connect(this, &MainWindow::stopTranslationButtonEnabled, popup->stopTranslationButton(), &QToolButton::setEnabled);

        connect(sourceButtonGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), popup, &PopupWindow::checkSourceButton);
        connect(translationButtonGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), popup, &PopupWindow::checkTranslationButton);

        connect(popup, &PopupWindow::destroyed, sourcePlayer, &QMediaPlayer::stop);
        connect(popup, &PopupWindow::destroyed, translationPlayer, &QMediaPlayer::stop);

        connect(popup->sourceButtons(),  qOverload<int, bool>(&QButtonGroup::buttonToggled), this, &MainWindow::checkSourceButton);
        connect(popup->translationButtons(),  qOverload<int, bool>(&QButtonGroup::buttonToggled), this, &MainWindow::checkTranslationButton);

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

        // Restore the keyboard shortcut
        connect(popup, &PopupWindow::destroyed, [this] {
            translateSelectionHotkey->blockSignals(false);
        });

        // Send selected text to source field and translate it
        if (ui->autoTranslateCheckBox->isChecked()) {
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

        showMainWindow();
        on_translateButton_clicked();

        // Restore the keyboard shortcut
        translateSelectionHotkey->blockSignals(false);
    }
}

void MainWindow::playSelection()
{
    QString selection = selectedText();
    if (selection.isEmpty()) {
        qDebug() << tr("The selection does not contain text");
        return;
    }
    // Pause other players
    if (translationPlayer->state() == QMediaPlayer::PlayingState)
        translationPlayer->pause();
    else if (sourcePlayer->state() == QMediaPlayer::PlayingState)
        sourcePlayer->pause();

    play(sourcePlayer, sourcePlaylist, selection);
}

void MainWindow::playTranslatedSelection()
{
    QString selection = selectedText();
    if (selection.isEmpty()) {
        qDebug() << tr("The selection does not contain text");
        return;
    }

    // Pause other players
    if (translationPlayer->state() == QMediaPlayer::PlayingState)
        translationPlayer->pause();
    else
        if (sourcePlayer->state() == QMediaPlayer::PlayingState)
            sourcePlayer->pause();

    // Detect languages
    QSettings settings;
    auto primaryLanguage = settings.value("Translation/PrimaryLanguage", QOnlineTranslator::Auto).value<QOnlineTranslator::Language>();
    if (primaryLanguage == QOnlineTranslator::Auto)
        primaryLanguage = uiLang;

    // Translate text
    if (!translateOutside(selection, primaryLanguage))
        return;

    if (onlineTranslator->sourceLanguage() == primaryLanguage) {
        auto secondaryLanguage = settings.value("Translation/SecondaryLanguage", QOnlineTranslator::Auto).value<QOnlineTranslator::Language>();
        if (secondaryLanguage == QOnlineTranslator::Auto)
            secondaryLanguage = uiLang;

        if (!translateOutside(selection, secondaryLanguage))
            return;
    }

    play(selectionPlayer, selectionPlaylist, onlineTranslator->translation(), onlineTranslator->translationLanguage());
}

void MainWindow::checkSourceButton(int id, bool checked)
{
    if (checked)
        sourceButtonGroup->button(id)->setChecked(true);
}

void MainWindow::checkTranslationButton(int id, bool checked)
{
    if (checked)
        translationButtonGroup->button(id)->setChecked(true);
}

void MainWindow::toggleSourceButton(QAbstractButton *button, bool checked)
{
    if (checked) {
        // If the target and source languages are the same (and they are not automatic translation buttons), then change target language to previous source language
        QSettings settings;
        if (sourceButtonGroup->id(button) != 0
                && translationButtonGroup->checkedId() != 0
                && button->property("Lang") == translationButtonGroup->checkedButton()->property("Lang")) {
            int previousCheckedButton = settings.value("Buttons/CheckedSourceButton", 0).toInt();
            insertLanguage(translationButtonGroup, sourceButtonGroup->button(previousCheckedButton)->property("Lang").value<QOnlineTranslator::Language>());
            settings.setValue("Buttons/CheckedTranslationButton", translationButtonGroup->checkedId()); // Save the pressed button
        }

        // Translate the text automatically if "Automatically translate" is checked or if a pop-up window is open
        if (ui->autoTranslateCheckBox->isChecked() || this->isHidden())
            autoTranslateTimer->start(300);

        settings.setValue("Buttons/CheckedSourceButton", sourceButtonGroup->checkedId());
    }
}

void MainWindow::toggleTranslationButton(QAbstractButton *button, bool checked)
{
    if (checked) {
        // If the target and source languages are the same (and they are not automatic translation buttons), then source target language to previous target language
        QSettings settings;
        if (translationButtonGroup->id(button) != 0
                && sourceButtonGroup->checkedId() != 0
                && button->property("Lang") == sourceButtonGroup->checkedButton()->property("Lang")) {
            int previousCheckedButton = settings.value("Buttons/CheckedTranslationButton", 0).toInt();
            insertLanguage(sourceButtonGroup, translationButtonGroup->button(previousCheckedButton)->property("Lang").value<QOnlineTranslator::Language>());
            settings.setValue("Buttons/CheckedSourceButton", sourceButtonGroup->checkedId());
        }

        // Translate the text automatically if "Automatically translate" is checked or if a pop-up window is open
        if (ui->autoTranslateCheckBox->isChecked() || this->isHidden())
            autoTranslateTimer->start(300);

        settings.setValue("Buttons/CheckedTranslationButton", translationButtonGroup->checkedId());
    }
}

void MainWindow::resetAutoSourceButtonText()
{
    ui->autoSourceButton->setText(tr("Auto"));
    ui->autoSourceButton->setProperty("Lang", QOnlineTranslator::Auto);
    disconnect(ui->sourceEdit, &QPlainTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);
    emit sourceButtonChanged(ui->autoSourceButton, 0);
}

void MainWindow::setProxy()
{
    QSettings settings;
    QNetworkProxy proxy;
    proxy.setType(static_cast<QNetworkProxy::ProxyType>(settings.value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt()));
    if (proxy.type() == QNetworkProxy::HttpProxy || proxy.type() == QNetworkProxy::Socks5Proxy) {
        proxy.setHostName(settings.value("Connection/ProxyHost", "").toString());
        proxy.setPort(settings.value("Connection/ProxyPort", 8080).value<quint16>());
        if (settings.value("Connection/ProxyAuthEnabled", false).toBool()) {
            proxy.setUser(settings.value("Connection/ProxyUsername", "").toString());
            proxy.setPassword(settings.value("Connection/ProxyPassword", "").toString());
        }
    }
    QNetworkProxy::setApplicationProxy(proxy);
}

void MainWindow::loadLanguageButtons(QButtonGroup *group)
{
    const QString groupCategory = group->property("GroupCategory").toString();

    // Load buttons text and tooltip
    QSettings settings;
    for (int i = 1; i < 4; ++i) {
        auto language = settings.value("Buttons/" + groupCategory + "Button" + QString::number(i), QOnlineTranslator::NoLanguage).value<QOnlineTranslator::Language>();

        // Check if the code is set
        if (language == QOnlineTranslator::NoLanguage) {
            group->button(i)->setVisible(false);
        } else {
            group->button(i)->setIcon(QIcon(":/icons/flags/" + QOnlineTranslator::languageCode(language) + ".svg"));
            group->button(i)->setText(onlineTranslator->languageString(language));
            group->button(i)->setProperty("Lang", language);
            group->button(i)->setVisible(true);
        }
    }

    // Load checked button
    group->button(settings.value("Buttons/Checked" + groupCategory + "Button", 0).toInt())->setChecked(true);
}

void MainWindow::loadSettings()
{
    QSettings settings;

    // Autotranslation
    ui->autoTranslateCheckBox->setChecked(settings.value("AutoTranslate", true).toBool());
    on_autoTranslateCheckBox_toggled(ui->autoTranslateCheckBox->isChecked());

    // System tray icon
    QString iconName = settings.value("TrayIcon", "crow-translate-tray").toString();
    if (iconName == "custom") {
        QIcon customIcon(settings.value("CustomIconPath", "").toString());
        if (customIcon.isNull())
            trayIcon->setIcon(QIcon::fromTheme("dialog-error"));
        else
            trayIcon->setIcon(customIcon);
    }
    else {
        QIcon crowIcon = QIcon::fromTheme(iconName);
        if (crowIcon.isNull())
            trayIcon->setIcon(QIcon::fromTheme("dialog-error"));
        else
            trayIcon->setIcon(crowIcon);
    }
    trayIcon->setVisible(settings.value("TrayIconVisible", true).toBool());
    QApplication::setQuitOnLastWindowClosed(!settings.value("TrayIconVisible", true).toBool());

    // Language buttons style
    ui->firstSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon)));
    ui->secondSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon)));
    ui->thirdSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon)));
    ui->firstTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon)));
    ui->secondTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon)));
    ui->thirdTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon)));

    // Control buttons style
    ui->playSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->stopSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->copySourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->playTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->stopTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->copyTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->copyAllTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));
    ui->settingsButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly)));

    // Shortcuts
    translateSelectionHotkey->setShortcut(settings.value("Hotkeys/TranslateSelection", "Ctrl+Alt+E").toString(), true);
    playSelectionHotkey->setShortcut(settings.value("Hotkeys/PlaySelection", "Ctrl+Alt+S").toString(), true);
    stopSelectionHotkey->setShortcut(settings.value("Hotkeys/StopSelection", "Ctrl+Alt+G").toString(), true);
    playTranslatedSelectionHotkey->setShortcut(settings.value("Hotkeys/PlayTranslatedSelection", "Ctrl+Alt+F").toString(), true);
    showMainWindowHotkey->setShortcut(settings.value("Hotkeys/ShowMainWindow", "Ctrl+Alt+C").toString(), true);
    ui->translateButton->setShortcut(settings.value("Hotkeys/Translate", "Ctrl+Return").toString());
    ui->playSourceButton->setShortcut(settings.value("Hotkeys/PlaySource", "Ctrl+S").toString());
    ui->stopSourceButton->setShortcut(settings.value("Hotkeys/StopSource", "Ctrl+G").toString());
    ui->playTranslationButton->setShortcut(settings.value("Hotkeys/PlayTranslation", "Ctrl+Shift+S").toString());
    ui->stopTranslationButton->setShortcut(settings.value("Hotkeys/StopTranslation", "Ctrl+Shift+G").toString());
    ui->copyTranslationButton->setShortcut(settings.value("Hotkeys/CopyTranslation", "Ctrl+Shift+C").toString());
    closeWindowsShortcut->setKey(settings.value("Hotkeys/CloseWindow", "Ctrl+Q").toString());
}

void MainWindow::loadLocale()
{
    QSettings settings;
    auto locale = settings.value("Language", QLocale::AnyLanguage).value<QLocale::Language>();
    if (locale == QLocale::AnyLanguage)
        QLocale::setDefault(QLocale::system());
    else
        QLocale::setDefault(QLocale(locale));

    if (interfaceTranslator->load(QLocale(), "crow", "_", ":/translations"))
        qApp->installTranslator(interfaceTranslator);

    uiLang = QOnlineTranslator::language(QLocale()); // Language for translator hints
}

void MainWindow::insertLanguage(QButtonGroup *group, QOnlineTranslator::Language language)
{
    // Exit the function if the current language already has a button
    for (int i = 0; i < 4; ++i) {
        if (language == group->button(i)->property("Lang").toInt()) {
            group->button(i)->setChecked(true);
            return;
        }
    }

    const QString groupCategory = group->property("GroupCategory").toString();

    // Shift buttons (2 -> 3, 1 -> 2)
    QSettings settings;
    for (int i = 3; i > 1; --i) {
        // Skip iteration, if previous button is not specified
        if (group->button(i-1)->property("Lang").toInt() == QOnlineTranslator::NoLanguage)
            continue;

        // Set values
        group->button(i)->setText(group->button(i-1)->text());
        group->button(i)->setProperty("Lang", group->button(i-1)->property("Lang"));
        group->button(i)->setIcon(group->button(i-1)->icon());
        group->button(i)->setVisible(true);

        // Send signal
        if (group == sourceButtonGroup)
            emit sourceButtonChanged(group->button(i), i);
        else
            emit translationButtonChanged(group->button(i), i);

        // Save settings
        settings.setValue("Buttons/" + groupCategory + "Button" + QString::number(i), group->button(i)->property("Lang"));
    }

    // Shift checked button in the settings
    if (group->checkedId() != 0 && group->checkedId() != 3)
        settings.setValue("Buttons/Checked" + groupCategory + "Button", settings.value("Buttons/Checked" + groupCategory + "Button", 0).toInt() + 1);

    // Insert new language to first button
    group->button(1)->setText(onlineTranslator->languageString(language));
    group->button(1)->setIcon(QIcon(":/icons/flags/" + QOnlineTranslator::languageCode(language) + ".svg"));
    group->button(1)->setProperty("Lang", language);
    group->button(1)->setVisible(true);

    if (group->button(1)->isChecked())
        emit group->buttonToggled(group->button(1), true);
    else
        group->button(1)->setChecked(true);

    // Send signal
    if (group == sourceButtonGroup)
        emit sourceButtonChanged(group->button(1), 1);
    else
        emit translationButtonChanged(group->button(1), 1);

    // Save first button settings
    settings.setValue("Buttons/" + groupCategory + "Button" + QString::number(1), group->button(1)->property("Lang"));
}

bool MainWindow::translate(QOnlineTranslator::Language translationLang, QOnlineTranslator::Language sourceLang)
{
    onlineTranslator->translate(ui->sourceEdit->toPlainText(), QOnlineTranslator::Google, translationLang, sourceLang, uiLang);

    // Check for error
    if (onlineTranslator->error()) {
        ui->translationEdit->setHtml(onlineTranslator->errorString());
        ui->translateButton->setEnabled(true);

        ui->autoTranslationButton->setText(tr("Auto"));
        ui->autoTranslationButton->setProperty("Lang", QOnlineTranslator::Auto);

        emit translationTextChanged(onlineTranslator->errorString());
        return false;
    } else {
        return true;
    }
}

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

void MainWindow::play(QMediaPlayer *player, QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang)
{
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
        action->setText(onlineTranslator->languageString(Language));
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
