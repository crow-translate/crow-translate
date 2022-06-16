/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CLI_H
#define CLI_H

#include "qonlinetranslator.h"

#include <QObject>
#include <QTextStream>
#include <QVector>

class QCoreApplication;
class QMediaPlayer;
class QStateMachine;
class QCommandLineParser;
class QCommandLineOption;

class Cli : public QObject
{
    Q_OBJECT

public:
    explicit Cli(QObject *parent = nullptr);

    void process(const QCoreApplication &app);

private slots:
    void requestTranslation();
    void parseTranslation();
    void printTranslation();

    void requestLanguage();
    void parseLanguage();

    void speakSource();
    void speakTranslation();

    void printLangCodes();

private:
    // Main state machines
    void buildShowCodesStateMachine();
    void buildTranslationStateMachine();

    // Helpers
    void speak(const QString &text, QOnlineTranslator::Language lang);
    static void checkIncompatibleOptions(QCommandLineParser &parser, const QCommandLineOption &option1, const QCommandLineOption &option2);

    static QByteArray readFilesFromStdin();
    static QByteArray readFilesFromArguments(const QStringList &arguments);

    static constexpr char s_langProperty[] = "Language";

    QMediaPlayer *m_player;
    QOnlineTranslator *m_translator;
    QStateMachine *m_stateMachine;
    QTextStream m_stdout{stdout};

    QString m_sourceText;
    QVector<QOnlineTranslator::Language> m_translationLanguages;
    QOnlineTranslator::Engine m_engine = QOnlineTranslator::Google;
    QOnlineTranslator::Language m_sourceLang = QOnlineTranslator::NoLanguage;
    QOnlineTranslator::Language m_uiLang = QOnlineTranslator::NoLanguage;
    bool m_speakSource = false;
    bool m_speakTranslation = false;
    bool m_sourcePrinted = false;
    bool m_brief = false;
    bool m_audioOnly = false;
    bool m_json = false;
};

#endif // CLI_H
