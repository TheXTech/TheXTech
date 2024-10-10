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

#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "player.h"
#include "game_main.h"
#include "sound.h"
#include "change_res.h"
#include "main/game_info.h"
#include "core/render.h"
#include "core/events.h"

#include "graphics/gfx_frame.h"
#include "graphics/gfx_camera.h"
#include "graphics/gfx_world.h"

#include "pseudo_vb.h"
#include "gfx.h"
#include "config.h"
#include "npc_traits.h"

#include <Utils/maths.h>


//  Get the screen position
void GetvScreen(vScreen_t& vscreen)
{
    auto &p = Player[vscreen.player];
    auto &pLoc = p.Location;

    if(p.Mount == 2)
        pLoc.Height = 0;

    // this check is new because players can now respawn in 1P mode through DropAdd
    double pLocY = (p.Effect == PLREFF_RESPAWN) ? p.Effect2 : pLoc.Y;

    vscreen.X = -pLoc.X + (vscreen.Width * 0.5) - pLoc.Width / 2.0;
    vscreen.Y = -pLocY + (vscreen.Height * 0.5) - vScreenYOffset - pLoc.Height;

    ProcessSmallScreenCam(vscreen);

    vscreen.X += -vscreen.tempX;
    vscreen.Y += -vscreen.TempY;

    // allow some overscan (needed for 3DS)
    int allow_X = (g_config.allow_multires && vscreen.Width == XRender::TargetW && !Screens[vscreen.screen_ref].is_canonical()) ? XRender::TargetCameraOverscanX : 0;

    // shift the level so that it is onscreen
    if(-vscreen.X < level[p.Section].X - allow_X)
        vscreen.X = -(level[p.Section].X - allow_X);
    if(-vscreen.X + vscreen.Width > level[p.Section].Width + allow_X)
        vscreen.X = -(level[p.Section].Width - vscreen.Width + allow_X);
    if(-vscreen.Y < level[p.Section].Y)
        vscreen.Y = -level[p.Section].Y;
    if(-vscreen.Y + vscreen.Height > level[p.Section].Height)
        vscreen.Y = -(level[p.Section].Height - vscreen.Height);

    if(vscreen.TempDelay > 0)
        vscreen.TempDelay -= 1;
    else
    {
        if(vscreen.tempX > 0)
            vscreen.tempX -= 1;
        if(vscreen.tempX < 0)
            vscreen.tempX += 1;
        if(vscreen.TempY > 0)
            vscreen.TempY -= 1;
        if(vscreen.TempY < 0)
            vscreen.TempY += 1;
    }

    if(p.Mount == 2)
        pLoc.Height = 128;
}

// Get the average screen position for all players
void GetvScreenAverage(vScreen_t& vscreen)
{
    int A = 0;
    int B = 0;
    double OldX = 0;
    double OldY = 0;

    OldX = vscreen.X;
    OldY = vscreen.Y;

    vscreen.X = 0;
    vscreen.Y = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead && (Player[A].Effect != PLREFF_RESPAWN || g_config.multiplayer_pause_controls))
        {
            vscreen.X += -Player[A].Location.X - Player[A].Location.Width / 2.0;

            double pLocY = (Player[A].Effect == PLREFF_RESPAWN) ? Player[A].Effect2 : Player[A].Location.Y;

            if(Player[A].Mount == 2)
                vscreen.Y += -pLocY;
            else
                vscreen.Y += -pLocY - Player[A].Location.Height;

            B += 1;
        }
    }

    // A = 1;
    if(B == 0)
    {
        if(GameMenu)
        {
            vscreen.X = -level[0].X;
            B = 1;
        }
        else
        {
            vscreen.X = OldX;
            vscreen.Y = OldY;
            return;
        }
    }

    // used ScreenW / ScreenH in VB6 code
    const Screen_t& screen = Screens[vscreen.screen_ref];

    const SpeedlessLocation_t& section = level[Player[1].Section];

    // remember that the screen will be limited to the section's size in all cases
    double use_width  = SDL_min(static_cast<double>(screen.W), section.Width  - section.X);
    double use_height = SDL_min(static_cast<double>(screen.H), section.Height - section.Y);

    vscreen.X = (vscreen.X / B) + (use_width * 0.5);
    vscreen.Y = (vscreen.Y / B) + (use_height * 0.5) - vScreenYOffset;

    // allow some overscan (needed for 3DS)
    int allow_X = (g_config.allow_multires && vscreen.Width == XRender::TargetW && !Screens[vscreen.screen_ref].is_canonical()) ? XRender::TargetCameraOverscanX : 0;

    if(-vscreen.X < section.X - allow_X)
        vscreen.X = -(section.X - allow_X);
    if(-vscreen.X + use_width > section.Width + allow_X)
        vscreen.X = -(section.Width - use_width + allow_X);
    if(-vscreen.Y < section.Y)
        vscreen.Y = -section.Y;
    if(-vscreen.Y + use_height > section.Height)
        vscreen.Y = -(section.Height - use_height);

    if(GameMenu)
    {
        if(vscreen.X > OldX)
        {
            if(fEqual(vscreen.X, -level[0].X))
                vscreen.X = OldX + 20;
            else
                vscreen.X = OldX;
        }
        else if(vscreen.X < OldX - 10)
            vscreen.X = OldX - 10;

        // on menu, bottom of screen always tracks bottom of level
        vscreen.Y = -(level[Player[1].Section].Height - vscreen.Height);
    }
}

