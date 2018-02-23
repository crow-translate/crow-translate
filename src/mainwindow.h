/*
 *  Copyright © 2018 Gennady Chernyshchuk <genaloner@gmail.com>
 *
 *  This file is part of Crow.
 *
 *  Crow is free software; you can redistribute it and/or modify
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
#include <QTranslator>
#include <QShortcut>

#include "qhotkey.h"
#include "buttongrouplanguages.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void setTranslation();

    ~MainWindow();

private slots:

    void on_translateButton_clicked();

    void on_inputLanguagesButton_triggered(QAction *language);

    void on_outputLanguagesButton_triggered(QAction *language);

    void on_swapButton_clicked();

    void on_settingsButton_clicked();

    void on_inputSpeakButton_clicked();

    void on_outputSpeakButton_clicked();

    void on_inputCopyButton_clicked();

    void on_outputCopyButton_clicked();

    void on_tray_activated(QSystemTrayIcon::ActivationReason reason);

    void on_translateSelectedHotkey_activated();

    void on_speakHotkey_activated();

    void on_showMainWindowHotkey_activated();

    void reloadTranslation();

private:
    static QString test;
    void loadSettings();

    QString getSelectedText();

    Ui::MainWindow *ui;

    QTranslator translator;

    QMenu *languagesMenu;
    QMenu *trayMenu;
    QAction *trayShowWindow;
    QAction *traySettings;
    QAction *trayExit;
    QSystemTrayIcon *trayIcon;

    QHotkey *translateSelectedHotkey;
    QHotkey *speakHotkey;
    QHotkey *showMainWindowHotkey;

    ButtonGroupLanguages *inputLanguages;
    ButtonGroupLanguages *outputLanguages;
};

#endif // MAINWINDOW_H
