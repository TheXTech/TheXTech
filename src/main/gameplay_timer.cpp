/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include <SDL2/SDL_stdinc.h>
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
    m_cyclesCurrent = 1;
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
    o.read("current", m_cyclesCurrent, 0);
    o.read("total", m_cyclesTotal, 0);
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
    o.setValue("current", m_cyclesCurrent);
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
        if(LevelSelect || (LevelBeatCode == 0))
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
    SuperPrint(formatTime(m_cyclesCurrent), 3, 300, 568, 1.f, 1.f, 1.f, a);
    SuperPrint(formatTime(m_cyclesTotal), 3, 300, 584, 1.f, 1.f, 1.f, a);
}
