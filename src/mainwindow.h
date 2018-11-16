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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QShortcut>
#include <QTimer>
#include <QMediaPlayer>
#include <QMenu>

#include "qhotkey.h"
#include "qonlinetranslator.h"
#include "langbuttongroup.h"

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
    void on_engineComboBox_currentIndexChanged(int index);

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
    void translateTimerExpires();

    // Other
    void showMainWindow();
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
    void loadSettings();
    void play(QMediaPlayer *player, QMediaPlaylist *playlist, const QString &text, QOnlineTranslator::Language lang = QOnlineTranslator::Auto);
    QString selectedText();

    Ui::MainWindow *ui;

    QOnlineTranslator translator{this};
    QOnlineTranslator::Language uiLang;

    QMediaPlayer sourcePlayer{this};
    QMediaPlayer translationPlayer{this};
    QMediaPlayer selectionPlayer{this};
    QMediaPlaylist sourcePlaylist{this};
    QMediaPlaylist translationPlaylist{this};
    QMediaPlaylist selectionPlaylist{this};

    QShortcut closeWindowsShortcut{this};
    QHotkey translateSelectionHotkey{this};
    QHotkey playSelectionHotkey{this};
    QHotkey playTranslatedSelectionHotkey{this};
    QHotkey stopSelectionHotkey{this};
    QHotkey showMainWindowHotkey{this};
    QHotkey copyTranslatedSelectionHotkey{this};

    LangButtonGroup sourceButtons{this};
    LangButtonGroup translationButtons{this};

    QMenu trayMenu{this};
    QSystemTrayIcon trayIcon{this};
    QTimer translateTimer{this};
};

#endif // MAINWINDOW_H
