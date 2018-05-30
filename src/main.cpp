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

#include <QCommandLineParser>
#include <QSettings>
#include <QTranslator>
#include <QMediaPlayer>

#include "singleapplication.h"
#include "qonlinetranslator.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // If there are no arguments, just launch Crow Translate
    if (argc == 1) {
        SingleApplication app(argc, argv);
        QApplication::setApplicationName("Crow Translate");
        QCoreApplication::setOrganizationName("crow");

#if defined(Q_OS_WIN)
        QIcon::setThemeName("Papirus");
#endif

        QSettings settings;
        MainWindow w;
        if (!settings.value("StartMinimized", false).toBool()) w.show();
        return app.exec();
    }
    else {
        QCoreApplication app(argc, argv);
        app.setApplicationName("Crow Translate");
        app.setApplicationVersion("0.9.8");

        QCommandLineParser parser;
        parser.setApplicationDescription("A simple and lightweight translator that allows to translate and say text using the Google Translate API and much more.");
        parser.addPositionalArgument("text", "Text to translate. By default, the translation will be done to the system language.");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(QCommandLineOption({"s", "source"}, "Specifies the source language. By default, Google will try to determine the language on its own.", "code", "auto"));
        parser.addOption(QCommandLineOption({"t", "translation"}, "Specifies the translation language(s), joined by '+'. By default, the system language is used.", "code", "auto"));
        parser.addOption(QCommandLineOption({"l", "translator"}, "Specifies the translator language. By default, the system language is used.", "code", "auto"));
        parser.addOption(QCommandLineOption({"e", "speak-translation"}, "Speaks the translation."));
        parser.addOption(QCommandLineOption({"q", "speak-source"}, "Speaks the original text."));
        parser.addOption(QCommandLineOption({"a", "audio-only"}, "Prints text only for playing when using --speak-translation or --speak-source."));
        parser.process(app);


        QTextStream out(stdout);
        QMediaPlayer player;
        QMediaPlaylist playlist;
        QEventLoop waitUntilPlayedLoop;
        QObject::connect(&player, &QMediaPlayer::stateChanged, [&](QMediaPlayer::State state) {
            if (state == QMediaPlayer::StoppedState)
                waitUntilPlayedLoop.quit();
        });

        // Translate and / or speach all arguments
        foreach (auto text, parser.positionalArguments()) {
            QStringList targetLanguages = parser.value("t").split("+");
            for (auto i = 0; i < targetLanguages.size(); i++) {
                // Translate into each target language
                if (parser.isSet("a")) {
                    // For only audio option
                    if (parser.isSet("q")) {
                        out << text << endl;
                        playlist.addMedia(QOnlineTranslator::media(text, targetLanguages.at(i)));
                        player.setPlaylist(&playlist);
                        player.play();
                        waitUntilPlayedLoop.exec();
                        playlist.clear();
                    }
                    if (parser.isSet("e")) {
                        QOnlineTranslator translationData(text, targetLanguages.at(i), parser.value("s"), parser.value("l"));
                        out << translationData.translation() << endl;
                        playlist.addMedia(translationData.translationMedia());
                        player.setPlaylist(&playlist);
                        player.play();
                        waitUntilPlayedLoop.exec();
                        playlist.clear();
                    }
                    continue;
                }

                QOnlineTranslator translationData(text, targetLanguages.at(i), parser.value("s"), parser.value("l"));

                // Check for network error
                if (translationData.error()) {
                    out << translationData.translation();
                    return 0;
                }

                // Show source text and transliteration only once
                if (i == 0) {
                    out << text << endl;
                    if (!translationData.sourceTranscription().isEmpty())
                        out << "(" << translationData.sourceTranscription().replace("\n", ")\n(") << ")" << endl << endl;
                    else
                        out << endl;
                }

                // Show languages
                out << "[ " << QOnlineTranslator::codeToLanguage(translationData.sourceLanguage()) << " -> ";
                out << QOnlineTranslator::codeToLanguage(translationData.translationLanguage()) << " ]" << endl << endl ;

                // Show translation text and transliteration
                if (!translationData.translation().isEmpty()) {
                    out << translationData.translation() << endl;
                    if (!translationData.translationTranscription().isEmpty())
                        out << "/" << translationData.translationTranscription().replace("\n", "/\n/") << "/" << endl << endl;
                    else
                        out << endl;
                }

                // Show translation options
                foreach (auto translationOptions, translationData.options()) {
                    out << translationOptions.first << endl;
                    foreach (QString wordsList, translationOptions.second)
                        out << "\t" << wordsList << endl;
                    out << endl;
                }

                if (parser.isSet("q")) {
                    playlist.addMedia(translationData.sourceMedia());
                    player.setPlaylist(&playlist);
                    player.play();
                    waitUntilPlayedLoop.exec();
                    playlist.clear();
                }
                if (parser.isSet("e")) {
                    playlist.addMedia(translationData.translationMedia());
                    player.setPlaylist(&playlist);
                    player.play();
                    waitUntilPlayedLoop.exec();
                    playlist.clear();
                }
            }
        }
        return 0;
    }
}
