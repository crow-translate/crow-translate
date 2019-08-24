#ifndef TRANSLATORABORTEDTRANSITION_H
#define TRANSLATORABORTEDTRANSITION_H

#include <QSignalTransition>

class QOnlineTranslator;

class TranslatorAbortedTransition : public QSignalTransition
{
public:
    explicit TranslatorAbortedTransition(QOnlineTranslator *translator, QState *sourceState = nullptr);

protected:
    bool eventTest(QEvent *event) override;

private:
    QOnlineTranslator *m_translator;
};

#endif // TRANSLATORABORTEDTRANSITION_H
