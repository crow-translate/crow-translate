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

#ifndef ADDLANGDIALOG_H
#define ADDLANGDIALOG_H

#include "qonlinetranslator.h"

#include <QDialog>

namespace Ui {
class AddLangDialog;
}

class AddLangDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddLangDialog(QWidget *parent = nullptr);
    ~AddLangDialog();

    QOnlineTranslator::Language language() const;

private slots:
    void on_AddLangDialog_accepted();
    void on_searchEdit_textChanged(const QString &text);

private:
    Ui::AddLangDialog *ui;

    QOnlineTranslator::Language m_lang;
};

#endif // ADDLANGDIALOG_H
