#include "languagedetectedtransition.h"
#include "langbuttongroup.h"

LanguageDetectedTransition::LanguageDetectedTransition(LangButtonGroup *group, QState *sourceState) :
    QAbstractTransition(sourceState),
    m_group(group)
{
}

bool LanguageDetectedTransition::eventTest(QEvent *)
{
    return m_group->checkedLanguage() != QOnlineTranslator::Auto;
}

void LanguageDetectedTransition::onTransition(QEvent *)
{
}
