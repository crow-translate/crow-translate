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

PopupWindow::PopupWindow(QMenu *languagesMenu, QButtonGroup *sourceGroup, QButtonGroup *translationGroup, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::PopupWindow),
    sourceButtonGroup (new QButtonGroup(this)),
    translationButtonGroup (new QButtonGroup(this))
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

    copyLanguageButtons(sourceButtonGroup, sourceGroup);
    copyLanguageButtons(translationButtonGroup, translationGroup);

    connect(this->sourceButtonGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, &PopupWindow::sourceButtonClicked);
    connect(this->translationButtonGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, &PopupWindow::translationButtonClicked);
    connect(ui->sayButton, &QToolButton::released, this, &PopupWindow::sayButtonClicked);
}

PopupWindow::~PopupWindow()
{
    delete ui;
}

void PopupWindow::setTranslation(const QString &text)
{
    ui->translationEdit->setText(text);
}

void PopupWindow::copySourceButton(QAbstractButton *button, const int &id)
{
    sourceButtonGroup->button(id)->setText(button->text());
    sourceButtonGroup->button(id)->setToolTip(button->toolTip());
    sourceButtonGroup->button(id)->setVisible(true);
}

void PopupWindow::copyTranslationButton(QAbstractButton *button, const int &id)
{
    translationButtonGroup->button(id)->setText(button->text());
    translationButtonGroup->button(id)->setToolTip(button->toolTip());
    translationButtonGroup->button(id)->setVisible(true);
}

void PopupWindow::checkSourceButton(const int &id, bool checked)
{
    if (checked)
        sourceButtonGroup->button(id)->setChecked(true);
}

void PopupWindow::checkTranslationButton(const int &id, bool checked)
{
    if (checked)
        translationButtonGroup->button(id)->setChecked(true);
}

void PopupWindow::on_sourceAutoButton_triggered(QAction *language)
{
    emit sourceLanguageInserted(language);
}

void PopupWindow::on_translationAutoButton_triggered(QAction *language)
{
    emit translationLanguageInserted(language);
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
}

void PopupWindow::copyLanguageButtons(QButtonGroup *existingGroup, QButtonGroup *copyingGroup)
{
    for (auto i = 0; i < 4; i++) {
        if (copyingGroup->buttons().at(i)->text() != "") {
            existingGroup->buttons().at(i)->setText(copyingGroup->buttons().at(i)->text());
            existingGroup->buttons().at(i)->setToolTip(copyingGroup->buttons().at(i)->toolTip());
        }
        else
            existingGroup->buttons().at(i)->setVisible(false);
    }
    existingGroup->button(copyingGroup->checkedId())->setChecked(true);
}
