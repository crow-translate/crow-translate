/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
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
#include "ui_settingsdialog.h"
#include "appsettings.h"
#include "singleapplication.h"
#include "shortcutsmodel/shortcutsmodel.h"
#include "shortcutsmodel/shortcutitem.h"
#if defined(Q_OS_WIN)
#include "updaterwindow.h"
#include "qgittag.h"
#endif

#include <QNetworkProxy>
#include <QFileDialog>
#include <QScreen>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDate>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->dialogButtonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);
    ui->logoLabel->setPixmap(QIcon::fromTheme("crow-translate").pixmap(512, 512));
    ui->versionLabel->setText(SingleApplication::applicationVersion());

    // Test voice
    m_player = new QMediaPlayer(this);
    m_playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(m_playlist);

    // Set item data in comboboxes
    ui->languageComboBox->setItemData(0, QLocale::AnyLanguage);
    ui->languageComboBox->setItemData(1, QLocale::English);
    ui->languageComboBox->setItemData(2, QLocale::Portuguese);
    ui->languageComboBox->setItemData(3, QLocale::Russian);
    ui->languageComboBox->setItemData(4, QLocale::Ukrainian);
    ui->languageComboBox->setItemData(5, QLocale::Turkish);
    ui->languageComboBox->setItemData(6, QLocale::Chinese);

    ui->primaryLanguageComboBox->addItem(tr("<System language>"), QOnlineTranslator::Auto);
    ui->secondaryLanguageComboBox->addItem(tr("<System language>"), QOnlineTranslator::Auto);
    for (int i = 1; i <= QOnlineTranslator::Zulu; ++i) {
        const auto lang = static_cast<QOnlineTranslator::Language>(i);
        const QIcon langIcon(":/icons/flags/" + QOnlineTranslator::languageCode(lang) + ".svg");

        ui->primaryLanguageComboBox->addItem(langIcon, QOnlineTranslator::languageString(lang), i);
        ui->secondaryLanguageComboBox->addItem(langIcon, QOnlineTranslator::languageString(lang), i);
    }

    // Sort languages in comboboxes alphabetically
    ui->primaryLanguageComboBox->model()->sort(0);
    ui->secondaryLanguageComboBox->model()->sort(0);

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

#if defined(Q_OS_WIN)
    // Add information about icons
    auto *papirusTitleLabel = new QLabel(this);
    papirusTitleLabel->setText(tr("Interface icons:"));
    ui->aboutGroupBox->layout()->addWidget(papirusTitleLabel);

    auto *papirusLabel = new QLabel(this);
    papirusLabel->setText("<a href=\"https://github.com/PapirusDevelopmentTeam/papirus-icon-theme\">Papirus</a>");
    papirusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    papirusLabel->setOpenExternalLinks(true);
    ui->aboutGroupBox->layout()->addWidget(papirusLabel);

    // Add updater options
    QGroupBox *updatesGroupBox = new QGroupBox(tr("Updates"));
    qobject_cast<QVBoxLayout *>(ui->generalPage->layout())->insertWidget(1, updatesGroupBox);

    QHBoxLayout *updatesLayout = new QHBoxLayout;
    updatesGroupBox->setLayout(updatesLayout);

    auto *checkForUpdatesLabel = new QLabel;
    checkForUpdatesLabel->setText(tr("Check for updates:"));
    updatesLayout->addWidget(checkForUpdatesLabel);

    m_checkForUpdatesComboBox = new QComboBox;
    m_checkForUpdatesComboBox->addItem(tr("Every day"));
    m_checkForUpdatesComboBox->addItem(tr("Every week"));
    m_checkForUpdatesComboBox->addItem(tr("Every month"));
    m_checkForUpdatesComboBox->addItem(tr("Never"));
    updatesLayout->addWidget(m_checkForUpdatesComboBox);

    m_checkForUpdatesButton = new QPushButton;
    m_checkForUpdatesButton->setText(tr("Check now"));
    m_checkForUpdatesButton->setToolTip(tr("Check for updates now"));
    connect(m_checkForUpdatesButton, &QPushButton::clicked, this, &SettingsDialog::checkForUpdates);
    updatesLayout->addWidget(m_checkForUpdatesButton);

    m_checkForUpdatesStatusLabel = new QLabel;
    updatesLayout->addWidget(m_checkForUpdatesStatusLabel);

    updatesLayout->addStretch();
