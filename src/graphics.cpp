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

#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "game_main.h"
#include "sound.h"
#include "change_res.h"
#include "main/game_info.h"
#include "core/render.h"
#include "core/events.h"

#include "graphics/gfx_frame.h"

#include "pseudo_vb.h"
#include "gfx.h"
#include "config.h"

#include <Utils/maths.h>


static int16_t s_vScreenOffsetX[2] = {0, 0};
static int8_t s_lastButtonsHeld[2] = {0, 0};
int16_t g_vScreenOffsetY[2] = {0, 0};
int16_t g_vScreenOffsetY_hold[2] = {0, 0};

void ResetCameraPanning()
{
    s_vScreenOffsetX[0] = 0;
    s_vScreenOffsetX[1] = 0;
    s_lastButtonsHeld[0] = 0;
    s_lastButtonsHeld[1] = 0;
    g_vScreenOffsetY[0] = 0;
    g_vScreenOffsetY[1] = 0;
    g_vScreenOffsetY_hold[0] = 0;
    g_vScreenOffsetY_hold[1] = 0;
}

static void s_ProcessSmallScreenFeatures(int A)
{
    if(g_config.small_screen_camera_features && ScreenW < 800 && !NoTurnBack[Player[A].Section])
    {
        int16_t max_offsetX = 360;
        if(max_offsetX > vScreen[A].Width - Player[A].Location.Width * 4)
            max_offsetX = vScreen[A].Width - Player[A].Location.Width * 4;

        int16_t lookX_target = max_offsetX * Player[A].Location.SpeedX * 1.5 / Physics.PlayerRunSpeed;
        if(lookX_target > max_offsetX)
            lookX_target = max_offsetX;
        if(lookX_target < -max_offsetX)
            lookX_target = -max_offsetX;
        lookX_target &= ~1;

        int16_t rateX = 1;
        // switching directions
        if((s_vScreenOffsetX[A - 1] < 0 && lookX_target > 0)
            || (s_vScreenOffsetX[A - 1] > 0 && lookX_target < 0))
        {
            rateX = 3;
        }
        // accelerating
        else if((s_vScreenOffsetX[A - 1] > 0) == (lookX_target > s_vScreenOffsetX[A - 1]))
        {
            rateX = 2;
        }

        if(GamePaused == PauseCode::None && !qScreen && !ForcedControls)
        {
            if(s_vScreenOffsetX[A - 1] < lookX_target)
                s_vScreenOffsetX[A - 1] += rateX;
            else if(s_vScreenOffsetX[A - 1] > lookX_target)
                s_vScreenOffsetX[A - 1] -= rateX;
        }

        vScreen[A].X -= s_vScreenOffsetX[A - 1]/2;
    }

    if(g_config.small_screen_camera_features && ScreenH < 600)
    {
        int16_t max_offsetY = 200;

        int16_t lookY_target = max_offsetY;

        bool on_ground = Player[A].Pinched.Bottom1 || Player[A].Slope || Player[A].StandingOnNPC || Player[A].Wet || Player[A].Quicksand;
        // bool duck_jump = !on_ground && Player[A].Duck;
        bool prevent_unlock = g_vScreenOffsetY_hold[A - 1] != 0 && (Player[A].Vine || !on_ground || Player[A].GrabTime);

        if(Player[A].Controls.Up == Player[A].Controls.Down || prevent_unlock)
            lookY_target = g_vScreenOffsetY_hold[A - 1];
        else if(Player[A].Controls.Down)
            lookY_target *= -1;

        int16_t rateY = 4;
        if((g_vScreenOffsetY[A - 1] < 0 && lookY_target > 0)
            || (g_vScreenOffsetY[A - 1] > 0 && lookY_target < 0))
        {
            if(g_vScreenOffsetY[A - 1] < 50 && g_vScreenOffsetY[A - 1] > -50)
                g_vScreenOffsetY[A - 1] *= -1;
        }

        if(GamePaused == PauseCode::None && !qScreen && !ForcedControls)
        {
            if(g_vScreenOffsetY[A - 1] < lookY_target)
            {
                g_vScreenOffsetY[A - 1] += rateY;

                if(g_vScreenOffsetY[A - 1] > lookY_target)
                    g_vScreenOffsetY[A - 1] = lookY_target;
            }
            else if(g_vScreenOffsetY[A - 1] > lookY_target)
            {
                g_vScreenOffsetY[A - 1] -= rateY;

                if(g_vScreenOffsetY[A - 1] < lookY_target)
                    g_vScreenOffsetY[A - 1] = lookY_target;
            }

            if(g_vScreenOffsetY_hold[A - 1] == 0 && g_vScreenOffsetY[A - 1] < -max_offsetY + 40 && (s_lastButtonsHeld[A - 1] & 1) == 0 && Player[A].Controls.Down)
            {
                g_vScreenOffsetY_hold[A - 1] = -max_offsetY;
                PlaySound(SFX_Camera);
            }
            else if(g_vScreenOffsetY_hold[A - 1] == 0 && g_vScreenOffsetY[A - 1] > max_offsetY - 40 && (s_lastButtonsHeld[A - 1] & 2) == 0 && Player[A].Controls.Up)
            {
                g_vScreenOffsetY_hold[A - 1] = max_offsetY;
                PlaySound(SFX_Camera);
            }
            else if(g_vScreenOffsetY_hold[A - 1] != 0 && g_vScreenOffsetY[A - 1] > -60 && g_vScreenOffsetY[A - 1] < 60)
            {
                g_vScreenOffsetY_hold[A - 1] = 0;
                PlaySound(SFX_Camera);
            }

            s_lastButtonsHeld[A - 1] = (int8_t)Player[A].Controls.Down | (int8_t)Player[A].Controls.Up << 1;
        }

        int16_t lookY = g_vScreenOffsetY[A - 1];

        if(lookY > -50 && lookY < 50)
            lookY = 0;
        else
        {
            if(lookY > 0)
                lookY -= 50;
            if(lookY < 0)
                lookY += 50;
            lookY /= 2;
        }

        vScreen[A].Y += lookY + 32;
    }
}

