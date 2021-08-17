/*
 *  Copyright © 2018-2021 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "popupwindow.h"
#include "ui_popupwindow.h"

#include "languagebuttonswidget.h"
#include "mainwindow.h"
#include "speakbuttons.h"
#include "translationedit.h"

#include <QCloseEvent>
#include <QMediaPlaylist>
#include <QScreen>
#include <QShortcut>
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#endif

PopupWindow::PopupWindow(MainWindow *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint)
    , ui(new Ui::PopupWindow)
    , m_closeWindowsShortcut(new QShortcut(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // Engine
    ui->engineComboBox->setCurrentIndex(parent->engineCombobox()->currentIndex());
    connect(ui->engineComboBox, qOverload<int>(&QComboBox::currentIndexChanged), parent->engineCombobox(), &QComboBox::setCurrentIndex);

    // Translation edit
    ui->translationEdit->setFont(parent->translationEdit()->font());
    connect(parent->translationEdit(), &TranslationEdit::translationDataParsed, ui->translationEdit, &QTextEdit::setHtml);

    // Player buttons
    ui->sourceSpeakButtons->setMediaPlayer(parent->sourceSpeakButtons()->mediaPlayer());
    ui->translationSpeakButtons->setMediaPlayer(parent->translationSpeakButtons()->mediaPlayer());
    ui->sourceSpeakButtons->setSpeakShortcut(parent->sourceSpeakButtons()->speakShortcut());
    ui->translationSpeakButtons->setSpeakShortcut(parent->translationSpeakButtons()->speakShortcut());
    connect(ui->sourceSpeakButtons, &SpeakButtons::playerMediaRequested, parent->sourceSpeakButtons(), &SpeakButtons::playerMediaRequested);
    connect(ui->translationSpeakButtons, &SpeakButtons::playerMediaRequested, parent->translationSpeakButtons(), &SpeakButtons::playerMediaRequested);

    // Language buttons
    connectLanguageButtons(ui->sourceLanguagesWidget, parent->sourceLanguageButtons());
    connectLanguageButtons(ui->translationLanguagesWidget, parent->translationLanguageButtons());

    // Buttons
    ui->copyTranslationButton->setShortcut(parent->copyTranslationButton()->shortcut());
    connect(ui->copyTranslationButton, &QToolButton::clicked, parent->copyTranslationButton(), &QToolButton::click);
    connect(ui->swapButton, &QToolButton::clicked, parent->swapButton(), &QToolButton::click);
    connect(ui->copySourceButton, &QToolButton::clicked, parent->copySourceButton(), &QToolButton::click);
    connect(ui->copyAllTranslationButton, &QToolButton::clicked, parent->copyAllTranslationButton(), &QToolButton::click);

    // Close window shortcut
    m_closeWindowsShortcut->setKey(parent->closeWindowShortcut());
    connect(m_closeWindowsShortcut, &QShortcut::activated, this, &PopupWindow::close);

    loadSettings();
}

PopupWindow::~PopupWindow()
{
    ui->sourceSpeakButtons->pauseSpeaking();
    ui->translationSpeakButtons->pauseSpeaking();
    delete ui;
}

void PopupWindow::loadSettings()
{
    const AppSettings settings;
    setWindowOpacity(settings.popupOpacity());
    resize(settings.popupWidth(), settings.popupHeight());

    ui->sourceLanguagesWidget->setLanguageFormat(settings.popupLanguageFormat());
    ui->translationLanguagesWidget->setLanguageFormat(settings.popupLanguageFormat());
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

    if (availableSize.width() - position.x() - geometry().width() < 0) {
        position.rx() -= frameGeometry().width();
        if (position.x() < 0)
            position.rx() = 0;
    }
    if (availableSize.height() - position.y() - geometry().height() < 0) {
        position.ry() -= frameGeometry().height();
        if (position.y() < 0)
            position.ry() = 0;
    }

    move(position);
    QWidget::showEvent(event);
}

bool PopupWindow::event(QEvent *event)
{
    // Close window when focus is lost
    if (event->type() == QEvent::WindowDeactivate) {
        // Do not close the window if the language selection menu is active
        if (QApplication::activeModalWidget() == nullptr)
            close();
    }
    return QWidget::event(event);
}

void PopupWindow::connectLanguageButtons(LanguageButtonsWidget *popupButtons, const LanguageButtonsWidget *mainWindowButtons)
{
    popupButtons->setLanguages(mainWindowButtons->languages());
    popupButtons->checkButton(mainWindowButtons->checkedId());
    connect(popupButtons, &LanguageButtonsWidget::buttonChecked, mainWindowButtons, &LanguageButtonsWidget::checkButton);
    connect(popupButtons, &LanguageButtonsWidget::languagesChanged, mainWindowButtons, &LanguageButtonsWidget::setLanguages);
    connect(mainWindowButtons, &LanguageButtonsWidget::buttonChecked, popupButtons, &LanguageButtonsWidget::checkButton);
    connect(mainWindowButtons, &LanguageButtonsWidget::autoLanguageChanged, popupButtons, &LanguageButtonsWidget::setAutoLanguage);
    connect(mainWindowButtons, &LanguageButtonsWidget::languageAdded, popupButtons, &LanguageButtonsWidget::addLanguage);
}
