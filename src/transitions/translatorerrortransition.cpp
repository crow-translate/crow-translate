#include "translatorerrortransition.h"
#include "qonlinetranslator.h"

TranslatorErrorTransition::TranslatorErrorTransition(QOnlineTranslator *translator, QState *sourceState) :
    QAbstractTransition(sourceState),
    m_translator(translator)
{
}

bool TranslatorErrorTransition::eventTest(QEvent *)
{
    return m_translator->error();
}

void TranslatorErrorTransition::onTransition(QEvent *)
{
}
