/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mainwindow.h"
#include "appsettings.h"
#include "singleapplication.h"

#include <QCommandLineParser>
#include <QFile>
#include <QRegularExpression>

bool speak(QOnlineTranslator &translator, const QString &text, QOnlineTranslator::Engine engine, QOnlineTranslator::Language language);

int main(int argc, char *argv[])
{
    // Launch GUI if there are no arguments
    if (argc == 1) {
        SingleApplication app(argc, argv);
        SingleApplication::setApplicationName("Crow Translate");
        SingleApplication::setOrganizationName("crow");
        SingleApplication::setApplicationVersion("2.1.0");

#if defined(Q_OS_WIN)
        QIcon::setThemeName("Papirus");
#endif

        AppSettings settings;
        settings.setupLocale();
        MainWindow window;
        if (!settings.isStartMinimized())
            window.show();
        return SingleApplication::exec();
    }

    // Command line interface
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Crow Translate");
    QCoreApplication::setApplicationVersion("2.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("A simple and lightweight translator that allows to translate and say text using the Google Translate API and much more.");
    parser.addPositionalArgument("text", "Text to translate. By default, the translation will be done to the system language.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"c", "codes"}, "Show all language codes."));
    parser.addOption(QCommandLineOption({"s", "source"}, "Specify the source language (by default, engine will try to determine the language on its own).", "code", "auto"));
    parser.addOption(QCommandLineOption({"t", "translation"}, " 	Specify the translation language(s), joined by '+' (by default, the system language is used).", "code", "auto"));
    parser.addOption(QCommandLineOption({"l", "locale"}, "Specify the translator language (by default, the system language is used).", "code", "auto"));
    parser.addOption(QCommandLineOption({"e", "engine"}, "Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default.", "engine", "google"));
    parser.addOption(QCommandLineOption({"p", "speak-translation"}, "Speak the translation."));
    parser.addOption(QCommandLineOption({"u", "speak-source"}, "Speak the source."));
    parser.addOption(QCommandLineOption({"a", "audio-only"}, "Print text only for speaking when using --speak-translation or --speak-source."));
    parser.addOption(QCommandLineOption({"f", "file"}, "Read source text from files. Arguments will be interpreted as file paths."));
    parser.addOption(QCommandLineOption({"i", "stdin"}, "Add stdin data to source text."));
    parser.process(app);

    QOnlineTranslator translator;
    QTextStream out(stdout);

    // Show all language codes
    if (parser.isSet("codes")) {
        for (int language = QOnlineTranslator::Auto; language != QOnlineTranslator::Zulu; ++language) {
            out << QOnlineTranslator::languageString(static_cast<QOnlineTranslator::Language>(language));
            out << " - ";
            out << QOnlineTranslator::languageCode(static_cast<QOnlineTranslator::Language>(language)) << endl;
        }
        return 0;
    }

    // Parse engine
    QOnlineTranslator::Engine engine;
    if (parser.value("engine") == "google")
        engine = QOnlineTranslator::Google;
    else if (parser.value("engine") == "yandex")
        engine = QOnlineTranslator::Yandex;
    else if (parser.value("engine") == "bing")
        engine = QOnlineTranslator::Bing;
    else {
        out << "Error: Unknown engine" << endl;
        parser.showHelp();
    }

    // Parse languages
    auto sourceLang = QOnlineTranslator::language(parser.value("source"));
    auto uiLang = QOnlineTranslator::language(parser.value("locale"));
    QVector<QOnlineTranslator::Language> translationLangs;
    foreach (const QString &language, parser.value("translation").split("+")) {
        translationLangs << QOnlineTranslator::language(language);
    }

    // Read text for translation
    QString text;
    if (parser.isSet("file")) {
        // Read from stdin first
        if (parser.isSet("stdin")) {
            QString stdinText = QTextStream(stdin).readAll();
            foreach (const QString &filePath,  stdinText.split(QRegularExpression("\\s+"), QString::SkipEmptyParts)) {
                QFile file(filePath);
                if (file.exists()) {
                    if (file.open(QFile::ReadOnly))
                        text += file.readAll();
                    else
                        out << "Error: Unable to open file: " << file.fileName() << endl;
                } else {
                    out << "Error: File does not exist: " << file.fileName() << endl;
                }
            }
        }

        // Read from arguments
        foreach (const QString &filePath,  parser.positionalArguments()) {
            QFile file(filePath);
            if (file.exists()) {
                if (file.open(QFile::ReadOnly))
                    text += file.readAll();
                else
                    out << "Error: Unable to open file: " << file.fileName() << endl;
            } else {
                out << "Error: File does not exist: " << file.fileName() << endl;
            }
        }
    } else {
        if (parser.isSet("stdin"))
            text += QTextStream(stdin).readAll();
        text += parser.positionalArguments().join(" ");
    }

    // Remove last line break
    if (text.endsWith("\n")) {
        text.chop(1);
    }

    if (text.isEmpty()) {
        out << "Error: There is no text for translation." << endl;
        return 0;
    }

    // Only tts option
    if (parser.isSet("audio-only")) {
        if (!parser.isSet("speak-source") && !parser.isSet("speak-translation")) {
            out << "Error: For --audio-only you must specify --speak-source or --speak-translation options." << endl;
            parser.showHelp();
        }

        // Speak source
        if (parser.isSet("speak-source")) {
            out << "Source text:" << endl;
            out << text << endl;

            if (!speak(translator, translator.source(), engine, translator.sourceLanguage()))
                return 0;
        }

        // Speak translation in all languages
        if (parser.isSet("speak-translation")) {
            foreach (QOnlineTranslator::Language translationLang, translationLangs) {
                // Speak into each target language
                translator.translate(text, engine, translationLang, sourceLang, uiLang);
                out << "Translation into " << translator.translationLanguageString() << ":" << endl;
                out << translator.translation() << endl;

                if (speak(translator, translator.translation(), engine, translator.translationLanguage()))
                    return 0;
            }
        }
        return 0;
    }

    // Translate into each target language
    for (auto i = 0; i < translationLangs.size(); i++) {
        translator.translate(text, engine, translationLangs.at(i), sourceLang, uiLang);

        // Check for network error
        if (translator.error()) {
            out << translator.errorString();
            return 0;
        }

        // Show source text and transliteration only once
        if (i == 0) {
            out << text << endl;
            if (!translator.sourceTranslit().isEmpty())
                out << "(" << translator.sourceTranslit().replace("\n", ")\n(") << ")" << endl << endl;
            else
                out << endl;
        } else {
            out << endl;
        }

        // Show languages
        out << "[ " << translator.sourceLanguageString() << " -> ";
        out << translator.translationLanguageString() << " ]" << endl << endl ;

        // Show text translation and transliteration
        if (!translator.translation().isEmpty()) {
            out << translator.translation() << endl;
            if (!translator.translationTranslit().isEmpty())
                out << "/" << translator.translationTranslit().replace("\n", "/\n/") << "/" << endl << endl;
            else
                out << endl;
        }

        // Show translation options
        if (!translator.translationOptions().isEmpty()) {
            out << translator.source() << " - translation options:" << endl;
            foreach (const QOption &option, translator.translationOptions()) {
                out << option.typeOfSpeech() << endl;
                for (int i = 0; i < option.count(); i++) {
                    out << "\t";
                    if (!option.gender(i).isEmpty())
                        out << option.gender(i) << " ";
                    out << option.word(i) << ": ";

                    out << option.translations(i) << endl;
                }
                out << endl;
            }
        }

        // Show examples
        if (!translator.examples().isEmpty()) {
            out << translator.source() << " - examples:" << endl;
            foreach (const QExample &example, translator.examples()) {
                out << example.typeOfSpeech() << endl;
                for (int i = 0; i < example.count(); ++ i) {
                    out << "\t" << example.description(i) << endl;
                    out << "\t" << example.example(i) << endl;
                }
            }
        }

        if (parser.isSet("speak-source") && i == 0)
            if (speak(translator, translator.source(), engine, translator.sourceLanguage()))
                return 0;

        if (parser.isSet("speak-translation"))
            if (speak(translator, translator.translation(), engine, translator.translationLanguage()))
                return 0;
    }

    return 0;
}

bool speak(QOnlineTranslator &translator, const QString &text, QOnlineTranslator::Engine engine, QOnlineTranslator::Language language) {
    QMediaPlayer player;
    QMediaPlaylist playlist;
    QEventLoop waitUntilPlayedLoop;
    QObject::connect(&player, &QMediaPlayer::stateChanged, [&](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState)
            waitUntilPlayedLoop.quit();
    });

    playlist.addMedia(translator.media(text, engine, language));
    if (translator.error() != QOnlineTranslator::NoError) {
        QTextStream out(stdout);
        out << translator.errorString() << endl;
        return false;
    }

    player.setPlaylist(&playlist);
    player.play();
    waitUntilPlayedLoop.exec();

    return true;
}
