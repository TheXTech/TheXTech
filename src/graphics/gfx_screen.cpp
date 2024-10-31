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

#include <sdl_proxy/sdl_stdinc.h>

#include "../globals.h"
#include "../graphics.h"
#include "../player.h"
#include "../sound.h"
#include "../change_res.h"
#include "../load_gfx.h"
#include "../core/window.h"

#include "config.h"
#include "core/render.h"

void SetScreenType(Screen_t& screen)
{
    if(!screen.is_active())
        return;

    auto old_type = screen.Type;

    // moved this code from game_main.cpp, but it occured elsewhere also
    //   it was always called before setup screens, now it is a part of setup screens.
    //   better to have it in one place so it can be updated
    if(screen.player_count == 1)
        screen.Type = ScreenTypes::SinglePlayer; // Follow 1 player
    else if(screen.player_count == 2)
    {
        if(BattleMode)
            screen.Type = ScreenTypes::Dynamic;
        else if(screen.two_screen_pref == MultiplayerPrefs::Split)
            screen.Type = ScreenTypes::LeftRight;
        else if(screen.two_screen_pref == MultiplayerPrefs::Shared)
            screen.Type = ScreenTypes::SharedScreen;
        else if(screen.two_screen_pref == MultiplayerPrefs::TopBottom)
            screen.Type = ScreenTypes::TopBottom;
        else
            screen.Type = ScreenTypes::Dynamic; // Dynamic screen
    }
    else
    {
        if(g_config.fix_npc_camera_logic && (BattleMode || screen.four_screen_pref == MultiplayerPrefs::Split))
            screen.Type = ScreenTypes::Quad;
        else
            screen.Type = ScreenTypes::SharedScreen; // Average, no one leaves the screen
    }

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

#ifndef PGE_MIN_PORT
    if(&screen == l_screen && (old_type == ScreenTypes::Quad) != (screen.Type == ScreenTypes::Quad))
        UpdateInternalRes();
#else
    (void)old_type;
#endif
}

// Sets up the split lines
void SetupScreens(Screen_t& screen, bool reset)
{
    if(!screen.is_active())
        return;

    SetScreenType(screen);

    vScreen_t& vscreen1 = screen.vScreen(1);
    vScreen_t& vscreen2 = screen.vScreen(2);

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
    case 7: // Credits (MODIFIED to include an adjusted chop feature)
        vscreen1.Left = 0;
        vscreen1.Top = (screen.H > 600) ? 100 : (screen.H / 6) & ~1;
        vscreen1.Height = screen.H - 2 * vscreen1.Top;
        vscreen1.Width = screen.W;
        vscreen2.Visible = false;
        break;
    case 8: // netplay
        vscreen1.Left = 0;
        vscreen1.Height = screen.H;
        vscreen1.Top = 0;
        vscreen1.Width = screen.W;
        vscreen2.Visible = false;
        break;
    case ScreenTypes::Quad: // quad
        for(int i = 0; i < 4 && i < maxLocalPlayers; i++)
        {
            vScreen_t& vscreen_i = screen.vScreen(i + 1);

            vscreen_i.Width = screen.W / 2;
            vscreen_i.Height = screen.H / 2;
            vscreen_i.Left = (i & 1) ? screen.W / 2 : 0;
            vscreen_i.Top = (i & 2) ? screen.H / 2 : 0;
        }
        break;
    }

    // clear dynamic screen / section resize offsets when called with reset
    if(reset)
    {
        for(int i = screen.active_begin(); i < screen.active_end(); i++)
        {
            vScreen_t& vscreen_i = screen.vScreen(i + 1);
            vscreen_i.TempDelay = 0;
            vscreen_i.tempX = 0;
            vscreen_i.TempY = 0;
        }
    }
}

void SetupScreens(bool reset)
{
    for(int i = 0; i < c_screenCount; i++)
        SetupScreens(Screens[i], reset);
}

