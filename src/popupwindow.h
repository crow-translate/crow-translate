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

#ifndef POPUPWINDOW_H
#define POPUPWINDOW_H

#include <QWidget>

class QShortcut;
class QTimer;
class MainWindow;
class LanguageButtonsWidget;

namespace Ui
{
class PopupWindow;
}

class PopupWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PopupWindow)

public:
    explicit PopupWindow(MainWindow *parent = nullptr);
    ~PopupWindow() override;

private:
    void loadSettings();
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

    static void connectLanguageButtons(LanguageButtonsWidget *popupButtons, const LanguageButtonsWidget *mainWindowButtons);

    Ui::PopupWindow *ui;
    QShortcut *m_closeWindowsShortcut;
    QTimer *m_closeWindowTimer = nullptr;
};

#endif // POPUPWINDOW_H
