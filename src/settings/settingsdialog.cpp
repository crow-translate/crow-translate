/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
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
#include "qhotkey.h"
#ifdef Q_OS_WIN
#include "updaterdialog.h"
#include "qgittag.h"
#endif

#include <QNetworkProxy>
#include <QFileDialog>
#include <QScreen>
#include <QMessageBox>
#include <QDate>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_translator(new QOnlineTranslator(this))
#ifdef PORTABLE_MODE
    , m_portableCheckbox(new QCheckBox(tr("Portable mode"), this))
#endif
{
    ui->setupUi(this);
    connect(ui->dialogButtonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);
    connect(ui->globalShortcutsCheckBox, &QCheckBox::toggled, ui->shortcutsTreeView->model(), &ShortcutsModel::setGlobalShortuctsEnabled);
    ui->logoLabel->setPixmap(QIcon::fromTheme(QStringLiteral("crow-translate")).pixmap(512, 512));
    ui->versionLabel->setText(SingleApplication::applicationVersion());

#ifdef PORTABLE_MODE
    m_portableCheckbox->setToolTip(tr("Use %1 from the application folder to store settings").arg(AppSettings::portableConfigName()));
    ui->generalGroupBox->layout()->addWidget(m_portableCheckbox);
#endif

    // Test voice
    ui->playerButtons->setMediaPlayer(new QMediaPlayer);
    connect(m_translator, &QOnlineTranslator::finished, this, &SettingsDialog::speakTestText);

    // Set item data in comboboxes
    ui->localeComboBox->addItem(tr("<System language>"), QLocale::AnyLanguage);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/my.svg")), QStringLiteral("Bahasa Melayu"), QLocale::Malay);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/gb.svg")), QStringLiteral("English"), QLocale::English);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/fr.svg")), QStringLiteral("Française"), QLocale::French);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/pl.svg")), QStringLiteral("Polski"), QLocale::Polish);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/br.svg")), QStringLiteral("Português (Brasil)"), QLocale::Portuguese);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/tr.svg")), QStringLiteral("Türk"), QLocale::Turkish);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/ru.svg")), QStringLiteral("Русский"), QLocale::Russian);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/ua.svg")), QStringLiteral("Українська"), QLocale::Ukrainian);
    ui->localeComboBox->addItem(QIcon(QStringLiteral(":/icons/flags/cn.svg")), QStringLiteral("简体中文 (中国)"), QLocale::Chinese);

    ui->primaryLanguageComboBox->addItem(tr("<System language>"), QOnlineTranslator::Auto);
    ui->secondaryLanguageComboBox->addItem(tr("<System language>"), QOnlineTranslator::Auto);
    for (int i = 1; i <= QOnlineTranslator::Zulu; ++i) {
        const auto lang = static_cast<QOnlineTranslator::Language>(i);
        const QIcon langIcon = LangButtonGroup::countryIcon(lang);

        ui->primaryLanguageComboBox->addItem(langIcon, QOnlineTranslator::languageString(lang), i);
        ui->secondaryLanguageComboBox->addItem(langIcon, QOnlineTranslator::languageString(lang), i);
    }

    // Sort languages in comboboxes alphabetically
    ui->primaryLanguageComboBox->model()->sort(0);
    ui->secondaryLanguageComboBox->model()->sort(0);

    // Set maximum and minimum values for the size of the popup window
    ui->popupWidthSlider->setMaximum(SingleApplication::primaryScreen()->availableGeometry().width());
    ui->popupWidthSpinBox->setMaximum(SingleApplication::primaryScreen()->availableGeometry().width());
    ui->popupHeightSlider->setMaximum(SingleApplication::primaryScreen()->availableGeometry().height());
    ui->popupHeightSpinBox->setMaximum(SingleApplication::primaryScreen()->availableGeometry().height());
    ui->popupWidthSlider->setMinimum(200);
    ui->popupWidthSpinBox->setMinimum(200);
    ui->popupHeightSlider->setMinimum(200);
    ui->popupHeightSpinBox->setMinimum(200);

    // Disable (enable) opacity slider if "Window mode" ("Popup mode") selected
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacityLabel, &QSlider::setDisabled);
    connect(ui->windowModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), ui->popupOpacitySlider, &QSlider::setDisabled);

