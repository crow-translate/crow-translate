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

#ifndef CONDITIONTRANSITION_H
#define CONDITIONTRANSITION_H

#include <QAbstractTransition>

template<typename Predicate>
class ConditionTransition : public QAbstractTransition
{
public:
    ConditionTransition(Predicate function, QState *sourceState = nullptr) :
        QAbstractTransition(sourceState),
        m_function(function)
    {
    }

    bool eventTest(QEvent *) override
    {
        return m_function();
    }

    void onTransition(QEvent *) override
    {
    }

private:
    Predicate m_function;
};

#endif // CONDITIONTRANSITION_H