void DynamicScreen(Screen_t& screen, bool mute)
{
    if(!screen.is_active())
        return;

    int A = 0;

    vScreen_t& vscreen1 = screen.vScreen(1);
    vScreen_t& vscreen2 = screen.vScreen(2);

    GetvScreenAverage(vscreen1);

    if(!g_config.multiplayer_pause_controls)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Effect == PLREFF_RESPAWN)
                return;
        }
    }

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 0;
    }

    Player_t& p1 = Player[screen.players[0]];
    Player_t& p2 = Player[screen.players[1]];

    double p1LocY = (p1.Effect == PLREFF_RESPAWN) ? p1.Effect2 : p1.Location.Y;
    double p2LocY = (p2.Effect == PLREFF_RESPAWN) ? p2.Effect2 : p2.Location.Y;

    if(!p1.Dead && !p2.Dead)
    {
        if(p1.Section == p2.Section)
        {
            const SpeedlessLocation_t& section = level[p1.Section];

            // use canonical width for checks in NoTurnBack case
            bool shrink_screen = (NoTurnBack[p1.Section] && g_config.allow_multires);
            const double check_W = shrink_screen ? screen.canonical_screen().W : screen.W;

            // a number of clauses check whether the section is larger than the screen
            bool section_wide = section.Width  - section.X > check_W;
            bool section_tall = section.Height - section.Y > screen.H;

            // observe that in the original code, there is a condition on whether vScreen 2 is visible, and this decides what P1 is compared with
            // (but NOT P2, which is always compared with vScreen 1). This is actually unnecessary and the logic works fine if both players are compared with
            // vScreen 1. It was likely a vestige from before the GetvScreenAverage call above was added. It must be removed if modern_section_change is set,
            // because if qScreen is active for vScreen2 (impossible prior to modern section change), the checks no longer work.
            const vScreen_t& p2_compare_vscreen = vscreen1;
            bool use_vscreen2 = !g_config.modern_section_change && vscreen2.Visible;
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

            if(section_wide && (p2.Location.X + p2_compare_vscreen.X >= check_W * 0.75 - p2.Location.Width / 2.0) && (p1.Location.X < section.Width - check_W * 0.75 - p1.Location.Width / 2.0))
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
                    double pLocY = (A == 1) ? p1LocY : p2LocY;

                    vscreena.TempDelay = 200;
                    vscreena.tempX = 0;
                    vscreena.TempY = -vscreen1.Y + screen.H * 0.5 - pLocY - vScreenYOffset - p.Location.Height;
                }
                vscreen2.Visible = true;
                screen.DType = 1;
            }

            // ElseIf level(Player(1).Section).Width - level(Player(1).Section).X > ScreenW And
            // (((vScreen(2).Visible = False And Player(1).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(1).Location.Width / 2) Or
            //   (vScreen(2).Visible = True  And Player(1).Location.X + vScreenX(2) >= ScreenW * 0.75 - Player(1).Location.Width / 2))
            // And (Player(2).Location.X < level(Player(1).Section).Width - ScreenW * 0.75 - Player(2).Location.Width / 2)) Then

            else if(section_wide && (p1.Location.X + p1_compare_vscreen.X >= check_W * 0.75 - p1.Location.Width / 2.0) && (p2.Location.X < section.Width - check_W * 0.75 - p2.Location.Width / 2.0))
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
                    double pLocY = (A == 1) ? p1LocY : p2LocY;

                    vscreena.TempDelay = 200;
                    vscreena.tempX = 0;
                    vscreena.TempY = -vscreen1.Y + screen.H * 0.5 - pLocY - vScreenYOffset - p.Location.Height;
                }
                screen.DType = 2;
                vscreen2.Visible = true;
            }

            // ElseIf (level(Player(1).Section).Height - level(Player(1).Section).Y > ScreenH And
            // ((vScreen(2).Visible = False And Player(1).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height) Or
            //  (vScreen(2).Visible = True  And Player(1).Location.Y + vScreenY(2) >= ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height))
            // And (Player(2).Location.Y < level(Player(1).Section).Height - ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height)) Then

            else if(section_tall && (p1LocY + p1_compare_vscreen.Y >= screen.H * 0.75 - vScreenYOffset - p1.Location.Height) && (p2LocY < section.Height - screen.H * 0.75 - vScreenYOffset - p2.Location.Height))
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
                    vscreena.tempX = -vscreen1.X + check_W * 0.5 - p.Location.X - p.Location.Width * 0.5;
                }
                vscreen2.Visible = true;
                screen.DType = 3;
            }

            // ElseIf (level(Player(1).Section).Height - level(Player(1).Section).Y > ScreenH And
            // ((vScreen(2).Visible = False And Player(2).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height) Or
            //  (vScreen(2).Visible = True  And Player(2).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height))
            // And (Player(1).Location.Y < level(Player(1).Section).Height - ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height)) Then

            else if(section_tall && (p2LocY + p2_compare_vscreen.Y >= screen.H * 0.75 - vScreenYOffset - p2.Location.Height) && (p1LocY < section.Height - screen.H * 0.75 - vScreenYOffset - p1.Location.Height))
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
                    vscreena.tempX = -vscreen1.X + check_W * 0.5 - p.Location.X - p.Location.Width * 0.5;
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

