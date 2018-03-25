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
    sourceButtonGroup (new LanguageButtonsGroup(this, "Source")),
    targetButtonGroup (new LanguageButtonsGroup(this, "Target"))
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

    languagesMenu->addActions(languagesList());

    // Add languageMenu to auto-language buttons
    ui->sourceAutoButton->setMenu(languagesMenu);
    ui->targetAutoButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    sourceButtonGroup->addButton(ui->sourceAutoButton, 0);
    sourceButtonGroup->addButton(ui->sourceFirstButton, 1);
    sourceButtonGroup->addButton(ui->sourceSecondButton, 2);
    sourceButtonGroup->addButton(ui->sourceThirdButton, 3);
    targetButtonGroup->addButton(ui->targetAutoButton, 0);
    targetButtonGroup->addButton(ui->targetFirstButton, 1);
    targetButtonGroup->addButton(ui->targetSecondButton, 2);
    targetButtonGroup->addButton(ui->targetThirdButton, 3);

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
    targetButtonGroup->loadSettings();
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_translateButton_clicked()
{
    if (ui->sourceEdit->toPlainText() != "") {
        QSettings settings;
        QString sourcelanguage = sourceButtonGroup->checkedButton()->toolTip();
        QString translationlanguage = targetButtonGroup->checkedButton()->toolTip();
        QString translatorlanguage = settings.value("Language", "auto").toString();
        m_translationData.translate(ui->sourceEdit->toPlainText(), translationlanguage, sourcelanguage, translatorlanguage);

        // Show translation and transcription
        ui->translationEdit->setText(m_translationData.text());
        if (m_translationData.targetTranscription() != "")
            ui->translationEdit->append("<font color=\"grey\"><i>/" + m_translationData.targetTranscription() + "/</i></font>");
        if (m_translationData.sourceTranscription() != "")
            ui->translationEdit->append("<font color=\"grey\"><i><b>(" + m_translationData.sourceTranscription() + ")</b></i></font>");
        ui->translationEdit->append("");

        // Show translation options
        foreach (auto translationOptions, m_translationData.options()) {
            ui->translationEdit->append("<i>" + translationOptions.first + "</i>");
            foreach (QString wordsList, translationOptions.second) {
                wordsList.prepend("&nbsp;&nbsp;<b>");
                wordsList.insert(wordsList.indexOf(":") + 1, "</b>");
                ui->translationEdit->append(wordsList);
            }
            ui->translationEdit->append("");
        }

        ui->translationEdit->moveCursor(QTextCursor::Start);
        emit translationChanged(ui->translationEdit->toHtml());
    }
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_sourceAutoButton_triggered(QAction *language)
{
    sourceButtonGroup->insertLanguage(language->toolTip());
}

void MainWindow::on_targetAutoButton_triggered(QAction *language)
{
    targetButtonGroup->insertLanguage(language->toolTip());
}

void MainWindow::on_swapButton_clicked()
{
    LanguageButtonsGroup::swapChecked(sourceButtonGroup, targetButtonGroup);
    on_translateButton_clicked();
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog config(this);
    connect(&config, &SettingsDialog::languageChanged, this, &MainWindow::reloadTranslation);
    if (config.exec()) {
        this->loadSettings();
        sourceButtonGroup->loadSettings();
        targetButtonGroup->loadSettings();
    }
}

void MainWindow::on_sourceSayButton_clicked()
{
    if (ui->sourceEdit->toPlainText() != "")
        QOnlineTranslator::say(ui->sourceEdit->toPlainText(), sourceButtonGroup->checkedButton()->toolTip());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_targetSayButton_clicked()
{
    if (ui->translationEdit->toPlainText() != "")
        QOnlineTranslator::say(m_translationData.text(), targetButtonGroup->checkedButton()->toolTip());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_sourceCopyButton_clicked()
{
    if (ui->sourceEdit->toPlainText() != "")
        QApplication::clipboard()->setText(ui->sourceEdit->toPlainText());
    else
        qDebug() << tr("Text field is empty");
}

void MainWindow::on_targetCopyButton_clicked()
{
    if (ui->translationEdit->toPlainText() != "")
        QApplication::clipboard()->setText(ui->translationEdit->toPlainText());
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
    ui->sourceEdit->setPlainText(selectedText());

    on_translateButton_clicked();

    QSettings settings;
    if (this->isHidden() && settings.value("WindowMode", 0).toInt() == 0) {
        // Show popup
        PopupWindow *popup = new PopupWindow(languagesMenu, ui->translationEdit->toHtml(), this);
        connect(this, &MainWindow::translationChanged, popup, &PopupWindow::setTranslation);
        connect(popup, &PopupWindow::sourceLanguageButtonPressed, sourceButtonGroup, &LanguageButtonsGroup::setChecked);
        connect(popup, &PopupWindow::targetLanguageButtonPressed, targetButtonGroup, &LanguageButtonsGroup::setChecked);
        connect(popup, &PopupWindow::sourceLanguageButtonPressed, this, &MainWindow::on_translateButton_clicked);
        connect(popup, &PopupWindow::targetLanguageButtonPressed, this, &MainWindow::on_translateButton_clicked);
        connect(popup, &PopupWindow::sourceLanguageInserted, this, &MainWindow::on_sourceAutoButton_triggered);
        connect(popup, &PopupWindow::targetLanguageInserted, this, &MainWindow::on_targetAutoButton_triggered);
        connect(popup, &PopupWindow::swapButtonClicked, this, &MainWindow::on_swapButton_clicked);
        connect(popup, &PopupWindow::sayButtonClicked, this, &MainWindow::on_targetSayButton_clicked);
        popup->show();
    }
    else
        // Show main window
        on_showMainWindowHotkey_activated();
}

void MainWindow::on_speakHotkey_activated()
{
    QOnlineTranslator::say(selectedText(), targetButtonGroup->checkedButton()->toolTip());
}

void MainWindow::on_showMainWindowHotkey_activated()
{
    sourceButtonGroup->loadSettings();
    targetButtonGroup->loadSettings();

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

    // Reload UI
    ui->retranslateUi(this);
    trayShowWindow->setText("Show window");
    traySettings->setText(tr("Settings"));
    trayExit->setText(tr("Exit"));
    languagesMenu->clear();
    languagesMenu->addActions(languagesList());
    sourceButtonGroup->loadSettings();
    targetButtonGroup->loadSettings();
}

QList<QAction *> MainWindow::languagesList()
{
    // Load all languages and codes from QOnlineTranslator
    QList<QAction *> languagesList;
    for (auto i=0; i<QOnlineTranslator::LANGUAGE_NAMES.size(); i++) {
        QAction *action = new QAction(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(i))));
        action->setToolTip(QOnlineTranslator::LANGUAGE_SHORT_CODES.at(i));
        languagesList.append(action);
    }

    // Sort alphabetically for easy access
    std::sort(languagesList.begin() + 1, languagesList.end(), [](QAction *first, QAction *second) {
        return first->text() < second->text();
    } );

    return languagesList;
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
    ui->sourceSayButton->setShortcut(settings.value("Hotkeys/SpeakInput", "Ctrl+S").toString());
    ui->targetSayButton->setShortcut(settings.value("Hotkeys/SpeakOutput", "Ctrl+Shift+S").toString());
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
