#include "playerbuttons.h"

#include <QAbstractButton>
#include <QMediaPlaylist>

QMediaPlayer *PlayerButtons::currentlyPlaying = nullptr;

PlayerButtons::PlayerButtons(QAbstractButton *playPauseButton, QAbstractButton *stopButton, QObject *parent) :
    QObject(parent)
{
    m_playPauseButton = playPauseButton;
    m_playPauseButton->setText("Play");
    connect(m_playPauseButton, &QAbstractButton::clicked, this, &PlayerButtons::processPlayPausePressed);

    m_stopButton = stopButton;
    m_stopButton->setIcon(QIcon::fromTheme("media-playback-stop"));
    m_stopButton->setText("Stop");
    connect(m_stopButton, &QAbstractButton::clicked, this, &PlayerButtons::stop);
}

QMediaPlayer *PlayerButtons::mediaPlayer() const
{
    return m_mediaPlayer;
}

void PlayerButtons::setMediaPlayer(QMediaPlayer *mediaPlayer)
{
    m_mediaPlayer = mediaPlayer;
    if (m_mediaPlayer->playlist() == nullptr)
        m_mediaPlayer->setPlaylist(new QMediaPlaylist);

    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::loadPlayerState);
    loadPlayerState(m_mediaPlayer->state());
}

QMediaPlaylist *PlayerButtons::playlist()
{
    return m_mediaPlayer->playlist();
}

void PlayerButtons::play()
{
    emit playerDataRequested(m_mediaPlayer->playlist());
    m_mediaPlayer->play();
}

void PlayerButtons::pause()
{
    m_mediaPlayer->pause();
}

void PlayerButtons::stop()
{
    m_mediaPlayer->stop();
}

void PlayerButtons::loadPlayerState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::StoppedState:
        if (currentlyPlaying == m_mediaPlayer)
            currentlyPlaying = nullptr;

        m_playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
        m_stopButton->setEnabled(false);
        break;
    case QMediaPlayer::PlayingState:
        if (currentlyPlaying != nullptr)
            currentlyPlaying->pause();
        currentlyPlaying = m_mediaPlayer;

        m_playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        m_stopButton->setEnabled(true);
        break;
    case QMediaPlayer::PausedState:
        if (currentlyPlaying == m_mediaPlayer)
            currentlyPlaying = nullptr;

        m_playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
        break;
    }
}

void PlayerButtons::processPlayPausePressed()
{
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::StoppedState:
        play();
        break;
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    case QMediaPlayer::PausedState:
        m_mediaPlayer->play();
        break;
    }
}
