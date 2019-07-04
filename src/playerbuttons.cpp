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

#include "playerbuttons.h"

#include <QAbstractButton>
#include <QMediaPlaylist>
#ifdef Q_OS_WIN
#include <QWinTaskbarProgress>
#endif

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
    if (m_mediaPlayer != nullptr) {
        disconnect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::loadPlayerState);
        disconnect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &PlayerButtons::processPositionChanged);
    }

    m_mediaPlayer = mediaPlayer;
    if (m_mediaPlayer->playlist() == nullptr)
        m_mediaPlayer->setPlaylist(new QMediaPlaylist);

    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::loadPlayerState);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &PlayerButtons::processPositionChanged);

    loadPlayerState(m_mediaPlayer->state());
}

QMediaPlaylist *PlayerButtons::playlist()
{
    return m_mediaPlayer->playlist();
}

void PlayerButtons::play()
{
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
        emit stopped();
        break;
    case QMediaPlayer::PlayingState:
        if (currentlyPlaying != nullptr)
            currentlyPlaying->pause();
        currentlyPlaying = m_mediaPlayer;

        m_playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        m_stopButton->setEnabled(true);
        emit played();
        break;
    case QMediaPlayer::PausedState:
        if (currentlyPlaying == m_mediaPlayer)
            currentlyPlaying = nullptr;

        m_playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
        emit paused();
        break;
    }
}

void PlayerButtons::processPlayPausePressed()
{
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::StoppedState:
        emit playerMediaRequested();
        break;
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    case QMediaPlayer::PausedState:
        m_mediaPlayer->play();
        break;
    }
}

void PlayerButtons::processPositionChanged(qint64 position)
{
    if (m_mediaPlayer->duration() != 0)
        emit positionChanged(static_cast<int>(position * 100 / m_mediaPlayer->duration()));
}
