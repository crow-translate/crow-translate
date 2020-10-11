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

#ifndef OCR_H
#define OCR_H

#include <QObject>

#include <tesseract/baseapi.h>

class QDir;

class Ocr : public QObject
{
    Q_OBJECT

public:
    explicit Ocr(QObject *parent = nullptr);

    QStringList availableLanguages() const;
    QByteArray languagesString() const;
    bool setLanguagesString(const QByteArray &languages, const QByteArray &languagesPath);

    void recognize(const QPixmap &pixmap, int dpi);

    static QStringList availableLanguages(const QString &languagesPath);

signals:
    void recognized(const QString &text);

private:
    static QStringList parseLanguageFiles(const QDir &directory);
    tesseract::TessBaseAPI m_tesseract;
};

#endif // OCR_H
