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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ocr.h"

#include "settings/appsettings.h"

#include <QPixmap>
#include <QtConcurrent>

#if TESSERACT_MAJOR_VERSION < 5
#include <tesseract/genericvector.h>
#endif

Ocr::Ocr(QObject *parent)
    : QObject(parent)
{
    // For the ability to cancel task
    m_monitor.cancel_this = &m_future;
    m_monitor.cancel = [](void *cancel_this, int) {
        return reinterpret_cast<QFuture<void> *>(cancel_this)->isCanceled();
    };
}

QStringList Ocr::availableLanguages() const
{
    QStringList availableLanguages;
#if TESSERACT_MAJOR_VERSION < 5
    GenericVector<STRING> languages;
#else
    std::vector<std::string> languages;
#endif
    m_tesseract.GetAvailableLanguagesAsVector(&languages);
    availableLanguages.reserve(languages.size());
    for (int i = 0; i < languages.size(); ++i) {
#if TESSERACT_MAJOR_VERSION < 5
        availableLanguages.append(languages[i].string());
#else
        availableLanguages.append(QString::fromStdString(languages[i]));
#endif
    }

    return availableLanguages;
}

QByteArray Ocr::languagesString() const
{
    return QByteArray::fromRawData(m_tesseract.GetInitLanguagesAsString(), qstrlen(m_tesseract.GetInitLanguagesAsString()));
}

void Ocr::setParameters(const QMap<QString, QVariant> &parameters)
{
        for (auto it = parameters.cbegin(); it != parameters.cend(); ++it) {
            if (!m_tesseract.SetVariable(it.key().toLocal8Bit(), it.value().toByteArray()))
                qWarning() << tr("%1 is not the name of a valid tesseract parameter.").arg(it.key());
        }
}

bool Ocr::setLanguagesString(const QByteArray &languages, const QByteArray &languagesPath)
{
    // Call even if the specified language is empty to initialize (Tesseract will try to load eng by default)
    if (languagesString() != languages || languages.isEmpty())
        return m_tesseract.Init(languagesPath.isEmpty() ? nullptr : languagesPath.data(), languages.isEmpty() ? nullptr : languages.data(), tesseract::OEM_LSTM_ONLY) == 0;

    // Language are already set
    return true;
}

void Ocr::recognize(const QPixmap &pixmap, int dpi) 
{
    Q_ASSERT_X(qstrlen(m_tesseract.GetInitLanguagesAsString()) != 0, "recognize", "You should call setLanguagesString first");

    m_future.waitForFinished();
    m_future = QtConcurrent::run([this, dpi, image = pixmap.toImage()] {
        m_tesseract.SetImage(image.constBits(), image.width(), image.height(), image.depth() / 8, image.bytesPerLine());
        m_tesseract.SetSourceResolution(dpi);
        m_tesseract.Recognize(&m_monitor);
        if (m_future.isCanceled()) {
            emit canceled();
            return;
        }

        QScopedPointer<char, QScopedPointerArrayDeleter<char>> resultText(m_tesseract.GetUTF8Text());
        QString recognizedText = resultText.data();
        if (AppSettings().isConvertLineBreaks())
            recognizedText.replace(QRegularExpression(QStringLiteral("(?<!\n)\n(?!\n)")), QStringLiteral(" "));
        emit recognized(recognizedText);
    });
}

void Ocr::cancel() 
{
    m_future.cancel();
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
        QStringList languages = parseLanguageFiles(path + QDir::separator() + QStringLiteral("tessdata"));
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