// Get the average screen position for all players with no level edge detection
void GetvScreenAverage2(vScreen_t& vscreen)
{
    // int A = 0;
    int B = 0;

    vscreen.X = 0;
    vscreen.Y = 0;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead)
        {
            vscreen.X += -Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vscreen.Y += -Player[A].Location.Y;
            else
                vscreen.Y += -Player[A].Location.Y - Player[A].Location.Height;
            B += 1;
        }
    }

    //A = 1; // Stored value gets never read

    if(B == 0)
        return;

    const Screen_t& screen = Screens[vscreen.screen_ref];

    const SpeedlessLocation_t& section = level[Player[1].Section];

    double use_width  = SDL_min(static_cast<double>(screen.W), section.Width  - section.X);
    double use_height = SDL_min(static_cast<double>(screen.H), section.Height - section.Y);

    vscreen.X = (vscreen.X / B) + (use_width * 0.5);
    vscreen.Y = (vscreen.Y / B) + (use_height * 0.5) - vScreenYOffset;
}

// Get the average screen position for all players for ScreenType 3
// Uses only the furthest left and right players for the X position
// Doubles the weight of the top player for the Y position
void GetvScreenAverage3(vScreen_t& vscreen)
{
    int plr_count = 0;
    double OldX = 0;
    double OldY = 0;

    OldX = vscreen.X;
    OldY = vscreen.Y;

    // calculate average Y position
    vscreen.Y = 0;
    double Y_not_warping = 0;
    int not_warping_count = 0;

    // find furthest left, right, top, and bottom players
    double l, r, t, b;

    const Screen_t& screen = Screens[vscreen.screen_ref];

    int section_idx = Player[1].Section;

    for(int i = 0; i < screen.player_count; i++)
    {
        const Player_t& plr = Player[screen.players[i]];

        if(plr.Dead)
            continue;

        if(plr_count == 0)
            section_idx = plr.Section;

        double pl = plr.Location.X;
        double pr = pl + plr.Location.Width;

        // DANGER: consider case where players are free-fall and a player respawns
        double pLocY = (plr.Effect == PLREFF_RESPAWN) ? plr.Effect2 : plr.Location.Y;

        double by = pLocY + plr.Location.Height;

        if(plr_count == 0 || by < t)
            t = by;
        if(plr_count == 0 || b < by)
            b = by;
        if(plr_count == 0 || pl < l)
            l = pl;
        if(plr_count == 0 || r < pr)
            r = pr;

        vscreen.Y -= by;
        plr_count += 1;

        if(!PlayerWaitingInWarp(plr))
        {
            Y_not_warping -= by;
            not_warping_count += 1;
        }
    }

    if(plr_count == 0)
    {
        vscreen.X = OldX;
        vscreen.Y = OldY;
        return;
    }

    // 2x the contribution of the top player to the result
    vscreen.Y -= t;
    plr_count += 1;

    // take the average, but if the players waiting to exit a warp are keeping the screen too high, don't let them control the screen
    double mean_Y = vscreen.Y / plr_count;
    if(not_warping_count != 0)
    {
        double mean_Y_not_warping = Y_not_warping / not_warping_count;
        // if the warping players are pulling the screen up by more than 100px, ignore them
        double allowed_height = 100;
        if(mean_Y > mean_Y_not_warping + allowed_height)
            mean_Y = mean_Y_not_warping + allowed_height;
    }

    const SpeedlessLocation_t& section = level[section_idx];

    double use_width  = screen.W;
    double use_height = screen.H;

    // allow canonical screen to expand to reach size of main screen, if there are players near the side of the screen
    if(g_config.allow_multires && !screen.Visible)
    {
        // vScreen boundaries that would have been present in SMBX 1.3 splitscreen
        double want_l = l - screen.W / 4;
        double want_r = r + screen.W / 4;
        double want_t = t - screen.H / 4;
        double want_b = b + screen.H / 4;

        if(want_t > -mean_Y - screen.H / 2)
            want_t = -mean_Y - screen.H / 2;

        if(want_b < -mean_Y + screen.H / 2)
            want_b = -mean_Y + screen.H / 2;

        if(want_l < section.X)
            want_l = section.X;
        if(want_r > section.Width)
            want_r = section.Width;
        if(want_t < section.Y)
            want_t = section.Y;
        if(want_b > section.Height)
            want_b = section.Height;

        if(want_r - want_l > use_width)
            use_width = want_r - want_l;

        if(want_b - want_t > use_height)
            use_height = want_b - want_t;

        use_width  = SDL_min(use_width,  static_cast<double>(screen.visible_screen().W));
        // don't limit by visible screen: needed to handle unusual shared screen cases found by Sapphire Bullet Bill
        // use_height = SDL_min(use_height, static_cast<double>(screen.visible_screen().H));

        vscreen.Width = use_width;
        vscreen.Height = use_height;

        if(use_height > screen.H)
            mean_Y = -(want_t + want_b) / 2;
    }

    // if a NoTurnBack section, make sure that the limited width is tracked
    if(g_config.allow_multires && NoTurnBack[section_idx])
        use_width = SDL_min(use_width, static_cast<double>(screen.canonical_screen().W));

    use_width  = SDL_min(use_width,  section.Width  - section.X);
    use_height = SDL_min(use_height, section.Height - section.Y);

    vscreen.X = -(l + r) / 2 + (use_width * 0.5);
    vscreen.Y = mean_Y + (use_height * 0.5) - vScreenYOffset;

    // allow some overscan (needed for 3DS)
    int allow_X = (g_config.allow_multires && vscreen.Width == XRender::TargetW && !Screens[vscreen.screen_ref].is_canonical()) ? XRender::TargetCameraOverscanX : 0;

    for(int i = 0; i < 2; i++)
    {
        if(-vscreen.X < section.X - allow_X)
            vscreen.X = -(section.X - allow_X);
        if(-vscreen.X + use_width > section.Width + allow_X)
            vscreen.X = -(section.Width - use_width + allow_X);
        if(-vscreen.Y < section.Y)
            vscreen.Y = -section.Y;
        if(-vscreen.Y + use_height > section.Height)
            vscreen.Y = -(section.Height - use_height);

        if(i == 1)
            break;

        if(vscreen.tempX == 0 && vscreen.TempY == 0 && vscreen.TempDelay == 0)
            break;

        // apply vScreen temp
        vscreen.X += -vscreen.tempX;
        vscreen.Y += -vscreen.TempY;

        if(vscreen.TempDelay > 0)
        {
            vscreen.TempDelay--;
            continue;
        }

        if(vscreen.tempX >= 2)
            vscreen.tempX -= 2;
        else if(vscreen.tempX <= -2)
            vscreen.tempX += 2;
        else
            vscreen.tempX = 0;

        if(vscreen.TempY >= 2)
            vscreen.TempY -= 2;
        else if(vscreen.TempY <= -2)
            vscreen.TempY += 2;
        else
            vscreen.TempY = 0;
    }
}

