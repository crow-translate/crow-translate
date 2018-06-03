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

#ifndef POPUPWINDOW_H
#define POPUPWINDOW_H

#include <QMenu>
#include <QButtonGroup>
#include <QToolButton>
#include <QTextEdit>

#include "qonlinetranslator.h"

namespace Ui {
class PopupWindow;
}

class PopupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PopupWindow(QMenu *languagesMenu, QButtonGroup *sourceGroup, QButtonGroup *translationGroup, QWidget *parent = 0);
    ~PopupWindow();

    QTextEdit *translationEdit();

    QToolButton *swapButton();
    QButtonGroup *sourceButtons();
    QButtonGroup *translationButtons();

    QToolButton *autoSourceButton();
    QToolButton *playSourceButton();
    QToolButton *stopSourceButton();
    QToolButton *copySourceButton();

    QToolButton *autoTranslationButton();
    QToolButton *playTranslationButton();
    QToolButton *stopTranslationButton();
    QToolButton *copyTranslationButton();
    QToolButton *copyAllTranslationButton();


public slots:
    void loadSourceButton(QAbstractButton *button, const int &id);
    void loadTranslationButton(QAbstractButton *button, const int &id);
    void checkSourceButton(const int &id, const bool &checked);
    void checkTranslationButton(const int &id, const bool &checked);

private:
    void showEvent(QShowEvent *event);
    void copyLanguageButtons(QButtonGroup *existingGroup, QButtonGroup *copyingGroup);

    Ui::PopupWindow *ui;

    QButtonGroup *sourceButtonGroup;
    QButtonGroup *translationButtonGroup;
};

#endif // POPUPWINDOW_H
