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

#include <QPixmap>
#include <QtConcurrent>

#include <tesseract/genericvector.h>

Ocr::Ocr(QObject *parent)
    : QObject(parent)
{
}

QStringList Ocr::availableLanguages() const
{
    QStringList availableLanguages;
    GenericVector<STRING> languages;
    m_tesseract.GetAvailableLanguagesAsVector(&languages);
    availableLanguages.reserve(languages.size());
    for (int i = 0; i < languages.size(); ++i)
        availableLanguages.append(languages[i].string());
    return availableLanguages;
}

QByteArray Ocr::languagesString() const
{
    return QByteArray::fromRawData(m_tesseract.GetInitLanguagesAsString(), qstrlen(m_tesseract.GetInitLanguagesAsString()));
}

bool Ocr::setLanguagesString(const QByteArray &languages, const QByteArray &languagesPath)
{
    // Call even if the specified language is empty to initialize (Tesseract will try to load eng by default)
    if (languagesString() != languages || languages.isEmpty())
        return m_tesseract.Init(languagesPath.isEmpty() ? nullptr : languagesPath.data(), languages.isEmpty() ? nullptr : languages.data(), tesseract::OEM_LSTM_ONLY) == 0;

    // Language are already set
    return true;
}

void Ocr::recognize(const QPixmap &pixmap) 
{
    Q_ASSERT_X(qstrlen(m_tesseract.GetInitLanguagesAsString()) != 0, "recognize", "You should call setLanguage first");

    QtConcurrent::run([this, image = pixmap.toImage()] {
        m_tesseract.SetImage(image.constBits() ,image.width(), image.height(), 4, image.bytesPerLine());
        m_tesseract.SetSourceResolution(70);
        QScopedPointer<char, QScopedPointerArrayDeleter<char>> resultText(m_tesseract.GetUTF8Text());
        emit recognized(resultText.data());
    });
}

QStringList Ocr::availableLanguages(const QString &languagesPath) 
{
    // From the specified directory
    if (!languagesPath.isEmpty())
        return parseLanguageFiles(languagesPath);

    // From the environment variable

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    if (const QString environmentLanguagesPath = qEnvironmentVariable("TESSDATA_PREFIX"); !environmentLanguagesPath.isEmpty())
#else
    if (const QString environmentLanguagesPath = qgetenv("TESSDATA_PREFIX"); !environmentLanguagesPath.isEmpty())
#endif
        return parseLanguageFiles(environmentLanguagesPath);

    // From the default location
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        if (path.isEmpty())
            continue;
        const QStringList languages = parseLanguageFiles(path + QDir::separator() + QStringLiteral("tessdata"));
        if (!languages.isEmpty())
            return languages;
    }

    return {};
}

QStringList Ocr::parseLanguageFiles(const QDir &directory) 
{
    const QFileInfoList files = directory.entryInfoList({QStringLiteral("*.traineddata")}, QDir::Files);
    QStringList languages;
    languages.reserve(files.size());
    for (const QFileInfo &file : files)
        languages.append(file.baseName());

    return languages;
}
