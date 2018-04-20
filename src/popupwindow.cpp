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

PopupWindow::PopupWindow(QMenu *languagesMenu, const QString &translation, const QString &sourceAutoButtonText, const QString &sourceAutoButtonToolTip, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::PopupWindow),
    sourceButtonGroup (new LanguageButtonsGroup(this, "Source")),
    translationButtonGroup (new LanguageButtonsGroup(this, "Translation"))
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
    ui->translationAutoButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    sourceButtonGroup->addButton(ui->sourceAutoButton, 0);
    sourceButtonGroup->addButton(ui->sourceFirstButton, 1);
    sourceButtonGroup->addButton(ui->sourceSecondButton, 2);
    sourceButtonGroup->addButton(ui->sourceThirdButton, 3);
    translationButtonGroup->addButton(ui->translationAutoButton, 0);
    translationButtonGroup->addButton(ui->translationFirstButton, 1);
    translationButtonGroup->addButton(ui->translationSecondButton, 2);
    translationButtonGroup->addButton(ui->translationThirdButton, 3);

    sourceButtonGroup->loadSettings();
    translationButtonGroup->loadSettings();

    // Translate text automatically when language buttons released
    connect(sourceButtonGroup, qOverload<int>(&LanguageButtonsGroup::buttonReleased), this, &PopupWindow::sourceLanguageButtonPressed);
    connect(translationButtonGroup, qOverload<int>(&LanguageButtonsGroup::buttonReleased), this, &PopupWindow::translationLanguageButtonPressed);

    connect(ui->sayButton, &QToolButton::released, this, &PopupWindow::sayButtonClicked);

    ui->translationEdit->setText(translation);
    ui->sourceAutoButton->setText(sourceAutoButtonText);
    ui->sourceAutoButton->setToolTip(sourceAutoButtonToolTip);
}

PopupWindow::~PopupWindow()
{
    delete ui;
}

void PopupWindow::setTranslation(const QString &text)
{
    ui->translationEdit->setText(text);
}

void PopupWindow::setSourceAutoButton(const QString &text, const QString &toolTip)
{
    ui->sourceAutoButton->setText(text);
    ui->sourceAutoButton->setToolTip(toolTip);
}

void PopupWindow::on_sourceAutoButton_triggered(QAction *language)
{
    emit sourceLanguageInserted(language);
    sourceButtonGroup->loadSettings();
}

void PopupWindow::on_translationAutoButton_triggered(QAction *language)
{
    emit translationLanguageInserted(language);
    translationButtonGroup->loadSettings();
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
    translationButtonGroup->loadSettings();
}
