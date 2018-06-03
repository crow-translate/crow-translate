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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QMenu>
#include <QTreeWidget>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QMenu *languagesMenu, QWidget *parent = 0);
    static const QStringList ICONS;
    ~SettingsDialog();

private slots:
    void on_dialogBox_accepted();
    void on_trayCheckBox_toggled(bool checked);
    void on_resetSettingsButton_clicked();

    void on_proxyTypeComboBox_currentIndexChanged(int index);
    void on_proxyAuthCheckBox_toggled(bool checked);

    void on_shortcutsTreeWidget_itemSelectionChanged();
    void on_shortcutSequenceEdit_editingFinished();
    void on_acceptShortcutButton_clicked();
    void on_resetShortcutButton_clicked();
    void on_resetAllShortcutsButton_clicked();

signals:
    void languageChanged();
    void proxyChanged();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
