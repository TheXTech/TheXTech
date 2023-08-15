/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <sdl_proxy/sdl_stdinc.h>

#include "../globals.h"
#include "../graphics.h"
#include "../player.h"
#include "../sound.h"
#include "../change_res.h"
#include "../load_gfx.h"
#include "../core/window.h"
#include "../compat.h"
#include "config.h"

#include "../compat.h"
#include "config.h"
#include "core/render.h"

void SetScreenType()
{
    // moved this code from game_main.cpp, but it occured elsewhere also
    //   it was always called before setup screens, now it is a part of setup screens.
    //   better to have it in one place so it can be updated
    if(numPlayers == 1)
        ScreenType = 0; // Follow 1 player
    else if(numPlayers == 2)
        ScreenType = 5; // Dynamic screen
    else
    {
        // ScreenType = 3 'Average, no one leaves the screen
        ScreenType = 2; // Average
    }

    // special cases
    if(SingleCoop > 0)
        ScreenType = 6;
    if(GameMenu)
        ScreenType = 2;
    if(GameOutro)
        ScreenType = 7;
    if(LevelEditor)
        ScreenType = 0;
}

// Sets up the split lines
void SetupScreens(bool reset)
{
    SetScreenType();

    vScreen[1].player = 1;
    vScreen[2].player = 2;

    switch(ScreenType)
    {
    case 0: // Follows Player 1
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = false;
        break;
    case 1: // Split Screen vertical
        vScreen[1].Height = ScreenH / 2;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH / 2;
        vScreen[2].Width = ScreenW;
        vScreen[2].Left = 0;
        vScreen[2].Top = ScreenH / 2;
        break;
    case 2: // Follows all players
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = false;
        break;
    case 3: // Follows all players. Noone leaves the screen
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = false;
        break;
    case 4: // Split Screen horizontal
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW / 2;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH;
        vScreen[2].Width = ScreenW / 2;
        vScreen[2].Left = ScreenW / 2;
        vScreen[2].Top = 0;
        break;
    case 5: // Dynamic screen detection
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;

        if(reset)
            vScreen[2].Visible = false;
        break;
    case 6: // VScreen Coop
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH;
        vScreen[2].Width = ScreenW;
        vScreen[2].Left = 0;
        vScreen[2].Top = 0;
        break;
    case 7: // Credits
    case 8: // netplay
        vScreen[1].Left = 0;
        vScreen[1].Height = ScreenH;
        vScreen[1].Top = 0;
        vScreen[1].Width = ScreenW;
        vScreen[2].Visible = false;
        break;
#if 0 // Merged with the branch above because they both are same
    case 8: // netplay
        vScreen[1].Left = 0;
        vScreen[1].Width = 800;
        vScreen[1].Top = 0;
        vScreen[1].Height = 600;
        vScreen[2].Visible = false;
        break;
#endif
//    End If
    }
}

