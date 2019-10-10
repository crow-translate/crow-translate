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
#include "qtaskbarcontrol.h"
#include "singleapplication.h"
#include "settings/settingsdialog.h"
#include "settings/appsettings.h"
#include "transitions/translatorabortedtransition.h"
#include "transitions/translatorerrortransition.h"
#include "transitions/textemptytransition.h"
#include "transitions/languagedetectedtransition.h"
#include "transitions/retranslationtransition.h"
#ifdef Q_OS_WIN
#include "updaterdialog.h"
#include "qgittag.h"
#endif

#include <QClipboard>
#include <QShortcut>
#include <QNetworkProxy>
#include <QMessageBox>
#include <QMenu>
#include <QMediaPlaylist>
#include <QStateMachine>
#include <QFinalState>
#ifdef Q_OS_WIN
#include <QMimeData>
#include <QThread>
#include <QTimer>
#include <QDate>

#include <windows.h>
#endif

MainWindow::MainWindow(const AppSettings &settings, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Show a message that the application is already running
    connect(qobject_cast<SingleApplication*>(SingleApplication::instance()), &SingleApplication::instanceStarted, this, &MainWindow::showAppRunningMessage);

    // Text speaking
    ui->sourcePlayerButtons->setMediaPlayer(new QMediaPlayer);
    ui->translationPlayerButtons->setMediaPlayer(new QMediaPlayer);

    // Taskbar progress for text speaking
    m_taskbar = new QTaskbarControl(this);
#if defined(Q_OS_LINUX)
    m_taskbar->setAttribute(QTaskbarControl::LinuxDesktopFile, "crow-translate.desktop");
#endif
    connect(ui->sourcePlayerButtons, &PlayerButtons::stateChanged, this, &MainWindow::setTaskbarState);
    connect(ui->translationPlayerButtons, &PlayerButtons::stateChanged, this, &MainWindow::setTaskbarState);
    connect(ui->sourcePlayerButtons, &PlayerButtons::positionChanged, m_taskbar, &QTaskbarControl::setProgress);
    connect(ui->translationPlayerButtons, &PlayerButtons::positionChanged, m_taskbar, &QTaskbarControl::setProgress);

    // Shortcuts
    m_translateSelectionHotkey = new QHotkey(this);
    m_playSelectionHotkey = new QHotkey(this);
    m_playTranslatedSelectionHotkey = new QHotkey(this);
    m_stopSpeakingHotkey = new QHotkey(this);
    m_showMainWindowHotkey = new QHotkey(this);
    m_copyTranslatedSelectionHotkey = new QHotkey(this);
    m_closeWindowsShortcut = new QShortcut(this);
    connect(m_showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::activate);
    connect(m_closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);
    connect(m_stopSpeakingHotkey, &QHotkey::activated, ui->sourcePlayerButtons, &PlayerButtons::stop);
    connect(m_stopSpeakingHotkey, &QHotkey::activated, ui->translationPlayerButtons, &PlayerButtons::stop);

    // Source button group
    m_sourceLangButtons = new LangButtonGroup(LangButtonGroup::Source, this);
    m_sourceLangButtons->addButton(ui->autoSourceButton);
    m_sourceLangButtons->addButton(ui->firstSourceButton);
    m_sourceLangButtons->addButton(ui->secondSourceButton);
    m_sourceLangButtons->addButton(ui->thirdSourceButton);
    m_sourceLangButtons->loadLanguages(settings);
    connect(m_sourceLangButtons, &LangButtonGroup::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->sourceEdit, &SourceTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);

    // Translation button group
    m_translationLangButtons = new LangButtonGroup(LangButtonGroup::Translation, this);
    m_translationLangButtons->addButton(ui->autoTranslationButton);
    m_translationLangButtons->addButton(ui->firstTranslationButton);
    m_translationLangButtons->addButton(ui->secondTranslationButton);
    m_translationLangButtons->addButton(ui->thirdTranslationButton);
    m_translationLangButtons->loadLanguages(settings);
    connect(m_translationLangButtons, &LangButtonGroup::buttonChecked, this, &MainWindow::checkLanguageButton);

    // System tray icon
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    m_trayMenu->addAction(QIcon::fromTheme("dialog-object-properties"), tr("Settings"), this, &MainWindow::openSettings);
    m_trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), SingleApplication::instance(), &SingleApplication::quit);
    m_trayIcon = new TrayIcon(this);
    m_trayIcon->setContextMenu(m_trayMenu);

    // State machine to handle translator signals async
    m_translator = new QOnlineTranslator(this);
    m_stateMachine = new QStateMachine(this);
    buildStateMachine();
    m_stateMachine->start();

    // Load app settings
    loadSettings(settings);

    // Load main window settings
    ui->autoTranslateCheckBox->setChecked(settings.isAutoTranslateEnabled());
    ui->engineComboBox->setCurrentIndex(settings.currentEngine());
    restoreGeometry(settings.mainWindowGeometry());
    if (!settings.isStartMinimized())
        show();

