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

#include <QSettings>
#include <QStandardPaths>
#include <QNetworkProxy>
#include <QFileDialog>
#include <QScreen>

#if defined(Q_OS_WIN)
#include "updaterwindow.h"
#endif

#include "qonlinetranslator.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QMenu *languagesMenu, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->shortcutsTreeWidget->expandAll();
    ui->shortcutsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->logoLabel->setPixmap(QIcon::fromTheme("crow-translate").pixmap(512, 512));
    ui->versionLabel->setText(qApp->applicationVersion());

    // Set item data in comboboxes
    ui->trayIconComboBox->setItemData(0, "crow-translate-tray");
    ui->trayIconComboBox->setItemData(1, "crow-translate-tray-light");
    ui->trayIconComboBox->setItemData(2, "crow-translate-tray-dark");
    ui->trayIconComboBox->setItemData(3, "custom");

    ui->languageComboBox->setItemData(0, "auto");
    ui->languageComboBox->setItemData(1, "en");
    ui->languageComboBox->setItemData(2, "ru");

    ui->primaryLanguageComboBox->addItem(tr("<System language>"), "auto");
    ui->secondaryLanguageComboBox->addItem(tr("<System language>"), "auto");
    foreach (auto language, languagesMenu->actions()) {
        ui->primaryLanguageComboBox->addItem(language->icon(), language->text(), language->toolTip());
        ui->secondaryLanguageComboBox->addItem(language->icon(), language->text(), language->toolTip());
    }

    // Disable (enable) opacity slider if "Window mode" ("Popup mode") selected
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacityLabel, &QSlider::setDisabled);
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacitySlider, &QSlider::setDisabled);

    // Connect sliders to their spin boxes
    connect(ui->popupOpacitySlider, &QSlider::valueChanged, ui->popupOpacitySpinBox, &QSpinBox::setValue);
    connect(ui->popupOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged), ui->popupOpacitySlider, &QSlider::setValue);
    connect(ui->popupWidthSlider, &QSlider::valueChanged, ui->popupWidthSpinBox, &QSpinBox::setValue);
    connect(ui->popupWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged), ui->popupWidthSlider, &QSlider::setValue);
    connect(ui->popupHeightSlider, &QSlider::valueChanged, ui->popupHeightSpinBox, &QSpinBox::setValue);
    connect(ui->popupHeightSpinBox, qOverload<int>(&QSpinBox::valueChanged), ui->popupHeightSlider, &QSlider::setValue);

    // Set maximum and minimum values for the size of the popup window
    ui->popupWidthSlider->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().width());
    ui->popupWidthSpinBox->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().width());
    ui->popupHeightSlider->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().height());
    ui->popupHeightSpinBox->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().height());
    ui->popupWidthSlider->setMinimum(200);
    ui->popupWidthSpinBox->setMinimum(200);
    ui->popupHeightSlider->setMinimum(200);
    ui->popupHeightSpinBox->setMinimum(200);

    // Pages selection mechanism
    connect(ui->pagesListWidget, &QListWidget::currentRowChanged, ui->pagesStackedWidget, &QStackedWidget::setCurrentIndex);

#if defined(Q_OS_WIN)
    // Add information about icons
    QLabel *papirusTitleLabel = new QLabel(tr("Interface icons:"), this);
    QLabel *papirusLabel = new QLabel("<a href=\"https://github.com/PapirusDevelopmentTeam/papirus-icon-theme\">Papirus</a>", this);
    papirusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    papirusLabel->setOpenExternalLinks(true);
    ui->aboutBox->layout()->addWidget(papirusTitleLabel);
    ui->aboutBox->layout()->addWidget(papirusLabel);

    // Add updater options
    checkForUpdatesLabel = new QLabel(tr("Check for updates:"), this);
    checkForUpdatesComboBox = new QComboBox(this);
    checkForUpdatesButton = new QPushButton(tr("Check now"), this);
    checkForUpdatesButton->setToolTip(tr("Check for updates now"));
    checkForUpdatesStatusLabel = new QLabel(this);
    checkForUpdatesStatusLabel->setWordWrap(true);
    checkForUpdatesComboBox->addItem(tr("Every day"));
    checkForUpdatesComboBox->addItem(tr("Every week"));
    checkForUpdatesComboBox->addItem(tr("Every month"));
    checkForUpdatesComboBox->addItem(tr("Never"));
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesLabel);
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesComboBox);
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesButton);
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesStatusLabel);
    ui->checkForUpdatesLayout->addStretch();
    connect(checkForUpdatesButton, &QPushButton::clicked, this, &SettingsDialog::checkForUpdates);
