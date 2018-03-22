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

#include "qonlinetranslator.h"
#include "ui_popupwindow.h"
#include "mainwindow.h"

PopupWindow::PopupWindow(QMenu *languagesMenu, QString text, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::PopupWindow),
    sourceButtonGroup (new LanguageButtonsGroup(this, "Input")),
    translationButtonGroup (new LanguageButtonsGroup(this, "Output"))
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
    ui->autoLanguageSourceButton->setMenu(languagesMenu);
    ui->autoLanguageTranslationButton->setMenu(languagesMenu);

    // Add all language buttons to button groups
    sourceButtonGroup->addButton(ui->autoLanguageSourceButton, 0);
    sourceButtonGroup->addButton(ui->languageSourceButton1, 1);
    sourceButtonGroup->addButton(ui->languageSourceButton2, 2);
    sourceButtonGroup->addButton(ui->languageSourceButton3, 3);
    translationButtonGroup->addButton(ui->autoLanguageTranslationButton, 0);
    translationButtonGroup->addButton(ui->languageTranslationButton1, 1);
    translationButtonGroup->addButton(ui->languageTranslationButton2, 2);
    translationButtonGroup->addButton(ui->languageTranslationButton3, 3);

    sourceButtonGroup->loadSettings();
    translationButtonGroup->loadSettings();

    // Translate text automatically when language buttons released
    connect(sourceButtonGroup, static_cast<void (LanguageButtonsGroup::*)(int)>(&LanguageButtonsGroup::buttonReleased), this, &PopupWindow::translateText);
    connect(translationButtonGroup, static_cast<void (LanguageButtonsGroup::*)(int)>(&LanguageButtonsGroup::buttonReleased), this, &PopupWindow::translateText);

    m_selectedText = text;
    translateText();
}

PopupWindow::~PopupWindow()
{
    emit popupClosed(ui->outputEdit->toHtml());
    delete ui;
}

void PopupWindow::on_autoLanguageSourceButton_triggered(QAction *language)
{
    sourceButtonGroup->insertLanguage(language->text());
    translateText();
}

void PopupWindow::on_autoLanguageTranslationButton_triggered(QAction *language)
{
    translationButtonGroup->insertLanguage(language->text());
    translateText();
}

void PopupWindow::on_speakButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "")
        QOnlineTranslator::say(ui->outputEdit->toPlainText(), translationButtonGroup->checkedButton()->toolTip());
    else
        qDebug() << tr("Text field is empty");
}

void PopupWindow::on_copyButton_clicked()
{
    if (ui->outputEdit->toPlainText() != "")
        QApplication::clipboard()->setText(ui->outputEdit->toPlainText());
    else
        qDebug() << tr("Text field is empty");
}

void PopupWindow::on_swapButton_clicked()
{
    LanguageButtonsGroup::swapChecked(sourceButtonGroup, translationButtonGroup);
    translateText();
}

void PopupWindow::translateText()
{
    QSettings settings;
    QString sourcelanguage = sourceButtonGroup->checkedButton()->toolTip();
    QString translationlanguage = translationButtonGroup->checkedButton()->toolTip();
    QString translatorlanguage = settings.value("Language", "auto").toString();
    QOnlineTranslator onlineTranslator(m_selectedText, translationlanguage, sourcelanguage, translatorlanguage);

    // Show translation and transcription
    ui->outputEdit->setText(onlineTranslator.text());
    if (onlineTranslator.translationTranscription() != "")
        ui->outputEdit->append("<font color=\"grey\"><i>/" + onlineTranslator.translationTranscription() + "/</i></font>");
    if (onlineTranslator.sourceTranscription() != "")
        ui->outputEdit->append("<font color=\"grey\"><i><b>(" + onlineTranslator.sourceTranscription() + ")</b></i></font>");
    ui->outputEdit->append("");

    // Show translation options
    foreach (auto translationOptions, onlineTranslator.options()) {
        ui->outputEdit->append("<i>" + translationOptions.first + "</i>");
        foreach (QString wordsList, translationOptions.second) {
            wordsList.prepend("&nbsp;&nbsp;<b>");
            wordsList.insert(wordsList.indexOf(":") + 1, "</b>");
            ui->outputEdit->append(wordsList);
        }
        ui->outputEdit->append("");
    }

    ui->outputEdit->moveCursor(QTextCursor::Start);
}
