#include "langbuttongroup.h"

#include <QAbstractButton>

LangButtonGroup::LangButtonGroup(QObject *parent) :
    QButtonGroup (parent)
{
    connect(this, qOverload<int, bool>(&LangButtonGroup::buttonToggled), [&](int id, bool checked) {
        if (checked)
            emit buttonChecked(id);
    });
}

void LangButtonGroup::addButton(QAbstractButton *button)
{
    QButtonGroup::addButton(button, buttons().count());
}

void LangButtonGroup::checkButton(int id)
{
    button(id)->setChecked(true);
}
