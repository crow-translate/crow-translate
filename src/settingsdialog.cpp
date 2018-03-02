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

#include "settingsdialog.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>

#include "ui_settingsdialog.h"

const QStringList SettingsDialog::ICONS = { ":/icons/app/classic.png", ":/icons/app/black.png", ":/icons/app/white.png", ":/icons/app/papirus.png" };

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Add items in comboboxes
    ui->languageComboBox->addItem(tr("System language"), QLocale::AnyLanguage);
    ui->languageComboBox->addItem("English", QLocale::English);
    ui->languageComboBox->addItem("Русский", QLocale::Russian);

    ui->windowModeComboBox->addItem(tr("Popup"));
    ui->windowModeComboBox->addItem(tr("Full window"));

    ui->appIconComboBox->addItem(QIcon(ICONS.at(0)), tr("Classic"));
    ui->appIconComboBox->addItem(QIcon(ICONS.at(1)), tr("Black"));
    ui->appIconComboBox->addItem(QIcon(ICONS.at(2)), tr("White"));
    ui->appIconComboBox->addItem(QIcon(ICONS.at(3)), "Papirus");

    ui->trayIconComboBox->addItem(QIcon(ICONS.at(0)), tr("Classic"));
    ui->trayIconComboBox->addItem(QIcon(ICONS.at(1)), tr("Black"));
    ui->trayIconComboBox->addItem(QIcon(ICONS.at(2)), tr("White"));
    ui->trayIconComboBox->addItem(QIcon(ICONS.at(3)), "Papirus");

    // Disable (enable) opacity slider if "Window mode" ("Popup mode") selected
    connect(ui->windowModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), ui->popupOpacityLabel, &QSlider::setDisabled);
    connect(ui->windowModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), ui->popupOpacitySlider, &QSlider::setDisabled);

    connect(ui->popupOpacitySlider, &QSlider::valueChanged, ui->popupOpacitySpinBox, &QSpinBox::setValue);
    connect(ui->popupOpacitySpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->popupOpacitySlider, &QSlider::setValue);

    this->loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_dialogBox_accepted()
{
    QSettings settings;

    // Check if language changed
    if (settings.value("Language", 0) != ui->languageComboBox->currentData()) {
        settings.setValue("Language", ui->languageComboBox->currentData());
        emit languageChanged(); // Emit signal if language changed
    }

    // Check if autostart options changed
    if (settings.value("Autostart", false).toBool() != ui->autostartCheckBox->isChecked()) {
        settings.setValue("Autostart", ui->autostartCheckBox->isChecked());
#if defined(Q_OS_LINUX)
        QString autostartPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QString("/autostart");
        QDir autorunDir(autostartPath);
        if(!autorunDir.exists()) autorunDir.mkpath(autostartPath); // Create autostart folder if it does not exist
        QFile autorunFile(autostartPath + QString("/crow-translate.desktop"));
        if(ui->autostartCheckBox->isChecked()) {
            // Create autorun file if checked
            if(!autorunFile.exists()){
                if(autorunFile.open(QFile::WriteOnly)){
                    QString autorunContent("[Desktop Entry]\n"
                                           "Type=Application\n"
                                           "Exec=" + QCoreApplication::applicationFilePath() + "\n"
                                           "Hidden=false\n"
                                           "NoDisplay=false\n"
                                           "Icon=crow-translate\n"
                                           "Name=Crow Translate\n"
                                           "Comment=A simple and lightweight translator that allows to translate and speak the selected text using the Google Translate API\n"
                                           "Comment[ru]=Простой и легковесный переводчик, который позволяет переводить и озвучивать выделенный текст с помощью Google Translate API.\n");
                    QTextStream outStream(&autorunFile);
                    outStream << autorunContent;

                    // Set permissions
                    autorunFile.setPermissions(QFileDevice::ExeUser|QFileDevice::ExeOwner|QFileDevice::ExeOther|QFileDevice::ExeGroup|
                                               QFileDevice::WriteUser|QFileDevice::ReadUser);
                    autorunFile.close();
                }
            }
        }
        // Remove autorun file if box unchecked
        else if(autorunFile.exists()) autorunFile.remove();
#elif defined(Q_OS_WIN)
        if (ui->autostartCheckBox->isChecked()) {
            QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
            settings.setValue("Crow Translate", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        }
        else {
            QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
            settings.remove("Crow Translate");
        }
#endif
    }

    // Other general settings
    settings.setValue("WindowMode", ui->windowModeComboBox->currentIndex());
    settings.setValue("PopupOpacity", static_cast<double>(ui->popupOpacitySlider->value()) / 100);
    settings.setValue("AppIcon", ui->appIconComboBox->currentIndex());
    settings.setValue("TrayIcon", ui->trayIconComboBox->currentIndex());
    settings.setValue("TrayVisible", ui->trayCheckBox->isChecked());
    settings.setValue("StartMinimized", ui->startMinimizedCheckBox->isChecked());

    // Global shortcuts
    settings.setValue("Hotkeys/TranslateSelected", ui->translateSelectedSequenceEdit->keySequence());
    settings.setValue("Hotkeys/SpeakSelected", ui->speakSelectedSequenceEdit->keySequence());
    settings.setValue("Hotkeys/ShowMainWindow", ui->showMainWindowSequenceEdit->keySequence());

    // Window shortcuts
    settings.setValue("Hotkeys/TranslateInput", ui->translateInputSequenceEdit->keySequence());
    settings.setValue("Hotkeys/SpeakInput", ui->speakInputSequenceEdit->keySequence());
    settings.setValue("Hotkeys/SpeakOutput", ui->speakOutputSequenceEdit->keySequence());
}

// Disable (enable) and make unchecked "Start minimized" option when tray mode is disabled (enabled)
void SettingsDialog::on_trayCheckBox_toggled(bool checked)
{
    ui->startMinimizedCheckBox->setEnabled(checked);
    ui->startMinimizedCheckBox->setChecked(false);
}

void SettingsDialog::on_resetButton_clicked()
{
    QSettings settings;
    settings.clear();
    accept();
}

void SettingsDialog::loadSettings()
{
    QSettings settings;

    // General settings
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData(settings.value("Language", 0)));
    ui->windowModeComboBox->setCurrentIndex(settings.value("WindowMode", 0).toInt());
    ui->popupOpacitySlider->setValue(settings.value("PopupOpacity", 0.8).toDouble() * 100);
    ui->appIconComboBox->setCurrentIndex(settings.value("AppIcon", 0).toInt());
    ui->trayIconComboBox->setCurrentIndex(settings.value("TrayIcon", 0).toInt());
    ui->trayCheckBox->setChecked(settings.value("TrayVisible", true).toBool());
    ui->startMinimizedCheckBox->setChecked(settings.value("StartMinimized", false).toBool());
    ui->autostartCheckBox->setChecked(settings.value("Autostart", false).toBool());

    // Global shortcuts
    ui->translateSelectedSequenceEdit->setKeySequence(settings.value("Hotkeys/TranslateSelected", "Alt+X").toString());
    ui->speakSelectedSequenceEdit->setKeySequence(settings.value("Hotkeys/SpeakSelected", "Alt+S").toString());
    ui->showMainWindowSequenceEdit->setKeySequence(settings.value("Hotkeys/ShowMainWindow", "Alt+C").toString());

    // Window shortcuts
    ui->translateInputSequenceEdit->setKeySequence(settings.value("Hotkeys/TranslateInput", "Ctrl+Return").toString());
    ui->speakInputSequenceEdit->setKeySequence(settings.value("Hotkeys/SpeakInput", "Ctrl+S").toString());
    ui->speakOutputSequenceEdit->setKeySequence(settings.value("Hotkeys/SpeakOutput", "Ctrl+Shift+S").toString());
}
