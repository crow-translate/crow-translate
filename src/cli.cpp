#include "cli.h"
#include "singleapplication.h"

#include <QCommandLineParser>
#include <QFile>
#include <QRegularExpression>
#include <QMediaPlayer>
#include <QMediaPlaylist>

CLI::CLI()
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

void CLI::parseArguments(QCoreApplication &app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("A simple and lightweight translator that allows to translate and say text using the Google Translate API and much more.");
    parser.addPositionalArgument("text", "Text to translate. By default, the translation will be done to the system language.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"c", "codes"}, "Show all language codes."));
    parser.addOption(QCommandLineOption({"a", "audio-only"}, "Print text only for speaking when using --speak-translation or --speak-source."));
    parser.addOption(QCommandLineOption({"s", "source"}, "Specify the source language (by default, engine will try to determine the language on its own).", "code", "auto"));
    parser.addOption(QCommandLineOption({"t", "translation"}, " 	Specify the translation language(s), joined by '+' (by default, the system language is used).", "code", "auto"));
    parser.addOption(QCommandLineOption({"l", "locale"}, "Specify the translator language (by default, the system language is used).", "code", "auto"));
    parser.addOption(QCommandLineOption({"e", "engine"}, "Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default.", "engine", "google"));
    parser.addOption(QCommandLineOption({"p", "speak-translation"}, "Speak the translation."));
    parser.addOption(QCommandLineOption({"u", "speak-source"}, "Speak the source."));
    parser.addOption(QCommandLineOption({"f", "file"}, "Read source text from files. Arguments will be interpreted as file paths."));
    parser.addOption(QCommandLineOption({"i", "stdin"}, "Add stdin data to source text."));
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
            m_text += readFilesFromStdin();

        m_text += readFilesFromArguments(parser.positionalArguments());
    } else {
        if (parser.isSet("stdin"))
            m_text += QTextStream(stdin).readAll();

        m_text += parser.positionalArguments().join(" ");
    }

    if (m_text.endsWith("\n"))
        m_text.chop(1);

    if (m_text.isEmpty()) {
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

int CLI::exec()
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

int CLI::printLangCodes()
{
    QTextStream out(stdout);

    for (int language = QOnlineTranslator::Auto; language != QOnlineTranslator::Zulu; ++language) {
        out << QOnlineTranslator::languageString(static_cast<QOnlineTranslator::Language>(language));
        out << " - ";
        out << QOnlineTranslator::languageCode(static_cast<QOnlineTranslator::Language>(language)) << endl;
    }

    return 0;
}

int CLI::audioOnly()
{
    QTextStream out(stdout);

    // Speak source
    if (m_speakSource) {
        out << "Source text:" << endl;
        out << m_text << endl;

        if (!speak(m_text, m_engine, m_sourceLang))
            return 1;
    }

    // Speak translation in all languages
    if (m_speakTranslation) {
        foreach (QOnlineTranslator::Language translationLang, m_translationLangs) {
            // Speak into each target language
            m_translator->translate(m_text, m_engine, translationLang, m_sourceLang, m_uiLang);

            out << "Translation into " << m_translator->translationLanguageString() << ":" << endl;
            out << m_translator->translation() << endl;

            if (!speak(m_translator->translation(), m_engine, m_translator->translationLanguage()))
                return 1;
        }
    }

    return 0;
}

int CLI::translation()
{
    QTextStream out(stdout);

    // Translate into each target language
    for (int i = 0; i < m_translationLangs.size(); ++i) {
        m_translator->translate(m_text, m_engine, m_translationLangs.at(i), m_sourceLang, m_uiLang);
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

bool CLI::speak(const QString &text, QOnlineTranslator::Engine engine, QOnlineTranslator::Language language)
{
    m_playlist->addMedia(m_translator->media(text, engine, language));
    if (m_translator->error() != QOnlineTranslator::NoError) {
        qCritical() << m_translator->errorString() << endl;
        return false;
    }

    m_player->setPlaylist(m_playlist);
    m_player->play();
    m_waitUntilPlayedLoop->exec();

    return true;
}


QByteArray CLI::readFilesFromStdin()
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

QByteArray CLI::readFilesFromArguments(const QStringList &arguments)
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
