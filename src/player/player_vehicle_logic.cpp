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
#include "config.h"
#include "sound.h"
#include "npc_traits.h"

#include "main/trees.h"

void PlayerVehicleDismountCheck(int A)
{
    if(!Player[A].Controls.Jump)
        Player[A].CanJump = true;

    if(!Player[A].Controls.AltJump && g_config.fix_vehicle_altjump_lock)
        Player[A].CanAltJump = true;

    if(Player[A].Controls.AltJump && Player[A].CanAltJump) // Jump out of the Clown Car
    {
        if(g_config.fix_vehicle_altjump_lock)
            Player[A].CanAltJump = false;

        Player[A].CanJump = false;

        bool dismount_safe = true;

        Location_t tempLocation = Player[A].Location;
        tempLocation.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
        tempLocation.Y += -Physics.PlayerHeight[Player[A].Character][Player[A].State];
        tempLocation.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
        tempLocation.X += 64 - tempLocation.Width / 2.0;

        // fBlock = FirstBlock[(tempLocation.X / 32) - 1];
        // lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
        // blockTileGet(tempLocation, fBlock, lBlock);

        for(int B : treeFLBlockQuery(tempLocation, SORTMODE_NONE))
        {
            if(!Block[B].Invis && !BlockIsSizable[Block[B].Type] && !BlockOnlyHitspot1[Block[B].Type] &&
               !BlockNoClipping[Block[B].Type] && !Block[B].Hidden)
            {
                if(CheckCollision(tempLocation, Block[B].Location))
                {
                    dismount_safe = false;
                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                    break;
                }
            }
        }

        if(dismount_safe) for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
        {
            if(NPC[B]->IsABlock && !NPC[B]->StandsOnPlayer && NPC[B].Active && NPC[B].Type != NPCID_VEHICLE)
            {
                if(CheckCollision(tempLocation, NPC[B].Location))
                {
                    dismount_safe = false;
                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                    break;
                }
            }
        }

        if(dismount_safe)
            PlayerDismount(A);
    }
}
