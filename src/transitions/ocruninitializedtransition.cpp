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

#include "ocruninitializedtransition.h"

#include "mainwindow.h"
#include "ocr/ocr.h"

#include <QMessageBox>

OcrUninitializedTransition::OcrUninitializedTransition(MainWindow *mainWindow, QState *sourceState)
    : QAbstractTransition(sourceState)
    , m_mainWindow(mainWindow)
{
}

bool OcrUninitializedTransition::eventTest(QEvent *)
{
    return m_mainWindow->ocr()->languagesString().isEmpty();
}

void OcrUninitializedTransition::onTransition(QEvent *)
{
    QMessageBox::critical(m_mainWindow, Ocr::tr("OCR languages are not loaded"), Ocr::tr("You should set at least one OCR language in the application settings"));
}
