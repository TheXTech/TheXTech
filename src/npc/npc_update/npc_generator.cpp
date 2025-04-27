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
#include "npc.h"
#include "effect.h"
#include "sound.h"
#include "collision.h"
#include "layers.h"
#include "editor.h"
#include "main/trees.h"
#include "npc_id.h"
#include "eff_id.h"
#include "npc_traits.h"

#include "npc/npc_queues.h"
#include "npc/npc_update/npc_update_priv.h"

// returns true if an NPC should be generated
bool NPCGeneratorLogic(int A)
{
    bool should_make_new = false;

    NPC[A].TimeLeft = 0;

    // Old timer logic
    // this did not achieve anything other than keeping the value from growing large, but was likely the reason Redigit thought floats were necessary
#if 0
    NPC[A].GeneratorTime() += 1;

    if(NPC[A].GeneratorTime() >= NPC[A].GeneratorTimeMax() * 6.5f)
        NPC[A].GeneratorTime() = NPC[A].GeneratorTimeMax() * 6.5f;
#endif

    // increase activation timer if not ready yet
    if(NPC[A].GeneratorTime() * 10 < NPC[A].GeneratorTimeMax() * 65)
        NPC[A].GeneratorTime() += 1;
    // if ready and onscreen, try to activate!
    else if(NPC[A].GeneratorActive)
    {
        bool blocked = false;

        if(numNPCs == maxNPCs - 100)
            blocked = true;

        // check if blocked by players
        if(NPC[A].Type != NPCID_ITEM_BURIED && !blocked)
        {
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].TimeToLive == 0)
                {
                    if(CheckCollision(NPC[A].Location, Player[B].Location))
                    {
                        blocked = true;
                        break;
                    }
                }
            }
        }

        // check if blocked by blocks
        if(NPC[A].Type != NPCID_ITEM_BURIED && !blocked)
        {
            for(int B : treeBlockQuery(NPC[A].Location, SORTMODE_NONE))
            {
                if(!Block[B].Hidden && !BlockIsSizable[Block[B].Type])
                {
                    if(CheckCollision(NPC[A].Location,
                                      newLoc(Block[B].Location.X + 0.1_n, Block[B].Location.Y + 0.1_n,
                                             Block[B].Location.Width - 0.2_n, Block[B].Location.Height - 0.2_n)))
                    {
                        blocked = true;
                        break;
                    }
                }
            }
        }

        // check if blocked by NPCs
        if(!blocked)
        {
            for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_NONE))
            {
                if(B != A && NPC[B].Active && NPC[B].Type != NPCID_CONVEYOR)
                {
                    if(CheckCollision(NPC[A].Location, NPC[B].Location))
                    {
                        blocked = true;
                        break;
                    }
                }
            }
        }

        // if blocked, reset GeneratorTime (measured in ticks) to GeneratorTimeMax (measured in deciseconds)
        // the effect is that we will wait for (6.5 - 1) / 6.5 = 11/13 of the normal generator time before checking again
        if(blocked)
            NPC[A].GeneratorTime() = NPC[A].GeneratorTimeMax();
        // generate a new NPC!
        else
        {
            NPC[A].GeneratorTime() = 0;
            should_make_new = true;
        }
    }

    NPC[A].GeneratorActive = false;

    return should_make_new;
}

