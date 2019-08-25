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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qonlinetranslator.h"

#include <QMainWindow>
#include <QMediaPlayer>

class TrayIcon;
class LangButtonGroup;
class PlayerButtons;
class AppSettings;
class QHotkey;
class QTaskbarControl;
class QShortcut;
class QMenu;
class QComboBox;
class TranslationEdit;
class QToolButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(const AppSettings &settings, QWidget *parent = nullptr);
    ~MainWindow() override;

    void activate();

    QComboBox *engineCombobox();
    QToolButton *addSourceLangButton();
    QToolButton *addTranslationLangButton();
    QToolButton *swapButton();
    QToolButton *copySourceButton();
    QToolButton *copyTranslationButton();
    QToolButton *copyAllTranslationButton();
    TranslationEdit *translationEdit();
    LangButtonGroup *sourceLangButtons();
    LangButtonGroup *translationLangButtons();
    PlayerButtons *sourcePlayerButtons();
    PlayerButtons *translationPlayerButtons();

private slots:
    // State machine's slots
    void requestTranslation();
    void requestRetranslation();
    void parseTranslation();
    void clearTranslation();

    void requestSourceLanguage();
    void parseSourceLanguage();

    void speakSource();
    void speakTranslation();

    void showTranslationWindow();
    void setSelectionAsSource();
    void copyTranslationToClipboard();

    void forceSourceAutodetect();
    void forceAutodetect();

    // UI
    void abortTranslation();
    void swapLanguages();
    void openSettings();
    void setAutoTranslateEnabled(bool enabled);

    void copySourceText();
    void copyTranslation();
    void copyAllTranslationInfo();

    void addSourceLanguage();
    void addTranslationLanguage();

    void resetAutoSourceButtonText();
    void setTaskbarState(QMediaPlayer::State state);

    // Other
    void showAppRunningMessage();
#ifdef Q_OS_WIN
    void checkForUpdates();
#endif

private:
    void changeEvent(QEvent *event) override;

    void buildStateMachine();
    void buildTranslationState(QState *state);
    void buildSpeakSourceState(QState *state);
    void buildTranslateSelectionState(QState *state);
    void buildSpeakTranslationState(QState *state);
    void buildSpeakSelectionState(QState *state);
    void buildSpeakTranslatedSelectionState(QState *state);
    void buildCopyTranslatedSelectionState(QState *state);

    void setupRequestStateButtons(QState *state);

    // Helper functions
    void loadSettings(const AppSettings &settings);
    void checkLanguageButton(LangButtonGroup *checkedGroup, int checkedId, LangButtonGroup *anotherGroup);
    QString selectedText();

    QOnlineTranslator::Engine currentEngine();

    Ui::MainWindow *ui;

    QShortcut *m_closeWindowsShortcut;
    QHotkey *m_translateSelectionHotkey;
    QHotkey *m_playSelectionHotkey;
    QHotkey *m_playTranslatedSelectionHotkey;
    QHotkey *m_stopSpeakingHotkey;
    QHotkey *m_showMainWindowHotkey;
    QHotkey *m_copyTranslatedSelectionHotkey;

    LangButtonGroup *m_sourceLangButtons;
    LangButtonGroup *m_translationLangButtons;

    QStateMachine *m_stateMachine;
    QOnlineTranslator *m_translator;
    QMenu *m_trayMenu;
    TrayIcon *m_trayIcon;
    QTaskbarControl *m_taskbar;
};

#endif // MAINWINDOW_H
