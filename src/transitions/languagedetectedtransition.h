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

#ifndef LANGUAGEDETECTEDTRANSITION_H
#define LANGUAGEDETECTEDTRANSITION_H

#include <QAbstractTransition>

class LangButtonGroup;

class LanguageDetectedTransition : public QAbstractTransition
{
public:
    explicit LanguageDetectedTransition(LangButtonGroup *group, QState *sourceState = nullptr);

protected:
    bool eventTest(QEvent *) override;
    void onTransition(QEvent *) override;

private:
    LangButtonGroup *m_group;
};

#endif // LANGUAGEDETECTEDTRANSITION_H
