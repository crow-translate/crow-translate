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
#include <QNetworkProxy>

#include "qonlinetranslator.h"
#include "ui_settingsdialog.h"

const QStringList SettingsDialog::ICONS = { ":/icons/app/classic.png", ":/icons/app/black.png", ":/icons/app/white.png", ":/icons/app/papirus.png" };

SettingsDialog::SettingsDialog(QMenu *languagesMenu, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Add items in comboboxes
    ui->proxyTypeComboBox->setItemData(0, QNetworkProxy::ProxyType::DefaultProxy);
    ui->proxyTypeComboBox->setItemData(1, QNetworkProxy::ProxyType::NoProxy);
    ui->proxyTypeComboBox->setItemData(2, QNetworkProxy::ProxyType::HttpProxy);

    ui->languageComboBox->setItemData(0, "auto");
    ui->languageComboBox->setItemData(1, "en");
    ui->languageComboBox->setItemData(2, "ru");

    ui->primaryLanguageComboBox->addItem(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(0))), QOnlineTranslator::LANGUAGE_SHORT_CODES.at(0));
    ui->secondaryLanguageComboBox->addItem(QCoreApplication::translate("QOnlineTranslator", qPrintable(QOnlineTranslator::LANGUAGE_NAMES.at(0))), QOnlineTranslator::LANGUAGE_SHORT_CODES.at(0));
    foreach (auto language, languagesMenu->actions()) {
        ui->primaryLanguageComboBox->addItem(language->text(), language->toolTip());
        ui->secondaryLanguageComboBox->addItem(language->text(), language->toolTip());
    }

    // Disable (enable) opacity slider if "Window mode" ("Popup mode") selected
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacityLabel, &QSlider::setDisabled);
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacitySlider, &QSlider::setDisabled);

    // Connect opacity slider and spinbox
    connect(ui->popupOpacitySlider, &QSlider::valueChanged, ui->popupOpacitySpinBox, &QSpinBox::setValue);
    connect(ui->popupOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged), ui->popupOpacitySlider, &QSlider::setValue);

    // Pages selection mechanism
    connect(ui->pagesListWidget, &QListWidget::currentRowChanged, ui->pagesStackedWidget, &QStackedWidget::setCurrentIndex);

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_dialogBox_accepted()
{
    QSettings settings;

    // Check if language changed
    if (settings.value("Language", "auto").toString() != ui->languageComboBox->currentData()) {
        settings.setValue("Language", ui->languageComboBox->currentData());
        emit languageChanged(); // Emit signal if language changed
    }

    // Check if proxy changed
    if (settings.value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt() != ui->proxyTypeComboBox->currentData() ||
            settings.value("Connection/ProxyHost", "").toString() != ui->proxyHostEdit->text() ||
            settings.value("Connection/ProxyPort", 8080).toInt() != ui->proxyPortSpinbox->value() ||
            settings.value("Connection/ProxyAuthEnabled", false).toInt() != ui->proxyAuthCheckBox->isChecked() ||
            settings.value("Connection/ProxyUsername", "").toString() != ui->proxyUsernameEdit->text() ||
            settings.value("Connection/ProxyPassword", "").toString() != ui->proxyPasswordEdit->text()) {
        settings.setValue("Connection/ProxyType", ui->proxyTypeComboBox->currentData());
        settings.setValue("Connection/ProxyHost", ui->proxyHostEdit->text());
        settings.setValue("Connection/ProxyPort", ui->proxyPortSpinbox->value());
        settings.setValue("Connection/ProxyAuthEnabled", ui->proxyAuthCheckBox->isChecked());
        settings.setValue("Connection/ProxyUsername", ui->proxyUsernameEdit->text());
        settings.setValue("Connection/ProxyPassword", ui->proxyPasswordEdit->text());
        emit proxyChanged(); // Emit signal if language changed
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
                                           "Comment=A simple and lightweight translator that allows to translate and say selected text using the Google Translate API\n"
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
        else
            if(autorunFile.exists())
                autorunFile.remove();
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
    settings.setValue("TrayIconVisible", ui->trayCheckBox->isChecked());
    settings.setValue("StartMinimized", ui->startMinimizedCheckBox->isChecked());

    // Automatic language detection
    settings.setValue("PrimaryLanguage", ui->primaryLanguageComboBox->currentData());
    settings.setValue("SecondaryLanguage", ui->secondaryLanguageComboBox->currentData());

    // Connection settings
    settings.setValue("Connection/ProxyType", ui->proxyTypeComboBox->currentData());
    settings.setValue("Connection/ProxyHost", ui->proxyHostEdit->text());
    settings.setValue("Connection/ProxyPort", ui->proxyPortSpinbox->value());
    settings.setValue("Connection/ProxyAuthEnabled", ui->proxyAuthCheckBox->isChecked());
    settings.setValue("Connection/ProxyUsername", ui->proxyUsernameEdit->text());
    settings.setValue("Connection/ProxyPassword", ui->proxyPasswordEdit->text());

    // Global shortcuts
    settings.setValue("Hotkeys/TranslateSelected", ui->translateSelectedSequenceEdit->keySequence());
    settings.setValue("Hotkeys/SaySelected", ui->saySelectedSequenceEdit->keySequence());
    settings.setValue("Hotkeys/ShowMainWindow", ui->showMainWindowSequenceEdit->keySequence());

    // Window shortcuts
    settings.setValue("Hotkeys/Translate", ui->translateSequenceEdit->keySequence());
    settings.setValue("Hotkeys/SaySource", ui->saySourceSequenceEdit->keySequence());
    settings.setValue("Hotkeys/SayTranslation", ui->sayTranslationSequenceEdit->keySequence());
    settings.setValue("Hotkeys/CopyTranslation", ui->copyTranslationSequenceEdit->keySequence());
    settings.setValue("Hotkeys/CloseWindow", ui->closeWindowSequenceEdit->keySequence());
}

// Disable (enable) "Start minimized" option when tray mode is disabled (enabled)
void SettingsDialog::on_trayCheckBox_toggled(bool checked)
{
    ui->startMinimizedCheckBox->setEnabled(checked);
    ui->startMinimizedCheckBox->setChecked(false);
}

void SettingsDialog::on_resetButton_clicked()
{
    // General settings
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData("auto"));
    ui->windowModeComboBox->setCurrentIndex(0);
    ui->popupOpacitySlider->setValue(80);
    ui->appIconComboBox->setCurrentIndex(0);
    ui->trayIconComboBox->setCurrentIndex(0);
    ui->trayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);

    // Automatic language detection
    ui->primaryLanguageComboBox->setCurrentIndex(0);
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->languageComboBox->findData("en"));

    // Connection settings
     ui->proxyTypeComboBox->setCurrentIndex(0);
     ui->proxyHostEdit->setText("");
     ui->proxyPortSpinbox->setValue(8080);
     ui->proxyAuthCheckBox->setChecked(false);
     ui->proxyUsernameEdit->setText("");
     ui->proxyPasswordEdit->setText("");

    // Global shortcuts
    ui->translateSelectedSequenceEdit->setKeySequence(QKeySequence("Ctrl+Alt+E"));
    ui->saySelectedSequenceEdit->setKeySequence(QKeySequence("Ctrl+Alt+S"));
    ui->showMainWindowSequenceEdit->setKeySequence(QKeySequence("Ctrl+Alt+C"));

    // Window shortcuts
    ui->translateSequenceEdit->setKeySequence(QKeySequence("Ctrl+Return"));
    ui->saySourceSequenceEdit->setKeySequence(QKeySequence("Ctrl+S"));
    ui->sayTranslationSequenceEdit->setKeySequence(QKeySequence("Ctrl+Shift+S"));
    ui->copyTranslationSequenceEdit->setKeySequence(QKeySequence("Ctrl+Shift+C"));
    ui->closeWindowSequenceEdit->setKeySequence(QKeySequence("Ctrl+Q"));
}

