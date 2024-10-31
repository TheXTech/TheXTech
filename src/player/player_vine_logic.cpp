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
#include "player.h"
#include "collision.h"
#include "effect.h"
#include "eff_id.h"
#include "sound.h"
#include "config.h"
#include "npc_traits.h"
#include "layers.h"

#include "main/trees.h"

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
                if(Player[A].Character == 5)
                {
                    bool hasNoMonts = (g_config.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                       !g_config.fix_char5_vehicle_climb;
                    if(hasNoMonts && Player[A].Immune == 0 && Player[A].Controls.Up)
                    {
                        Player[A].FairyCD = 0;

                        if(!Player[A].Fairy)
                        {
                            Player[A].Fairy = true;
                            SizeCheck(Player[A]);
                            PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                            Player[A].Immune = 10;
                            Player[A].Effect = PLREFF_WAITING;
                            Player[A].Effect2 = 4;
                            NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                        }

                        if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20)
                            Player[A].FairyTime = 20;
                    }

                    break;
                }
                else if(!Player[A].Fairy && !Player[A].Stoned)
                {
                    if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                    {
                        if(Player[A].Vine > 0)
                        {
                            if(Player[A].Duck)
                                UnDuck(Player[A]);

                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                Player[A].Vine = 2;

                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                Player[A].Vine = 3;
                        }
                        else if((Player[A].Controls.Up || (Player[A].Controls.Down &&
                                                           Player[A].Location.SpeedY != 0 &&
                                                           Player[A].StandingOnNPC == 0 &&
                                                           Player[A].Slope <= 0)) && Player[A].Jump == 0)
                        {
                            if(Player[A].Duck)
                                UnDuck(Player[A]);

                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                Player[A].Vine = 2;

                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                Player[A].Vine = 3;
                        }

                        if(Player[A].Vine > 0)
                        {
                            Player[A].VineNPC = -1;
                            if(g_config.fix_climb_bgo_speed_adding)
                                Player[A].VineBGO = B;
                        }

                        if(Player[A].Vine == 3)
                            break;
                    }
                } // !Fairy & !Stoned
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
        Player[A].Location.SpeedX = -1.5;
    else if(Player[A].Controls.Right)
        Player[A].Location.SpeedX = 1.5;
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
            Player[A].Location.SpeedX += layer.ApplySpeedX;
            Player[A].Location.SpeedY += layer.ApplySpeedY;
        }
    }
    else
    {
        Player[A].Location.SpeedX += NPC[Player[A].VineNPC].Location.SpeedX;
        Player[A].Location.SpeedY += NPC[Player[A].VineNPC].Location.SpeedY;
    }
}
