/*
 *  Copyright © 2018-2021 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#ifndef ADDLANGDIALOG_H
#define ADDLANGDIALOG_H

#include "qonlinetranslator.h"

#include <QDialog>

class QListWidget;
class QAbstractButton;
class QShortcut;

namespace Ui {
class AddLanguageDialog;
}

class AddLanguageDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(AddLanguageDialog)

public:
    explicit AddLanguageDialog(const QVector<QOnlineTranslator::Language> &currentLang, QWidget *parent = nullptr);
    ~AddLanguageDialog() override;

    QVector<QOnlineTranslator::Language> languages() const;

public slots:
    void accept() override;

private slots:
    void filterLanguages(const QString &text);

    void moveLanguageRight();
    void moveLanguageLeft();
    void moveLanguageUp();
    void moveLanguageDown();

    void checkVerticalMovement(int row);

private:
    static void addLanguage(QListWidget *widget, QOnlineTranslator::Language lang);
    static void moveLanguageVertically(QListWidget *widget, int offset);
    static void moveLanguageHorizontally(QListWidget *from, QListWidget *to, QAbstractButton *addButton, QAbstractButton *removeButton);

    Ui::AddLanguageDialog *ui;
    QShortcut *m_searchShortcut;
    QVector<QOnlineTranslator::Language> m_languages;
};

#endif // ADDLANGDIALOG_H