#ifdef Q_OS_WIN
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
    connect(m_checkForUpdatesButton, &QPushButton::clicked, this, &SettingsDialog::downloadUpdatesInfo);
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

void SettingsDialog::accept()
{
    QDialog::accept();

    // Set settings location first
#ifdef PORTABLE_MODE
    AppSettings::setPortableModeEnabled(m_portableCheckbox->isChecked());
#endif

    // General settings
    AppSettings settings;
    settings.setWindowMode(static_cast<AppSettings::WindowMode>(ui->windowModeComboBox->currentIndex()));
    settings.setLanguage(ui->localeComboBox->currentData().value<QLocale::Language>());
    settings.setShowTrayIcon(ui->showTrayIconCheckBox->isChecked());
    settings.setStartMinimized(ui->startMinimizedCheckBox->isChecked());
    AppSettings::setAutostartEnabled(ui->autostartCheckBox->isChecked());
#ifdef Q_OS_WIN
    settings.setCheckForUpdatesInterval(static_cast<AppSettings::Interval>(m_checkForUpdatesComboBox->currentIndex()));
#endif

    // Interface settings
    settings.setPopupOpacity(static_cast<double>(ui->popupOpacitySlider->value()) / 100);
    settings.setPopupWidth(ui->popupWidthSpinBox->value());
    settings.setPopupHeight(ui->popupHeightSpinBox->value());
    settings.setTrayIconType(static_cast<TrayIcon::IconType>(ui->trayIconComboBox->currentIndex()));
    settings.setCustomIconPath(ui->customTrayIconEdit->text());

    // Translation settings
    settings.setSourceTranslitEnabled(ui->sourceTranslitCheckBox->isChecked());
    settings.setTranslationTranslitEnabled(ui->translationTranslitCheckBox->isChecked());
    settings.setSourceTranscriptionEnabled(ui->sourceTranscriptionCheckBox->isChecked());
    settings.setTranslationOptionsEnabled(ui->translationOptionsCheckBox->isChecked());
    settings.setExamplesEnabled(ui->examplesCheckBox->isChecked());
    settings.setSimplifySource(ui->sourceSimplificationCheckBox->isChecked());
    settings.setPrimaryLanguage(ui->primaryLanguageComboBox->currentData().value<QOnlineTranslator::Language>());
    settings.setSecondaryLanguage(ui->secondaryLanguageComboBox->currentData().value<QOnlineTranslator::Language>());
    settings.setForceSourceAutodetect(ui->forceSourceAutoCheckBox->isChecked());
    settings.setForceTranslationAutodetect(ui->forceTranslationAutoCheckBox->isChecked());

    // Speech synthesis settings
    settings.setVoice(QOnlineTranslator::Yandex, ui->playerButtons->voice(QOnlineTranslator::Yandex));
    settings.setEmotion(QOnlineTranslator::Yandex, ui->playerButtons->emotion(QOnlineTranslator::Yandex));

    // Connection settings
    settings.setProxyType(static_cast<QNetworkProxy::ProxyType>(ui->proxyTypeComboBox->currentIndex()));
    settings.setProxyHost(ui->proxyHostEdit->text());
    settings.setProxyPort(static_cast<quint16>(ui->proxyPortSpinbox->value()));
    settings.setProxyAuthEnabled(ui->proxyAuthCheckBox->isChecked());
    settings.setProxyUsername(ui->proxyUsernameEdit->text());
    settings.setProxyPassword(ui->proxyPasswordEdit->text());

    // Shortcuts
    if (QHotkey::isPlatformSupported())
        settings.setGlobalShortcutsEnabled(ui->globalShortcutsCheckBox->isChecked());
    ui->shortcutsTreeView->model()->saveShortcuts(settings);
}

