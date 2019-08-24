#ifndef LANGUAGEDETECTEDTRANSITION_H
#define LANGUAGEDETECTEDTRANSITION_H

#include <QAbstractTransition>

class LangButtonGroup;

class LanguageDetectedTransition : public QAbstractTransition
{
public:
    explicit LanguageDetectedTransition(LangButtonGroup *group, QState *sourceState = nullptr);

protected:
    bool eventTest(QEvent *) override;
    void onTransition(QEvent *) override;

private:
    LangButtonGroup *m_group;
};

#endif // LANGUAGEDETECTEDTRANSITION_H
