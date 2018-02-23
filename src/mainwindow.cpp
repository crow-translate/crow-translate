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

#include "mainwindow.h"

#include <QClipboard>

#include "qonlinetranslator.h"
#include "ui_mainwindow.h"
#include "popupwindow.h"
#include "settingsdialog.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui (new Ui::MainWindow),
    languagesMenu (new QMenu),
    trayMenu (new QMenu),
    trayIcon (new QSystemTrayIcon(this)),
    translateSelectedHotkey (new QHotkey(this)),
    speakHotkey (new QHotkey(this)),
    showMainWindowHotkey (new QHotkey(this)),
    inputLanguages (new ButtonGroupLanguages(this)),
    outputLanguages (new ButtonGroupLanguages(this))
{
    trayIcon->setObjectName("tray");
    translateSelectedHotkey->setObjectName("translateSelectedHotkey");
    speakHotkey->setObjectName("speakHotkey");
    showMainWindowHotkey->setObjectName("showMainWindowHotkey");
    inputLanguages->setObjectName("inputLanguages");
    outputLanguages->setObjectName("outputLanguages");

    // Load translation
    QSettings settings;
    if (translator.load(QLocale(qvariant_cast<QLocale::Language>(settings.value("Language", 0))), QString("crow"), QString("_"), QString(":/translations")))
        qApp->installTranslator(&translator);

    ui->setupUi(this);

    // Add all languages to languageMenu
    for (auto i=0; i<QOnlineTranslator::LANGUAGE_NAMES.size(); i++) {
        QString language = QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(i)));
        QAction *action = new QAction(language);
        action->setData(i); // Set data as index
        languagesMenu->addAction(action);
    }

    // Add languageMenu to auto-language buttons
    ui->inputLanguagesButton->setMenu(languagesMenu);
    ui->outputLanguagesButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    inputLanguages->addButton(ui->inputLanguagesButton, 0);
    inputLanguages->addButton(ui->inputLanguageButton1);
    inputLanguages->addButton(ui->inputLanguageButton2);
    inputLanguages->addButton(ui->inputLanguageButton3);

    outputLanguages->addButton(ui->outputLanguagesButton, 0);
    outputLanguages->addButton(ui->outputLanguageButton1);
    outputLanguages->addButton(ui->outputLanguageButton2);
    outputLanguages->addButton(ui->outputLanguageButton3);

    // Create context menu for tray
    trayShowWindow = new QAction(tr("Show window"));
    traySettings = new QAction(tr("Settings"));
    trayExit = new QAction(tr("Exit"));

    trayMenu->addAction(trayShowWindow);
    trayMenu->addAction(traySettings);
    trayMenu->addAction(trayExit);

    connect(trayShowWindow, &QAction::triggered, this, &MainWindow::show);
    connect(traySettings, &QAction::triggered, this, &MainWindow::on_settingsButton_clicked);
    connect(trayExit, &QAction::triggered, qApp, &QApplication::quit);

    // Load language quick-buttons
    inputLanguages->loadSettings();
    outputLanguages->loadSettings();

    // Load app settings
    loadSettings();

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_translateButton_clicked()
{
    if(ui->inputEdit->toPlainText() != "") {
        QString translatedText = QOnlineTranslator::translate(ui->inputEdit->toPlainText(), inputLanguages->checkedId(), outputLanguages->checkedId());
        ui->outputEdit->setPlainText(translatedText);
    }
    else qDebug() << tr("Text field is empty");
}

void MainWindow::on_inputLanguagesButton_triggered(QAction *language)
{
    short languageIndex = language->data().toInt();
    inputLanguages->insertLanguage(languageIndex);
}

void MainWindow::on_outputLanguagesButton_triggered(QAction *language)
{
    short languageIndex = language->data().toInt();
    outputLanguages->insertLanguage(languageIndex);
}

void MainWindow::on_swapButton_clicked()
{
    ButtonGroupLanguages::swapChecked(inputLanguages, outputLanguages);
    on_translateButton_clicked();
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(this);
    connect(&config, &SettingsDialog::languageChanged, this, &MainWindow::reloadTranslation);
    if (config.exec()) {
        //Load generic settings
        this->loadSettings();
        // Load language quick-buttons
        inputLanguages->loadSettings();
        outputLanguages->loadSettings();
    }
}

void MainWindow::on_inputSpeakButton_clicked()
{
    if (ui->inputEdit->toPlainText() != "") QOnlineTranslator::say(ui->inputEdit->toPlainText(), inputLanguages->checkedId());
    else qDebug() << tr("Text field is empty");
}

void MainWindow::on_outputSpeakButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "") QOnlineTranslator::say(ui->outputEdit->toPlainText(), outputLanguages->checkedId());
    else qDebug() << tr("Text field is empty");
}

