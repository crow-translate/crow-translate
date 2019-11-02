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

#ifndef CLI_H
#define CLI_H

#include "qonlinetranslator.h"

#include <QObject>
#include <QVector>

class QCoreApplication;
class QMediaPlayer;
class QStateMachine;

class Cli : public QObject
{
    Q_OBJECT

public:
    explicit Cli(QObject *parent = nullptr);

    void process(const QCoreApplication &app);

private slots:
    void printLangCodes();

    void requestTranslation();
    void printTranslation();

    void requestLanguage();
    void parseLanguage();

    void speakSource();
    void speakTranslation();

    void printSpeakingSourceText();
    void printSpeakingTranslation();

private:
    // Main state machines
    void buildShowCodesStateMachine();
    void buildAudioOnlyStateMachine();
    void buildTranslationStateMachine();

    // Nested states
    void buildSpeakSourceState(QState *parent);
    void buildSpeakTranslationsState(QState *parent);

    // Helper functions
    void error(const QString &error);
    void speak(const QString &text, QOnlineTranslator::Language lang);

    static QByteArray readFilesFromStdin();
    static QByteArray readFilesFromArguments(const QStringList &arguments);

    QMediaPlayer *m_player;
    QOnlineTranslator *m_translator;
    QStateMachine *m_stateMachine;

    QString m_sourceText;
    QVector<QOnlineTranslator::Language> m_translationLangs;
    QOnlineTranslator::Engine m_engine = QOnlineTranslator::Google;
    QOnlineTranslator::Language m_sourceLang = QOnlineTranslator::NoLanguage;
    QOnlineTranslator::Language m_uiLang = QOnlineTranslator::NoLanguage;
    bool m_speakSource = false;
    bool m_speakTranslation = false;

    bool m_sourcePrinted = false;

    static constexpr char s_langProperty[] = "Language";
};

#endif // CLI_H
