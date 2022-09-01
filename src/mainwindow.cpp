/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "popupwindow.h"
#include "qhotkey.h"
#include "qtaskbarcontrol.h"
#include "screenwatcher.h"
#include "selection.h"
#include "singleapplication.h"
#include "trayicon.h"
#include "ocr/ocr.h"
#include "ocr/screengrabbers/abstractscreengrabber.h"
#include "ocr/snippingarea.h"
#include "settings/settingsdialog.h"
#include "transitions/languagedetectedtransition.h"
#include "transitions/ocruninitializedtransition.h"
#include "transitions/retranslationtransition.h"
#include "transitions/snippingvisibletransition.h"
#include "transitions/textemptytransition.h"
#include "transitions/translatorabortedtransition.h"
#include "transitions/translatorerrortransition.h"
#ifdef Q_OS_WIN
#include "qgittag.h"
#include "updaterdialog.h"
#endif

#include <QClipboard>
#include <QFinalState>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QNetworkProxyFactory>
#include <QScreen>
#include <QShortcut>
#include <QStateMachine>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_translateSelectionHotkey(new QHotkey(this))
    , m_speakSelectionHotkey(new QHotkey(this))
    , m_speakTranslatedSelectionHotkey(new QHotkey(this))
    , m_stopSpeakingHotkey(new QHotkey(this))
    , m_playPauseSpeakingHotkey(new QHotkey(this))
    , m_showMainWindowHotkey(new QHotkey(this))
    , m_copyTranslatedSelectionHotkey(new QHotkey(this))
    , m_recognizeScreenAreaHotkey(new QHotkey(this))
    , m_translateScreenAreaHotkey(new QHotkey(this))
    , m_delayedRecognizeScreenAreaHotkey(new QHotkey(this))
    , m_delayedTranslateScreenAreaHotkey(new QHotkey(this))
    , m_closeWindowsShortcut(new QShortcut(this))
    , m_stateMachine(new QStateMachine(this))
    , m_translator(new QOnlineTranslator(this))
    , m_trayIcon(new TrayIcon(this))
    , m_taskbar(new QTaskbarControl(this))
    , m_ocr(new Ocr(this))
    , m_screenCaptureTimer(new QTimer(this))
    , m_orientationWatcher(new ScreenWatcher(this))
    , m_screenGrabber(AbstractScreenGrabber::createScreenGrabber(this))
    , m_snippingArea(new SnippingArea(m_screenGrabber->ignoreDevicePixelRatio(), this))
{
    ui->setupUi(this);

    // Screen orientation
    connect(m_orientationWatcher, &ScreenWatcher::screenOrientationChanged, this, &MainWindow::setOrientation);

    // Show the main window if a secondary instance has been started
    connect(qobject_cast<SingleApplication *>(QCoreApplication::instance()), &SingleApplication::instanceStarted, this, &MainWindow::open);

    // Selection requests
    connect(&Selection::instance(), &Selection::requestedSelectionAvailable, ui->sourceEdit, &SourceTextEdit::replaceText);

    // Taskbar progress for text speaking
    connect(ui->sourceSpeakButtons, &SpeakButtons::stateChanged, this, &MainWindow::setTaskbarState);
    connect(ui->translationSpeakButtons, &SpeakButtons::stateChanged, this, &MainWindow::setTaskbarState);
    connect(ui->sourceSpeakButtons, &SpeakButtons::positionChanged, m_taskbar, &QTaskbarControl::setProgress);
    connect(ui->translationSpeakButtons, &SpeakButtons::positionChanged, m_taskbar, &QTaskbarControl::setProgress);

    // Shortcuts
    connect(m_closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);
    connect(m_showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::open);
    connect(m_translateSelectionHotkey, &QHotkey::activated, this, &MainWindow::translateSelection);
    connect(m_speakSelectionHotkey, &QHotkey::activated, this, &MainWindow::speakSelection);
    connect(m_speakTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::speakTranslatedSelection);
    connect(m_stopSpeakingHotkey, &QHotkey::activated, this, &MainWindow::stopSpeaking);
    connect(m_playPauseSpeakingHotkey, &QHotkey::activated, this, &MainWindow::playPauseSpeaking);
    connect(m_copyTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::copyTranslatedSelection);
    connect(m_recognizeScreenAreaHotkey, &QHotkey::activated, this, &MainWindow::recognizeScreenArea);
    connect(m_translateScreenAreaHotkey, &QHotkey::activated, this, &MainWindow::translateScreenArea);
    connect(m_delayedRecognizeScreenAreaHotkey, &QHotkey::activated, this, &MainWindow::delayedRecognizeScreenArea);
    connect(m_delayedTranslateScreenAreaHotkey, &QHotkey::activated, this, &MainWindow::delayedTranslateScreenArea);

    // Source and translation logic
    connect(ui->sourceLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->translationLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->sourceEdit, &SourceTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);

    // OCR logic
    connect(m_screenGrabber, &AbstractScreenGrabber::grabbed, m_snippingArea, &SnippingArea::snip);
    connect(m_snippingArea, &SnippingArea::snipped, m_ocr, &Ocr::recognize);
    connect(m_ocr, &Ocr::recognized, ui->sourceEdit, &SourceTextEdit::replaceText);
    m_screenCaptureTimer->setSingleShot(true);

#if defined(Q_OS_WIN)
    // Windows must have a widget be set to display a playback progress
    m_taskbar->setWidget(this);
#endif

    // Setup players for speak buttons
    ui->sourceSpeakButtons->setMediaPlayer(new QMediaPlayer);
    ui->translationSpeakButtons->setMediaPlayer(new QMediaPlayer);

    // State machine to handle translator signals async
    buildStateMachine();
    m_stateMachine->start();

    // App settings
    loadAppSettings();
    loadMainWindowSettings();
}