#endif

    // General settings
    QSettings settings;
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData(settings.value("Language", "auto").toString()));
    ui->windowModeComboBox->setCurrentIndex(settings.value("WindowMode", 0).toInt());
    ui->trayCheckBox->setChecked(settings.value("TrayIconVisible", true).toBool());
    ui->startMinimizedCheckBox->setChecked(settings.value("StartMinimized", false).toBool());
#if defined(Q_OS_LINUX)
    ui->autostartCheckBox->setChecked(QFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/crow-translate.desktop").exists());
#elif defined(Q_OS_WIN)
    QSettings autostartSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    ui->autostartCheckBox->setChecked(autostartSettings.contains("Crow Translate"));
    checkForUpdatesComboBox->setCurrentIndex(settings.value("CheckForUpdatesInterval", 1).toInt());
#endif

    // Interface settings
    ui->popupOpacitySlider->setValue(settings.value("PopupOpacity", 0.8).toDouble() * 100);
    ui->popupHeightSpinBox->setValue(settings.value("PopupSize", QSize(350, 300)).toSize().height());
    ui->popupWidthSpinBox->setValue(settings.value("PopupSize", QSize(350, 300)).toSize().width());
    ui->popupLanguagesComboBox->setCurrentIndex(settings.value("PopupLanguagesStyle", Qt::ToolButtonIconOnly).toInt());
    ui->popupControlsComboBox->setCurrentIndex(settings.value("PopupControlsStyle", Qt::ToolButtonIconOnly).toInt());

    ui->windowLanguagesComboBox->setCurrentIndex(settings.value("WindowLanguagesStyle", Qt::ToolButtonTextBesideIcon).toInt());
    ui->windowControlsComboBox->setCurrentIndex(settings.value("WindowControlsStyle", Qt::ToolButtonIconOnly).toInt());

    ui->trayIconComboBox->setCurrentIndex(ui->trayIconComboBox->findData(settings.value("TrayIcon", "crow-translate-tray").toString()));
    ui->customTrayIconLineEdit->setText(settings.value("CustomIconPath", "").toString());

    // Translation settings
    ui->sourceTransliterationCheckBox->setChecked(settings.value("Translation/ShowSourceTransliteration", true).toBool());
    ui->translationTransliterationCheckBox->setChecked(settings.value("Translation/ShowTranslationTransliteration", true).toBool());
    ui->translationOptionsCheckBox->setChecked(settings.value("Translation/TranslationOptions", true).toBool());
    ui->definitionsCheckBox->setChecked(settings.value("Translation/Definitions", true).toBool());
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(settings.value("Translation/PrimaryLanguage", "auto").toString()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(settings.value("Translation/SecondaryLanguage", "en").toString()));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(settings.value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt());

    ui->proxyHostEdit->setText(settings.value("Connection/ProxyHost", "").toString());
    ui->proxyPortSpinbox->setValue(settings.value("Connection/ProxyPort", 8080).toInt());
    ui->proxyAuthCheckBox->setChecked(settings.value("Connection/ProxyAuthEnabled", false).toBool());
    ui->proxyUsernameEdit->setText(settings.value("Connection/ProxyUsername", "").toString());
    ui->proxyPasswordEdit->setText(settings.value("Connection/ProxyPassword", "").toString());

    // Global shortcuts
    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setText(1, settings.value("Hotkeys/TranslateSelection", "Ctrl+Alt+E").toString());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setText(1, settings.value("Hotkeys/PlaySelection", "Ctrl+Alt+S").toString());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setText(1, settings.value("Hotkeys/PlayTranslatedSelection", "Ctrl+Alt+F").toString());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->setText(1, settings.value("Hotkeys/StopSelection", "Ctrl+Alt+G").toString());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->setText(1, settings.value("Hotkeys/ShowMainWindow", "Ctrl+Alt+C").toString());

    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setData(1, Qt::UserRole, "Ctrl+Alt+E");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setData(1, Qt::UserRole, "Ctrl+Alt+S");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setData(1, Qt::UserRole, "Ctrl+Alt+F");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->setData(1, Qt::UserRole, "Ctrl+Alt+G");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->setData(1, Qt::UserRole, "Ctrl+Alt+C");

    // Window shortcuts
    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setText(1, settings.value("Hotkeys/Translate", "Ctrl+Return").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setText(1, settings.value("Hotkeys/CloseWindow", "Ctrl+Q").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setText(1, settings.value("Hotkeys/PlaySource", "Ctrl+S").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setText(1, settings.value("Hotkeys/StopSource", "Ctrl+G").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setText(1, settings.value("Hotkeys/PlayTranslation", "Ctrl+Shift+S").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setText(1, settings.value("Hotkeys/StopTranslation", "Ctrl+Shift+G").toString());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setText(1, settings.value("Hotkeys/CopyTranslation", "Ctrl+Shift+C").toString());

    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setData(1, Qt::UserRole, "Ctrl+Return");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setData(1, Qt::UserRole, "Ctrl+Q");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setData(1, Qt::UserRole, "Ctrl+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setData(1, Qt::UserRole, "Ctrl+G");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setData(1, Qt::UserRole, "Ctrl+Shift+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setData(1, Qt::UserRole, "Ctrl+Shift+G");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setData(1, Qt::UserRole, "Ctrl+Shift+C");
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_dialogBox_accepted()
{
    // Check if language changed
    QSettings settings;
    if (settings.value("Language", "auto").toString() != ui->languageComboBox->currentData()) {
        settings.setValue("Language", ui->languageComboBox->currentData());
        emit languageChanged(); // Emit signal if language changed
    }

    // Check if proxy changed
    if (ui->proxyTypeComboBox->currentIndex() != settings.value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt() ||
            ui->proxyHostEdit->text() != settings.value("Connection/ProxyHost", "").toString() ||
            ui->proxyPortSpinbox->value() != settings.value("Connection/ProxyPort", 8080).toInt() ||
            ui->proxyAuthCheckBox->isChecked() != settings.value("Connection/ProxyAuthEnabled", false).toInt() ||
            ui->proxyUsernameEdit->text() != settings.value("Connection/ProxyUsername", "").toString() ||
            ui->proxyPasswordEdit->text() != settings.value("Connection/ProxyPassword", "").toString()) {
        settings.setValue("Connection/ProxyType", ui->proxyTypeComboBox->currentIndex());
        settings.setValue("Connection/ProxyHost", ui->proxyHostEdit->text());
        settings.setValue("Connection/ProxyPort", ui->proxyPortSpinbox->value());
        settings.setValue("Connection/ProxyAuthEnabled", ui->proxyAuthCheckBox->isChecked());
        settings.setValue("Connection/ProxyUsername", ui->proxyUsernameEdit->text());
        settings.setValue("Connection/ProxyPassword", ui->proxyPasswordEdit->text());
        emit proxyChanged(); // Emit signal if language changed
    }

    // Check if autostart options changed
#if defined(Q_OS_LINUX)
    QFile autorunFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/crow-translate.desktop");
    if(ui->autostartCheckBox->isChecked()) {
        // Create autorun file if checked
        if(!autorunFile.exists()) {
            if(autorunFile.open(QFile::WriteOnly)){
                QString autorunContent("[Desktop Entry]\n"
                                       "Type=Application\n"
                                       "Exec=crow\n"
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
    QSettings autostartSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (ui->autostartCheckBox->isChecked())
        autostartSettings.setValue("Crow Translate", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    else
        autostartSettings.remove("Crow Translate");
#endif

    // Other general settings
    settings.setValue("WindowMode", ui->windowModeComboBox->currentIndex());
    settings.setValue("TrayIconVisible", ui->trayCheckBox->isChecked());
    settings.setValue("StartMinimized", ui->startMinimizedCheckBox->isChecked());
#if defined(Q_OS_WIN)
    settings.setValue("CheckForUpdatesInterval", checkForUpdatesComboBox->currentIndex());
#endif

    // Interface settings
    settings.setValue("PopupOpacity", static_cast<double>(ui->popupOpacitySlider->value()) / 100);
    settings.setValue("PopupSize", QSize(ui->popupWidthSpinBox->value(), ui->popupHeightSpinBox->value()));
    settings.setValue("PopupLanguagesStyle", ui->popupLanguagesComboBox->currentIndex());
    settings.setValue("PopupControlsStyle", ui->popupControlsComboBox->currentIndex());

    settings.setValue("WindowLanguagesStyle", ui->windowLanguagesComboBox->currentIndex());
    settings.setValue("WindowControlsStyle", ui->windowControlsComboBox->currentIndex());

    settings.setValue("TrayIcon", ui->trayIconComboBox->currentData());
    settings.setValue("CustomIconPath", ui->customTrayIconLineEdit->text());

    // Translation settings
    settings.setValue("Translation/ShowSourceTransliteration", ui->sourceTransliterationCheckBox->isChecked());
    settings.setValue("Translation/ShowTranslationTransliteration", ui->translationTransliterationCheckBox->isChecked());
    settings.setValue("Translation/TranslationOptions", ui->translationOptionsCheckBox->isChecked());
    settings.setValue("Translation/Definitions", ui->definitionsCheckBox->isChecked());
    settings.setValue("Translation/PrimaryLanguage", ui->primaryLanguageComboBox->currentData());
    settings.setValue("Translation/SecondaryLanguage", ui->secondaryLanguageComboBox->currentData());

    // Connection settings
    settings.setValue("Connection/ProxyType", ui->proxyTypeComboBox->currentIndex());
    settings.setValue("Connection/ProxyHost", ui->proxyHostEdit->text());
    settings.setValue("Connection/ProxyPort", ui->proxyPortSpinbox->value());
    settings.setValue("Connection/ProxyAuthEnabled", ui->proxyAuthCheckBox->isChecked());
    settings.setValue("Connection/ProxyUsername", ui->proxyUsernameEdit->text());
    settings.setValue("Connection/ProxyPassword", ui->proxyPasswordEdit->text());

    // Global shortcuts
    settings.setValue("Hotkeys/TranslateSelection", ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->text(1));
    settings.setValue("Hotkeys/PlaySelection", ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->text(1));
    settings.setValue("Hotkeys/PlayTranslatedSelection", ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->text(1));
    settings.setValue("Hotkeys/StopSelection", ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->text(1));
    settings.setValue("Hotkeys/ShowMainWindow", ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->text(1));

    // Window shortcuts
    settings.setValue("Hotkeys/Translate", ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->text(1));
    settings.setValue("Hotkeys/CloseWindow", ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->text(1));
    settings.setValue("Hotkeys/PlaySource", ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->text(1));
    settings.setValue("Hotkeys/StopSource", ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->text(1));
    settings.setValue("Hotkeys/PlayTranslation", ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->text(1));
    settings.setValue("Hotkeys/StopTranslation", ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->text(1));
    settings.setValue("Hotkeys/CopyTranslation", ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->text(1));
}

void SettingsDialog::on_resetSettingsButton_clicked()
{
    // General settings
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData("auto"));
    ui->windowModeComboBox->setCurrentIndex(0);
    ui->trayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);
#if defined(Q_OS_WIN)
    checkForUpdatesComboBox->setCurrentIndex(1);
#endif

    // Interface settings
    ui->popupOpacitySlider->setValue(80);
    ui->popupWidthSpinBox->setValue(350);
    ui->popupHeightSpinBox->setValue(300);
    ui->popupLanguagesComboBox->setCurrentIndex(0);
    ui->popupControlsComboBox->setCurrentIndex(0);

    ui->windowLanguagesComboBox->setCurrentIndex(2);
    ui->windowControlsComboBox->setCurrentIndex(0);

    ui->trayIconComboBox->setCurrentIndex(0);
    ui->customTrayIconLineEdit->setText("");

    // Translation settings
    ui->sourceTransliterationCheckBox->setChecked(true);
    ui->translationTransliterationCheckBox->setChecked(true);
    ui->translationOptionsCheckBox->setChecked(true);
    ui->definitionsCheckBox->setChecked(true);
    ui->primaryLanguageComboBox->setCurrentIndex(0);
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->languageComboBox->findData("en"));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(1);
    ui->proxyHostEdit->setText("");
    ui->proxyPortSpinbox->setValue(8080);
    ui->proxyAuthCheckBox->setChecked(false);
    ui->proxyUsernameEdit->setText("");
    ui->proxyPasswordEdit->setText("");

    // Shortcuts
    on_resetAllShortcutsButton_clicked();
}

// Disable (enable) "Start minimized" option when tray mode is disabled (enabled)
void SettingsDialog::on_trayCheckBox_toggled(bool checked)
{
    ui->startMinimizedCheckBox->setEnabled(checked);
    ui->startMinimizedCheckBox->setChecked(false);
}

void SettingsDialog::on_trayIconComboBox_currentIndexChanged(int index)
{
    if (index == 3) {
        ui->customTrayIconLabel->setEnabled(true);
        ui->customTrayIconLineEdit->setEnabled(true);
        ui->customTrayIconButton->setEnabled(true);
    }
    else {
        ui->customTrayIconLabel->setEnabled(false);
        ui->customTrayIconLineEdit->setEnabled(false);
        ui->customTrayIconButton->setEnabled(false);
    }
}

void SettingsDialog::on_customTrayIconButton_clicked()
{
    QString path = ui->customTrayIconLineEdit->text().left(ui->customTrayIconLineEdit->text().lastIndexOf("/"));
    QString file = QFileDialog::getOpenFileName(this, tr("Select icon"), path, tr("Images (*.png *.ico *.svg *.jpg);;All files()"));
    if (file != "")
        ui->customTrayIconLineEdit->setText(file);
}

void SettingsDialog::on_proxyTypeComboBox_currentIndexChanged(int index)
{
    if (index == QNetworkProxy::HttpProxy || index == QNetworkProxy::Socks5Proxy) {
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
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setText(1, "Ctrl+Alt+F");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->setText(1, "Ctrl+Alt+G");
    ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->setText(1, "Ctrl+Alt+C");

    // Window shortcuts
    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setText(1, "Ctrl+Return");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setText(1, "Ctrl+Q");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setText(1, "Ctrl+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setText(1, "Ctrl+D");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setText(1, "Ctrl+Shift+S");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setText(1, "Ctrl+Shift+D");
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setText(1, "Ctrl+Shift+C");
}

#if defined(Q_OS_WIN)
void SettingsDialog::checkForUpdates()
{
    checkForUpdatesButton->setEnabled(false);
    checkForUpdatesStatusLabel->setText(tr("Checking for updates..."));
    QGitRelease release("Shatur95", "Crow-Translate");

    if (release.error())
        checkForUpdatesStatusLabel->setText("<font color=\"red\">" + release.body() + "</font>");
    else {
        if (qApp->applicationVersion() > release.tagName()) {
            checkForUpdatesStatusLabel->setText("<font color=\"green\">" + tr("Update available!") + "</font>");
            UpdaterWindow *updaterWindow = new UpdaterWindow(release, this);
            updaterWindow->show();
        }
        else
            checkForUpdatesStatusLabel->setText("<font color=\"grey\">" + tr("No updates available.") + "</font>");
    }

    checkForUpdatesButton->setEnabled(true);
}
#endif