//  Get the screen position
void GetvScreen(const int A)
{
    auto &p = Player[A];
    auto &pLoc = p.Location;

    // Netplay code, disabled
//    if(ScreenType == 8)
//    {
//        if(Player[nPlay.MySlot + 1].Mount == 2)
//            Player[nPlay.MySlot + 1].Location.Height = 0;
//        vScreen[1].X = -Player[nPlay.MySlot + 1].Location.X + (vScreen[1].Width * 0.5) - Player[nPlay.MySlot + 1].Location.Width / 2.0;
//        vScreen[1].Y = -Player[nPlay.MySlot + 1].Location.Y + (vScreen[1].Height * 0.5) - vScreenYOffset - Player[nPlay.MySlot + 1].Location.Height;
//        vScreen[1].X += -vScreen[1].tempX;
//        vScreen[1].Y += -vScreen[1].TempY;
//        if(-vScreen[1].X < level[Player[nPlay.MySlot + 1].Section].X)
//            vScreen[1].X = -level[Player[nPlay.MySlot + 1].Section].X;
//        if(-vScreen[1].X + vScreen[1].Width > level[Player[nPlay.MySlot + 1].Section].Width)
//            vScreen[1].X = -(level[Player[nPlay.MySlot + 1].Section].Width - vScreen[1].Width);
//        if(-vScreen[1].Y < level[Player[nPlay.MySlot + 1].Section].Y)
//            vScreen[1].Y = -level[Player[nPlay.MySlot + 1].Section].Y;
//        if(-vScreen[1].Y + vScreen[1].Height > level[Player[nPlay.MySlot + 1].Section].Height)
//            vScreen[1].Y = -(level[Player[nPlay.MySlot + 1].Section].Height - vScreen[1].Height);
//        if(vScreen[1].TempDelay > 0)
//            vScreen[1].TempDelay -= 1;
//        else
//        {
//            if(vScreen[1].tempX > 0)
//                vScreen[1].tempX -= 1;
//            if(vScreen[1].tempX < 0)
//                vScreen[1].tempX += 1;
//            if(vScreen[1].TempY > 0)
//                vScreen[1].TempY -= 1;
//            if(vScreen[1].TempY < 0)
//                vScreen[1].TempY += 1;
//        }
//        if(Player[nPlay.MySlot + 1].Mount == 2)
//            Player[nPlay.MySlot + 1].Location.Height = 128;
//    }
//    else
    {
        if(p.Mount == 2)
            pLoc.Height = 0;
        // this guard is new because players can now respawn in 1P mode through DropAdd
        if(Player[A].Effect != 6)
        {
            vScreen[A].X = -pLoc.X + (vScreen[A].Width * 0.5) - pLoc.Width / 2.0;
            vScreen[A].Y = -pLoc.Y + (vScreen[A].Height * 0.5) - vScreenYOffset - pLoc.Height;

            s_ProcessSmallScreenFeatures(A);

            vScreen[A].X += -vScreen[A].tempX;
            vScreen[A].Y += -vScreen[A].TempY;

            // center the level if too small, otherwise shift so that it is onscreen
            if(vScreen[A].Width + level[p.Section].X > level[p.Section].Width)
                vScreen[A].X = -level[p.Section].X/2 + -(level[p.Section].Width - vScreen[A].Width)/2;
            else if(-vScreen[A].X < level[p.Section].X)
                vScreen[A].X = -level[p.Section].X;
            else if(-vScreen[A].X + vScreen[A].Width > level[p.Section].Width)
                vScreen[A].X = -(level[p.Section].Width - vScreen[A].Width);

            // center the level if too small, otherwise shift so that it is onscreen
            if(vScreen[A].Height + level[p.Section].Y > level[p.Section].Height)
                vScreen[A].Y = -level[p.Section].Y/2 + -(level[p.Section].Height - vScreen[A].Height)/2;
            else if(-vScreen[A].Y < level[p.Section].Y)
                vScreen[A].Y = -level[p.Section].Y;
            else if(-vScreen[A].Y + vScreen[A].Height > level[p.Section].Height)
                vScreen[A].Y = -(level[p.Section].Height - vScreen[A].Height);
        }

        // there is some 3DS-specific code to ensure the screen boundaries work with the 3D effect, not ported yet.

        if(vScreen[A].TempDelay > 0)
            vScreen[A].TempDelay -= 1;
        else
        {
            if(vScreen[A].tempX > 0)
                vScreen[A].tempX -= 1;
            if(vScreen[A].tempX < 0)
                vScreen[A].tempX += 1;
            if(vScreen[A].TempY > 0)
                vScreen[A].TempY -= 1;
            if(vScreen[A].TempY < 0)
                vScreen[A].TempY += 1;
        }
        if(p.Mount == 2)
            pLoc.Height = 128;
    }
}