MainWindow::~MainWindow()
{
    AppSettings settings;
    settings.setMainWindowGeometry(saveGeometry());
    settings.setAutoTranslateEnabled(ui->autoTranslateCheckBox->isChecked());
    settings.setCurrentEngine(currentEngine());
    settings.setLanguages(AppSettings::Source, ui->sourceLanguagesWidget->languages());
    settings.setLanguages(AppSettings::Translation, ui->translationLanguagesWidget->languages());
    settings.setCheckedButton(AppSettings::Source, ui->sourceLanguagesWidget->checkedId());
    settings.setCheckedButton(AppSettings::Translation, ui->translationLanguagesWidget->checkedId());
    delete ui;
}

const QComboBox *MainWindow::engineCombobox() const
{
    return ui->engineComboBox;
}

const TranslationEdit *MainWindow::translationEdit() const
{
    return ui->translationEdit;
}

const QToolButton *MainWindow::swapButton() const
{
    return ui->swapButton;
}

const QToolButton *MainWindow::copySourceButton() const
{
    return ui->copySourceButton;
}

const QToolButton *MainWindow::copyTranslationButton() const
{
    return ui->copyTranslationButton;
}

const QToolButton *MainWindow::copyAllTranslationButton() const
{
    return ui->copyAllTranslationButton;
}

const LanguageButtonsWidget *MainWindow::sourceLanguageButtons() const
{
    return ui->sourceLanguagesWidget;
}

const LanguageButtonsWidget *MainWindow::translationLanguageButtons() const
{
    return ui->translationLanguagesWidget;
}

const SpeakButtons *MainWindow::sourceSpeakButtons() const
{
    return ui->sourceSpeakButtons;
}

const SpeakButtons *MainWindow::translationSpeakButtons() const
{
    return ui->translationSpeakButtons;
}

QKeySequence MainWindow::closeWindowShortcut() const
{
    return m_closeWindowsShortcut->key();
}

Ocr *MainWindow::ocr() const
{
    return m_ocr;
}

void MainWindow::open()
{
    ui->sourceEdit->setFocus();

    setWindowState(windowState() & ~Qt::WindowMinimized);
    show();

    // Required to show the application on some WMs like XFWM
    // if window already opened on different workspace. Doesn't
    // affect KWin.
    raise();

    activateWindow();
}

void MainWindow::translateSelection()
{
    emit translateSelectionRequested();
}

void MainWindow::speakSelection()
{
    emit speakSelectionRequested();
}

void MainWindow::speakTranslatedSelection()
{
    emit speakTranslatedSelectionRequested();
}

void MainWindow::stopSpeaking()
{
    ui->sourceSpeakButtons->stopSpeaking();
    ui->translationSpeakButtons->stopSpeaking();
}

void MainWindow::playPauseSpeaking()
{
    ui->sourceSpeakButtons->playPauseSpeaking();
    ui->translationSpeakButtons->playPauseSpeaking();
}

void MainWindow::copyTranslatedSelection()
{
    emit copyTranslatedSelectionRequested();
}

void MainWindow::recognizeScreenArea()
{
    emit recognizeScreenAreaRequested();
}

void MainWindow::translateScreenArea()
{
    emit translateScreenAreaRequested();
}

Q_SCRIPTABLE void MainWindow::delayedRecognizeScreenArea()
{
    emit delayedRecognizeScreenAreaRequested();
}

Q_SCRIPTABLE void MainWindow::delayedTranslateScreenArea()
{
    emit delayedTranslateScreenAreaRequested();
}

void MainWindow::clearText()
{
    ui->sourceEdit->removeText();
    clearTranslation();
}

void MainWindow::cancelOperation()
{
    m_translator->abort();
    m_ocr->cancel();
}

void MainWindow::swapLanguages()
{
    // Temporary disable toggle logic
    disconnect(ui->translationLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    disconnect(ui->sourceLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);

    LanguageButtonsWidget::swapCurrentLanguages(ui->sourceLanguagesWidget, ui->translationLanguagesWidget);

    // Re-enable toggle logic
    connect(ui->translationLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->sourceLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);

    // Copy translation to source text
    ui->sourceEdit->replaceText(ui->translationEdit->translation());
    markContentAsChanged();
}

void MainWindow::openSettings()
{
    SettingsDialog config(this);
    if (config.exec() == QDialog::Accepted)
        loadAppSettings();
}

void MainWindow::setAutoTranslateEnabled(bool enabled)
{
    ui->autoTranslateCheckBox->setChecked(enabled);
}

void MainWindow::copySourceText()
{
    if (!ui->sourceEdit->toPlainText().isEmpty())
        QGuiApplication::clipboard()->setText(ui->sourceEdit->toPlainText());
}

void MainWindow::copyTranslation()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        QGuiApplication::clipboard()->setText(ui->translationEdit->translation());
}

