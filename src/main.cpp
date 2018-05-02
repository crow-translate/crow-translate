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
        app.setApplicationVersion("0.9.7");

        QCommandLineParser parser;
        parser.setApplicationDescription("This program translates text using Google Translate API\n\t"
                                         "Usage: crow <text>\n\t"
                                         "or: crow [output language] <text>\n\t"
                                         "or: crow [input language] [output language] <text>");

        // Add all languages to parser
        for (auto i = 0; i < QOnlineTranslator::LANGUAGE_NAMES.size(); i++) {
            parser.addOption({{QOnlineTranslator::LANGUAGE_SHORT_CODES.at(i), QOnlineTranslator::LANGUAGE_LONG_CODES.at(i)}, QOnlineTranslator::LANGUAGE_NAMES.at(i)+" language"});
        }

        parser.addPositionalArgument("text", "Text to translate");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.process(app);

        switch (parser.optionNames().size()) {
        case 0: // Just translate the text if no language options
        {
            qInfo() << QOnlineTranslator::translateText(parser.positionalArguments().join(" "));
            break;
        }
        case 1:
        {
            qInfo() << QOnlineTranslator::translateText(parser.positionalArguments().join(" "), parser.optionNames().at(0));
            break;
        }
        case 2:
        {
            qInfo() << QOnlineTranslator::translateText(parser.positionalArguments().join(" "), parser.optionNames().at(1), parser.optionNames().at(0));
            break;
        }
        default:
        {
            qInfo() << "Too many options";
            parser.showHelp();
            break;
        }
        }
        return 0;
    }
}
