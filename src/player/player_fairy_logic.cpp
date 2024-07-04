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

#include "player/player_update_priv.h"

#include "main/trees.h"

void PlayerFairyTimerUpdate(int A)
{
    if(Player[A].FairyTime != 0 && Player[A].Fairy)
    {
        if(iRand(10) == 0)
        {
            NewEffect(EFFID_SPARKLE,
                      newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                             Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = dRand() * 0.5 - 0.25;
            Effect[numEffects].Location.SpeedY = dRand() * 0.5 - 0.25;
            Effect[numEffects].Frame = 1;
        }

        if(Player[A].FairyTime > 0)
            Player[A].FairyTime -= 1;

        if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20 && Player[A].Character == 5 && PlayerFairyOnVine(A))
        {
            Player[A].FairyTime = 20;
            Player[A].FairyCD = 0;
        }
    }
    // lose fairy power
    else if(Player[A].Fairy)
    {
        PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
        Player[A].Immune = 10;
        Player[A].Effect = PLREFF_WAITING;
        Player[A].Effect2 = 4;
        Player[A].Fairy = false;
        SizeCheck(Player[A]);
        NewEffect(EFFID_SMOKE_S5, Player[A].Location);
        PlayerPush(A, 3);
    }
    else
        Player[A].FairyTime = 0;

    if(Player[A].FairyCD != 0 && (Player[A].Location.SpeedY == 0.0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0 || Player[A].WetFrame))
        Player[A].FairyCD -= 1;
}

void PlayerFairyMovementX(int A)
{
    if(Player[A].Controls.Right)
    {
        if(Player[A].Location.SpeedX < 3)
            Player[A].Location.SpeedX += 0.15;
        if(Player[A].Location.SpeedX < 0)
            Player[A].Location.SpeedX += 0.1;
    }
    else if(Player[A].Controls.Left)
    {
        if(Player[A].Location.SpeedX > -3)
            Player[A].Location.SpeedX -= 0.15;
        if(Player[A].Location.SpeedX > 0)
            Player[A].Location.SpeedX -= 0.1;
    }
    else if(Player[A].Location.SpeedX > 0.1)
        Player[A].Location.SpeedX -= 0.1;
    else if(Player[A].Location.SpeedX < -0.1)
        Player[A].Location.SpeedX += 0.1;
    else
        Player[A].Location.SpeedX = 0;
}

void PlayerFairyMovementY(int A)
{
    Player[A].WetFrame = false;
    Player[A].Wet = 0;
    if(Player[A].FairyCD == 0)
    {
        if(Player[A].Controls.Jump || Player[A].Controls.AltJump || Player[A].Controls.Up)
        {
            Player[A].Location.SpeedY -= 0.15;
            if(Player[A].Location.SpeedY > 0)
                Player[A].Location.SpeedY -= 0.1;
        }
        else if(Player[A].Location.SpeedY < -0.1 || Player[A].Controls.Down)
        {
            if(Player[A].Location.SpeedY < 3)
                Player[A].Location.SpeedY += double(Physics.PlayerGravity * 0.05f);
            if(Player[A].Location.SpeedY < 0)
                Player[A].Location.SpeedY += double(Physics.PlayerGravity * 0.05f);
            Player[A].Location.SpeedY += double(Physics.PlayerGravity * 0.1f);
            if(Player[A].Controls.Down)
                Player[A].Location.SpeedY += 0.05;
        }
        else if(Player[A].Location.SpeedY > 0.1)
            Player[A].Location.SpeedY -= 0.15;
        else
            Player[A].Location.SpeedY = 0;
    }
    else
    {
        if(Player[A].Controls.Jump || Player[A].Controls.AltJump || Player[A].Controls.Up)
        {
            Player[A].Location.SpeedY -= 0.15;
            if(Player[A].Location.SpeedY > 0)
                Player[A].Location.SpeedY -= 0.1;
        }
        else
        {
            if(Player[A].Location.SpeedY < 3)
                Player[A].Location.SpeedY += Physics.PlayerGravity * 0.05;
            if(Player[A].Location.SpeedY < 0)
                Player[A].Location.SpeedY += Physics.PlayerGravity * 0.05;
            Player[A].Location.SpeedY += Physics.PlayerGravity * 0.1;
            if(Player[A].Controls.Down)
                Player[A].Location.SpeedY += 0.05;
        }
    }

    if(Player[A].Location.SpeedY > 4)
        Player[A].Location.SpeedY = 4;
    else if(Player[A].Location.SpeedY < -3)
        Player[A].Location.SpeedY = -3;
}