void DynamicScreens()
{
    for(int i = 0; i < c_screenCount; i++)
    {
        Screen_t& screen = Screens[i];
        if(screen.Type == ScreenTypes::Dynamic)
            DynamicScreen(screen, !screen.Visible);
    }
}

// NEW: limit vScreens to playable section area and center them on the real screen
void CenterScreens(Screen_t& screen)
{
    if(!screen.is_active())
        return;

    // approximate positions of player screens
    double cX1, cY1, cX2, cY2;
    GetPlayerScreen(screen.canonical_screen().W, screen.canonical_screen().H, Player[screen.players[0]], cX1, cY1);
    GetPlayerScreen(screen.canonical_screen().W, screen.canonical_screen().H, Player[screen.players[1]], cX2, cY2);

    double screen_X_distance = SDL_abs(cX1 - cX2);
    double screen_Y_distance = SDL_abs(cY1 - cY2);

    for(int v = 1; v <= maxLocalPlayers; v++)
    {
        vScreen_t& vscreen = screen.vScreen(v);
        const Player_t& p = Player[screen.players[v - 1]];
        const SpeedlessLocation_t& section = level[p.Section];

        vscreen.ScreenLeft = vscreen.Left;
        vscreen.ScreenTop  = vscreen.Top;

        bool in_map = (LevelSelect && !GameMenu);

        // skip centering in all these places (world map sections handled elsewhere)
        if(LevelEditor || WorldEditor || in_map)
            continue;

        // restrict the vScreen to the level if the level is smaller than the screen
        double MaxWidth = section.Width - section.X;
        double MaxHeight = section.Height - section.Y;

        // on 3DS allow a slight amount of expansion for 3D overdraw, if the vscreen covers (and will cover) the entire screen
        int allow_X = (g_config.allow_multires && vscreen.Width == XRender::TargetW && !screen.is_canonical()) ? XRender::TargetCameraOverscanX : 0;

        // don't do overscan if the section will be smaller than the screen after overscan (add 1 for floating precision margin)
        if(MaxWidth + XRender::TargetCameraOverscanX * 2 + 1 < vscreen.Width)
            allow_X = 0;

        MaxWidth += allow_X * 2;

        double MinWidth = 0;
        double MinHeight = 0;

        bool no_turn_back = NoTurnBack[p.Section];

        // restrict single vScreens on NoTurnBack sections
        if(g_config.allow_multires && no_turn_back)
        {
            MaxWidth = SDL_min(MaxWidth, static_cast<double>(screen.canonical_screen().W));

            // limit Visible screens further during DynamicScreen
            if(screen.Visible && screen.Type == 5 && screen.vScreen(2).Visible)
                MaxWidth = SDL_min(MaxWidth, static_cast<double>(screen.canonical_screen().W / 2));
        }

        // allow the canonical vScreens to approach normal screen size during dynamic screen
        if(g_config.allow_multires && !screen.Visible && !no_turn_back)
        {
            MinWidth = screen.W;
            MinHeight = screen.H;

            // allow to grow up to half the size of the visible screen (in dynamic screen), otherwise full screen
            if(screen.Type == ScreenTypes::Dynamic)
            {
                MinWidth = SDL_min(MinWidth, static_cast<double>(screen.visible_screen().W / 2));
                MinHeight = SDL_min(MinHeight, static_cast<double>(screen.visible_screen().H / 2));
            }
            else
            {
                MinWidth = SDL_min(MinWidth, static_cast<double>(screen.visible_screen().W));
                MinHeight = SDL_min(MinHeight, static_cast<double>(screen.visible_screen().H));
            }

            // this ensures a gradual transition between canonical screen size / 2 and visible screen size / 2
            // (no need to use it if in different sections or in permanent splitscreen)
            if(screen.Type == ScreenTypes::Dynamic && screen.DType != DScreenTypes::DiffSections)
            {
                MinWidth = SDL_min(MinWidth, screen_X_distance);
                MinHeight = SDL_min(MinHeight, screen_Y_distance);
            }

            // never get larger than section
            MinWidth = SDL_min(MinWidth, MaxWidth);
            MinHeight = SDL_min(MinHeight, MaxHeight);
        }

        if(MinWidth > vscreen.Width || MaxWidth < vscreen.Width)
        {
            double SetWidth = (MaxWidth < vscreen.Width) ? MaxWidth : MinWidth;

            int left_from_center = vscreen.ScreenLeft - (screen.W / 2);
            int right_from_center = (screen.W / 2) - (vscreen.ScreenLeft + vscreen.Width);
            int total_from_center = (left_from_center + right_from_center);
            int size_diff = vscreen.Width - SetWidth;

            // Move towards center of screen. If left is on center, don't need to move left side of screen.
            // If right is on center, need to fully move left so right can stay. Yields following formula:
            if(total_from_center)
                vscreen.ScreenLeft += size_diff * left_from_center / total_from_center;

            vscreen.Width = SetWidth;
        }

        if(MinHeight > vscreen.Height || MaxHeight < vscreen.Height)
        {
            double SetHeight = (MaxHeight < vscreen.Height) ? MaxHeight : MinHeight;

            int top_from_center = vscreen.ScreenTop - (screen.H / 2);
            int bottom_from_center = (screen.H / 2) - (vscreen.ScreenTop + vscreen.Height);
            int total_from_center = (top_from_center + bottom_from_center);
            int size_diff = vscreen.Height - SetHeight;

            // Move towards center of screen. If top is on center, don't need to move top side of screen.
            // If bottom is on center, need to fully move top so bottom can stay. Yields following formula:
            if(total_from_center)
                vscreen.ScreenTop += size_diff * top_from_center / total_from_center;

            vscreen.Height = SetHeight;
        }
    }
}

