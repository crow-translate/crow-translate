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
    sourceButtonGroup (new LanguageButtonsGroup(this, "Input")),
    translationButtonGroup (new LanguageButtonsGroup(this, "Output"))
{
    // Set object names for signals autoconnection
    trayIcon->setObjectName("tray");
    translateSelectedHotkey->setObjectName("translateSelectedHotkey");
    speakHotkey->setObjectName("speakHotkey");
    showMainWindowHotkey->setObjectName("showMainWindowHotkey");

    // Load translation
    QSettings settings;
    QString localeCode = settings.value("Language", "auto").toString();
    if (localeCode == "auto")
        QLocale::setDefault(QLocale::system());
    else
        QLocale::setDefault(QLocale(localeCode));
    if (translator.load(QLocale(), QString("crow"), QString("_"), QString(":/translations")))
        qApp->installTranslator(&translator);

    ui->setupUi(this);

    // Add all languages to languageMenu
    for (auto i=0; i<QOnlineTranslator::LANGUAGE_NAMES.size(); i++) {
        QAction *action = new QAction(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(i))));
        action->setData(QOnlineTranslator::LANGUAGE_SHORT_CODES.at(i));
        languagesMenu->addAction(action);
    }

    // Add languageMenu to auto-language buttons
    ui->autoLanguageSourceButton->setMenu(languagesMenu);
    ui->autoLanguageTranslationButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    sourceButtonGroup->addButton(ui->autoLanguageSourceButton, 0);
    sourceButtonGroup->addButton(ui->languageSourceButton1, 1);
    sourceButtonGroup->addButton(ui->languageSourceButton2, 2);
    sourceButtonGroup->addButton(ui->languageSourceButton3, 3);
    translationButtonGroup->addButton(ui->autoLanguageTranslationButton, 0);
    translationButtonGroup->addButton(ui->languageTranslationButton1, 1);
    translationButtonGroup->addButton(ui->languageTranslationButton2, 2);
    translationButtonGroup->addButton(ui->languageTranslationButton3, 3);

    // Create context menu for tray
    trayShowWindow = new QAction(tr("Show window"));
    traySettings = new QAction(tr("Settings"));
    trayExit = new QAction(tr("Exit"));
    trayMenu->addAction(trayShowWindow);
    trayMenu->addAction(traySettings);
    trayMenu->addAction(trayExit);
    trayIcon->setContextMenu(trayMenu);
    connect(trayShowWindow, &QAction::triggered, this, &MainWindow::show);
    connect(traySettings, &QAction::triggered, this, &MainWindow::on_settingsButton_clicked);
    connect(trayExit, &QAction::triggered, qApp, &QApplication::quit);

    sourceButtonGroup->loadSettings();
    translationButtonGroup->loadSettings();
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_translateButton_clicked()
{
    if (ui->inputEdit->toPlainText() != "") {
        QSettings settings;
        QString sourcelanguage = sourceButtonGroup->checkedButton()->toolTip();
        QString translationlanguage = translationButtonGroup->checkedButton()->toolTip();
        QString translatorlanguage = settings.value("Language", "auto").toString();
        QOnlineTranslator onlineTranslator(ui->inputEdit->toPlainText(), translationlanguage, sourcelanguage, translatorlanguage);

        // Show translation and transcription
        ui->outputEdit->setText(onlineTranslator.text());
        if (onlineTranslator.translationTranscription() != "")
            ui->outputEdit->append("<font color=\"grey\"><i>/" + onlineTranslator.translationTranscription() + "/</i></font>");
        if (onlineTranslator.sourceTranscription() != "")
            ui->outputEdit->append("<font color=\"grey\"><i><b>(" + onlineTranslator.sourceTranscription() + ")</b></i></font>");
        ui->outputEdit->append("");

        // Show translation options
        foreach (auto translationOptions, onlineTranslator.options()) {
            ui->outputEdit->append("<i>" + translationOptions.first + "</i>");
            foreach (QString wordsList, translationOptions.second) {
                wordsList.prepend("&nbsp;&nbsp;<b>");
                wordsList.insert(wordsList.indexOf(":") + 1, "</b>");
                ui->outputEdit->append(wordsList);
            }
            ui->outputEdit->append("");
        }
    }
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_autoLanguageSourceButton_triggered(QAction *language)
{
    sourceButtonGroup->insertLanguage(language->data().toString());
}

void MainWindow::on_autoLanguageTranslationButton_triggered(QAction *language)
{
    translationButtonGroup->insertLanguage(language->data().toString());
}

void MainWindow::on_swapButton_clicked()
{
    LanguageButtonsGroup::swapChecked(sourceButtonGroup, translationButtonGroup);
    on_translateButton_clicked();
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(this);
    connect(&config, &SettingsDialog::languageChanged, this, &MainWindow::reloadTranslation);
    if (config.exec()) {
        this->loadSettings();
        sourceButtonGroup->loadSettings();
        translationButtonGroup->loadSettings();
    }
}

void MainWindow::on_speakSourceButton_clicked()
{
    if (ui->inputEdit->toPlainText() != "")
        QOnlineTranslator::say(ui->inputEdit->toPlainText(), sourceButtonGroup->checkedButton()->toolTip());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_speakTranslationButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "")
        QOnlineTranslator::say(ui->outputEdit->toPlainText(), translationButtonGroup->checkedButton()->toolTip());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_copySourceButton_clicked()
{
    if (ui->inputEdit->toPlainText() != "")
        QApplication::clipboard()->setText(ui->inputEdit->toPlainText());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_copyTranslationButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "")
        QApplication::clipboard()->setText(ui->outputEdit->toPlainText());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_tray_activated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (!this->isVisible())
            on_showMainWindowHotkey_activated();
        else
            this->hide();
    }
}