#ifdef Q_OS_WIN
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
        connect(release, &QGitTag::finished, this, &MainWindow::checkForUpdates);
        release->get("crow-translate", "crow-translate");
    }
#endif
}

MainWindow::~MainWindow()
{
    AppSettings settings;
    settings.setMainWindowGeometry(saveGeometry());
    settings.setAutoTranslateEnabled(ui->autoTranslateCheckBox->isChecked());
    settings.setCurrentEngine(currentEngine());

    m_sourceLangButtons->saveLanguages(settings);
    m_translationLangButtons->saveLanguages(settings);
    delete ui;
}

void MainWindow::activate()
{
    ui->sourceEdit->setFocus();

    show();
    activateWindow();
    raise();
}

QComboBox *MainWindow::engineCombobox()
{
    return ui->engineComboBox;
}

TranslationEdit *MainWindow::translationEdit()
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
    return ui->sourcePlayerButtons;
}

PlayerButtons *MainWindow::translationPlayerButtons()
{
    return ui->translationPlayerButtons;
}

void MainWindow::requestTranslation()
{
    QOnlineTranslator::Language translationLang;
    if (m_translationLangButtons->isAutoButtonChecked())
        translationLang = AppSettings().preferredTranslationLanguage(m_sourceLangButtons->checkedLanguage());
    else
        translationLang = m_translationLangButtons->checkedLanguage();

    m_translator->translate(ui->sourceEdit->toPlainText(), currentEngine(), translationLang, m_sourceLangButtons->checkedLanguage());
}

// Re-translate to a secondary or a primary language if the autodetected source language and the translation language are the same
void MainWindow::requestRetranslation()
{
    const QOnlineTranslator::Language translationLang = AppSettings().preferredTranslationLanguage(m_translator->sourceLanguage());

    m_translator->translate(ui->sourceEdit->toPlainText(), currentEngine(), translationLang, m_translator->sourceLanguage());
}

void MainWindow::parseTranslation()
{
    if (!ui->translationEdit->parseTranslationData(m_translator)) {
        // Reset language on translation "Auto" button
        m_translationLangButtons->setLanguage(0, QOnlineTranslator::Auto);
        return;
    }

    // Display languages on "Auto" buttons
    if (ui->autoSourceButton->isChecked())
        m_sourceLangButtons->setLanguage(0, m_translator->sourceLanguage());

    if (ui->autoTranslationButton->isChecked())
        m_translationLangButtons->setLanguage(0, m_translator->translationLanguage());
    else
        m_translationLangButtons->setLanguage(0, QOnlineTranslator::Auto);
}

void MainWindow::clearTranslation()
{
    ui->translationEdit->clearTranslation();
    m_translationLangButtons->setLanguage(0, QOnlineTranslator::Auto);
}

void MainWindow::requestSourceLanguage()
{
    m_translator->detectLanguage(ui->sourceEdit->toPlainText(), currentEngine());
}

void MainWindow::parseSourceLanguage()
{
    if (m_translator->error()) {
        QMessageBox::critical(this, tr("Unable to detect language"), m_translator->errorString());
        return;
    }

    m_sourceLangButtons->setLanguage(0, m_translator->sourceLanguage());
}

void MainWindow::speakSource()
{
    ui->sourcePlayerButtons->play(ui->sourceEdit->toPlainText(), m_sourceLangButtons->checkedLanguage(), currentEngine());
}

void MainWindow::speakTranslation()
{
    ui->translationPlayerButtons->play(ui->translationEdit->translation(), ui->translationEdit->translationLanguage(), currentEngine());
}