#endif

    // Check current date
    const QDate date = QDate::currentDate();
    if ((date.month() == 12 && date.day() == 31)
            || (date.month() == 1 && date.day() == 1)) {
        ui->testSpeechEdit->setText(tr("Happy New Year!"));
    }

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    // General settings
    AppSettings settings;
    settings.setWindowMode(static_cast<AppSettings::WindowMode>(ui->windowModeComboBox->currentIndex()));
    settings.setLocale(ui->languageComboBox->currentData().value<QLocale::Language>());
    settings.setTrayIconVisible(ui->trayCheckBox->isChecked());
    settings.setStartMinimized(ui->startMinimizedCheckBox->isChecked());
    settings.setAutostartEnabled(ui->autostartCheckBox->isChecked());
#if defined(Q_OS_WIN)
    settings.setCheckForUpdatesInterval(static_cast<AppSettings::Interval>(m_checkForUpdatesComboBox->currentIndex()));
#endif

    // Interface settings
    settings.setPopupOpacity(static_cast<double>(ui->popupOpacitySlider->value()) / 100);
    settings.setPopupWidth(ui->popupWidthSpinBox->value());
    settings.setPopupHeight(ui->popupHeightSpinBox->value());
    settings.setPopupLanguagesStyle(static_cast<Qt::ToolButtonStyle>(ui->popupLanguagesComboBox->currentIndex()));
    settings.setPopupControlsStyle(static_cast<Qt::ToolButtonStyle>(ui->popupControlsComboBox->currentIndex()));
    settings.setWindowLanguagesStyle(static_cast<Qt::ToolButtonStyle>(ui->windowLanguagesComboBox->currentIndex()));
    settings.setWindowControlsStyle(static_cast<Qt::ToolButtonStyle>(ui->windowControlsComboBox->currentIndex()));
    settings.setTrayIconType(static_cast<TrayIcon::IconType>(ui->trayIconComboBox->currentIndex()));
    settings.setCustomIconPath(ui->customTrayIconEdit->text());

    // Translation settings
    settings.setSourceTranslitEnabled(ui->sourceTranslitCheckBox->isChecked());
    settings.setTranslationTranslitEnabled(ui->translationTranslitCheckBox->isChecked());
    settings.setSourceTranscriptionEnabled(ui->sourceTranscriptionCheckBox->isChecked());
    settings.setTranslationOptionsEnabled(ui->translationOptionsCheckBox->isChecked());
    settings.setExamplesEnabled(ui->examplesCheckBox->isChecked());
    settings.setPrimaryLanguage(ui->primaryLanguageComboBox->currentData().value<QOnlineTranslator::Language>());
    settings.setSecondaryLanguage(ui->secondaryLanguageComboBox->currentData().value<QOnlineTranslator::Language>());

    // Speech synthesis settings
    settings.setYandexVoice(yandexVoice);
    settings.setBingVoice(bingVoice);
    settings.setYandexEmotion(yandexEmotion);

    // Connection settings
    settings.setProxyType(static_cast<QNetworkProxy::ProxyType>(ui->proxyTypeComboBox->currentIndex()));
    settings.setProxyHost(ui->proxyHostEdit->text());
    settings.setProxyPort(static_cast<quint16>(ui->proxyPortSpinbox->value()));
    settings.setProxyAuthEnabled(ui->proxyAuthCheckBox->isChecked());
    settings.setProxyUsername(ui->proxyUsernameEdit->text());
    settings.setProxyPassword(ui->proxyPasswordEdit->text());

    // Shortcuts
    ui->shortcutsTreeView->model()->saveShortcuts(settings);
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

// Disable (enable) "Custom icon path" option
void SettingsDialog::on_trayIconComboBox_currentIndexChanged(int index)
{
    if (index == TrayIcon::CustomIcon) {
        ui->customTrayIconLabel->setEnabled(true);
        ui->customTrayIconEdit->setEnabled(true);
        ui->customTrayIconButton->setEnabled(true);
    } else {
        ui->customTrayIconLabel->setEnabled(false);
        ui->customTrayIconEdit->setEnabled(false);
        ui->customTrayIconButton->setEnabled(false);
    }
}

