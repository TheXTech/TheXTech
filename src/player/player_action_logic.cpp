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
#include "config.h"
#include "layers.h"
#include "npc.h"
#include "npc_traits.h"

void PlayerPoundLogic(int A)
{
    if(Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0)
    {
        if(Player[A].Mount == 3 && Player[A].MountType == 6) // Purple Yoshi Pound
        {
            bool groundPoundByAltRun = !ForcedControls && g_config.pound_by_alt_run;
            bool poundKeyPressed = groundPoundByAltRun ? Player[A].Controls.AltRun : Player[A].Controls.Down;
            bool poundKeyRelease = groundPoundByAltRun ? Player[A].AltRunRelease   : Player[A].DuckRelease;

            if(poundKeyPressed && poundKeyRelease && Player[A].CanPound)
            {
                Player[A].GroundPound = true;
                Player[A].GroundPound2 = true;
                if(Player[A].Location.SpeedY < 0)
                    Player[A].Location.SpeedY = 0;
            }
        }
    }
    else
        Player[A].CanPound = false;

    if(Player[A].GroundPound)
    {
        if(!Player[A].CanPound && Player[A].Location.SpeedY < 0)
            Player[A].GroundPound = false;

        bool groundPoundByAltRun = !ForcedControls && g_config.pound_by_alt_run;
        if(groundPoundByAltRun)
            Player[A].Controls.AltRun = true;
        else
            Player[A].Controls.Down = true;

        Player[A].CanJump = false;
        Player[A].Controls.Left = false;
        Player[A].Controls.Up = false;
        Player[A].Controls.Right = false;
        Player[A].Controls.Jump = true;
        Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
        Player[A].RunRelease = false;
        Player[A].CanFly = false;
        Player[A].FlyCount = 0;
        Player[A].CanFly2 = false;
        Player[A].Location.SpeedY += 1;
        Player[A].CanPound = false;
        Player[A].Jump = 0;
    }
    else
    {
        // allow pounding again
        if(Player[A].Location.SpeedY < -5 && ((Player[A].Jump < 15 && Player[A].Jump != 0) || Player[A].CanFly))
            Player[A].CanPound = true;

        // rebound from hitting the ground
        if(Player[A].GroundPound2)
        {
            Player[A].Location.SpeedY = -4;
            Player[A].StandingOnNPC = 0;
            Player[A].GroundPound2 = false;
        }
    }
}

void PlayerShootChar5Beam(int A)
{
    Player[A].FireBallCD2 = 40;
    if(Player[A].State == 6)
        Player[A].FireBallCD2 = 25;

    if(Player[A].State == 6)
        PlaySoundSpatial(SFX_HeroSwordBeam, Player[A].Location);
    else if(Player[A].State == 7)
        PlaySoundSpatial(SFX_HeroIce, Player[A].Location);
    else
        PlaySoundSpatial(SFX_HeroFireRod, Player[A].Location);

    numNPCs++;
    NPC[numNPCs] = NPC_t();

    if(ShadowMode)
        NPC[numNPCs].Shadow = true;

    NPC[numNPCs].Type = NPCID_PLR_FIREBALL;

    if(Player[A].State == 7)
        NPC[numNPCs].Type = NPCID_PLR_ICEBALL;

    if(Player[A].State == 6)
        NPC[numNPCs].Type = NPCID_SWORDBEAM;

    NPC[numNPCs].Projectile = true;
    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
    NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 + (40 * Player[A].Direction) - 8;

    if(!Player[A].Duck)
    {
        NPC[numNPCs].Location.Y = Player[A].Location.Y + 5;
        if(Player[A].State == 6)
            NPC[numNPCs].Location.Y += 7;
    }
    else
    {
        NPC[numNPCs].Location.Y = Player[A].Location.Y + 18;
        if(Player[A].State == 6)
            NPC[numNPCs].Location.Y += 4;
    }


    NPC[numNPCs].Active = true;
    NPC[numNPCs].TimeLeft = 100;
    NPC[numNPCs].Location.SpeedY = 20;
    NPC[numNPCs].CantHurt = 100;
    NPC[numNPCs].CantHurtPlayer = A;
    NPC[numNPCs].Special = Player[A].Character;

    if(NPC[numNPCs].Type == NPCID_PLR_FIREBALL)
        NPC[numNPCs].Frame = 16;

    NPC[numNPCs].WallDeath = 5;
    NPC[numNPCs].Location.SpeedY = 0;
    NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3);

    if(Player[A].State == 6)
        NPC[numNPCs].Location.SpeedX = 9 * Player[A].Direction + (Player[A].Location.SpeedX / 3);

    if(Player[A].StandingOnNPC != 0)
        NPC[numNPCs].Location.Y += -Player[A].Location.SpeedY;

    syncLayers_NPC(numNPCs);
    CheckSectionNPC(numNPCs);
}
