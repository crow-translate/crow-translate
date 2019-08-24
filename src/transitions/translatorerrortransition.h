#ifndef TRANSLATORERRORTRANSITION_H
#define TRANSLATORERRORTRANSITION_H

#include <QAbstractTransition>

class QOnlineTranslator;

class TranslatorErrorTransition : public QAbstractTransition
{
public:
    explicit TranslatorErrorTransition(QOnlineTranslator *translator, QState *sourceState = nullptr);

protected:
    bool eventTest(QEvent *) override;
    void onTransition(QEvent *) override;

private:
    QOnlineTranslator *m_translator;
};

#endif // TRANSLATORERRORTRANSITION_H