void SettingsDialog::processProxyTypeChanged(int type)
{
    if (type == QNetworkProxy::HttpProxy || type == QNetworkProxy::Socks5Proxy) {
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
void SettingsDialog::processTrayIconTypeChanged(int type)
{
    if (type == TrayIcon::CustomIcon) {
        ui->customTrayIconLabel->setEnabled(true);
        ui->customTrayIconEdit->setEnabled(true);
        ui->customTrayIconButton->setEnabled(true);
    } else {
        ui->customTrayIconLabel->setEnabled(false);
        ui->customTrayIconEdit->setEnabled(false);
        ui->customTrayIconButton->setEnabled(false);
    }
}

void SettingsDialog::chooseCustomTrayIcon()
{
    const QString path = ui->customTrayIconEdit->text().left(ui->customTrayIconEdit->text().lastIndexOf(QDir::separator()));
    const QString file = QFileDialog::getOpenFileName(this, tr("Select icon"), path, tr("Images (*.png *.ico *.svg *.jpg);;All files()"));
    if (!file.isEmpty())
        ui->customTrayIconEdit->setText(file);
}

void SettingsDialog::setCustomTrayIconPreview(const QString &iconPath)
{
    ui->customTrayIconButton->setIcon(TrayIcon::customTrayIcon(iconPath));
}

// Disable unsupported voice settings for engines.
void SettingsDialog::showAvailableTtsOptions(int engine)
{
    // Avoid index changed signal
    ui->voiceComboBox->blockSignals(true);
    ui->emotionComboBox->blockSignals(true);

    switch (engine) {
    case QOnlineTranslator::Bing:
        setSpeechTestEnabled(false);
        setVoiceOptions({});
        setEmotionOptions({});
        break;
    case QOnlineTranslator::Google:
        setSpeechTestEnabled(true);
        setVoiceOptions({});
        setEmotionOptions({});
        break;
    case QOnlineTranslator::Yandex:
        setSpeechTestEnabled(true);
        setVoiceOptions({{tr("Zahar"), QOnlineTts::Zahar},
                         {tr("Ermil"), QOnlineTts::Ermil},
                         {tr("Jane"), QOnlineTts::Jane},
                         {tr("Oksana"), QOnlineTts::Oksana},
                         {tr("Alyss"), QOnlineTts::Alyss},
                         {tr("Omazh"), QOnlineTts::Omazh}});
        setEmotionOptions({{tr("Neutral"), QOnlineTts::Neutral},
                           {tr("Good"), QOnlineTts::Good},
                           {tr("Evil"), QOnlineTts::Evil}});

        const QOnlineTts::Voice voice = ui->playerButtons->voice(QOnlineTranslator::Yandex);
        const QOnlineTts::Emotion emotion = ui->playerButtons->emotion(QOnlineTranslator::Yandex);

        ui->voiceComboBox->setCurrentIndex(ui->voiceComboBox->findData(voice));
        ui->emotionComboBox->setCurrentIndex(ui->emotionComboBox->findData(emotion));
        break;
    }

    ui->voiceComboBox->blockSignals(false);
    ui->emotionComboBox->blockSignals(false);
}

// Save current engine voice settings
void SettingsDialog::saveEngineVoice(int voice)
{
    ui->playerButtons->setVoice(static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex()), ui->voiceComboBox->itemData(voice).value<QOnlineTts::Voice>());
}

// Save current engine emotion settings
void SettingsDialog::saveEngineEmotion(int emotion)
{
    ui->playerButtons->setEmotion(static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex()), ui->emotionComboBox->itemData(emotion).value<QOnlineTts::Emotion>());
}

