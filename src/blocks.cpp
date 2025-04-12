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


#include <algorithm>

#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "frame_timer.h"
#include "blocks.h"
#include "sound.h"
#include "graphics.h"
#include "effect.h"
#include "collision.h"
#include "npc.h"
#include "npc_id.h"
#include "eff_id.h"
#include "blk_id.h"
#include "npc_traits.h"
#include "player.h"
#include "sorting.h"
#include "layers.h"
#include "config.h"
#include "editor.h"
#include "game_main.h"

#include "graphics/gfx_update.h"
#include "npc/npc_queues.h"
#include "main/trees.h"
#include "main/game_loop_interrupt.h"


void BlockHit(int A, bool HitDown, int whatPlayer)
{
    // int tempPlayer = 0;
    // bool makeShroom = false; // if true make a mushroom
    int newBlock = 0; // what the block should turn into if anything
    // int C = 0;
//    int B = 0;
    // Block_t blankBlock;
    int oldSpecial = 0; // previous .Special
    // bool tempBool = false;
    // Location_t tempLocation;

    auto &b = Block[A];

    if(BattleMode && b.RespawnDelay == 0)
        b.RespawnDelay = 1;

    // if(whatPlayer != 0)
    //     Controls::Rumble(whatPlayer, 10, .1);

    // character switch blocks
    if((b.Type >= 622 && b.Type <= 625) || b.Type == 631)
    {
        if(whatPlayer == 0)
        {
            return;
        }
        else
        {
            b.Special = 0;

            // note: this line was previously inside the below for loop
            SavedChar[Player[whatPlayer].Character] = Player[whatPlayer];

            // mark characters in current use as unavailable
            for(auto B = 1; B <= numPlayers; B++)
            {
                switch(Player[B].Character)
                {
                case 1: BlockFrame[622] = 4; break;
                case 2: BlockFrame[623] = 4; break;
                case 3: BlockFrame[624] = 4; break;
                case 4: BlockFrame[625] = 4; break;
                case 5: BlockFrame[631] = 4; break;
                default: break;
                }
            }

            if(BlockFrame[b.Type] < 4)
            {
                // UnDuck whatPlayer // (commented since SMBX64)

                int transform_to = 0;

                switch(b.Type)
                {
                case 622: transform_to = 1; break;
                case 623: transform_to = 2; break;
                case 624: transform_to = 3; break;
                case 625: transform_to = 4; break;
                case 631: transform_to = 5; break;
                default: break;
                }

                // moved SwapCharacter logic into player.cpp

                if(transform_to)
                    SwapCharacter(whatPlayer, transform_to, true);

                PlaySoundSpatial(SFX_Transform, b.Location);
            }
            else
            {
                return;
            }
        }
    }

    oldSpecial = b.Special;

    // if(b.ShakeY != 0 || b.ShakeY2 != 0 || b.ShakeY3 != 0) // if the block has just been hit, ignore
    if(b.ShakeCounter != 0) // if the block has just been hit, ignore
    {
        if(b.RapidHit > 0 && IF_INRANGE(whatPlayer, 1, maxPlayers) && Player[whatPlayer].Character == 4)
            b.RapidHit = iRand(3) + 1;
        return;
    }

    b.Invis = false;

    // check if block with contents is blocked from below, if so cancel hit from above
    if(HitDown && b.Special > 0)
    {
        // tempBool = false; // unused since SMBX64, must have been replaced with the simple modification of HitDown

        // this is symmetric to the original transformation on the *other* block's location, but much cheaper
        const auto query_loc = newLoc(b.Location.X + 4, b.Location.Y + 16, b.Location.Width - 8, b.Location.Height);

        for(int B : treeBlockQuery(query_loc, SORTMODE_NONE))
        {
            if(B != A)
            {
                const auto &bLoc = Block[B].Location;
                // if(CheckCollision(b.Location, newLoc(bLoc.X + 4, bLoc.Y - 16, bLoc.Width - 8, bLoc.Height)))
                if(CheckCollision(query_loc, bLoc))
                {
                    HitDown = false;
                    break;
                }
            }
        }
    }

    if(b.Special == 1000 + NPCID_RED_VINE_TOP_S3 || b.Special == 1000 + NPCID_GRN_VINE_TOP_S3 || b.Special == 1000 + NPCID_GRN_VINE_TOP_S4)
        HitDown = false;


    // Shake the block
    if(b.Type == 4 || b.Type == 615 || b.Type == 55 ||
       b.Type == 60 || b.Type == 90 || b.Type == 159 ||
       b.Type == 169 || b.Type == 170 || b.Type == 173 ||
       b.Type == 176 || b.Type == 179 || b.Type == 188 ||
       b.Type == 226 || b.Type == 281 || b.Type == 282 ||
       b.Type == 283 || (b.Type >= 622 && b.Type <= 625))
    {
        if(!HitDown)
            BlockShakeUp(A);
        else
            BlockShakeDown(A);
    }


    if(b.Type == 169)
    {
        PlaySoundSpatial(SFX_PSwitch, b.Location);
        BeltDirection = -BeltDirection; // for the blet direction changing block

        for(auto B = 1; B <= numBlock; B++)
        {
            if(Block[B].Type >= BLKID_CONVEYOR_L_START && Block[B].Type <= BLKID_CONVEYOR_L_END)
            {
                Block[B].Type += BLKID_CONVEYOR_R_START - BLKID_CONVEYOR_L_START;
                Block[B].Location.SpeedX = (num_t)Layer[Block[B].Layer].ApplySpeedX + 0.8_n;
            }
            else if(Block[B].Type >= BLKID_CONVEYOR_R_START && Block[B].Type <= BLKID_CONVEYOR_R_END)
            {
                Block[B].Type -= BLKID_CONVEYOR_R_START - BLKID_CONVEYOR_L_START;
                Block[B].Location.SpeedX = (num_t)Layer[Block[B].Layer].ApplySpeedX - 0.8_n;
            }
        }
    }


    // note: these four cases were previously handled using separate code
    NPCID switch_npc = NPCID_NULL;

    if(b.Type == 170)
        switch_npc = NPCID_YEL_PLATFORM;
    else if(b.Type == 173)
        switch_npc = NPCID_BLU_PLATFORM;
    else if(b.Type == 176)
        switch_npc = NPCID_GRN_PLATFORM;
    else if(b.Type == 179)
        switch_npc = NPCID_RED_PLATFORM;

    if(switch_npc != NPCID_NULL) // switch blocks
    {
        PlaySoundSpatial(SFX_PSwitch, b.Location);
        for(auto B = 1; B <= numBlock; B++)
        {
            if(Block[B].Type == b.Type + 1)
                Block[B].Type = b.Type + 2;
            else if(Block[B].Type == b.Type + 2)
                Block[B].Type = b.Type + 1;
        }

        for(auto B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Type == switch_npc)
                NPC[B].Direction = -NPC[B].Direction;
        }
    }

    // Find out what the block should turn into
    if(b.Type == 88 || b.Type == 90 || b.Type == 89 || b.Type == 171 || b.Type == 174 || b.Type == 177 || b.Type == 180) // SMW
        newBlock = 89;
    else if(b.Type == 188 || b.Type == 192 || b.Type == 193 || b.Type == 60 || b.Type == 369) // SMB1
        newBlock = 192;
    else if(b.Type == 224 || b.Type == 225 || b.Type == 226) // Large SMB3 blocks
        newBlock = 225;
    else if(b.Type == 159) // SMB3 Battle Block
        newBlock = 159;
    else // Everything else defaults to SMB3
        newBlock = 2;

    // invincibility continuation block
    if(b.Special == 110)
    {
        if(InvincibilityTime)
            b.Special = 1000 + NPCID_INVINCIBILITY_POWER;
        else
            b.Special = 1;
    }

    if(b.Special > 0 && b.Special < 100) // Block has coins
    {
        if(whatPlayer > 0 && Player[whatPlayer].Character == 4)
            b.RapidHit = iRand(3) + 1;

        if(!HitDown)
            BlockShakeUp(A);
        else
            BlockShakeDown(A);


        // chars 2 and 5 make all coins come out at once
        if(whatPlayer > 0 && (Player[whatPlayer].Character == 2 || Player[whatPlayer].Character == 5))
        {
            // check if blocked from above
            bool blocked_above = false;
            const auto query_loc = newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30);

            for(int B : treeBlockQuery(query_loc, SORTMODE_NONE))
            {
                if(B != A && !Block[B].Hidden && !BlockOnlyHitspot1[Block[B].Type] && !BlockIsSizable[Block[B].Type])
                {
                    // if(CheckCollision(Block[B].Location, newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30)))
                    if(CheckCollision(Block[B].Location, query_loc))
                    {
                        blocked_above = true;
                        break;
                    }
                }
            }

            if(!blocked_above)
            {
                for(auto B = 1; B <= b.Special; B++)
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];

                    nn = NPC_t();
                    nn.Active = true;
                    nn.TimeLeft = 100;

                    if(newBlock == 89)
                        nn.Type = NPCID_COIN_S4;
                    else if(newBlock == 192)
                        nn.Type = NPCID_COIN_S1;
                    else
                        nn.Type = NPCID_COIN_S3;

                    if(Player[whatPlayer].Character == 5)
                    {
                        nn.Type = NPCID_GEM_1;
                        if(iRand(20) < 3)
                            nn.Type = NPCID_GEM_5;
                        if(iRand(60) < 3)
                            nn.Type = NPCID_GEM_20;
                        PlaySoundSpatial(SFX_HeroRupee, b.Location);
                    }
                    else
                    {
                        PlaySoundSpatial(SFX_Coin, b.Location);
                    }

                    auto &nLoc = nn.Location;
                    nLoc.Width = nn->TWidth;
                    nLoc.Height = nn->THeight;
                    nLoc.X = b.Location.X + (b.Location.Width - nLoc.Width) / 2;
                    nLoc.Y = b.Location.Y - nLoc.Height - 0.01_n;
                    nLoc.SpeedX = dRand() * 3 - 1.5_n;
                    nLoc.SpeedY = -(dRand() * 4) - 3;
                    if(HitDown)
                    {
                        nLoc.SpeedY = -nLoc.SpeedY / 2;
                        nLoc.Y = b.Location.Y + b.Location.Height;
                    }

                    nn.Special = 1;
                    nn.Immune = 20;
                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);

                    if(B > 20 || (Player[whatPlayer].Character == 5 && B > 5))
                        break;
                }
                b.Special = 0;
            }
            else
            {
                Coins += 1;
                if(Coins >= 100)
                    Got100Coins();

                PlaySoundSpatial(SFX_Coin, b.Location);
                NewEffect(EFFID_COIN_BLOCK_S3, b.Location);
                b.Special -= 1;
            }
        }
        else if(b.RapidHit > 0) // (whatPlayer > 0 And Player(whatPlayer).Character = 3)
        {
            // check if anything is blocking from above
            bool blocked_above = false;
            const auto query_loc = newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30);

            for(int B : treeBlockQuery(query_loc, SORTMODE_NONE))
            {
                if(B != A && !Block[B].Hidden && !(BlockOnlyHitspot1[Block[B].Type] && !BlockIsSizable[Block[B].Type]))
                {
                    if(CheckCollision(Block[B].Location, query_loc))
                    {
                        blocked_above = true;
                        break;
                    }
                }
            }

            if(!blocked_above)
            {
                numNPCs++;
                auto &nn = NPC[numNPCs];
                nn = NPC_t();
                nn.Active = true;
                nn.TimeLeft = 100;

                // NOTE: This code was already broken by Redigit. It should spawn a coin depending on thematic block
#if 0 // Useless code, needs a fix. Right now its broken by overriding by constant value
                if(newBlock == 89)
                {
                    nn.Type = NPCID_COIN_S4;
                }
                else if(newBlock == 192)
                {
                    nn.Type = NPCID_COIN_S1;
                }
                else
                {
                    nn.Type = NPCID_COIN_S3;
                }
#endif
                nn.Type = NPCID_COIN_S2;

                nn.Location.Width = nn->TWidth;
                nn.Location.Height = nn->THeight;
                nn.Location.X = b.Location.X + (b.Location.Width - nn.Location.Width) / 2;
                nn.Location.Y = b.Location.Y - nn.Location.Height - 0.01_n;
                nn.Location.SpeedX = dRand() * 3 - 1.5_n;
                nn.Location.SpeedY = -(dRand() * 4) - 3;
                nn.Special = 1;
                nn.Immune = 20;
                PlaySoundSpatial(SFX_Coin, b.Location);
                syncLayers_NPC(numNPCs);
                CheckSectionNPC(numNPCs);
                b.Special -= 1;
            }
            else
            {
                Coins += 1;
                if(Coins >= 100)
                    Got100Coins();

                PlaySoundSpatial(SFX_Coin, b.Location);
                NewEffect(EFFID_COIN_BLOCK_S3, b.Location);
                b.Special -= 1;
            }
        }
        else
        {
            Coins += 1;
            if(Coins >= 100)
                Got100Coins();

            PlaySoundSpatial(SFX_Coin, b.Location);
            NewEffect(EFFID_COIN_BLOCK_S3, b.Location);
            b.Special -= 1;
        }

        if(b.Special == 0 && b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
    }
    else if(b.Special >= 100) // New spawn code
    {
        // represents behavior for ancient 101 / 104 / 201 Special values (other ancient values replaced in OpenLevel)
        bool is_ancient = false;

        NPCID C = NPCID_NULL;

        if(b.Special >= 1000)
        {
            C = NPCID(b.Special - 1000); // this finds the NPC type and puts in the variable C
        }
        else if(b.Special == 201)
        {
            C = NPCID_LIFE_S3;
            is_ancient = true;
        }
        else if(b.Special == 104)
        {
            C = NPCID_GRN_BOOT;
            is_ancient = true;
        }
        else // b.Special == 101
        {
            C = NPCID_FODDER_S3;
            is_ancient = true;
        }

        if(!HitDown)
            BlockShakeUp(A);
        else
            BlockShakeDown(A);

        b.Special = 0;

        if(b.Type != 55) // 55 is the bouncy note block
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];

            // Was always set in SMBX64. Doing this check here keeps the easy bonus pickup and prevents movement. -- ds-sloth
            if(!g_config.fix_restored_block_move || !b.getShrinkResized())
                b.Location.Width = BlockWidth[newBlock];
        }

        // was duplicated for some reason in ancient code
        if(is_ancient)
        {
            if(!HitDown)
                BlockShakeUp(A);
            else
                BlockShakeDown(A);
        }