void MainWindow::showTranslationWindow()
{
    // Prevent pressing the translation hotkey again
    m_translateSelectionHotkey->blockSignals(true);

    const AppSettings settings;
    if (this->isHidden() && settings.windowMode() == AppSettings::PopupWindow) {
        auto *popup = new PopupWindow(this);
        popup->show();
        popup->activateWindow();

        if (!ui->autoTranslateCheckBox->isChecked())
            ui->sourceEdit->setListenForChanges(true);

        // Restore the keyboard shortcut
        connect(popup, &PopupWindow::destroyed, [&] {
            m_translateSelectionHotkey->blockSignals(false);
            if (!ui->autoTranslateCheckBox->isChecked())
                ui->sourceEdit->setListenForChanges(false);
        });
    } else {
        activate();

        // Restore the keyboard shortcut
        m_translateSelectionHotkey->blockSignals(false);
    }
}

void MainWindow::setSelectionAsSource()
{
    ui->sourceEdit->setListenForChanges(false);
    ui->sourceEdit->setPlainText(selectedText());
    if (ui->autoTranslateCheckBox->isChecked())
        ui->sourceEdit->setListenForChanges(true);
}

void MainWindow::copyTranslationToClipboard()
{
    if (m_translator->error()) {
        QMessageBox::critical(this, tr("Unable to translate text"), m_translator->errorString());
        return;
    }

    SingleApplication::clipboard()->setText(ui->translationEdit->translation());
}

void MainWindow::forceSourceAutodetect()
{
    const AppSettings settings;
    if (settings.isForceSourceAutodetect()) {
        ui->sourceEdit->setListenForChanges(false);

        m_sourceLangButtons->checkAutoButton();

        if (ui->autoTranslateCheckBox->isChecked())
            ui->sourceEdit->setListenForChanges(true);
    }
}

void MainWindow::forceAutodetect()
{
    const AppSettings settings;
    ui->sourceEdit->setListenForChanges(false);

    if (settings.isForceTranslationAutodetect())
        m_translationLangButtons->checkAutoButton();

    if (settings.isForceSourceAutodetect())
        m_sourceLangButtons->checkAutoButton();

    if (ui->autoTranslateCheckBox->isChecked())
        ui->sourceEdit->setListenForChanges(true);
}

void MainWindow::clearText()
{
    // Clear source text without tracking for changes
    ui->sourceEdit->setListenForChanges(false);
    ui->sourceEdit->clear();
    if (ui->autoTranslateCheckBox->isChecked())
        ui->sourceEdit->setListenForChanges(true);

    clearTranslation();
}

void MainWindow::abortTranslation()
{
    m_translator->abort();
}

void MainWindow::swapLanguages()
{
    // Temporary disable toggle logic
    disconnect(m_translationLangButtons, &LangButtonGroup::buttonChecked, this, &MainWindow::checkLanguageButton);
    disconnect(m_sourceLangButtons, &LangButtonGroup::buttonChecked, this, &MainWindow::checkLanguageButton);

    // Backup source buttons properties
    const QOnlineTranslator::Language sourceLang = m_sourceLangButtons->checkedLanguage();
    const bool isSourceAutoButtonChecked = m_sourceLangButtons->isAutoButtonChecked();

    // Insert current translation language to source buttons
    if (m_translationLangButtons->isAutoButtonChecked())
        m_sourceLangButtons->checkAutoButton();
    else
        m_sourceLangButtons->addLanguage(m_translationLangButtons->checkedLanguage());

    // Insert current source language to translation buttons
    if (isSourceAutoButtonChecked)
        m_translationLangButtons->checkAutoButton();
    else
        m_translationLangButtons->addLanguage(sourceLang);

    // Re-enable toggle logic
    connect(m_translationLangButtons, &LangButtonGroup::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(m_sourceLangButtons, &LangButtonGroup::buttonChecked, this, &MainWindow::checkLanguageButton);

    // Copy translation to source text
    ui->sourceEdit->setPlainText(ui->translationEdit->translation());
    ui->sourceEdit->moveCursor(QTextCursor::End);
}

void MainWindow::openSettings()
{
    SettingsDialog config(this);
    if (config.exec() == QDialog::Accepted) {
        const AppSettings settings;
        loadSettings(settings);
    }
}

void MainWindow::setAutoTranslateEnabled(bool enabled)
{
    ui->sourceEdit->setListenForChanges(enabled);
    if (enabled)
        ui->sourceEdit->markSourceAsChanged();
}

void MainWindow::copySourceText()
{
    if (!ui->sourceEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(ui->sourceEdit->toPlainText());
}

void MainWindow::copyTranslation()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(ui->translationEdit->translation());
}

void MainWindow::copyAllTranslationInfo()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        SingleApplication::clipboard()->setText(ui->translationEdit->toPlainText());
}