// NEW: update a vScreen with the correct procedure based on its screen's Type and DType
void GetvScreenAuto(vScreen_t& vscreen)
{
    const Screen_t& screen = Screens[vscreen.screen_ref];

    if(screen.Type == 3)
        GetvScreenAverage3(vscreen);
    else if(screen.Type == 2 || (screen.Type == 5 && !screen.vScreen(2).Visible))
        GetvScreenAverage(vscreen);
    else if(screen.Type == 7)
        GetvScreenCredits(vscreen);
    else
        GetvScreen(vscreen);
}

void SharedScreenAvoidJump_Pre(Screen_t& screen)
{
    if(screen.Type != ScreenTypes::SharedScreen)
        return;

    auto& vscreen = screen.vScreen(1);

    GetvScreenAverage3(vscreen);

    if(!screen.is_canonical())
        SharedScreenAvoidJump_Pre(screen.canonical_screen());
}

void SharedScreenAvoidJump_Post(Screen_t& screen, int Delay)
{
    if(screen.Type != ScreenTypes::SharedScreen)
        return;

    auto& vscreen = screen.vScreen(1);

    double curX = vscreen.X;
    double curY = vscreen.Y;

    vscreen.tempX = 0;
    vscreen.TempY = 0;

    GetvScreenAverage3(vscreen);

    vscreen.tempX = vscreen.X - curX;
    vscreen.TempY = vscreen.Y - curY;

    vscreen.X = curX;
    vscreen.Y = curY;

    vscreen.TempDelay = Delay;

    if(!screen.is_canonical())
        SharedScreenAvoidJump_Post(screen.canonical_screen(), Delay);
}

