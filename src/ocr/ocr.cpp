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

#include "ocr.h"

#include <tesseract/genericvector.h>

#include <QImage>
#include <QtConcurrent>

Ocr::Ocr(QObject *parent)
    : QObject(parent)
{
}

QStringList Ocr::availableLanguages() const
{
    QStringList availableLanguages;
    GenericVector<STRING> languages;
    m_tesseract.GetAvailableLanguagesAsVector(&languages);
    for (int i = 0; i < languages.size(); ++i)
        availableLanguages.append(languages[i].string());
    return availableLanguages;
}

QByteArray Ocr::language() const
{
    return QByteArray::fromRawData(m_tesseract.GetInitLanguagesAsString(), qstrlen(m_tesseract.GetInitLanguagesAsString()));
}

bool Ocr::setLanguage(const QByteArray &language) 
{
    return m_tesseract.Init(nullptr, language, tesseract::OEM_LSTM_ONLY, nullptr, 0, nullptr, nullptr, true) == 0;
}

void Ocr::recognize(const QImage &image) 
{
    Q_ASSERT_X(qstrlen(m_tesseract.GetInitLanguagesAsString()) != 0, "recognize", "You should call setLanguage first");

    QtConcurrent::run([this, image] {
        m_tesseract.SetImage(image.constBits() ,image.width(), image.height(), 4, image.bytesPerLine());
        m_tesseract.SetSourceResolution(70);
        QScopedPointer<char, QScopedPointerArrayDeleter<char>> resultText(m_tesseract.GetUTF8Text());
        emit recognized(resultText.data());
    });
}