void SettingsDialog::on_customTrayIconButton_clicked()
{
    const QString path = ui->customTrayIconEdit->text().left(ui->customTrayIconEdit->text().lastIndexOf("/"));
    const QString file = QFileDialog::getOpenFileName(this, tr("Select icon"), path, tr("Images (*.png *.ico *.svg *.jpg);;All files()"));
    if (!file.isEmpty())
        ui->customTrayIconEdit->setText(file);
}

void SettingsDialog::on_customTrayIconEdit_textChanged(const QString &iconPath)
{
    ui->customTrayIconButton->setIcon(TrayIcon::customTrayIcon(iconPath));
}

// Disable unsupported voice settings for engines.
void SettingsDialog::on_engineComboBox_currentIndexChanged(int index)
{
    // Avoid index changed signal
    ui->voiceComboBox->blockSignals(true);
    ui->emotionComboBox->blockSignals(true);

    switch (index) {
    case QOnlineTranslator::Google:
        ui->voiceLabel->setEnabled(false);
        ui->voiceComboBox->setEnabled(false);
        ui->emotionLabel->setEnabled(false);
        ui->emotionComboBox->setEnabled(false);

        // Google has no voice settings
        ui->voiceComboBox->clear();
        ui->voiceComboBox->addItem(tr("Default"));

        // Google has no emotion settings
        if (ui->emotionComboBox->count() != 1) {
            ui->emotionComboBox->clear();
            ui->emotionComboBox->addItem(tr("Default"));
        }
        break;
    case QOnlineTranslator::Yandex:
        ui->voiceLabel->setEnabled(true);
        ui->voiceComboBox->setEnabled(true);
        ui->emotionLabel->setEnabled(true);
        ui->emotionComboBox->setEnabled(true);

        // Add Yandex voices
        ui->voiceComboBox->clear();
        ui->voiceComboBox->addItem(tr("Zahar"), QOnlineTts::Zahar);
        ui->voiceComboBox->addItem(tr("Ermil"), QOnlineTts::Ermil);
        ui->voiceComboBox->addItem(tr("Jane"), QOnlineTts::Jane);
        ui->voiceComboBox->addItem(tr("Oksana"), QOnlineTts::Oksana);
        ui->voiceComboBox->addItem(tr("Alyss"), QOnlineTts::Alyss);
        ui->voiceComboBox->addItem(tr("Omazh"), QOnlineTts::Omazh);
        ui->voiceComboBox->setCurrentIndex(ui->voiceComboBox->findData(yandexVoice));

        // Add Yandex emotion options
        ui->emotionComboBox->clear();
        ui->emotionComboBox->addItem(tr("Neutral"), QOnlineTts::Neutral);
        ui->emotionComboBox->addItem(tr("Good"), QOnlineTts::Good);
        ui->emotionComboBox->addItem(tr("Evil"), QOnlineTts::Evil);
        ui->emotionComboBox->setCurrentIndex(ui->emotionComboBox->findData(yandexEmotion));
        break;
    case QOnlineTranslator::Bing:
        ui->voiceLabel->setEnabled(true);
        ui->voiceComboBox->setEnabled(true);
        ui->emotionLabel->setEnabled(false);
        ui->emotionComboBox->setEnabled(false);

        // Add Bing voices
        ui->voiceComboBox->clear();
        ui->voiceComboBox->addItem(tr("Female"), QOnlineTts::Female);
        ui->voiceComboBox->addItem(tr("Male"), QOnlineTts::Male);
        ui->voiceComboBox->setCurrentIndex(ui->voiceComboBox->findData(bingVoice));

        // Bing has no emotion settings
        if (ui->emotionComboBox->count() != 1) {
            ui->emotionComboBox->clear();
            ui->emotionComboBox->addItem(tr("Default"));
        }
        break;
    }

    ui->voiceComboBox->blockSignals(false);
    ui->emotionComboBox->blockSignals(false);
}

