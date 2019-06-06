#ifndef PLAYERBUTTONS_H
#define PLAYERBUTTONS_H

#include <QObject>
#include <QMediaPlayer>

class QAbstractButton;

class PlayerButtons : public QObject
{
    Q_OBJECT

public:
    PlayerButtons(QAbstractButton *playPauseButton, QAbstractButton *stopButton, QObject *parent = nullptr);

    QMediaPlayer *mediaPlayer() const;
    void setMediaPlayer(QMediaPlayer *mediaPlayer);

    QMediaPlaylist *playlist();

    void play();
    void pause();
    void stop();

signals:
    void playerDataRequested(QMediaPlaylist *playist);

private slots:
    void loadPlayerState(QMediaPlayer::State state);
    void processPlayPausePressed();

private:
    static QMediaPlayer *currentlyPlaying;

    QMediaPlayer *m_mediaPlayer;

    QAbstractButton *m_playPauseButton = nullptr;
    QAbstractButton *m_stopButton = nullptr;
};

#endif // PLAYERBUTTONS_H
