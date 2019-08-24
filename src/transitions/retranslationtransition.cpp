#include "retranslationtransition.h"

#include "qonlinetranslator.h"
#include "langbuttongroup.h"

RetranslationTransition::RetranslationTransition(QOnlineTranslator *translator, LangButtonGroup *group, QState *sourceState) :
    QAbstractTransition(sourceState),
    m_translator(translator),
    m_group(group)
{
}

bool RetranslationTransition::eventTest(QEvent *)
{
    return !m_translator->error() && m_group->isAutoButtonChecked() && m_translator->sourceLanguage() == m_translator->translationLanguage();
}

void RetranslationTransition::onTransition(QEvent *)
{
}