#if 0 // Completely disable the DEAD the code that spawns the player
        if(NPCIsABonus(C) && C != 169 && C != 170) // check to see if it should spawn a dead player
        {
            tempPlayer = CheckDead();
            if(g_ClonedPlayerMode)
            {
                tempPlayer = 0;
            }
        }

        // don't spawn players from blocks anymore
        tempPlayer = 0;

        if(tempPlayer == 0) // Spawn the npc
#endif
        // Spawn the npc
        {
            numNPCs++; // create a new NPC
            auto &nn = NPC[numNPCs];
            nn.Active = true;
            nn.TimeLeft = 1000;

            if(is_ancient)
                nn.TimeLeft = 100;

            // always give char 5 or big player full power
            bool player_gets_full = (whatPlayer > 0 && (Player[whatPlayer].State > 1 || Player[whatPlayer].Character == 5));

            // replacement index if full powerup should not be received
            NPCID replacement = C;

            if(g_ClonedPlayerMode || BattleMode || player_gets_full)
            {
                // never replace if cloned player, in battle mode, or the player that hit the block should get the full powerup
            }
            else if(C == NPCID_FIRE_POWER_S3 || C == NPCID_LEAF_POWER || C == NPCID_ICE_POWER_S3 || C == NPCID_ICE_POWER_S4 || C == NPCID_CYCLONE_POWER || C == NPCID_AQUATIC_POWER)
                replacement = NPCID_POWER_S3;
            else if(C == NPCID_FIRE_POWER_S4)
                replacement = NPCID_POWER_S4;
            else if(C == NPCID_FIRE_POWER_S1)
                replacement = NPCID_POWER_S1;

            if(NPCIsYoshi(C))
            {
                nn.Type = NPCID_ITEM_POD;
                nn.Special = C;
            }
            else if(replacement != C)
            {
                // if any player is small, then replace the NPC
                bool do_replace = false;

                for(auto B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].State == 1 && Player[B].Character != 5)
                    {
                        do_replace = true;
                        break;
                    }
                }

                if(!do_replace)
                    nn.Type = C;
                else
                    nn.Type = replacement;
            }
            else
            {
                nn.Type = C;
            }

            // replace random power immediately
            if(nn.Type == NPCID_RANDOM_POWER)
                nn.Type = RandomBonus();

            CharStuff(numNPCs);

            // note: minor SMBX64 bugfix, should be nn->TWidth in case nn's type has changed
            nn.Location.Width = (g_config.fix_npc_emerge_size) ? nn->TWidth : NPCWidth(C);

            // bug from ancient 101 case
            if(is_ancient && C == NPCID_FODDER_S3)
                nn.Location.Width = NPCWidth(NPCID_POWER_S3);

            // Make block a bit smaller to allow player take a bonus easier (Redigit's idea)
            if(!is_ancient && num_t::fEqual_d(b.Location.Width, 32.0_n))
            {
                // set value directly to make sure Location.Width == 31.9 heuristic works on low-mem builds
                // b.Location.Width -= 0.1;
                b.Location.Width = 31.9_n;

                b.Location.X += 0.05_n;
                b.setShrinkResized();
            }

            nn.Location.Height = 0;
            nn.Location.X = (b.Location.X + (b.Location.Width - nn.Location.Width) / 2);
            nn.Location.SpeedX = 0;
            nn.Location.SpeedY = 0;

            // direction set to -1 if ancient boot block (104 case)
            if(is_ancient && C == NPCID_GRN_BOOT)
                nn.Direction = -1;

            if(NPCIsYoshi(C)) // if the npc is pet then set the color of the pod
            {
                if(C == NPCID_PET_BLUE)
                    nn.Frame = 1;
                else if(C == NPCID_PET_YELLOW)
                    nn.Frame = 2;
                else if(C == NPCID_PET_RED)
                    nn.Frame = 3;
                else if(C == NPCID_PET_BLACK)
                    nn.Frame = 4;
                else if(C == NPCID_PET_PURPLE)
                    nn.Frame = 5;
                else if(C == NPCID_PET_PINK)
                    nn.Frame = 6;
            }

            if(!HitDown)
            {
                nn.Location.Y = b.Location.Y; // - 0.1
                nn.Location.Height = 0;

                // old behavior was never changed for ancient 101 / 104 / 201 cases
                if(is_ancient)
                    nn.Location.Y = b.Location.Y - 0.1_n;

                if(NPCIsYoshi(C))
                {
                    nn.Effect = NPCEFF_NORMAL;
                    nn.Location.Height = 32;
                    nn.Location.Y = b.Location.Y - 32;
                }
                else if(nn.Type == NPCID_LEAF_POWER || nn.Type == NPCID_INVINCIBILITY_POWER || nn.Type == NPCID_CYCLONE_POWER)
                {
                    nn.Effect = NPCEFF_NORMAL;
                    nn.Location.Y = b.Location.Y - 32;
                    nn.Location.SpeedY = -6;
                    nn.Location.Height = NPCHeight(C);
                    // PlaySound(SFX_ItemEmerge); // Don't play mushroom sound on leaf, like in original SMB3 (Redigit's comment)
                }
                else
                {
                    nn.Effect = NPCEFF_EMERGE_UP;
                    switch(C)
                    {
                    case NPCID_GRN_VINE_TOP_S3:
                    case NPCID_RED_VINE_TOP_S3:
                    case NPCID_GRN_VINE_TOP_S4:
                        PlaySoundSpatial(SFX_SproutVine, b.Location);
                        break;
                    default:
                        PlaySoundSpatial(SFX_ItemEmerge, b.Location);
                        break;
                    }
                }
            }
            else
            {
                nn.Location.Y = b.Location.Y + 4;
                nn.Location.Height = NPCHeight(C);

                // hardcoded to 32 in ancient 101 / 104 / 201 cases
                if(is_ancient)
                    nn.Location.Height = 32;

                nn.Effect = NPCEFF_EMERGE_DOWN;
                PlaySoundSpatial(SFX_ItemEmerge, b.Location);
            }

            nn.Effect2 = 0;
            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);

            if(is_ancient)
            {
                // the logic is the next clause didn't exist in ancient cases
            }
            else if(NPCIsYoshi(nn) ||
               NPCIsBoot(nn) || nn.Type == NPCID_POWER_S3 ||
               nn.Type == NPCID_FIRE_POWER_S3 || nn.Type == NPCID_CANNONITEM ||
               nn.Type == NPCID_LIFE_S3 || nn.Type == NPCID_POISON ||
               nn.Type == NPCID_STATUE_POWER || nn.Type == NPCID_HEAVY_POWER ||
               nn.Type == NPCID_AQUATIC_POWER || nn.Type == NPCID_POLAR_POWER ||
               nn.Type == NPCID_CYCLONE_POWER || nn.Type == NPCID_SHELL_POWER ||
               nn.Type == NPCID_FIRE_POWER_S1 || nn.Type == NPCID_FIRE_POWER_S4 ||
               nn.Type == NPCID_POWER_S1 || nn.Type == NPCID_POWER_S4 ||
               nn.Type == NPCID_LIFE_S1 || nn.Type == NPCID_LIFE_S4 ||
               nn.Type == NPCID_3_LIFE || nn.Type == NPCID_FLIPPED_RAINBOW_SHELL)
            {
                nn.TimeLeft = Physics.NPCTimeOffScreen * 20;
            }
        }