void SharedScreenResetTemp(Screen_t& screen)
{
    if(screen.Type != ScreenTypes::SharedScreen)
        return;

    auto& vscreen = screen.vScreen(1);

    vscreen.tempX = 0;
    vscreen.TempY = 0;
    vscreen.TempDelay = 0;

    if(!screen.is_canonical())
        SharedScreenResetTemp(screen.canonical_screen());
}

// NEW: get the fixed-resolution vScreen position for a player, and write the top-left coordinate to (left, top)
void GetPlayerScreen(double W, double H, const Player_t& p, double& left, double& top)
{
    auto &pLoc = p.Location;

    double pHeight = (p.Mount != 2) ? pLoc.Height : 0;

    left = -pLoc.X + (W * 0.5) - pLoc.Width / 2.0;
    top = -pLoc.Y + (H * 0.5) - vScreenYOffset - pHeight;

    // limit to level bounds
    if(-left < level[p.Section].X)
        left = -level[p.Section].X;
    else if(-left + W > level[p.Section].Width)
        left = -(level[p.Section].Width - W);

    if(-top < level[p.Section].Y)
        top = -level[p.Section].Y;
    else if(-top + H > level[p.Section].Height)
        top = -(level[p.Section].Height - H);
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

static inline int s_round2int(double d)
{
    return std::floor(d + 0.5);
}

static inline int s_round2int_plr(double d)
{
#ifdef PGE_MIN_PORT
    return (int)(std::floor(d / 2 + 0.5)) * 2;
#else
    return std::floor(d + 0.5);
#endif
}

static inline IntegerLocation_t s_round2int(const SpeedlessLocation_t& d)
{
    IntegerLocation_t ret;

    ret.X = s_round2int(d.X); ret.Y = s_round2int(d.Y); ret.Width = s_round2int(d.Width); ret.Height = s_round2int(d.Height);

    return ret;
}

static inline IntegerLocation_t s_round2int_plr(const SpeedlessLocation_t& d)
{
    IntegerLocation_t ret;

    ret.X = s_round2int_plr(d.X); ret.Y = s_round2int_plr(d.Y); ret.Width = s_round2int_plr(d.Width); ret.Height = s_round2int_plr(d.Height);

    return ret;
}

void PlayerWarpGFX(int A, IntegerLocation_t &tempLocation, int &X2, int &Y2)
{
    auto &player = Player[A];
    bool backward = player.WarpBackward;
    auto &warp = Warp[player.Warp];
    const SpeedlessLocation_t &_warp_enter = backward ? warp.Exit : warp.Entrance;
    const SpeedlessLocation_t &_warp_exit = backward ? warp.Entrance : warp.Exit;
    auto &warp_dir_enter = backward ? warp.Direction2 : warp.Direction;
    auto &warp_dir_exit = backward ? warp.Direction : warp.Direction2;

    IntegerLocation_t warp_enter = s_round2int_plr(_warp_enter);
    IntegerLocation_t warp_exit = s_round2int_plr(_warp_exit);

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
                Y2 = warp_enter.Y - tempLocation.Y;
                tempLocation.Y = warp_enter.Y;
                tempLocation.Height += -Y2;
            }
        }
        else if(warp_dir_enter == 4) // Moving right
            tempLocation.Width = (warp_enter.X + warp_enter.Width) - (tempLocation.X);
        else if(warp_dir_enter == 2) // Moving left
        {
            X2 = warp_enter.X - tempLocation.X;
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
                Y2 = warp_exit.Y - tempLocation.Y;
                tempLocation.Y = warp_exit.Y;
                tempLocation.Height += -Y2;
            }
        }
        else if(warp_dir_exit == 4) // Moving left
            tempLocation.Width = (warp_exit.X + warp_exit.Width) - (tempLocation.X);
        else if(warp_dir_exit == 2) // Moving right
        {
            X2 = warp_exit.X - tempLocation.X;
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

    tempLocation.Width -= X2;
}