void MainWindow::addSourceLanguage()
{
    AddLangDialog langDialog(this);
    if (langDialog.exec() == QDialog::Accepted)
        m_sourceLangButtons->addLanguage(langDialog.language());
}

void MainWindow::addTranslationLanguage()
{
    AddLangDialog langDialog(this);
    if (langDialog.exec() == QDialog::Accepted)
        m_translationLangButtons->addLanguage(langDialog.language());
}

void MainWindow::resetAutoSourceButtonText()
{
    m_sourceLangButtons->setLanguage(0, QOnlineTranslator::Auto);
}

void MainWindow::setTaskbarState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_taskbar->setProgressVisible(true);
#if defined(Q_OS_WIN)
        m_taskbar->setAttribute(QTaskbarControl::WindowsProgressState, QTaskbarControl::Running);
#endif
        break;
    case QMediaPlayer::PausedState:
#if defined(Q_OS_WIN)
        m_taskbar->setAttribute(QTaskbarControl::WindowsProgressState, QTaskbarControl::Paused);
#endif
        break;
    case QMediaPlayer::StoppedState:
        m_taskbar->setProgressVisible(false);
#if defined(Q_OS_WIN)
        m_taskbar->setAttribute(QTaskbarControl::WindowsProgressState, QTaskbarControl::Stopped);
#endif
        break;
    }
}

void MainWindow::showAppRunningMessage()
{
    auto *message = new QMessageBox(QMessageBox::Information, SingleApplication::applicationName(), tr("The application is already running"));
    message->setAttribute(Qt::WA_DeleteOnClose); // Need to allocate on heap to avoid crash!
    activate();
    message->show();
}

