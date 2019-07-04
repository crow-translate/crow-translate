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

#include "cli.h"
#include "qonlinetts.h"
#include "singleapplication.h"
#include "transitions/playerstoppedtransition.h"

#include <QCommandLineParser>
#include <QFile>
#include <QRegularExpression>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStateMachine>
#include <QFinalState>

constexpr char langProperty[] = "Language";

Cli::Cli(QObject *parent) :
    QObject(parent)
{
    m_translator = new QOnlineTranslator(this);
    m_player = new QMediaPlayer(this);
    m_player->setPlaylist(new QMediaPlaylist);
    m_stateMachine = new QStateMachine(this);

    connect(m_stateMachine, &QStateMachine::finished, QCoreApplication::instance(), &QCoreApplication::quit);
    connect(m_stateMachine, &QStateMachine::stopped, QCoreApplication::instance(), &QCoreApplication::quit);
}

void Cli::process(const QCoreApplication &app)
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
        buildShowCodesStateMachine();
        m_stateMachine->start();
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
    foreach (const QString &language, parser.value("translation").split('+'))
        m_translationLangs << QOnlineTranslator::language(language);

    // Source text
    if (parser.isSet("file")) {
        if (parser.isSet("stdin"))
            m_sourceText += readFilesFromStdin();

        m_sourceText += readFilesFromArguments(parser.positionalArguments());
    } else {
        if (parser.isSet("stdin"))
            m_sourceText += QTextStream(stdin).readAll();

        m_sourceText += parser.positionalArguments().join(' ');
    }

    if (m_sourceText.endsWith('\n'))
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

        buildAudioOnlyStateMachine();
    } else {
        buildTranslationStateMachine();
    }

    m_stateMachine->start();
}

void Cli::printLangCodes()
{
    QTextStream out(stdout);

    for (int languageIndex = QOnlineTranslator::Auto; languageIndex != QOnlineTranslator::Zulu; ++languageIndex) {
        const auto language = static_cast<QOnlineTranslator::Language>(languageIndex);
        out << QOnlineTranslator::languageString(language) << " - " << QOnlineTranslator::languageCode(language) << '\n';
    }
}

void Cli::requestTranslation()
{
    auto *state = qobject_cast<QState *>(sender());
    auto translationLang = state->property(langProperty).value<QOnlineTranslator::Language>();

    m_translator->translate(m_sourceText, m_engine, translationLang, m_sourceLang, m_uiLang);
}

void Cli::printTranslation()
{
    QTextStream out(stdout);

    if (m_translator->error()) {
        error(m_translator->errorString());
        return;
    }

    // Show source text and its transliteration only once
    if (!m_sourcePrinted) {
        out << m_translator->source() << endl;
        if (!m_translator->sourceTranslit().isEmpty())
            out << '(' << m_translator->sourceTranslit().replace('\n', ")\n(") << ')' << endl << endl;
        else
            out << endl;
        m_sourcePrinted = true;
    } else {
        out << endl;
    }

    // Languages
    out << "[ " << m_translator->sourceLanguageString() << " -> ";
    out << m_translator->translationLanguageString() << " ]" << endl << endl;
    if (m_sourceLang == QOnlineTranslator::Auto)
        m_sourceLang = m_translator->sourceLanguage();

    // Translation and its transliteration
    if (!m_translator->translation().isEmpty()) {
        out << m_translator->translation() << endl;
        if (!m_translator->translationTranslit().isEmpty())
            out << '/' << m_translator->translationTranslit().replace('\n', "/\n/") << '/' << endl << endl;
        else
            out << endl;
    }

    // Translation options
    if (!m_translator->translationOptions().isEmpty()) {
        out << m_translator->source() << " - translation options:" << endl;
        foreach (const QOption &option, m_translator->translationOptions()) {
            out << option.typeOfSpeech() << endl;
            for (int i = 0; i < option.count(); ++i) {
                out << '\t';
                if (!option.gender(i).isEmpty())
                    out << option.gender(i) << ' ';
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
                out << '\t' << example.description(i) << endl;
                out << '\t' << example.example(i) << endl;
            }
        }
    }
}

void Cli::requestLanguage()
{
    m_translator->detectLanguage(m_sourceText, m_engine);
}

void Cli::parseLanguage()
{
    if (m_translator->error()) {
        error(m_translator->errorString());
        return;
    }

    m_sourceLang = m_translator->sourceLanguage();
}

void Cli::printSpeakingSourceText()
{
    QTextStream out(stdout);

    out << "Source text:" << endl;
    out << m_sourceText << endl;
}

void Cli::printSpeakingTranslation()
{
    QTextStream out(stdout);

    out << "Translation into " << m_translator->translationLanguageString() << ':' << endl;
    out << m_translator->translation() << endl;
}

void Cli::speakSource()
{
    speak(m_sourceText, m_sourceLang);
}

void Cli::speakTranslation()
{
    speak(m_translator->translation(), m_translator->translationLanguage());
}

void Cli::buildShowCodesStateMachine()
{
    auto *showCodesState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(showCodesState);

    connect(showCodesState, &QState::entered, this, &Cli::printLangCodes);
    showCodesState->addTransition(new QFinalState(m_stateMachine));
}

