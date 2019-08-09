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

#include "qonlinetranslator.h"

#include <QWidget>
#include <QMediaPlayer>

namespace Ui {
class PlayerButtons;
}

class PlayerButtons : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlayerButtons)

public:
    explicit PlayerButtons(QWidget *parent = nullptr);
    ~PlayerButtons() override;

    QMediaPlayer *mediaPlayer() const;
    void setMediaPlayer(QMediaPlayer *mediaPlayer);
    QMediaPlaylist *playlist();

    void play(const QString &text, QOnlineTranslator::Language language, QOnlineTranslator::Engine engine);

    void play();
    void pause();
    void stop();

    void setPlayPauseShortcut(const QKeySequence &shortcut);
    QKeySequence playPauseShortcut();

    void setButtonsStyle(Qt::ToolButtonStyle style);

signals:
    void playerMediaRequested();
    void stateChanged(QMediaPlayer::State state);
    void positionChanged(double progress);

private slots:
    void loadPlayerState(QMediaPlayer::State state);
    void processPlayPausePressed();
    void processPositionChanged(qint64 position);

private:
    Ui::PlayerButtons *ui;
    QMediaPlayer *m_mediaPlayer = nullptr;

    static QMediaPlayer *currentlyPlaying;
};

#endif // PLAYERBUTTONS_H
