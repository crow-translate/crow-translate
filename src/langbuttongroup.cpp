#include "langbuttongroup.h"

#include <QAbstractButton>

#include "appsettings.h"

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

void LangButtonGroup::loadLanguageButtons(QOnlineTranslator *translator)
{
    // Load buttons text and tooltip
    AppSettings settings;
    for (int i = 1; i < buttons().size(); ++i) {
        auto language = settings.buttonLanguage(this, i);

        // Check if the code is set
        if (language == QOnlineTranslator::NoLanguage) {
            button(i)->setVisible(false);
        } else {
            button(i)->setIcon(QIcon(":/icons/flags/" + QOnlineTranslator::languageCode(language) + ".svg"));
            button(i)->setText(translator->languageString(language));
            button(i)->setProperty("Lang", language);
            button(i)->setVisible(true);
        }
    }

    // Load checked button
    button(settings.checkedButton(this))->setChecked(true);
}

QOnlineTranslator::Language LangButtonGroup::checkedLang()
{
    return checkedButton()->property("Lang").value<QOnlineTranslator::Language>();
}

QOnlineTranslator::Language LangButtonGroup::lang(int id)
{
    return button(id)->property("Lang").value<QOnlineTranslator::Language>();
}

QString LangButtonGroup::name() const
{
    return m_name;
}

void LangButtonGroup::setName(const QString &name)
{
    m_name = name;
}

void LangButtonGroup::checkButton(int id)
{
    button(id)->setChecked(true);
}
