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

#include <QNetworkProxy>
#include <QFileDialog>
#include <QScreen>

#if defined(Q_OS_WIN)
#include "singleapplication.h"
#include "updaterwindow.h"
#endif

#include "appsettings.h"
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

    ui->languageComboBox->setItemData(0, QLocale::AnyLanguage);
    ui->languageComboBox->setItemData(1, QLocale::English);
    ui->languageComboBox->setItemData(2, QLocale::Russian);

    ui->primaryLanguageComboBox->addItem(tr("<System language>"), QOnlineTranslator::Auto);
    ui->secondaryLanguageComboBox->addItem(tr("<System language>"), QOnlineTranslator::Auto);
    foreach (const auto language, languagesMenu->actions()) {
        ui->primaryLanguageComboBox->addItem(language->icon(), language->text(), language->data());
        ui->secondaryLanguageComboBox->addItem(language->icon(), language->text(), language->data());
    }

    // Set maximum and minimum values for the size of the popup window
    ui->popupWidthSlider->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().width());
    ui->popupWidthSpinBox->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().width());
    ui->popupHeightSlider->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().height());
    ui->popupHeightSpinBox->setMaximum(QGuiApplication::primaryScreen()->availableGeometry().height());
    ui->popupWidthSlider->setMinimum(200);
    ui->popupWidthSpinBox->setMinimum(200);
    ui->popupHeightSlider->setMinimum(200);
    ui->popupHeightSpinBox->setMinimum(200);

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

    // Pages selection mechanism
    connect(ui->pagesListWidget, &QListWidget::currentRowChanged, ui->pagesStackedWidget, &QStackedWidget::setCurrentIndex);

#if defined(Q_OS_WIN)
    // Add information about icons
    papirusTitleLabel = new QLabel(tr("Interface icons:"), this);

    papirusLabel = new QLabel("<a href=\"https://github.com/PapirusDevelopmentTeam/papirus-icon-theme\">Papirus</a>", this);
    papirusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    papirusLabel->setOpenExternalLinks(true);

    ui->aboutBox->layout()->addWidget(papirusTitleLabel);
    ui->aboutBox->layout()->addWidget(papirusLabel);

    // Add updater options
    checkForUpdatesLabel = new QLabel(tr("Check for updates:"), this);

    checkForUpdatesComboBox = new QComboBox(this);
    checkForUpdatesComboBox->addItem(tr("Every day"), AppSettings::Day);
    checkForUpdatesComboBox->addItem(tr("Every week"), AppSettings::Week);
    checkForUpdatesComboBox->addItem(tr("Every month"), AppSettings::Month);
    checkForUpdatesComboBox->addItem(tr("Never"), AppSettings::Never);

    checkForUpdatesButton = new QPushButton(tr("Check now"), this);
    checkForUpdatesButton->setToolTip(tr("Check for updates now"));
    connect(checkForUpdatesButton, &QPushButton::clicked, this, &SettingsDialog::checkForUpdates);

    checkForUpdatesStatusLabel = new QLabel(this);

    ui->checkForUpdatesLayout->addWidget(checkForUpdatesLabel);
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesComboBox);
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesButton);
    ui->checkForUpdatesLayout->addWidget(checkForUpdatesStatusLabel);
    ui->checkForUpdatesLayout->addStretch();
#endif

    // General settings
    AppSettings settings;
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData(settings.locale()));
    ui->windowModeComboBox->setCurrentIndex(settings.windowMode());
#if defined(Q_OS_WIN)
    checkForUpdatesComboBox->setCurrentIndex(checkForUpdatesComboBox->findData(settings.checkForUpdatesInterval()));
