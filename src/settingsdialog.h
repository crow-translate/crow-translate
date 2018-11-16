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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMenu>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#if defined(Q_OS_WIN)
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
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
    void on_dialogButtonBox_accepted();
    void on_resetSettingsButton_clicked();
    void on_proxyTypeComboBox_currentIndexChanged(int index);

    void on_trayIconComboBox_currentIndexChanged(int index);
    void on_customTrayIconButton_clicked();

    void on_engineComboBox_currentIndexChanged(int index);
    void on_testSpeechButton_clicked();

    void on_shortcutsTreeWidget_itemSelectionChanged();
    void on_shortcutSequenceEdit_editingFinished();
    void on_acceptShortcutButton_clicked();
    void on_clearShortcutButton_clicked();
    void on_resetShortcutButton_clicked();
    void on_resetAllShortcutsButton_clicked();

#if defined(Q_OS_WIN)
    void checkForUpdates();
#endif
private:
    Ui::SettingsDialog *ui;
    QMediaPlayer m_player{this};
    QMediaPlaylist m_playlist{this};

#if defined(Q_OS_WIN)
    QLabel papirusTitleLabel;
    QLabel papirusLabel;
    QLabel checkForUpdatesLabel;

    QComboBox checkForUpdatesComboBox;
    QPushButton checkForUpdatesButton;
    QLabel checkForUpdatesStatusLabel;
#endif
};

#endif // SETTINGSDIALOG_H
