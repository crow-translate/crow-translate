/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LANGUAGEBUTTONSWIDGET_H
#define LANGUAGEBUTTONSWIDGET_H

#include "settings/appsettings.h"

#include <QWidget>

class QAbstractButton;
class QButtonGroup;

namespace Ui
{
class LanguageButtonsWidget;
}

class LanguageButtonsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LanguageButtonsWidget(QWidget *parent = nullptr);
    ~LanguageButtonsWidget() override;

    QVector<QOnlineTranslator::Language> languages() const;
    void setLanguages(const QVector<QOnlineTranslator::Language> &languages);

    QOnlineTranslator::Language checkedLanguage() const;
    QOnlineTranslator::Language previousCheckedLanguage() const;
    QOnlineTranslator::Language language(int id) const;
    bool checkLanguage(QOnlineTranslator::Language lang);
    void setLanguageFormat(AppSettings::LanguageFormat languageFormat);

    int checkedId() const;
    bool isAutoButtonChecked() const;
    void retranslate();

    static QIcon countryIcon(QOnlineTranslator::Language lang);
    static void swapCurrentLanguages(LanguageButtonsWidget *first, LanguageButtonsWidget *second);

    static constexpr int autoButtonId()
    {
        return s_autoButtonId;
    }

signals:
    void buttonChecked(int id);
    void languageAdded(QOnlineTranslator::Language lang);
    void languagesChanged(const QVector<QOnlineTranslator::Language> &languages);
    void autoLanguageChanged(QOnlineTranslator::Language lang);

public slots:
    void checkAutoButton();
    void checkButton(int id);
    void addLanguage(QOnlineTranslator::Language lang);
    void setAutoLanguage(QOnlineTranslator::Language lang);

private slots:
    void editLanguages();
    void savePreviousToggledButton(int id, bool checked);
    void checkAvailableScreenWidth();
    void minimizeWindowWidth();

private:
    void changeEvent(QEvent *event) override;

    void setWindowWidthCheckEnabled(bool enable) const;
    void addOrCheckLanguage(QOnlineTranslator::Language lang);
    void addButton(QOnlineTranslator::Language lang);
    void setButtonLanguage(QAbstractButton *button, QOnlineTranslator::Language lang);

    QString languageString(QOnlineTranslator::Language lang);

    static constexpr int s_autoButtonId = -2; // -1 is reserved by QButtonGroup

    Ui::LanguageButtonsWidget *ui;
    QButtonGroup *m_buttonGroup;

    QVector<QOnlineTranslator::Language> m_languages;
    QOnlineTranslator::Language m_autoLang = QOnlineTranslator::Auto;
    AppSettings::LanguageFormat m_languageFormat = AppSettings::FullName;
    int m_previousCheckedId = 0;
};

#endif // LANGUAGEBUTTONSWIDGET_H
