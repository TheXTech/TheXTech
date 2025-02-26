/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

                // give wings to a player that would be killed by pinching
                if(p.Pinched.Right4 && p.Pinched.Moving && !p.Pinched.Left2)
                    p.Effect = PLREFF_COOP_WINGS;
                else
                    p.Pinched.Left2 = 2;

                if(p.Location.SpeedX < 0)
                    p.Location.SpeedX = 0;
            }

            if(p.Location.SpeedX >= 0 && p.Location.SpeedX < 1)
                p.Location.SpeedX = 1;

            // give wings to a player that gets stuck to the left
            if(NoTurnBack[p.Section] && p.Pinched.Right4)
                p.Effect = PLREFF_COOP_WINGS;
        }
        else if(p.Location.X + p.Location.Width >= -vscreen.X + vscreen.Width - 8 && check_right)
        {
            if(p.Location.X + p.Location.Width >= -vscreen.X + vscreen.Width)
            {
                p.Location.X = -vscreen.X + vscreen.Width - p.Location.Width;

                // give wings to a player that would be killed by pinching
                if(p.Pinched.Left2 && p.Pinched.Moving && !p.Pinched.Right4)
                    p.Effect = PLREFF_COOP_WINGS;
                else
                    p.Pinched.Right4 = 2;

                if(p.Location.SpeedX > 0)
                    p.Location.SpeedX = 0;
            }

            if(p.Location.SpeedX > -1 && p.Location.SpeedX <= 0)
                p.Location.SpeedX = -1;
        }


        // give wings to a player that falls offscreen but not off-section
        if(p.Location.Y > -vscreen.Y + vscreen.Height + 64 && p.Location.Y <= level[p.Section].Height)
            p.Effect = PLREFF_COOP_WINGS;

        if(p.Effect == PLREFF_COOP_WINGS)
        {
            SharedScreenAvoidJump_Pre(screen);
            p.Dead = true;
            p.Effect2 = 0;
            SizeCheck(p);
            SharedScreenAvoidJump_Post(screen, 0);
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
            double target_Y = (target_plr) ? Player[target_plr].Location.Y - 8 : -vscreen.Y + vscreen.Height / 2;

            int randomness = (target_plr) ? 8 : vscreen.Width / 10;
            target_X += iRand(randomness * 2) - randomness;
            target_Y += iRand(randomness * 2) - randomness;

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
        bool found_player = false;
        for(int B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Dead || Player[B].TimeToLive != 0 || Player[B].Effect != PLREFF_NORMAL)
                continue;

            if(!CheckCollision(Player[A].Location, Player[B].Location))
                continue;

            found_player = true;
            break;
        }

        // just collided with player
        if(found_player && !p.Effect2)
        {
            // if we're inside a block, then we can't respawn yet
            bool hit_block = false;
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

                if(BlockSlope[b.Type] || BlockSlope2[b.Type])
                    continue;

                hit_block = true;
                break;
            }

            if(!hit_block)
            {
                p.Effect = PLREFF_NORMAL;
                PlayerCollide(A);
                PlaySoundSpatial(SFX_Transform, p.Location);
            }
        }

        p.Effect2 = found_player;
    }
    else
        p.Effect = PLREFF_NORMAL;

    if(p.Effect != PLREFF_COOP_WINGS)
    {
        SharedScreenAvoidJump_Pre(screen);

        p.Dead = false;
        p.CanJump = false;
        p.Effect2 = 0;
        p.Immune = 50;
        p.Immune2 = false;
        CheckSection(A);
        PlayerFrame(p);

        SharedScreenAvoidJump_Post(screen, 0);
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
                o_p.RespawnY = o_p.Location.Y;
            }
        }

        GetvScreenAuto(vscreen);
    }
}

void PlayerOffscreenExitCheck(int A)
{
    bool offScreenExit = false;
    double nearby_left = (Player[A].Location.X + Player[A].Location.Width) - level[Player[A].Section].X;
    double nearby_right = level[Player[A].Section].Width - Player[A].Location.X;
    if(nearby_left < 0)
    {
        offScreenExit = true;
        for(int B = 1; B <= numPlayers; B++)
            Player[B].TailCount = 0;
    }
    else if(nearby_right < 0)
    {
        offScreenExit = true;
    }
    else if(g_config.EnableInterLevelFade)
    {
        int nearby = SDL_min((int)nearby_left, (int)nearby_right);
        int fade = (32 - nearby) * 2;

        if(fade > 0 && g_levelScreenFader.m_current_fade < fade && g_levelScreenFader.m_target_fade == 0 && g_levelScreenFader.m_current_fade == g_levelScreenFader.m_step)
            g_levelScreenFader.setupFader(fade, fade, 0, ScreenFader::S_FADE);
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
        g_levelScreenFader.setupFader(65, 65, 0, ScreenFader::S_FADE);
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
