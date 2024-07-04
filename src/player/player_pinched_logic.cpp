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

void PlayerPinchedTimerUpdate(int A)
{
    if(Player[A].Pinched.Bottom1 > 0)
        Player[A].Pinched.Bottom1 -= 1;

    if(Player[A].Pinched.Left2 > 0)
        Player[A].Pinched.Left2 -= 1;

    if(Player[A].Pinched.Top3 > 0)
        Player[A].Pinched.Top3 -= 1;

    if(Player[A].Pinched.Right4 > 0)
        Player[A].Pinched.Right4 -= 1;

    if(Player[A].Pinched.Moving > 0)
    {
        Player[A].Pinched.Moving -= 1;

        if(Player[A].Pinched.Moving == 0)
        {
            Player[A].Pinched.MovingLR = false;
            Player[A].Pinched.MovingUD = false;
        }
    }

    if(Player[A].Effect == PLREFF_NORMAL && Player[A].Pinched.Strict > 0)
        Player[A].Pinched.Strict -= 1;
}

void PlayerPinchedDeathCheck(int A)
{
    const auto& pi = Player[A].Pinched;

    bool vcrush = pi.Bottom1 && pi.Top3;
    bool hcrush = pi.Left2 && pi.Right4;

    // When the player is pushed through the floor or stops ducking while being crushed, they get left+right hits but no bottom hit
    bool vcrush_plus = vcrush || (hcrush && (pi.Bottom1 || pi.Top3));

    bool old_condition = pi.Moving && (vcrush || hcrush);

    bool new_condition = (pi.MovingUD && vcrush_plus) || (pi.MovingLR && hcrush);

    bool pinch_death = (g_config.fix_player_crush_death && !pi.Strict) ? new_condition : old_condition;

    if(pinch_death && Player[A].Mount != 2)
    {
        if(Player[A].Mount != 2)
            Player[A].Mount = 0;

        Player[A].State = 1;
        Player[A].Immune = 0;
        Player[A].Immune2 = false;

        // Pinch death should occur (but might get cancelled for some reason); set a timer of 15 frames to use stricter old condition
        // Why is that needed here? The details of hitspot detection mean that if a player is pushed through the floor, they get left+right hits, but no bottom hit
        Player[A].Pinched.Strict = 15;
        PlayerHurt(A);
    }
}
