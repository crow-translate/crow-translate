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

#include "qonlinetranslator.h"
#include "languagebuttonsgroup.h"

namespace Ui {
class PopupWindow;
}

class PopupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PopupWindow(QMenu *languagesMenu, QString text, QWidget *parent = 0);
    ~PopupWindow();

private slots:
    void on_autoLanguageSourceButton_triggered(QAction *language);
    void on_autoLanguageTranslationButton_triggered(QAction *language);
    void on_speakButton_clicked();
    void on_copyButton_clicked();
    void on_swapButton_clicked();

signals:
    void popupClosed(QString translation);

private:
    void translateText();

    Ui::PopupWindow *ui;

    QString m_selectedText;
    QOnlineTranslator translationData;

    LanguageButtonsGroup *sourceButtonGroup;
    LanguageButtonsGroup *translationButtonGroup;
};

#endif // POPUPWINDOW_H
