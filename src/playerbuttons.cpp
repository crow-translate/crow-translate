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
    QOnlineTts onlineTts;
    onlineTts.generateUrls(text, engine, language, voice(engine), emotion(engine));
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

QKeySequence PlayerButtons::playPauseShortcut() const
{
    return ui->playPauseButton->shortcut();
}

QOnlineTts::Voice PlayerButtons::voice(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        return m_yandexVoice;
    default:
        return QOnlineTts::NoVoice;
    }
}

void PlayerButtons::setVoice(QOnlineTranslator::Engine engine, QOnlineTts::Voice voice)
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        m_yandexVoice = voice;
        break;
    default:
        break;
    }
}

QOnlineTts::Emotion PlayerButtons::emotion(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        return m_yandexEmotion;
    default:
        return QOnlineTts::NoEmotion;
    }
}

void PlayerButtons::setEmotion(QOnlineTranslator::Engine engine, QOnlineTts::Emotion emotion)
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        m_yandexEmotion = emotion;
        break;
    default:
        break;
    }
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