void NPCWarpGFX(int A, IntegerLocation_t &tempLocation, int &X2, int &Y2)
{
    auto &player = Player[A];
    bool backward = player.WarpBackward;
    auto &warp = Warp[player.Warp];
    const SpeedlessLocation_t &_warp_enter = backward ? warp.Exit : warp.Entrance;
    const SpeedlessLocation_t &_warp_exit = backward ? warp.Entrance : warp.Exit;
    auto &warp_dir_enter = backward ? warp.Direction2 : warp.Direction;
    auto &warp_dir_exit = backward ? warp.Direction : warp.Direction2;

    IntegerLocation_t warp_enter = s_round2int_plr(_warp_enter);
    IntegerLocation_t warp_exit = s_round2int_plr(_warp_exit);

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
                Y2 = warp_enter.Y - tempLocation.Y;
                tempLocation.Y = warp_enter.Y;
                tempLocation.Height += -Y2;
            }
        }
        else if(warp_dir_enter == 4) // Moving right
            tempLocation.Width = (warp_enter.X + warp_enter.Width) - (tempLocation.X);
        else if(warp_dir_enter == 2) // Moving left
        {
            X2 = warp_enter.X - tempLocation.X;
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
                Y2 = warp_exit.Y - tempLocation.Y;
                tempLocation.Y = warp_exit.Y;
                tempLocation.Height += -Y2;
            }
        }
        else if(warp_dir_exit == 4) // Moving left
            tempLocation.Width = (warp_exit.X + warp_exit.Width) - (tempLocation.X);
        else if(warp_dir_exit == 2) // Moving right
        {
            X2 = warp_exit.X - tempLocation.X;
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
#ifndef RENDER_FULLSCREEN_ALWAYS
    // shouldn't be possible
    if(g_config.fullscreen.m_set > ConfigSetLevel::user_config)
    {
        PlaySoundMenu(SFX_BlockHit);
        return;
    }

    g_config_game_user.fullscreen = !g_config.fullscreen;
    UpdateConfig();
    SaveConfig();
#endif
}

