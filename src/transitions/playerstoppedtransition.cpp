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

#include "playerstoppedtransition.h"

#include <QMediaPlayer>
#include <QStateMachine>

PlayerStoppedTransition::PlayerStoppedTransition(QMediaPlayer *player, QState *sourceState)
    : QSignalTransition(player, &QMediaPlayer::stateChanged, sourceState)
{
}

bool PlayerStoppedTransition::eventTest(QEvent *event)
{
    if (!QSignalTransition::eventTest(event))
        return false;

    auto *signalEvent = dynamic_cast<QStateMachine::SignalEvent *>(event);
    return signalEvent->arguments().first().toInt() == QMediaPlayer::StoppedState;
}

