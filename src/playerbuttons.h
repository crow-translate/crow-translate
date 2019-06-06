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