void GetvScreenCredits(vScreen_t& vscreen)
{
    int A = 0;
    int B = 0;

    vscreen.X = 0;
    vscreen.Y = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        if((!Player[A].Dead || g_gameInfo.outroDeadMode) && Player[A].Effect != PLREFF_RESPAWN)
        {
            vscreen.X += -Player[A].Location.X - Player[A].Location.Width / 2.0;

            if(Player[A].Mount == 2)
                vscreen.Y += -Player[A].Location.Y;
            else
                vscreen.Y += -Player[A].Location.Y - Player[A].Location.Height;

            B++;
        }
    }

    A = 1;
    if(B == 0)
        return;

    // used ScreenW / ScreenH in VB6 code, using vScreen.Width / Height here

    const SpeedlessLocation_t& section = level[Player[1].Section];

    // remember that the screen will be limited to the section's size in all cases
    double use_width  = SDL_min(vscreen.Width,  section.Width  - section.X);
    double use_height = SDL_min(vscreen.Height, section.Height - section.Y);

    vscreen.X = (vscreen.X / B) + (use_width * 0.5);
    vscreen.Y = (vscreen.Y / B) + (use_height * 0.5) - vScreenYOffset;

    if(-vscreen.X < section.X)
        vscreen.X = -section.X;
    if(-vscreen.X + use_width > section.Width)
        vscreen.X = -(section.Width - use_width);
    if(-vscreen.Y < section.Y)
        vscreen.Y = -section.Y;
    if(-vscreen.Y + use_height > section.Height)
        vscreen.Y = -(section.Height - use_height);
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
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    auto &n = NPC[A];

    int sX = camX + s_round2int(NPC[A].Location.X);
    int sY = camY + s_round2int(NPC[A].Location.Y);
    int w = s_round2int(NPC[A].Location.Width);
    int h = s_round2int(NPC[A].Location.Height);

    if(NPC[A].HoldingPlayer != 0)
    {
        sX = camX + s_round2int_plr(NPC[A].Location.X);
        sY = camY + s_round2int_plr(NPC[A].Location.Y);
    }

    // collision already checked elsewhere
    // if((vScreenCollision(Z, n.Location) ||
    //     vScreenCollision(Z, newLoc(n.Location.X - (n->WidthGFX - n.Location.Width) / 2,
    //                         n.Location.Y, CDbl(n->WidthGFX), CDbl(n->THeight)))) && !n.Hidden)
    if(true)
    {
// draw npc
        XTColor c = n.Shadow ? XTColor(0, 0, 0) : XTColor();
        int content = n.Special;
        int contentFrame = n.Special2;

        // SDL_assert_release(content >= 0 && content <= maxNPCType);

        // Draw frozen NPC body in only condition the content value is valid
        if(content > 0 && content <= maxNPCType)
        {
             XRender::renderTextureBasic(sX + 2,
                                    sY + 2,
                                    w - 4,
                                    h - 4,
                                    GFXNPCBMP[content],
                                    2, 2 + contentFrame * NPCHeight(content),
                                    c);
        }

        // draw ice
         XRender::renderTextureBasic(sX + n->FrameOffsetX,
                                sY + n->FrameOffsetY,
                                w - 6, h - 6,
                                GFXNPCBMP[n.Type], 0, 0, c);
         XRender::renderTextureBasic(sX + n->FrameOffsetX + w - 6,
                                sY + n->FrameOffsetY,
                                6, h - 6,
                                GFXNPCBMP[n.Type], 128 - 6, 0, c);
         XRender::renderTextureBasic(sX + n->FrameOffsetX,
                                sY + n->FrameOffsetY + h - 6,
                                w - 6, 6,
                                GFXNPCBMP[n.Type], 0, 128 - 6, c);
         XRender::renderTextureBasic(sX + n->FrameOffsetX + w - 6,
                                sY + n->FrameOffsetY + h - 6,
                                6, 6, GFXNPCBMP[n.Type],
                                128 - 6, 128 - 6, c);
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

Location_t WorldLevel_t::LocationOnscreen()
{
    Location_t ret = LocationGFX();

    if(Path2)
    {
        if(ret.Height < 40)
            ret.Height = 40;

        if(ret.Width < 64)
        {
            ret.X += (ret.Width - 64) / 2;
            ret.Width = 64;
        }
    }

    return ret;
}

void DrawBackdrop(const Screen_t& screen)
{
    if(GFX.Backdrop.inited)
    {
        bool border_valid = GFX.Backdrop_Border.tex.inited && (!GFX.isCustom(71) || GFX.isCustom(72));

        // special case for world map
        if(LevelSelect && !GameMenu && !GameOutro && !LevelEditor)
        {
            Location_t full = newLoc(0, 0, XRender::TargetW, XRender::TargetH);
            Location_t inner = newLoc(screen.TargetX(), screen.TargetY(), screen.W, screen.H);

            // if world map frame assets missing, use the 800x600 area isntead
            if(!worldHasFrameAssets())
            {
                inner.X = screen.vScreen(1).TargetX() - 66;
                inner.Y = screen.vScreen(1).TargetY() - 130;
                inner.Width = 800;
                inner.Height = 600;
            }

            RenderFrameBorder(full, inner,
                GFX.Backdrop, border_valid ? &GFX.Backdrop_Border : nullptr);

            return;
        }

        for(int i = screen.active_begin(); i < screen.active_end(); i++)
        {
            const auto& s = screen.vScreen(i + 1);

            Location_t full = newLoc(0, 0, XRender::TargetW, XRender::TargetH);
            // horizontal
            if(screen.Type == 4 || (screen.Type == 5 && (screen.DType == 1 || screen.DType == 2)))
            {
                full.Width = XRender::TargetW / 2;
                // our screen on right
                if(((screen.Type == 4 || (screen.Type == 5 && screen.DType == 1)) && i == 1) || (screen.DType == 2 && i == 0))
                    full.X = XRender::TargetW / 2;
            }
            // vertical
            else if(screen.Type == 1 || (screen.Type == 5 && (screen.DType == 3 || screen.DType == 4 || screen.DType == 6)))
            {
                full.Height = XRender::TargetH / 2;
                // our screen on bottom
                if(((screen.Type == 1 || (screen.Type == 5 && (screen.DType == 3 || screen.DType == 6))) && i == 1) || (screen.DType == 4 && i == 0))
                    full.Y = XRender::TargetH / 2;
            }

            RenderFrameBorder(full, newLoc(s.TargetX(), s.TargetY(), s.Width, s.Height),
                GFX.Backdrop, border_valid ? &GFX.Backdrop_Border : nullptr);
        }
    }
}
