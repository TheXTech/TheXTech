/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_timer.h>

#include <Logger/logger.h>
#include "pge_delay.h"

#include "frame_timer.h"
#include "globals.h"
#include "graphics.h"


void resetFrameTimer()
{
    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    fpsTime = 0;
    cycleCount = 0;
    gameTime = 0;
    D_pLogDebugNA("Time counter reset was called");
}

bool frameSkipNeeded()
{
    return SDL_GetTicks() + SDL_floor(1000 * (1 - (cycleCount / 63.0))) > GoalTime;
}


extern void CheckActive(); // game_main.cpp

static SDL_INLINE bool canProcessFrameCond()
{
    return tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS;
}

bool canProceedFrame()
{
    tempTime = SDL_GetTicks();
    return canProcessFrameCond();
}

static SDL_INLINE void computeFrameTime1Real()
{
    if(fpsCount >= 32000)
        fpsCount = 0; // Fixes Overflow bug

    if(cycleCount >= 32000)
        cycleCount = 0; // Fixes Overflow bug

    overTime = overTime + (tempTime - (gameTime + frameRate));

    if(gameTime == 0.0)
        overTime = 0;

    if(overTime <= 1)
        overTime = 0;
    else if(overTime > 1000)
        overTime = 1000;

    gameTime = tempTime - overTime;
    overTime = (overTime - (tempTime - gameTime));
}

static SDL_INLINE void computeFrameTime2Real()
{
    if(SDL_GetTicks() > fpsTime)
    {
        if(cycleCount >= 65)
        {
            overTime = 0;
            gameTime = tempTime;
        }
        cycleCount = 0;
        fpsTime = SDL_GetTicks() + 1000;
        GoalTime = fpsTime;
//      if(Debugger == true)
//          frmLevelDebugger.lblFPS = fpsCount;
        if(ShowFPS)
            PrintFPS = fpsCount;
        fpsCount = 0;
    }
}


void computeFrameTime1()
{
    computeFrameTime1Real();
}

void computeFrameTime2()
{
    computeFrameTime2Real();
}

void runFrameLoop(LoopCall_t doLoopCallbackPre,
                  LoopCall_t doLoopCallbackPost,
                  std::function<bool(void)> condition,
                  std::function<bool ()> subCondition,
                  std::function<void ()> preTimerExtraPre,
                  std::function<void ()> preTimerExtraPost)
{
    do
    {
        if(preTimerExtraPre)
            preTimerExtraPre();

        DoEvents();
        tempTime = SDL_GetTicks();

        if(preTimerExtraPost)
            preTimerExtraPost();

        if(canProcessFrameCond())
        {
            CheckActive();
            if(doLoopCallbackPre)
                doLoopCallbackPre();

            computeFrameTime1Real();

            if(doLoopCallbackPost)
                doLoopCallbackPost(); // Run the loop callback
            DoEvents();

            computeFrameTime2Real();

            if(subCondition && subCondition())
                break;
        }

        PGE_Delay(1);
        if(!GameIsActive)
            break;// Break on quit
    } while(condition());
}
