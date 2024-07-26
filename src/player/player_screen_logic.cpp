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

#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "player.h"
#include "graphics.h"
#include "screen.h"
#include "blocks.h"
#include "collision.h"
#include "config.h"

#include "main/trees.h"
#include "main/game_globals.h"

void PlayerSharedScreenLogic(int A)
{
    Screen_t& screen = ScreenByPlayer(A);

    // if(!LevelWrap[Player[A].Section] && LevelMacro == LEVELMACRO_OFF)
    // This section is fully new logic
    if(screen.Type == ScreenTypes::SharedScreen)
    {
        Player_t& p = Player[A];
        const SpeedlessLocation_t& section = level[p.Section];

        const vScreen_t& vscreen = vScreenByPlayer(A);

        // section for shared screen push
        bool check_left = true;
        bool check_right = true;

        bool vscreen_at_section_bound_left = -vscreen.X <= section.X + 8;
        bool vscreen_at_section_bound_right = -vscreen.X + vscreen.Width >= section.Width - 8;

        // normally, don't use the shared screen push at section boundaries
        if(vscreen_at_section_bound_left)
            check_left = false;

        if(vscreen_at_section_bound_right)
            check_right = false;

        // do use shared screen push if there's a different player at the other side of the screen
        for(int o_p_i = 0; o_p_i < screen.player_count; o_p_i++)
        {
            const Player_t& o_p = Player[screen.players[o_p_i]];

            if(o_p.Location.X <= -vscreen.X + 8 && !vscreen_at_section_bound_left)
                check_right = true;
            else if(o_p.Location.X + o_p.Location.Width >= -vscreen.X + vscreen.Width - 8 && !vscreen_at_section_bound_right)
                check_left = true;
        }

        if(p.Location.X <= -vscreen.X + 8 && check_left)
        {
            if(p.Location.X <= -vscreen.X)
            {
                p.Location.X = -vscreen.X;
                p.Pinched.Left2 = 2;

                if(p.Location.SpeedX < 0)
                    p.Location.SpeedX = 0;
            }

            if(p.Location.SpeedX >= 0 && p.Location.SpeedX < 1)
                p.Location.SpeedX = 1;

            // give wings to a player that gets stuck to the left
            if(NoTurnBack[p.Section] && p.Pinched.Right4)
            {
                p.Dead = true;
                p.Effect = PLREFF_COOP_WINGS;
                SharedScreenAvoidJump(screen, 0);
            }
        }
        else if(p.Location.X + p.Location.Width >= -vscreen.X + vscreen.Width - 8 && check_right)
        {
            if(p.Location.X + p.Location.Width >= -vscreen.X + vscreen.Width)
            {
                p.Location.X = -vscreen.X + vscreen.Width - p.Location.Width;
                p.Pinched.Right4 = 2;

                if(p.Location.SpeedX > 0)
                    p.Location.SpeedX = 0;
            }

            if(p.Location.SpeedX > -1 && p.Location.SpeedX <= 0)
                p.Location.SpeedX = -1;
        }


        // give wings to a player that falls offscreen but not off-section
        if(p.Location.Y > -vscreen.Y + vscreen.Height + 64 && p.Location.Y <= level[p.Section].Height + 64)
        {
            p.Dead = true;
            p.Effect = PLREFF_COOP_WINGS;
            SharedScreenAvoidJump(screen, 0);
        }
    }
}

