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

#include "globals.h"
#include "player.h"
#include "collision.h"
#include "effect.h"
#include "eff_id.h"
#include "sound.h"
#include "config.h"
#include "npc_traits.h"
#include "layers.h"

#include "main/trees.h"

bool p_PlayerTouchVine(Player_t& p, num_t vine_top, int VineNPC, int VineBGO)
{
    if(p.Character == 5)
    {
        bool hasNoMonts = (g_config.fix_char5_vehicle_climb && p.Mount <= 0) ||
                           !g_config.fix_char5_vehicle_climb;
        if(hasNoMonts && p.Immune == 0 && p.Controls.Up)
        {
            p.FairyCD = 0;

            if(!p.Fairy)
            {
                p.Fairy = true;
                SizeCheck(p);
                PlaySoundSpatial(SFX_HeroFairy, p.Location);
                p.Immune = 10;
                p.Effect = PLREFF_WAITING;
                p.Effect2 = 4;
                NewEffect(EFFID_SMOKE_S5, p.Location);
            }

            if(p.FairyTime != -1 && p.FairyTime < 20)
                p.FairyTime = 20;
        }

        return true;
    }
    else if(!p.Fairy && !p.Stoned && !p.AquaticSwim)
    {
        if(p.Mount == 0 && p.HoldingNPC <= 0)
        {
            if(p.Vine > 0)
            {
                if(p.Duck)
                    UnDuck(p);

                if(p.Location.Y >= vine_top - 20 && p.Vine < 2)
                    p.Vine = 2;

                if(p.Location.Y >= vine_top - 18)
                    p.Vine = 3;
            }
            else if((p.Controls.Up ||
                     (p.Controls.Down &&
                      !num_t::fEqual_d(p.Location.SpeedY, 0) && // Not .Location.SpeedY = 0
                      p.StandingOnNPC == 0 && // Not .StandingOnNPC <> 0
                      p.Slope <= 0) // Not .Slope > 0
                    ) && p.Jump == 0)
            {
                if(p.Duck)
                    UnDuck(p);

                if(p.Location.Y >= vine_top - 20 && p.Vine < 2)
                    p.Vine = 2;

                if(p.Location.Y >= vine_top - 18)
                    p.Vine = 3;
            }

            if(p.Vine > 0)
            {
                p.VineNPC = VineNPC;
                if(g_config.fix_climb_bgo_speed_adding)
                    p.VineBGO = VineBGO;
            }

            if(p.Vine == 3)
                return true;
        }
    }

    return false;
}

void PlayerVineLogic(int A)
{
    if(Player[A].Vine > 0)
        Player[A].Vine -= 1;

    // check vine backgrounds
    for(int B : treeBackgroundQuery(Player[A].Location, SORTMODE_NONE))
    {
        if(B > numBackground)
            continue;

        if(BackgroundFence[Background[B].Type] && (!g_config.fix_climb_invisible_fences || !Background[B].Hidden))
        {
            // if(CheckCollision(Player[A].Location, Background[B].Location))
            //{
            SpeedlessLocation_t tempLocation = Background[B].Location;
            tempLocation.Height -= 16;
            tempLocation.Width -= 20;
            tempLocation.X += 10;

            if(CheckCollision(Player[A].Location, tempLocation))
            {
                if(p_PlayerTouchVine(Player[A], Background[B].Location.Y, -1, B))
                    break;
            } // Collide player and temp location
            // }// Collide player and BGO
        } // Is BGO climbable and visible?
    } // Next A
}

bool PlayerFairyOnVine(int A)
{
    Location_t tempLocation = Player[A].Location;
    tempLocation.Width += 32;
    tempLocation.Height += 32;
    tempLocation.X -= 16;
    tempLocation.Y -= 16;

    for(int Bi : treeNPCQuery(tempLocation, SORTMODE_NONE))
    {
        if(NPC[Bi].Active && !NPC[Bi].Hidden && NPC[Bi]->IsAVine)
        {
            if(CheckCollision(tempLocation, NPC[Bi].Location))
                return true;
        }
    }

    for(int B : treeBackgroundQuery(tempLocation, SORTMODE_NONE))
    {
        if(B > numBackground)
            continue;

        if(BackgroundFence[Background[B].Type] && !Background[B].Hidden)
        {
            if(CheckCollision(tempLocation, Background[B].Location))
                return true;
        }
    }

    return false;
}

void PlayerVineMovement(int A)
{
    if(Player[A].StandingOnNPC > 0 && !Player[A].Controls.Up)
        Player[A].Vine = 0;
    Player[A].CanFly = false;
    Player[A].CanFly2 = false;
    Player[A].RunCount = 0;
    Player[A].SpinJump = false;

    if(Player[A].Controls.Left)
        Player[A].Location.SpeedX = -1.5_n;
    else if(Player[A].Controls.Right)
        Player[A].Location.SpeedX = 1.5_n;
    else
        Player[A].Location.SpeedX = 0;

    if(Player[A].Controls.Up && Player[A].Vine > 2)
        Player[A].Location.SpeedY = -2;
    else if(Player[A].Controls.Down)
        Player[A].Location.SpeedY = 3;
    else
        Player[A].Location.SpeedY = 0;

    if(g_config.fix_climb_bgo_speed_adding && Player[A].VineBGO > 0)
    {
        const Layer_t& layer = Layer[Background[Player[A].VineBGO].Layer];
        if(g_config.enable_climb_bgo_layer_move)
        {
            Player[A].Location.SpeedX += (num_t)layer.ApplySpeedX;
            Player[A].Location.SpeedY += (num_t)layer.ApplySpeedY;
        }
    }
    else
    {
        Player[A].Location.SpeedX += NPC[Player[A].VineNPC].Location.SpeedX;
        Player[A].Location.SpeedY += NPC[Player[A].VineNPC].Location.SpeedY;
    }
}
