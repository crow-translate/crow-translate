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

#include "popupwindow.h"

#include <QBitmap>
#include <QDesktopWidget>
#include <QClipboard>
#include <QSettings>

#include "ui_popupwindow.h"
#include "mainwindow.h"

PopupWindow::PopupWindow(QMenu *languagesMenu, const QString &text, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::PopupWindow),
    sourceButtonGroup (new LanguageButtonsGroup(this, "Source")),
    targetButtonGroup (new LanguageButtonsGroup(this, "Target"))
{
    ui->setupUi(this);

    // Delete this widget when the widget has accepted the close event
    this->setAttribute(Qt::WA_DeleteOnClose);

    // Move popup to cursor and prevent moving offscreen
    QDesktopWidget *screen = QApplication::desktop(); // Screen properties
    QPoint position = QCursor::pos(); // Cursor position
    if (screen->availableGeometry(QCursor::pos()).width() - position.x() - 700 < 0) position.rx()-=700;
    if (screen->availableGeometry(QCursor::pos()).height() - position.y() - 200 < 0) position.ry()-=200;
    PopupWindow::move(position);

    QSettings settings;
    PopupWindow::setWindowOpacity(settings.value("PopupOpacity", 0.8).toDouble());

    // Add languagesMenu to auto-language buttons
    ui->sourceAutoButton->setMenu(languagesMenu);
    ui->targetAutoButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    sourceButtonGroup->addButton(ui->sourceAutoButton, 0);
    sourceButtonGroup->addButton(ui->sourceFirstButton, 1);
    sourceButtonGroup->addButton(ui->sourceSecondButton, 2);
    sourceButtonGroup->addButton(ui->sourceThirdButton, 3);
    targetButtonGroup->addButton(ui->targetAutoButton, 0);
    targetButtonGroup->addButton(ui->targetFirstButton, 1);
    targetButtonGroup->addButton(ui->targetSecondButton, 2);
    targetButtonGroup->addButton(ui->targetThirdButton, 3);

    sourceButtonGroup->loadSettings();
    targetButtonGroup->loadSettings();

    // Translate text automatically when language buttons released
    connect(sourceButtonGroup, static_cast<void (LanguageButtonsGroup::*)(int)>(&LanguageButtonsGroup::buttonReleased), this, &PopupWindow::sourceLanguageButtonPressed);
    connect(targetButtonGroup, static_cast<void (LanguageButtonsGroup::*)(int)>(&LanguageButtonsGroup::buttonReleased), this, &PopupWindow::targetLanguageButtonPressed);

    connect(ui->sayButton, &QToolButton::released, this, &PopupWindow::sayButtonClicked);

    ui->translationEdit->setText(text);
}

PopupWindow::~PopupWindow()
{
    delete ui;
}

void PopupWindow::setTranslation(const QString &text)
{
    ui->translationEdit->setText(text);
}

void PopupWindow::on_sourceAutoButton_triggered(QAction *language)
{
    emit sourceLanguageInserted(language);
    sourceButtonGroup->loadSettings();
}

void PopupWindow::on_targetAutoButton_triggered(QAction *language)
{
    emit targetLanguageInserted(language);
    targetButtonGroup->loadSettings();
}

void PopupWindow::on_copyButton_clicked()
{
    if (ui->translationEdit->toPlainText() != "")
        QApplication::clipboard()->setText(ui->translationEdit->toPlainText());
    else
        qDebug() << tr("Text field is empty");
}

void PopupWindow::on_swapButton_clicked()
{
    emit swapButtonClicked();
    sourceButtonGroup->loadSettings();
    targetButtonGroup->loadSettings();
}
