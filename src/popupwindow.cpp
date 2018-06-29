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
#include <QSettings>
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#endif

#include "ui_popupwindow.h"
#include "mainwindow.h"

PopupWindow::PopupWindow(QMenu *languagesMenu, QButtonGroup *sourceGroup, QButtonGroup *translationGroup, QWidget *parent) :
    QWidget(parent, Qt::Popup),
    ui(new Ui::PopupWindow),
    sourceButtonGroup (new QButtonGroup(this)),
    translationButtonGroup (new QButtonGroup(this))
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    QSettings settings;
    PopupWindow::setWindowOpacity(settings.value("PopupOpacity", 0.8).toDouble());

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
    ui->firstSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle)));
    ui->secondSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle)));
    ui->thirdSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle)));
    ui->firstTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle)));
    ui->secondTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle)));
    ui->thirdTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("LanguagesStyle", Qt::ToolButtonFollowStyle)));

    // Control buttons style
    ui->playSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));
    ui->stopSourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));
    ui->copySourceButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));
    ui->playTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));
    ui->stopTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));
    ui->copyTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));
    ui->copyAllTranslationButton->setToolButtonStyle(qvariant_cast<Qt::ToolButtonStyle>(settings.value("ControlsStyle", Qt::ToolButtonFollowStyle)));

    // Shortcuts
    ui->playSourceButton->setShortcut(settings.value("Hotkeys/PlaySource", "Ctrl+S").toString());
    ui->playTranslationButton->setShortcut(settings.value("Hotkeys/PlayTranslation", "Ctrl+Shift+S").toString());
    ui->copyTranslationButton->setShortcut(settings.value("Hotkeys/CopyTranslation", "Ctrl+Shift+C").toString());

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

void PopupWindow::loadSourceButton(QAbstractButton *button, const int &id)
{
    sourceButtonGroup->button(id)->setText(button->text());
    sourceButtonGroup->button(id)->setToolTip(button->toolTip());
    sourceButtonGroup->button(id)->setIcon(button->icon());
    sourceButtonGroup->button(id)->setVisible(true);
}

void PopupWindow::loadTranslationButton(QAbstractButton *button, const int &id)
{
    translationButtonGroup->button(id)->setText(button->text());
    translationButtonGroup->button(id)->setToolTip(button->toolTip());
    translationButtonGroup->button(id)->setIcon(button->icon());
    translationButtonGroup->button(id)->setVisible(true);
}

void PopupWindow::checkSourceButton(const int &id, const bool &checked)
{
    if (checked)
        sourceButtonGroup->button(id)->setChecked(true);
}

void PopupWindow::checkTranslationButton(const int &id, const bool &checked)
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

// Move popup to cursor and prevent window from appearing outside the screen
void PopupWindow::showEvent(QShowEvent *event)
{
    QPoint position = QCursor::pos(); // Cursor position
    QSettings settings;
    QSize windowSize = settings.value("PopupSize", QSize(350, 300)).toSize();
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QSize availableSize = QGuiApplication::screenAt(position)->availableSize();
#else
    QSize availableSize = QApplication::desktop()->screenGeometry(position).size();
#endif

    // Check if the size of the pop-up window is more than the allowed size
    if (windowSize.width() > availableSize.width() / 2 || windowSize.height() > availableSize.height() / 2) {
        windowSize.rwidth() = windowSize.width() / 2;
        windowSize.rheight() = windowSize.height() / 2;
        settings.setValue("PopupSize", windowSize);
    }
    resize(windowSize);

    if (availableSize.width() - position.x() - this->geometry().width() < 0)
        position.rx()-= this->frameGeometry().width();
    if (availableSize.height() - position.y() - this->geometry().height() < 0)
        position.ry()-= this->frameGeometry().height();

    PopupWindow::move(position);
    QWidget::showEvent(event);
}

void PopupWindow::copyLanguageButtons(QButtonGroup *existingGroup, QButtonGroup *copyingGroup)
{
    for (auto i = 0; i < 4; i++) {
        if (copyingGroup->button(i)->text() != "") {
            existingGroup->button(i)->setText(copyingGroup->button(i)->text());
            existingGroup->button(i)->setToolTip(copyingGroup->button(i)->toolTip());
            existingGroup->button(i)->setIcon(copyingGroup->button(i)->icon());
        }
        else
            existingGroup->button(i)->setVisible(false);
    }
    existingGroup->button(copyingGroup->checkedId())->setChecked(true);
}
