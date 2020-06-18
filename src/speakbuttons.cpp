/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#include "speakbuttons.h"
#include "ui_speakbuttons.h"

#include "settings/appsettings.h"

#include <QMediaPlaylist>
#include <QMessageBox>

QMediaPlayer *SpeakButtons::s_currentlyPlaying = nullptr;

SpeakButtons::SpeakButtons(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SpeakButtons)
{
    ui->setupUi(this);

    connect(ui->playPauseButton, &QAbstractButton::clicked, this, &SpeakButtons::processSpeakPressed);
    connect(ui->stopButton, &QAbstractButton::clicked, this, &SpeakButtons::stopSpeaking);
}

SpeakButtons::~SpeakButtons()
{
    delete ui;
}

QMediaPlayer *SpeakButtons::mediaPlayer() const
{
    return m_mediaPlayer;
}

void SpeakButtons::setMediaPlayer(QMediaPlayer *mediaPlayer)
{
    if (m_mediaPlayer != nullptr) {
        disconnect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &SpeakButtons::loadPlayerState);
        disconnect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &SpeakButtons::stateChanged);
        disconnect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &SpeakButtons::processPositionChanged);
    }

    m_mediaPlayer = mediaPlayer;
    if (m_mediaPlayer->playlist() == nullptr)
        m_mediaPlayer->setPlaylist(new QMediaPlaylist);

    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &SpeakButtons::processPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &SpeakButtons::loadPlayerState);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &SpeakButtons::stateChanged);

    loadPlayerState(m_mediaPlayer->state());
}

QMediaPlaylist *SpeakButtons::playlist()
{
    return m_mediaPlayer->playlist();
}

void SpeakButtons::setSpeakShortcut(const QKeySequence &shortcut)
{
    ui->playPauseButton->setShortcut(shortcut);
}

QKeySequence SpeakButtons::speakShortcut() const
{
    return ui->playPauseButton->shortcut();
}

QOnlineTts::Voice SpeakButtons::voice(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        return m_yandexVoice;
    default:
        return QOnlineTts::NoVoice;
    }
}

void SpeakButtons::setVoice(QOnlineTranslator::Engine engine, QOnlineTts::Voice voice)
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        m_yandexVoice = voice;
        break;
    default:
        break;
    }
}

QOnlineTts::Emotion SpeakButtons::emotion(QOnlineTranslator::Engine engine) const
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        return m_yandexEmotion;
    default:
        return QOnlineTts::NoEmotion;
    }
}

void SpeakButtons::setEmotion(QOnlineTranslator::Engine engine, QOnlineTts::Emotion emotion)
{
    switch (engine) {
    case QOnlineTranslator::Yandex:
        m_yandexEmotion = emotion;
        break;
    default:
        break;
    }
}

void SpeakButtons::speak(const QString &text, QOnlineTranslator::Language lang, QOnlineTranslator::Engine engine)
{
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("No text specified"), tr("Playback text is empty"));
        return;
    }

    QOnlineTts onlineTts;
    onlineTts.generateUrls(text, engine, lang, voice(engine), emotion(engine));
    if (onlineTts.error() != QOnlineTts::NoError) {
        QMessageBox::critical(this, tr("Unable to generate URLs for TTS"), onlineTts.errorString());
        return;
    }

    // Use playlist to split long queries due engines limit
    const QList<QMediaContent> media = onlineTts.media();
    playlist()->clear();
    playlist()->addMedia(media);
    m_mediaPlayer->play();
}

void SpeakButtons::pauseSpeaking()
{
    m_mediaPlayer->pause();
}

void SpeakButtons::stopSpeaking()
{
    m_mediaPlayer->stop();
}

void SpeakButtons::loadPlayerState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::StoppedState:
        if (s_currentlyPlaying == m_mediaPlayer)
            s_currentlyPlaying = nullptr;

        ui->playPauseButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
        ui->stopButton->setEnabled(false);
        break;
    case QMediaPlayer::PlayingState:
        if (s_currentlyPlaying != nullptr)
            s_currentlyPlaying->pause();
        s_currentlyPlaying = m_mediaPlayer;

        ui->playPauseButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
        ui->stopButton->setEnabled(true);
        break;
    case QMediaPlayer::PausedState:
        if (s_currentlyPlaying == m_mediaPlayer)
            s_currentlyPlaying = nullptr;

        ui->playPauseButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
        break;
    }
}

void SpeakButtons::processSpeakPressed()
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

void SpeakButtons::processPositionChanged(qint64 position)
{
    if (m_mediaPlayer->duration() != 0)
        emit positionChanged(static_cast<double>(position) / m_mediaPlayer->duration());
    else
        emit positionChanged(0);
}
