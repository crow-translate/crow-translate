#include "translatorabortedtransition.h"
#include "qonlinetranslator.h"

TranslatorAbortedTransition::TranslatorAbortedTransition(QOnlineTranslator *translator, QState *sourceState) :
    QSignalTransition(translator, &QOnlineTranslator::finished, sourceState),
    m_translator(translator)
{
}

bool TranslatorAbortedTransition::eventTest(QEvent *event)
{
    return !m_translator->isRunning() || QSignalTransition::eventTest(event);
}
