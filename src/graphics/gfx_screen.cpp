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

#include "../globals.h"
#include "../graphics.h"
#include "../player.h"
#include "../sound.h"
#include "../change_res.h"
#include "../load_gfx.h"
#include "../core/window.h"
#include "../compat.h"


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
void SetupScreens()
{
    SetScreenType();

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

void DynamicScreen()
{
    int A = 0;
    GetvScreenAverage();

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

    if(CheckDead() == 0)
    {
        if(Player[1].Section == Player[2].Section)
        {
            if(level[Player[1].Section].Width - level[Player[1].Section].X > ScreenW && (((!vScreen[2].Visible && Player[2].Location.X + vScreenX[1] >= ScreenW * 0.75 - Player[2].Location.Width / 2.0) || (vScreen[2].Visible && Player[2].Location.X + vScreenX[1] >= ScreenW * 0.75 - Player[2].Location.Width / 2.0)) && (Player[1].Location.X < level[Player[1].Section].Width - ScreenW * 0.75 - Player[1].Location.Width / 2.0)))
            {
                vScreen[2].Height = ScreenH;
                vScreen[2].Width = ScreenW / 2;
                vScreen[2].Left = ScreenW / 2.0;
                vScreen[2].Top = 0;
                vScreen[1].Height = ScreenH;
                vScreen[1].Width = ScreenW / 2;
                vScreen[1].Left = 0;
                vScreen[1].Top = 0;
                GetvScreenAverage2();
                if(DScreenType != 1)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].tempX = 0;
                    vScreen[A].TempY = -vScreenY[1] + ScreenH * 0.5 - Player[A].Location.Y - vScreenYOffset - Player[A].Location.Height;
                }
                vScreen[2].Visible = true;
                DScreenType = 1;
            }
            else if(level[Player[1].Section].Width - level[Player[1].Section].X > ScreenW && (((!vScreen[2].Visible && Player[1].Location.X + vScreenX[1] >= ScreenW * 0.75 - Player[1].Location.Width / 2.0) || (vScreen[2].Visible && Player[1].Location.X + vScreenX[2] >= ScreenW * 0.75 - Player[1].Location.Width / 2.0)) && (Player[2].Location.X < level[Player[1].Section].Width - ScreenW * 0.75 - Player[2].Location.Width / 2.0)))
            {
                vScreen[1].Height = ScreenH;
                vScreen[1].Width = ScreenW / 2;
                vScreen[1].Left = ScreenW / 2.0;
                vScreen[1].Top = 0;
                vScreen[2].Height = ScreenH;
                vScreen[2].Width = ScreenW / 2;
                vScreen[2].Left = 0;
                vScreen[2].Top = 0;
                GetvScreenAverage2();
                if(DScreenType != 2)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].tempX = 0;
                    vScreen[A].TempY = -vScreenY[1] + ScreenH * 0.5 - Player[A].Location.Y - vScreenYOffset - Player[A].Location.Height;
                }
                DScreenType = 2;
                vScreen[2].Visible = true;
            }
            else if(level[Player[1].Section].Height - level[Player[1].Section].Y > ScreenH && ((!vScreen[2].Visible && Player[1].Location.Y + vScreenY[1] >= ScreenH * 0.75 - vScreenYOffset - Player[1].Location.Height) || (vScreen[2].Visible && Player[1].Location.Y + vScreenY[2] >= ScreenH * 0.75 - vScreenYOffset - Player[1].Location.Height)) && (Player[2].Location.Y < level[Player[1].Section].Height - ScreenH * 0.75 - vScreenYOffset - Player[2].Location.Height))
            {
                vScreen[1].Height = ScreenH / 2;
                vScreen[1].Width = ScreenW;
                vScreen[1].Left = 0;
                vScreen[1].Top = ScreenH / 2.0;
                vScreen[2].Height = ScreenH / 2;
                vScreen[2].Width = ScreenW;
                vScreen[2].Left = 0;
                vScreen[2].Top = 0;
                GetvScreenAverage2();
                if(DScreenType != 3)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].TempY = 0;
                    vScreen[A].tempX = -vScreenX[1] + ScreenW * 0.5 - Player[A].Location.X - Player[A].Location.Width * 0.5;
                }
                vScreen[2].Visible = true;
                DScreenType = 3;
            }
            else if(level[Player[1].Section].Height - level[Player[1].Section].Y > ScreenH && ((!vScreen[2].Visible && Player[2].Location.Y + vScreenY[1] >= ScreenH * 0.75 - vScreenYOffset - Player[2].Location.Height) || (vScreen[2].Visible && Player[2].Location.Y + vScreenY[1] >= ScreenH * 0.75 - vScreenYOffset - Player[2].Location.Height)) && (Player[1].Location.Y < level[Player[1].Section].Height - ScreenH * 0.75 - vScreenYOffset - Player[1].Location.Height))
            {
                vScreen[1].Height = ScreenH / 2;
                vScreen[1].Width = ScreenW;
                vScreen[1].Left = 0;
                vScreen[1].Top = 0;
                vScreen[2].Height = ScreenH / 2;
                vScreen[2].Width = ScreenW;
                vScreen[2].Left = 0;
                vScreen[2].Top = ScreenH / 2.0;
                GetvScreenAverage2();
                if(DScreenType != 4)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].TempY = 0;
                    vScreen[A].tempX = -vScreenX[1] + ScreenW * 0.5 - Player[A].Location.X - Player[A].Location.Width * 0.5;
                }
                vScreen[2].Visible = true;
                DScreenType = 4;
            }
            else
            {
                if(vScreen[2].Visible)
                {
                    if(DScreenType != 5)
                        PlaySound(SFX_Camera);
                    vScreen[2].Visible = false;
                    vScreen[1].Height = ScreenH;
                    vScreen[1].Width = ScreenW;
                    vScreen[1].Left = 0;
                    vScreen[1].Top = 0;
                    vScreen[1].tempX = 0;
                    vScreen[1].TempY = 0;
                    vScreen[2].tempX = 0;
                    vScreen[2].TempY = 0;
                }
                DScreenType = 5;
            }
            for(A = 1; A <= 2; A++)
            {
                if(vScreen[A].TempY > (vScreen[A].Height * 0.25))
                    vScreen[A].TempY = (vScreen[A].Height * 0.25);
                if(vScreen[A].TempY < -(vScreen[A].Height * 0.25))
                    vScreen[A].TempY = -(vScreen[A].Height * 0.25);
                if(vScreen[A].tempX > (vScreen[A].Width * 0.25))
                    vScreen[A].tempX = (vScreen[A].Width * 0.25);
                if(vScreen[A].tempX < -(vScreen[A].Width * 0.25))
                    vScreen[A].tempX = -(vScreen[A].Width * 0.25);
            }
        }
        else
        {
            vScreen[1].Height = ScreenH / 2;
            vScreen[1].Width = ScreenW;
            vScreen[1].Left = 0;
            vScreen[1].Top = 0;
            vScreen[2].Height = ScreenH / 2;
            vScreen[2].Width = ScreenW;
            vScreen[2].Left = 0;
            vScreen[2].Top = ScreenH / 2.0;
            vScreen[1].tempX = 0;
            vScreen[1].TempY = 0;
            vScreen[2].tempX = 0;
            vScreen[2].TempY = 0;
            GetvScreenAverage2();
            if(DScreenType != 6)
                PlaySound(SFX_Camera);
            DScreenType = 6;
            vScreen[2].Visible = true;
        }
    }
    else
    {
        if(vScreen[2].Visible)
        {
            vScreen[2].Visible = false;
            // vScreen[1].Visible = false; // Useless, because code below sets it as TRUE back
            vScreen[1].Height = ScreenH;
            vScreen[1].Width = ScreenW;
            vScreen[1].Left = 0;
            vScreen[1].Top = 0;
            vScreen[1].Visible = true;
            vScreen[1].tempX = 0;
            vScreen[1].TempY = 0;
            vScreen[2].tempX = 0;
            vScreen[2].TempY = 0;
        }
    }
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 128;
    }
}

