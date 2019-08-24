#ifndef TEXTEMPTYTRANSITION_H
#define TEXTEMPTYTRANSITION_H

#include <QAbstractTransition>

class SourceTextEdit;

class TextEmptyTransition : public QAbstractTransition
{
public:
    explicit TextEmptyTransition(SourceTextEdit *edit, QState *sourceState = nullptr);

protected:
    bool eventTest(QEvent *) override;
    void onTransition(QEvent *) override;

private:
    SourceTextEdit *m_edit;
};

#endif // TEXTEMPTYTRANSITION_H