void MainWindow::copyAllTranslationInfo()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        QGuiApplication::clipboard()->setText(ui->translationEdit->toPlainText());
}

void MainWindow::quit()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    QMetaObject::invokeMethod(QCoreApplication::instance(), &QCoreApplication::quit, Qt::QueuedConnection);
#else
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
#endif
}

void MainWindow::requestTranslation()
{
    QOnlineTranslator::Language translationLang;
    if (ui->translationLanguagesWidget->isAutoButtonChecked())
        translationLang = preferredTranslationLanguage(ui->sourceLanguagesWidget->checkedLanguage());
    else
        translationLang = ui->translationLanguagesWidget->checkedLanguage();

    m_translator->translate(ui->sourceEdit->toSourceText(), currentEngine(), translationLang, ui->sourceLanguagesWidget->checkedLanguage());
}

// Re-translate to a secondary or a primary language if the autodetected source language and the translation language are the same
void MainWindow::requestRetranslation()
{
    const QOnlineTranslator::Language translationLang = preferredTranslationLanguage(m_translator->sourceLanguage());

    m_translator->translate(ui->sourceEdit->toSourceText(), currentEngine(), translationLang, m_translator->sourceLanguage());
}

void MainWindow::displayTranslation()
{
    if (!ui->translationEdit->parseTranslationData(m_translator)) {
        // Reset language on translation "Auto" button
        ui->translationLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);
        return;
    }

    // Display languages on "Auto" buttons
    if (ui->sourceLanguagesWidget->isAutoButtonChecked())
        ui->sourceLanguagesWidget->setAutoLanguage(m_translator->sourceLanguage());

    if (ui->translationLanguagesWidget->isAutoButtonChecked())
        ui->translationLanguagesWidget->setAutoLanguage(m_translator->translationLanguage());
    else
        ui->translationLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);

    // If window mode is notification, send a notification including the translation result
    if (this->isHidden() && m_windowMode == AppSettings::Notification)
        m_trayIcon->showTranslationMessage(ui->translationEdit->toPlainText());
}

void MainWindow::clearTranslation()
{
    ui->translationEdit->clearTranslation();
    ui->translationLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);
}

void MainWindow::requestSourceLanguage()
{
    m_translator->detectLanguage(ui->sourceEdit->toSourceText(), currentEngine());
}

void MainWindow::parseSourceLanguage()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        QMessageBox::critical(this, tr("Unable to detect language"), m_translator->errorString());
        return;
    }

    ui->sourceLanguagesWidget->setAutoLanguage(m_translator->sourceLanguage());
}

void MainWindow::speakSource()
{
    ui->sourceSpeakButtons->speak(ui->sourceEdit->toSourceText(), ui->sourceLanguagesWidget->checkedLanguage(), currentEngine());
}

void MainWindow::speakTranslation()
{
    ui->translationSpeakButtons->speak(ui->translationEdit->translation(), ui->translationEdit->translationLanguage(), currentEngine());
}

void MainWindow::showTranslationWindow()
{
    // Always show main window if it already opened
    if (!isHidden()) {
        open();
        return;
    }

    switch (m_windowMode) {
    case AppSettings::PopupWindow: {
        auto *popup = new PopupWindow(this);
        popup->show();
        popup->activateWindow();

        // Force listening for changes in source field
        if (!m_listenForContentChanges) {
            setListenForContentChanges(true);
            connect(popup, &PopupWindow::destroyed, [this] {
                setListenForContentChanges(ui->autoTranslateCheckBox->isChecked());
            });
        }

        break;
    }
    case AppSettings::MainWindow:
        open();
        break;
    case AppSettings::Notification:
        break;
    }
}

void MainWindow::copyTranslationToClipboard()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        QMessageBox::critical(this, tr("Unable to translate text"), m_translator->errorString());
        return;
    }

    QGuiApplication::clipboard()->setText(ui->translationEdit->translation());
}

void MainWindow::forceSourceAutodetect()
{
    if (m_forceSourceAutodetect) {
        bool before = m_listenForContentChanges;
        m_listenForContentChanges = false;

        ui->sourceLanguagesWidget->checkAutoButton();

        m_listenForContentChanges = before;
    }
}

void MainWindow::forceTranslationAutodetect()
{
    if (m_forceTranslationAutodetect) {
        bool before = m_listenForContentChanges;
        m_listenForContentChanges = false;

        ui->translationLanguagesWidget->checkAutoButton();

        m_listenForContentChanges = before;
    }
}

void MainWindow::minimize()
{
    setWindowState(windowState() | Qt::WindowMinimized);
}

void MainWindow::markContentAsChanged()
{
    if (m_listenForContentChanges) {
        ui->sourceEdit->stopEditTimer();
        emit contentChanged();
    }
}

void MainWindow::setListenForContentChanges(bool listen)
{
    m_listenForContentChanges = listen;
    ui->sourceEdit->setListenForEdits(m_listenForContentChanges);
}

void MainWindow::resetAutoSourceButtonText()
{
    ui->sourceLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);
}

