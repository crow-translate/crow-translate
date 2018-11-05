#ifndef LANGBUTTONGROUP_H
#define LANGBUTTONGROUP_H

#include <QButtonGroup>

class LangButtonGroup : public QButtonGroup
{
    Q_OBJECT

public:
    LangButtonGroup(QObject *parent = nullptr);
    void addButton(QAbstractButton *button);

signals:
    void buttonChecked(int id);

public slots:
    void checkButton(int id);
};

#endif // LANGBUTTONGROUP_H
