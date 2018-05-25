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
        foreach (auto text, parser.positionalArguments()) {
            // For only audio option
            if (parser.isSet("a")) {
                if (parser.isSet("e")) {
                    QOnlineTranslator translationData(text, parser.value("t"), parser.value("s"), parser.value("l"));
                    out << translationData.text() << endl;
                    translationData.say();
                }
                if (parser.isSet("q")) {
                    out << text << endl;
                    QOnlineTranslator::say(text, parser.value("s"));
                }
                continue;
            }

            // Translate into each target language
            QStringList targetLanguages = parser.value("t").split("+");
            for (auto i = 0; i < targetLanguages.size(); i++) {
                QOnlineTranslator translationData(text, targetLanguages.at(i), parser.value("s"), parser.value("l"));

                // Check for network error
                if (translationData.error()) {
                    out << translationData.text();
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
                if (!translationData.text().isEmpty()) {
                    out << translationData.text() << endl;
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
                    QOnlineTranslator::say(text, parser.value("s"));
                }
                if (parser.isSet("e"))
                    translationData.say();
            }
        }
        return 0;
    }
}
