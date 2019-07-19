#ifndef TRANSLATIONEDIT_H
#define TRANSLATIONEDIT_H

#include "qonlinetranslator.h"

#include <QTextEdit>

class TranslationEdit : public QTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(TranslationEdit)

public:
    explicit TranslationEdit(QWidget *parent = nullptr);

    bool parseTranslationData(QOnlineTranslator *translator);
    QString translation() const;

signals:
    void translationDataParsed(const QString &text);

private:
    QString m_translation;
};

#endif // TRANSLATIONEDIT_H