void PlayerEffectWings(int A)
{
    Screen_t& screen = ScreenByPlayer(A);
    Player_t& p = Player[A];

    int vscreen_i = vScreenIdxByPlayer(A);
    const vScreen_t& vscreen = vScreen[vscreen_i];

    p.Fairy = false;

    if(screen.Type == ScreenTypes::SharedScreen)
    {
        bool button_pressed = p.Controls.Jump && p.CanJump;
        if(p.Controls.Jump)
            p.CanJump = false;

        int WingsFrame = 0;

        // check if the player should be moving towards another player or towards the screen
        int target_plr = CheckNearestLiving(A);
        if(target_plr)
            p.Section = Player[target_plr].Section;

        if(!button_pressed)
            target_plr = 0;

        bool offscreen = !vScreenCollision(vscreen_i, p.Location);

        // force to be close to the screen
        if(offscreen)
        {
            if(p.Location.X + p.Location.Width < -vscreen.X - 100)
                p.Location.X = -vscreen.X - 100 - p.Location.Width;
            if(p.Location.X > -vscreen.X + vscreen.Width + 100)
                p.Location.X = -vscreen.X + vscreen.Width + 100;
            if(p.Location.Y + p.Location.Height < -vscreen.Y - 100)
                p.Location.Y = -vscreen.Y - 100 - p.Location.Height;
            if(p.Location.Y > -vscreen.Y + vscreen.Height + 100)
                p.Location.Y = -vscreen.Y + vscreen.Height + 100;
        }

        // move towards screen center or living player
        if(target_plr || offscreen)
        {
            double target_X = (target_plr) ? (Player[target_plr].Location.X + Player[target_plr].Location.Width / 2) : -vscreen.X + vscreen.Width / 2;
            double target_Y = (target_plr) ? (Player[target_plr].Location.Y + Player[target_plr].Location.Height / 2) : -vscreen.Y + vscreen.Height / 2;

            double center_X = p.Location.X + p.Location.Width / 2;
            double center_Y = p.Location.Y + p.Location.Height / 2;

            double target_SpeedX = (target_X - center_X);
            double target_SpeedY = (target_Y - center_Y);

            double target_speed = SDL_sqrt(target_SpeedX * target_SpeedX + target_SpeedY * target_SpeedY);

            if(target_speed != 0.0)
            {
                target_SpeedX *= 8.0 / target_speed;
                target_SpeedY *= 8.0 / target_speed;
            }

            p.Location.SpeedX = p.Location.SpeedX * 0.5 + target_SpeedX * 0.5;
            p.Location.SpeedY = p.Location.SpeedY * 0.5 + target_SpeedY * 0.5;

            if(SoundPause[SFX_Swim] == 0)
            {
                PlaySoundSpatial(SFX_Swim, p.Location);
                SoundPause[SFX_Swim] = 15;
            }
        }
        // deceleration
        else
        {
            double sq_speed = p.Location.SpeedX * p.Location.SpeedX + p.Location.SpeedY * p.Location.SpeedY;
            double decelerate_rate = (sq_speed > 4.0) ? 0.95 : (sq_speed > 1.0) ? 0.99 : 0.999;

            if(sq_speed <= 4.0)
            {
                WingsFrame = 1;

                if(!p.Controls.Jump)
                    p.CanJump = true;
            }

            p.Location.SpeedX *= decelerate_rate;
            p.Location.SpeedY *= decelerate_rate;
        }

        // update frame
        p.SpinJump = false;
        p.WetFrame = false;
        if(p.Location.SpeedY == 0)
            p.Location.SpeedY = 0.00001;
        PlayerFrame(p);
        if(p.Location.SpeedX >= 0)
        {
            p.Direction = 1;
            p.YoshiWingsFrame = WingsFrame + 2;
        }
        else
        {
            p.Direction = -1;
            p.YoshiWingsFrame = WingsFrame;
        }

        // apply movement
        p.Location.X += p.Location.SpeedX;
        p.Location.Y += p.Location.SpeedY;

        // tag other players
        for(int B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Dead || Player[B].TimeToLive != 0)
                continue;

            if(!CheckCollision(Player[A].Location, Player[B].Location))
                continue;

            // want to end up in P2's current location, to avoid clipping blocks
            double target_loc_cX = Player[B].Location.X + Player[B].Location.Width / 2;
            double target_loc_bY = Player[B].Location.Y + Player[B].Location.Height;

            p.Effect = PLREFF_NORMAL;
            PlaySoundSpatial(SFX_Transform, p.Location);

            PlayerCollide(A);

            // if we're inside a block, force ourselves to use the other player's old location.
            // do the Y coordinate first (more gentle), and disable collisions.
            for(int attempt = 0; attempt < 2; attempt++)
            {
                bool had_collision = false;
                for(BlockRef_t b_ref : treeBlockQuery(Player[A].Location, SORTMODE_NONE))
                {
                    const Block_t& b = b_ref;
                    int B = (int)b_ref;

                    if(b.Hidden || b.Invis || BlockIsSizable[b.Type] || BlockOnlyHitspot1[b.Type] || BlockNoClipping[b.Type])
                        continue;

                    if(BlockCheckPlayerFilter(B, A))
                        continue;

                    if(!CheckCollision(Player[A].Location, b.Location))
                        continue;

                    had_collision = true;

                    if(attempt == 0)
                        Player[A].Location.Y = target_loc_bY - Player[A].Location.Height;
                    else
                        Player[A].Location.X = target_loc_cX - Player[A].Location.Width / 2;

                    Player[A].Effect = PLREFF_NO_COLLIDE;

                    break;
                }

                if(!had_collision)
                    break;
            }

            break;
        }
    }
    else
        p.Effect = PLREFF_NORMAL;

    if(p.Effect != PLREFF_COOP_WINGS)
    {
        p.Dead = false;
        p.CanJump = false;
        p.Immune = 50;
        p.Immune2 = false;
        CheckSection(A);
        PlayerFrame(p);

        SharedScreenAvoidJump(screen, 0);
    }
}

