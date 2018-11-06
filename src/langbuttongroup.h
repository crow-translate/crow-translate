#ifndef LANGBUTTONGROUP_H
#define LANGBUTTONGROUP_H

#include <QButtonGroup>

#include "qonlinetranslator.h"

class LangButtonGroup : public QButtonGroup
{
    Q_OBJECT

public:
    LangButtonGroup(QObject *parent = nullptr);

    void addButton(QAbstractButton *button);
    void loadLanguages();
    void loadLanguages(const LangButtonGroup *group);
    void insertLanguage(QOnlineTranslator::Language lang);

    QOnlineTranslator::Language checkedLanguage() const;
    QOnlineTranslator::Language language(int id) const;

    QString name() const;
    void setName(const QString &name);

signals:
    void buttonChecked(int id);
    void languageChanged(int id, QOnlineTranslator::Language lang);

public slots:
    void checkButton(int id);
    void setLanguage(int id, QOnlineTranslator::Language language);

private:
    void setAtributes(int id, QOnlineTranslator::Language lang);

    QString m_name;
};

#endif // LANGBUTTONGROUP_H