void MainWindow::setTaskbarState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_taskbar->setProgressVisible(true);
#if defined(Q_OS_WIN)
        m_taskbar->setWindowsProgressState(QTaskbarControl::Running);
#endif
        break;
    case QMediaPlayer::PausedState:
#if defined(Q_OS_WIN)
        m_taskbar->setWindowsProgressState(QTaskbarControl::Paused);
#endif
        break;
    case QMediaPlayer::StoppedState:
        m_taskbar->setProgressVisible(false);
#if defined(Q_OS_WIN)
        m_taskbar->setWindowsProgressState(QTaskbarControl::Stopped);
#endif
        break;
    }
}

void MainWindow::setOrientation(Qt::ScreenOrientation orientation)
{
    if (orientation == Qt::PrimaryOrientation)
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        orientation = screen()->orientation();
#else
        orientation = QGuiApplication::primaryScreen()->orientation();
#endif

    switch (orientation) {
    case Qt::LandscapeOrientation:
    case Qt::InvertedLandscapeOrientation:
        ui->centralLayout->setDirection(QBoxLayout::LeftToRight);
        ui->translationButtonsLayout->setDirection(QBoxLayout::LeftToRight);
        ui->translationLanguagesWidget->setLayoutDirection(Qt::RightToLeft);
        break;
    case Qt::PortraitOrientation:
        ui->centralLayout->setDirection(QBoxLayout::TopToBottom);
        ui->translationButtonsLayout->setDirection(QBoxLayout::RightToLeft);
        ui->translationLanguagesWidget->setLayoutDirection(Qt::LeftToRight);
        break;
    case Qt::InvertedPortraitOrientation:
        ui->centralLayout->setDirection(QBoxLayout::BottomToTop);
        ui->translationButtonsLayout->setDirection(QBoxLayout::RightToLeft);
        ui->translationLanguagesWidget->setLayoutDirection(Qt::LeftToRight);
        break;
    default:
        Q_UNREACHABLE(); // Will never be called with Qt::PrimaryOrientation
    }
}