void Cli::buildAudioOnlyStateMachine()
{
    // States
    auto *speakSourceState = new QState(m_stateMachine);
    auto *speakTranslationsState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(speakSourceState);

    if (m_speakSource)
        buildSpeakSourceState(speakSourceState);
    else
        speakSourceState->setInitialState(new QFinalState(speakSourceState));

    if (m_speakTranslation)
        buildSpeakTranslationsState(speakTranslationsState);
    else
        speakTranslationsState->setInitialState(new QFinalState(speakTranslationsState));

    speakSourceState->addTransition(speakSourceState, &QState::finished, speakTranslationsState);
    speakTranslationsState->addTransition(speakTranslationsState, &QState::finished, new QFinalState(m_stateMachine));
}

void Cli::buildTranslationStateMachine()
{
    QTextStream out(stdout);

    auto *nextTranslationState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(nextTranslationState);

    foreach (QOnlineTranslator::Language lang, m_translationLangs) {
        auto *requestTranslationState = nextTranslationState;
        auto *parseDataState = new QState(m_stateMachine);
        auto *speakSourceText = new QState(m_stateMachine);
        auto *speakTranslation = new QState(m_stateMachine);
        nextTranslationState = new QState(m_stateMachine);

        connect(requestTranslationState, &QState::entered, this, &Cli::requestTranslation);
        connect(parseDataState, &QState::entered, this, &Cli::printTranslation);

        requestTranslationState->setProperty(langProperty, lang);

        requestTranslationState->addTransition(m_translator, &QOnlineTranslator::finished, parseDataState);
        parseDataState->addTransition(speakSourceText);

        if (m_speakSource) {
            connect(speakSourceText, &QState::entered, this, &Cli::speakSource);

            auto *speakSourceTransition = new PlayerStoppedTransition(m_player);
            speakSourceTransition->setTargetState(speakTranslation);
            speakSourceText->addTransition(speakSourceTransition);
        } else {
            speakSourceText->addTransition(speakTranslation);
        }

        if (m_speakTranslation) {
            connect(speakTranslation, &QState::entered, this, &Cli::speakTranslation);

            auto *speakTranslationTransition = new PlayerStoppedTransition(m_player);
            speakTranslationTransition->setTargetState(nextTranslationState);
            speakTranslation->addTransition(speakTranslationTransition);
        } else {
            speakTranslation->addTransition(nextTranslationState);
        }
    }

    nextTranslationState->addTransition(new QFinalState(m_stateMachine));
}

void Cli::buildSpeakSourceState(QState *parent)
{
    // Speak source substates
    auto *printTextState = new QState(parent);
    auto *detectLanguageState = new QState(parent);
    auto *speakTextState = new QState(parent);

    connect(printTextState, &QState::entered, this, &Cli::printSpeakingSourceText);
    connect(speakTextState, &QState::entered, this, &Cli::speakSource);

    // Transitions
    parent->setInitialState(printTextState);
    printTextState->addTransition(detectLanguageState);
    detectLanguageState->addTransition(detectLanguageState, &QState::finished, speakTextState);

    auto *speakTextTransition = new PlayerStoppedTransition(m_player);
    speakTextState->addTransition(speakTextTransition);
    speakTextTransition->setTargetState(new QFinalState(parent));

    // Setup detect language state
    if (m_sourceLang == QOnlineTranslator::Auto) {
        auto *requestingState = new QState(detectLanguageState);
        auto *parseState = new QState(detectLanguageState);

        connect(requestingState, &QState::entered, this, &Cli::requestLanguage);
        connect(parseState, &QState::entered, this, &Cli::parseLanguage);

        detectLanguageState->setInitialState(requestingState);

        requestingState->addTransition(m_translator, &QOnlineTranslator::finished, parseState);
        parseState->addTransition(new QFinalState(detectLanguageState));
    } else {
        detectLanguageState->setInitialState(new QFinalState(detectLanguageState));
    }
}

void Cli::buildSpeakTranslationsState(QState *parent)
{
    auto *nextSpeakTranslationState = new QState(parent);
    parent->setInitialState(nextSpeakTranslationState);

    foreach (QOnlineTranslator::Language language, m_translationLangs) {
        auto *requestTranslationState = nextSpeakTranslationState;
        auto *printTextState = new QState(parent);
        auto *speakTextState = new QState(parent);
        nextSpeakTranslationState = new QState(parent);

        connect(requestTranslationState, &QState::entered, this, &Cli::requestTranslation);
        connect(printTextState, &QState::entered, this, &Cli::printSpeakingTranslation);
        connect(speakTextState, &QState::entered, this, &Cli::speakTranslation);

        requestTranslationState->setProperty(langProperty, language);

        requestTranslationState->addTransition(m_translator, &QOnlineTranslator::finished, printTextState);
        printTextState->addTransition(speakTextState);

        auto *speakTextTransition = new PlayerStoppedTransition(m_player);
        speakTextState->addTransition(speakTextTransition);
        speakTextTransition->setTargetState(nextSpeakTranslationState);
    }

    nextSpeakTranslationState->addTransition(new QFinalState(parent));
}

void Cli::error(const QString &error)
{
    QTextStream out(stdout);

    out << "Error: " << error << endl;
    m_stateMachine->stop();
}

void Cli::speak(const QString &text, QOnlineTranslator::Language lang)
{
    QOnlineTts tts;
    tts.generateUrls(text, m_engine, lang);
    if (tts.error()) {
        error(tts.errorString());
        return;
    }

    m_player->playlist()->clear();
    m_player->playlist()->addMedia(tts.media());
    m_player->play();
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