#if 0 // don't spawn players from blocks anymore
        else // Spawn the player
        {
            PlaySound(SFX_ItemEmerge);
            Player[tempPlayer].State = 1;
            Player[tempPlayer].Location.Width = Physics.PlayerWidth[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.Height = Physics.PlayerHeight[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Frame = 1;
            Player[tempPlayer].Dead = false;
            Player[tempPlayer].Location.X = b.Location.X + (b.Location.Width - Player[tempPlayer].Location.Width) / 2;
            if(!HitDown)
            {
                Player[tempPlayer].Location.Y = b.Location.Y - 0.1 - Player[tempPlayer].Location.Height;
            }
            else
            {
                Player[tempPlayer].Location.Y = b.Location.Y + 0.1 + b.Location.Height;
            }
            Player[tempPlayer].Location.SpeedX = 0;
            Player[tempPlayer].Location.SpeedY = 0;
            Player[tempPlayer].Immune = 150;
        }
#endif

    }

    // There was a long passage of dead code that handled the (b.Special >= 100 && b.Special < 1000) cases here
    // No longer needed -- most of these numbers are replaced with normal NPC ID at OpenLevel side in SMBX 1.3, and the others have been added to the above code.

    if(PSwitchTime > 0 && newBlock == 89 && b.Special == 0 && oldSpecial > 0)
    {
        numNPCs++;
        auto &nn = NPC[numNPCs];
        nn = NPC_t();
        nn.Active = true;
        nn.TimeLeft = 1;
        nn.Type = NPCID_COIN_S4;
        nn.coinSwitchBlockType = 89;
        nn.Location = b.Location;
        nn.Location.Width = nn->TWidth;
        nn.Location.Height = nn->THeight;
        nn.Location.X += (b.Location.Width - nn.Location.Width) / 2;
        nn.Location.Y -= 0.01_n;
        nn.DefaultLocationX = nn.Location.X;
        nn.DefaultLocationY = nn.Location.Y;
        nn.DefaultType = nn.Type;
        syncLayers_NPC(numNPCs);
        CheckSectionNPC(numNPCs);
        b = Block_t();
    }

    if(b.Type == 90)
        BlockHitHard(A);
}

enum BlockShakeProgram : uint8_t
{
    SHAKE_UPDOWN12_BEG = 16 + 0,
    SHAKE_UPDOWN12_MID = 16 + 6,
    SHAKE_UPDOWN12_END = 16 + 12,