// NEW: get the vScreen position if it were 800x600, and write the top-left coordinate to (left, top)
void GetvScreenCanonical(int A, double* left, double* top, bool ignore_qScreen)
{
    auto &p = Player[A];
    auto &pLoc = p.Location;

    double pHeight = (p.Mount != 2) ? pLoc.Height : 0;

    *left = -pLoc.X + (800 * 0.5) - pLoc.Width / 2.0;
    *top = -pLoc.Y + (600 * 0.5) - vScreenYOffset - pHeight;
    *left -= vScreen[A].tempX;
    *top -= vScreen[A].TempY;
    if(-(*left) < level[p.Section].X)
        *left = -level[p.Section].X;
    else if(-(*left) + 800 > level[p.Section].Width)
        *left = -(level[p.Section].Width - 800);
    if(-(*top) < level[p.Section].Y)
        *top = -level[p.Section].Y;
    else if(-(*top) + 600 > level[p.Section].Height)
        *top = -(level[p.Section].Height - 600);

    if(qScreen && !ignore_qScreen)
    {
        double tX = vScreen[A].X;
        double tY = vScreen[A].Y;
        GetvScreen(A);
        *left += tX - vScreen[A].X;
        *top += tY - vScreen[A].Y;
        vScreen[A].X = tX;
        vScreen[A].Y = tY;
    }
}

// Get the average screen position for all players
void GetvScreenAverage()
{
    int A = 0;
    int B = 0;
    double OldX = 0;
    double OldY = 0;

    OldX = vScreen[1].X;
    OldY = vScreen[1].Y;

    vScreen[1].X = 0;
    vScreen[1].Y = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead && Player[A].Effect != 6)
        {
            vScreen[1].X += -Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vScreen[1].Y += -Player[A].Location.Y;
            else
                vScreen[1].Y += -Player[A].Location.Y - Player[A].Location.Height;
            B += 1;
        }
    }

    A = 1;
    if(B == 0)
    {
        if(GameMenu)
        {
            vScreen[1].X = -level[0].X;
            B = 1;
        }
        else
        {
            vScreen[1].X = OldX;
            vScreen[1].Y = OldY;
            return;
        }
    }
    vScreen[1].X = (vScreen[1].X / B) + (vScreen[1].Width * 0.5);
    vScreen[1].Y = (vScreen[1].Y / B) + (vScreen[1].Height * 0.5) - vScreenYOffset;

    // case one: level is too small, center it.
    if(vScreen[A].Width + level[Player[1].Section].X > level[Player[1].Section].Width)
        vScreen[A].X = -level[Player[1].Section].X/2 + -(level[Player[1].Section].Width - vScreen[A].Width)/2;
    // case two: we are too close to the left
    else if(-vScreen[A].X < level[Player[1].Section].X)
        vScreen[A].X = -level[Player[1].Section].X;
    // case three: we are too close to the right
    else if(-vScreen[A].X + vScreen[A].Width > level[Player[1].Section].Width)
        vScreen[A].X = -(level[Player[1].Section].Width - vScreen[A].Width);

    // case one: level is too small, center it.
    if(vScreen[A].Height + level[Player[1].Section].Y > level[Player[1].Section].Height)
        vScreen[A].Y = -level[Player[1].Section].Y/2 + -(level[Player[1].Section].Height - vScreen[A].Height)/2;
    // case two: we are too close to the top
    else if(-vScreen[A].Y < level[Player[1].Section].Y)
        vScreen[A].Y = -level[Player[1].Section].Y;
    // case three: we are too close to the bottom
    else if(-vScreen[A].Y + vScreen[A].Height > level[Player[1].Section].Height)
        vScreen[A].Y = -(level[Player[1].Section].Height - vScreen[A].Height);

    // keep vScreen boundary even (on 1x platforms)
