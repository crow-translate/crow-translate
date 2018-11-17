/*
 *  Copyright © 2018 Gennady Chernyshchuk <genaloner@gmail.com>
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

#include "singleapplication.h"
#include "appsettings.h"
#include "qonlinetranslator.h"
#include "mainwindow.h"

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
        SingleApplication::setApplicationVersion("2.0.0");

#if defined(Q_OS_WIN)
        QIcon::setThemeName("Papirus");
#endif

        AppSettings settings;
        settings.setupLocale();
        MainWindow w;
        if (!settings.isStartMinimized())
            w.show();
        return app.exec();
    }

    // Command line interface
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Crow Translate");
    QCoreApplication::setApplicationVersion("2.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("A simple and lightweight translator that allows to translate and say text using the Google Translate API and much more.");
    parser.addPositionalArgument("text", "Text to translate. By default, the translation will be done to the system language.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"c", "codes"}, "Show all language codes."));
    parser.addOption(QCommandLineOption({"s", "source"}, "Specifies the source language. By default, Google will try to determine the language on its own.", "code", "auto"));
    parser.addOption(QCommandLineOption({"t", "translation"}, "Specifies the translation language(s), joined by '+'. By default, the system language is used.", "code", "auto"));
    parser.addOption(QCommandLineOption({"l", "locale"}, "Specifies the translator language. By default, the system language is used.", "code", "auto"));
    parser.addOption(QCommandLineOption({"e", "engine"}, "Specifies the translator engine ('google' or 'yandex'). Google is used by default.", "engine", "google"));
    parser.addOption(QCommandLineOption({"p", "speak-translation"}, "Speaks the translation."));
    parser.addOption(QCommandLineOption({"u", "speak-source"}, "Speaks the original text."));
    parser.addOption(QCommandLineOption({"a", "audio-only"}, "Prints text only for playing when using --speak-translation or --speak-source."));
    parser.addOption(QCommandLineOption({"f", "file"}, "Read source text from files. Arguments will be interpreted as file paths."));
    parser.addOption(QCommandLineOption({"i", "stdin"}, "Add stdin data to source text."));
    parser.process(app);

    QOnlineTranslator translator;
    QTextStream out(stdout);

    // Show all language codes
    if (parser.isSet("codes")) {
        for (int language = QOnlineTranslator::Auto; language != QOnlineTranslator::Zulu; ++language) {
            out << translator.languageString(static_cast<QOnlineTranslator::Language>(language));
            out << " - ";
            out << translator.languageCode(static_cast<QOnlineTranslator::Language>(language)) << endl;
        }
        return 0;
    }

    // Parse engine
    QOnlineTranslator::Engine engine;
    if (parser.value("engine") == "google")
        engine = QOnlineTranslator::Google;
    else if (parser.value("engine") == "yandex")
        engine = QOnlineTranslator::Yandex;
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

        // Play source
        if (parser.isSet("speak-source")) {
            out << "Source text:" << endl;
            out << text << endl;

            if (!speak(translator, translator.source(), engine, translator.sourceLanguage()))
                return 0;
        }

        // Play translation in all languages
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
        if (!translator.dictionaryList().isEmpty()) {
            out << translator.source() << " - translation options:" << endl;
            foreach (const QDictionary &dictionary, translator.dictionaryList()) {
                out << dictionary.typeOfSpeech() << endl;
                for (auto i = 0; i < dictionary.count(); i++) {
                    out << "\t";
                    if (!dictionary.gender(i).isEmpty())
                        out << dictionary.gender(i) << " ";
                    out << dictionary.word(i) << ": ";

                    out << dictionary.translations(i) << endl;
                }
                out << endl;
            }
        }

        // Show definitions
        if (!translator.definitionsList().isEmpty()) {
            out << translator.source() << " - definitions:" << endl;
            foreach (const QDefinition &definition, translator.definitionsList()) {
                out << definition.typeOfSpeech() << endl;
                out << "\t" << definition.description() << endl;
                out << "\t" << definition.example() << endl;
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
