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

#include "sdl_proxy/sdl_timer.h"

#include "../gfx.h"
#include "../globals.h"
#include "../config.h"
#include "../graphics.h"
#include "../player.h"
#include "../frame_timer.h"
#include "../screen_fader.h"
#include "../core/render.h"
#include "../core/events.h"
#include "../controls.h"
#include "pge_delay.h"
#include "npc_id.h"

#include "main/hints.h"


static void initPlayers(Player_t tempPlayer[maxLocalPlayers])
{
    for(int A = 0; A < numPlayers && A < maxLocalPlayers; ++A)
    {
        auto &ref = Player[A + 1];
        auto &dst = tempPlayer[A];

        dst.Character = ref.Character;
        dst.State = ref.State;
        dst.Hearts = ref.Hearts;
        dst.Mount = ref.Mount;
        dst.MountType = ref.MountType;

        if(dst.Character == 0)
        {
            dst.Character = 1; // Sets as Mario
            if(numPlayers == 2 && A == 1 /*&& nPlay.Online == false*/) // Sets as Luigi
                dst.Character = 2;
        }

        if(dst.State == 0) // if no state it defaults to small mario
            dst.State = 1;

        if(dst.Character == 3 || dst.Character == 4 || dst.Character == 5) // Peach and Toad
        {
            if(dst.Hearts <= 0)
                dst.Hearts = 1;

            if(dst.Hearts <= 1 && dst.State > 1 && dst.Character != 5)
                dst.Hearts = 2;

            if(dst.HeldBonus > 0)
            {
                dst.Hearts += 1;
                dst.HeldBonus = NPCID(0);
            }

            if(dst.State == 1 && dst.Hearts > 1)
                dst.State = 2;

            if(dst.Hearts > 3)
                dst.Hearts = 3;

            if(dst.Mount == 3)
                dst.Mount = 0;
        }
        else // Mario and Luigi
        {
            if(dst.Hearts == 3 && dst.HeldBonus == 0)
                dst.HeldBonus = NPCID_POWER_S3;
            dst.Hearts = 0;
        }

        if(dst.Character == 5) // Link
            dst.Mount = 0;

        dst.Location.Width = Physics.PlayerWidth[dst.Character][dst.State]; // set height
        dst.Location.Height = Physics.PlayerHeight[dst.Character][dst.State]; // set width
        if(dst.State == 1 && dst.Mount == 1) // if small and in a shoe then set the height to super mario
            dst.Location.Height = Physics.PlayerHeight[1][2];

        // reset all variables
        if(dst.Mount == 2)
            dst.Mount = 0;

        if(dst.Character >= 3 && dst.Mount == 3)
            dst.Mount = 0;

        dst.Frame = 1;

        if(dst.Character == 3)
            dst.CanFloat = true;

        if(dst.Character == 3 || dst.Character == 4)
        {
            if(dst.State == 1)
                dst.Hearts = 1;

            if(dst.State > 1 && dst.Hearts < 2)
                dst.Hearts = 2;
        }

        dst.Section = -1;

        dst.Location.SpeedY = 0;
    }

    if(numPlayers == 1 || g_ClonedPlayerMode)
    {
        tempPlayer[0].Location.X = XRender::TargetW / 2.0 - tempPlayer[0].Location.Width / 2.0;
        tempPlayer[0].Location.Y = XRender::TargetH / 2.0 - tempPlayer[0].Location.Height + 24;
        tempPlayer[0].Direction = 1;
    }
    else if(numPlayers == 2)
    {
        tempPlayer[0].Location.X = XRender::TargetW / 2.0 - tempPlayer[0].Location.Width / 2.0 - 30;
        tempPlayer[0].Location.Y = XRender::TargetH / 2.0 - tempPlayer[0].Location.Height + 24;
        tempPlayer[0].Direction = -1;

        tempPlayer[1].Location.X = XRender::TargetW / 2.0 - tempPlayer[1].Location.Width / 2.0 + 32;
        tempPlayer[1].Location.Y = XRender::TargetH / 2.0 - tempPlayer[1].Location.Height + 24;
        tempPlayer[1].Direction = 1;
    }
    else
    {
        int start_x = XRender::TargetW / 2.0 - (numPlayers - 1) * 32;
        for(int i = 0; i < numPlayers && i < maxLocalPlayers; i++)
        {
            tempPlayer[i].Location.X = start_x + 64 * i - tempPlayer[i].Location.Width / 2.0;
            tempPlayer[i].Location.Y = XRender::TargetH / 2.0 - tempPlayer[i].Location.Height + 24;
            tempPlayer[i].Direction = 1;
        }
    }

    for(int i = 0; i < numPlayers && i < maxLocalPlayers; i++)
        PlayerFrame(tempPlayer[i]);
}


static void drawEnterScreen(Player_t tempPlayer[maxLocalPlayers])
{
    for(int A = 0; A < numPlayers && A < maxLocalPlayers; ++A)
    {
        DrawPlayer(tempPlayer[A], 0);

        if(g_ClonedPlayerMode)
            break;
    }

    if(TestLevel)
        SuperPrintScreenCenter("LOADING...", 3, XRender::TargetH / 2.0f + 32);
    else
        DrawLives(XRender::TargetW / 2 - 14, XRender::TargetH / 2 + 31, Lives, g_100s);

    XHints::Draw(XRender::TargetH / 2.0 + 64, 0);
}


void GameThing(int waitms, int fadeSpeed)
{
    Player_t tempPlayer[maxLocalPlayers];
    initPlayers(tempPlayer);

    XHints::Select();

    if(waitms <= 0)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();
        drawEnterScreen(tempPlayer);
        XRender::repaint();
        XEvents::doEvents();
    }
    else
    {
        ScreenFader fader;
        uint32_t targetTime = SDL_GetTicks() + waitms;

        if(g_config.EnableInterLevelFade && fadeSpeed > 0)
            fader.setupFader(fadeSpeed, 65, 0, ScreenFader::S_FADE);

        while(SDL_GetTicks() < targetTime && GameIsActive)
        {
            XEvents::doEvents();

            if(canProceedFrame())
            {
                computeFrameTime1();

                Controls::Update(false);

                XRender::setTargetTexture();
                XRender::clearBuffer();
                drawEnterScreen(tempPlayer);

                if(fadeSpeed > 0 && fader.m_active)
                {
                    fader.update();
                    fader.draw();
                }

                XRender::repaint();
                XEvents::doEvents();
                computeFrameTime2();
            }
            if(!MaxFPS)
                PGE_Delay(1);
        }
    }
}
