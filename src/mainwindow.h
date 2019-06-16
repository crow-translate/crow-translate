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
class QShortcut;
class QTimer;
class QMenu;
class QComboBox;
class QTextEdit;
class QToolButton;
#ifdef Q_OS_WIN
class QWinTaskbarButton;
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void activate();

    QComboBox *engineCombobox();
    QTextEdit *translationEdit();
    QToolButton *addSourceLangButton();
    QToolButton *addTranslationLangButton();
    QToolButton *swapButton();
    QToolButton *copySourceButton();
    QToolButton *copyTranslationButton();
    QToolButton *copyAllTranslationButton();
    LangButtonGroup *sourceLangButtons();
    LangButtonGroup *translationLangButtons();
    PlayerButtons *sourcePlayerButtons();
    PlayerButtons *translationPlayerButtons();

signals:
    void translationTextChanged(const QString &text);

private slots:
    // UI
    void on_translateButton_clicked();
    void on_swapButton_clicked();
    void on_settingsButton_clicked();
    void on_autoTranslateCheckBox_toggled(bool checked);
    void on_engineComboBox_currentIndexChanged(int);

    void on_copySourceButton_clicked();
    void on_copyTranslationButton_clicked();
    void on_copyAllTranslationButton_clicked();

    void on_addSourceLangButton_clicked();
    void on_addTranslationLangButton_clicked();

    // Shortcuts
    void translateSelectedText();
    void copyTranslatedSelection();
    void playSelection();
    void playTranslatedSelection();

    // Language buttons
    void checkLanguageButton(LangButtonGroup *checkedGroup, LangButtonGroup *anotherGroup, int id);
    void resetAutoSourceButtonText();

    // Autotranslate timer
    void startTranslateTimer();

    // Other
    void showAppRunningMessage();
#ifdef Q_OS_WIN
    void checkForUpdates();
#endif

private:
#ifdef Q_OS_WIN
    void showEvent(QShowEvent *event) override;
#endif
    void changeEvent(QEvent *event) override;

    // Translation
    bool translate(QOnlineTranslator::Language translationLang, QOnlineTranslator::Language sourceLang);
    bool translateOutside(const QString &text, QOnlineTranslator::Language translationLang);

    // Helper functions
    void loadSettings(const AppSettings &settings);
    void setPlayingText(QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang = QOnlineTranslator::Auto);
    QString selectedText();

    Ui::MainWindow *ui;

    QOnlineTranslator *m_translator;
    QOnlineTranslator::Language m_uiLang;

    PlayerButtons *m_sourcePlayerButtons;
    PlayerButtons *m_translationPlayerButtons;

    QShortcut *m_closeWindowsShortcut;
    QHotkey *m_translateSelectionHotkey;
    QHotkey *m_playSelectionHotkey;
    QHotkey *m_playTranslatedSelectionHotkey;
    QHotkey *m_stopSpeakingHotkey;
    QHotkey *m_showMainWindowHotkey;
    QHotkey *m_copyTranslatedSelectionHotkey;

    LangButtonGroup *m_sourceLangButtons;
    LangButtonGroup *m_translationLangButtons;

    QMenu *m_trayMenu;
    TrayIcon *m_trayIcon;
    QTimer *m_translateTimer;
#ifdef Q_OS_WIN
    QWinTaskbarButton *m_taskbarButton;
#endif
};

#endif // MAINWINDOW_H