// To play test text
void SettingsDialog::detectTextLanguage()
{
    if (ui->testSpeechEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("Nothing to play"), tr("Playback text is empty"));
        return;
    }

    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    m_translator->detectLanguage(ui->testSpeechEdit->text(), engine);
}

void SettingsDialog::speakTestText()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        QMessageBox::critical(this, tr("Unable to detect language"), m_translator->errorString());
        return;
    }

    const auto engine = static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
    ui->playerButtons->speak(ui->testSpeechEdit->text(), m_translator->sourceLanguage(), engine);
}

void SettingsDialog::loadShortcut(ShortcutItem *item)
{
    if (item->childCount() == 0) {
        ui->shortcutGroupBox->setEnabled(true);
        ui->shortcutSequenceEdit->setKeySequence(item->shortcut());
    } else {
        ui->shortcutGroupBox->setEnabled(false);
        ui->shortcutSequenceEdit->clear();
    }
}

void SettingsDialog::updateAcceptButton()
{
    if (ui->shortcutsTreeView->currentItem()->shortcut() != ui->shortcutSequenceEdit->keySequence())
        ui->acceptShortcutButton->setEnabled(true);
    else
        ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::acceptCurrentShortcut()
{
    ui->shortcutsTreeView->currentItem()->setShortcut(ui->shortcutSequenceEdit->keySequence());
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::clearCurrentShortcut()
{
    ui->shortcutSequenceEdit->clear();
    ui->acceptShortcutButton->setEnabled(true);
}

void SettingsDialog::resetCurrentShortcut()
{
    ui->shortcutsTreeView->currentItem()->resetShortcut();
    ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeView->currentItem()->shortcut());
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::resetAllShortcuts()
{
    ui->shortcutsTreeView->model()->resetAllShortcuts();
}

#ifdef Q_OS_WIN
void SettingsDialog::downloadUpdatesInfo()
{
    m_checkForUpdatesButton->setEnabled(false);
    m_checkForUpdatesStatusLabel->setText(tr("Checking for updates..."));

    // Get update information
    auto *release = new QGitTag(this);
    connect(release, &QGitTag::finished, this, &SettingsDialog::checkForUpdates);
    release->get("crow-translate", "crow-translate");
}

void SettingsDialog::checkForUpdates()
{
    auto *release = qobject_cast<QGitTag *>(sender());
    release->deleteLater();
    m_checkForUpdatesButton->setEnabled(true);

    if (release->error()) {
        m_checkForUpdatesStatusLabel->setStyleSheet("color: red");
        m_checkForUpdatesStatusLabel->setText(release->errorString());
        return;
    }

    if (const int installer = release->assetId(".exe"); SingleApplication::applicationVersion() < release->tagName() && installer != -1) {
        m_checkForUpdatesStatusLabel->setStyleSheet("color: green");
        m_checkForUpdatesStatusLabel->setText(tr("Update available!"));
        auto *updaterDialog = new UpdaterDialog(release, installer, this);
        updaterDialog->setAttribute(Qt::WA_DeleteOnClose);
        updaterDialog->open();
    } else {
        m_checkForUpdatesStatusLabel->setStyleSheet("");
        m_checkForUpdatesStatusLabel->setText(tr("No updates available."));
    }

    AppSettings settings;
    settings.setLastUpdateCheckDate(QDate::currentDate());
}
#endif


void SettingsDialog::restoreDefaults()
{
    // General settings
    ui->localeComboBox->setCurrentIndex(ui->localeComboBox->findData(AppSettings::defaultLanguage()));
    ui->windowModeComboBox->setCurrentIndex(AppSettings::defaultWindowMode());
    ui->showTrayIconCheckBox->setChecked(AppSettings::defaultShowTrayIcon());
    ui->startMinimizedCheckBox->setChecked(AppSettings::defaultStartMinimized());
    ui->autostartCheckBox->setChecked(AppSettings::defaultAutostartEnabled());
#ifdef Q_OS_WIN
    m_checkForUpdatesComboBox->setCurrentIndex(AppSettings::defaultCheckForUpdatesInterval());
#endif

    // Interface settings
    ui->popupOpacitySlider->setValue(static_cast<int>(AppSettings::defaultPopupOpacity() * 100));
    ui->popupWidthSpinBox->setValue(AppSettings::defaultPopupWidth());
    ui->popupHeightSpinBox->setValue(AppSettings::defaultPopupHeight());
    ui->trayIconComboBox->setCurrentIndex(AppSettings::defaultTrayIconType());
    ui->customTrayIconEdit->setText(AppSettings::defaultCustomIconPath());

    // Translation settings
    ui->sourceTranslitCheckBox->setChecked(AppSettings::defaultSourceTranslitEnabled());
    ui->translationTranslitCheckBox->setChecked(AppSettings::defaultTranslationTranslitEnabled());
    ui->sourceTranscriptionCheckBox->setChecked(AppSettings::defaultSourceTranscriptionEnabled());
    ui->translationOptionsCheckBox->setChecked(AppSettings::defaultTranslationOptionsEnabled());
    ui->examplesCheckBox->setChecked(AppSettings::defaultExamplesEnabled());
    ui->sourceSimplificationCheckBox->setChecked(AppSettings::defaultSimplifySource());
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(AppSettings::defaultPrimaryLanguage()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(AppSettings::defaultSecondaryLanguage()));
    ui->forceSourceAutoCheckBox->setChecked(AppSettings::defaultForceSourceAutodetect());
    ui->forceTranslationAutoCheckBox->setChecked(AppSettings::defaultForceTranslationAutodetect());

    // Speech synthesis settings
    ui->playerButtons->setVoice(QOnlineTranslator::Yandex, AppSettings::defaultVoice(QOnlineTranslator::Yandex));
    ui->playerButtons->setEmotion(QOnlineTranslator::Yandex, AppSettings::defaultEmotion(QOnlineTranslator::Yandex));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(AppSettings::defaultProxyType());
    ui->proxyHostEdit->setText(AppSettings::defaultProxyHost());
    ui->proxyPortSpinbox->setValue(AppSettings::defaultProxyPort());
    ui->proxyAuthCheckBox->setChecked(AppSettings::defaultProxyAuthEnabled());
    ui->proxyUsernameEdit->setText(AppSettings::defaultProxyUsername());
    ui->proxyPasswordEdit->setText(AppSettings::defaultProxyPassword());

    // Shortcuts
    if (QHotkey::isPlatformSupported())
        ui->globalShortcutsCheckBox->setEnabled(AppSettings::defaultGlobalShortcutsEnabled());
    resetAllShortcuts();
}

void SettingsDialog::loadSettings()
{
    // General settings
    const AppSettings settings;
    ui->localeComboBox->setCurrentIndex(ui->localeComboBox->findData(settings.language()));
    ui->windowModeComboBox->setCurrentIndex(settings.windowMode());
    ui->showTrayIconCheckBox->setChecked(settings.isShowTrayIcon());
    ui->startMinimizedCheckBox->setChecked(settings.isStartMinimized());
    ui->autostartCheckBox->setChecked(AppSettings::isAutostartEnabled());
#ifdef PORTABLE_MODE
    m_portableCheckbox->setChecked(settings.isPortableModeEnabled());
#endif
#ifdef Q_OS_WIN
    m_checkForUpdatesComboBox->setCurrentIndex(settings.checkForUpdatesInterval());
#endif

    // Interface settings
    ui->popupOpacitySlider->setValue(static_cast<int>(settings.popupOpacity() * 100));
    ui->popupWidthSpinBox->setValue(settings.popupWidth());
    ui->popupHeightSpinBox->setValue(settings.popupHeight());

    ui->trayIconComboBox->setCurrentIndex(settings.trayIconType());
    ui->customTrayIconEdit->setText(settings.customIconPath());

    // Translation settings
    ui->sourceTranslitCheckBox->setChecked(settings.isSourceTranslitEnabled());
    ui->translationTranslitCheckBox->setChecked(settings.isTranslationTranslitEnabled());
    ui->sourceTranscriptionCheckBox->setChecked(settings.isSourceTranscriptionEnabled());
    ui->translationOptionsCheckBox->setChecked(settings.isTranslationOptionsEnabled());
    ui->examplesCheckBox->setChecked(settings.isExamplesEnabled());
    ui->sourceSimplificationCheckBox->setChecked(settings.isSimplifySource());
    ui->primaryLanguageComboBox->setCurrentIndex(ui->primaryLanguageComboBox->findData(settings.primaryLanguage()));
    ui->secondaryLanguageComboBox->setCurrentIndex(ui->secondaryLanguageComboBox->findData(settings.secondaryLanguage()));
    ui->forceSourceAutoCheckBox->setChecked(settings.isForceSourceAutodetect());
    ui->forceTranslationAutoCheckBox->setChecked(settings.isForceTranslationAutodetect());

    // Speech synthesis settings
    ui->playerButtons->setVoice(QOnlineTranslator::Yandex, settings.voice(QOnlineTranslator::Yandex));
    ui->playerButtons->setEmotion(QOnlineTranslator::Yandex, settings.emotion(QOnlineTranslator::Yandex));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(settings.proxyType());
    ui->proxyHostEdit->setText(settings.proxyHost());
    ui->proxyPortSpinbox->setValue(settings.proxyPort());
    ui->proxyAuthCheckBox->setChecked(settings.isProxyAuthEnabled());
    ui->proxyUsernameEdit->setText(settings.proxyUsername());
    ui->proxyPasswordEdit->setText(settings.proxyPassword());

    // Shortcuts
    if (QHotkey::isPlatformSupported()) {
        ui->globalShortcutsCheckBox->setChecked(settings.isGlobalShortuctsEnabled());
    } else {
        ui->globalShortcutsCheckBox->setChecked(false);
        ui->globalShortcutsCheckBox->setEnabled(false);
    }
    ui->shortcutsTreeView->model()->loadShortcuts(settings);
}

void SettingsDialog::setVoiceOptions(const QMap<QString, QOnlineTts::Voice> &voices)
{
    ui->voiceComboBox->clear();

    if (voices.isEmpty()) {
        // Disable voice settings
        ui->voiceLabel->setEnabled(false);
        ui->voiceComboBox->setEnabled(false);
        ui->voiceComboBox->addItem(tr("Default"));
        return;
    }

    ui->voiceLabel->setEnabled(true);
    ui->voiceComboBox->setEnabled(true);
    for (auto it = voices.cbegin(); it != voices.cend(); ++it)
        ui->voiceComboBox->addItem(it.key(), it.value());
}

void SettingsDialog::setEmotionOptions(const QMap<QString, QOnlineTts::Emotion> &emotions)
{
    ui->emotionComboBox->clear();

    if (emotions.isEmpty()) {
        // Disable emotion settings
        ui->emotionLabel->setEnabled(false);
        ui->emotionComboBox->setEnabled(false);
        ui->emotionComboBox->addItem(tr("Default"));
        return;
    }

    ui->emotionLabel->setEnabled(true);
    ui->emotionComboBox->setEnabled(true);
    for (auto it = emotions.cbegin(); it != emotions.cend(); ++it)
        ui->emotionComboBox->addItem(it.key(), it.value());
}

void SettingsDialog::setSpeechTestEnabled(bool enabled)
{
    ui->testSpeechEdit->setEnabled(enabled);
    ui->playerButtons->setEnabled(enabled);
    ui->testSpeechLabel->setEnabled(enabled);
}
