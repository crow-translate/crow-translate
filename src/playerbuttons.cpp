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
#include "ui_playerbuttons.h"
#include "settings/appsettings.h"

#include <QMediaPlaylist>
#include <QMessageBox>

QMediaPlayer *PlayerButtons::currentlyPlaying = nullptr;

PlayerButtons::PlayerButtons(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerButtons)
{
    ui->setupUi(this);

    connect(ui->playPauseButton, &QAbstractButton::clicked, this, &PlayerButtons::processPlayPausePressed);
    connect(ui->stopButton, &QAbstractButton::clicked, this, &PlayerButtons::stop);
}

PlayerButtons::~PlayerButtons()
{
    delete ui;
}

QMediaPlayer *PlayerButtons::mediaPlayer() const
{
    return m_mediaPlayer;
}

void PlayerButtons::setMediaPlayer(QMediaPlayer *mediaPlayer)
{
    if (m_mediaPlayer != nullptr) {
        disconnect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::loadPlayerState);
        disconnect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::stateChanged);
        disconnect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &PlayerButtons::processPositionChanged);
    }

    m_mediaPlayer = mediaPlayer;
    if (m_mediaPlayer->playlist() == nullptr)
        m_mediaPlayer->setPlaylist(new QMediaPlaylist);

    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &PlayerButtons::processPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::loadPlayerState);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerButtons::stateChanged);

    loadPlayerState(m_mediaPlayer->state());
}

QMediaPlaylist *PlayerButtons::playlist()
{
    return m_mediaPlayer->playlist();
}

void PlayerButtons::play(const QString &text, QOnlineTranslator::Language language, QOnlineTranslator::Engine engine)
{
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("No text specified"), tr("Playback text is empty"));
        return;
    }

    const AppSettings settings;
    const QOnlineTts::Voice voice = settings.voice(engine);
    const QOnlineTts::Emotion emotion = settings.emotion(engine);

    QOnlineTts onlineTts;
    onlineTts.generateUrls(text, engine, language, voice, emotion);
    if (onlineTts.error()) {
        QMessageBox::critical(this, tr("Unable to generate URLs for TTS"), onlineTts.errorString());
        return;
    }

    // Use playlist to split long queries due engines limit
    const QList<QMediaContent> media = onlineTts.media();
    playlist()->clear();
    playlist()->addMedia(media);
    play();
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

void PlayerButtons::setPlayPauseShortcut(const QKeySequence &shortcut)
{
    ui->playPauseButton->setShortcut(shortcut);
}

QKeySequence PlayerButtons::playPauseShortcut()
{
    return ui->playPauseButton->shortcut();
}

void PlayerButtons::setButtonsStyle(Qt::ToolButtonStyle style)
{
    ui->playPauseButton->setToolButtonStyle(style);
    ui->stopButton->setToolButtonStyle(style);
}

void PlayerButtons::loadPlayerState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::StoppedState:
        if (currentlyPlaying == m_mediaPlayer)
            currentlyPlaying = nullptr;

        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->stopButton->setEnabled(false);
        break;
    case QMediaPlayer::PlayingState:
        if (currentlyPlaying != nullptr)
            currentlyPlaying->pause();
        currentlyPlaying = m_mediaPlayer;

        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        ui->stopButton->setEnabled(true);
        break;
    case QMediaPlayer::PausedState:
        if (currentlyPlaying == m_mediaPlayer)
            currentlyPlaying = nullptr;

        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
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
        emit positionChanged(static_cast<double>(position) / m_mediaPlayer->duration());
    else
        emit positionChanged(0);
}
