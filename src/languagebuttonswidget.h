#ifndef LANGUAGEBUTTONSWIDGET_H
#define LANGUAGEBUTTONSWIDGET_H

#include "qonlinetranslator.h"

#include <QWidget>

class QAbstractButton;
class QButtonGroup;

namespace Ui {
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
    QOnlineTranslator::Language language(int index) const;

    bool checkLanguage(QOnlineTranslator::Language language);

    bool isAutoButtonChecked() const;
    void retranslate();

    static QIcon countryIcon(QOnlineTranslator::Language language);
    static void swapCurrentLanguages(LanguageButtonsWidget *first, LanguageButtonsWidget *second);

signals:
    void buttonChecked(int index);
    void languageAdded(QOnlineTranslator::Language language);
    void languagesChanged(const QVector<QOnlineTranslator::Language> &languages);
    void autoLanguageChanged(QOnlineTranslator::Language language);

public slots:
    void checkAutoButton();
    void checkButton(int index);
    void addLanguage(QOnlineTranslator::Language language);
    void setAutoLanguage(QOnlineTranslator::Language language);

private slots:
    void editLanguages();
    void savePreviousToggledButton(int index, bool checked);

private:
    void addOrCheckLanguage(QOnlineTranslator::Language language);
    void addButton(QOnlineTranslator::Language language);
    void setButtonLanguage(QAbstractButton *button, QOnlineTranslator::Language language);

    Ui::LanguageButtonsWidget *ui;
    QButtonGroup *m_buttonGroup;

    QVector<QOnlineTranslator::Language> m_languages;
    QOnlineTranslator::Language m_autoLanguage = QOnlineTranslator::Auto;
    int m_previousCheckedId = 0;

    static constexpr int s_autoButtonIndex = -2; // -1 is reserved by QButtonGroup
};

#endif // LANGUAGEBUTTONSWIDGET_H