#ifdef Q_OS_WIN
void MainWindow::checkForUpdates()
{
    auto *release = qobject_cast<QGitTag *>(sender());
    release->deleteLater();
    if (release->error())
        return;

    const int installer = release->assetId(".exe");
    if (installer != -1 && QCoreApplication::applicationVersion() < release->tagName()) {
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
    case QEvent::LocaleChange: {
        // System language chaged
        if (const QLocale locale = AppSettings().locale(); locale == AppSettings::defaultLocale())
            AppSettings::applyLocale(locale); // Reload language if application use system language
        break;
    }
    case QEvent::LanguageChange:
        // Reload UI if application language changed
        ui->retranslateUi(this);
        m_trayIcon->retranslateMenu();
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
    auto *recognizeScreenAreaState = new QState(m_stateMachine);
    auto *translateScreenAreaState = new QState(m_stateMachine);
    auto *delayedRecognizeScreenAreaState = new QState(m_stateMachine);
    auto *delayedTranslateScreenAreaState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(idleState);

    buildTranslationState(translationState);
    buildTranslateSelectionState(translateSelectionState);
    buildSpeakSourceState(speakSourceState);
    buildSpeakTranslationState(speakTranslationState);
    buildSpeakSelectionState(speakSelectionState);
    buildSpeakTranslatedSelectionState(speakTranslatedSelectionState);
    buildCopyTranslatedSelectionState(copyTranslatedSelectionState);
    buildRecognizeScreenAreaState(recognizeScreenAreaState);
    buildTranslateScreenAreaState(translateScreenAreaState);
    buildDelayedOcrState(delayedRecognizeScreenAreaState, &MainWindow::buildRecognizeScreenAreaState, &MainWindow::open);
    buildDelayedOcrState(delayedTranslateScreenAreaState, &MainWindow::buildTranslateScreenAreaState);

    // Add transitions between all states
    for (QState *state : m_stateMachine->findChildren<QState *>()) {
        state->addTransition(ui->translateButton, &QToolButton::clicked, translationState);
        state->addTransition(ui->sourceSpeakButtons, &SpeakButtons::playerMediaRequested, speakSourceState);
        state->addTransition(ui->translationSpeakButtons, &SpeakButtons::playerMediaRequested, speakTranslationState);

        state->addTransition(this, &MainWindow::contentChanged, translationState);
        state->addTransition(this, &MainWindow::translateSelectionRequested, translateSelectionState);
        state->addTransition(this, &MainWindow::speakSelectionRequested, speakSelectionState);
        state->addTransition(this, &MainWindow::speakTranslatedSelectionRequested, speakTranslatedSelectionState);
        state->addTransition(this, &MainWindow::copyTranslatedSelectionRequested, copyTranslatedSelectionState);
        state->addTransition(this, &MainWindow::recognizeScreenAreaRequested, recognizeScreenAreaState);
        state->addTransition(this, &MainWindow::translateScreenAreaRequested, translateScreenAreaState);
        state->addTransition(this, &MainWindow::delayedRecognizeScreenAreaRequested, delayedRecognizeScreenAreaState);
        state->addTransition(this, &MainWindow::delayedTranslateScreenAreaRequested, delayedTranslateScreenAreaState);
    }

    translationState->addTransition(translationState, &QState::finished, idleState);
    speakSourceState->addTransition(speakSourceState, &QState::finished, idleState);
    speakTranslationState->addTransition(speakTranslationState, &QState::finished, idleState);
    translateSelectionState->addTransition(translateSelectionState, &QState::finished, idleState);
    speakSelectionState->addTransition(speakSelectionState, &QState::finished, idleState);
    speakTranslatedSelectionState->addTransition(speakTranslatedSelectionState, &QState::finished, idleState);
    recognizeScreenAreaState->addTransition(recognizeScreenAreaState, &QState::finished, idleState);
    translateScreenAreaState->addTransition(translateScreenAreaState, &QState::finished, idleState);
    delayedRecognizeScreenAreaState->addTransition(delayedRecognizeScreenAreaState, &QState::finished, idleState);
    delayedTranslateScreenAreaState->addTransition(delayedTranslateScreenAreaState, &QState::finished, idleState);
}

void MainWindow::buildTranslationState(QState *state) const
{
    auto *abortPreviousState = new QState(state);
    auto *requestState = new QState(state);
    auto *checkLanguagesState = new QState(state);
    auto *requestInOtherLangState = new QState(state);
    auto *parseState = new QFinalState(state);
    state->setInitialState(abortPreviousState);

    connect(abortPreviousState, &QState::entered, m_translator, &QOnlineTranslator::abort);
    connect(abortPreviousState, &QState::entered, ui->translationSpeakButtons, &SpeakButtons::stopSpeaking); // Stop translation speaking
    connect(requestState, &QState::entered, this, &MainWindow::requestTranslation);
    connect(requestInOtherLangState, &QState::entered, this, &MainWindow::requestRetranslation);
    connect(parseState, &QState::entered, this, &MainWindow::displayTranslation);
    setupRequestStateButtons(requestState);
    setupRequestStateButtons(abortPreviousState);

    auto *noTextTransition = new TextEmptyTransition(ui->sourceEdit, abortPreviousState);
    connect(noTextTransition, &TextEmptyTransition::triggered, this, &MainWindow::clearTranslation);
    noTextTransition->setTargetState(m_stateMachine->initialState());

    auto *translationRunningTransition = new TranslatorAbortedTransition(m_translator, abortPreviousState);
    translationRunningTransition->setTargetState(requestState);

    auto *otherLangTransition = new RetranslationTransition(m_translator, ui->translationLanguagesWidget, checkLanguagesState);
    otherLangTransition->setTargetState(requestInOtherLangState);

    requestState->addTransition(m_translator, &QOnlineTranslator::finished, checkLanguagesState);
    checkLanguagesState->addTransition(parseState);
    requestInOtherLangState->addTransition(m_translator, &QOnlineTranslator::finished, parseState);
}

void MainWindow::buildSpeakSourceState(QState *state) const
{
    auto *initialState = new QState(state);
    auto *abortPreviousState = new QState(state);
    auto *requestLangState = new QState(state);
    auto *parseLangState = new QState(state);
    auto *speakTextState = new QFinalState(state);
    state->setInitialState(initialState);

    connect(abortPreviousState, &QState::entered, m_translator, &QOnlineTranslator::abort);
    connect(requestLangState, &QState::entered, this, &MainWindow::requestSourceLanguage);
    connect(parseLangState, &QState::entered, this, &MainWindow::parseSourceLanguage);
    connect(speakTextState, &QState::entered, this, &MainWindow::speakSource);
    setupRequestStateButtons(requestLangState);

    auto *langDetectedTransition = new LanguageDetectedTransition(ui->sourceLanguagesWidget, initialState);
    langDetectedTransition->setTargetState(speakTextState);

    auto *translationRunningTransition = new TranslatorAbortedTransition(m_translator, abortPreviousState);
    translationRunningTransition->setTargetState(requestLangState);

    auto *errorTransition = new TranslatorErrorTransition(m_translator, parseLangState);
    errorTransition->setTargetState(m_stateMachine->initialState());

    initialState->addTransition(abortPreviousState);
    requestLangState->addTransition(m_translator, &QOnlineTranslator::finished, parseLangState);
    parseLangState->addTransition(speakTextState);
}

void MainWindow::buildTranslateSelectionState(QState *state) const
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *showWindowState = new QState(state);
    auto *translationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceTranslationAutodetect);
    connect(showWindowState, &QState::entered, this, &MainWindow::showTranslationWindow);
    buildSetSelectionAsSourceState(setSelectionAsSourceState);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(setSelectionAsSourceState, &QState::finished, showWindowState);
    showWindowState->addTransition(translationState);
    translationState->addTransition(translationState, &QState::finished, finalState);
}

void MainWindow::buildSpeakTranslationState(QState *state) const
{
    auto *speakTextState = new QFinalState(state);
    state->setInitialState(speakTextState);

    connect(speakTextState, &QState::entered, this, &MainWindow::speakTranslation);
}

void MainWindow::buildSpeakSelectionState(QState *state) const
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *speakSourceState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    buildSetSelectionAsSourceState(setSelectionAsSourceState);
    buildSpeakSourceState(speakSourceState);

    setSelectionAsSourceState->addTransition(setSelectionAsSourceState, &QState::finished, speakSourceState);
    speakSourceState->addTransition(speakSourceState, &QState::finished, finalState);
}

