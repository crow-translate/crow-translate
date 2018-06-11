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

SettingsDialog::SettingsDialog(QMenu *languagesMenu, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->shortcutsTreeWidget->expandAll();
    ui->shortcutsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Set item data in comboboxes
    ui->trayIconComboBox->setItemData(0, "crow-translate");
    ui->trayIconComboBox->setItemData(1, ":/icons/app/dark-theme.png");
    ui->trayIconComboBox->setItemData(2, ":/icons/app/white-theme.png");
#if defined(Q_OS_LINUX)
    ui->trayIconComboBox->insertItem(0, QIcon::fromTheme("crow-translate-tray"), tr("From system theme"), "crow-translate-tray");
#endif

    ui->languageComboBox->setItemData(0, "auto");
    ui->languageComboBox->setItemData(1, "en");
    ui->languageComboBox->setItemData(2, "ru");

    ui->proxyTypeComboBox->setItemData(0, QNetworkProxy::ProxyType::DefaultProxy);
    ui->proxyTypeComboBox->setItemData(1, QNetworkProxy::ProxyType::NoProxy);
    ui->proxyTypeComboBox->setItemData(2, QNetworkProxy::ProxyType::HttpProxy);
    ui->proxyTypeComboBox->setItemData(3, QNetworkProxy::ProxyType::Socks5Proxy);

    ui->languagesStyleComboBox->setItemData(0, Qt::ToolButtonFollowStyle);
    ui->languagesStyleComboBox->setItemData(1, Qt::ToolButtonIconOnly);
    ui->languagesStyleComboBox->setItemData(2, Qt::ToolButtonTextOnly);
    ui->languagesStyleComboBox->setItemData(3, Qt::ToolButtonTextBesideIcon);
    ui->languagesStyleComboBox->setItemData(4, Qt::ToolButtonTextUnderIcon);

    ui->controlsStyleComboBox->setItemData(0, Qt::ToolButtonFollowStyle);
    ui->controlsStyleComboBox->setItemData(1, Qt::ToolButtonIconOnly);
    ui->controlsStyleComboBox->setItemData(2, Qt::ToolButtonTextOnly);
    ui->controlsStyleComboBox->setItemData(3, Qt::ToolButtonTextBesideIcon);
    ui->controlsStyleComboBox->setItemData(4, Qt::ToolButtonTextUnderIcon);

    ui->primaryLanguageComboBox->addItem(tr("<System language>"), "auto");
    ui->secondaryLanguageComboBox->addItem(tr("<System language>"), "auto");
    foreach (auto language, languagesMenu->actions()) {
        ui->primaryLanguageComboBox->addItem(language->icon(), language->text(), language->toolTip());
        ui->secondaryLanguageComboBox->addItem(language->icon(), language->text(), language->toolTip());
    }

    // Disable (enable) opacity slider if "Window mode" ("Popup mode") selected
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacityLabel, &QSlider::setDisabled);
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacitySlider, &QSlider::setDisabled);

    // Connect opacity slider and spinbox
    connect(ui->popupOpacitySlider, &QSlider::valueChanged, ui->popupOpacitySpinBox, &QSpinBox::setValue);
    connect(ui->popupOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged), ui->popupOpacitySlider, &QSlider::setValue);

    // Pages selection mechanism
    connect(ui->pagesListWidget, &QListWidget::currentRowChanged, ui->pagesStackedWidget, &QStackedWidget::setCurrentIndex);

    // General settings
    QSettings settings;
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData(settings.value("Language", "auto").toString()));
    ui->windowModeComboBox->setCurrentIndex(settings.value("WindowMode", 0).toInt());
    ui->trayCheckBox->setChecked(settings.value("TrayIconVisible", true).toBool());
    ui->startMinimizedCheckBox->setChecked(settings.value("StartMinimized", false).toBool());
    ui->autostartCheckBox->setChecked(settings.value("Autostart", false).toBool());

    // Interface settings
    ui->popupOpacitySlider->setValue(settings.value("PopupOpacity", 0.8).toDouble() * 100);
    ui->trayIconComboBox->setCurrentIndex(ui->trayIconComboBox->findData(settings.value("TrayIcon", "crow-translate").toString()));
    ui->languagesStyleComboBox->setCurrentIndex(ui->languagesStyleComboBox->findData(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle).toInt()));
    ui->controlsStyleComboBox->setCurrentIndex(ui->controlsStyleComboBox->findData(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle).toInt()));

    // Translation settings
    ui->sourceTransliterationCheckBox->setChecked(settings.value("Translation/ShowSourceTransliteration", true).toBool());
    ui->translationTransliterationCheckBox->setChecked(settings.value("Translation/ShowTranslationTransliteration", true).toBool());
    ui->translationOptionsCheckBox->setChecked(settings.value("Translation/TranslationOptions", true).toBool());
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(settings.value("Translation/PrimaryLanguage", "auto").toString()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(settings.value("Translation/SecondaryLanguage", "en").toString()));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(ui->proxyTypeComboBox->findData(settings.value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt()));
    ui->proxyHostEdit->setText(settings.value("Connection/ProxyHost", "").toString());
    ui->proxyPortSpinbox->setValue(settings.value("Connection/ProxyPort", 8080).toInt());
    ui->proxyAuthCheckBox->setChecked(settings.value("Connection/ProxyAuthEnabled", false).toBool());
    ui->proxyUsernameEdit->setText(settings.value("Connection/ProxyUsername", "").toString());
    ui->proxyPasswordEdit->setText(settings.value("Connection/ProxyPassword", "").toString());

    // Global shortcuts
    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setText(1, settings.value("Hotkeys/TranslateSelected", "Ctrl+Alt+E").toString());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setText(1, settings.value("Hotkeys/PlaySelected", "Ctrl+Alt+S").toString());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setText(1, settings.value("Hotkeys/ShowMainWindow", "Ctrl+Alt+C").toString());

    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setData(1, Qt::UserRole, "Ctrl+Alt+E;");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setData(1, Qt::UserRole, "Ctrl+Alt+S");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setData(1, Qt::UserRole, "Ctrl+Alt+C");

    // Window shortcuts
    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setText(1, settings.value("Hotkeys/Translate", "Ctrl+Return").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setText(1, settings.value("Hotkeys/CloseWindow", "Ctrl+Q").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setText(1, settings.value("Hotkeys/PlaySource", "Ctrl+S").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setText(1, settings.value("Hotkeys/StopSource", "Ctrl+D").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setText(1, settings.value("Hotkeys/PlayTranslation", "Ctrl+Shift+S").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setText(1, settings.value("Hotkeys/StopTranslation", "Ctrl+Shift+D").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setText(1, settings.value("Hotkeys/CopyTranslation", "Ctrl+Shift+C").toString());

    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setData(1, Qt::UserRole, "Ctrl+Return");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setData(1, Qt::UserRole, "Ctrl+Q");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setData(1, Qt::UserRole, "Ctrl+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setData(1, Qt::UserRole, "Ctrl+D");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setData(1, Qt::UserRole, "Ctrl+Shift+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setData(1, Qt::UserRole, "Ctrl+Shift+D");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setData(1, Qt::UserRole, "Ctrl+Shift+C");
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
                                           "Comment=A simple and lightweight translator that allows to translate and say selected text using the Google Translate API and much more\n"
                                           "Comment[ru]=Простой и легковесный переводчик, который позволяет переводить и озвучивать выделенный текст с помощью Google Translate API, а также многое другое\n");
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
    settings.setValue("TrayIconVisible", ui->trayCheckBox->isChecked());
    settings.setValue("StartMinimized", ui->startMinimizedCheckBox->isChecked());

    // Interface settings
    settings.setValue("PopupOpacity", static_cast<double>(ui->popupOpacitySlider->value()) / 100);
    settings.setValue("TrayIcon", ui->trayIconComboBox->currentData());
    settings.setValue("LanguagesStyle", ui->languagesStyleComboBox->currentData());
    settings.setValue("ControlsStyle", ui->controlsStyleComboBox->currentData());

    // Translation settings
    settings.setValue("Translation/ShowSourceTransliteration", ui->sourceTransliterationCheckBox->isChecked());
    settings.setValue("Translation/ShowTranslationTransliteration", ui->translationTransliterationCheckBox->isChecked());
    settings.setValue("Translation/TranslationOptions", ui->translationOptionsCheckBox->isChecked());
    settings.setValue("Translation/PrimaryLanguage", ui->primaryLanguageComboBox->currentData());
    settings.setValue("Translation/SecondaryLanguage", ui->secondaryLanguageComboBox->currentData());

    // Connection settings
    settings.setValue("Connection/ProxyType", ui->proxyTypeComboBox->currentData());
    settings.setValue("Connection/ProxyHost", ui->proxyHostEdit->text());
    settings.setValue("Connection/ProxyPort", ui->proxyPortSpinbox->value());
    settings.setValue("Connection/ProxyAuthEnabled", ui->proxyAuthCheckBox->isChecked());
    settings.setValue("Connection/ProxyUsername", ui->proxyUsernameEdit->text());
    settings.setValue("Connection/ProxyPassword", ui->proxyPasswordEdit->text());

    // Global shortcuts
    settings.setValue("Hotkeys/TranslateSelected", ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->text(1));
    settings.setValue("Hotkeys/PlaySelected", ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->text(1));
    settings.setValue("Hotkeys/ShowMainWindow", ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->text(1));

    // Window shortcuts
    settings.setValue("Hotkeys/Translate", ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->text(1));
    settings.setValue("Hotkeys/CloseWindow", ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->text(1));
    settings.setValue("Hotkeys/PlaySource", ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->text(1));
    settings.setValue("Hotkeys/StopSource", ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->text(1));
    settings.setValue("Hotkeys/PlayTranslation", ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->text(1));
    settings.setValue("Hotkeys/StopTranslation", ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->text(1));
    settings.setValue("Hotkeys/CopyTranslation", ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->text(1));
}

// Disable (enable) "Start minimized" option when tray mode is disabled (enabled)
void SettingsDialog::on_trayCheckBox_toggled(bool checked)
{
    ui->startMinimizedCheckBox->setEnabled(checked);
    ui->startMinimizedCheckBox->setChecked(false);
}

void SettingsDialog::on_resetSettingsButton_clicked()
{
    // General settings
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData("auto"));
    ui->windowModeComboBox->setCurrentIndex(0);
    ui->trayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);

    // Interface settings
    ui->popupOpacitySlider->setValue(80);
    ui->trayIconComboBox->setCurrentIndex(0);
    ui->languagesStyleComboBox->setCurrentIndex(0);
    ui->controlsStyleComboBox->setCurrentIndex(0);

    // Translation settings
    ui->sourceTransliterationCheckBox->setChecked(true);
    ui->translationTransliterationCheckBox->setChecked(true);
    ui->translationOptionsCheckBox->setChecked(true);
    ui->primaryLanguageComboBox->setCurrentIndex(0);
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->languageComboBox->findData("en"));

    // Connection settings
     ui->proxyTypeComboBox->setCurrentIndex(0);
     ui->proxyHostEdit->setText("");
     ui->proxyPortSpinbox->setValue(8080);
     ui->proxyAuthCheckBox->setChecked(false);
     ui->proxyUsernameEdit->setText("");
     ui->proxyPasswordEdit->setText("");

     // Shortcuts
     on_resetAllShortcutsButton_clicked();
}

void SettingsDialog::on_proxyTypeComboBox_currentIndexChanged(int index)
{
    if (index >= 2) {
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

void SettingsDialog::on_shortcutsTreeWidget_itemSelectionChanged()
{
    if (ui->shortcutsTreeWidget->currentItem()->data(1, Qt::UserRole).toString() != "") {
        ui->shortcutGroupBox->setEnabled(true);
        ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    }
    else {
        ui->shortcutGroupBox->setEnabled(false);
        ui->shortcutSequenceEdit->clear();
    }
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_shortcutSequenceEdit_editingFinished()
{
    if (ui->shortcutsTreeWidget->currentItem()->text(1) != ui->shortcutSequenceEdit->keySequence().toString())
        ui->acceptShortcutButton->setEnabled(true);
    else
        ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_acceptShortcutButton_clicked()
{
    ui->shortcutsTreeWidget->currentItem()->setText(1, ui->shortcutSequenceEdit->keySequence().toString());
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_resetShortcutButton_clicked()
{
    ui->shortcutsTreeWidget->currentItem()->setText(1, ui->shortcutsTreeWidget->currentItem()->data(1, Qt::UserRole).toString());
    ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_resetAllShortcutsButton_clicked()
{
    // Global shortcuts
    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setText(1, "Ctrl+Alt+E");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setText(1, "Ctrl+Alt+S");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setText(1, "Ctrl+Alt+C");

    // Window shortcuts
    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setText(1, "Ctrl+Return");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setText(1, "Ctrl+Q");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setText(1, "Ctrl+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setText(1, "Ctrl+D");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setText(1, "Ctrl+Shift+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setText(1, "Ctrl+Shift+D");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setText(1, "Ctrl+Shift+C");
}