void PlayerLevelWrapLogic(int A)
{
    const Screen_t& screen = ScreenByPlayer(A);
    vScreen_t& vscreen = vScreenByPlayer(A);
    Location_t& pLoc = Player[A].Location;
    const SpeedlessLocation_t& section = level[Player[A].Section];

    // track whether screen wrapped in each of the four ways
    bool did_wrap_lr = false;
    bool did_wrap_rl = false;
    bool did_wrap_tb = false;
    bool did_wrap_bt = false;

    // horizontally
    if(LevelWrap[Player[A].Section])
    {
        if(pLoc.X + pLoc.Width < section.X)
        {
            pLoc.X = section.Width - 1;

            if(vscreen.Width < section.Width - section.X)
                did_wrap_lr = true;
        }
        else if(pLoc.X > section.Width)
        {
            pLoc.X = section.X - pLoc.Width + 1;

            if(vscreen.Width < section.Width - section.X)
                did_wrap_rl = true;
        }
    }

    // vertically
    if(LevelVWrap[Player[A].Section])
    {
        if(pLoc.Y + pLoc.Height < section.Y)
        {
            pLoc.Y = section.Height - 1;

            if(vscreen.Height < section.Height - section.Y)
                did_wrap_tb = true;
        }
        else if(pLoc.Y > section.Height)
        {
            pLoc.Y = section.Y - pLoc.Height + 1;

            if(vscreen.Height < section.Height - section.Y)
                did_wrap_bt = true;
        }
    }

    // shared screen: teleport other players to other side of section
    if(screen.Type == ScreenTypes::SharedScreen && (did_wrap_lr || did_wrap_rl || did_wrap_tb || did_wrap_bt))
    {
        double target_Y = pLoc.Y + ((Player[A].Mount != 2) ? pLoc.Height : 0);

        for(int i = 0; i < screen.player_count; i++)
        {
            int o_A = screen.players[i];

            if(o_A == A)
                continue;

            Player_t& o_p = Player[o_A];

            if(o_p.Dead)
                continue;

            // center on player that wrapped
            o_p.Location.X = pLoc.X + pLoc.Width / 2 - o_p.Location.Width / 2;
            o_p.Location.Y = target_Y - ((o_p.Mount != 2) ? o_p.Location.Height : 0);

            // make sure fully in section
            if(did_wrap_lr)
            {
                o_p.Location.X = section.Width - 1;
                if(o_p.Location.SpeedX > 0)
                    o_p.Location.SpeedX = 0;
            }

            if(did_wrap_rl)
            {
                o_p.Location.X = section.X - o_p.Location.Width + 1;
                if(o_p.Location.SpeedX < 0)
                    o_p.Location.SpeedX = 0;
            }

            if(did_wrap_tb)
                o_p.Location.Y = section.Height - 1;

            if(did_wrap_bt)
                o_p.Location.Y = section.Y - o_p.Location.Height + 1;

            // following effects only needed for living players
            if(o_p.TimeToLive != 0)
                continue;

            // remove from any pet's mouth that doesn't belong to this screen
            bool onscreen_pet = InOnscreenPet(o_A, screen);

            // disable collisions and remove from any offscreen pets
            if(!onscreen_pet)
            {
                RemoveFromPet(o_A);
                o_p.Effect = PLREFF_RESPAWN;
                o_p.Effect2 = o_p.Location.Y;
            }
        }

        GetvScreenAuto(vscreen);
    }
}

