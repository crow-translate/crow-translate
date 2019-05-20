#include "cli.h"
#include "qonlinetts.h"
#include "singleapplication.h"

#include <QCommandLineParser>
#include <QFile>
#include <QRegularExpression>
#include <QMediaPlayer>
#include <QMediaPlaylist>

Cli::Cli(QObject *parent) :
    QObject(parent)
{
    m_translator = new QOnlineTranslator(this);
    m_player = new QMediaPlayer(this);
    m_playlist = new QMediaPlaylist(this);
    m_waitUntilPlayedLoop = new QEventLoop(this);
    connect(m_player, &QMediaPlayer::stateChanged, [&](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState)
            m_waitUntilPlayedLoop->quit();
    });
}

void Cli::parseArguments(QCoreApplication &app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("A simple and lightweight translator that allows to translate and say text using the Google Translate API and much more.");
    parser.addPositionalArgument("text", "Text to translate. By default, the translation will be done to the system language.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"c", "codes"}, "Show all language codes."});
    parser.addOption({{"a", "audio-only"}, "Print text only for speaking when using --speak-translation or --speak-source."});
    parser.addOption({{"s", "source"}, "Specify the source language (by default, engine will try to determine the language on its own).", "code", "auto"});
    parser.addOption({{"t", "translation"}, " 	Specify the translation language(s), joined by '+' (by default, the system language is used).", "code", "auto"});
    parser.addOption({{"l", "locale"}, "Specify the translator language (by default, the system language is used).", "code", "auto"});
    parser.addOption({{"e", "engine"}, "Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default.", "engine", "google"});
    parser.addOption({{"p", "speak-translation"}, "Speak the translation."});
    parser.addOption({{"u", "speak-source"}, "Speak the source."});
    parser.addOption({{"f", "file"}, "Read source text from files. Arguments will be interpreted as file paths."});
    parser.addOption({{"i", "stdin"}, "Add stdin data to source text."});
    parser.process(app);

    // Only show language codes
    if (parser.isSet("codes")) {
        m_mode = PrintLangCodes;
        return;
    }

    // Engine
    if (parser.value("engine") == "google")
        m_engine = QOnlineTranslator::Google;
    else if (parser.value("engine") == "yandex")
        m_engine = QOnlineTranslator::Yandex;
    else if (parser.value("engine") == "bing")
        m_engine = QOnlineTranslator::Bing;
    else {
        qCritical() << "Error: Unknown engine" << endl;
        parser.showHelp();
    }

    // Audio options
    m_speakSource = parser.isSet("speak-source");
    m_speakTranslation = parser.isSet("speak-translation");

    // Translation languages
    m_sourceLang = QOnlineTranslator::language(parser.value("source"));
    m_uiLang = QOnlineTranslator::language(parser.value("locale"));
    foreach (const QString &language, parser.value("translation").split("+"))
        m_translationLangs << QOnlineTranslator::language(language);

    // Source text
    if (parser.isSet("file")) {
        if (parser.isSet("stdin"))
            m_sourceText += readFilesFromStdin();

        m_sourceText += readFilesFromArguments(parser.positionalArguments());
    } else {
        if (parser.isSet("stdin"))
            m_sourceText += QTextStream(stdin).readAll();

        m_sourceText += parser.positionalArguments().join(" ");
    }

    if (m_sourceText.endsWith("\n"))
        m_sourceText.chop(1);

    if (m_sourceText.isEmpty()) {
        qCritical() << "Error: There is no text for translation." << endl;
        parser.showHelp();
    }

    // Audio only mode
    if (parser.isSet("audio-only")) {
        if (!m_speakSource && !m_speakTranslation) {
            qCritical() << "Error: For --audio-only you must specify --speak-source or --speak-translation options." << endl;
            parser.showHelp();
        }

        m_mode = AudioOnly;
    }
}

int Cli::exec()
{
    switch (m_mode) {
    case PrintLangCodes:
        return printLangCodes();
    case AudioOnly:
        return audioOnly();
    default:
        return translation();
    }
}

int Cli::printLangCodes()
{
    QTextStream out(stdout);

    for (int languageIndex = QOnlineTranslator::Auto; languageIndex != QOnlineTranslator::Zulu; ++languageIndex) {
        const auto language = static_cast<QOnlineTranslator::Language>(languageIndex);
        out << QOnlineTranslator::languageString(language) << " - " << QOnlineTranslator::languageCode(language) << endl;
    }

    return 0;
}

int Cli::audioOnly()
{
    QTextStream out(stdout);

    // Speak source
    if (m_speakSource) {
        out << "Source text:" << endl;
        out << m_sourceText << endl;

        if (!speak(m_sourceText, m_engine, m_sourceLang))
            return 1;
    }

    // Speak translation in all languages
    if (m_speakTranslation) {
        foreach (QOnlineTranslator::Language translationLang, m_translationLangs) {
            // Speak into each target language
            m_translator->translate(m_sourceText, m_engine, translationLang, m_sourceLang, m_uiLang);

            out << "Translation into " << m_translator->translationLanguageString() << ":" << endl;
            out << m_translator->translation() << endl;

            if (!speak(m_translator->translation(), m_engine, m_translator->translationLanguage()))
                return 1;
        }
    }

    return 0;
}

int Cli::translation()
{
    QTextStream out(stdout);

    // Translate into each target language
    for (int i = 0; i < m_translationLangs.size(); ++i) {
        m_translator->translate(m_sourceText, m_engine, m_translationLangs.at(i), m_sourceLang, m_uiLang);
        if (m_translator->error()) {
            qCritical() << m_translator->errorString();
            return 1;
        }

        // Show source text and its transliteration only once
        if (i == 0) {
            out << m_translator->source() << endl;
            if (!m_translator->sourceTranslit().isEmpty())
                out << "(" << m_translator->sourceTranslit().replace("\n", ")\n(") << ")" << endl << endl;
            else
                out << endl;
        } else {
            out << endl;
        }

        // Languages
        out << "[ " << m_translator->sourceLanguageString() << " -> ";
        out << m_translator->translationLanguageString() << " ]" << endl << endl ;

        // Translation and its transliteration
        if (!m_translator->translation().isEmpty()) {
            out << m_translator->translation() << endl;
            if (!m_translator->translationTranslit().isEmpty())
                out << "/" << m_translator->translationTranslit().replace("\n", "/\n/") << "/" << endl << endl;
            else
                out << endl;
        }

        // Translation options
        if (!m_translator->translationOptions().isEmpty()) {
            out << m_translator->source() << " - translation options:" << endl;
            foreach (const QOption &option, m_translator->translationOptions()) {
                out << option.typeOfSpeech() << endl;
                for (int i = 0; i < option.count(); ++i) {
                    out << "\t";
                    if (!option.gender(i).isEmpty())
                        out << option.gender(i) << " ";
                    out << option.word(i) << ": ";

                    out << option.translations(i) << endl;
                }
                out << endl;
            }
        }

        // Examples
        if (!m_translator->examples().isEmpty()) {
            out << m_translator->source() << " - examples:" << endl;
            foreach (const QExample &example, m_translator->examples()) {
                out << example.typeOfSpeech() << endl;
                for (int i = 0; i < example.count(); ++ i) {
                    out << "\t" << example.description(i) << endl;
                    out << "\t" << example.example(i) << endl;
                }
            }
        }

        // Speaking
        if (m_speakSource && i == 0) {
            if (!speak(m_translator->source(), m_engine, m_translator->sourceLanguage()))
                return 1;
        }

        if (m_speakTranslation) {
            if (!speak(m_translator->translation(), m_engine, m_translator->translationLanguage()))
                return 1;
        }
    }

    return 0;
}

bool Cli::speak(const QString &text, QOnlineTranslator::Engine engine, QOnlineTranslator::Language language)
{
    QOnlineTts tts;
    tts.generateUrls(text, engine, language);
    m_playlist->addMedia(tts.media());
    if (m_translator->error() != QOnlineTranslator::NoError) {
        qCritical() << m_translator->errorString() << endl;
        return false;
    }

    m_player->setPlaylist(m_playlist);
    m_player->play();
    m_waitUntilPlayedLoop->exec();

    return true;
}


QByteArray Cli::readFilesFromStdin()
{
    QString stdinText = QTextStream(stdin).readAll();
    QByteArray filesData;
    foreach (const QString &filePath, stdinText.split(QRegularExpression("\\s+"), QString::SkipEmptyParts)) {
        QFile file(filePath);
        if (!file.exists()) {
            qCritical() << "Error: File does not exist: " << file.fileName() << endl;
            continue;
        }

        if (!file.open(QFile::ReadOnly)) {
            qCritical() << "Error: Unable to open file: " << file.fileName() << endl;
            continue;
        }

        filesData += file.readAll();
    }

    return filesData;
}

QByteArray Cli::readFilesFromArguments(const QStringList &arguments)
{
    QByteArray filesData;
    foreach (const QString &filePath, arguments) {
        QFile file(filePath);
        if (!file.exists()) {
            qCritical() << "Error: File does not exist: " << file.fileName() << endl;
            continue;
        }

        if (!file.open(QFile::ReadOnly)) {
            qCritical() << "Error: Unable to open file: " << file.fileName() << endl;
            continue;
        }

        filesData += file.readAll();
    }

    return filesData;
}
