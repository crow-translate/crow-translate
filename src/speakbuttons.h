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

#ifndef PLAYERBUTTONS_H
#define PLAYERBUTTONS_H

#include "qonlinetranslator.h"
#include "qonlinetts.h"

#include <QMediaPlayer>
#include <QWidget>

class AppSettings;

namespace Ui {
class SpeakButtons;
}

class SpeakButtons : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SpeakButtons)

public:
    explicit SpeakButtons(QWidget *parent = nullptr);
    ~SpeakButtons() override;

    QMediaPlayer *mediaPlayer() const;
    void setMediaPlayer(QMediaPlayer *mediaPlayer);
    QMediaPlaylist *playlist();

    void setSpeakShortcut(const QKeySequence &shortcut);
    QKeySequence speakShortcut() const;

    QOnlineTts::Voice voice(QOnlineTranslator::Engine engine) const;
    void setVoice(QOnlineTranslator::Engine engine, QOnlineTts::Voice voice);

    QOnlineTts::Emotion emotion(QOnlineTranslator::Engine engine) const;
    void setEmotion(QOnlineTranslator::Engine engine, QOnlineTts::Emotion emotion);

    void speak(const QString &text, QOnlineTranslator::Language lang, QOnlineTranslator::Engine engine);
    void pauseSpeaking();

public slots:
    void stopSpeaking();

signals:
    void playerMediaRequested();
    void stateChanged(QMediaPlayer::State state);
    void positionChanged(double progress);

private slots:
    void loadPlayerState(QMediaPlayer::State state);
    void onSpeakButtonPressed();
    void onPlayerPositionChanged(qint64 position);

private:
    static QMediaPlayer *s_currentlyPlaying;

    Ui::SpeakButtons *ui;
    QMediaPlayer *m_mediaPlayer = nullptr;
    QOnlineTts::Voice m_yandexVoice = QOnlineTts::NoVoice;
    QOnlineTts::Emotion m_yandexEmotion = QOnlineTts::NoEmotion;
};

#endif // PLAYERBUTTONS_H