void DynamicScreen(Screen_t& screen)
{
    int A = 0;

    vScreen_t& vscreen1 = screen.vScreen(1);
    vScreen_t& vscreen2 = screen.vScreen(2);

    if(g_compatibility.free_level_res)
        GetvScreenAverage(vscreen1);
    else
    {
        double l, t;
        GetvScreenAverageCanonical(&l, &t, true);
        vscreen1.X = l;
        vscreen1.Y = t;
    }

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Effect == 6)
            return;
    }

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 0;
    }

    Player_t& p1 = Player[1];
    Player_t& p2 = Player[2];

    if(CheckDead() == 0)
    {
        if(p1.Section == p2.Section)
        {
            double FieldW = g_compatibility.free_level_res ? screen.W : 800;
            double FieldH = g_compatibility.free_level_res ? screen.H : 600;

            if(level[Player[1].Section].Width - level[Player[1].Section].X > FieldW && (((!vscreen2.Visible && Player[2].Location.X + vscreen1.X >= FieldW * 0.75 - Player[2].Location.Width / 2.0) || (vscreen2.Visible && Player[2].Location.X + vscreen1.X >= FieldW * 0.75 - Player[2].Location.Width / 2.0)) && (Player[1].Location.X < level[Player[1].Section].Width - FieldW * 0.75 - Player[1].Location.Width / 2.0)))
            {
                vscreen2.Height = screen.H;
                vscreen2.Width = screen.W / 2;
                vscreen2.Left = screen.W / 2.0;
                vscreen2.Top = 0;
                vscreen1.Height = screen.H;
                vscreen1.Width = screen.W / 2;
                vscreen1.Left = 0;
                vscreen1.Top = 0;

                double l, t;
                if(g_compatibility.free_level_res)
                {
                    GetvScreenAverage2(vscreen1);
                    t = vscreen1.Y;
                }
                else
                {
                    GetvScreenAverage2Canonical(&l, &t, true);
                }

                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);

                    vscreena.TempDelay = 200;
                    vscreena.tempX = 0;
                    vscreena.TempY = -t + FieldH * 0.5 - Player[A].Location.Y - vScreenYOffset - Player[A].Location.Height;
                }

                if(screen.DType != 1)
                    PlaySound(SFX_Camera);
                screen.DType = 1;
                vscreen2.Visible = true;
            }
            else if(level[Player[1].Section].Width - level[Player[1].Section].X > FieldW && (((!vscreen2.Visible && Player[1].Location.X + vscreen1.X >= FieldW * 0.75 - Player[1].Location.Width / 2.0) || (vscreen2.Visible && Player[1].Location.X + vscreen2.X >= FieldW * 0.75 - Player[1].Location.Width / 2.0)) && (Player[2].Location.X < level[Player[1].Section].Width - FieldW * 0.75 - Player[2].Location.Width / 2.0)))
            {
                vscreen1.Height = screen.H;
                vscreen1.Width = screen.W / 2;
                vscreen1.Left = screen.W / 2.0;
                vscreen1.Top = 0;
                vscreen2.Height = screen.H;
                vscreen2.Width = screen.W / 2;
                vscreen2.Left = 0;
                vscreen2.Top = 0;

                double l, t;
                if(g_compatibility.free_level_res)
                {
                    GetvScreenAverage2(vscreen1);
                    t = vscreen1.Y;
                }
                else
                {
                    GetvScreenAverage2Canonical(&l, &t, true);
                }

                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);

                    vscreena.TempDelay = 200;
                    vscreena.tempX = 0;
                    vscreena.TempY = -t + FieldH * 0.5 - Player[A].Location.Y - vScreenYOffset - Player[A].Location.Height;
                }

                if(screen.DType != 2)
                    PlaySound(SFX_Camera);
                screen.DType = 2;
                vscreen2.Visible = true;
            }
            else if(level[Player[1].Section].Height - level[Player[1].Section].Y > FieldH && ((!vscreen2.Visible && Player[1].Location.Y + vscreen1.Y >= FieldH * 0.75 - vScreenYOffset - Player[1].Location.Height) || (vscreen2.Visible && Player[1].Location.Y + vscreen2.Y >= FieldH * 0.75 - vScreenYOffset - Player[1].Location.Height)) && (Player[2].Location.Y < level[Player[1].Section].Height - FieldH * 0.75 - vScreenYOffset - Player[2].Location.Height))
            {
                vscreen1.Height = screen.H / 2;
                vscreen1.Width = screen.W;
                vscreen1.Left = 0;
                vscreen1.Top = screen.H / 2.0;
                vscreen2.Height = screen.H / 2;
                vscreen2.Width = screen.W;
                vscreen2.Left = 0;
                vscreen2.Top = 0;

                double l, t;
                if(g_compatibility.free_level_res)
                {
                    GetvScreenAverage2(vscreen1);
                    l = vscreen1.X;
                }
                else
                {
                    GetvScreenAverage2Canonical(&l, &t, true);
                }

                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);

                    vscreena.TempDelay = 200;
                    vscreena.TempY = 0;
                    vscreena.tempX = -l + FieldW * 0.5 - Player[A].Location.X - Player[A].Location.Width * 0.5;
                }

                if(screen.DType != 3)
                    PlaySound(SFX_Camera);
                screen.DType = 3;
                vscreen2.Visible = true;
            }
            else if(level[Player[1].Section].Height - level[Player[1].Section].Y > FieldH && ((!vscreen2.Visible && Player[2].Location.Y + vscreen1.Y >= FieldH * 0.75 - vScreenYOffset - Player[2].Location.Height) || (vscreen2.Visible && Player[2].Location.Y + vscreen1.Y >= FieldH * 0.75 - vScreenYOffset - Player[2].Location.Height)) && (Player[1].Location.Y < level[Player[1].Section].Height - FieldH * 0.75 - vScreenYOffset - Player[1].Location.Height))
            {
                vscreen1.Height = screen.H / 2;
                vscreen1.Width = screen.W;
                vscreen1.Left = 0;
                vscreen1.Top = 0;
                vscreen2.Height = screen.H / 2;
                vscreen2.Width = screen.W;
                vscreen2.Left = 0;
                vscreen2.Top = screen.H / 2.0;

                double l, t;
                if(g_compatibility.free_level_res)
                {
                    GetvScreenAverage2(vscreen1);
                    l = vscreen1.X;
                }
                else
                {
                    GetvScreenAverage2Canonical(&l, &t, true);
                }

                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);

                    vscreena.TempDelay = 200;
                    vscreena.TempY = 0;
                    vscreena.tempX = -l + FieldW * 0.5 - Player[A].Location.X - Player[A].Location.Width * 0.5;
                }

                screen.DType = 4;
                if(screen.DType != 4)
                    PlaySound(SFX_Camera);
                vscreen2.Visible = true;
            }
            else
            {
                if(vscreen2.Visible)
                {
                    if(screen.DType != 5)
                        PlaySound(SFX_Camera);
                    vscreen2.Visible = false;
                    vscreen1.Height = screen.H;
                    vscreen1.Width = screen.W;
                    vscreen1.Left = 0;
                    vscreen1.Top = 0;
                    vscreen1.tempX = 0;
                    vscreen1.TempY = 0;
                    vscreen2.tempX = 0;
                    vscreen2.TempY = 0;
                }
                screen.DType = 5;
            }
            for(A = 1; A <= 2; A++)
            {
                vScreen_t& vscreena = screen.vScreen(A);

                if(vscreena.TempY > (vscreena.Height * 0.25))
                    vscreena.TempY = (vscreena.Height * 0.25);
                if(vscreena.TempY < -(vscreena.Height * 0.25))
                    vscreena.TempY = -(vscreena.Height * 0.25);
                if(vscreena.tempX > (vscreena.Width * 0.25))
                    vscreena.tempX = (vscreena.Width * 0.25);
                if(vscreena.tempX < -(vscreena.Width * 0.25))
                    vscreena.tempX = -(vscreena.Width * 0.25);

                if(vscreena.TempY > (FieldH * 0.25))
                    vscreena.TempY = (FieldH * 0.25);
                if(vscreena.TempY < -(FieldH * 0.25))
                    vscreena.TempY = -(FieldH * 0.25);
                if(vscreena.tempX > (FieldW * 0.25))
                    vscreena.tempX = (FieldW * 0.25);
                if(vscreena.tempX < -(FieldW * 0.25))
                    vscreena.tempX = -(FieldW * 0.25);
            }
        }
        else
        {
            vscreen1.Height = screen.H / 2;
            vscreen1.Width = screen.W;
            vscreen1.Left = 0;
            vscreen1.Top = 0;
            vscreen2.Height = screen.H / 2;
            vscreen2.Width = screen.W;
            vscreen2.Left = 0;
            vscreen2.Top = screen.H / 2.0;
            vscreen1.tempX = 0;
            vscreen1.TempY = 0;
            vscreen2.tempX = 0;
            vscreen2.TempY = 0;
            GetvScreenAverage2(vscreen1);
            if(screen.DType != 6)
                PlaySound(SFX_Camera);
            screen.DType = 6;
            vscreen2.Visible = true;
        }
    }
    else
    {
        if(vscreen2.Visible)
        {
            vscreen2.Visible = false;
            // vscreen1.Visible = false; // Useless, because code below sets it as TRUE back
            vscreen1.Height = screen.H;
            vscreen1.Width = screen.W;
            vscreen1.Left = 0;
            vscreen1.Top = 0;
            vscreen1.Visible = true;
            vscreen1.tempX = 0;
            vscreen1.TempY = 0;
            vscreen2.tempX = 0;
            vscreen2.TempY = 0;
        }
    }
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 128;
    }
}