void MainWindow::buildSpeakTranslatedSelectionState(QState *state) const
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *translationState = new QState(state);
    auto *speakTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceTranslationAutodetect);
    buildSetSelectionAsSourceState(setSelectionAsSourceState);
    buildTranslationState(translationState);
    buildSpeakTranslationState(speakTranslationState);

    setSelectionAsSourceState->addTransition(setSelectionAsSourceState, &QState::finished, translationState);
    translationState->addTransition(translationState, &QState::finished, speakTranslationState);
    speakTranslationState->addTransition(speakTranslationState, &QState::finished, finalState);
}

void MainWindow::buildCopyTranslatedSelectionState(QState *state) const
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *translationState = new QState(state);
    auto *copyTranslationState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceTranslationAutodetect);
    connect(copyTranslationState, &QState::entered, this, &MainWindow::copyTranslationToClipboard);
    buildSetSelectionAsSourceState(setSelectionAsSourceState);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(setSelectionAsSourceState, &QState::finished, translationState);
    translationState->addTransition(translationState, &QState::finished, copyTranslationState);
}

void MainWindow::buildRecognizeScreenAreaState(QState *state, void (MainWindow::*showFunction)())
{
    auto *initialState = new QState(state);
    auto *grabState = new QState(state);
    auto *snippingState = new QState(state);
    auto *recognizeState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(initialState);

    connect(grabState, &QState::entered, m_screenGrabber, &AbstractScreenGrabber::grab);
    connect(grabState, &QState::exited, m_screenGrabber, &AbstractScreenGrabber::cancel);
    connect(recognizeState, &QState::entered, ui->sourceEdit, &SourceTextEdit::removeText);
    connect(recognizeState, &QState::entered, this, &MainWindow::forceSourceAutodetect);
    connect(recognizeState, &QState::entered, this, showFunction);
    connect(recognizeState, &QState::exited, m_ocr, &Ocr::cancel);
    setupRequestStateButtons(recognizeState);

    auto *ocrUninitializedTransition = new OcrUninitializedTransition(this, initialState);
    ocrUninitializedTransition->setTargetState(m_stateMachine->initialState());

    auto *snippingVisibleTranstion = new SnippingVisibleTransition(m_snippingArea, initialState);
    snippingVisibleTranstion->setTargetState(snippingState);

    initialState->addTransition(grabState);
    grabState->addTransition(m_screenGrabber, &AbstractScreenGrabber::grabbingFailed, m_stateMachine->initialState());
    grabState->addTransition(m_screenGrabber, &AbstractScreenGrabber::grabbed, snippingState);
    snippingState->addTransition(m_snippingArea, &SnippingArea::cancelled, m_stateMachine->initialState());
    snippingState->addTransition(m_snippingArea, &SnippingArea::snipped, recognizeState);
    recognizeState->addTransition(m_ocr, &Ocr::canceled, m_stateMachine->initialState());
    recognizeState->addTransition(m_ocr, &Ocr::recognized, finalState);
}

void MainWindow::buildTranslateScreenAreaState(QState *state)
{
    auto *recognizeState = new QState(state);
    auto *translationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(recognizeState);

    connect(recognizeState, &QState::entered, this, &MainWindow::forceTranslationAutodetect);
    buildRecognizeScreenAreaState(recognizeState, &MainWindow::showTranslationWindow);
    buildTranslationState(translationState);

    recognizeState->addTransition(recognizeState, &QState::finished, translationState);
    translationState->addTransition(translationState, &QState::finished, finalState);
}

template<typename Func, typename... Args>
void MainWindow::buildDelayedOcrState(QState *state, Func buildState, Args... additionalArgs)
{
    auto *waitState = new QState(state);
    auto *ocrState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(waitState);

    connect(waitState, &QState::entered, this, &MainWindow::minimize);
    connect(waitState, &QState::entered, m_screenCaptureTimer, qOverload<>(&QTimer::start));
    (this->*buildState)(ocrState, additionalArgs...);

    waitState->addTransition(m_screenCaptureTimer, &QTimer::timeout, ocrState);
    ocrState->addTransition(ocrState, &QState::finished, finalState);
}

void MainWindow::buildSetSelectionAsSourceState(QState *state) const
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, &Selection::instance(), &Selection::requestSelection);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceSourceAutodetect);

    setSelectionAsSourceState->addTransition(&Selection::instance(), &Selection::requestedSelectionAvailable, finalState);
}

void MainWindow::setupRequestStateButtons(QState *state) const
{
    state->assignProperty(ui->translateButton, "enabled", false);
    state->assignProperty(ui->clearButton, "enabled", false);
    state->assignProperty(ui->abortButton, "enabled", true);
}