// does NOT call the activation event (because that requires safe ProcEvent handling)
void NPCGeneratorMakeNew(int A)
{
    numNPCs++;
    NPC[numNPCs] = NPC[A];

    if(NPC[A].GeneratorEffect() == 1) // Warp NPC
    {
        // NOTE: this code previously used Effect2 to store the destination position, and now it uses SpecialX/Y
        NPC[numNPCs].Layer = NPC[A].Layer;
        NPC[numNPCs].Effect3 = NPC[A].GeneratorDirection();
        NPC[numNPCs].Effect2 = 0;
        NPC[numNPCs].Effect = NPCEFF_WARP;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].TimeLeft = 100;
        if(NPC[A].GeneratorDirection() == 1)
        {
            if(NPC[A]->HeightGFX > NPC[A].Location.Height)
            {
                NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A]->HeightGFX;
                NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y - (NPC[A]->HeightGFX - NPC[A].Location.Height);
            }
            else
            {
                NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y;
            }
        }
        else if(NPC[A].GeneratorDirection() == 3)
        {
            if(NPC[A]->HeightGFX > NPC[A].Location.Height)
            {
                NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y + NPC[A].Location.Height + (NPC[A]->HeightGFX - NPC[A].Location.Height);
            }
            else
            {
                NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y + NPC[A].Location.Height;
            }
        }
        else if(NPC[A].GeneratorDirection() == 2)
        {
            NPC[numNPCs].Location.Y -= 4;
            NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width;
            NPC[numNPCs].SpecialX = NPC[numNPCs].Location.X;
        }
        else if(NPC[A].GeneratorDirection() == 4)
        {
            NPC[numNPCs].Location.Y -= 4;
            NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[A].Location.Width;
            NPC[numNPCs].SpecialX = NPC[numNPCs].Location.X + NPC[A].Location.Width;
        }
    }
    else if(NPC[A].GeneratorEffect() == 2) // projectile
    {
        NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
        PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
        NPC[numNPCs].Projectile = true;
        if(NPC[numNPCs].Type == NPCID_BULLET) // Normal Bullet Bills
            NPC[numNPCs].Projectile = false;

        if(NPC[numNPCs].Type == NPCID_SLIDE_BLOCK)
            NPC[numNPCs].Special = 1;

        if(NPC[A].GeneratorDirection() == 1)
        {
            NPC[numNPCs].Location.SpeedY = -10;
            NPC[numNPCs].Location.SpeedX = 0;
            NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X, NPC[A].Location.Y + 16, 32, 32));
            if(NPCIsVeggie(NPC[numNPCs]))
                NPC[numNPCs].Location.SpeedX = dRand() * 2 - 1;
            // NPC(numNPCs).Location.SpeedY = -1
        }
        else if(NPC[A].GeneratorDirection() == 2)
        {
            NPC[numNPCs].Location.SpeedX = -Physics.NPCShellSpeed;
            NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X + 16, NPC[A].Location.Y, 32, 32));
        }
        else if(NPC[A].GeneratorDirection() == 3)
        {
            NPC[numNPCs].Location.SpeedY = 8;
            NPC[numNPCs].Location.SpeedX = 0;
            NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X, NPC[A].Location.Y - 16, 32, 32));
        }
        else
        {
            NPC[numNPCs].Location.SpeedX = Physics.NPCShellSpeed;
            SoundPause[SFX_BlockHit] = 1;
            NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X - 16, NPC[A].Location.Y, 32, 32));
        }
    }

    NPC[numNPCs].Direction = NPC[numNPCs].DefaultDirection;
    NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
    NPC[numNPCs].DefaultDirection = NPC[numNPCs].Direction;
    NPC[numNPCs].DefaultType = NPCID_NULL;
    NPC[numNPCs].Generator = false;
    NPC[numNPCs].Active = true;
    NPC[numNPCs].TimeLeft = 100;
    NPC[numNPCs].TriggerActivate = NPC[A].TriggerActivate;
    NPC[numNPCs].TriggerDeath = NPC[A].TriggerDeath;
    NPC[numNPCs].TriggerLast = NPC[A].TriggerLast;
    NPC[numNPCs].TriggerTalk = NPC[A].TriggerTalk;
    // new because generator variables now share memory with Special3/4/5
    NPC[numNPCs].Special3 = 0;
    NPC[numNPCs].GeneratorTime() = 0;
    NPC[numNPCs].GeneratorTimeMax() = 0;
    CheckSectionNPC(numNPCs);
    syncLayers_NPC(numNPCs);
}
