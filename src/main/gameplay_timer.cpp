/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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


#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "core/render.h"

#include "gameplay_timer.h"
#include "graphics.h"
#include "game_main.h"
#include "globals.h"
#include "config.h"

#include "menu_main.h"

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

void GameplayTimer::updateColorSpin(float delta)
{
    float in_s = 0.7f;
    float in_v = 1.0f;
    double      hh, p, q, t, ff;
    long        i;

    m_colorSpinHue += delta;
    if(m_colorSpinHue >= 360.f)
        m_colorSpinHue -= 360.f;
    else if(m_colorSpinHue < 0.f)
        m_colorSpinHue += 360.f;

    if(in_s <= 0.0) // < is bogus, just shuts up warnings
    {
        m_colorSpin[0] = in_v;
        m_colorSpin[1] = in_v;
        m_colorSpin[2] = in_v;
        return;
    }

    hh = m_colorSpinHue;
    if(hh >= 360.0f)
        hh = 0.0f;

    hh /= 60.0f;
    i = (long)hh;
    ff = hh - i;
    p = in_v * (1.0f - in_s);
    q = in_v * (1.0f - (in_s * ff));
    t = in_v * (1.0f - (in_s * (1.0f - ff)));

    switch(i)
    {
    case 0:
        m_colorSpin[0] = in_v;
        m_colorSpin[1] = t;
        m_colorSpin[2] = p;
        break;
    case 1:
        m_colorSpin[0] = q;
        m_colorSpin[1] = in_v;
        m_colorSpin[2] = p;
        break;
    case 2:
        m_colorSpin[0] = p;
        m_colorSpin[1] = in_v;
        m_colorSpin[2] = t;
        break;
    case 3:
        m_colorSpin[0] = p;
        m_colorSpin[1] = q;
        m_colorSpin[2] = in_v;
        break;
    case 4:
        m_colorSpin[0] = t;
        m_colorSpin[1] = p;
        m_colorSpin[2] = in_v;
        break;
    case 5:
    default:
        m_colorSpin[0] = in_v;
        m_colorSpin[1] = p;
        m_colorSpin[2] = q;
        break;
    }
}

GameplayTimer::GameplayTimer() = default;


void GameplayTimer::reset()
{
    m_invalidContinue = false;
    m_cyclesInt = false;
    m_cyclesAtWin = 0;
    m_cyclesAtWinDisplay = 0;
    m_cyclesCurrent = 0;
    m_cyclesTotal = 0;
    m_levelBlinkActive = false;
    m_worldBlinkActive = false;
    m_blinkingFactor = 0.0f;
}

void GameplayTimer::resetCurrent()
{
    m_cyclesCurrent = 0;
    m_levelBlinkActive = false;
    m_worldBlinkActive = false;
    m_blinkingFactor = 0.0f;
}

void GameplayTimer::load()
{
    if(TestLevel || selSave == 0)
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
    o.read("total", m_cyclesTotal, 0);

    // no longer permanently stop the timer following credits roll, but do store the cycles at first win for future display
    bool legacy_fin = false;
    o.read("fin", legacy_fin, false);
    int64_t cyclesAtWin_default = (legacy_fin) ? m_cyclesTotal : 0;

    o.read("win", m_cyclesAtWin, cyclesAtWin_default);
    m_cyclesCurrent = 0; // Reset the counter
    m_cyclesAtWinDisplay = 0;
    o.endGroup();

    if(!m_cyclesInt)
        m_invalidContinue = true;
}

void GameplayTimer::save()
{
    // should Cheater also be a condition here?
    if(TestLevel || !selSave || m_invalidContinue)
        return;

    IniProcessing o;
    std::string savePath = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                            SelectWorld[selWorld].WorldFile,
                                            fmt::format_ne("timers{0}.ini", selSave));
    o.open(savePath);

    o.beginGroup("timers");
    o.setValue("int", m_cyclesInt);
    o.setValue("total", m_cyclesTotal);
    o.setValue("win", m_cyclesAtWin);
    o.endGroup();

    o.writeIniFile();
}

void GameplayTimer::tick()
{
    // initialize timer
    if(!m_cyclesInt)
    {
        m_cyclesInt = true;
        m_cyclesCurrent = 0;
        m_cyclesTotal = 0;
        m_cyclesAtWin = 0;
        m_cyclesAtWinDisplay = 0;
        m_levelBlinkActive = false;
        m_worldBlinkActive = false;
        m_blinkingFactor = 0.0f;
    }

    bool in_leveltest_restart_screen = (GamePaused == PauseCode::PauseScreen && LevelBeatCode < 0);
    bool in_normal_level_play = (!LevelSelect && LevelMacro == 0);

    if(!in_leveltest_restart_screen && (LevelSelect || in_normal_level_play))
        m_cyclesCurrent += 1;
    else if(!in_leveltest_restart_screen && g_config.show_playtime_counter == Config_t::PLAYTIME_COUNTER_ANIMATED)
        m_levelBlinkActive = true;

    m_cyclesTotal += 1;

    if(m_levelBlinkActive)
        updateColorSpin(5.0f);

    if(m_worldBlinkActive)
    {
        m_blinkingFactor += m_blinkingDir * 0.02;
        if(m_blinkingFactor >= 0.3f || m_blinkingFactor <= -0.3f)
            m_blinkingDir *= -1.0f;
    }
}

void GameplayTimer::onBossDead()
{
    m_cyclesAtWinDisplay = m_cyclesTotal;

    if(m_cyclesAtWin == 0)
        m_cyclesAtWin = m_cyclesTotal;

    if(g_config.show_playtime_counter == Config_t::PLAYTIME_COUNTER_ANIMATED)
        m_worldBlinkActive = true;
}

void GameplayTimer::render()
{
    float a = (g_config.show_playtime_counter == Config_t::PLAYTIME_COUNTER_SUBTLE) ? 0.5f : 1.f;
    // int x = (XRender::TargetW / 2) - (144 / 2);
    int y = XRender::TargetH;

    float lc[3] =
    {
        m_levelBlinkActive ? m_colorSpin[0] : 0.6f,
        m_levelBlinkActive ? m_colorSpin[1] : 0.6f,
        m_levelBlinkActive ? m_colorSpin[2] : 0.6f
    };
    float wc = m_worldBlinkActive ? 0.5f + m_blinkingFactor : 1.f;

    SuperPrintScreenCenter(formatTime(m_cyclesCurrent), 3, y - 34, XTColorF(lc[0], lc[1], lc[2], a));

    if(m_invalidContinue)
        SuperPrintScreenCenter(g_mainMenu.caseNone,         3, y - 18, XTColorF(0.5f, 0.5f, 0.5f, a));
    else if(!TestLevel)
    {
        int64_t use_total = m_cyclesTotal;
        if(m_cyclesAtWinDisplay)
            use_total = m_cyclesAtWinDisplay;

        SuperPrintScreenCenter(formatTime(use_total),       3, y - 18, XTColorF(wc, 1.0f, wc, a));
    }
}