#ifdef Q_OS_WIN
void MainWindow::checkForUpdates()
{
    auto *release = qobject_cast<QGitTag *>(sender());
    release->deleteLater();
    if (release->error())
        return;

    const int installer = release->assetId(".exe");
    if (installer != -1 && SingleApplication::applicationVersion() < release->tagName()) {
        auto *updater = new UpdaterDialog(release, installer, this);
        updater->setAttribute(Qt::WA_DeleteOnClose);
        updater->open();
    }

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

void MainWindow::buildStateMachine()
{
    m_stateMachine->setGlobalRestorePolicy(QStateMachine::RestoreProperties);

    auto *idleState = new QState(m_stateMachine);
    auto *translationState = new QState(m_stateMachine);
    auto *translateSelectionState = new QState(m_stateMachine);
    auto *speakSourceState = new QState(m_stateMachine);
    auto *speakTranslationState = new QState(m_stateMachine);
    auto *speakSelectionState = new QState(m_stateMachine);
    auto *speakTranslatedSelectionState = new QState(m_stateMachine);
    auto *copyTranslatedSelectionState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(idleState);

    buildTranslationState(translationState);
    buildTranslateSelectionState(translateSelectionState);
    buildSpeakSourceState(speakSourceState);
    buildSpeakTranslationState(speakTranslationState);
    buildSpeakSelectionState(speakSelectionState);
    buildSpeakTranslatedSelectionState(speakTranslatedSelectionState);
    buildCopyTranslatedSelectionState(copyTranslatedSelectionState);

    // Add transitions between all states
    for (QState *state : m_stateMachine->findChildren<QState *>()) {
        state->addTransition(ui->translateButton, &QToolButton::clicked, translationState);
        state->addTransition(ui->sourceEdit, &SourceTextEdit::sourceChanged, translationState);
        state->addTransition(m_translateSelectionHotkey, &QHotkey::activated, translateSelectionState);
        state->addTransition(ui->sourcePlayerButtons, &PlayerButtons::playerMediaRequested, speakSourceState);
        state->addTransition(ui->translationPlayerButtons, &PlayerButtons::playerMediaRequested, speakTranslationState);
        state->addTransition(m_playSelectionHotkey, &QHotkey::activated, speakSelectionState);
        state->addTransition(m_playTranslatedSelectionHotkey, &QHotkey::activated, speakTranslatedSelectionState);
        state->addTransition(m_copyTranslatedSelectionHotkey, &QHotkey::activated, copyTranslatedSelectionState);
    }

    translationState->addTransition(translationState, &QState::finished, idleState);
    speakSourceState->addTransition(speakSourceState, &QState::finished, idleState);
    speakTranslationState->addTransition(speakTranslationState, &QState::finished, idleState);
    translateSelectionState->addTransition(translateSelectionState, &QState::finished, idleState);
    speakSelectionState->addTransition(speakSelectionState, &QState::finished, idleState);
    speakTranslatedSelectionState->addTransition(speakTranslatedSelectionState, &QState::finished, idleState);
}

void MainWindow::buildTranslationState(QState *state)
{
    auto *abortPreviousState = new QState(state);
    auto *requestState = new QState(state);
    auto *checkLanguagesState = new QState(state);
    auto *requestInOtherLanguageState = new QState(state);
    auto *parseState = new QState(state);
    auto *clearTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(abortPreviousState);

    connect(abortPreviousState, &QState::entered, m_translator, &QOnlineTranslator::abort);
    connect(abortPreviousState, &QState::entered, ui->translationPlayerButtons, &PlayerButtons::stop); // Stop translation speaking
    connect(requestState, &QState::entered, this, &MainWindow::requestTranslation);
    connect(requestInOtherLanguageState, &QState::entered, this, &MainWindow::requestRetranslation);
    connect(parseState, &QState::entered, this, &MainWindow::parseTranslation);
    connect(clearTranslationState, &QState::entered, this, &MainWindow::clearTranslation);
    setupRequestStateButtons(requestState);
    setupRequestStateButtons(abortPreviousState);

    auto *noTextTransition = new TextEmptyTransition(ui->sourceEdit, abortPreviousState);
    noTextTransition->setTargetState(clearTranslationState);

    auto *translationRunningTransition = new TranslatorAbortedTransition(m_translator, abortPreviousState);
    translationRunningTransition->setTargetState(requestState);

    auto *otherLanguageTransition = new RetranslationTransition(m_translator, m_translationLangButtons, checkLanguagesState);
    otherLanguageTransition->setTargetState(requestInOtherLanguageState);

    requestState->addTransition(m_translator, &QOnlineTranslator::finished, checkLanguagesState);
    checkLanguagesState->addTransition(parseState);
    requestInOtherLanguageState->addTransition(m_translator, &QOnlineTranslator::finished, parseState);
    parseState->addTransition(finalState);
    clearTranslationState->addTransition(finalState);
}

void MainWindow::buildSpeakSourceState(QState *state)
{
    auto *initialState = new QState(state);
    auto *abortPreviousState = new QState(state);
    auto *requestLangState = new QState(state);
    auto *parseLangState = new QState(state);
    auto *speakTextState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(initialState);

    connect(abortPreviousState, &QState::entered, m_translator, &QOnlineTranslator::abort);
    connect(requestLangState, &QState::entered, this, &MainWindow::requestSourceLanguage);
    connect(parseLangState, &QState::entered, this, &MainWindow::parseSourceLanguage);
    connect(speakTextState, &QState::entered, this, &MainWindow::speakSource);
    setupRequestStateButtons(requestLangState);

    auto *langDetectedTransition = new LanguageDetectedTransition(m_sourceLangButtons, initialState);
    langDetectedTransition->setTargetState(speakTextState);

    auto *translationRunningTransition = new TranslatorAbortedTransition(m_translator, abortPreviousState);
    translationRunningTransition->setTargetState(requestLangState);

    auto *errorTransition = new TranslatorErrorTransition(m_translator, parseLangState);
    errorTransition->setTargetState(finalState);

    initialState->addTransition(abortPreviousState);
    requestLangState->addTransition(m_translator, &QOnlineTranslator::finished, parseLangState);
    parseLangState->addTransition(speakTextState);
    speakTextState->addTransition(finalState);
}

void MainWindow::buildTranslateSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *showWindowState = new QState(state);
    auto *translationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::setSelectionAsSource);
    connect(showWindowState, &QState::entered, this, &MainWindow::showTranslationWindow);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(showWindowState);
    showWindowState->addTransition(translationState);
    translationState->addTransition(translationState, &QState::finished, finalState);
}

void MainWindow::buildSpeakTranslationState(QState *state)
{
    auto *speakTextState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(speakTextState);

    connect(speakTextState, &QState::entered, this, &MainWindow::speakTranslation);

    speakTextState->addTransition(finalState);
}

void MainWindow::buildSpeakSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *speakSourceState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::setSelectionAsSource);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceSourceAutodetect);
    buildSpeakSourceState(speakSourceState);

    setSelectionAsSourceState->addTransition(speakSourceState);
    speakSourceState->addTransition(speakSourceState, &QState::finished, finalState);
}