#endif
    ui->trayCheckBox->setChecked(settings.isTrayIconVisible());
    ui->startMinimizedCheckBox->setChecked(settings.isStartMinimized());
    ui->autostartCheckBox->setChecked(settings.isAutostartEnabled());

    // Interface settings
    ui->popupOpacitySlider->setValue(static_cast<int>(settings.popupOpacity() * 100));
    ui->popupWidthSpinBox->setValue(settings.popupWidth());
    ui->popupHeightSpinBox->setValue(settings.popupHeight());
    ui->popupLanguagesComboBox->setCurrentIndex(settings.popupLanguagesStyle());
    ui->popupControlsComboBox->setCurrentIndex(settings.popupControlsStyle());

    ui->windowLanguagesComboBox->setCurrentIndex(settings.windowLanguagesStyle());
    ui->windowControlsComboBox->setCurrentIndex(settings.windowControlsStyle());

    ui->trayIconComboBox->setCurrentIndex(ui->trayIconComboBox->findData(settings.trayIconName()));
    ui->customTrayIconLineEdit->setText(settings.customIconPath());

    // Translation settings
    ui->sourceTransliterationCheckBox->setChecked(settings.showSourceTranslit());
    ui->translationTransliterationCheckBox->setChecked(settings.showTranslationTranslit());
    ui->translationOptionsCheckBox->setChecked(settings.showTranslationOptions());
    ui->definitionsCheckBox->setChecked(settings.showDefinitions());
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(settings.primaryLanguage()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(settings.secondaryLanguage()));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(settings.proxyType());

    ui->proxyHostEdit->setText(settings.proxyHost());
    ui->proxyPortSpinbox->setValue(settings.proxyPort());
    ui->proxyAuthCheckBox->setChecked(settings.isProxyAuthEnabled());
    ui->proxyUsernameEdit->setText(settings.proxyUsername());
    ui->proxyPasswordEdit->setText(settings.proxyPassword());

    // Global shortcuts
    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setText(1, settings.translateSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setText(1, settings.playSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setText(1, settings.playTranslatedSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->setText(1, settings.stopSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->setText(1, settings.showMainWindowHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(5)->setText(1, settings.copyTranslatedSelectionHotkey());

    // Window shortcuts
    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setText(1, settings.translateHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setText(1, settings.closeWindowHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setText(1, settings.playSourceHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setText(1, settings.stopSourceHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setText(1, settings.playTranslationHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setText(1, settings.stopTranslationHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setText(1, settings.copyTranslationHotkey());

    // Save default shortcuts
    ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->setData(1, Qt::UserRole, settings.defaultTranslateSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->setData(1, Qt::UserRole, settings.defaultPlaySelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->setData(1, Qt::UserRole, settings.defaultPlayTranslatedSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->setData(1, Qt::UserRole, settings.defaultStopSelectionHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->setData(1, Qt::UserRole, settings.defaultShowMainWindowHotkey());
    ui->shortcutsTreeWidget->topLevelItem(0)->child(5)->setData(1, Qt::UserRole, settings.defaultCopyTranslatedSelectionHotkey());

    ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->setData(1, Qt::UserRole, settings.defaultTranslateHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->setData(1, Qt::UserRole, settings.defaultCloseWindowHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->setData(1, Qt::UserRole, settings.defaultPlaySourceHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->setData(1, Qt::UserRole, settings.defaultStopSourceHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->setData(1, Qt::UserRole, settings.defaultPlayTranslationHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->setData(1, Qt::UserRole, settings.defaultStopTranslationHotkey());
    ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->setData(1, Qt::UserRole, settings.defaultCopyTranslationHotkey());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_dialogBox_accepted()
{
    // General settings
    AppSettings settings;
    settings.setWindowMode(static_cast<AppSettings::WindowMode>(ui->windowModeComboBox->currentIndex()));
    settings.setLocale(ui->languageComboBox->currentData().value<QLocale::Language>());
    settings.setTrayIconVisible(ui->trayCheckBox->isChecked());
    settings.setStartMinimized(ui->startMinimizedCheckBox->isChecked());
    settings.setAutostartEnabled(ui->autostartCheckBox->isChecked());
#if defined(Q_OS_WIN)
    settings.setCheckForUpdatesInterval(checkForUpdatesComboBox->currentData().value<AppSettings::Interval>());
#endif

    // Interface settings
    settings.setPopupOpacity(static_cast<double>(ui->popupOpacitySlider->value()) / 100);
    settings.setPopupWidth(ui->popupWidthSpinBox->value());
    settings.setPopupHeight(ui->popupHeightSpinBox->value());
    settings.setPopupLanguagesStyle(static_cast<Qt::ToolButtonStyle>(ui->popupLanguagesComboBox->currentIndex()));
    settings.setPopupControlsStyle(static_cast<Qt::ToolButtonStyle>(ui->popupControlsComboBox->currentIndex()));

    settings.setWindowLanguagesStyle(static_cast<Qt::ToolButtonStyle>(ui->windowLanguagesComboBox->currentIndex()));
    settings.setWindowControlsStyle(static_cast<Qt::ToolButtonStyle>(ui->windowControlsComboBox->currentIndex()));

    settings.setTrayIconName(ui->trayIconComboBox->currentData().toString());
    settings.setCustomIconPath(ui->customTrayIconLineEdit->text());

    // Translation settings
    settings.setShowSourceTranslit(ui->sourceTransliterationCheckBox->isChecked());
    settings.setShowTranslationTranslit(ui->translationTransliterationCheckBox->isChecked());
    settings.setShowTranslationOptions(ui->translationOptionsCheckBox->isChecked());
    settings.setShowDefinitions(ui->definitionsCheckBox->isChecked());
    settings.setPrimaryLanguage(ui->primaryLanguageComboBox->currentData().value<QOnlineTranslator::Language>());
    settings.setSecondaryLanguage(ui->secondaryLanguageComboBox->currentData().value<QOnlineTranslator::Language>());

    // Connection settings
    settings.setProxyType(static_cast<QNetworkProxy::ProxyType>(ui->proxyTypeComboBox->currentIndex()));
    settings.setProxyHost(ui->proxyHostEdit->text());
    settings.setProxyPort(static_cast<quint16>(ui->proxyPortSpinbox->value()));
    settings.setProxyAuthEnabled(ui->proxyAuthCheckBox->isChecked());
    settings.setProxyUsername(ui->proxyUsernameEdit->text());
    settings.setProxyPassword(ui->proxyPasswordEdit->text());

    // Global shortcuts
    settings.setTranslateSelectionHotkey(ui->shortcutsTreeWidget->topLevelItem(0)->child(0)->text(1));
    settings.setPlaySelectionHotkey(ui->shortcutsTreeWidget->topLevelItem(0)->child(1)->text(1));
    settings.setPlayTranslatedSelectionHotkey(ui->shortcutsTreeWidget->topLevelItem(0)->child(2)->text(1));
    settings.setStopSelectionHotkey(ui->shortcutsTreeWidget->topLevelItem(0)->child(3)->text(1));
    settings.setShowMainWindowHotkey(ui->shortcutsTreeWidget->topLevelItem(0)->child(4)->text(1));
    settings.setCopyTranslatedSelectionHotkeyHotkey(ui->shortcutsTreeWidget->topLevelItem(0)->child(5)->text(1));

    // Window shortcuts
    settings.setTranslateHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(0)->text(1));
    settings.setCloseWindowHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(1)->text(1));
    settings.setPlaySourceHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(0)->text(1));
    settings.setStopSourceHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(2)->child(1)->text(1));
    settings.setPlayTranslationHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(0)->text(1));
    settings.setStopTranslationHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(1)->text(1));
    settings.setCopyTranslationHotkey(ui->shortcutsTreeWidget->topLevelItem(1)->child(3)->child(2)->text(1));
}

void SettingsDialog::on_resetSettingsButton_clicked()
{
    // General settings
    ui->languageComboBox->setCurrentIndex(0);
    ui->windowModeComboBox->setCurrentIndex(AppSettings::PopupWindow);
    ui->trayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);
#if defined(Q_OS_WIN)
    checkForUpdatesComboBox->setCurrentIndex(checkForUpdatesComboBox->findData(AppSettings::Month));
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
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(QOnlineTranslator::Auto));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(QOnlineTranslator::English));

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

// Disable (enable) "Custom icon path" option
void SettingsDialog::on_trayIconComboBox_currentIndexChanged(int index)
{
    if (index == 3) {
        ui->customTrayIconLabel->setEnabled(true);
        ui->customTrayIconLineEdit->setEnabled(true);
        ui->customTrayIconButton->setEnabled(true);
    } else {
        ui->customTrayIconLabel->setEnabled(false);
        ui->customTrayIconLineEdit->setEnabled(false);
        ui->customTrayIconButton->setEnabled(false);
    }
}

void SettingsDialog::on_customTrayIconButton_clicked()
{
    QString path = ui->customTrayIconLineEdit->text().left(ui->customTrayIconLineEdit->text().lastIndexOf("/"));
    QString file = QFileDialog::getOpenFileName(this, tr("Select icon"), path, tr("Images (*.png *.ico *.svg *.jpg);;All files()"));
    if (!file.isEmpty())
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
    } else {
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
    if (ui->shortcutsTreeWidget->currentItem()->childCount() == 0) {
        ui->shortcutGroupBox->setEnabled(true);
        ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    } else {
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

void SettingsDialog::on_clearShortcutButton_clicked()
{
    ui->shortcutSequenceEdit->clear();
    ui->acceptShortcutButton->setEnabled(true);
}

void SettingsDialog::on_resetShortcutButton_clicked()
{
    ui->shortcutsTreeWidget->currentItem()->setText(1, ui->shortcutsTreeWidget->currentItem()->data(1, Qt::UserRole).toString());
    ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_resetAllShortcutsButton_clicked()
{
    QTreeWidgetItemIterator it(ui->shortcutsTreeWidget, QTreeWidgetItemIterator::NoChildren);
    while (*it) {
        (*it)->setText(1, (*it)->data(1, Qt::UserRole).toString());
        ++it;
    }
}

#if defined(Q_OS_WIN)
void SettingsDialog::checkForUpdates()
{
    checkForUpdatesButton->setEnabled(false);
    checkForUpdatesStatusLabel->setText(tr("Checking for updates..."));

    // Get update information
    auto release = new QGitTag(this);
    QEventLoop loop;
    connect(release, &QGitTag::requestFinished, &loop, &QEventLoop::quit);
    release->get("Shatur95", "crow-translate");
    loop.exec();

    if (release->error()) {
        checkForUpdatesStatusLabel->setText("<font color=\"red\">" + release->body() + "</font>");
        delete release;
    } else {
        const int installer = release->assetId(".exe");
        if (SingleApplication::applicationVersion() < release->tagName() && installer != -1) {
            checkForUpdatesStatusLabel->setText("<font color=\"green\">" + tr("Update available!") + "</font>");
            auto updaterWindow = new UpdaterWindow(release, installer, this);
            updaterWindow->show();
        } else {
            checkForUpdatesStatusLabel->setText("<font color=\"grey\">" + tr("No updates available.") + "</font>");
            delete release;
        }

        AppSettings settings;
        settings.setLastUpdateCheckDate(QDate::currentDate());
    }

    checkForUpdatesButton->setEnabled(true);
}
#endif