    SHAKE_DOWNUP12_BEG = 32 + 0,
    SHAKE_DOWNUP12_MID = 32 + 6,
    SHAKE_DOWNUP12_END = 32 + 12,

    SHAKE_UPDOWN06_BEG = 48 + 0,
    SHAKE_UPDOWN06_MID = 48 + 3,
    SHAKE_UPDOWN06_END = 48 + 6,
};

void BlockShakeUp(int A)
{
    if(Block[A].Hidden)
        return;

    // Block[A].ShakeY = -12; // Go up
    // Block[A].ShakeY2 = 12; // Come back down
    // Block[A].ShakeY3 = 0;

    Block[A].ShakeCounter = SHAKE_UPDOWN12_BEG;
    Block[A].ShakeOffset = 0;

    if(A != iBlock[iBlocks])
    {
        iBlocks++;
        iBlock[iBlocks] = A;
    }
}

void BlockShakeUpPow(int A)
{
    if(Block[A].Hidden)
        return;

    // Block[A].ShakeY = -6; // Go up
    // Block[A].ShakeY2 = 6; // Come back down
    // Block[A].ShakeY3 = 0;

    Block[A].ShakeCounter = SHAKE_UPDOWN06_BEG;
    Block[A].ShakeOffset = 0;

    if(A != iBlock[iBlocks])
    {
        iBlocks++;
        iBlock[iBlocks] = A;
    }
}

void BlockShakeDown(int A)
{
    if(Block[A].Hidden)
        return;

    // Block[A].ShakeY = 12; // Go down
    // Block[A].ShakeY2 = -12; // Come back up
    // Block[A].ShakeY3 = 0;

    Block[A].ShakeCounter = SHAKE_DOWNUP12_BEG;
    Block[A].ShakeOffset = 0;

    if(A != iBlock[iBlocks])
    {
        iBlocks++;
        iBlock[iBlocks] = A;
    }
}

void BlockHitHard(int A)
{
    if(Block[A].Hidden)
        return;

    if(Block[A].Type == 90 && !Block[A].forceSmashable)
    {
        // Block(A).Hidden = True
        // NewEffect 82, Block(A).Location, , A
        // PlaySound 3
    }
    else
    {
        Block[A].Kill = true;
        iBlocks++;
        iBlock[iBlocks] = A;
    }
}

void SafelyKillBlock(int A)
{
    auto& block = Block[A];

    if(block.Hidden)
        return;

    // emulate old immediate kill behavior if it is safe to do so
    if(g_config.playstyle == Config_t::MODE_VANILLA && block.TriggerDeath == EVENT_NONE && block.TriggerLast == EVENT_NONE)
    {
        KillBlock(A, true);
        return;
    }

    block.Kill = 9;
    iBlocks++;
    iBlock[iBlocks] = A;
}

bool KillBlock(int A, bool Splode)
{
    Block_t blankBlock;
    bool tempBool = false;

    Block_t& b = Block[A];

    switch(g_gameLoopInterrupt.site)
    {
    case GameLoopInterrupt::UpdateBlocks_KillBlock:
    case GameLoopInterrupt::UpdateBlocks_SwitchOff_KillBlock:
        if(g_gameLoopInterrupt.bool1)
            goto resume_TriggerLast;
        else
            goto resume_TriggerDeath;
    default:
        break;
    }

    if(Block[A].Hidden)
        return false;

    if(BattleMode && Block[A].RespawnDelay == 0)
        Block[A].RespawnDelay = 1;

    if(Splode)
    {
        if(Block[A].Type == 526)
            PlaySoundSpatial(SFX_SMBlockHit, b.Location);
        else if(Block[A].Type == 186)
            PlaySoundSpatial(SFX_Fireworks, b.Location);
        else
            PlaySoundSpatial(SFX_BlockSmashed, b.Location); // Block smashed

        // Create the break effect
        if(Block[A].Type == 60)
            NewEffect(EFFID_BLU_BLOCK_SMASH, Block[A].Location);
        else if(Block[A].Type == 188)
            NewEffect(EFFID_BLOCK_S1_SMASH, Block[A].Location);
        else if(Block[A].Type == 457)
            NewEffect(EFFID_GRY_BLOCK_SMASH, Block[A].Location);
        else if(Block[A].Type == 526)
            NewEffect(EFFID_SPACE_BLOCK_SMASH, Block[A].Location);
        else if(Block[A].Type == 293)
            NewEffect(EFFID_DIRT_BLOCK_SMASH, Block[A].Location);
        else
            NewEffect(EFFID_BLOCK_SMASH, Block[A].Location);
    }

    if(LevelEditor)
    {
        if(numBlock > 0)
        {
            Block[A] = Block[numBlock];
            Block[numBlock] = blankBlock;
            numBlock--;
            syncLayersTrees_Block(A);
            syncLayersTrees_Block(numBlock + 1);
        }

        return false;
    }

    Score += 50;

    if(Block[A].TriggerDeath != EVENT_NONE)
    {
        eventindex_t resume_index;
        resume_index = ProcEvent_Safe(false, Block[A].TriggerDeath, 0);
        while(resume_index != EVENT_NONE)
        {
            g_gameLoopInterrupt.C = resume_index;
            g_gameLoopInterrupt.bool1 = false; // marks as TriggerDeath
            return true;

resume_TriggerDeath:
            resume_index = g_gameLoopInterrupt.C;
            g_gameLoopInterrupt.site = GameLoopInterrupt::None;

            resume_index = ProcEvent_Safe(true, resume_index, 0);
        }
    }

    if(Block[A].TriggerLast != EVENT_NONE)
    {
        tempBool = false;

        if(Block[A].Layer != LAYER_NONE)
        {
            int C = Block[A].Layer;

            for(int npc : Layer[C].NPCs)
            {
                if(!NPC[npc].Generator)
                {
                    tempBool = true;
                    break;
                }
            }

            if(!tempBool)
            {
                for(int other_block : Layer[C].blocks)
                {
                    if(other_block != A)
                    {
                        tempBool = true;
                        break;
                    }
                }
            }
        }

        if(!tempBool)
        {
            eventindex_t resume_index;
            resume_index = ProcEvent_Safe(false, Block[A].TriggerLast, 0);
            while(resume_index != EVENT_NONE)
            {
                g_gameLoopInterrupt.C = resume_index;
                g_gameLoopInterrupt.bool1 = true; // marks as TriggerLast
                return true;

resume_TriggerLast:
                resume_index = g_gameLoopInterrupt.C;
                g_gameLoopInterrupt.site = GameLoopInterrupt::None;

                resume_index = ProcEvent_Safe(true, resume_index, 0);
            }
        }
    }

    Block[A].Hidden = true;
    Block[A].Layer = LAYER_DESTROYED_BLOCKS;
    Block[A].Kill = false;
    syncLayersTrees_Block(A);

    return false;
}

