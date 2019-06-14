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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "qonlinetts.h"

#include <QDialog>

class QMediaPlayer;
class QMediaPlaylist;
class ShortcutItem;
#ifdef Q_OS_WIN
class QComboBox;
class QPushButton;
class QLabel;
#endif

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    // UI
    void on_SettingsDialog_accepted();
    void on_proxyTypeComboBox_currentIndexChanged(int index);

    void on_trayIconComboBox_currentIndexChanged(int index);
    void on_customTrayIconButton_clicked();
    void on_customTrayIconEdit_textChanged(const QString &iconPath);

    void on_engineComboBox_currentIndexChanged(int index);
    void on_voiceComboBox_currentIndexChanged(int index);
    void on_emotionComboBox_currentIndexChanged(int index);
    void on_testSpeechButton_clicked();

    void on_shortcutsTreeView_currentItemChanged(ShortcutItem *item);
    void on_shortcutSequenceEdit_editingFinished();
    void on_acceptShortcutButton_clicked();
    void on_clearShortcutButton_clicked();
    void on_resetShortcutButton_clicked();
    void on_resetAllShortcutsButton_clicked();

#ifdef Q_OS_WIN
    void checkForUpdates();
#endif
    void restoreDefaults();

private:
    void loadSettings();

    Ui::SettingsDialog *ui;

    // Test voice
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;

    // Engine voice settings
    QOnlineTts::Voice yandexVoice;
    QOnlineTts::Voice bingVoice;
    QOnlineTts::Emotion yandexEmotion;

#ifdef Q_OS_WIN
    // Check for updates box stuff
    QComboBox *m_checkForUpdatesComboBox;
    QPushButton *m_checkForUpdatesButton;
    QLabel *m_checkForUpdatesStatusLabel;
#endif
};

#endif // SETTINGSDIALOG_H
