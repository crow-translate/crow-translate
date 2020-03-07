/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "popupwindow.h"
#include "ui_popupwindow.h"
#include "langbuttongroup.h"
#include "translationedit.h"
#include "singleapplication.h"
#include "mainwindow.h"
#include "speakbuttons.h"
#include "settings/appsettings.h"

#include <QScreen>
#include <QMediaPlaylist>
#include <QShortcut>
#include <QCloseEvent>
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#endif

PopupWindow::PopupWindow(MainWindow *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , ui(new Ui::PopupWindow)
    , m_closeWindowsShortcut(new QShortcut(this))
    , m_sourceLangButtons(new LangButtonGroup(LangButtonGroup::Source, this))
    , m_translationLangButtons(new LangButtonGroup(LangButtonGroup::Translation, this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(parent->translationEdit(), &TranslationEdit::translationDataParsed, ui->translationEdit, &QTextEdit::setHtml);
    ui->engineComboBox->setCurrentIndex(parent->engineCombobox()->currentIndex());

    // Window settings
    setWindowOpacity(parent->popupOpacity());
    resize(parent->popupSize());
    ui->translationEdit->setFont(parent->translationEdit()->font());

    // Player buttons
    ui->sourceSpeakButtons->setMediaPlayer(parent->sourceSpeakButtons()->mediaPlayer());
    ui->translationSpeakButtons->setMediaPlayer(parent->translationSpeakButtons()->mediaPlayer());
    connect(ui->sourceSpeakButtons, &SpeakButtons::playerMediaRequested, parent->sourceSpeakButtons(), &SpeakButtons::playerMediaRequested);
    connect(ui->translationSpeakButtons, &SpeakButtons::playerMediaRequested, parent->translationSpeakButtons(), &SpeakButtons::playerMediaRequested);

    // Source button group
    m_sourceLangButtons->addButton(ui->autoSourceButton);
    m_sourceLangButtons->addButton(ui->firstSourceButton);
    m_sourceLangButtons->addButton(ui->secondSourceButton);
    m_sourceLangButtons->addButton(ui->thirdSourceButton);
    m_sourceLangButtons->loadLanguages(parent->sourceLangButtons());
    connect(parent->sourceLangButtons(), &LangButtonGroup::buttonChecked, m_sourceLangButtons, &LangButtonGroup::checkButton);
    connect(parent->sourceLangButtons(), &LangButtonGroup::languageChanged, m_sourceLangButtons,  &LangButtonGroup::setLanguage);

    // Translation button group
    m_translationLangButtons->addButton(ui->autoTranslationButton);
    m_translationLangButtons->addButton(ui->firstTranslationButton);
    m_translationLangButtons->addButton(ui->secondTranslationButton);
    m_translationLangButtons->addButton(ui->thirdTranslationButton);
    m_translationLangButtons->loadLanguages(parent->translationLangButtons());
    connect(parent->translationLangButtons(), &LangButtonGroup::buttonChecked, m_translationLangButtons, &LangButtonGroup::checkButton);
    connect(parent->translationLangButtons(), &LangButtonGroup::languageChanged, m_translationLangButtons,  &LangButtonGroup::setLanguage);

    // Shortcuts
    m_closeWindowsShortcut->setKey(parent->closeWindowShortcut()->key());
    connect(m_closeWindowsShortcut, &QShortcut::activated, this, &PopupWindow::close);

    ui->copyTranslationButton->setShortcut(parent->copyTranslationButton()->shortcut());
    ui->sourceSpeakButtons->setSpeakShortcut(parent->sourceSpeakButtons()->speakShortcut());
    ui->translationSpeakButtons->setSpeakShortcut(parent->translationSpeakButtons()->speakShortcut());

    // Connect popup window events
    connect(ui->engineComboBox, qOverload<int>(&QComboBox::currentIndexChanged), parent->engineCombobox(), &QComboBox::setCurrentIndex);
    connect(m_sourceLangButtons, &LangButtonGroup::buttonChecked, parent->sourceLangButtons(), &LangButtonGroup::checkButton);
    connect(m_translationLangButtons, &LangButtonGroup::buttonChecked, parent->translationLangButtons(), &LangButtonGroup::checkButton);
    connect(ui->addSourceLangButton, &QToolButton::clicked, parent->addSourceLangButton(), &QToolButton::click);
    connect(ui->addTranslationLangButton, &QToolButton::clicked, parent->addTranslationLangButton(), &QToolButton::click);
    connect(ui->swapButton, &QToolButton::clicked, parent->swapButton(), &QToolButton::click);
    connect(ui->copySourceButton, &QToolButton::clicked, parent->copySourceButton(), &QToolButton::click);
    connect(ui->copyTranslationButton, &QToolButton::clicked, parent->copyTranslationButton(), &QToolButton::click);
    connect(ui->copyAllTranslationButton, &QToolButton::clicked, parent->copyAllTranslationButton(), &QToolButton::click);
}

PopupWindow::~PopupWindow()
{
    ui->sourceSpeakButtons->pauseSpeaking();
    ui->translationSpeakButtons->pauseSpeaking();
    delete ui;
}

// Move popup to cursor and prevent appearing outside the screen
void PopupWindow::showEvent(QShowEvent *event)
{
    QPoint position = QCursor::pos(); // Cursor position
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const QSize availableSize = SingleApplication::screenAt(position)->availableSize();
#else
    const QSize availableSize = SingleApplication::desktop()->screenGeometry(position).size();
#endif

    if (availableSize.width() - position.x() - geometry().width() < 0) {
        position.rx()-= frameGeometry().width();
        if (position.x() < 0)
            position.rx() = 0;
    }
    if (availableSize.height() - position.y() - geometry().height() < 0) {
        position.ry()-= frameGeometry().height();
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
        if (SingleApplication::activeModalWidget() == nullptr)
            close();
    }
    return QWidget::event(event);
}