// NEW: limit vScreens to playable section area and center them on the real screen
void CenterScreens(Screen_t& screen)
{
    vScreen_t& vscreen1 = screen.vScreen(1);
    vScreen_t& vscreen2 = screen.vScreen(2);

    vscreen1.ScreenLeft = vscreen1.Left;
    vscreen2.ScreenLeft = vscreen2.Left;
    vscreen1.ScreenTop = vscreen1.Top;
    vscreen2.ScreenTop = vscreen2.Top;

    if(GameOutro || LevelEditor || WorldEditor)
        return;

    // restrict the vScreen to the level if the level is smaller than the screen
    double MaxWidth1, MaxWidth2, MaxHeight1, MaxHeight2;

    // set based on level bounds
    if(GameMenu || !LevelSelect)
    {
        MaxWidth1 = level[Player[1].Section].Width - level[Player[1].Section].X;
        MaxWidth2 = level[Player[2].Section].Width - level[Player[2].Section].X;
        MaxHeight1 = level[Player[1].Section].Height - level[Player[1].Section].Y;
        MaxHeight2 = level[Player[2].Section].Height - level[Player[2].Section].Y;

        // set to canonical width if NoTurnBack is enabled
        if(NoTurnBack[Player[1].Section])
            MaxWidth1 = 800;
        if(NoTurnBack[Player[2].Section])
            MaxWidth2 = 800;
    }
    else if(!g_compatibility.free_world_res || !g_config.world_map_expand_view)
    {
        MaxWidth1 = MaxWidth2 = 800;
        MaxHeight1 = MaxHeight2 = 600;
    }
    else
    {
        return;
    }

    if(!LevelSelect && !g_compatibility.free_level_res)
    {
        double CanonicalW = 800;
        double CanonicalH = 600;

        if(vscreen2.Visible)
        {
            if(DScreenType == 3 || DScreenType == 4 || DScreenType == 6)
                CanonicalH /= 2;
            else
                CanonicalW /= 2;
        }

        if(MaxWidth1 > CanonicalW)
            MaxWidth1 = CanonicalW;
        if(MaxWidth2 > CanonicalW)
            MaxWidth2 = CanonicalW;

        if(MaxHeight1 > CanonicalH)
            MaxHeight1 = CanonicalH;
        if(MaxHeight2 > CanonicalH)
            MaxHeight2 = CanonicalH;
    }

    if(MaxWidth1 < vscreen1.Width)
    {
        if(vscreen2.Visible && !(DScreenType == 3 || DScreenType == 4 || DScreenType == 6))
        {
            if(vscreen1.ScreenLeft == 0)
            {
                vscreen1.ScreenLeft += (vscreen1.Width - MaxWidth1);
                vscreen2.Left -= (vscreen1.Width - MaxWidth1);
            }
        }
        else
            vscreen1.ScreenLeft += (vscreen1.Width - MaxWidth1) / 2;

        vscreen1.Width = MaxWidth1;
    }

    if(MaxWidth2 < vscreen2.Width)
    {
        if(vscreen2.Visible && !(DScreenType == 3 || DScreenType == 4 || DScreenType == 6))
        {
            if(vscreen2.ScreenLeft == 0)
            {
                vscreen2.ScreenLeft += (vscreen2.Width - MaxWidth2);
                vscreen1.Left -= (vscreen2.Width - MaxWidth2);
            }
        }
        else
            vscreen2.ScreenLeft += (vscreen2.Width - MaxWidth2) / 2;

        vscreen2.Width = MaxWidth2;
    }

    if(MaxHeight1 < vscreen1.Height)
    {
        if(vscreen2.Visible && (DScreenType == 3 || DScreenType == 4 || DScreenType == 6))
        {
            if(vscreen1.ScreenTop == 0)
            {
                vscreen1.ScreenTop += (vscreen1.Height - MaxHeight1);
                vscreen2.Top -= (vscreen1.Height - MaxHeight1);
            }
        }
        else
            vscreen1.ScreenTop += (vscreen1.Height - MaxHeight1) / 2;

        vscreen1.Height = MaxHeight1;
    }

    if(MaxHeight2 < vscreen2.Height)
    {
        if(vscreen2.Visible && (DScreenType == 3 || DScreenType == 4 || DScreenType == 6))
        {
            if(vscreen2.ScreenTop == 0)
            {
                vscreen2.ScreenTop += (vscreen2.Height - MaxHeight2);
                vscreen1.Top -= (vscreen2.Height - MaxHeight2);
            }
        }
        else
            vscreen2.ScreenTop += (vscreen2.Height - MaxHeight2) / 2;

        vscreen2.Height = MaxHeight2;
    }
}

