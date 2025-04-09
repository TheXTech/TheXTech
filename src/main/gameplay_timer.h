/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef GAMEPLAYTIMER_H
#define GAMEPLAYTIMER_H

#include <string>

#include "xt_color.h"

class GameplayTimer
{
    bool    m_invalidContinue = false;
    bool    m_cyclesInt = false;
    // bool    m_cyclesFin = false;
    int64_t m_cyclesAtWin = 0;
    int64_t m_cyclesAtWinDisplay = 0;
    int64_t m_cyclesCurrent = 0;
    int64_t m_cyclesTotal = 0;

    bool    m_levelBlinkActive = false;
    bool    m_worldBlinkActive = false;
    int     m_blinkingFactor = 0;
    int     m_blinkingDir = 1;
    XTColor m_colorSpin;
    int     m_colorSpinHue = 0;
    void    updateColorSpin(int delta);

public:
    static std::string formatTime(int64_t t);

    GameplayTimer();

    void reset();
    void resetCurrent();

    void load();
    void save();

    void tick();

    void onBossDead();

    void render();
};

#endif // GAMEPLAYTIMER_H