// NEW: limit vScreens to playable section area and center them on the real screen
void CenterScreens()
{
    vScreen[1].ScreenLeft = vScreen[1].Left;
    vScreen[2].ScreenLeft = vScreen[2].Left;
    vScreen[1].ScreenTop = vScreen[1].Top;
    vScreen[2].ScreenTop = vScreen[2].Top;

    if(GameMenu || GameOutro || LevelEditor || WorldEditor)
        return;

    // restrict the vScreen to the level if the level is smaller than the screen
    double MaxWidth1, MaxWidth2, MaxHeight1, MaxHeight2;

    MaxWidth1 = MaxWidth2 = ScreenW;
    MaxHeight1 = MaxHeight2 = ScreenH;
    if(LevelSelect && !g_compatibility.free_world_res)
    {
        MaxWidth1 = MaxWidth2 = 800;
        MaxHeight1 = MaxHeight2 = 600;
    }
    else if(!LevelSelect && !g_compatibility.free_level_res)
    {
        MaxWidth1 = MaxWidth2 = 800;
        MaxHeight1 = MaxHeight2 = 600;
    }
    else if(!LevelSelect)
    {
        MaxWidth1 = level[Player[1].Section].Width - level[Player[1].Section].X;
        MaxWidth2 = level[Player[2].Section].Width - level[Player[2].Section].X;
        MaxHeight1 = level[Player[1].Section].Height - level[Player[1].Section].Y;
        MaxHeight2 = level[Player[2].Section].Height - level[Player[2].Section].Y;
        if(NoTurnBack[Player[1].Section])
            MaxWidth1 = 800;
        if(NoTurnBack[Player[2].Section])
            MaxWidth2 = 800;
    }

    if(MaxWidth1 < vScreen[1].Width)
    {
        vScreen[1].ScreenLeft += (vScreen[1].Width - MaxWidth1) / 2;
        vScreen[1].Width = MaxWidth1;
    }

    if(MaxWidth2 < vScreen[2].Width)
    {
        vScreen[2].ScreenLeft += (vScreen[2].Width - MaxWidth2) / 2;
        vScreen[2].Width = MaxWidth2;
    }

    if(MaxHeight1 < vScreen[1].Height)
    {
        vScreen[1].ScreenTop += (vScreen[1].Height - MaxHeight1) / 2;
        vScreen[1].Height = MaxHeight1;
    }

    if(MaxHeight2 < vScreen[2].Height)
    {
        vScreen[2].ScreenTop += (vScreen[2].Height - MaxHeight2) / 2;
        vScreen[2].Height = MaxHeight2;
    }
}