#ifdef PGE_MIN_PORT
    vScreen[A].X += 1;
    if(vScreen[A].X > 0)
        vScreen[A].X -= std::fmod(vScreen[A].X, 2.);
    else
        vScreen[A].X += std::fmod(vScreen[A].X, 2.);
#endif

    if(GameMenu)
    {
        if(vScreen[1].X > OldX)
        {
            if(fEqual(vScreen[1].X, -level[0].X))
                vScreen[1].X = OldX + 20;
            else
                vScreen[1].X = OldX;
        }
        else if(vScreen[1].X < OldX - 10)
            vScreen[1].X = OldX - 10;

        // on menu, bottom of screen always tracks bottom of level
        vScreen[A].Y = -(level[Player[1].Section].Height - vScreen[A].Height);
    }
}

// NEW: get the average screen position for all players if it were 800x600, and write the top-left coordinate to (left, top)
void GetvScreenAverageCanonical(double* left, double* top, bool ignore_qScreen)
{
    int A = 0;
    int B = 0;

    *left = 0;
    *top = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead && Player[A].Effect != 6)
        {
            *left -= Player[A].Location.X + Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                *top -= Player[A].Location.Y;
            else
                *top -= Player[A].Location.Y + Player[A].Location.Height;
            B += 1;
        }
    }

    if(B == 0)
    {
        if(GameMenu)
        {
            *left = -level[0].X;
            B = 1;
        }
        else
            return;
    }
    *left = (*left / B) + (800 / 2);
    *top = (*top / B) + (600 / 2) - vScreenYOffset;

    if(-(*left) < level[Player[1].Section].X)
        *left = -level[Player[1].Section].X;
    else if(-(*left) + 800 > level[Player[1].Section].Width)
        *left = -(level[Player[1].Section].Width - 800);
    if(-(*top) < level[Player[1].Section].Y)
        *top = -level[Player[1].Section].Y;
    else if(-(*top) + 600 > level[Player[1].Section].Height)
        *top = -(level[Player[1].Section].Height - 600);

    if(qScreen && !ignore_qScreen)
    {
        double tX = vScreen[1].X;
        double tY = vScreen[1].Y;
        GetvScreenAverage();
        *left += tX - vScreen[1].X;
        *top += tY - vScreen[1].Y;
        vScreen[1].X = tX;
        vScreen[1].Y = tY;
    }
}

// Get the average screen position for all players with no level edge detection
void GetvScreenAverage2()
{
    // int A = 0;
    int B = 0;
    vScreen[1].X = 0;
    vScreen[1].Y = 0;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead)
        {
            vScreen[1].X += -Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vScreen[1].Y += -Player[A].Location.Y;
            else
                vScreen[1].Y += -Player[A].Location.Y - Player[A].Location.Height;
            B += 1;
        }
    }

    //A = 1; // Stored value gets never read

    if(B == 0)
        return;

    vScreen[1].X = (vScreen[1].X / B) + (ScreenW * 0.5);
    vScreen[1].Y = (vScreen[1].Y / B) + (ScreenH * 0.5) - vScreenYOffset;
}

// NEW: Get the average screen position for all players with no level edge detection if it were 800x600, and write the top-left coordinate to (left, top)
void GetvScreenAverage2Canonical(double* left, double* top, bool ignore_qScreen)
{
    // int A = 0;
    int B = 0;
    double l = 0;
    double t = 0;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead)
        {
            l += -Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                t += -Player[A].Location.Y;
            else
                t += -Player[A].Location.Y - Player[A].Location.Height;
            B += 1;
        }
    }

    //A = 1; // Stored value gets never read

    if(B == 0)
    {
        *left = l;
        *top = t;
        return;
    }

    *left = (l / B) + (800 * 0.5);
    *top = (t / B) + (600 * 0.5) - vScreenYOffset;

    if(qScreen && !ignore_qScreen)
    {
        double tX = vScreen[1].X;
        double tY = vScreen[1].Y;
        GetvScreenAverage2();
        *left += tX - vScreen[1].X;
        *top += tY - vScreen[1].Y;
        vScreen[1].X = tX;
        vScreen[1].Y = tY;
    }
}

