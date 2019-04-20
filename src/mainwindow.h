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
#include <QSystemTrayIcon>

class LangButtonGroup;
class AppSettings;
class QHotkey;
class QShortcut;
class QTimer;
class QMenu;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void translationTextChanged(const QString &text);
    void playSourceButtonIconChanged(QIcon icon);
    void stopSourceButtonEnabled(bool enabled);
    void playTranslationButtonIconChanged(QIcon icon);
    void stopTranslationButtonEnabled(bool enabled);

private slots:
    // UI
    void on_translateButton_clicked();
    void on_swapButton_clicked();
    void on_settingsButton_clicked();
    void on_autoTranslateCheckBox_toggled(bool checked);
    void on_engineComboBox_currentIndexChanged(int);

    void on_playSourceButton_clicked();
    void on_playTranslationButton_clicked();
    void on_stopSourceButton_clicked();
    void on_stopTranslationButton_clicked();

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

    // Player icons
    void changeSourcePlayerState(QMediaPlayer::State state);
    void changeTranslationPlayerState(QMediaPlayer::State state);
    void changeSelectionPlayerState(QMediaPlayer::State state);

    // Autotranslate timer
    void startTranslateTimer();

    // Other
    void showMainWindow();
    void showAppRunningMessage();
    void activateTray(QSystemTrayIcon::ActivationReason reason);
#if defined(Q_OS_WIN)
    void checkForUpdates();
#endif

private:
    void changeEvent(QEvent *event) override;

    // Translation
    bool translate(QOnlineTranslator::Language translationLang, QOnlineTranslator::Language sourceLang);
    bool translateOutside(const QString &text, QOnlineTranslator::Language translationLang);

    // Helper functions
    void loadSettings(const AppSettings &settings);
    void play(QMediaPlayer *player, QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang = QOnlineTranslator::Auto);
    QString selectedText();

    Ui::MainWindow *ui;

    QOnlineTranslator *m_translator;
    QOnlineTranslator::Language m_uiLang;

    QMediaPlayer *m_sourcePlayer;
    QMediaPlayer *m_translationPlayer;
    QMediaPlayer *m_selectionPlayer;
    QMediaPlaylist *m_sourcePlaylist;
    QMediaPlaylist *m_translationPlaylist;
    QMediaPlaylist *m_selectionPlaylist;

    QShortcut *m_closeWindowsShortcut;
    QHotkey *m_translateSelectionHotkey;
    QHotkey *m_playSelectionHotkey;
    QHotkey *m_playTranslatedSelectionHotkey;
    QHotkey *m_stopSelectionHotkey;
    QHotkey *m_showMainWindowHotkey;
    QHotkey *m_copyTranslatedSelectionHotkey;

    LangButtonGroup *m_sourceButtons;
    LangButtonGroup *m_translationButtons;

    QMenu *m_trayMenu;
    QSystemTrayIcon *m_trayIcon;
    QTimer *m_translateTimer;
};

#endif // MAINWINDOW_H
