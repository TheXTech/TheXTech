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

#include "../globals.h"
#include "../graphics.h"
#include "../player.h"
#include "../sound.h"
#include "../change_res.h"
#include "../load_gfx.h"


// Sets up the split lines
void SetupScreens()
{
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
    case 7:
        vScreen[1].Left = 0;
        vScreen[1].Width = 800;
        vScreen[1].Top = 0;
        vScreen[1].Height = 600;
        vScreen[2].Visible = false;
        break;
    case 8: // netplay
        vScreen[1].Left = 0;
        vScreen[1].Width = 800;
        vScreen[1].Top = 0;
        vScreen[1].Height = 600;
        vScreen[2].Visible = false;
        break;
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
            vScreen[1].Visible = false;
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

void SetRes()
{
//    GetCurrentRes(); // Dummy, useless
    resChanged = true;
    ChangeRes(800, 600, 16, 60); // 800x600 pixels, 16 bit Color, 60Hz
    showCursor(0);
}
