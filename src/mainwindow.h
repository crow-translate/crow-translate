/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settings/appsettings.h"

#include <QMainWindow>
#include <QMediaPlayer>

class LanguageButtonsWidget;
class Ocr;
class ScreenGrabber;
class SpeakButtons;
class QHotkey;
class QTaskbarControl;
class QShortcut;
class QComboBox;
class TranslationEdit;
class QToolButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.crow_translate.CrowTranslate.MainWindow")
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(const AppSettings &settings, QWidget *parent = nullptr);
    ~MainWindow() override;

    const QComboBox *engineCombobox() const;
    const QToolButton *swapButton() const;
    const QToolButton *copySourceButton() const;
    const QToolButton *copyTranslationButton() const;
    const QToolButton *copyAllTranslationButton() const;
    const TranslationEdit *translationEdit() const;
    const LanguageButtonsWidget *sourceLanguageButtons() const;
    const LanguageButtonsWidget *translationLanguageButtons() const;
    const SpeakButtons *sourceSpeakButtons() const;
    const SpeakButtons *translationSpeakButtons() const;
    const QShortcut *closeWindowShortcut() const;
    const Ocr *ocr() const;

    AppSettings::LanguageFormat popupLanguageFormat() const;
    QSize popupSize() const;
    double popupOpacity() const;

public slots:
    // Global shortcuts
    Q_SCRIPTABLE void translateSelection();
    Q_SCRIPTABLE void speakSelection();
    Q_SCRIPTABLE void speakTranslatedSelection();
    Q_SCRIPTABLE void stopSpeaking();
    Q_SCRIPTABLE void open();
    Q_SCRIPTABLE void copyTranslatedSelection();
    Q_SCRIPTABLE void translateScreenArea();

    // Main window shortcuts
    Q_SCRIPTABLE void clearText();
    Q_SCRIPTABLE void abortTranslation();
    Q_SCRIPTABLE void swapLanguages();
    Q_SCRIPTABLE void openSettings();
    Q_SCRIPTABLE void setAutoTranslateEnabled(bool enabled);
    Q_SCRIPTABLE void copySourceText();
    Q_SCRIPTABLE void copyTranslation();
    Q_SCRIPTABLE void copyAllTranslationInfo();
    Q_SCRIPTABLE static void quit();

signals:
    void translateSelectionRequested();
    void speakSelectionRequested();
    void speakTranslatedSelectionRequested();
    void copyTranslatedSelectionRequested();
    void translateScreenAreaRequested();

private slots:
    // State machine's slots
    void requestTranslation();
    void requestRetranslation();
    void displayTranslation();
    void clearTranslation();

    void requestSourceLanguage();
    void parseSourceLanguage();

    void speakSource();
    void speakTranslation();

    void showTranslationWindow();
    void copyTranslationToClipboard();

    void forceSourceAutodetect();
    void forceAutodetect();

    // UI
    void setTranslationOnEditEnabled(bool enabled);
    void resetAutoSourceButtonText();
    void setTaskbarState(QMediaPlayer::State state);

    // Other
    void showAppRunningMessage();
    void setSourceText(const QString &text);
#ifdef Q_OS_WIN
    void checkForUpdates();
#endif

private:
    void changeEvent(QEvent *event) override;

    void buildStateMachine();
    void buildTranslationState(QState *state) const;
    void buildSpeakSourceState(QState *state) const;
    void buildTranslateSelectionState(QState *state) const;
    void buildSpeakTranslationState(QState *state) const;
    void buildSpeakSelectionState(QState *state) const;
    void buildSpeakTranslatedSelectionState(QState *state) const;
    void buildCopyTranslatedSelectionState(QState *state) const;
    void buildTranslateScreenAreaState(QState *state);

    void setupRequestStateButtons(QState *state) const;

    // Helper functions
    void loadMainWindowSettings(const AppSettings &settings);
    void loadAppSettings(const AppSettings &settings);
    void checkLanguageButton(int checkedId);

    QOnlineTranslator::Engine currentEngine();

    Ui::MainWindow *ui;

    QHotkey *m_translateSelectionHotkey;
    QHotkey *m_speakSelectionHotkey;
    QHotkey *m_speakTranslatedSelectionHotkey;
    QHotkey *m_stopSpeakingHotkey;
    QHotkey *m_showMainWindowHotkey;
    QHotkey *m_copyTranslatedSelectionHotkey;
    QHotkey *m_translateScreenAreaHotkey;
    QShortcut *m_closeWindowsShortcut;

    QStateMachine *m_stateMachine;
    QOnlineTranslator *m_translator;
    TrayIcon *m_trayIcon;
    QTaskbarControl *m_taskbar;
    Ocr *m_ocr;
    ScreenGrabber *m_screenGrabber;

    AppSettings::LanguageFormat m_popupLanguageFormat;
    QSize m_popupSize;
    double m_popupOpacity;
};

#endif // MAINWINDOW_H