void BlockFrames()
{
    int A = 0;

    bool pChar[maxPlayers] = {false};
    bool tempBool = false;
    SDL_memset(pChar, 0, sizeof(pChar));

    if(FreezeNPCs)
        return;

    // Update block frame counter
    BlockFrame2[4] += 1;
    if(BlockFrame2[4] == 8)
        BlockFrame2[4] = 0;

    BlockFrame2[5] += 1;
    if(BlockFrame2[5] == 8)
        BlockFrame2[5] = 0;

    BlockFrame2[30] += 1;
    if(BlockFrame2[30] == 8)
        BlockFrame2[30] = 0;

    BlockFrame2[55] += 1;
    if(BlockFrame2[55] == 8)
        BlockFrame2[55] = 0;

    BlockFrame2[88] += 1;
    if(BlockFrame2[88] == 8)
        BlockFrame2[88] = 0;

    BlockFrame2[109] += 1;
    if(BlockFrame2[109] == 4)
        BlockFrame2[109] = 0;

    BlockFrame2[371] += 1;
    if(BlockFrame2[371] == 8)
        BlockFrame2[371] = 0;

    BlockFrame2[379] += 1;
    if(BlockFrame2[379] >= 12)
        BlockFrame2[379] = 0;

    // Check if the block type is ready for the next frame
    if(BlockFrame2[4] == 0)
    {
        BlockFrame[4] += 1;
        if(BlockFrame[4] == 4)
            BlockFrame[4] = 0;
    }

    if(BlockFrame2[5] == 0)
    {
        BlockFrame[5] += 1;
        if(BlockFrame[5] == 4)
            BlockFrame[5] = 0;
    }

    BlockFrame[598] = BlockFrame[5];
    BlockFrame[511] = BlockFrame[5];
    BlockFrame[169] = BlockFrame[5];
    BlockFrame[173] = BlockFrame[5];
    BlockFrame[176] = BlockFrame[5];
    BlockFrame[179] = BlockFrame[5];
    BlockFrame[193] = BlockFrame[5];
    BlockFrame[389] = BlockFrame[5];
    BlockFrame[391] = BlockFrame[5];
    BlockFrame[392] = BlockFrame[5];
    BlockFrame[404] = BlockFrame[5];
    BlockFrame[459] = BlockFrame[5];
    BlockFrame[460] = BlockFrame[5];
    BlockFrame[461] = BlockFrame[5];
    BlockFrame[462] = BlockFrame[5];
    BlockFrame[463] = BlockFrame[5];
    BlockFrame[464] = BlockFrame[5];
    BlockFrame[465] = BlockFrame[5];
    BlockFrame[466] = BlockFrame[5];
    BlockFrame[468] = BlockFrame[5];
    BlockFrame[469] = BlockFrame[5];
    BlockFrame[470] = BlockFrame[5];
    BlockFrame[471] = BlockFrame[5];
    BlockFrame[472] = BlockFrame[5];
    BlockFrame[473] = BlockFrame[5];
    BlockFrame[474] = BlockFrame[5];
    BlockFrame[475] = BlockFrame[5];
    BlockFrame[476] = BlockFrame[5];
    BlockFrame[477] = BlockFrame[5];
    BlockFrame[478] = BlockFrame[5];
    BlockFrame[479] = BlockFrame[5];
    BlockFrame[480] = BlockFrame[5];
    BlockFrame[481] = BlockFrame[5];
    BlockFrame[482] = BlockFrame[5];
    BlockFrame[483] = BlockFrame[5];
    BlockFrame[484] = BlockFrame[5];
    BlockFrame[485] = BlockFrame[5];
    BlockFrame[486] = BlockFrame[5];
    BlockFrame[487] = BlockFrame[5];
    BlockFrame[622] = BlockFrame[5];
    BlockFrame[623] = BlockFrame[5];
    BlockFrame[624] = BlockFrame[5];
    BlockFrame[625] = BlockFrame[5];
    BlockFrame[631] = BlockFrame[5];

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Character == 1)
            BlockFrame[622] = 4;

        if(Player[A].Character == 2)
            BlockFrame[623] = 4;

        if(Player[A].Character == 3)
            BlockFrame[624] = 4;

        if(Player[A].Character == 4)
            BlockFrame[625] = 4;

        if(Player[A].Character == 5)
            BlockFrame[631] = 4;
    }

    BlockFrame2[626] += 1;
    if(BlockFrame2[626] < 8)
        BlockFrame[626] = 3;
    else if(BlockFrame2[626] < 16)
        BlockFrame[626] = 2;
    else if(BlockFrame2[626] < 23)
        BlockFrame[626] = 1;
    else
    {
        BlockFrame2[626] = 0;
        BlockFrame[626] = 1;
    }

    BlockFrame[627] = BlockFrame[626];
    BlockFrame[628] = BlockFrame[626];
    BlockFrame[629] = BlockFrame[626];
    BlockFrame[632] = BlockFrame[626];

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Character <= 5)
            pChar[Player[A].Character] = true;
    }

    if(!pChar[1])
        BlockFrame[626] = 0;

    if(!pChar[2])
        BlockFrame[627] = 0;

    if(!pChar[3])
        BlockFrame[628] = 0;

    if(!pChar[4])
        BlockFrame[629] = 0;

    if(!pChar[5])
        BlockFrame[632] = 0;

    if(BlockFrame2[30] == 0)
    {
        BlockFrame[30] += 1;
        if(BlockFrame[30] == 4)
            BlockFrame[30] = 0;
    }

    if(BlockFrame2[55] == 0)
    {
        BlockFrame[55] += 1;
        if(BlockFrame[55] == 4)
            BlockFrame[55] = 0;
    }

    if(BlockFrame2[88] == 0)
    {
        BlockFrame[88] += 1;
        if(BlockFrame[88] == 4)
            BlockFrame[88] = 0;
    }

    BlockFrame[170] = BlockFrame[88];

    if(BlockFrame2[109] == 0)
    {
        BlockFrame[109] += 1;
        if(BlockFrame[109] == 8)
            BlockFrame[109] = 0;
    }

    if(BlockFrame2[371] == 0)
    {
        BlockFrame[371] += 1;
        if(BlockFrame[371] == 8)
            BlockFrame[371] = 0;
    }

    if(BlockFrame2[379] == 0)
    {
        BlockFrame[379] += 1;
        if(BlockFrame[379] == 4)
            BlockFrame[379] = 0;
    }

    BlockFrame[380] = BlockFrame[379];
    BlockFrame[381] = BlockFrame[379];
    BlockFrame[382] = BlockFrame[379];

    BlockFrame2[530] += 1;

    if(BlockFrame2[530] <= 8)
        BlockFrame[530] = 0;
    else if(BlockFrame2[530] <= 16)
        BlockFrame[530] = 1;
    else if(BlockFrame2[530] <= 24)
        BlockFrame[530] = 2;
    else if(BlockFrame2[530] <= 32)
        BlockFrame[530] = 3;
    else if(BlockFrame2[530] <= 40)
        BlockFrame[530] = 2;
    else if(BlockFrame2[530] <= 48)
        BlockFrame[530] = 1;
    else
    {
        BlockFrame[530] = 0;
        BlockFrame2[530] = 0;
    }

    if(LevelEditor && !TestLevel)
        BlockFrame[458] = 5;
    else
    {
        tempBool = false;
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Stoned)
            {
                tempBool = true;
                break;
            }
        }

        if(BlockFrame[458] < 5 && tempBool)
        {
            BlockFrame2[458] += 1;
            if(BlockFrame2[458] >= 4)
            {
                BlockFrame2[458] = 0;
                BlockFrame[458] += 1;
            }
        }
        else if(BlockFrame[458] > 0 && !tempBool)
        {
            BlockFrame2[458] += 1;
            if(BlockFrame2[458] >= 4)
            {
                BlockFrame2[458] = 0;
                BlockFrame[458] -= 1;
            }
        }
        else
            BlockFrame2[458] = 0;
    }
}

bool PSwitch(bool enabled);

