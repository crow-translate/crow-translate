#include "translationedit.h"

TranslationEdit::TranslationEdit(QWidget *parent) :
    QTextEdit(parent)
{
}

bool TranslationEdit::parseTranslationData(QOnlineTranslator *translator)
{
    // Check for error
    if (translator->error()) {
        m_translation.clear();
        setHtml(translator->errorString());
        emit translationDataParsed(translator->errorString());
        return false;
    }

    // Translation
    m_translation = translator->translation();
    setHtml(m_translation.toHtmlEscaped().replace("\n", "<br>"));

    // Translit
    if (!translator->translationTranslit().isEmpty())
        append("<font color=\"grey\"><i>/" + translator->translationTranslit().replace("\n", "/<br>/") + "/</i></font>");
    if (!translator->sourceTranslit().isEmpty())
        append("<font color=\"grey\"><i><b>(" + translator->sourceTranslit().replace("\n", "/<br>/") + ")</b></i></font>");

    // Transcription
    if (!translator->sourceTranscription().isEmpty())
        append("<font color=\"grey\">[" + translator->sourceTranscription() + "]</font>");

    append(""); // Add new line before translation options

    // Translation options
    if (!translator->translationOptions().isEmpty()) {
        append("<font color=\"grey\"><i>" + translator->source() + "</i> – " + tr("translation options:") + "</font>");

        // Print words for each type of speech
        for (const QString &typeOfSpeech : translator->translationOptions().keys()) {
            append("<b>" + typeOfSpeech + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            textCursor().setBlockFormat(indent);

            for (const QOption &option : translator->translationOptions().value(typeOfSpeech)) {
                // Show word gender
                QString wordLine;
                if (!option.gender().isEmpty())
                    wordLine.append("<i>" + option.gender() + "</i> ");

                // Show Word
                wordLine.append(option.word());

                // Show word meaning
                if (!option.translations().isEmpty()) {
                    wordLine.append(": ");
                    wordLine.append("<font color=\"grey\"><i>");
                    wordLine.append(option.translations().join(", "));
                    wordLine.append("</i></font>");
                }

                // Add generated line to edit
                append(wordLine);
            }

            indent.setTextIndent(0);
            textCursor().setBlockFormat(indent);
            append(""); // Add a new line before the next type of speech
        }
    }

    // Examples
    if (!translator->examples().isEmpty()) {
        append("<font color=\"grey\"><i>" + translator->source() + "</i> – " + tr("examples:") + "</font>");
        for (const QString &typeOfSpeech : translator->examples().keys()) {
            append("<b>" + typeOfSpeech + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            textCursor().setBlockFormat(indent);
            for (const QExample &example : translator->examples().value(typeOfSpeech)) {
                append(example.description());
                append("<font color=\"grey\"><i>" + example.example() + "</i></font>");
                append("");
            }
            indent.setTextIndent(0);
            textCursor().setBlockFormat(indent);
        }
    }

    moveCursor(QTextCursor::Start);
    emit translationDataParsed(toHtml());
    return true;
}

QString TranslationEdit::translation() const
{
    return m_translation;
}
