#include "textemptytransition.h"
#include "sourcetextedit.h"

TextEmptyTransition::TextEmptyTransition(SourceTextEdit *edit, QState *sourceState) :
    QAbstractTransition(sourceState),
    m_edit(edit)
{
}

bool TextEmptyTransition::eventTest(QEvent *)
{
    return m_edit->toPlainText().isEmpty();
}

void TextEmptyTransition::onTransition(QEvent *)
{
}
