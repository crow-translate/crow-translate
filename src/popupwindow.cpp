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

PopupWindow::PopupWindow(QMenu *languagesMenu, LangButtonGroup *sourceGroup, LangButtonGroup *translationGroup, QWidget *parent) :
    QWidget(parent, Qt::Popup),
    ui(new Ui::PopupWindow)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    AppSettings settings;
    setWindowOpacity(settings.popupOpacity());
    resize(settings.popupWidth(), settings.popupHeight());

    // Translation button group
    sourceButtonGroup.addButton(ui->autoSourceButton);
    sourceButtonGroup.addButton(ui->firstSourceButton);
    sourceButtonGroup.addButton(ui->secondSourceButton);
    sourceButtonGroup.addButton(ui->thirdSourceButton);
    sourceButtonGroup.loadLanguages(sourceGroup);

    // Source button group
    translationButtonGroup.addButton(ui->autoTranslationButton);
    translationButtonGroup.addButton(ui->firstTranslationButton);
    translationButtonGroup.addButton(ui->secondTranslationButton);
    translationButtonGroup.addButton(ui->thirdTranslationButton);
    translationButtonGroup.loadLanguages(translationGroup);

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
    closeWindowsShortcut.setKey(settings.closeWindowHotkey());
    connect(&closeWindowsShortcut, &QShortcut::activated, this, &PopupWindow::close);

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

QComboBox *PopupWindow::engineCombobox()
{
    return ui->engineComboBox;
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

LangButtonGroup *PopupWindow::sourceButtons()
{
    return &sourceButtonGroup;
}

LangButtonGroup *PopupWindow::translationButtons()
{
    return &translationButtonGroup;
}

// Move popup to cursor and prevent appearing outside the screen
void PopupWindow::showEvent(QShowEvent *event)
{
    QPoint position = QCursor::pos(); // Cursor position
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const QSize availableSize = QGuiApplication::screenAt(position)->availableSize();
#else
    const QSize availableSize = QApplication::desktop()->screenGeometry(position).size();
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
