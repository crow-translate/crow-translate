/*
 *  Copyright © 2018-2021 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
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
#include "transitions/playerstoppedtransition.h"

#include <QCommandLineParser>
#include <QFile>
#include <QFinalState>
#include <QJsonDocument>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QRegularExpression>
#include <QStateMachine>

Cli::Cli(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_translator(new QOnlineTranslator(this))
    , m_stateMachine(new QStateMachine(this))
{
    m_player->setPlaylist(new QMediaPlaylist);

    connect(m_stateMachine, &QStateMachine::finished, QCoreApplication::instance(), &QCoreApplication::quit, Qt::QueuedConnection);
    connect(m_stateMachine, &QStateMachine::stopped, QCoreApplication::instance(), [] {
        QCoreApplication::exit(1);
    }, Qt::QueuedConnection);
}

void Cli::process(const QCoreApplication &app)
{
    const QCommandLineOption codes({"c", "codes"}, tr("Display all language codes."));
    const QCommandLineOption source({"s", "source"}, tr("Specify the source language (by default, engine will try to determine the language on its own)."), QStringLiteral("code"), QStringLiteral("auto"));
    const QCommandLineOption translation({"t", "translation"}, tr("Specify the translation language(s), splitted by '+' (by default, the system language is used)."), QStringLiteral("code"), QStringLiteral("auto"));
    const QCommandLineOption locale({"l", "locale"}, tr("Specify the translator language (by default, the system language is used)."), QStringLiteral("code"), QStringLiteral("auto"));
    const QCommandLineOption engine({"e", "engine"}, tr("Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default."), QStringLiteral("engine"), QStringLiteral("google"));
    const QCommandLineOption speakTranslation({"p", "speak-translation"}, tr("Speak the translation."));
    const QCommandLineOption speakSource({"u", "speak-source"}, tr("Speak the source."));
    const QCommandLineOption file({"f", "file"}, tr("Read source text from files. Arguments will be interpreted as file paths."));
    const QCommandLineOption readStdin({"i", "stdin"}, tr("Add stdin data to source text."));
    const QCommandLineOption audioOnly({"a", "audio-only"}, tr("Do not print any text when using --%1 or --%2.").arg(speakSource.names().at(1), speakTranslation.names().at(1)));
    const QCommandLineOption brief({"b", "brief"}, tr("Print only translations."));
    const QCommandLineOption json({"j", "json"}, tr("Print output formatted as JSON."));

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("A simple and lightweight translator that allows to translate and speak text using Google, Yandex and Bing."));
    parser.addPositionalArgument(QStringLiteral("text"), tr("Text to translate. By default, the translation will be done to the system language."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(codes);
    parser.addOption(source);
    parser.addOption(translation);
    parser.addOption(locale);
    parser.addOption(engine);
    parser.addOption(speakTranslation);
    parser.addOption(speakSource);
    parser.addOption(file);
    parser.addOption(readStdin);
    parser.addOption(audioOnly);
    parser.addOption(brief);
    parser.addOption(json);
    parser.process(app);

    checkIncompatibleOptions(parser, audioOnly, brief);
    checkIncompatibleOptions(parser, json, audioOnly);
    checkIncompatibleOptions(parser, json, brief);

    if (parser.isSet(audioOnly) && !parser.isSet(speakSource) && !parser.isSet(speakTranslation)) {
        qCritical() << tr("Error: For --%1 you must specify --%2 and/or --%3 options").arg(audioOnly.names().at(1), speakSource.names().at(1), speakTranslation.names().at(1)) << '\n';
        parser.showHelp();
    }

    // Only show language codes
    if (parser.isSet(codes)) {
        buildShowCodesStateMachine();
        m_stateMachine->start();
        return;
    }

    // Translation languages
    m_sourceLang = QOnlineTranslator::language(parser.value(source));
    m_uiLang = QOnlineTranslator::language(parser.value(locale));
    for (const QString &langCode : parser.value(translation).split('+'))
        m_translationLanguages << QOnlineTranslator::language(langCode);

    // Source text
    if (parser.isSet(file)) {
        if (parser.isSet(readStdin))
            m_sourceText += readFilesFromStdin();

        m_sourceText += readFilesFromArguments(parser.positionalArguments());
    } else {
        if (parser.isSet(readStdin))
            m_sourceText += QTextStream(stdin).readAll();

        m_sourceText += parser.positionalArguments().join(' ');
    }

    if (m_sourceText.endsWith('\n'))
        m_sourceText.chop(1);

    if (m_sourceText.isEmpty()) {
        qCritical() << tr("Error: There is no text for translation") << '\n';
        parser.showHelp();
    }

    // Engine
    if (parser.value(engine) == QLatin1String("google")) {
        m_engine = QOnlineTranslator::Google;
    } else if (parser.value(engine) == QLatin1String("yandex")) {
        m_engine = QOnlineTranslator::Yandex;
    } else if (parser.value(engine) == QLatin1String("bing")) {
        m_engine = QOnlineTranslator::Bing;
    } else {
        qCritical() << tr("Error: Unknown engine") << '\n';
        parser.showHelp();
    }

    // Audio options
    m_speakSource = parser.isSet(speakSource);
    m_speakTranslation = parser.isSet(speakTranslation);

    // Modes
    m_audioOnly = parser.isSet(audioOnly);
    m_brief = parser.isSet(brief);
    m_json = parser.isSet(json);
    if (m_brief || m_audioOnly) {
        m_translator->setExamplesEnabled(false);
        m_translator->setTranslationOptionsEnabled(false);
        m_translator->setSourceTranscriptionEnabled(false);
        m_translator->setTranslationTranslitEnabled(false);
        m_translator->setSourceTranslitEnabled(false);
    }

    buildTranslationStateMachine();
    m_stateMachine->start();
}

void Cli::requestTranslation()
{
    auto *state = qobject_cast<QState *>(sender());
    auto translationLang = state->property(s_langProperty).value<QOnlineTranslator::Language>();

    m_translator->translate(m_sourceText, m_engine, translationLang, m_sourceLang, m_uiLang);
}

void Cli::parseTranslation()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        qCritical() << tr("Error: %1").arg(m_translator->errorString());
        m_stateMachine->stop();
        return;
    }

    if (m_sourceLang == QOnlineTranslator::Auto)
        m_sourceLang = m_translator->sourceLanguage();
}

void Cli::printTranslation()
{
    // JSON mode
    if (m_json) {
        m_stdout << m_translator->toJson().toJson();
        return;
    }

    // Short mode
    if (m_brief) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_stdout << m_translator->translation() << Qt::endl;
#else
        m_stdout << m_translator->translation() << endl;
#endif
        return;
    }

    // Show source text and its transliteration only once
    if (!m_sourcePrinted) {
        m_stdout << m_translator->source() << '\n';
        if (!m_translator->sourceTranslit().isEmpty())
            m_stdout << '(' << m_translator->sourceTranslit().replace('\n', QStringLiteral(")\n(")) << ")\n";
        m_sourcePrinted = true;
    }
    m_stdout << '\n';

    // Languages
    m_stdout << "[ " << m_translator->sourceLanguageName() << " -> ";
    m_stdout << m_translator->translationLanguageName() << " ]" << "\n\n";

    // Translation and its transliteration
    if (!m_translator->translation().isEmpty()) {
        m_stdout << m_translator->translation() << '\n';
        if (!m_translator->translationTranslit().isEmpty())
            m_stdout << '/' << m_translator->translationTranslit().replace('\n', QStringLiteral("/\n/")) << "/\n";
        m_stdout << '\n';
    }

    // Translation options
    if (!m_translator->translationOptions().isEmpty()) {
        m_stdout << tr("%1 - translation options:").arg(m_translator->source()) << '\n';
        const QMap<QString, QVector<QOption>> translationOptions = m_translator->translationOptions();
        for (auto it = translationOptions.cbegin(); it != translationOptions.cend(); ++it) {
            m_stdout << it.key() << '\n';
            for (const auto &[word, gender, translations] : it.value()) {
                m_stdout << '\t';
                if (!gender.isEmpty())
                    m_stdout << gender << ' ';
                m_stdout << word << ": ";
                m_stdout << translations.join(QStringLiteral(", ")) << '\n';
            }
            m_stdout << '\n';
        }
    }

    // Examples
    if (!m_translator->examples().isEmpty()) {
        m_stdout << tr("%1 - examples:").arg(m_translator->source()) << '\n';
        const QMap<QString, QVector<QExample>> examples = m_translator->examples();
        for (auto it = examples.cbegin(); it != examples.cend(); ++it) {
            m_stdout << it.key() << '\n';
            for (const auto &[example, description] : it.value()) {
                m_stdout << '\t' << description << '\n';
                m_stdout << '\t' << example << '\n';
            }
        }
    }

    m_stdout.flush();
}

void Cli::requestLanguage()
{
    m_translator->detectLanguage(m_sourceText, m_engine);
}

void Cli::parseLanguage()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        qCritical() << tr("Error: %1").arg(m_translator->errorString());
        m_stateMachine->stop();
        return;
    }

    m_sourceLang = m_translator->sourceLanguage();
}

void Cli::printLangCodes()
{
    for (int langIndex = QOnlineTranslator::Auto; langIndex != QOnlineTranslator::Zulu; ++langIndex) {
        const auto lang = static_cast<QOnlineTranslator::Language>(langIndex);
        m_stdout << QOnlineTranslator::languageName(lang) << " - " << QOnlineTranslator::languageCode(lang) << '\n';
    }
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

void Cli::buildTranslationStateMachine()
{
    auto *nextTranslationState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(nextTranslationState);

    for (QOnlineTranslator::Language lang : qAsConst(m_translationLanguages)) {
        auto *requestTranslationState = nextTranslationState;
        auto *parseDataState = new QState(m_stateMachine);
        auto *speakSourceText = new QState(m_stateMachine);
        auto *speakTranslation = new QState(m_stateMachine);
        nextTranslationState = new QState(m_stateMachine);

        if (m_audioOnly && m_speakSource && !m_speakTranslation && m_sourceLang == QOnlineTranslator::Auto) {
            connect(requestTranslationState, &QState::entered, this, &Cli::requestLanguage);
            connect(parseDataState, &QState::entered, this, &Cli::parseLanguage);
        } else {
            connect(requestTranslationState, &QState::entered, this, &Cli::requestTranslation);
            connect(parseDataState, &QState::entered, this, &Cli::parseTranslation);
            if (!m_audioOnly)
                connect(parseDataState, &QState::entered, this, &Cli::printTranslation);

            requestTranslationState->setProperty(s_langProperty, lang);
        }

        requestTranslationState->addTransition(m_translator, &QOnlineTranslator::finished, parseDataState);
        parseDataState->addTransition(speakSourceText);

        if (m_speakSource) {
            connect(speakSourceText, &QState::entered, this, &Cli::speakSource);

            auto *speakSourceTransition = new PlayerStoppedTransition(m_player, speakSourceText);
            speakSourceTransition->setTargetState(speakTranslation);
        } else {
            speakSourceText->addTransition(speakTranslation);
        }

        if (m_speakTranslation) {
            connect(speakTranslation, &QState::entered, this, &Cli::speakTranslation);

            auto *speakTranslationTransition = new PlayerStoppedTransition(m_player, speakTranslation);
            speakTranslationTransition->setTargetState(nextTranslationState);
        } else {
            speakTranslation->addTransition(nextTranslationState);
        }
    }

    nextTranslationState->addTransition(new QFinalState(m_stateMachine));
}

void Cli::speak(const QString &text, QOnlineTranslator::Language lang)
{
    QOnlineTts tts;
    tts.generateUrls(text, m_engine, lang);
    if (tts.error() != QOnlineTts::NoError) {
        qCritical() << tr("Error: %1").arg(tts.errorString());
        m_stateMachine->stop();
        return;
    }

    m_player->playlist()->clear();
    m_player->playlist()->addMedia(tts.media());
    m_player->play();
}

void Cli::checkIncompatibleOptions(QCommandLineParser &parser, const QCommandLineOption &option1, const QCommandLineOption &option2)
{
    if (parser.isSet(option1) && parser.isSet(option2)) {
        qCritical() << tr("Error: You can't use --%1 with --%2").arg(option1.names().at(1), option2.names().at(1)) << '\n';
        parser.showHelp();
    }
}

QByteArray Cli::readFilesFromStdin()
{
    QString stdinText = QTextStream(stdin).readAll();
    QByteArray filesData;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    for (const QString &filePath : stdinText.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts)) {
#else
    for (const QString &filePath : stdinText.split(QRegularExpression(QStringLiteral("\\s+")), QString::SkipEmptyParts)) {
#endif
        QFile file(filePath);
        if (!file.exists()) {
            qCritical() << tr("Error: File does not exist: %1").arg(file.fileName());
            continue;
        }

        if (!file.open(QFile::ReadOnly)) {
            qCritical() << tr("Error: Unable to open file: %1").arg(file.fileName());
            continue;
        }

        filesData += file.readAll();
    }

    return filesData;
}

QByteArray Cli::readFilesFromArguments(const QStringList &arguments)
{
    QByteArray filesData;
    for (const QString &filePath : arguments) {
        QFile file(filePath);
        if (!file.exists()) {
            qCritical() << tr("Error: File does not exist: %1").arg(file.fileName());
            continue;
        }

        if (!file.open(QFile::ReadOnly)) {
            qCritical() << tr("Error: Unable to open file: %1").arg(file.fileName());
            continue;
        }

        filesData += file.readAll();
    }

    return filesData;
}