void SetupGraphics()
{
    //DUMMY AND USELESS

    // Creates the back buffer for the main game
    // myBackBuffer = CreateCompatibleDC(GetDC(0))
    // myBufferBMP = CreateCompatibleBitmap(GetDC(0), screenw, screenh)
//    myBackBuffer = CreateCompatibleDC(frmMain::hdc);
//    myBufferBMP = CreateCompatibleBitmap(frmMain::hdc, ScreenW, ScreenH);
//    SelectObject myBackBuffer, myBufferBMP;
//    GFX.Split(2).Width = ScreenW;
//    GFX.Split(2).Height = ScreenH;
    // GFX.BackgroundColor(1).Width = Screen.Width
    // GFX.BackgroundColor(1).Height = Screen.Height
    // GFX.BackgroundColor(2).Width = Screen.Width
    // GFX.BackgroundColor(2).Height = Screen.Height
}

void SetupEditorGraphics()
{
    //DUMMY AND USELESS

//    GFX.Split(1).Width = frmLevelWindow.vScreen(1).Width
//    GFX.Split(1).Height = frmLevelWindow.vScreen(1).Height
//    GFX.Split(2).Width = frmLevelWindow.vScreen(1).Width
//    GFX.Split(2).Height = frmLevelWindow.vScreen(1).Height
//    vScreen(1).Height = frmLevelWindow.vScreen(1).ScaleHeight
//    vScreen(1).Width = frmLevelWindow.vScreen(1).ScaleWidth
//    vScreen(1).Left = 0
//    vScreen(1).Top = 0
//    vScreen(2).Visible = False
//    'Creates the back buffer for the level editor
//    'myBackBuffer = CreateCompatibleDC(GetDC(0))
//    'myBufferBMP = CreateCompatibleBitmap(GetDC(0), screenw, screenh)
//    'SelectObject myBackBuffer, myBufferBMP
//    GFX.BackgroundColor(1).Width = frmLevelWindow.vScreen(1).Width
//    GFX.BackgroundColor(1).Height = frmLevelWindow.vScreen(1).Height
//    GFX.BackgroundColor(2).Width = frmLevelWindow.vScreen(1).Width
//    GFX.BackgroundColor(2).Height = frmLevelWindow.vScreen(1).Height
}

