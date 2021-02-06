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

#include <fmt_format_ne.h>
#include "speedrunner.h"
#include "globals.h"
#include "graphics.h"

#include "gameplay_timer.h"


static      GameplayTimer s_gamePlayTimer;
int                       g_speedRunnerMode = SPEEDRUN_MODE_OFF;

void speedRun_loadStats()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.load();
}

void speedRun_saveStats()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing
    if(GameMenu || GameOutro || BattleMode)
        return; // Do nothing when out of the game

    s_gamePlayTimer.save();
}

void speedRun_resetCurrent()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.resetCurrent();
}

void speedRun_resetTotal()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.reset();
}

#define bool2alpha(b) (b ? 1.f : 0.1f)

void speedRun_render()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.render();

    const auto &c = Player[1].Controls;

#if 1
    if(c.Up)
        SuperPrint("]", 3, 8, 576);

    if(c.Down)
        SuperPrint("v", 3, 24, 584);

    if(c.Left)
        SuperPrint("<", 3, 40, 584);

    if(c.Right)
        SuperPrint(">", 3, 56, 584);

    if(c.AltJump)
        SuperPrint("A", 3, 72, 584);

    if(c.Jump)
        SuperPrint("B", 3, 88, 584);

    if(c.AltRun)
        SuperPrint("X", 3, 104, 584);

    if(c.Run)
        SuperPrint("Y", 3, 120, 584);

    if(c.Start)
    {
        SuperPrint("S", 3, 136, 584);
        SuperPrint("T", 3, 152, 584);
    }

    if(c.Drop)
    {
        SuperPrint("S", 3, 168, 584);
        SuperPrint("E", 3, 184, 584);
    }

#else
    SuperPrint("]", 3, 8, 576, 1.f, 1.f, 1.f, bool2alpha(c.Up));
    SuperPrint("v", 3, 24, 584, 1.f, 1.f, 1.f, bool2alpha(c.Down));
    SuperPrint("<", 3, 40, 584, 1.f, 1.f, 1.f, bool2alpha(c.Left));
    SuperPrint(">", 3, 56, 584, 1.f, 1.f, 1.f, bool2alpha(c.Right));
    SuperPrint("A", 3, 72, 584, 1.f, 1.f, 1.f, bool2alpha(c.AltJump));
    SuperPrint("B", 3, 88, 584, 1.f, 1.f, 1.f, bool2alpha(c.Jump));
    SuperPrint("X", 3, 104, 584, 1.f, 1.f, 1.f, bool2alpha(c.AltRun));
    SuperPrint("Y", 3, 120, 584, 1.f, 1.f, 1.f, bool2alpha(c.Run));

    SuperPrint("S", 3, 136, 584, 1.f, 1.f, 1.f, bool2alpha(c.Start));
    SuperPrint("T", 3, 152, 584, 1.f, 1.f, 1.f, bool2alpha(c.Start));

    SuperPrint("S", 3, 168, 584, 1.f, 1.f, 1.f, bool2alpha(c.Drop));
    SuperPrint("E", 3, 184, 584, 1.f, 1.f, 1.f, bool2alpha(c.Drop));
#endif

}

#undef bool2alpha

void speedRun_tick()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.tick();
}

void speedRun_bossDeadEvent()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.onBossDead();
}

void speedRun_setSemitransparentRender(bool r)
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.setSemitransparent(r);
}
