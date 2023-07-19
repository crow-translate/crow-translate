/*
 *  Copyright © 2018-2023 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "qonlinetts.h"
#include "fileutil/fileutil.h"
#include <QDialog>

class MainWindow;
class AbstractAutostartManager;
class QOnlineTranslator;
class QMediaPlayer;
class QMediaPlaylist;
class ShortcutItem;
class FileUtil;
class ShowMsg;

#ifdef WITH_PORTABLE_MODE
class QCheckBox;
#endif
#ifdef Q_OS_WIN
class QComboBox;
class QPushButton;
class QLabel;
#endif

namespace Ui
{
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsDialog)

public:
    explicit SettingsDialog(MainWindow *parent = nullptr);
    ~SettingsDialog() override;

public slots:
    void accept() override;


private slots:
    void setCurrentPage(int index);

    void onProxyTypeChanged(int type);
    void onWindowModeChanged(int mode);

    void onTrayIconTypeChanged(int type);
    void selectCustomTrayIcon();
    void setCustomTrayIconPreview(const QString &iconPath);

    //Batista: for Workspace and files .txt or .md and mp3
    void onCustFileNameChanged(int mode);//To trate field Cust File Name
    void onCustAudioTTSChanged(int mode);//To trate field Cust Audio File name
    //
    void onSelectWorkSpaceData(); //To Select new WorkSpace
    void onValidFileNameData();   //To Validate data entry to File Name
    void onValidAudioFileData();  //To Validate data entry to Audio Name
    //end
    void selectOcrLanguagesPath();
    void onOcrLanguagesPathChanged(const QString &path);
    void onTesseractParametersCurrentItemChanged();

    void saveYandexEngineVoice(int voice);
    void saveYandexEngineEmotion(int emotion);
    void detectYandexTextLanguage();
    void speakYandexTestText();

    void onGoogleLanguageSelectionChanged(int languageIndex);
    void saveGoogleEngineRegion(int region);
    void detectGoogleTextLanguage();
    void speakGoogleTestText();

    void loadShortcut(ShortcutItem *item);
    void updateAcceptButton();
    void acceptCurrentShortcut();
    void clearCurrentShortcut();
    void resetCurrentShortcut();
    void resetAllShortcuts();

#ifdef Q_OS_WIN
    void downloadUpdatesInfo();
    void checkForUpdates();
#endif
    void restoreDefaults();

private:
    void addLocale(const QLocale &locale);
    void activateCompactMode();
    void loadSettings();

    void detectTestTextLanguage(QOnlineTranslator &translator, QOnlineTranslator::Engine engine);
    void speakTestText(QOnlineTranslator &translator, QOnlineTranslator::Engine engine);

    Ui::SettingsDialog *ui;

    // Manage platform-dependant autostart
    AbstractAutostartManager *m_autostartManager;

    // Test voice
    QOnlineTranslator *m_yandexTranslator;
    QOnlineTranslator *m_googleTranslator;

    //Batista: Work on Files and validation
    FileUtil * m_fileUtil;
    ShowMsg * m_msg;

#ifdef WITH_PORTABLE_MODE
    QCheckBox *m_portableCheckbox;
#endif

#ifdef Q_OS_WIN
    // Check for updates box stuff
    QComboBox *m_checkForUpdatesComboBox;
    QPushButton *m_checkForUpdatesButton;
    QLabel *m_checkForUpdatesStatusLabel;
#endif
};

#endif // SETTINGSDIALOG_H