// These settings are loaded only at startup and cannot be configured in the settings dialog
void MainWindow::loadMainWindowSettings()
{
    const AppSettings settings;
    ui->autoTranslateCheckBox->setChecked(settings.isAutoTranslateEnabled());
    ui->engineComboBox->setCurrentIndex(settings.currentEngine());
    ui->sourceLanguagesWidget->setLanguages(settings.languages(AppSettings::Source));
    ui->translationLanguagesWidget->setLanguages(settings.languages(AppSettings::Translation));
    ui->translationLanguagesWidget->checkButton(settings.checkedButton(AppSettings::Translation));
    ui->sourceLanguagesWidget->checkButton(settings.checkedButton(AppSettings::Source));

    restoreGeometry(settings.mainWindowGeometry());
    if (!settings.isShowTrayIcon() || !settings.isStartMinimized()) {
        show();
        ui->sourceEdit->setFocus();
    }

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
        release->get(QStringLiteral("crow-translate"), QStringLiteral("crow-translate"));
    }
#endif
}

void MainWindow::loadAppSettings()
{
    AppSettings settings;

    // General
    m_windowMode = settings.windowMode();
    setOrientation(settings.mainWindowOrientation());
    m_trayIcon->setTranslationNotificationTimeout(settings.translationNotificationTimeout());

    // Interface
    ui->translationEdit->setFont(settings.font());
    ui->sourceEdit->setFont(settings.font());

    ui->sourceLanguagesWidget->setLanguageFormat(settings.mainWindowLanguageFormat());
    ui->translationLanguagesWidget->setLanguageFormat(settings.mainWindowLanguageFormat());

    if (const AppSettings::IconType iconType = settings.trayIconType(); iconType == AppSettings::CustomIcon) {
        const QString customIconPath = settings.customIconPath();
        m_trayIcon->setIcon(TrayIcon::customTrayIcon(customIconPath));
        if (m_trayIcon->icon().isNull()) {
            m_trayIcon->showMessage(TrayIcon::tr("Invalid tray icon"), TrayIcon::tr("The specified icon '%1' is invalid. The default icon will be used.").arg(customIconPath));
            m_trayIcon->setIcon(QIcon::fromTheme(TrayIcon::trayIconName(AppSettings::DefaultIcon)));
            settings.setTrayIconType(AppSettings::DefaultIcon);
        }
    } else {
        m_trayIcon->setIcon(QIcon::fromTheme(TrayIcon::trayIconName(iconType)));
    }
    m_trayIcon->setVisible(settings.isShowTrayIcon());
    QGuiApplication::setQuitOnLastWindowClosed(!m_trayIcon->isVisible());

    // Translation
    m_translator->setSourceTranslitEnabled(settings.isSourceTranslitEnabled());
    m_translator->setTranslationTranslitEnabled(settings.isTranslationTranslitEnabled());
    m_translator->setSourceTranscriptionEnabled(settings.isSourceTranscriptionEnabled());
    m_translator->setTranslationOptionsEnabled(settings.isTranslationOptionsEnabled());
    m_translator->setExamplesEnabled(settings.isExamplesEnabled());
    ui->sourceEdit->setSimplifySource(settings.isSimplifySource());
    m_primaryLanguage = settings.primaryLanguage();
    m_secondaryLanguage = settings.secondaryLanguage();
    m_forceSourceAutodetect = settings.isForceSourceAutodetect();
    m_forceTranslationAutodetect = settings.isForceTranslationAutodetect();

    // Engine settings
    m_translator->setEngineUrl(QOnlineTranslator::LibreTranslate, settings.engineUrl(QOnlineTranslator::LibreTranslate));
    m_translator->setEngineApiKey(QOnlineTranslator::LibreTranslate, settings.engineApiKey(QOnlineTranslator::LibreTranslate));
    m_translator->setEngineUrl(QOnlineTranslator::Lingva, settings.engineUrl(QOnlineTranslator::Lingva));

    // OCR settings
    if (const QByteArray languages = settings.ocrLanguagesString(), path = settings.ocrLanguagesPath(); !m_ocr->init(languages, path, settings.tesseractParameters())) {
        // Show error only if languages was specified by user
        if (languages != AppSettings::defaultOcrLanguagesString() || path != AppSettings::defaultOcrLanguagesPath())
            m_trayIcon->showMessage(Ocr::tr("Unable to set OCR languages"), Ocr::tr("Unable to initialize Tesseract with %1").arg(QString(languages)));
    }
    if (const AppSettings::RegionRememberType type = settings.regionRememberType(); m_snippingArea->regionRememberType() != type) {
        m_snippingArea->setRegionRememberType(type);
        // Apply last remembered selection only if remember type was changed
        if (type == AppSettings::RememberAlways)
            m_snippingArea->setCropRegion(settings.cropRegion());
    }
    m_ocr->setConvertLineBreaks(settings.isConvertLineBreaks());
    m_screenCaptureTimer->setInterval(settings.captureDelay());
    m_snippingArea->setCaptureOnRelese(settings.isConfirmOnRelease());
    m_snippingArea->setShowMagnifier(settings.isShowMagnifier());
    m_snippingArea->setApplyLightMask(settings.isApplyLightMask());

    // TTS
    ui->sourceSpeakButtons->setVoice(QOnlineTranslator::Yandex, settings.voice(QOnlineTranslator::Yandex));
    ui->sourceSpeakButtons->setEmotion(QOnlineTranslator::Yandex, settings.emotion(QOnlineTranslator::Yandex));
    ui->sourceSpeakButtons->setRegions(QOnlineTranslator::Google, settings.regions(QOnlineTranslator::Google));
    ui->translationSpeakButtons->setVoice(QOnlineTranslator::Yandex, settings.voice(QOnlineTranslator::Yandex));
    ui->translationSpeakButtons->setEmotion(QOnlineTranslator::Yandex, settings.emotion(QOnlineTranslator::Yandex));
    ui->translationSpeakButtons->setRegions(QOnlineTranslator::Google, settings.regions(QOnlineTranslator::Google));

    // Connection
    if (const QNetworkProxy::ProxyType proxyType = settings.proxyType(); proxyType == QNetworkProxy::DefaultProxy) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
    } else {
        QNetworkProxy proxy(proxyType);
        if (proxyType == QNetworkProxy::HttpProxy || proxyType == QNetworkProxy::Socks5Proxy) {
            proxy.setHostName(settings.proxyHost());
            proxy.setPort(settings.proxyPort());
            if (settings.isProxyAuthEnabled()) {
                proxy.setUser(settings.proxyUsername());
                proxy.setPassword(settings.proxyPassword());
            }
        }
        QNetworkProxy::setApplicationProxy(proxy);
    }

    // Global shortcuts
    if (QHotkey::isPlatformSupported() && settings.isGlobalShortuctsEnabled()) {
        m_translateSelectionHotkey->setShortcut(settings.translateSelectionShortcut(), true);
        m_speakSelectionHotkey->setShortcut(settings.speakSelectionShortcut(), true);
        m_stopSpeakingHotkey->setShortcut(settings.stopSpeakingShortcut(), true);
        m_playPauseSpeakingHotkey->setShortcut(settings.playPauseSpeakingShortcut(), true);
        m_speakTranslatedSelectionHotkey->setShortcut(settings.speakTranslatedSelectionShortcut(), true);
        m_showMainWindowHotkey->setShortcut(settings.showMainWindowShortcut(), true);
        m_copyTranslatedSelectionHotkey->setShortcut(settings.copyTranslatedSelectionShortcut(), true);
        m_recognizeScreenAreaHotkey->setShortcut(settings.recognizeScreenAreaShortcut(), true);
        m_translateScreenAreaHotkey->setShortcut(settings.translateScreenAreaShortcut(), true);
        m_delayedRecognizeScreenAreaHotkey->setShortcut(settings.delayedRecognizeScreenAreaShortcut(), true);
        m_delayedTranslateScreenAreaHotkey->setShortcut(settings.delayedTranslateScreenAreaShortcut(), true);
    } else {
        m_translateSelectionHotkey->setRegistered(false);
        m_speakSelectionHotkey->setRegistered(false);
        m_stopSpeakingHotkey->setRegistered(false);
        m_playPauseSpeakingHotkey->setRegistered(false);
        m_speakTranslatedSelectionHotkey->setRegistered(false);
        m_showMainWindowHotkey->setRegistered(false);
        m_copyTranslatedSelectionHotkey->setRegistered(false);
        m_recognizeScreenAreaHotkey->setRegistered(false);
        m_translateScreenAreaHotkey->setRegistered(false);
        m_delayedRecognizeScreenAreaHotkey->setRegistered(false);
        m_delayedTranslateScreenAreaHotkey->setRegistered(false);
    }

    // Window shortcuts
    ui->sourceSpeakButtons->setSpeakShortcut(settings.speakSourceShortcut());
    ui->translationSpeakButtons->setSpeakShortcut(settings.speakTranslationShortcut());
    ui->translateButton->setShortcut(settings.translateShortcut());
    ui->swapButton->setShortcut(settings.swapShortcut());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationShortcut());
    m_closeWindowsShortcut->setKey(settings.closeWindowShortcut());
}

