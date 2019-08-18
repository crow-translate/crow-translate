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
        for (auto it = translator->translationOptions().cbegin(); it != translator->translationOptions().cend(); ++it) {
            append("<b>" + it.key() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            textCursor().setBlockFormat(indent);

            for (const auto &[word, gender, translations] : it.value()) {
                // Show word gender
                QString wordLine;
                if (!gender.isEmpty())
                    wordLine.append("<i>" + gender + "</i> ");

                // Show Word
                wordLine.append(word);

                // Show word meaning
                if (!translations.isEmpty()) {
                    wordLine.append(": ");
                    wordLine.append("<font color=\"grey\"><i>");
                    wordLine.append(translations.join(", "));
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
        for (auto it = translator->examples().cbegin(); it != translator->examples().cend(); ++it) {
            append("<b>" + it.key() + "</b>");
            QTextBlockFormat indent;
            indent.setTextIndent(20);
            textCursor().setBlockFormat(indent);
            for (const auto &[example, description]: it.value()) {
                append(description);
                append("<font color=\"grey\"><i>" + example + "</i></font>");
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
