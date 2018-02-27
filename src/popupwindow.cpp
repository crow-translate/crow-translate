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

#include "qonlinetranslator.h"
#include "ui_popupwindow.h"
#include "mainwindow.h"

PopupWindow::PopupWindow(QMenu *languagesMenu, QString text, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::PopupWindow),
    selectedText(new QString(text)),
    inputLanguages (new ButtonGroupLanguages(this)),
    outputLanguages (new ButtonGroupLanguages(this))
{
    inputLanguages->setObjectName("inputLanguages");
    outputLanguages->setObjectName("outputLanguages");

    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose); // Delete this widget when the widget has accepted the close event

    // Prevent moving popup offscreen
    QDesktopWidget *screen = QApplication::desktop(); // Screen properties
    QPoint position = QCursor::pos(); // Cursor position
    if (screen->availableGeometry(QCursor::pos()).width() - position.x() - 700 < 0) position.rx()-=700;
    if (screen->availableGeometry(QCursor::pos()).height() - position.y() - 200 < 0) position.ry()-=200;

    // Move popup to cursor
    PopupWindow::move(position);
    PopupWindow::setWindowOpacity(0.8);

//    setMask(QPixmap(":/images/data/images/popupmask.png").scaled(size()).mask());

    // Add languagesMenu to auto-language buttons
    ui->inputLanguagesButton->setMenu(languagesMenu);
    ui->outputLanguagesButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    inputLanguages->addButton(ui->inputLanguagesButton, 0);
    inputLanguages->addButton(ui->inputLanguageButton1);
    inputLanguages->addButton(ui->inputLanguageButton2);
    inputLanguages->addButton(ui->inputLanguageButton3);

    outputLanguages->addButton(ui->outputLanguagesButton, 0);
    outputLanguages->addButton(ui->outputLanguageButton1);
    outputLanguages->addButton(ui->outputLanguageButton2);
    outputLanguages->addButton(ui->outputLanguageButton3);

    inputLanguages->loadSettings();
    outputLanguages->loadSettings();

    // Translate text automatically when language buttons released
    connect(inputLanguages, static_cast<void (ButtonGroupLanguages::*)(int)>(&ButtonGroupLanguages::buttonReleased), this, &PopupWindow::translateText);
    connect(outputLanguages, static_cast<void (ButtonGroupLanguages::*)(int)>(&ButtonGroupLanguages::buttonReleased), this, &PopupWindow::translateText);

    translateText();
}

PopupWindow::~PopupWindow()
{
    delete selectedText;
    delete ui;
}

// Insert new language to input buttons
void PopupWindow::on_inputLanguagesButton_triggered(QAction *language)
{
    short languageIndex = language->data().toInt();
    inputLanguages->insertLanguage(languageIndex);
    translateText();
}

// Insert new language to output buttons
void PopupWindow::on_outputLanguagesButton_triggered(QAction *language)
{
    short languageIndex = language->data().toInt();
    outputLanguages->insertLanguage(languageIndex);
    translateText();
}

void PopupWindow::on_speakButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "") {
        QOnlineTranslator::say(ui->outputEdit->toPlainText(), outputLanguages->checkedId());
    }
    else qDebug() << tr("Text field is empty");
}

void PopupWindow::on_copyButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "") {
        QApplication::clipboard()->setText(ui->outputEdit->toPlainText());
    }
    else qDebug() << tr("Text field is empty");
}

void PopupWindow::on_swapButton_clicked()
{
    ButtonGroupLanguages::swapChecked(inputLanguages, outputLanguages);
    translateText();
}

void PopupWindow::translateText()
{
    ui->outputEdit->setPlainText(QOnlineTranslator::translate(*selectedText, inputLanguages->checkedId(), outputLanguages->checkedId()));
}