void SettingsDialog::on_proxyTypeComboBox_currentIndexChanged(int index)
{
    if (index == 2) {
        ui->proxyHostEdit->setEnabled(true);
        ui->proxyHostLabel->setEnabled(true);
        ui->proxyPortLabel->setEnabled(true);
        ui->proxyPortSpinbox->setEnabled(true);
        ui->proxyInfoLabel->setEnabled(true);
        ui->proxyAuthCheckBox->setEnabled(true);
    }
    else {
        ui->proxyHostEdit->setEnabled(false);
        ui->proxyHostLabel->setEnabled(false);
        ui->proxyPortLabel->setEnabled(false);
        ui->proxyPortSpinbox->setEnabled(false);
        ui->proxyInfoLabel->setEnabled(false);
        ui->proxyAuthCheckBox->setEnabled(false);
    }
}

void SettingsDialog::on_proxyAuthCheckBox_toggled(bool checked)
{
    ui->proxyUsernameEdit->setEnabled(checked);
    ui->proxyUsernameLabel->setEnabled(checked);
    ui->proxyPasswordEdit->setEnabled(checked);
    ui->proxyPasswordLabel->setEnabled(checked);
    ui->proxyPasswordInfoLabel->setEnabled(checked);
}

void SettingsDialog::loadSettings()
{
    QSettings settings;

    // General settings
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData(settings.value("Language", "auto").toString()));
    ui->windowModeComboBox->setCurrentIndex(settings.value("WindowMode", 0).toInt());
    ui->popupOpacitySlider->setValue(settings.value("PopupOpacity", 0.8).toDouble() * 100);
    ui->appIconComboBox->setCurrentIndex(settings.value("AppIcon", 0).toInt());
    ui->trayIconComboBox->setCurrentIndex(settings.value("TrayIcon", 0).toInt());
    ui->trayCheckBox->setChecked(settings.value("TrayIconVisible", true).toBool());
    ui->startMinimizedCheckBox->setChecked(settings.value("StartMinimized", false).toBool());
    ui->autostartCheckBox->setChecked(settings.value("Autostart", false).toBool());

    // Automatic language detection
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(settings.value("PrimaryLanguage", "auto").toString()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(settings.value("SecondaryLanguage", "en").toString()));

    // Connection settings
     ui->proxyTypeComboBox->setCurrentIndex(ui->proxyTypeComboBox->findData(settings.value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt()));
     ui->proxyHostEdit->setText(settings.value("Connection/ProxyHost", "").toString());
     ui->proxyPortSpinbox->setValue(settings.value("Connection/ProxyPort", 8080).toInt());
     ui->proxyAuthCheckBox->setChecked(settings.value("Connection/ProxyAuthEnabled", false).toBool());
     ui->proxyUsernameEdit->setText(settings.value("Connection/ProxyUsername", "").toString());
     ui->proxyPasswordEdit->setText(settings.value("Connection/ProxyPassword", "").toString());

    // Global shortcuts
    ui->translateSelectedSequenceEdit->setKeySequence(settings.value("Hotkeys/TranslateSelected", "Ctrl+Alt+E").toString());
    ui->saySelectedSequenceEdit->setKeySequence(settings.value("Hotkeys/SaySelected", "Ctrl+Alt+S").toString());
    ui->showMainWindowSequenceEdit->setKeySequence(settings.value("Hotkeys/ShowMainWindow", "Ctrl+Alt+C").toString());

    // Window shortcuts
    ui->translateSequenceEdit->setKeySequence(settings.value("Hotkeys/Translate", "Ctrl+Return").toString());
    ui->saySourceSequenceEdit->setKeySequence(settings.value("Hotkeys/SaySource", "Ctrl+S").toString());
    ui->sayTranslationSequenceEdit->setKeySequence(settings.value("Hotkeys/SayTranslation", "Ctrl+Shift+S").toString());
    ui->copyTranslationSequenceEdit->setKeySequence(settings.value("Hotkeys/CopyTranslation", "Ctrl+Shift+C").toString());
    ui->closeWindowSequenceEdit->setKeySequence(settings.value("Hotkeys/CloseWindow", "Ctrl+Q").toString());
}