// Toggle language logic
void MainWindow::checkLanguageButton(int checkedId)
{
    LanguageButtonsWidget *checkedGroup;
    LanguageButtonsWidget *anotherGroup;
    if (sender() == ui->sourceLanguagesWidget) {
        checkedGroup = ui->sourceLanguagesWidget;
        anotherGroup = ui->translationLanguagesWidget;
    } else {
        checkedGroup = ui->translationLanguagesWidget;
        anotherGroup = ui->sourceLanguagesWidget;
    }

    /* If the target and source languages are the same and they are not autodetect buttons,
     * then select previous checked language from just checked language group to another group */
    const QOnlineTranslator::Language checkedLang = checkedGroup->language(checkedId);
    if (checkedLang == anotherGroup->checkedLanguage() && !checkedGroup->isAutoButtonChecked() && !anotherGroup->isAutoButtonChecked()) {
        if (!anotherGroup->checkLanguage(checkedGroup->previousCheckedLanguage()))
            anotherGroup->checkAutoButton(); // Select "Auto" button if group do not have such language
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

    markContentAsChanged();
}

// Selected primary or secondary language depends on sourceLang
QOnlineTranslator::Language MainWindow::preferredTranslationLanguage(QOnlineTranslator::Language sourceLang) const
{
    QOnlineTranslator::Language translationLang = m_primaryLanguage;
    if (translationLang == QOnlineTranslator::Auto)
        translationLang = QOnlineTranslator::language(QLocale());

    if (translationLang != sourceLang)
        return translationLang;

    return m_secondaryLanguage;
}

QOnlineTranslator::Engine MainWindow::currentEngine() const
{
    return static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
}
