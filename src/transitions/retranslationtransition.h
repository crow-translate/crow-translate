#ifndef RETRANSLATIONTRANSITION_H
#define RETRANSLATIONTRANSITION_H

#include <QAbstractTransition>

class QOnlineTranslator;
class LangButtonGroup;

class RetranslationTransition : public QAbstractTransition
{
public:
    RetranslationTransition(QOnlineTranslator *translator, LangButtonGroup *group, QState *sourceState = nullptr);

protected:
    bool eventTest(QEvent *) override;
    void onTransition(QEvent *) override;

private:
    QOnlineTranslator *m_translator;
    LangButtonGroup *m_group;
};

#endif // RETRANSLATIONTRANSITION_H