// NEW: moves qScreen towards vScreen, now including the screen size
void Update_qScreen()
{
    // take the slower option of 2px per second camera (vanilla)
    //   or 2px per second resize, then scale the speed of the faster one to match
    double camRateX = 2;
    double camRateY = 2;

    double resizeRateX = 2;
    double resizeRateY = 2;

    double camFramesX = std::abs(vScreenX[1] - qScreenX[1])/camRateX;
    double camFramesY = std::abs(vScreenY[1] - qScreenY[1])/camRateY;
    double resizeFramesX = std::abs(vScreen[1].ScreenLeft - qScreenLoc[1].ScreenLeft)/resizeRateX;
    double resizeFramesY = std::abs(vScreen[1].ScreenTop - qScreenLoc[1].ScreenTop)/resizeRateY;
    double qFramesX = (camFramesX > resizeFramesX ? camFramesX : resizeFramesX);
    double qFramesY = (camFramesY > resizeFramesY ? camFramesY : resizeFramesY);

    // don't continue after this frame if it would arrive next frame
    // (this is equivalent to the <5 condition in the vanilla game)
    if(qFramesX < 2.5 && qFramesY < 2.5)
        qScreen = false;

    if(qFramesX < 1)
        qFramesX = 1;
    if(qFramesY < 1)
        qFramesY = 1;

    camRateX = std::abs(vScreenX[1] - qScreenX[1])/qFramesX;
    camRateY = std::abs(vScreenY[1] - qScreenY[1])/qFramesY;

    resizeRateX = std::abs(vScreen[1].ScreenLeft - qScreenLoc[1].ScreenLeft)/qFramesX;
    resizeRateY = std::abs(vScreen[1].ScreenTop - qScreenLoc[1].ScreenTop)/qFramesY;

    if(vScreenX[1] < qScreenX[1] - camRateX)
        qScreenX[1] = qScreenX[1] - camRateX;
    else if(vScreenX[1] > qScreenX[1] + camRateX)
        qScreenX[1] = qScreenX[1] + camRateX;
    else
        qScreenX[1] = vScreenX[1];

    if(vScreenY[1] < qScreenY[1] - camRateY)
        qScreenY[1] = qScreenY[1] - camRateY;
    else if(vScreenY[1] > qScreenY[1] + camRateY)
        qScreenY[1] = qScreenY[1] + camRateY;
    else
        qScreenY[1] = vScreenY[1];

    if(vScreen[1].ScreenLeft < qScreenLoc[1].ScreenLeft - resizeRateX)
        qScreenLoc[1].ScreenLeft -= resizeRateX;
    else if(vScreen[1].ScreenLeft > qScreenLoc[1].ScreenLeft + resizeRateX)
        qScreenLoc[1].ScreenLeft += resizeRateX;
    else
        qScreenLoc[1].ScreenLeft = vScreen[1].ScreenLeft;

    if(vScreen[1].ScreenTop < qScreenLoc[1].ScreenTop - resizeRateY)
        qScreenLoc[1].ScreenTop -= resizeRateY;
    else if(vScreen[1].ScreenTop > qScreenLoc[1].ScreenTop + resizeRateY)
        qScreenLoc[1].ScreenTop += resizeRateY;
    else
        qScreenLoc[1].ScreenTop = vScreen[1].ScreenTop;

    vScreenX[1] = qScreenX[1];
    vScreenY[1] = qScreenY[1];

    vScreen[1].Width -= 2*(std::floor(qScreenLoc[1].ScreenLeft) - vScreen[1].ScreenLeft);
    vScreen[1].Height -= 2*(std::floor(qScreenLoc[1].ScreenTop) - vScreen[1].ScreenTop);
    vScreen[1].ScreenLeft = std::floor(qScreenLoc[1].ScreenLeft);
    vScreen[1].ScreenTop = std::floor(qScreenLoc[1].ScreenTop);
}

void SetRes()
{
//    GetCurrentRes(); // Dummy, useless
    resChanged = true;
    ChangeRes(800, 600, 16, 60); // 800x600 pixels, 16 bit Color, 60Hz
    XWindow::showCursor(0);
}
