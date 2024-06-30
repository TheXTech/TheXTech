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
#include "config.h"
#include "sound.h"
#include "effect.h"
#include "eff_id.h"
#include "collision.h"

#include "player/player_update_priv.h"
#include "main/trees.h"

void PlayerChar5Logic(int A)
{
    if(Player[A].State == 4 || Player[A].State == 5)
    {
        bool hasNoMonts = (g_config.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                           !g_config.fix_char5_vehicle_climb;

        bool turnFairy = Player[A].FlyCount > 0 ||
                        ((Player[A].Controls.AltJump || (Player[A].Controls.Jump && Player[A].FloatRelease)) &&
                          Player[A].Location.SpeedY != Physics.PlayerGravity && Player[A].Slope == 0 &&
                          Player[A].StandingOnNPC == 0);

        if(turnFairy && hasNoMonts)
        {
            if(Player[A].FlyCount > 0)
                Player[A].FairyCD = 0;

            if(!Player[A].Fairy && Player[A].FairyCD == 0 && Player[A].Jump == 0 && Player[A].Wet == 0)
            {
                Player[A].Jump = 0;
                if(Player[A].FlyCount == 0)
                    Player[A].FlyCount = 50;
                Player[A].FairyTime = Player[A].FlyCount;
                Player[A].FairyCD = 1;
                Player[A].FlyCount = 0;
                Player[A].Fairy = true;
                SizeCheck(Player[A]);
                PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                Player[A].Immune = 10;
                Player[A].Effect = PLREFF_WAITING;
                Player[A].Effect2 = 4;
                NewEffect(EFFID_SMOKE_S5, Player[A].Location);
            }
        }

        if(Player[A].Controls.Run && Player[A].RunRelease && (Player[A].FairyTime > 0 || Player[A].Effect == PLREFF_WAITING))
        {
            Player[A].FairyTime = 0;
            Player[A].Controls.Run = false;
        }

        if(Player[A].Fairy)
        {
            if(Player[A].Slope > 0 || Player[A].StandingOnNPC > 0)
            {
                Player[A].FairyTime = 0;
                Player[A].FairyCD = 0;
            }
        }
        // Coins += -1
        // If Coins < 0 Then
        // Lives += -1
        // Coins += 99
        // If Lives < 0 Then
        // Lives = 0
        // Coins = 0
        // .FairyTime = 0
        // End If
        // End If
        // End If
    }

    if(Player[A].HasKey)
    {
        for(int B : treeBackgroundQuery(Player[A].Location, SORTMODE_NONE))
        {
            if(B > numBackground)
                continue;

            if(Background[B].Type == 35)
            {
                Location_t tempLocation = Background[B].Location;
                tempLocation.Width = 16;
                tempLocation.X += 8;
                tempLocation.Height = 26;
                tempLocation.Y += 2;
                if(CheckCollision(Player[A].Location, tempLocation))
                {
                    PlaySound(SFX_Key);
                    StopMusic();
                    LevelMacro = LEVELMACRO_KEYHOLE_EXIT;
                    LevelMacroWhich = B;
                    break;
                }
            }
        }
    }

    if(Player[A].SwordPoke < 0)
    {
        Player[A].SwordPoke -= 1;

        if(Player[A].SwordPoke == -7)
            Player[A].SwordPoke = 1;

        if(Player[A].SwordPoke == -40)
            Player[A].SwordPoke = 0;

        if(!(Player[A].Slippy && !Player[A].Controls.Left && !Player[A].Controls.Right))
        {
            if(Player[A].FireBallCD == 0 && Player[A].Location.SpeedX != 0)
                Player[A].SwordPoke = 0;
        }
    }
    else if(Player[A].SwordPoke > 0)
    {
        if(Player[A].SwordPoke == 1)
        {
            TailSwipe(A, true, true);
            PlaySoundSpatial(SFX_HeroStab, Player[A].Location);

            if((Player[A].State == 3 || Player[A].State == 7 || Player[A].State == 6) && Player[A].FireBallCD2 == 0)
                PlayerShootChar5Beam(A);
        }
        else
            TailSwipe(A, false, true);

        Player[A].SwordPoke += 1;

        if(Player[A].Duck)
        {
            if(Player[A].SwordPoke >= 10)
            {
                Player[A].SwordPoke = 0;
                Player[A].FireBallCD = 7;
            }
        }
        else
        {
            if(Player[A].SwordPoke >= 10)
            {
                Player[A].SwordPoke = -11;
                Player[A].FireBallCD = 0;
            }
        }
    }

    if(Player[A].FireBallCD == 0 && Player[A].Wet == 0 && !Player[A].Fairy && Player[A].Mount == 0)
    {
        // Link ducks when jumping
        // Holding Up cancels this and allows upwards stab
        if(!Player[A].Duck && Player[A].Location.SpeedY < Physics.PlayerGravity && Player[A].StandingOnNPC == 0 &&
            Player[A].Slope == 0 && !Player[A].Controls.Up && !Player[A].Stoned)
        {
            Player[A].SwordPoke = 0;
            Player[A].Duck = true;
            Player[A].Location.Y += Player[A].Location.Height;
            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
            Player[A].Location.Y += -Player[A].Location.Height;
        }
        // Link stands when falling
        else if(Player[A].Duck && Player[A].Location.SpeedY > Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0)
        {
            Player[A].SwordPoke = 0;
            UnDuck(Player[A]);
        }
    }

    if(Player[A].Mount > 0 && Player[A].Mount != 2)
    {
        PlayerHurt(A);
        Player[A].Mount = 0;
    }

    Player[A].HoldingNPC = -1;
}