void PlayerOffscreenExitCheck(int A)
{
    bool offScreenExit = false;
    if(Player[A].Location.X + Player[A].Location.Width < level[Player[A].Section].X)
    {
        offScreenExit = true;
        for(int B = 1; B <= numPlayers; B++)
            Player[B].TailCount = 0;
    }
    else if(Player[A].Location.X > level[Player[A].Section].Width)
    {
        offScreenExit = true;
    }

    if(offScreenExit)
    {
        // Always quit to the world map by off-screen exit
        if(!NoMap && !FileRecentSubHubLevel.empty())
        {
            FileRecentSubHubLevel.clear();
            ReturnWarp = 0;
            ReturnWarpSaved = 0;
        }

        LevelBeatCode = 3;
        EndLevel = true;
        LevelMacro = LEVELMACRO_OFF;
        LevelMacroCounter = 0;

        if(g_config.EnableInterLevelFade)
            g_levelScreenFader.setupFader(4, 0, 65, ScreenFader::S_FADE);
        else
            g_levelScreenFader.setupFader(65, 0, 65, ScreenFader::S_FADE);

        levelWaitForFade();
    }
}

void PlayerLevelEdgeCheck(int A, bool check_X)
{
    if(check_X)
    {
        // Check edge of levels
        if(Player[A].Location.X < level[Player[A].Section].X)
        {
            Player[A].Location.X = level[Player[A].Section].X;

            if(Player[A].Location.SpeedX < 0)
                Player[A].Location.SpeedX = 0;

            Player[A].Pinched.Left2 = 2;

            if(AutoX[Player[A].Section] != 0.0f)
            {
                Player[A].Pinched.Moving = 2;
                Player[A].Pinched.MovingLR = true;
            }
        }
        else if(Player[A].Location.X + Player[A].Location.Width > level[Player[A].Section].Width)
        {
            Player[A].Location.X = level[Player[A].Section].Width - Player[A].Location.Width;

            if(Player[A].Location.SpeedX > 0)
                Player[A].Location.SpeedX = 0;

            Player[A].Pinched.Right4 = 2;

            if(AutoX[Player[A].Section] != 0.f)
            {
                Player[A].Pinched.Moving = 2;
                Player[A].Pinched.MovingLR = true;
            }
        }
    }

    if(Player[A].Location.Y < level[Player[A].Section].Y - Player[A].Location.Height - 32 && Player[A].StandingOnVehiclePlr == 0)
    {
        Player[A].Location.Y = level[Player[A].Section].Y - Player[A].Location.Height - 32;

        if(AutoY[Player[A].Section] != 0.f)
        {
            Player[A].Pinched.Moving = 3;
            Player[A].Pinched.MovingUD = true;
        }
    }
}

void PlayerLevelBoundsLogic(int A)
{
    // When it's true - don't check horizontal section's bounds
    bool hBoundsHandled = false;

    // level wrap
    if(LevelWrap[Player[A].Section] || LevelVWrap[Player[A].Section])
        PlayerLevelWrapLogic(A);

    if(LevelWrap[Player[A].Section])
        hBoundsHandled = true;

    // Walk offscreen exit
    if(!hBoundsHandled && OffScreenExit[Player[A].Section])
    {
        PlayerOffscreenExitCheck(A);
        hBoundsHandled = true;
    }

    if(LevelMacro == LEVELMACRO_CARD_ROULETTE_EXIT || LevelMacro == LEVELMACRO_GOAL_TAPE_EXIT || GameMenu)
        hBoundsHandled = true;

    PlayerLevelEdgeCheck(A, !hBoundsHandled);
}