void PlayerWarpGFX(int A, Location_t &tempLocation, float &X2, float &Y2)
{
    auto &player = Player[A];
    bool backward = player.WarpBackward;
    auto &warp = Warp[player.Warp];
    auto &warp_enter = backward ? warp.Exit : warp.Entrance;
    auto &warp_exit = backward ? warp.Entrance : warp.Exit;
    auto &warp_dir_enter = backward ? warp.Direction2 : warp.Direction;
    auto &warp_dir_exit = backward ? warp.Direction : warp.Direction2;

    // .Effect = 3      -- Warp Pipe
    // .Effect2 = 0     -- Entering
    // .Effect2 = 1     -- Move to next spot
    // .Effect2 => 100  -- Delay at next spot
    // .Effect2 = 2     -- Exiting
    // .Effect2 = 3     -- Done
    if(player.Effect2 == 0.0)
    {
        if(warp_dir_enter == 3) // Moving down
        {
            if(tempLocation.Height > (warp_enter.Y + warp_enter.Height) - (tempLocation.Y))
                tempLocation.Height = (warp_enter.Y + warp_enter.Height) - (tempLocation.Y);
        }
        else if(warp_dir_enter == 1) // Moving up
        {
            if(warp_enter.Y > tempLocation.Y)
            {
                Y2 = float(warp_enter.Y - tempLocation.Y);
                tempLocation.Y = warp_enter.Y;
                tempLocation.Height += -Y2;
            }
        }
        else if(warp_dir_enter == 4) // Moving right
            tempLocation.Width = (warp_enter.X + warp_enter.Width) - (tempLocation.X);
        else if(warp_dir_enter == 2) // Moving left
        {
            X2 = float(warp_enter.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = warp_enter.X;
        }
    }
    else if(Maths::iRound(player.Effect2) == 2)
    {
        if(warp_dir_exit == 3) // Moving up
        {
            if(tempLocation.Height > (warp_exit.Y + warp_exit.Height) - (tempLocation.Y))
                tempLocation.Height = (warp_exit.Y + warp_exit.Height) - (tempLocation.Y);
        }
        else if(warp_dir_exit == 1) // Moving down
        {
            if(warp_exit.Y > tempLocation.Y)
            {
                Y2 = float(warp_exit.Y - tempLocation.Y);
                tempLocation.Y = warp_exit.Y;
                tempLocation.Height += -double(Y2);
            }
        }
        else if(warp_dir_exit == 4) // Moving left
            tempLocation.Width = (warp_exit.X + warp_exit.Width) - (tempLocation.X);
        else if(warp_dir_exit == 2) // Moving right
        {
            X2 = float(warp_exit.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = warp_exit.X;
        }
    }

    if(Maths::iRound(player.Effect2) == 1 || player.Effect2 >= 100)
        tempLocation.Height = 0;

    if(tempLocation.Height < 0)
    {
        tempLocation.Height = 0;
        tempLocation.Width = 0;
    }

    tempLocation.Width -= double(X2);
}

void NPCWarpGFX(int A, Location_t &tempLocation, float &X2, float &Y2)
{
    auto &player = Player[A];
    bool backward = player.WarpBackward;
    auto &warp = Warp[player.Warp];
    auto &warp_enter = backward ? warp.Exit : warp.Entrance;
    auto &warp_exit = backward ? warp.Entrance : warp.Exit;
    auto &warp_dir_enter = backward ? warp.Direction2 : warp.Direction;
    auto &warp_dir_exit = backward ? warp.Direction : warp.Direction2;

    // player(a).effect = 3      -- Warp Pipe
    // player(a).effect2 = 0     -- Entering
    // player(a).effect2 = 1     -- Move to next spot
    // player(a).effect2 => 100  -- Delay at next spot
    // player(a).effect2 = 2     -- Exiting
    // player(a).effect2 = 3     -- Done
    if(player.Effect2 == 0.0)
    {
        if(warp_dir_enter == 3) // Moving down
        {
            if(tempLocation.Height > (warp_enter.Y + warp_enter.Height) - (tempLocation.Y))
                tempLocation.Height = (warp_enter.Y + warp_enter.Height) - (tempLocation.Y);
        }
        else if(warp_dir_enter == 1) // Moving up
        {
            if(warp_enter.Y > tempLocation.Y)
            {
                Y2 = float(warp_enter.Y - tempLocation.Y);
                tempLocation.Y = warp_enter.Y;
                tempLocation.Height += -double(Y2);
            }
        }
        else if(warp_dir_enter == 4) // Moving right
            tempLocation.Width = (warp_enter.X + warp_enter.Width) - (tempLocation.X);
        else if(warp_dir_enter == 2) // Moving left
        {
            X2 = float(warp_enter.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = warp_enter.X;
        }
    }
    else if(Maths::iRound(player.Effect2) == 2)
    {
        if(warp_dir_exit == 3) // Moving up
        {
            if(tempLocation.Height > (warp_exit.Y + warp_exit.Height) - (tempLocation.Y))
                tempLocation.Height = (warp_exit.Y + warp_exit.Height) - (tempLocation.Y);
        }
        else if(warp_dir_exit == 1) // Moving down
        {
            if(warp_exit.Y > tempLocation.Y)
            {
                Y2 = float(warp_exit.Y - tempLocation.Y);
                tempLocation.Y = warp_exit.Y;
                tempLocation.Height += -double(Y2);
            }
        }
        else if(warp_dir_exit == 4) // Moving left
            tempLocation.Width = (warp_exit.X + warp_exit.Width) - (tempLocation.X);
        else if(warp_dir_exit == 2) // Moving right
        {
            X2 = float(warp_exit.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = warp_exit.X;
        }
    }

    if(Maths::iRound(player.Effect2) == 1 || player.Effect2 >= 100)
        tempLocation.Height = 0;

    if(tempLocation.Height < 0)
    {
        tempLocation.Height = 0;
        tempLocation.Width = 0;
    }
}

// change from fullscreen to windowed mode
void ChangeScreen()
{
//    frmMain.LockSize = True
//    If resChanged = True Then
    if(resChanged)
    {
//        SetOrigRes
        SetOrigRes();
//        DoEvents
        XEvents::doEvents();
//        DeleteDC myBackBuffer
//        DeleteObject myBufferBMP
//        DoEvents
//        myBackBuffer = CreateCompatibleDC(frmMain.hdc)
//        myBufferBMP = CreateCompatibleBitmap(frmMain.hdc, 800, 600)
//        SelectObject myBackBuffer, myBufferBMP
//        frmMain.BorderStyle = 2
//        frmMain.Caption = "Super Mario Bros. X - Version 1.3 - www.SuperMarioBrothers.org"
//        frmMain.Left = 0
//        frmMain.Top = 0
//    Else
    } else {
//        If frmMain.WindowState = 2 Then
//            frmMain.WindowState = 0
//        End If
//        frmMain.Width = 12240
//        frmMain.Height = 9570
//        Do While frmMain.ScaleWidth > 800
//            frmMain.Width += -5
//        Loop
//        Do While frmMain.ScaleHeight > 600
//            frmMain.Height += -5
//        Loop
//        Do While frmMain.ScaleWidth < 800
//            frmMain.Width += 5
//        Loop
//        Do While frmMain.ScaleHeight < 600
//            frmMain.Height += 5
//        Loop
//        SetRes
        SetRes();
//        DoEvents
        XEvents::doEvents();
//        DeleteDC myBackBuffer
//        DeleteObject myBufferBMP
//        DoEvents
//        myBackBuffer = CreateCompatibleDC(frmMain.hdc)
//        myBufferBMP = CreateCompatibleBitmap(frmMain.hdc, 800, 600)
//        SelectObject myBackBuffer, myBufferBMP
//        frmMain.BorderStyle = 0
//        frmMain.Caption = ""
//        frmMain.Left = 0
//        frmMain.Top = 0
//    End If
    }
//    SaveConfig
    SaveConfig();
//    frmMain.LockSize = False
}

void GetvScreenCredits()
{
    int A = 0;
    int B = 0;

    vScreen[1].X = 0;
    vScreen[1].Y = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        if((!Player[A].Dead || g_gameInfo.outroDeadMode) && Player[A].Effect != 6)
        {
            vScreen[1].X += -Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vScreen[1].Y += -Player[A].Location.Y;
            else
                vScreen[1].Y += -Player[A].Location.Y - Player[A].Location.Height;
            B++;
        }
    }

    A = 1;
    if(B == 0)
        return;

    vScreen[1].X = (vScreen[1].X / B) + (ScreenW * 0.5);
    vScreen[1].Y = (vScreen[1].Y / B) + (ScreenH * 0.5) - vScreenYOffset;
    if(-vScreen[A].X < level[Player[1].Section].X)
        vScreen[A].X = -level[Player[1].Section].X;
    if(-vScreen[A].X + ScreenW > level[Player[1].Section].Width)
        vScreen[A].X = -(level[Player[1].Section].Width - ScreenW);
    if(-vScreen[A].Y < level[Player[1].Section].Y + 100)
        vScreen[A].Y = -level[Player[1].Section].Y + 100;
    if(-vScreen[A].Y + ScreenH > level[Player[1].Section].Height - 100)
        vScreen[A].Y = -(level[Player[1].Section].Height - ScreenH) - 100;
}

#if 0
// old, 100x100-based functions
// now defined at gfx_draw_player.cpp

int pfrXo(int plrFrame)
{
#if 1
    return pfrX(plrFrame - 100);
#else
    // Old Redigit's code, does the same as a small one-line formula
    int A;
    A = plrFrame;
    A -= 50;
    while(A > 100)
        A -= 100;
    if(A > 90)
        A = 9;
    else if(A > 90)
        A = 9;
    else if(A > 80)
        A = 8;
    else if(A > 70)
        A = 7;
    else if(A > 60)
        A = 6;
    else if(A > 50)
        A = 5;
    else if(A > 40)
        A = 4;
    else if(A > 30)
        A = 3;
    else if(A > 20)
        A = 2;
    else if(A > 10)
        A = 1;
    else
        A = 0;
    return A * 100;
#endif
}

int pfrYo(int plrFrame)
{
#if 1
    return pfrX(plrFrame - 100);
#else
    // Old Redigit's code, does the same as a small one-line formula
    int A;
    A = plrFrame;
    A -= 50;
    while(A > 100)
        A -= 100;
    A -= 1;
    while(A > 9)
        A -= 10;
    return A * 100;
#endif
}

int pfrX(int plrFrame)
{
    return ((plrFrame + 49) / 10) * 100;
}

int pfrY(int plrFrame)
{
    return ((plrFrame + 49) % 10) * 100;
}
#endif

void ScreenShot()
{
#ifdef USE_SCREENSHOTS_AND_RECS
    XRender::setTargetTexture();
    XRender::makeShot();
    XRender::setTargetScreen();
    PlaySoundMenu(SFX_GotItem);
#endif
    TakeScreen = false;
}

void DrawFrozenNPC(int Z, int A)
{
    auto &n = NPC[A];
    if((vScreenCollision(Z, n.Location) ||
        vScreenCollision(Z, newLoc(n.Location.X - (NPCWidthGFX[n.Type] - n.Location.Width) / 2,
                            n.Location.Y, CDbl(NPCWidthGFX[n.Type]), CDbl(NPCHeight[n.Type])))) && !n.Hidden)
    {
// draw npc
        float c = n.Shadow ? 0.f : 1.f;
        int content = int(n.Special);
        int contentFrame = int(n.Special2);

        // SDL_assert_release(content >= 0 && content <= maxNPCType);

        // Draw frozen NPC body in only condition the content value is valid
        if(content > 0 && content <= maxNPCType)
        {
             XRender::renderTexture(float(vScreen[Z].X + n.Location.X + 2),
                                    float(vScreen[Z].Y + n.Location.Y + 2),
                                    float(n.Location.Width - 4),
                                    float(n.Location.Height - 4),
                                    GFXNPCBMP[content],
                                    2, 2 + contentFrame * NPCHeight[content], c, c, c);
        }

        // draw ice
         XRender::renderTexture(float(vScreen[Z].X + n.Location.X + NPCFrameOffsetX[n.Type]),
                                float(vScreen[Z].Y + n.Location.Y + NPCFrameOffsetY[n.Type]),
                                float(n.Location.Width - 6), float(n.Location.Height - 6),
                                GFXNPCBMP[n.Type], 0, 0, c, c, c);
         XRender::renderTexture(float(vScreen[Z].X + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6),
                                float(vScreen[Z].Y + n.Location.Y + NPCFrameOffsetY[n.Type]),
                                6, float(n.Location.Height - 6),
                                GFXNPCBMP[n.Type], 128 - 6, 0, c, c, c);
         XRender::renderTexture(float(vScreen[Z].X + n.Location.X + NPCFrameOffsetX[n.Type]),
                                float(vScreen[Z].Y + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6),
                                float(n.Location.Width - 6), 6,
                                GFXNPCBMP[n.Type], 0, 128 - 6, c, c, c);
         XRender::renderTexture(float(vScreen[Z].X + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6),
                                float(vScreen[Z].Y + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6),
                                6, 6, GFXNPCBMP[n.Type],
                                128 - 6, 128 - 6, c, c, c);
    }
}

Location_t WorldLevel_t::LocationGFX()
{
    Location_t ret = static_cast<Location_t>(Location);

    if(Type >= 1 && Type <= maxLevelType && GFXLevelBig[Type])
    {
        ret.X -= (GFXLevelWidth[Type] - ret.Width) / 2;
        ret.Y -= (GFXLevelHeight[Type] - ret.Height);
        ret.Width = GFXLevelWidth[Type];
        ret.Height = GFXLevelHeight[Type];
    }

    return ret;
}

void DrawBackdrop()
{
    if(g_config.show_backdrop && GFX.Backdrop.inited)
    {
        bool border_valid = GFX.Backdrop_Border.inited && (!GFX.isCustom(68) || GFX.isCustom(69));

        for(int Z = 1; Z <= 2; Z++)
        {
            const auto& s = vScreen[Z];
            if((Z == 1 && SingleCoop == 2) || (Z == 2 && !s.Visible))
                continue;

            Location_t full = newLoc(0, 0, ScreenW, ScreenH);
            // horizontal
            if(ScreenType == 4 || (ScreenType == 5 && (DScreenType == 1 || DScreenType == 2)))
            {
                full.Width = ScreenW / 2;
                // our screen on right
                if(((ScreenType == 4 || (ScreenType == 5 && DScreenType == 1)) && Z == 2) || (DScreenType == 2 && Z == 1))
                    full.X = ScreenW / 2;
            }
            // vertical
            else if(ScreenType == 1 || (ScreenType == 5 && (DScreenType == 3 || DScreenType == 4 || DScreenType == 6)))
            {
                full.Height = ScreenH / 2;
                // our screen on bottom
                if(((ScreenType == 1 || (ScreenType == 5 && (DScreenType == 3 || DScreenType == 6))) && Z == 2) || (DScreenType == 4 && Z == 1))
                    full.Y = ScreenH / 2;
            }

            RenderFrame(full, newLoc(s.ScreenLeft, s.ScreenTop, s.Width, s.Height),
                GFX.Backdrop, border_valid ? &GFX.Backdrop_Border : nullptr, &g_backdropBorderInfo);
        }
    }
}
