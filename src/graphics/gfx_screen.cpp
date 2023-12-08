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

#include "../globals.h"
#include "../graphics.h"
#include "../player.h"
#include "../sound.h"
#include "../change_res.h"
#include "../load_gfx.h"
#include "../core/window.h"

#include "../compat.h"
#include "config.h"
#include "core/render.h"

void SetScreenType(Screen_t& screen)
{
    // TODO: move this logic elsewhere once multiple screens are supported
    screen.player_count = SDL_min(numPlayers, maxLocalPlayers);

    // moved this code from game_main.cpp, but it occured elsewhere also
    //   it was always called before setup screens, now it is a part of setup screens.
    //   better to have it in one place so it can be updated
    if(screen.player_count == 1)
        screen.Type = ScreenTypes::SinglePlayer; // Follow 1 player
    else if(screen.player_count == 2)
    {
        if(screen.multiplayer_pref == MultiplayerPrefs::Split)
            screen.Type = ScreenTypes::LeftRight;
        else if(screen.multiplayer_pref == MultiplayerPrefs::Shared)
            screen.Type = ScreenTypes::SharedScreen;
        else
            screen.Type = ScreenTypes::Dynamic; // Dynamic screen
    }
    else
        screen.Type = ScreenTypes::SharedScreen; // Average, no one leaves the screen

    // special cases
    if(g_ClonedPlayerMode)
        screen.Type = ScreenTypes::Average;
    if(SingleCoop > 0)
        screen.Type = ScreenTypes::SingleCoop;
    if(GameMenu)
        screen.Type = ScreenTypes::Average;
    if(GameOutro)
        screen.Type = ScreenTypes::Credits;
    if(LevelEditor)
        screen.Type = ScreenTypes::SinglePlayer;
}

// Sets up the split lines
void SetupScreens(Screen_t& screen, bool reset)
{
    SetScreenType(screen);

    vScreen_t& vscreen1 = screen.vScreen(1);
    vScreen_t& vscreen2 = screen.vScreen(2);

    vscreen1.player = screen.players[0];
    vscreen2.player = screen.players[1];

    switch(screen.Type)
    {
    case 0: // Follows Player 1
        vscreen1.Height = screen.H;
        vscreen1.Width = screen.W;
        vscreen1.Left = 0;
        vscreen1.Top = 0;
        vscreen2.Visible = false;
        break;
    case 1: // Split Screen vertical
        vscreen1.Height = screen.H / 2;
        vscreen1.Width = screen.W;
        vscreen1.Left = 0;
        vscreen1.Top = 0;
        vscreen2.Height = screen.H / 2;
        vscreen2.Width = screen.W;
        vscreen2.Left = 0;
        vscreen2.Top = screen.H / 2;
        break;
    case 2: // Follows all players
        vscreen1.Height = screen.H;
        vscreen1.Width = screen.W;
        vscreen1.Left = 0;
        vscreen1.Top = 0;
        vscreen2.Visible = false;
        break;
    case 3: // Follows all players. Noone leaves the screen
        vscreen1.Height = screen.H;
        vscreen1.Width = screen.W;
        vscreen1.Left = 0;
        vscreen1.Top = 0;
        vscreen2.Visible = false;
        break;
    case 4: // Split Screen horizontal
        vscreen1.Height = screen.H;
        vscreen1.Width = screen.W / 2;
        vscreen1.Left = 0;
        vscreen1.Top = 0;
        vscreen2.Height = screen.H;
        vscreen2.Width = screen.W / 2;
        vscreen2.Left = screen.W / 2;
        vscreen2.Top = 0;
        break;
    case 5: // Dynamic screen detection
        vscreen1.Height = screen.H;
        vscreen1.Width = screen.W;
        vscreen1.Left = 0;
        vscreen1.Top = 0;

        if(reset)
            vscreen2.Visible = false;
        break;
    case 6: // VScreen Coop
        vscreen1.Height = screen.H;
        vscreen1.Width = screen.W;
        vscreen1.Left = 0;
        vscreen1.Top = 0;
        vscreen2.Height = screen.H;
        vscreen2.Width = screen.W;
        vscreen2.Left = 0;
        vscreen2.Top = 0;
        break;
    case 7: // Credits
    case 8: // netplay
        vscreen1.Left = 0;
        vscreen1.Height = screen.H;
        vscreen1.Top = 0;
        vscreen1.Width = screen.W;
        vscreen2.Visible = false;
        break;
#if 0 // Merged with the branch above because they both are same
    case 8: // netplay
        vscreen1.Left = 0;
        vscreen1.Width = 800;
        vscreen1.Top = 0;
        vscreen1.Height = 600;
        vscreen2.Visible = false;
        break;
#endif
//    End If
    }
}

