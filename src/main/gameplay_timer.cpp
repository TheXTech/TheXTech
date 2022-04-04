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


#include <SDL2/SDL_stdinc.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

#include "gameplay_timer.h"
#include "graphics.h"
#include "game_main.h"
#include "globals.h"
#include "presetup.h"
#include "compat.h"


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

void GameplayTimer::updateAllowBlink()
{
    int effectBlink = (m_blinkEffect == SPEEDRUN_EFFECT_BLINK_UNDEFINED) ?
                          g_compatibility.speedrun_blink_effect :
                          m_blinkEffect;

    switch(effectBlink)
    {
    default:
    case SPEEDRUN_EFFECT_BLINK_OPAQUEONLY:
        m_allowBlink = !m_semiTransparent;
        break;

    case SPEEDRUN_EFFECT_BLINK_ALWAYS:
        m_allowBlink = true;
        break;

    case SPEEDRUN_EFFECT_BLINK_NEVER:
        m_allowBlink = false;
        break;
    }
}

void GameplayTimer::setSemitransparent(bool t)
{
    m_semiTransparent = t;
}

void GameplayTimer::setBlinkEffect(int be)
{
    m_blinkEffect = be;
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
    m_levelBlinkActive = false;
    m_worldBlinkActive = false;
    m_blinkingFactor = 0.0f;
    updateAllowBlink();
}

void GameplayTimer::resetCurrent()
{
    m_cyclesCurrent = 0;
    m_levelBlinkActive = false;
    m_worldBlinkActive = false;
    m_blinkingFactor = 0.0f;
    updateAllowBlink();
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
    updateAllowBlink();
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
        m_levelBlinkActive = false;
        m_worldBlinkActive = false;
        m_blinkingFactor = 0.0f;
    }
    else
    {
        if(LevelSelect || (!LevelSelect && LevelMacro == 0))
            m_cyclesCurrent += 1;
        else if(m_allowBlink && !m_levelBlinkActive)
            m_levelBlinkActive = true;

        if(!m_cyclesFin)
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
}

void GameplayTimer::onBossDead()
{
    m_cyclesFin = true;
    if(m_allowBlink)
        m_worldBlinkActive = true;
}

void GameplayTimer::render()
{
    float a = m_semiTransparent ? 0.5f : 1.f;
    // int x = (ScreenW / 2) - (144 / 2);
    int y = ScreenH;

    float lc[3] =
    {
        m_levelBlinkActive ? m_colorSpin[0] : 0.6f,
        m_levelBlinkActive ? m_colorSpin[1] : 0.6f,
        m_levelBlinkActive ? m_colorSpin[2] : 0.6f
    };
    float wc = m_worldBlinkActive ? 0.5f + m_blinkingFactor : 1.f;

    SuperPrintScreenCenter(formatTime(m_cyclesCurrent), 3, y - 34, lc[0], lc[1], lc[2], a);
    SuperPrintScreenCenter(formatTime(m_cyclesTotal),   3, y - 18, wc, 1.0f, wc, a);
}