bool UpdateBlocks()
{
    int B = 0;
    if(FreezeNPCs)
        return false;

    switch(g_gameLoopInterrupt.site)
    {
    case GameLoopInterrupt::UpdateBlocks_KillBlock:
    case GameLoopInterrupt::UpdateBlocks_TriggerHit:
        goto resume_iBlocks;
    case GameLoopInterrupt::UpdateBlocks_SwitchOn:
        goto resume_SwitchOn;
    case GameLoopInterrupt::UpdateBlocks_SwitchOff:
    case GameLoopInterrupt::UpdateBlocks_SwitchOff_KillBlock:
        goto resume_SwitchOff;
    default:
        break;
    }

    BlockFrames();

    if(BattleMode)
    {
        for(int A = 1; A <= numBlock; A++)
        {
            auto &b = Block[A];
            // respawn
            if(b.RespawnDelay > 0)
            {
                b.RespawnDelay += 1;
                if(b.RespawnDelay >= 65 * 60)
                {
                    if(b.DefaultType > 0 || b.DefaultSpecial > 0 || b.Layer == LAYER_DESTROYED_BLOCKS)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(CheckCollision(b.Location, newLoc(Player[B].Location.X - 64, Player[B].Location.Y - 64, 128, 128)))
                            {
                                B = 0;
                                break;
                            }
                        }

                        if(B > 0)
                        {
                            if(b.Layer == LAYER_DESTROYED_BLOCKS)
                                b.Layer = LAYER_DEFAULT;

                            if(b.Hidden)
                            {
                                syncLayersTrees_Block_SetHidden(A);

                                if(!b.Hidden) // -V547 // False positive: the b.Hidden gets changed inside syncLayersTrees_Block_SetHidden() call
                                    NewEffect(EFFID_SMOKE_S3_CENTER, b.Location);
                            }
                            else
                            {
                                syncLayersTrees_Block(A);
                            }

                            if(b.Type != b.DefaultType || b.Special != b.DefaultSpecial)
                            {
                                if(b.Type != b.DefaultType)
                                    NewEffect(EFFID_SMOKE_S3_CENTER, b.Location);
                                b.Special = b.DefaultSpecial;
                                b.Type = b.DefaultType;
                            }

                            b.RespawnDelay = 0;
                        }
                        else
                            b.RespawnDelay = 65 * 30;
                    }
                    else
                        b.RespawnDelay = 0;
                }
            }
        }
    }

    int A;
    for(A = 1; A <= iBlocks; A++)
    {
        bool is_resume;
        if(false)
        {
resume_iBlocks:
            A = g_gameLoopInterrupt.A;
            is_resume = true;
        }
        else
            is_resume = false;

        auto &ib = Block[iBlock[A]];

        if(is_resume)
        {
            switch(g_gameLoopInterrupt.site)
            {
            case GameLoopInterrupt::UpdateBlocks_KillBlock:
                goto resume_KillBlock;
            case GameLoopInterrupt::UpdateBlocks_TriggerHit:
                goto resume_TriggerHit;
            default:
                break;
            }
        }

        // Update the shake effect
        if(ib.Hidden)
        {
            // ib.ShakeY = 0;
            // ib.ShakeY2 = 0;
            // ib.ShakeY3 = 0;

            ib.ShakeCounter = 0;
            ib.ShakeOffset = 0;
        }

        // check modBlocks.bas for the old shake logic
#if 0
        if(ib.ShakeY < 0) // Block Shake Up
        else if(ib.ShakeY > 0) // Block Shake Down
        else if(ib.ShakeY2 > 0) // Come back down
        else if(ib.ShakeY2 < 0) // Go back up
#endif

        if(ib.ShakeCounter != 0)
        {
            if(    (ib.ShakeCounter >= SHAKE_UPDOWN06_BEG && ib.ShakeCounter < SHAKE_UPDOWN06_MID)
                || (ib.ShakeCounter >= SHAKE_UPDOWN12_BEG && ib.ShakeCounter < SHAKE_UPDOWN12_MID)
                || (ib.ShakeCounter >= SHAKE_DOWNUP12_MID && ib.ShakeCounter < SHAKE_DOWNUP12_END))
            {
                ib.ShakeOffset -= 2;
            }
            else
                ib.ShakeOffset += 2;

            ib.ShakeCounter++;

            // do hit events at the middle of the shake program
            if(    ib.ShakeCounter == SHAKE_UPDOWN06_MID
                || ib.ShakeCounter == SHAKE_UPDOWN12_MID
                || ib.ShakeCounter == SHAKE_DOWNUP12_MID)
            {
                if(ib.TriggerHit != EVENT_NONE)
                {
                    eventindex_t resume_index;
                    resume_index = ProcEvent_Safe(false, ib.TriggerHit, 0);
                    while(resume_index != EVENT_NONE)
                    {
                        g_gameLoopInterrupt.C = resume_index;
                        g_gameLoopInterrupt.A = A;
                        g_gameLoopInterrupt.site = GameLoopInterrupt::UpdateBlocks_TriggerHit;
                        return true;

resume_TriggerHit:
                        resume_index = g_gameLoopInterrupt.C;
                        g_gameLoopInterrupt.site = GameLoopInterrupt::None;

                        resume_index = ProcEvent_Safe(true, resume_index, 0);
                    }
                }

                // on/off block
                if(ib.Type == 282)
                    ib.Type = 283;
                else if(ib.Type == 283)
                    ib.Type = 282;

                // spin block
                if(ib.Type == 90 && (ib.ShakeCounter == SHAKE_DOWNUP12_MID || ib.Special == 0) && !ib.forceSmashable)
                {
                    ib.Hidden = true;
                    invalidateDrawBlocks();
                    NewEffect(EFFID_SPINBLOCK, ib.Location, 1, iBlock[A]);
                    ib.ShakeCounter = 0;
                    ib.ShakeOffset = 0;
                }
            }

            // finish the shake program
            if(    ib.ShakeCounter == SHAKE_UPDOWN06_END
                || ib.ShakeCounter == SHAKE_UPDOWN12_END
                || ib.ShakeCounter == SHAKE_DOWNUP12_END)
            {
                SDL_assert(ib.ShakeOffset == 0);

                if(ib.RapidHit > 0 && ib.Special > 0 && ib.ShakeCounter != SHAKE_DOWNUP12_END)
                {
                    ib.ShakeCounter = 0;

                    BlockHit(iBlock[A]);
                    ib.RapidHit -= 1;
                }
                else
                    ib.ShakeCounter = 0;
            }
        }

        if(ib.ShakeOffset != 0)
        {
            Location_t query_loc = ib.Location;
            query_loc.Y += ib.ShakeOffset;

            for(int B : treeNPCQuery(query_loc, SORTMODE_ID))
            {
                if(NPC[B].Active && NPC[B].Killed == 0 && NPC[B].Effect == NPCEFF_NORMAL && NPC[B].HoldingPlayer == 0 && (!NPC[B]->NoClipping || NPC[B]->IsACoin))
                {
                    if(ib.ShakeOffset <= 0 || NPC[B]->IsACoin)
                    {
                        if(ShakeCollision(NPC[B].Location, ib))
                        {
                            if(iBlock[A] != NPC[B].tempBlock && ib.tempBlockNpcIdx != B)
                            {
                                if(!BlockIsSizable[ib.Type] && !BlockOnlyHitspot1[ib.Type])
                                    NPCHit(B, 2, iBlock[A]);
                                else if(ib.Location.Y + 1 >= NPC[B].Location.Y + NPC[B].Location.Height - 1)
                                    NPCHit(B, 2, iBlock[A]);
                            }
                        }
                    }
                }
            }

            for(auto B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead)
                {
                    if(Player[B].Effect == PLREFF_NORMAL && ib.Type != 55)
                    {
                        if(ShakeCollision(Player[B].Location, ib))
                        {
                            if((!BlockIsSizable[ib.Type] && !BlockOnlyHitspot1[ib.Type]) || (ib.Location.Y + 1 >= Player[B].Location.Y + Player[B].Location.Height - 1))
                            {
                                Player[B].Location.SpeedY = Physics.PlayerJumpVelocity;
                                Player[B].StandUp = true;
                                PlaySoundSpatial(SFX_Stomp, Player[B].Location);
                            }
                        }
                    }
                }
            }
        }

        if(ib.Kill) // See if block should be broke
        {
            bool just_kill_it, is_breakable;
            {
                just_kill_it = (ib.Kill == 9);
                bool is_breakable_type = (ib.Type == 4 || ib.Type == 60 ||
                   ib.Type == 90 || ib.Type == 188 ||
                   ib.Type == 226 || ib.Type == 293 ||
                   ib.Type == 526); // Check to see if it is breakable
                bool is_empty = (ib.Special <= 0);
                is_breakable = (is_breakable_type && is_empty);
            }

            ib.Kill = false;

            if(is_breakable || just_kill_it)
            {
                if(false)
                {
resume_KillBlock:
                    (void)nullptr;
                }

                if(KillBlock(iBlock[A])) // Destroy the block
                {
                    g_gameLoopInterrupt.A = A;
                    g_gameLoopInterrupt.site = GameLoopInterrupt::UpdateBlocks_KillBlock;
                    return true;
                }
            }
        }
    }

    for(auto A = iBlocks; A >= 1; A--)
    {
        auto &ib = Block[iBlock[A]];

        // if(ib.ShakeY1 == 0 && ib.ShakeY2 == 0 && ib.ShakeY3 == 0)
        if(ib.ShakeCounter == 0)
        {
            iBlock[A] = iBlock[iBlocks];
            iBlocks -= 1;
        }
    }

    if(PSwitchTime > 0)
    {
        if(PSwitchTime == Physics.NPCPSwitch)
        {
            StopMusic();
            StartMusic(-1);

            if(PSwitchPlayer >= 1 && PSwitchPlayer <= numPlayers)
                PlaySoundSpatial(SFX_PSwitch, Player[PSwitchPlayer].Location);
            else
                PlaySound(SFX_PSwitch);

resume_SwitchOn:
            if(PSwitch(true))
                return true;
        }

        PSwitchTime--;

        if(PSwitchTime == 195)
            PlaySound(SFX_CoinSwitchTimeout);

        if(PSwitchTime <= 1)
        {
resume_SwitchOff:
            if(PSwitch(false))
                return true;

            SwitchEndResumeMusic();
        }
    }

    return false;
}

