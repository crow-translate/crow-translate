#ifndef CLI_H
#define CLI_H

#include "qonlinetranslator.h"

#include <QObject>
#include <QVector>

class QCoreApplication;
class QMediaPlayer;
class QMediaPlaylist;
class QEventLoop;

class Cli : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        PrintLangCodes,
        AudioOnly,
        Translation
    };

    explicit Cli(QObject *parent = nullptr);

    void parseArguments(QCoreApplication &app);
    int exec();

private:
    // Modes
    int printLangCodes();
    int audioOnly();
    int translation();

    // Helper functions
    bool speak(const QString &text, QOnlineTranslator::Engine engine, QOnlineTranslator::Language language);
    static QByteArray readFilesFromStdin();
    static QByteArray readFilesFromArguments(const QStringList &arguments);

    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
    QEventLoop *m_waitUntilPlayedLoop;
    QOnlineTranslator *m_translator;

    QString m_sourceText;
    Mode m_mode = Translation;
    QOnlineTranslator::Engine m_engine = QOnlineTranslator::Google;
    QOnlineTranslator::Language m_sourceLang = QOnlineTranslator::NoLanguage;
    QOnlineTranslator::Language m_uiLang = QOnlineTranslator::NoLanguage;
    QVector<QOnlineTranslator::Language> m_translationLangs;
    bool m_speakSource = false;
    bool m_speakTranslation = false;
};

#endif // CLI_H