void SetupScreens(bool reset)
{
    SetupScreens(Screens[0], reset);
}

void DynamicScreen(Screen_t& screen, bool mute)
{
    int A = 0;

    vScreen_t& vscreen1 = screen.vScreen(1);
    vScreen_t& vscreen2 = screen.vScreen(2);

    GetvScreenAverage(vscreen1);

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

    Player_t& p1 = Player[screen.players[0]];
    Player_t& p2 = Player[screen.players[1]];

    if(CheckDead() == 0)
    {
        if(p1.Section == p2.Section)
        {
            const Location_t& section = level[p1.Section];

            // a number of clauses check whether the section is larger than the screen
            bool section_wide = section.Width  - section.X > screen.W;
            bool section_tall = section.Height - section.Y > screen.H;

            // observe that in the original code, there is a condition on whether vScreen 2 is visible, and this decides what P1 is compared with
            // (but NOT P2, which is always compared with vScreen 1). This is actually unnecessary and the logic works fine if both players are compared with
            // vScreen 1. It was likely a vestige from before the GetvScreenAverage call above was added. It must be removed in the modern game
            // because if qScreen is active for vScreen2 (impossible prior to modern section change), the checks no longer work.
            const vScreen_t& p2_compare_vscreen = vscreen1;
            bool use_vscreen2 = !g_compatibility.modern_section_change && vscreen2.Visible;
            const vScreen_t& p1_compare_vscreen = use_vscreen2 ? vscreen2 : vscreen1;

            // explanation of logic (example of first case, all are similar):
            //     if (1) the section is wider than the screen,
            //    and (2) P2's center is >75% to the right of the screen,
            //    and (3) the players could not both fit within the right 75% of the screen at the right of the section, then split

            // the VB6 code here was extremely complicated; including it below so that simplifications can be verified

            // If level(Player(1).Section).Width - level(Player(1).Section).X > ScreenW And
            //   (((vScreen(2).Visible = False   And Player(2).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(2).Location.Width / 2) Or
            //       (vScreen(2).Visible = True  And Player(2).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(2).Location.Width / 2))
            //   And (Player(1).Location.X < level(Player(1).Section).Width - ScreenW * 0.75 - Player(1).Location.Width / 2)) Then

            if(section_wide && (p2.Location.X + p2_compare_vscreen.X >= screen.W * 0.75 - p2.Location.Width / 2.0) && (p1.Location.X < section.Width - screen.W * 0.75 - p1.Location.Width / 2.0))
            {
                vscreen2.Height = screen.H;
                vscreen2.Width = screen.W / 2;
                vscreen2.Left = screen.W / 2.0;
                vscreen2.Top = 0;
                vscreen1.Height = screen.H;
                vscreen1.Width = screen.W / 2;
                vscreen1.Left = 0;
                vscreen1.Top = 0;
                GetvScreenAverage2(vscreen1);
                if(screen.DType != 1 && !mute)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);
                    Player_t& p = Player[screen.players[A - 1]];

                    vscreena.TempDelay = 200;
                    vscreena.tempX = 0;
                    vscreena.TempY = -vscreen1.Y + screen.H * 0.5 - p.Location.Y - vScreenYOffset - p.Location.Height;
                }
                vscreen2.Visible = true;
                screen.DType = 1;
            }

            // ElseIf level(Player(1).Section).Width - level(Player(1).Section).X > ScreenW And
            // (((vScreen(2).Visible = False And Player(1).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(1).Location.Width / 2) Or
            //   (vScreen(2).Visible = True  And Player(1).Location.X + vScreenX(2) >= ScreenW * 0.75 - Player(1).Location.Width / 2))
            // And (Player(2).Location.X < level(Player(1).Section).Width - ScreenW * 0.75 - Player(2).Location.Width / 2)) Then

            else if(section_wide && (p1.Location.X + p1_compare_vscreen.X >= screen.W * 0.75 - p1.Location.Width / 2.0) && (p2.Location.X < section.Width - screen.W * 0.75 - p2.Location.Width / 2.0))
            {
                vscreen1.Height = screen.H;
                vscreen1.Width = screen.W / 2;
                vscreen1.Left = screen.W / 2.0;
                vscreen1.Top = 0;
                vscreen2.Height = screen.H;
                vscreen2.Width = screen.W / 2;
                vscreen2.Left = 0;
                vscreen2.Top = 0;
                GetvScreenAverage2(vscreen1);
                if(screen.DType != 2 && !mute)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);
                    Player_t& p = Player[screen.players[A - 1]];

                    vscreena.TempDelay = 200;
                    vscreena.tempX = 0;
                    vscreena.TempY = -vscreen1.Y + screen.H * 0.5 - p.Location.Y - vScreenYOffset - p.Location.Height;
                }
                screen.DType = 2;
                vscreen2.Visible = true;
            }

            // ElseIf (level(Player(1).Section).Height - level(Player(1).Section).Y > ScreenH And
            // ((vScreen(2).Visible = False And Player(1).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height) Or
            //  (vScreen(2).Visible = True  And Player(1).Location.Y + vScreenY(2) >= ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height))
            // And (Player(2).Location.Y < level(Player(1).Section).Height - ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height)) Then

            else if(section_tall && (p1.Location.Y + p1_compare_vscreen.Y >= screen.H * 0.75 - vScreenYOffset - p1.Location.Height) && (p2.Location.Y < section.Height - screen.H * 0.75 - vScreenYOffset - p2.Location.Height))
            {
                vscreen1.Height = screen.H / 2;
                vscreen1.Width = screen.W;
                vscreen1.Left = 0;
                vscreen1.Top = screen.H / 2.0;
                vscreen2.Height = screen.H / 2;
                vscreen2.Width = screen.W;
                vscreen2.Left = 0;
                vscreen2.Top = 0;
                GetvScreenAverage2(vscreen1);
                if(screen.DType != 3 && !mute)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);
                    Player_t& p = Player[screen.players[A - 1]];

                    vscreena.TempDelay = 200;
                    vscreena.TempY = 0;
                    vscreena.tempX = -vscreen1.X + screen.W * 0.5 - p.Location.X - p.Location.Width * 0.5;
                }
                vscreen2.Visible = true;
                screen.DType = 3;
            }

            // ElseIf (level(Player(1).Section).Height - level(Player(1).Section).Y > ScreenH And
            // ((vScreen(2).Visible = False And Player(2).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height) Or
            //  (vScreen(2).Visible = True  And Player(2).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height))
            // And (Player(1).Location.Y < level(Player(1).Section).Height - ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height)) Then

            else if(section_tall && (p2.Location.Y + p2_compare_vscreen.Y >= screen.H * 0.75 - vScreenYOffset - p2.Location.Height) && (p1.Location.Y < section.Height - screen.H * 0.75 - vScreenYOffset - p1.Location.Height))
            {
                vscreen1.Height = screen.H / 2;
                vscreen1.Width = screen.W;
                vscreen1.Left = 0;
                vscreen1.Top = 0;
                vscreen2.Height = screen.H / 2;
                vscreen2.Width = screen.W;
                vscreen2.Left = 0;
                vscreen2.Top = screen.H / 2.0;
                GetvScreenAverage2(vscreen1);
                if(screen.DType != 4 && !mute)
                    PlaySound(SFX_Camera);
                for(A = 1; A <= 2; A++)
                {
                    vScreen_t& vscreena = screen.vScreen(A);
                    Player_t& p = Player[screen.players[A - 1]];

                    vscreena.TempDelay = 200;
                    vscreena.TempY = 0;
                    vscreena.tempX = -vscreen1.X + screen.W * 0.5 - p.Location.X - p.Location.Width * 0.5;
                }
                vscreen2.Visible = true;
                screen.DType = 4;
            }
            else
            {
                if(vscreen2.Visible)
                {
                    if(screen.DType != 5 && !mute)
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
            if(screen.DType != 6 && !mute)
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

    double camFramesX = std::abs(vScreen[Z].X - qScreenLoc[Z].X) / camRateX;
    double camFramesY = std::abs(vScreen[Z].Y - qScreenLoc[Z].Y) / camRateY;

    // qScreenLoc Width and Height values are only valid if modern section change is disabled
    if(g_compatibility.modern_section_change)
    {
        double camFramesX_r = std::abs(vScreen[Z].X - vScreen[Z].Width - qScreenLoc[Z].X + qScreenLoc[Z].Width) / camRateX;
        double camFramesY_b = std::abs(vScreen[Z].Y - vScreen[Z].Height - qScreenLoc[Z].Y + qScreenLoc[Z].Height) / camRateY;

        camFramesX = SDL_min(camFramesX, camFramesX_r);
        camFramesY = SDL_min(camFramesY, camFramesY_b);
    }

    double resizeFramesX = 0;
    double resizeFramesY = 0;

    double qFramesX = SDL_max(camFramesX, resizeFramesX);
    double qFramesY = SDL_max(camFramesY, resizeFramesY);

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

    camRateX = std::abs(vScreen[Z].X - qScreenLoc[Z].X) / qFramesX;
    camRateY = std::abs(vScreen[Z].Y - qScreenLoc[Z].Y) / qFramesY;

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

    return continue_qScreen;
}

void SetRes()
{
//    GetCurrentRes(); // Dummy, useless
    resChanged = true;
    ChangeRes(800, 600, 16, 60); // 800x600 pixels, 16 bit Color, 60Hz
    XWindow::showCursor(0);
}