// Save current engine voice settings
void SettingsDialog::on_voiceComboBox_currentIndexChanged(int index)
{
    switch (ui->engineComboBox->currentIndex()) {
    case QOnlineTranslator::Yandex:
        yandexVoice = ui->voiceComboBox->itemData(index).value<QOnlineTts::Voice>();
        break;
    case QOnlineTranslator::Bing:
        bingVoice = ui->voiceComboBox->itemData(index).value<QOnlineTts::Voice>();
        break;
    }
}

// Save current engine emotion settings
void SettingsDialog::on_emotionComboBox_currentIndexChanged(int index)
{
    if (ui->engineComboBox->currentIndex() == QOnlineTranslator::Yandex)
        yandexEmotion = ui->emotionComboBox->itemData(index).value<QOnlineTts::Emotion>();
}

// Play test text
void SettingsDialog::on_testSpeechButton_clicked()
{
    // Clear previous playlist (this will stop playback)
    m_playlist->clear();

    if (ui->testSpeechEdit->text().isEmpty()) {
        QMessageBox errorMessage(QMessageBox::Information, tr("Nothing to play"), tr("Playback text is empty"));
        errorMessage.exec();
        return;
    }

    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    QOnlineTts::Voice voice = QOnlineTts::DefaultVoice;
    QOnlineTts::Emotion emotion = QOnlineTts::DefaultEmotion;
    switch (engine) {
    case QOnlineTranslator::Yandex:
        voice = yandexVoice;
        emotion = yandexEmotion;
        break;
    case QOnlineTranslator::Bing:
        voice = bingVoice;
        break;
    default:
        break;
    }

    QOnlineTranslator translator;
    translator.detectLanguage(ui->testSpeechEdit->text(), engine);
    if (translator.error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to detect language"), translator.errorString());
        errorMessage.exec();
        return;
    }

    QOnlineTts tts;
    tts.generateUrls(ui->testSpeechEdit->text(), engine, translator.sourceLanguage(), voice, emotion);
    QList<QMediaContent> media = tts.media();
    if (tts.error()) {
        QMessageBox errorMessage(QMessageBox::Critical, tr("Unable to play text"), tts.errorString());
        errorMessage.exec();
        return;
    }

    m_playlist->addMedia(media);
    m_player->play();
}

void SettingsDialog::on_shortcutsTreeView_currentItemChanged(ShortcutItem *item)
{
    if (item->childCount() == 0) {
        ui->shortcutGroupBox->setEnabled(true);
        ui->shortcutSequenceEdit->setKeySequence(item->shortcut());
    } else {
        ui->shortcutGroupBox->setEnabled(false);
        ui->shortcutSequenceEdit->clear();
    }
}

