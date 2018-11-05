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
    void loadLanguageButtons(QOnlineTranslator *translator);
    QOnlineTranslator::Language checkedLang();
    QOnlineTranslator::Language lang(int id);

    QString name() const;
    void setName(const QString &name);

signals:
    void buttonChecked(int id);

public slots:
    void checkButton(int id);

private:
    QString m_name;
};

#endif // LANGBUTTONGROUP_H