void CenterScreens()
{
    for(int i = 0; i < c_screenCount; i++)
    {
        Screen_t& screen = Screens[i];
        CenterScreens(screen);
    }
}

// NEW: moves qScreen towards vScreen, now including the screen size
bool Update_qScreen(int Z, double camRate, double resizeRate)
{
    if(Z == 2 && !g_config.modern_section_change)
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
    if(g_config.modern_section_change)
    {
        double camFramesX_r = std::abs(vScreen[Z].X - vScreen[Z].Width - qScreenLoc[Z].X + qScreenLoc[Z].Width) / camRateX;
        double camFramesY_b = std::abs(vScreen[Z].Y - vScreen[Z].Height - qScreenLoc[Z].Y + qScreenLoc[Z].Height) / camRateY;

        camFramesX = SDL_min(camFramesX, camFramesX_r);
        camFramesY = SDL_min(camFramesY, camFramesY_b);
    }

    double resizeFramesX = std::abs(vScreen[Z].Width - qScreenLoc[Z].Width) / resizeRateX;
    double resizeFramesY = std::abs(vScreen[Z].Height - qScreenLoc[Z].Height) / resizeRateY;

    if(!g_config.modern_section_change)
    {
        resizeFramesX = 0;
        resizeFramesY = 0;
    }

    double qFramesX = SDL_max(camFramesX, resizeFramesX);
    double qFramesY = SDL_max(camFramesY, resizeFramesY);

    // don't continue after this frame if it would arrive next frame
    // (this is the intent of the <5 condition in the vanilla game)
    if(qFramesX < 2.5 && qFramesY < 2.5)
        continue_qScreen = false;

    // but, the original condition occurred *after* adding/subtracting 2, so actually
    // the original game would not continue if it would arrive the frame after next, too
    if(!g_config.modern_section_change && qFramesX < 3.5 && qFramesY < 3.5)
        continue_qScreen = false;

    if(qFramesX < 1)
        qFramesX = 1;
    if(qFramesY < 1)
        qFramesY = 1;

    camRateX = std::abs(vScreen[Z].X - qScreenLoc[Z].X) / qFramesX;
    camRateY = std::abs(vScreen[Z].Y - qScreenLoc[Z].Y) / qFramesY;

    resizeRateX = std::abs(vScreen[Z].Width - qScreenLoc[Z].Width) / qFramesX;
    resizeRateY = std::abs(vScreen[Z].Height - qScreenLoc[Z].Height) / qFramesY;

    double screenRateX = std::abs(vScreen[Z].ScreenLeft - qScreenLoc[Z].ScreenLeft) / qFramesX;
    double screenRateY = std::abs(vScreen[Z].ScreenTop - qScreenLoc[Z].ScreenTop) / qFramesY;

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

    if(vScreen[Z].ScreenLeft < qScreenLoc[Z].ScreenLeft - screenRateX)
        qScreenLoc[Z].ScreenLeft -= screenRateX;
    else if(vScreen[Z].ScreenLeft > qScreenLoc[Z].ScreenLeft + screenRateX)
        qScreenLoc[Z].ScreenLeft += screenRateX;
    else
        qScreenLoc[Z].ScreenLeft = vScreen[Z].ScreenLeft;

    if(vScreen[Z].ScreenTop < qScreenLoc[Z].ScreenTop - screenRateY)
        qScreenLoc[Z].ScreenTop -= screenRateY;
    else if(vScreen[Z].ScreenTop > qScreenLoc[Z].ScreenTop + screenRateY)
        qScreenLoc[Z].ScreenTop += screenRateY;
    else
        qScreenLoc[Z].ScreenTop = vScreen[Z].ScreenTop;

    if(vScreen[Z].Width < qScreenLoc[Z].Width - resizeRateX)
        qScreenLoc[Z].Width -= resizeRateX;
    else if(vScreen[Z].Width > qScreenLoc[Z].Width + resizeRateX)
        qScreenLoc[Z].Width += resizeRateX;
    else
        qScreenLoc[Z].Width = vScreen[Z].Width;

    if(vScreen[Z].Height < qScreenLoc[Z].Height - resizeRateY)
        qScreenLoc[Z].Height -= resizeRateY;
    else if(vScreen[Z].Height > qScreenLoc[Z].Height + resizeRateY)
        qScreenLoc[Z].Height += resizeRateY;
    else
        qScreenLoc[Z].Height = vScreen[Z].Height;

    vScreen[Z].X = qScreenLoc[Z].X;
    vScreen[Z].Y = qScreenLoc[Z].Y;

    // update vScreen width / height
    if(g_config.modern_section_change)
    {
        vScreen[Z].Width = std::floor(qScreenLoc[Z].Width / 2) * 2;
        vScreen[Z].Height = std::floor(qScreenLoc[Z].Height / 2) * 2;
        vScreen[Z].ScreenLeft = std::floor(qScreenLoc[Z].ScreenLeft);
        vScreen[Z].ScreenTop = std::floor(qScreenLoc[Z].ScreenTop);
    }

    return continue_qScreen;
}