// NEW: moves qScreen towards vScreen, now including the screen size
bool Update_qScreen(int Z, int camRate, int resizeRate)
{
    if(Z == 2 && !g_compatibility.modern_section_change)
        return false;

    bool continue_qScreen = true;

    // take the slower option of 2px per second camera (vanilla)
    //   or 2px per second resize, then scale the speed of the faster one to match
    double camRateX = camRate;
    double camRateY = camRate;

    double resizeRateX = resizeRate;
    double resizeRateY = resizeRate;

    double camFramesX_l = std::abs(vScreen[Z].X - qScreenLoc[Z].X) / camRateX;
    double camFramesY_t = std::abs(vScreen[Z].Y - qScreenLoc[Z].Y) / camRateY;

    double camFramesX_r = std::abs(vScreen[Z].X - vScreen[Z].Width - qScreenLoc[Z].X + qScreenLoc[Z].Width) / camRateX;
    double camFramesY_b = std::abs(vScreen[Z].Y - vScreen[Z].Height - qScreenLoc[Z].Y + qScreenLoc[Z].Height) / camRateY;

    double camFramesX = SDL_min(camFramesX_l, camFramesX_r);
    double camFramesY = SDL_min(camFramesY_t, camFramesY_b);

    double resizeFramesX = std::abs(vScreen[Z].ScreenLeft - qScreenLoc[Z].ScreenLeft) / resizeRateX;
    double resizeFramesY = std::abs(vScreen[Z].ScreenTop - qScreenLoc[Z].ScreenTop) / resizeRateY;

    if(!g_compatibility.modern_section_change)
    {
        resizeFramesX = 0;
        resizeFramesY = 0;
    }

    double qFramesX = (camFramesX > resizeFramesX ? camFramesX : resizeFramesX);
    double qFramesY = (camFramesY > resizeFramesY ? camFramesY : resizeFramesY);

    // don't continue after this frame if it would arrive next frame
    // (this is the intent of the <5 condition in the vanilla game)
    if(qFramesX < 2.5 && qFramesY < 2.5)
        continue_qScreen = false;

    // but, the original condition occurred *after* adding/subtracting 2, so actually
    // the original game would not continue if it would arrive the frame after next, too
    if(!g_compatibility.modern_section_change && qFramesX < 3.5 && qFramesY < 3.5)
        continue_qScreen = false;

    if(qFramesX < 1)
        qFramesX = 1;
    if(qFramesY < 1)
        qFramesY = 1;

    camRateX = std::abs(vScreen[Z].X - qScreenLoc[Z].X)/qFramesX;
    camRateY = std::abs(vScreen[Z].Y - qScreenLoc[Z].Y)/qFramesY;

    resizeRateX = std::abs(vScreen[Z].ScreenLeft - qScreenLoc[Z].ScreenLeft)/qFramesX;
    resizeRateY = std::abs(vScreen[Z].ScreenTop - qScreenLoc[Z].ScreenTop)/qFramesY;

    if(vScreen[Z].X < qScreenLoc[Z].X - camRateX)
        qScreenLoc[Z].X -= camRateX;
    else if(vScreen[Z].X > qScreenLoc[Z].X + camRateX)
        qScreenLoc[Z].X += camRateX;
    else
        qScreenLoc[Z].X = vScreen[Z].X;

    if(vScreen[Z].Y < qScreenLoc[Z].Y - camRateY)
        qScreenLoc[Z].Y -= camRateY;
    else if(vScreen[Z].Y > qScreenLoc[Z].Y + camRateY)
        qScreenLoc[Z].Y += camRateY;
    else
        qScreenLoc[Z].Y = vScreen[Z].Y;

    if(vScreen[Z].ScreenLeft < qScreenLoc[Z].ScreenLeft - resizeRateX)
        qScreenLoc[Z].ScreenLeft -= resizeRateX;
    else if(vScreen[Z].ScreenLeft > qScreenLoc[Z].ScreenLeft + resizeRateX)
        qScreenLoc[Z].ScreenLeft += resizeRateX;
    else
        qScreenLoc[Z].ScreenLeft = vScreen[Z].ScreenLeft;

    if(vScreen[Z].ScreenTop < qScreenLoc[Z].ScreenTop - resizeRateY)
        qScreenLoc[Z].ScreenTop -= resizeRateY;
    else if(vScreen[Z].ScreenTop > qScreenLoc[Z].ScreenTop + resizeRateY)
        qScreenLoc[Z].ScreenTop += resizeRateY;
    else
        qScreenLoc[Z].ScreenTop = vScreen[Z].ScreenTop;

    if(vScreen[Z].Width < qScreenLoc[Z].Width - resizeRateX * 2)
        qScreenLoc[Z].Width -= resizeRateX * 2;
    else if(vScreen[Z].Width > qScreenLoc[Z].Width + resizeRateX * 2)
        qScreenLoc[Z].Width += resizeRateX * 2;
    else
        qScreenLoc[Z].Width = vScreen[Z].Width;

    if(vScreen[Z].Height < qScreenLoc[Z].Height - resizeRateY * 2)
        qScreenLoc[Z].Height -= resizeRateY * 2;
    else if(vScreen[Z].Height > qScreenLoc[Z].Height + resizeRateY * 2)
        qScreenLoc[Z].Height += resizeRateY * 2;
    else
        qScreenLoc[Z].Height = vScreen[Z].Height;

    vScreen[Z].X = qScreenLoc[Z].X;
    vScreen[Z].Y = qScreenLoc[Z].Y;

    // update vScreen width / height
    if(g_compatibility.modern_section_change)
    {
        vScreen[Z].Width -= 2*(std::floor(qScreenLoc[Z].ScreenLeft) - vScreen[Z].ScreenLeft);
        vScreen[Z].Height -= 2*(std::floor(qScreenLoc[Z].ScreenTop) - vScreen[Z].ScreenTop);
        vScreen[Z].ScreenLeft = std::floor(qScreenLoc[Z].ScreenLeft);
        vScreen[Z].ScreenTop = std::floor(qScreenLoc[Z].ScreenTop);
    }

    return continue_qScreen;
}

void SetRes()
{
//    GetCurrentRes(); // Dummy, useless
    resChanged = true;
    ChangeRes(800, 600, 16, 60); // 800x600 pixels, 16 bit Color, 60Hz
    XWindow::showCursor(0);
}