void SwitchEndResumeMusic()
{
    StopMusic();

    if(InvincibilityTime)
    {
        StartMusic(-1);
        return;
    }

    int switch_player_section = Player[PSwitchPlayer].Section;

#ifndef THEXTECH_ENABLE_SDL_NET
    // originally, just used section of the player that triggered the switch
    StartMusic(switch_player_section);
#else
    // now, prefer that section if it's onscreen, but fallback to section of first onscreen player
    for(int i = l_screen->player_count - 1; i >= 0; i--)
    {
        int pi_section = Player[l_screen->players[i]].Section;
        if(i == 0 || pi_section == switch_player_section)
        {
            StartMusic(pi_section);
            return;
        }
    }
#endif
}

bool PSwitch(bool enabled)
{
    int A = 0;
    // int B = 0;
    Block_t blankBlock;
    eventindex_t resume_index = EVENT_NONE;

    switch(g_gameLoopInterrupt.site)
    {
    case GameLoopInterrupt::UpdateBlocks_SwitchOn:
    case GameLoopInterrupt::UpdateBlocks_SwitchOff:
        goto resume_ProcEvent;
    case GameLoopInterrupt::UpdateBlocks_SwitchOff_KillBlock:
        goto resume_KillBlock;
    default:
        break;
    }

    if(enabled)
    {
        for(A = 1; A <= numNPCs; A++)
        {
            bool transform = NPC[A]->IsACoin && NPC[A].coinSwitchBlockType == 0 && !NPC[A].Hidden && NPC[A].Special == 0;

            if(NPC[A].Type == NPCID_MEDAL && g_config.fix_special_coin_switch)
                transform = false;

            if(transform)
            {
                if(numBlock < maxBlocks)
                {
                    numBlock++;
                    auto &nb = Block[numBlock];

                    if((NPC[A].Type == NPCID_GEM_1 || NPC[A].Type == NPCID_GEM_5 || NPC[A].Type == NPCID_GEM_20) && NPC[A].DefaultType != 0)
                        NPC[A].Type = NPC[A].DefaultType;

                    if(NPC[A].Type == NPCID_COIN_S4 || NPC[A].Type == NPCID_COIN_5)
                        nb.Type = 89;
                    else if(NPC[A].Type == NPCID_COIN_S1)
                        nb.Type = 188;
                    else if(NPC[A].Type == NPCID_RED_COIN)
                        nb.Type = 280;
                    else if(NPC[A].Type == NPCID_COIN_S2)
                        nb.Type = 293;
                    else
                        nb.Type = 4;

                    nb.TriggerDeath = NPC[A].TriggerDeath;
                    nb.TriggerLast = NPC[A].TriggerLast;
                    nb.Layer = NPC[A].Layer;
                    nb.Invis = false;
                    nb.Hidden = false;
                    nb.Location = NPC[A].Location;
                    nb.Location.Width = BlockWidth[nb.Type];
                    nb.Location.Height = BlockHeight[nb.Type];
                    nb.Location.X += (NPC[A].Location.Width - nb.Location.Width) / 2;
                    nb.Location.SpeedX = 0;
                    nb.Location.SpeedY = 0;
                    nb.Special = -NPC[A].Variant; // NEW: store Variant to preserve medals tracking
                    nb.Kill = false;
                    nb.coinSwitchNpcType = NPC[A].Type;

                    if(g_config.fix_switched_block_clipping)
                    {
                        nb.tempBlockNpcIdx = 0;
                        nb.tempBlockNpcType = NPCID_NULL;
                        nb.tempBlockVehiclePlr = 0;
                        nb.tempBlockVehicleYOffset = 0;
                    }

                    syncLayersTrees_Block(numBlock);
                }

                NPC[A].Killed = 9;
                NPCQueues::Killed.push_back(A);
            }
        }

        // From release v1.3.6.1 to release v1.3.7, this section had a much more complex routine
        // because I was trying to emulate sorting here without the blocks actually being sorted.
        // However, the blocks ARE sorted in compat mode since v1.3.6.1, so the code was actually
        // overcomplicated. -- ds-sloth

        // make the NPCs and kill the blocks
        for(A = numBlock; A >= 1; A--)
        {
            if(BlockPSwitch[Block[A].Type] && Block[A].Special == 0 && Block[A].coinSwitchNpcType == 0 && !Block[A].Hidden)
            {
                if(numNPCs < maxNPCs)
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];
                    nn = NPC_t();
                    nn.Active = true;
                    nn.TimeLeft = 1;

                    if(Block[A].Type == 89)
                        nn.Type = NPCID_COIN_S4;
                    else if(Block[A].Type == 188 || Block[A].Type == 60)
                        nn.Type = NPCID_COIN_S1;
                    else if(Block[A].Type == 280)
                        nn.Type = NPCID_RED_COIN;
                    else if(Block[A].Type == 293)
                        nn.Type = NPCID_COIN_S2;
                    else
                        nn.Type = NPCID_COIN_S3;

                    nn.Layer = Block[A].Layer;
                    nn.TriggerDeath = Block[A].TriggerDeath;
                    nn.TriggerLast = Block[A].TriggerLast;
                    nn.coinSwitchBlockType = Block[A].Type;
                    nn.Hidden = false;
                    nn.Location = Block[A].Location;
                    nn.Location.SpeedX = 0;
                    nn.Location.SpeedY = 0;
                    nn.Location.Width = nn->TWidth;
                    nn.Location.Height = nn->THeight;
                    nn.Location.X += (Block[A].Location.Width - nn.Location.Width) / 2;
                    nn.DefaultLocationX = nn.Location.X;
                    nn.DefaultLocationY = nn.Location.Y;
                    nn.DefaultType = nn.Type;

                    // WARNING: this is new logic from #167. Check in case of any inconsistencies after Coin Switch is activated.
                    if(nn->TFrames > 0)
                    {
                        nn.Direction = 1;
                        nn.Frame = EditorNPCFrame(nn.Type, nn.Direction);
                        nn.DefaultDirection = nn.Direction;
                    }

                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);

                    Block[A] = Block[numBlock];
                    Block[numBlock] = blankBlock;
                    numBlock--;

                    // make sure that iBlock isn't forgotten (this is done for all blocks below if the compat flag is set)
                    if(!g_config.emulate_classic_block_order)
                    {
                        iBlocks++;
                        iBlock[iBlocks] = A;
                    }

                    syncLayersTrees_Block(A);
                    syncLayersTrees_Block(numBlock + 1);
                }
                else
                    break;
            }
        }

        resume_index = ProcEvent_Safe(false, EVENT_PSWITCH_START, 0, true);
    }
    else
    {
        for(A = 1; A <= numNPCs; A++)
        {
            if(NPC[A].coinSwitchBlockType > 0)
            {
                if(numBlock < maxBlocks)
                {
                    numBlock++;
                    auto &nb = Block[numBlock];
                    nb.Layer = NPC[A].Layer;
                    nb.TriggerDeath = NPC[A].TriggerDeath;
                    nb.TriggerLast = NPC[A].TriggerLast;
                    nb.Hidden = NPC[A].Hidden;
                    nb.Invis = false;
                    nb.Type = NPC[A].coinSwitchBlockType;
                    nb.Location = NPC[A].Location;
                    nb.Location.SpeedX = 0;
                    nb.Location.SpeedY = 0;
                    nb.Location.Width = BlockWidth[nb.Type];
                    nb.Location.Height = BlockHeight[nb.Type];
                    nb.Location.X += (NPC[A].Location.Width - nb.Location.Width) / 2;
                    nb.Special = 0;
                    nb.Kill = false;

                    if(g_config.fix_switched_block_clipping)
                    {
                        nb.coinSwitchNpcType = NPCID_NULL;
                        nb.tempBlockNpcIdx = 0;
                        nb.tempBlockNpcType = NPCID_NULL;
                        nb.tempBlockVehiclePlr = 0;
                        nb.tempBlockVehicleYOffset = 0;
                    }

                    syncLayersTrees_Block(numBlock);
                }
                NPC[A].Killed = 9;
                NPCQueues::Killed.push_back(A);
            }
        }


        // See comment in the previous section. There was a much more complicated routine here for some versions. -- ds-sloth

        // restore the NPCs
        for(A = numBlock; A >= 1; A--)
        {
            if(Block[A].coinSwitchNpcType > 0 && !Block[A].Hidden)
            {
                if(numNPCs >= maxNPCs)
                    break;

                // scoping nn
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];
                    nn = NPC_t();
                    nn.Layer = Block[A].Layer;
                    nn.TriggerDeath = Block[A].TriggerDeath;
                    nn.TriggerLast = Block[A].TriggerLast;
                    nn.Active = true;
                    nn.TimeLeft = 1;
                    nn.Hidden = Block[A].Hidden;
                    nn.Type = Block[A].coinSwitchNpcType;
                    nn.Location = Block[A].Location;
                    nn.Location.SpeedX = 0;
                    nn.Location.SpeedY = 0;
                    nn.Location.Width = nn->TWidth;
                    nn.Location.Height = nn->THeight;
                    nn.Location.X += (Block[A].Location.Width - nn.Location.Width) / 2;
                    nn.DefaultLocationX = nn.Location.X;
                    nn.DefaultLocationY = nn.Location.Y;
                    nn.DefaultType = nn.Type;
                    nn.Variant = -Block[A].Special;

                    // WARNING: this is new logic from #167. Check in case of any inconsistencies after Coin Switch is activated.
                    if(nn->TFrames > 0)
                    {
                        nn.Direction = 1;
                        nn.Frame = EditorNPCFrame(nn.Type, nn.Direction);
                        nn.DefaultDirection = nn.Direction;
                    }

                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);
                    nn.Killed = 0;
                }

                if(false)
                {
resume_KillBlock:
                    A = g_gameLoopInterrupt.A;
                }

                if(KillBlock(A, false))
                {
                    g_gameLoopInterrupt.A = A;
                    g_gameLoopInterrupt.site = GameLoopInterrupt::UpdateBlocks_SwitchOff_KillBlock;
                    return true;
                }

                Block[A].Layer = LAYER_USED_P_SWITCH;
                syncLayersTrees_Block(A);
                // this is as close to a permanent death as blocks get in the game,
                // because this layer usually doesn't exist
            }
        }

        resume_index = ProcEvent_Safe(false, EVENT_PSWITCH_END, 0, true);
    }

    while(resume_index != EVENT_NONE)
    {
        g_gameLoopInterrupt.C = resume_index;
        g_gameLoopInterrupt.site = (enabled) ? GameLoopInterrupt::UpdateBlocks_SwitchOn : GameLoopInterrupt::UpdateBlocks_SwitchOff;
        return true;

resume_ProcEvent:
        resume_index = g_gameLoopInterrupt.C;
        g_gameLoopInterrupt.site = GameLoopInterrupt::None;

        resume_index = ProcEvent_Safe(true, resume_index, 0, true);
    }

    // qSortBlocksX(1, numBlock);
    // B = 1;

    // for(A = 2; A <= numBlock; A++)
    // {
    //     if(Block[A].Location.X > Block[B].Location.X)
    //     {
    //         qSortBlocksY(B, A - 1);
    //         B = A;
    //     }
    // }

    // qSortBlocksY(B, A - 1);
    // FindSBlocks();
    // FindBlocks();
    // SO expensive, can't wait to get rid of this.
    // syncLayersTrees_AllBlocks();

    if(g_config.emulate_classic_block_order)
    {
        // Doing this just to replicate some unusual, unpredictable glitches
        // that sometimes occur when blocks' relative order is changing during the level

        qSortBlocksX(1, numBlock);
        int B = 1;

        for(A = 2; A <= numBlock; A++)
        {
            if(Block[A].Location.X > Block[B].Location.X)
            {
                qSortBlocksY(B, A - 1);
                B = A;
            }
        }

        qSortBlocksY(B, A - 1);

        syncLayersTrees_AllBlocks();

        BlocksSorted = true;

        // make sure that iBlock references weren't broken if blocks got sorted
        iBlocks = numBlock;
        for(A = 1; A <= numBlock; A++)
            iBlock[A] = A;
    }

    resetFrameTimer();

    return false;
}