void MainWindow::on_inputCopyButton_clicked()
{
    if (ui->inputEdit->toPlainText() != "") QApplication::clipboard()->setText(ui->inputEdit->toPlainText());
    else qDebug() << tr("Text field is empty");
}

void MainWindow::on_outputCopyButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "") QApplication::clipboard()->setText(ui->outputEdit->toPlainText());
    else qDebug() << tr("Text field is empty");
}

void MainWindow::on_tray_activated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (!this->isVisible()) {
            on_showMainWindowHotkey_activated();
        } else {
            this->hide();
        }
    }
}

void MainWindow::on_translateSelectedHotkey_activated()
{
    // Send selected text to input field
    ui->inputEdit->setPlainText(getSelectedText());

    // Translate selected text
    QSettings settings;
    if (this->isHidden() && settings.value("WindowMode", 0).toInt() == 0) {
        // Translate in popup
        PopupWindow *popup = new PopupWindow(languagesMenu, ui->inputEdit->toPlainText());
        popup->show();
    }
    else {
        // Translate in main window
        this->on_showMainWindowHotkey_activated();
        this->on_translateButton_clicked();
    }
}

void MainWindow::on_speakHotkey_activated()
{
    QOnlineTranslator::say(getSelectedText(), outputLanguages->checkedId());
}

// Show main window
void MainWindow::on_showMainWindowHotkey_activated()
{
    // Load language quick-buttons
    inputLanguages->loadSettings();
    outputLanguages->loadSettings();

    this->showNormal();
    this->activateWindow();
}

void MainWindow::reloadTranslation()
{
    QSettings settings;
    if (translator.load(QLocale(qvariant_cast<QLocale::Language>(settings.value("Language", 0))), QString("crow"), QString("_"), QString(":/translations")))
        qApp->installTranslator(&translator);

    ui->retranslateUi(this);
    trayShowWindow->setText("Show window");
    traySettings->setText(tr("Settings"));
    trayExit->setText(tr("Exit"));
    for (auto i=0; i<languagesMenu->actions().size(); i++)
        languagesMenu->actions().at(i)->setText(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(i))));
    inputLanguages->loadSettings();
    outputLanguages->loadSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    // Load icons
    this->setWindowIcon(QIcon(SettingsDialog::ICONS.at(settings.value("AppIcon", 0).toInt())));
    trayIcon->setIcon(QIcon(SettingsDialog::ICONS.at(settings.value("TrayIcon", 0).toInt())));

    // Load tray visibility
    trayIcon->setVisible(settings.value("Tray", true).toBool());
    QApplication::setQuitOnLastWindowClosed(!settings.value("TrayVisible", true).toBool());

    // Load shortcuts
    translateSelectedHotkey->setShortcut(QKeySequence(settings.value("Hotkeys/TranslateSelected", "Alt+X").toString()), true);
    speakHotkey->setShortcut(QKeySequence(settings.value("Hotkeys/SpeakSelected", "Alt+S").toString()), true);
    showMainWindowHotkey->setShortcut(QKeySequence(settings.value("Hotkeys/ShowMainWindow", "Alt+C").toString()), true);

    ui->translateButton->setShortcut(settings.value("Hotkeys/TranslateInput", "Ctrl+Return").toString());
    ui->inputSpeakButton->setShortcut(settings.value("Hotkeys/SpeakInput", "Ctrl+S").toString());
    ui->outputSpeakButton->setShortcut(settings.value("Hotkeys/SpeakOutput", "Ctrl+Shift+S").toString());
}

// Get selection
QString MainWindow::getSelectedText()
{
    QString selectedText;
#if defined(Q_OS_LINUX)
    selectedText = QApplication::clipboard()->text(QClipboard::Selection);
#elif defined(Q_OS_WIN)
    QString originalText = QApplication::clipboard()->text();

    // Some WinAPI code to send Ctrl+C
    INPUT copyText;
    copyText.type = INPUT_KEYBOARD;
    copyText.ki.wScan = 0;
    copyText.ki.time = 0;
    copyText.ki.dwExtraInfo = 0;

    Sleep(200);
    // Press the "Ctrl" key
    copyText.ki.wVk = VK_CONTROL;
    copyText.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &copyText, sizeof(INPUT));

    // Press the "C" key
    copyText.ki.wVk = 'C';
    copyText.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &copyText, sizeof(INPUT));

    Sleep(50);

    // Release the "C" key
    copyText.ki.wVk = 'C';
    copyText.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &copyText, sizeof(INPUT));

    // Release the "Ctrl" key
    copyText.ki.wVk = VK_CONTROL;
    copyText.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &copyText, sizeof(INPUT));
    Sleep(50);

    // If no text copied use previous text from clipboard
    if (QApplication::clipboard()->text() == "") selectedText = originalText;
    else selectedText = QApplication::clipboard()->text();

    QApplication::clipboard()->setText(originalText);
#endif
    return selectedText;
}
