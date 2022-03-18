/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

class GameplayTimer
{
    static std::string formatTime(int64_t t);
    bool    m_cyclesInt = false;
    bool    m_cyclesFin = false;
    int64_t m_cyclesCurrent = 0;
    int64_t m_cyclesTotal = 0;

    bool    m_levelBlinkActive = false;
    bool    m_worldBlinkActive = false;
    float   m_blinkingFactor = 0.f;
    float   m_blinkingDir = 1.0f;
    float   m_colorSpin[3] = {0.0f, 0.8f, 0.8f};
    float   m_colorSpinHue = 0.f;
    void    updateColorSpin(float delta);

    bool    m_semiTransparent = false;

public:
    GameplayTimer();

    void setSemitransparent(bool t);
    bool semitransparent();

    void reset();
    void resetCurrent();

    void load();
    void save();

    void tick();

    void onBossDead();

    void render();
};

#endif // GAMEPLAYTIMER_H
