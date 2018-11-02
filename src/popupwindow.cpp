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

#include <QScreen>
#include <QClipboard>
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#endif

#include "ui_popupwindow.h"
#include "appsettings.h"
#include "mainwindow.h"

PopupWindow::PopupWindow(QMenu *languagesMenu, QButtonGroup *sourceGroup, QButtonGroup *translationGroup, QWidget *parent) :
    QWidget(parent, Qt::Popup),
    ui(new Ui::PopupWindow),
    sourceButtonGroup (new QButtonGroup(this)),
    translationButtonGroup (new QButtonGroup(this)),
    closeWindowsShortcut (new QShortcut(this))
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    AppSettings settings;
    setWindowOpacity(settings.popupOpacity());
    resize(settings.popupWidth(), settings.popupHeight());

    // Translation button group
    sourceButtonGroup->addButton(ui->autoSourceButton, 0);
    sourceButtonGroup->addButton(ui->firstSourceButton, 1);
    sourceButtonGroup->addButton(ui->secondSourceButton, 2);
    sourceButtonGroup->addButton(ui->thirdSourceButton, 3);
    copyLanguageButtons(sourceButtonGroup, sourceGroup);

    // Source button group
    translationButtonGroup->addButton(ui->autoTranslationButton, 0);
    translationButtonGroup->addButton(ui->firstTranslationButton, 1);
    translationButtonGroup->addButton(ui->secondTranslationButton, 2);
    translationButtonGroup->addButton(ui->thirdTranslationButton, 3);
    copyLanguageButtons(translationButtonGroup, translationGroup);

    // Language buttons style
    Qt::ToolButtonStyle langsStyle = settings.popupLanguagesStyle();
    ui->firstSourceButton->setToolButtonStyle(langsStyle);
    ui->secondSourceButton->setToolButtonStyle(langsStyle);
    ui->thirdSourceButton->setToolButtonStyle(langsStyle);
    ui->firstTranslationButton->setToolButtonStyle(langsStyle);
    ui->secondTranslationButton->setToolButtonStyle(langsStyle);
    ui->thirdTranslationButton->setToolButtonStyle(langsStyle);

    // Control buttons style
    Qt::ToolButtonStyle controlsStyle = settings.popupLanguagesStyle();
    ui->playSourceButton->setToolButtonStyle(controlsStyle);
    ui->stopSourceButton->setToolButtonStyle(controlsStyle);
    ui->copySourceButton->setToolButtonStyle(controlsStyle);
    ui->playTranslationButton->setToolButtonStyle(controlsStyle);
    ui->stopTranslationButton->setToolButtonStyle(controlsStyle);
    ui->copyTranslationButton->setToolButtonStyle(controlsStyle);
    ui->copyAllTranslationButton->setToolButtonStyle(controlsStyle);

    // Shortcuts
    ui->playSourceButton->setShortcut(settings.playSourceHotkey());
    ui->playTranslationButton->setShortcut(settings.playTranslationHotkey());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationHotkey());
    closeWindowsShortcut->setKey(settings.closeWindowHotkey());
    connect(closeWindowsShortcut, &QShortcut::activated, this, &PopupWindow::close);

    // Add languages to auto-language buttons
    ui->autoSourceButton->setMenu(languagesMenu);
    ui->autoTranslationButton->setMenu(languagesMenu);
}

PopupWindow::~PopupWindow()
{
    delete ui;
}

QTextEdit *PopupWindow::translationEdit()
{
    return ui->translationEdit;
}

QToolButton *PopupWindow::swapButton()
{
    return ui->swapButton;
}

void PopupWindow::loadSourceButton(QAbstractButton *button, int id)
{
    copyButton(sourceButtonGroup, button, id);
}

void PopupWindow::loadTranslationButton(QAbstractButton *button, int id)
{
    copyButton(translationButtonGroup, button, id);
}

void PopupWindow::checkSourceButton(int id, bool checked)
{
    if (checked)
        sourceButtonGroup->button(id)->setChecked(true);
}

void PopupWindow::checkTranslationButton(int id, bool checked)
{
    if (checked)
        translationButtonGroup->button(id)->setChecked(true);
}

QToolButton *PopupWindow::autoSourceButton()
{
    return ui->autoSourceButton;
}

QToolButton *PopupWindow::playSourceButton()
{
    return ui->playSourceButton;
}

QToolButton *PopupWindow::stopSourceButton()
{
    return ui->stopSourceButton;
}

QToolButton *PopupWindow::copySourceButton()
{
    return ui->copySourceButton;
}

QToolButton *PopupWindow::autoTranslationButton()
{
    return ui->autoTranslationButton;
}

QToolButton *PopupWindow::playTranslationButton()
{
    return ui->playTranslationButton;
}

QToolButton *PopupWindow::stopTranslationButton()
{
    return ui->stopTranslationButton;
}

QToolButton *PopupWindow::copyTranslationButton()
{
    return ui->copyTranslationButton;
}

QToolButton *PopupWindow::copyAllTranslationButton()
{
    return ui->copyAllTranslationButton;
}

QButtonGroup *PopupWindow::sourceButtons()
{
    return sourceButtonGroup;
}

QButtonGroup *PopupWindow::translationButtons()
{
    return translationButtonGroup;
}

// Move popup to cursor and prevent appearing outside the screen
void PopupWindow::showEvent(QShowEvent *event)
{
    QPoint position = QCursor::pos(); // Cursor position
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QSize availableSize = QGuiApplication::screenAt(position)->availableSize();
#else
    QSize availableSize = QApplication::desktop()->screenGeometry(position).size();
#endif

    if (availableSize.width() - position.x() - this->geometry().width() < 0) {
        position.rx()-= this->frameGeometry().width();
        if (position.x() < 0)
            position.rx() = 0;
    }
    if (availableSize.height() - position.y() - this->geometry().height() < 0) {
        position.ry()-= this->frameGeometry().height();
        if (position.y() < 0)
            position.ry() = 0;
    }

    move(position);
    QWidget::showEvent(event);
}

void PopupWindow::copyButton(QButtonGroup *group, QAbstractButton *button, int id)
{
    if (button->property("Lang").toInt() != QOnlineTranslator::NoLanguage) {
        group->button(id)->setText(button->text());
        group->button(id)->setProperty("Lang", button->property("Lang"));
        group->button(id)->setIcon(button->icon());
        group->button(id)->setVisible(true);
    } else {
        group->button(id)->setVisible(false);
    }
}

void PopupWindow::copyLanguageButtons(QButtonGroup *existingGroup, QButtonGroup *copyingGroup)
{
    for (int i = 0; i < 4; ++i)
        copyButton(existingGroup, copyingGroup->button(i), i);

    existingGroup->button(copyingGroup->checkedId())->setChecked(true);
}
