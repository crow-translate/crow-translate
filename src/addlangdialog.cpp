/*
 *  Copyright © 2018 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "addlangdialog.h"
#include "ui_addlangdialog.h"

#include <QPushButton>

AddLangDialog::AddLangDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddLangDialog)
{
    ui->setupUi(this);

    // Load languages
    for (int i = 1; i <= QOnlineTranslator::Zulu; ++i) {
        const auto lang = static_cast<QOnlineTranslator::Language>(i);

        auto item = new QListWidgetItem();
        item->setText(QOnlineTranslator::languageString(lang));
        item->setIcon(QIcon(":/icons/flags/" + QOnlineTranslator::languageCode(lang) + ".svg"));
        item->setData(Qt::UserRole, lang);
        ui->langListWidget->addItem(item);
    }
    ui->langListWidget->setCurrentRow(0);
}

AddLangDialog::~AddLangDialog()
{
    delete ui;
}

void AddLangDialog::on_searchEdit_textChanged(const QString &text)
{
    bool isItemSelected = false;
    for (int i = 0; i < ui->langListWidget->count(); ++i) {
        QListWidgetItem *item = ui->langListWidget->item(i);
        if (item->text().contains(text, Qt::CaseInsensitive)) {
            item->setHidden(false);
            if (!isItemSelected) {
                ui->langListWidget->setCurrentItem(item); // Select first unhidden item
                isItemSelected = true;
            }
        } else {
            item->setHidden(true);
        }
    }

    // Disable Ok button if no item selected
    ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isItemSelected);
}

void AddLangDialog::on_AddLangDialog_accepted()
{
    m_lang = ui->langListWidget->currentItem()->data(Qt::UserRole).value<QOnlineTranslator::Language>();
}

QOnlineTranslator::Language AddLangDialog::language() const
{
    return m_lang;
}