void PowBlock()
{
    // For NetPlay: make this only affect screens where the earthquake block NPC is near at least one vScreen

    int numScreens = 0;
    int Z = 0;

    PlaySound(SFX_Stone);

    // Shake blocks. This isn't just cosmetic, it damages NPCs, so we should check the canonical screens here.

    for(int screen_i = 0; screen_i < c_screenCount; screen_i++)
    {
        Screen_t& screen = Screens[screen_i];

        if(!screen.is_active())
            continue;

        // use modern screen iteration bounds
        if(g_config.allow_multires)
        {
            Z = screen.active_begin() + 1;
            numScreens = screen.active_end();
        }
        // buggy original code, doesn't correctly handle SingleCoop
        else
        {
            Z = 1;
            numScreens = 1;

            if(!LevelEditor)
            {
                if(screen.Type == 1 || screen.Type == 4)
                    numScreens = 2;

                if(screen.Type == 5)
                {
                    DynamicScreen(screen);
                    if(screen.vScreen(2).Visible)
                        numScreens = 2;
                }
            }
        }

        for(; Z <= numScreens; Z++)
        {
            uint8_t vscreen_Z = screen.vScreen_refs[Z - 1];
            const vScreen_t& vscreen = vScreen[vscreen_Z];
            const Location_t query_loc = newLoc(-vscreen.X, -vscreen.Y, vscreen.Width, vscreen.Height);

            for(int A : treeBlockQuery(query_loc, SORTMODE_COMPAT))
            {
                if(vScreenCollision(vscreen_Z, Block[A].Location))
                {
                    if(!Block[A].Hidden)
                        BlockShakeUpPow(A);
                }
            }

            // force-activate coins on the canonical screens
            if(g_config.allow_multires && screen.is_canonical())
            {
                for(int A : treeNPCQuery(query_loc, SORTMODE_NONE))
                {
                    if(!NPC[A].Active && NPC[A]->IsACoin)
                    {
                        NPC[A].JustActivated = vscreen_Z;

                        NPCQueues::Active.insert(A);
                        NPC[A].Active = true;

                        NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                    }
                }
            }
        }

        if(!g_config.allow_multires)
            break;
    }

    for(int A : NPCQueues::Active.no_change)
    {
        if(NPC[A].Active)
        {
            if(NPC[A]->IsACoin)
            {
                NPC[A].Special = 1;
                NPC[A].Location.SpeedX = dRand() - 0.5_n;
            }
        }
    }

    doShakeScreen(20, SHAKE_RANDOM);
}

bool BlockCheckPlayerFilter(int blockIdx, int playerIdx)
{
    auto block = Block[blockIdx].Type;
    auto player = Player[playerIdx].Character;

    switch(block)
    {
    case 626:
        return (player == 1);
    case 627:
        return (player == 2);
    case 628:
        return (player == 3);
    case 629:
        return (player == 4);
    case 632:
        return (player == 5);
    }

    return false;
}
