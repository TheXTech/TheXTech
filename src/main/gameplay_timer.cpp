/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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


#ifndef NO_SDL
#include <SDL2/SDL_stdinc.h>
#else
#include "SDL_supplement.h"
#endif

#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

#include "gameplay_timer.h"
#include "graphics.h"
#include "game_main.h"
#include "globals.h"


std::string GameplayTimer::formatTime(int64_t t)
{
    std::string displayTime;

    int64_t realMiliseconds = (int64_t)SDL_floor(double(t) * 15.6);
    int miliseconds = realMiliseconds % 1000;
    int64_t realSeconds = (int64_t)SDL_floor(double(realMiliseconds) / 1000.0);
    int seconds = realSeconds % 60;
    int64_t realMinutes = (int64_t)SDL_floor(double(realSeconds) / 60.0);
    int minutes = realMinutes % 60;
    int64_t realHours = (int64_t)SDL_floor(double(realMinutes) / 60.0);
    int hours = realHours % 24;
    int days = (int)SDL_floor(realHours / 24.0);

    if(days >= 1)
        displayTime = fmt::sprintf_ne("%02d:%02d:%02d:%02d.%03d", days, hours, minutes, seconds, miliseconds);
    else if(realHours >= 1)
        displayTime = fmt::sprintf_ne("%02d:%02d:%02d.%03d", hours, minutes, seconds, miliseconds);
    else if(realHours < 1)
        displayTime = fmt::sprintf_ne("%02d:%02d.%03d", minutes, seconds, miliseconds);

    return displayTime;
}

void GameplayTimer::setSemitransparent(bool t)
{
    m_semiTransparent = t;
}

bool GameplayTimer::semitransparent()
{
    return m_semiTransparent;
}

GameplayTimer::GameplayTimer() = default;


void GameplayTimer::reset()
{
    m_cyclesInt = false;
    m_cyclesFin = false;
    m_cyclesCurrent = 0;
    m_cyclesTotal = 0;
}

void GameplayTimer::resetCurrent()
{
    m_cyclesCurrent = 0;
}

void GameplayTimer::load()
{
    if(TestLevel)
    {
        reset();
        return;
    }

    IniProcessing o;
    std::string savePath = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                            SelectWorld[selWorld].WorldFile,
                                            fmt::format_ne("timers{0}.ini", selSave));
    o.open(savePath);

    o.beginGroup("timers");
    o.read("int", m_cyclesInt, false);
    o.read("fin", m_cyclesFin, false);
    o.read("total", m_cyclesTotal, 0);
    m_cyclesCurrent = 0; // Reset the counter
    o.endGroup();
}

void GameplayTimer::save()
{
    if(TestLevel)
        return;

    IniProcessing o;
    std::string savePath = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                            SelectWorld[selWorld].WorldFile,
                                            fmt::format_ne("timers{0}.ini", selSave));
    o.open(savePath);

    o.beginGroup("timers");
    o.setValue("int", m_cyclesInt);
    o.setValue("fin", m_cyclesFin);
    o.setValue("total", m_cyclesTotal);
    o.endGroup();

    o.writeIniFile();
}

void GameplayTimer::tick()
{
    if(!m_cyclesInt)
    {
        m_cyclesInt = true;
        m_cyclesCurrent = 1;
        m_cyclesTotal = 1;
        m_cyclesFin = 0;
    }
    else
    {
        if(LevelSelect || (!LevelSelect && LevelMacro == 0))
            m_cyclesCurrent += 1;

        if(!m_cyclesFin)
            m_cyclesTotal += 1;
    }
}

void GameplayTimer::onBossDead()
{
    m_cyclesFin = true;
}

void GameplayTimer::render()
{
    float a = m_semiTransparent ? 0.5f : 1.f;
    // int x = (ScreenW / 2) - (144 / 2);
    int y = ScreenH;
    SuperPrintScreenCenter(formatTime(m_cyclesCurrent), 3, y - 34, 1.f, 1.f, 1.f, a);
    SuperPrintScreenCenter(formatTime(m_cyclesTotal),   3, y - 18, 1.f, 1.f, 1.f, a);
}