void MainWindow::buildSpeakTranslatedSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *translationState = new QState(state);
    auto *speakTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::setSelectionAsSource);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceAutodetect);
    buildSpeakTranslationState(speakTranslationState);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(translationState);
    translationState->addTransition(translationState, &QState::finished, speakTranslationState);
    speakTranslationState->addTransition(speakTranslationState, &QState::finished, finalState);
}

void MainWindow::buildCopyTranslatedSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *translationState = new QState(state);
    auto *copyTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::setSelectionAsSource);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceAutodetect);
    connect(copyTranslationState, &QState::entered, this, &MainWindow::copyTranslationToClipboard);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(translationState);
    translationState->addTransition(translationState, &QState::finished, copyTranslationState);
    copyTranslationState->addTransition(finalState);
}

void MainWindow::setupRequestStateButtons(QState *state)
{
    state->assignProperty(ui->translateButton, "enabled", false);
    state->assignProperty(ui->clearButton, "enabled", false);
    state->assignProperty(ui->abortButton, "enabled", true);
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

    // TTS
    ui->sourcePlayerButtons->setVoice(QOnlineTranslator::Yandex, settings.voice(QOnlineTranslator::Yandex));
    ui->sourcePlayerButtons->setEmotion(QOnlineTranslator::Yandex, settings.emotion(QOnlineTranslator::Yandex));

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
    ui->sourcePlayerButtons->setButtonsStyle(controlsStyle);
    ui->translationPlayerButtons->setButtonsStyle(controlsStyle);
    ui->copySourceButton->setToolButtonStyle(controlsStyle);
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
    ui->sourcePlayerButtons->setPlayPauseShortcut(settings.speakSourceHotkey());
    ui->translationPlayerButtons->setPlayPauseShortcut(settings.speakTranslationHotkey());
    ui->translateButton->setShortcut(settings.translateHotkey());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationHotkey());
    m_closeWindowsShortcut->setKey(settings.closeWindowHotkey());
}

// Toggle language logic
void MainWindow::checkLanguageButton(int checkedId)
{
    LangButtonGroup *checkedGroup;
    LangButtonGroup *anotherGroup;
    if (sender() == m_sourceLangButtons) {
        checkedGroup = m_sourceLangButtons;
        anotherGroup = m_translationLangButtons;
    } else {
        checkedGroup = m_translationLangButtons;
        anotherGroup = m_sourceLangButtons;
    }

    /* If the target and source languages are the same (and they are not autodetect buttons),
     * then insert previous checked language from just checked language group to another group */
    const QOnlineTranslator::Language checkedLang = checkedGroup->language(checkedId);
    if (checkedId != 0 && checkedLang == anotherGroup->checkedLanguage() && !anotherGroup->isAutoButtonChecked()) {
        anotherGroup->addLanguage(checkedGroup->previousCheckedLanguage());
        return;
    }

    // Check if selected language is supported by engine
    if (!QOnlineTranslator::isSupportTranslation(currentEngine(), checkedLang)) {
        for (int i = 0; i < ui->engineComboBox->count(); ++i) {
            if (QOnlineTranslator::isSupportTranslation(static_cast<QOnlineTranslator::Engine>(i), checkedLang)) {
                ui->engineComboBox->setCurrentIndex(i); // Check first supported language
                break;
            }
        }
        return;
    }

    ui->sourceEdit->markSourceAsChanged();
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

    // Wait until the hot key is unpressed
    QVector<int> nativeKeyCodes;
    nativeKeyCodes.append(static_cast<int>(m_translateSelectionHotkey->currentNativeShortcut().key));
    if (m_translateSelectionHotkey->modifiers().testFlag(Qt::ShiftModifier))
        nativeKeyCodes.append(VK_SHIFT);
    if (m_translateSelectionHotkey->modifiers().testFlag(Qt::ControlModifier))
        nativeKeyCodes.append(VK_CONTROL);
    if (m_translateSelectionHotkey->modifiers().testFlag(Qt::MetaModifier))
        nativeKeyCodes.append({VK_LWIN, VK_RWIN});
    if (m_translateSelectionHotkey->modifiers().testFlag(Qt::AltModifier))
        nativeKeyCodes.append(VK_MENU);
    while(std::any_of(nativeKeyCodes.begin(), nativeKeyCodes.end(), [](int keyCode) {
            return GetAsyncKeyState(keyCode);
    }));

    // Generate Ctrl + C input
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
    SendInput(std::size(copyText), copyText, sizeof(INPUT));

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

QOnlineTranslator::Engine MainWindow::currentEngine()
{
    return static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
}