void MainWindow::on_translateSelectedHotkey_activated()
{
    // Send selected text to input field
    ui->inputEdit->setPlainText(selectedText());

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
    QOnlineTranslator::say(selectedText(), translationButtonGroup->checkedButton()->toolTip());
}

void MainWindow::on_showMainWindowHotkey_activated()
{
    sourceButtonGroup->loadSettings();
    translationButtonGroup->loadSettings();

    this->showNormal();
    this->activateWindow();
}

void MainWindow::reloadTranslation()
{
    // Install translation
    QSettings settings;
    QString localeCode = settings.value("Language", "auto").toString();
    if (localeCode == "auto")
        QLocale::setDefault(QLocale::system());
    else
        QLocale::setDefault(QLocale(localeCode));
    if (translator.load(QLocale(), QString("crow"), QString("_"), QString(":/translations")))
        qApp->installTranslator(&translator);

    // Retranslate UI
    ui->retranslateUi(this);
    trayShowWindow->setText("Show window");
    traySettings->setText(tr("Settings"));
    trayExit->setText(tr("Exit"));
    for (auto i=0; i<languagesMenu->actions().size(); i++)
        languagesMenu->actions().at(i)->setText(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(i))));
    sourceButtonGroup->loadSettings();
    translationButtonGroup->loadSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    // Load icons
    this->setWindowIcon(QIcon(SettingsDialog::ICONS.at(settings.value("AppIcon", 0).toInt())));
    trayIcon->setIcon(QIcon(SettingsDialog::ICONS.at(settings.value("TrayIcon", 0).toInt())));

    // Load tray visibility
    trayIcon->setVisible(settings.value("TrayIconVisible", true).toBool());
    QApplication::setQuitOnLastWindowClosed(!settings.value("TrayIconVisible", true).toBool());

    // Load shortcuts
    translateSelectedHotkey->setShortcut(QKeySequence(settings.value("Hotkeys/TranslateSelected", "Alt+X").toString()), true);
    speakHotkey->setShortcut(QKeySequence(settings.value("Hotkeys/SpeakSelected", "Alt+S").toString()), true);
    showMainWindowHotkey->setShortcut(QKeySequence(settings.value("Hotkeys/ShowMainWindow", "Alt+C").toString()), true);
    ui->translateButton->setShortcut(settings.value("Hotkeys/TranslateInput", "Ctrl+Return").toString());
    ui->speakSourceButton->setShortcut(settings.value("Hotkeys/SpeakInput", "Ctrl+S").toString());
    ui->speakTranslationButton->setShortcut(settings.value("Hotkeys/SpeakOutput", "Ctrl+Shift+S").toString());
}

QString MainWindow::selectedText()
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