void SettingsDialog::on_shortcutSequenceEdit_editingFinished()
{
    if (ui->shortcutsTreeView->currentItem()->shortcut() != ui->shortcutSequenceEdit->keySequence())
        ui->acceptShortcutButton->setEnabled(true);
    else
        ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_acceptShortcutButton_clicked()
{
    ui->shortcutsTreeView->currentItem()->setShortcut(ui->shortcutSequenceEdit->keySequence());
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_clearShortcutButton_clicked()
{
    ui->shortcutSequenceEdit->clear();
    ui->acceptShortcutButton->setEnabled(true);
}

void SettingsDialog::on_resetShortcutButton_clicked()
{
    ui->shortcutsTreeView->currentItem()->resetShortcut();
    ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeView->currentItem()->shortcut());
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_resetAllShortcutsButton_clicked()
{
    ui->shortcutsTreeView->model()->resetAllShortcuts();
}

#if defined(Q_OS_WIN)
void SettingsDialog::checkForUpdates()
{
    m_checkForUpdatesButton->setEnabled(false);
    m_checkForUpdatesStatusLabel->setText(tr("Checking for updates..."));

    // Get update information
    auto *release = new QGitTag(this);
    QEventLoop loop;
    connect(release, &QGitTag::finished, &loop, &QEventLoop::quit);
    release->get("Shatur95", "crow-translate");
    loop.exec();

    if (release->error()) {
        m_checkForUpdatesStatusLabel->setText("<font color=\"red\">" + release->body() + "</font>");
        delete release;
    } else {
        const int installer = release->assetId(".exe");
        if (SingleApplication::applicationVersion() < release->tagName() && installer != -1) {
            m_checkForUpdatesStatusLabel->setText("<font color=\"green\">" + tr("Update available!") + "</font>");
            auto *updaterWindow = new UpdaterWindow(release, installer, this);
            updaterWindow->show();
        } else {
            m_checkForUpdatesStatusLabel->setText("<font color=\"grey\">" + tr("No updates available.") + "</font>");
            delete release;
        }

        AppSettings settings;
        settings.setLastUpdateCheckDate(QDate::currentDate());
    }

    m_checkForUpdatesButton->setEnabled(true);
}
#endif


void SettingsDialog::restoreDefaults()
{
    // General settings
    ui->languageComboBox->setCurrentIndex(0);
    ui->windowModeComboBox->setCurrentIndex(AppSettings::PopupWindow);
    ui->trayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);
#if defined(Q_OS_WIN)
    m_checkForUpdatesComboBox->setCurrentIndex(AppSettings::Month);
#endif

    // Interface settings
    ui->popupOpacitySlider->setValue(80);
    ui->popupWidthSpinBox->setValue(350);
    ui->popupHeightSpinBox->setValue(300);
    ui->popupLanguagesComboBox->setCurrentIndex(0);
    ui->popupControlsComboBox->setCurrentIndex(0);
    ui->windowLanguagesComboBox->setCurrentIndex(2);
    ui->windowControlsComboBox->setCurrentIndex(0);
    ui->trayIconComboBox->setCurrentIndex(TrayIcon::DefaultIcon);
    ui->customTrayIconEdit->setText("");

    // Translation settings
    ui->sourceTranslitCheckBox->setChecked(true);
    ui->translationTranslitCheckBox->setChecked(true);
    ui->sourceTranscriptionCheckBox->setChecked(true);
    ui->translationOptionsCheckBox->setChecked(true);
    ui->examplesCheckBox->setChecked(true);
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(QOnlineTranslator::Auto));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(QOnlineTranslator::English));

    // Speech synthesis settings
    yandexVoice = QOnlineTts::Zahar;
    bingVoice = QOnlineTts::Female;
    yandexEmotion = QOnlineTts::Neutral;

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

void SettingsDialog::loadSettings()
{
    // General settings
    const AppSettings settings;
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findData(settings.locale()));
    ui->windowModeComboBox->setCurrentIndex(settings.windowMode());
#if defined(Q_OS_WIN)
    m_checkForUpdatesComboBox->setCurrentIndex(settings.checkForUpdatesInterval());
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

    ui->trayIconComboBox->setCurrentIndex(settings.trayIconType());
    ui->customTrayIconEdit->setText(settings.customIconPath());

    // Translation settings
    ui->sourceTranslitCheckBox->setChecked(settings.isSourceTranslitEnabled());
    ui->translationTranslitCheckBox->setChecked(settings.isTranslationTranslitEnabled());
    ui->sourceTranscriptionCheckBox->setChecked(settings.isSourceTranscriptionEnabled());
    ui->translationOptionsCheckBox->setChecked(settings.isTranslationOptionsEnabled());
    ui->examplesCheckBox->setChecked(settings.isExamplesEnabled());
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(settings.primaryLanguage()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(settings.secondaryLanguage()));

    // Speech synthesis settings
    yandexVoice = settings.yandexVoice();
    bingVoice = settings.bingVoice();
    yandexEmotion = settings.yandexEmotion();

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(settings.proxyType());
    ui->proxyHostEdit->setText(settings.proxyHost());
    ui->proxyPortSpinbox->setValue(settings.proxyPort());
    ui->proxyAuthCheckBox->setChecked(settings.isProxyAuthEnabled());
    ui->proxyUsernameEdit->setText(settings.proxyUsername());
    ui->proxyPasswordEdit->setText(settings.proxyPassword());

    // Shortcuts
    ui->shortcutsTreeView->model()->loadShortcuts(settings);
}
