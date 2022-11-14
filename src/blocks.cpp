/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "core/sdl.h"

#include "globals.h"
#include "frame_timer.h"
#include "blocks.h"
#include "sound.h"
#include "graphics.h"
#include "effect.h"
#include "collision.h"
#include "npc.h"
#include "npc_id.h"
#include "player.h"
#include "sorting.h"
#include "layers.h"
#include "compat.h"
#include "editor.h"

#include "main/trees.h"

void BlockHit(int A, bool HitDown, int whatPlayer)
{
    int tempPlayer = 0;
    bool makeShroom = false; // if true make amushroom
    int newBlock = 0; // what the block should turn into if anything
    int C = 0;
//    int B = 0;
    Block_t blankBlock;
    bool tempBool = false;
    int oldSpecial = 0; // previous .Special
    Location_t tempLocation;

    auto &b = Block[A];

    if(BattleMode && b.RespawnDelay == 0)
        b.RespawnDelay = 1;

    // if(whatPlayer != 0)
    //     Controls::Rumble(whatPlayer, 10, .1);

    if((b.Type >= 622 && b.Type <= 625) || b.Type == 631)
    {
        if(whatPlayer == 0)
        {
            return;
        }
        else
        {
            b.Special = 0;
            auto tmpNumPlayers = numPlayers;

            for(auto B = 1; B <= tmpNumPlayers; B++)
            {
                // why is this inside the for loop, seems silly.
                SavedChar[Player[whatPlayer].Character] = Player[whatPlayer];
                if(Player[B].Character == 1)
                {
                    BlockFrame[622] = 4;
                }
                if(Player[B].Character == 2)
                {
                    BlockFrame[623] = 4;
                }
                if(Player[B].Character == 3)
                {
                    BlockFrame[624] = 4;
                }
                if(Player[B].Character == 4)
                {
                    BlockFrame[625] = 4;
                }
                if(Player[B].Character == 5)
                {
                    BlockFrame[631] = 4;
                }
            }

            if(BlockFrame[b.Type] < 4)
            {
                // UnDuck whatPlayer

                // moved SwapCharacter logic into player.cpp

                if(b.Type == 622)
                {
                    SwapCharacter(whatPlayer, 1, false, true);
                }
                if(b.Type == 623)
                {
                    SwapCharacter(whatPlayer, 2, false, true);
                }
                if(b.Type == 624)
                {
                    SwapCharacter(whatPlayer, 3, false, true);
                }
                if(b.Type == 625)
                {
                    SwapCharacter(whatPlayer, 4, false, true);
                }
                if(b.Type == 631)
                {
                    SwapCharacter(whatPlayer, 5, false, true);
                }

                PlaySound(SFX_Raccoon);
            }
            else
            {
                return;
            }
        }
    }

    oldSpecial = b.Special;

    if(b.ShakeY != 0 || b.ShakeY2 != 0 || b.ShakeY3 != 0) // if the block has just been hit, ignore
    {
        if(b.RapidHit > 0 && IF_INRANGE(whatPlayer, 1, maxPlayers) && Player[whatPlayer].Character == 4)
        {
            b.RapidHit = iRand(3) + 1;
        }
        return;
    }

    b.Invis = false;

    if(HitDown && b.Special > 0)
    {
        tempBool = false;
        auto tmpNumBlocks = numBlock;
        for(auto B = 1; B <= tmpNumBlocks; B++)
        {
            if(B != A)
            {
                const auto &bLoc = Block[B].Location;
                if(CheckCollision(b.Location, newLoc(bLoc.X + 4, bLoc.Y - 16, bLoc.Width - 8, bLoc.Height)))
                {
                    HitDown = false;
                    break;
                }
            }
        }
    }

    if(b.Special == 1225 || b.Special == 1226 || b.Special == 1227)
    {
        HitDown = false;
    }

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


    if(Block[A].Type == 169)
    {
        PlaySound(SFX_PSwitch);
        BeltDirection = -BeltDirection; // for the blet direction changing block
    }


    if(b.Type == 170) // smw switch blocks
    {
        PlaySound(SFX_PSwitch);
        for(auto B = 1; B <= numBlock; B++)
        {
            if(Block[B].Type == 171)
            {
                Block[B].Type = 172;
            }
            else if(Block[B].Type == 172)
            {
                Block[B].Type = 171;
            }
        }
        for(auto B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Type == 60)
            {
                NPC[B].Direction = -NPC[B].Direction;
            }
        }
    }

    if(b.Type == 173) // smw switch blocks
    {
        PlaySound(SFX_PSwitch);
        for(auto B = 1; B <= numBlock; B++)
        {
            if(Block[B].Type == 174)
            {
                Block[B].Type = 175;
            }
            else if(Block[B].Type == 175)
            {
                Block[B].Type = 174;
            }
        }
        for(auto B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Type == 62)
            {
                NPC[B].Direction = -NPC[B].Direction;
            }
        }
    }

    if(b.Type == 176) // smw switch blocks
    {
        PlaySound(SFX_PSwitch);
        for(auto B = 1; B <= numBlock; B++)
        {
            if(Block[B].Type == 177)
            {
                Block[B].Type = 178;
            }
            else if(Block[B].Type == 178)
            {
                Block[B].Type = 177;
            }
        }
        for(auto B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Type == 64)
            {
                NPC[B].Direction = -NPC[B].Direction;
            }
        }
    }

    if(b.Type == 179) // smw switch blocks
    {
        PlaySound(SFX_PSwitch);
        for(auto B = 1; B <= numBlock; B++)
        {
            if(Block[B].Type == 180)
            {
                Block[B].Type = 181;
            }
            else if(Block[B].Type == 181)
            {
                Block[B].Type = 180;
            }
        }
        for(auto B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Type == 66)
            {
                NPC[B].Direction = -NPC[B].Direction;
            }
        }
    }

    // Find out what the block should turn into
    if(b.Type == 88 || b.Type == 90 || b.Type == 89 || b.Type == 171 || b.Type == 174 || b.Type == 177 || b.Type == 180) // SMW
    {
        newBlock = 89;
    }
    else if(b.Type == 188 || b.Type == 192 || b.Type == 193 || b.Type == 60 || b.Type == 369) // SMB1
    {
        newBlock = 192;
    }
    else if(b.Type == 224 || b.Type == 225 || b.Type == 226) // Large SMB3 blocks
    {
        newBlock = 225;
    }
    else if(b.Type == 159) // SMB3 Battle Block
    {
        newBlock = 159;
    }
    else // Everything else defaults to SMB3
    {
        newBlock = 2;
    }





    if(b.Special > 0 && b.Special < 100) // Block has coins
    {
        if(whatPlayer > 0 && Player[whatPlayer].Character == 4)
        {
            b.RapidHit = iRand(3) + 1;
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }


        if(whatPlayer > 0 && (Player[whatPlayer].Character == 2 || Player[whatPlayer].Character == 5))
        {
            tempBool = false;

            for(auto B = 1; B <= numBlock; B++)
            {
                if(B != A && !Block[B].Hidden && !(BlockOnlyHitspot1[Block[B].Type] && !BlockIsSizable[Block[B].Type]))
                {
                    if(CheckCollision(Block[B].Location, newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30)))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                for(auto B = 1; B <= b.Special; B++)
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];

                    nn = NPC_t();
                    nn.Active = true;
                    nn.TimeLeft = 100;

                    if(newBlock == 89)
                    {
                        nn.Type = 33;
                    }
                    else if(newBlock == 192)
                    {
                        nn.Type = 88;
                    }
                    else
                    {
                        nn.Type = 10;
                    }

                    if(Player[whatPlayer].Character == 5)
                    {
                        nn.Type = 251;
                        if(iRand(20) < 3)
                            nn.Type = 252;
                        if(iRand(60) < 3)
                            nn.Type = 253;
                        PlaySound(SFX_ZeldaRupee);
                    }
                    else
                    {
                        PlaySound(SFX_Coin);
                    }

                    auto &nLoc = nn.Location;
                    nLoc.Width = NPCWidth[nn.Type];
                    nLoc.Height = NPCHeight[nn.Type];
                    nLoc.X = b.Location.X + b.Location.Width / 2.0 - nLoc.Width / 2.0;
                    nLoc.Y = b.Location.Y - nLoc.Height - 0.01;
                    nLoc.SpeedX = dRand() * 3 - 1.5;
                    nLoc.SpeedY = -(dRand() * 4) - 3;
                    if(HitDown)
                    {
                        nLoc.SpeedY = -nLoc.SpeedY * 0.5;
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
                {
                    if(Lives < 99)
                    {
                        Lives += 1;
                        PlaySound(SFX_1up);
                        Coins -= 100;
                    }
                    else
                    {
                        Coins = 99;
                    }
                }
                PlaySound(SFX_Coin);
                NewEffect(11, b.Location);
                b.Special -= 1;
            }
        }
        else if(b.RapidHit > 0) // (whatPlayer > 0 And Player(whatPlayer).Character = 3)
        {
            tempBool = false;

            for(auto B = 1; B <= numBlock; B++)
            {
                if(B != A && !Block[B].Hidden && !(BlockOnlyHitspot1[Block[B].Type] && !BlockIsSizable[Block[B].Type]))
                {
                    if(CheckCollision(Block[B].Location, newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30)))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                numNPCs++;
                auto &nn = NPC[numNPCs];
                nn = NPC_t();
                nn.Active = true;
                nn.TimeLeft = 100;

                // FIXME: This code was already broken by Redigit. It should spawn a coin depending on thematic block
#if 0 // Useless code, needs a fix. Right now its broken by overriding by constant value
                if(newBlock == 89)
                {
                    nn.Type = NPCID_COIN_SMW;
                }
                else if(newBlock == 192)
                {
                    nn.Type = NPCID_COIN_SMB;
                }
                else
                {
                    nn.Type = NPCID_COIN_SMB3;
                }
#endif
                nn.Type = NPCID_COIN_SMB2;

                nn.Location.Width = NPCWidth[nn.Type];
                nn.Location.Height = NPCHeight[nn.Type];
                nn.Location.X = b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0;
                nn.Location.Y = b.Location.Y - nn.Location.Height - 0.01;
                nn.Location.SpeedX = dRand() * 3.0 - 1.5;
                nn.Location.SpeedY = -(dRand() * 4) - 3;
                nn.Special = 1;
                nn.Immune = 20;
                PlaySound(SFX_Coin);
                syncLayers_NPC(numNPCs);
                CheckSectionNPC(numNPCs);
                b.Special -= 1;
            }
            else
            {
                Coins += 1;
                if(Coins >= 100)
                {
                    if(Lives < 99)
                    {
                        Lives += 1;
                        PlaySound(SFX_1up);
                        Coins -= 100;
                    }
                    else
                    {
                        Coins = 99;
                    }
                }
                PlaySound(SFX_Coin);
                NewEffect(11, b.Location);
                b.Special -= 1;
            }

        }
        else
        {
            Coins += 1;
            if(Coins >= 100)
            {
                if(Lives < 99)
                {
                    Lives++;
                    PlaySound(SFX_1up);
                    Coins -= 100;
                }
                else
                {
                    Coins = 99;
                }
            }
            PlaySound(SFX_Coin);
            NewEffect(11, b.Location);
            b.Special -= 1;
        }

        if(b.Special == 0 && b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }




    }
    else if(b.Special >= 1000) // New spawn code
    {
        C = b.Special - 1000; // this finds the NPC type and puts in the variable C

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        if(b.Type != 55) // 55 is the bouncy note block
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

#if 0 // Completely disable the DEAD the code that spawns the player
        if(NPCIsABonus[C] && C != 169 && C != 170) // check to see if it should spawn a dead player
        {
            tempPlayer = CheckDead();
            if(numPlayers > 2 /*&& nPlay.Online == false*/)
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

            if(NPCIsYoshi[C])
            {
                nn.Type = 96;
                nn.Special = C;
            }
            else if(numPlayers > 2)
            {
                nn.Type = C;
            }
            else if(C == 14 || C == 34 || C == 264 || C == 277)
            {
                for(auto B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].State == 1 && Player[B].Character != 5)
                    {
                        makeShroom = true;
                    }
                }

                if(!makeShroom)
                {
                    nn.Type = C;
                }
                else
                {
                    nn.Type = 9;
                }
            }
            else if(C == 183)
            {
                for(auto B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].State == 1 && Player[B].Character != 5)
                    {
                        makeShroom = true;
                    }
                }

                if(!makeShroom)
                {
                    nn.Type = C;
                }
                else
                {
                    nn.Type = 185;
                }
            }
            else if(C == 182)
            {
                for(auto B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].State == 1 && Player[B].Character != 5)
                    {
                        makeShroom = true;
                    }
                }

                if(!makeShroom)
                {
                    nn.Type = C;
                }
                else
                {
                    nn.Type = 184;
                }
            }
            else
            {
                nn.Type = C;
            }

            if(makeShroom && whatPlayer > 0 &&
               (Player[whatPlayer].State > 1 || Player[whatPlayer].Character == 5)) // set the NPC type if the conditions are met
            {
                nn.Type = C;
            }

            if(makeShroom && BattleMode) // always spawn the item in battlemode
            {
                nn.Type = C;
            }

            if(nn.Type == 287)
            {
                nn.Type = RandomBonus();
            }

            CharStuff(numNPCs);
            nn.Location.Width = NPCWidth[C];

            // Make block a bit smaller to allow player take a bonus easier (Redigit's idea)
            if(fEqual(b.Location.Width, 32) && !b.wasShrinkResized)
            {
                b.Location.Width -= 0.1;
                b.Location.X += 0.05;
                b.wasShrinkResized = true; // Don't move it!!!
            }

            nn.Location.Height = 0;
            nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
            nn.Location.SpeedX = 0;
            nn.Location.SpeedY = 0;

            if(NPCIsYoshi[C]) // if the npc is yoshi then set the color of the egg
            {
                if(C == 98)
                {
                    nn.Frame = 1;
                }
                else if(C == 99)
                {
                    nn.Frame = 2;
                }
                else if(C == 100)
                {
                    nn.Frame = 3;
                }
                else if(C == 148)
                {
                    nn.Frame = 4;
                }
                else if(C == 149)
                {
                    nn.Frame = 5;
                }
                else if(C == 150)
                {
                    nn.Frame = 6;
                }
            }

            if(!HitDown)
            {
                nn.Location.Y = b.Location.Y; // - 0.1
                nn.Location.Height = 0;
                if(NPCIsYoshi[C])
                {
                    nn.Effect = 0;
                    nn.Location.Height = 32;
                    nn.Location.Y = b.Location.Y - 32;
                }
                else if(nn.Type == NPCID_LEAF)
                {
                    nn.Effect = 0;
                    nn.Location.Y = b.Location.Y - 32;
                    nn.Location.SpeedY = -6;
                    nn.Location.Height = NPCHeight[C];
                    // PlaySound(SFX_Mushroom); // Don't play mushroom sound on leaf, like in original SMB3
                }
                else
                {
                    nn.Effect = 1;
                    switch(C)
                    {
                    case NPCID_VINEHEAD_GREEN_SMB3:
                    case NPCID_VINEHEAD_RED_SMB3:
                    case NPCID_VINEHEAD_SMW:
                        PlaySound(SFX_SproutVine);
                        break;
                    default:
                        PlaySound(SFX_Mushroom);
                        break;
                    }
                }
            }
            else
            {
                nn.Location.Y = b.Location.Y + 4;
                nn.Location.Height = NPCHeight[C];
                nn.Effect = 3;
                PlaySound(SFX_Mushroom);
            }

            nn.Effect2 = 0;
            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
            if(NPCIsYoshi[nn.Type] ||
               NPCIsBoot[nn.Type] || nn.Type == 9 ||
               nn.Type == 14 || nn.Type == 22 ||
               nn.Type == 90 || nn.Type == 153 ||
               nn.Type == 169 || nn.Type == 170 ||
               nn.Type == 182 || nn.Type == 183 ||
               nn.Type == 184 || nn.Type == 185 ||
               nn.Type == 186 || nn.Type == 187 ||
               nn.Type == 188 || nn.Type == 195)
            {
                nn.TimeLeft = Physics.NPCTimeOffScreen * 20;
            }
        }
#if 0 // don't spawn players from blocks anymore
        else // Spawn the player
        {
            PlaySound(SFX_Mushroom);
            Player[tempPlayer].State = 1;
            Player[tempPlayer].Location.Width = Physics.PlayerWidth[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.Height = Physics.PlayerHeight[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Frame = 1;
            Player[tempPlayer].Dead = false;
            Player[tempPlayer].Location.X = b.Location.X + b.Location.Width * 0.5 - Player[tempPlayer].Location.Width * 0.5;
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
#if 1 // FIXME: Possibly dead code (because all such numbers were replaced with normal NPC ID at PGE-FL side)
    else if(b.Special == 100) // Block contains a mushroom
    {
        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

        tempPlayer = CheckDead();

        if(numPlayers > 2 /*&& nPlay.Online == false*/)
        {
            tempPlayer = 0;
        }

        PlaySound(SFX_Mushroom);
        if(tempPlayer == 0)
        {
            numNPCs++;
            auto &nn = NPC[numNPCs];
            nn = NPC_t();
            nn.Active = true;
            nn.TimeLeft = 1000;
            nn.Type = 9;
            nn.Location.Width = NPCWidth[9];
            nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
            nn.Location.SpeedX = 0;
            nn.Location.SpeedY = 0;

            if(!HitDown)
            {
                nn.Location.Y = b.Location.Y - 0.1;
                nn.Location.Height = 0;
                nn.Effect = 1;
            }
            else
            {
                nn.Location.Y = b.Location.Y + 4;
                nn.Location.Height = 32;
                nn.Effect = 3;
            }

            nn.Effect2 = 0;
            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
        }
        else
        {
            auto &tp = Player[tempPlayer];
            tp.Location.Width = Physics.PlayerWidth[tp.Character][tp.State];
            tp.Location.Height = Physics.PlayerHeight[tp.Character][tp.State];
            tp.Frame = 1;
            tp.Dead = false;
            tp.Location.X = b.Location.X + b.Location.Width * 0.5 - tp.Location.Width * 0.5;

            if(!HitDown)
            {
                tp.Location.Y = b.Location.Y - 0.1 - tp.Location.Height;
            }
            else
            {
                tp.Location.Y = b.Location.Y + 0.1 + b.Location.Height;
            }

            tp.Location.SpeedX = 0;
            tp.Location.SpeedY = 0;
            tp.Immune = 150;
        }
    }
    else if(b.Special == 102) // Block contains a fire flower
    {
        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        tempPlayer = CheckDead();

        if(numPlayers > 2 /*&& nPlay.Online == false*/)
        {
            tempPlayer = 0;
        }

        PlaySound(SFX_Mushroom);

        if(tempPlayer == 0)
        {
            numNPCs++;
            auto &nn = NPC[numNPCs];
            nn = NPC_t();
            nn.Active = true;
            nn.TimeLeft = 1000;

            for(auto B = 1; B <= numPlayers; B++)
            {
                if(Player[B].State == 1)
                {
                    makeShroom = true;
                }
            }

            if(!makeShroom)
            {
                nn.Type = 14;
            }
            else
            {
                nn.Type = 9;
            }

            nn.Location.Width = NPCWidth[nn.Type];
            nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
            nn.Location.SpeedX = 0;
            nn.Location.SpeedY = 0;

            if(!HitDown)
            {
                nn.Location.Y = b.Location.Y - 0.1;
                nn.Location.Height = 0;
                nn.Effect = 1;
            }
            else
            {
                nn.Location.Y = b.Location.Y + 4;
                nn.Location.Height = 32;
                nn.Effect = 3;
            }

            nn.Effect2 = 0;
            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
        }
        else // Rez player
        {
            auto &tp = Player[tempPlayer];
            tp.Frame = 1;
            tp.Dead = false;
            tp.Location.Width = Physics.PlayerWidth[tp.Character][tp.State];
            tp.Location.Height = Physics.PlayerHeight[tp.Character][tp.State];
            tp.Location.X = b.Location.X + b.Location.Width * 0.5 - tp.Location.Width * 0.5;
            if(!HitDown)
            {
                tp.Location.Y = b.Location.Y - 0.1 - tp.Location.Height;
            }
            else
            {
                tp.Location.Y = b.Location.Y + 0.1 + b.Location.Height;
            }
            tp.Location.SpeedX = 0;
            tp.Location.SpeedY = 0;
            tp.Immune = 150;
        }
    }
    else if(b.Special == 103) // Block contains a Leaf
    {
        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[b.Type];
            b.Location.Width = BlockWidth[b.Type];
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        tempPlayer = CheckDead();

        if(numPlayers > 2/*&& nPlay.Online == false*/)
        {
            tempPlayer = 0;
        }

        // TODO: @Wohlstand why was this commented out?
        // PlaySound(SFX_Mushroom);

        if(tempPlayer == 0)
        {
            numNPCs++;
            auto &nn = NPC[numNPCs];
            nn = NPC_t();
            nn.Active = true;
            nn.TimeLeft = 1000;

            for(auto B = 1; B <= numPlayers; B++)
            {
                if(Player[B].State == 1)
                {
                    makeShroom = true;
                }
            }

            if(!makeShroom)
            {
                nn.Type = 34;
            }
            else
            {
                nn.Type = 9;
            }

            nn.Location.Width = NPCWidth[nn.Type];
            nn.Location.Height = NPCHeight[nn.Type];
            nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
            nn.Location.SpeedX = 0;

            if(!HitDown)
            {
                if(nn.Type == 34)
                {
                    nn.Location.Y = b.Location.Y - 32;
                    nn.Location.SpeedY = -6;
                    nn.Location.Height = NPCHeight[34];
                }
                else
                {
                    nn.Location.Y = b.Location.Y - 0.1;
                    nn.Location.Height = 0;
                    nn.Effect = 1;
                }
            }
            else
            {
                nn.Location.Y = b.Location.Y + 4;
                nn.Location.Height = 32;
                nn.Effect = 3;
            }

            nn.Effect2 = 0;
            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
        }
        else // Rez player
        {
            auto &tp = Player[tempPlayer];
            tp.Location.Width = Physics.PlayerWidth[tp.Character][tp.State];
            tp.Location.Height = Physics.PlayerHeight[tp.Character][tp.State];
            tp.Frame = 1;
            tp.Dead = false;
            tp.Location.X = b.Location.X + b.Location.Width * 0.5 - tp.Location.Width * 0.5;

            if(!HitDown)
            {
                tp.Location.Y = b.Location.Y - 0.1 - tp.Location.Height;
            }
            else
            {
                tp.Location.Y = b.Location.Y + 0.1 + b.Location.Height;
            }

            tp.Location.SpeedX = 0;
            tp.Location.SpeedY = 0;
            tp.Immune = 150;
        }
    }
    else if(b.Special == 104) // Block contains a Shoe
    {
        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        //if(!(b.Type == 55))
        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        PlaySound(SFX_Mushroom);
        numNPCs++;
        auto &nn = NPC[numNPCs];
        nn = NPC_t();
        nn.Active = true;
        nn.TimeLeft = 100;
        nn.Type = 35;
        nn.Direction = -1;
        nn.Location.Width = NPCWidth[35];
        nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
        nn.Location.Y = b.Location.Y - 0.1;
        nn.Location.SpeedX = 0;
        nn.Location.SpeedY = 0;

        if(!HitDown)
        {
            nn.Location.Y = b.Location.Y - 0.1;
            nn.Location.Height = 0;
            nn.Effect = 1;
        }
        else
        {
            nn.Location.Y = b.Location.Y + 4;
            nn.Location.Height = 32;
            nn.Effect = 3;
        }

        nn.Effect2 = 0;
        syncLayers_NPC(numNPCs);
        CheckSectionNPC(numNPCs);
    }
    else if(b.Special == 105) // Block contains a Green Yoshi
    {
        SoundPause[2] = 2;
        PlaySound(SFX_Mushroom);

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        //if(!(b.Type == 55))
        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        numNPCs++;
        auto &nn = NPC[numNPCs];
        nn = NPC_t();
        nn.Active = true;
        nn.TimeLeft = 100;
        nn.Type = 96;
        nn.Special = 95;
        nn.Direction = 1;
        nn.Location.Width = NPCWidth[96];
        nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
        nn.Location.Y = b.Location.Y - 0.1;
        nn.Location.SpeedX = 0;
        nn.Location.SpeedY = 0;

        if(!HitDown)
        {
            nn.Location.Height = 32;
            nn.Location.Y = b.Location.Y - 32;
            nn.Effect = 0;
        }
        else
        {
            PlaySound(SFX_Mushroom);
            nn.Location.Y = b.Location.Y + 4;
            nn.Location.Height = 32;
            nn.Effect = 3;
        }

        nn.Effect2 = 0;
        syncLayers_NPC(numNPCs);
        CheckSectionNPC(numNPCs);
    }
    else if(b.Special == 101) // Block contains a Goomba
    {
        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        //if(!(b.Type == 55))
        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        PlaySound(SFX_Mushroom);

        numNPCs++;
        auto &nn = NPC[numNPCs];
        nn = NPC_t();
        nn.Active = true;
        nn.TimeLeft = 100;
        nn.Type = 1;
        nn.Location.Width = NPCWidth[9];
        nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
        nn.Location.Y = b.Location.Y - 0.1;
        nn.Location.SpeedX = 0;
        nn.Location.SpeedY = 0;

        if(!HitDown)
        {
            nn.Location.Y = b.Location.Y - 0.1;
            nn.Location.Height = 0;
            nn.Effect = 1;
        }
        else
        {
            nn.Location.Y = b.Location.Y + 4;
            nn.Location.Height = 32;
            nn.Effect = 3;
        }

        nn.Effect2 = 0;
        syncLayers_NPC(numNPCs);
        CheckSectionNPC(numNPCs);
    }
    else if(b.Special == 201) // Block contains a 1-up
    {
        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        b.Special = 0;

        //if(!(b.Type == 55))
        if(b.Type != 55)
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }

        if(!HitDown)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }

        PlaySound(SFX_Mushroom);
        numNPCs++;
        auto &nn = NPC[numNPCs];
        nn = NPC_t();
        nn.Active = true;
        nn.TimeLeft = 100;
        nn.Type = 90;
        nn.Location.Width = NPCWidth[90];
        nn.Location.X = (b.Location.X + b.Location.Width / 2.0 - nn.Location.Width / 2.0);
        nn.Location.Y = b.Location.Y - 0.1;
        nn.Location.SpeedX = 0;
        nn.Location.SpeedY = 0;

        if(!HitDown)
        {
            nn.Location.Y = b.Location.Y - 0.1;
            nn.Location.Height = 0;
            nn.Effect = 1;
        }
        else
        {
            nn.Location.Y = b.Location.Y + 4;
            nn.Location.Height = 32;
            nn.Effect = 3;
        }

        nn.Effect2 = 0;
        syncLayers_NPC(numNPCs);
        CheckSectionNPC(numNPCs);
    }
#endif

    if(PSwitchTime > 0 && newBlock == 89 && b.Special == 0 && oldSpecial > 0)
    {
        numNPCs++;
        auto &nn = NPC[numNPCs];
        nn = NPC_t();
        nn.Active = true;
        nn.TimeLeft = 1;
        nn.Type = 33;
        nn.Block = 89;
        nn.Location = b.Location;
        nn.Location.Width = NPCWidth[nn.Type];
        nn.Location.Height = NPCHeight[nn.Type];
        nn.Location.X += (b.Location.Width - nn.Location.Width) / 2.0;
        nn.Location.Y -= 0.01;
        nn.DefaultLocation = nn.Location;
        nn.DefaultType = nn.Type;
        syncLayers_NPC(numNPCs);
        CheckSectionNPC(numNPCs);
        b = blankBlock;
    }

    if(b.Type == 90)
    {
        BlockHitHard(A);
    }
}

void BlockShakeUp(int A)
{
    if(Block[A].Hidden)
        return;

    Block[A].ShakeY = -12; // Go up
    Block[A].ShakeY2 = 12; // Come back down
    Block[A].ShakeY3 = 0;

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

    Block[A].ShakeY = -6; // Go up
    Block[A].ShakeY2 = 6; // Come back down
    Block[A].ShakeY3 = 0;
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

    Block[A].ShakeY = 12; // Go down
    Block[A].ShakeY2 = -12; // Come back up
    Block[A].ShakeY3 = 0;

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

    if(Block[A].Type == 90 && Block[A].Special2 != 1)
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

void KillBlock(int A, bool Splode)
{
    Block_t blankBlock;
    bool tempBool = false;

    if(Block[A].Hidden)
        return;

    if(BattleMode && Block[A].RespawnDelay == 0)
        Block[A].RespawnDelay = 1;

    if(Splode)
    {
        if(Block[A].Type == 526)
            PlaySound(SFX_SMBlockHit);
        else if(Block[A].Type == 186)
            PlaySound(SFX_Fireworks);
        else
            PlaySound(SFX_BlockSmashed); // Block smashed
        // Create the break effect
        if(Block[A].Type == 60)
            NewEffect(21, Block[A].Location);
        else if(Block[A].Type == 188)
            NewEffect(51, Block[A].Location);
        else if(Block[A].Type == 457)
            NewEffect(100, Block[A].Location);
        else if(Block[A].Type == 526)
            NewEffect(107, Block[A].Location);
        else if(Block[A].Type == 293)
            NewEffect(135, Block[A].Location);
        else
            NewEffect(1, Block[A].Location);
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
    }
    else
    {
        Score += 50;

        if(Block[A].TriggerDeath != EVENT_NONE)
        {
            ProcEvent(Block[A].TriggerDeath);
        }

        if(Block[A].TriggerLast != EVENT_NONE)
        {
            tempBool = false;

            int C = Block[A].Layer;
            if(C != LAYER_NONE)
            {
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
                ProcEvent(Block[A].TriggerLast);
            }
        }

        Block[A].Hidden = true;
        Block[A].Layer = LAYER_DESTROYED_BLOCKS;
        Block[A].Kill = false;
        syncLayersTrees_Block(A);
    }

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

void UpdateBlocks()
{
    int A = 0;
    int B = 0;
    if(FreezeNPCs)
        return;

    BlockFrames();

    if(BattleMode)
    {
        for(A = 1; A <= numBlock; A++)
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

                                if(!b.Hidden)
                                    NewEffect(10, newLoc(b.Location.X + b.Location.Width / 2.0 - EffectWidth[10] / 2, b.Location.Y + b.Location.Height / 2.0 - EffectHeight[10] / 2));
                            }
                            else
                            {
                                syncLayersTrees_Block(A);
                            }

                            if(b.Type != b.DefaultType || b.Special != b.DefaultSpecial)
                            {
                                if(b.Type != b.DefaultType)
                                    NewEffect(10, newLoc(b.Location.X + b.Location.Width / 2.0 - EffectWidth[10] / 2, b.Location.Y + b.Location.Height / 2.0 - EffectHeight[10] / 2));
                                b.Special = b.DefaultSpecial;
                                b.Special2 = b.DefaultSpecial2;
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

    for(auto A = 1; A <= iBlocks; A++)
    {
        auto &ib = Block[iBlock[A]];
        // Update the shake effect
        if(ib.Hidden)
        {
            ib.ShakeY = 0;
            ib.ShakeY2 = 0;
            ib.ShakeY3 = 0;
        }

        if(ib.ShakeY < 0) // Block Shake Up
        {
            ib.ShakeY += 2;
            ib.ShakeY3 -= 2;

            if(ib.ShakeY == 0)
            {
                if(ib.TriggerHit != EVENT_NONE)
                {
                    ProcEvent(ib.TriggerHit);
                }

                if(ib.Type == 282)
                    ib.Type = 283;
                else if(ib.Type == 283)
                    ib.Type = 282;
                if(ib.Type == 90 && ib.Special == 0 && ib.Special2 != 1)
                {
                    ib.Hidden = true;
                    NewEffect(82, ib.Location, 1, iBlock[A]);
                    ib.ShakeY = 0;
                    ib.ShakeY2 = 0;
                    ib.ShakeY3 = 0;
                }
            }
        }
        else if(ib.ShakeY > 0) // Block Shake Down
        {
            ib.ShakeY -= 2;
            ib.ShakeY3 += 2;

            if(ib.ShakeY == 0)
            {
                if(ib.TriggerHit != EVENT_NONE)
                    ProcEvent(ib.TriggerHit);

                if(ib.Type == 282)
                    ib.Type = 283;
                else if(ib.Type == 283)
                    ib.Type = 282;

                if(ib.Type == 90)
                {
                    ib.Hidden = true;
                    NewEffect(82, ib.Location, 1, iBlock[A]);
                    ib.ShakeY = 0;
                    ib.ShakeY2 = 0;
                    ib.ShakeY3 = 0;
                }
            }
        }
        else if(ib.ShakeY2 > 0) // Come back down
        {
            ib.ShakeY2 -= 2;
            ib.ShakeY3 += 2;

            if(ib.RapidHit > 0 && ib.Special > 0 && ib.ShakeY3 == 0)
            {
                BlockHit(iBlock[A]);
                ib.RapidHit -= 1;
            }
        }
        else if(ib.ShakeY2 < 0) // Go back up
        {
            ib.ShakeY2 += 2;
            ib.ShakeY3 -= 2;
        }

        if(ib.ShakeY3 != 0)
        {
            for(auto B = 1; B <= numNPCs; B++)
            {
                if(NPC[B].Active)
                {
                    if(NPC[B].Killed == 0 && NPC[B].Effect == 0 && NPC[B].HoldingPlayer == 0 && (!NPCNoClipping[NPC[B].Type] || NPCIsACoin[NPC[B].Type]))
                    {
                        if(ib.ShakeY3 <= 0 || NPCIsACoin[NPC[B].Type])
                        {
                            if(ShakeCollision(NPC[B].Location, ib.Location, ib.ShakeY3))
                            {
                                if(iBlock[A] != NPC[B].tempBlock)
                                {
                                    if(ib.IsReally != B)
                                    {
                                        if(!BlockIsSizable[ib.Type] && !BlockOnlyHitspot1[ib.Type])
                                        {
                                            NPCHit(B, 2, iBlock[A]);
                                        }
                                        else
                                        {
                                            if(ib.Location.Y + 1 >= NPC[B].Location.Y + NPC[B].Location.Height - 1)
                                            {
                                                NPCHit(B, 2, iBlock[A]);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            for(auto B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead)
                {
                    if(Player[B].Effect == 0 && ib.Type != 55)
                    {
                        if(ShakeCollision(Player[B].Location, ib.Location, ib.ShakeY3))
                        {
                            if(!BlockIsSizable[ib.Type] && !BlockOnlyHitspot1[ib.Type])
                            {
                                Player[B].Location.SpeedY = double(Physics.PlayerJumpVelocity);
                                Player[B].StandUp = true;
                                PlaySound(SFX_Stomp);
                            }
                            else
                            {
                                if(ib.Location.Y + 1 >= Player[B].Location.Y + Player[B].Location.Height - 1)
                                {
                                    Player[B].Location.SpeedY = double(Physics.PlayerJumpVelocity);
                                    Player[B].StandUp = true;
                                    PlaySound(SFX_Stomp);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(ib.Kill) // See if block should be broke
        {
            ib.Kill = false;

            if(ib.Special == 0)
            {
                if(ib.Type == 4 || ib.Type == 60 ||
                   ib.Type == 90 || ib.Type == 188 ||
                   ib.Type == 226 || ib.Type == 293 ||
                   ib.Type == 526) // Check to see if it is breakable
                    KillBlock(iBlock[A]); // Destroy the block
            }
        }
    }

    for(auto A = iBlocks; A >= 1; A--)
    {
        auto &ib = Block[iBlock[A]];
        if(ib.ShakeY == 0)
        {
            if(ib.ShakeY2 == 0)
            {
                if(ib.ShakeY3 == 0)
                {
                    iBlock[A] = iBlock[iBlocks];
                    iBlocks -= 1;
                }
            }
        }
    }

    if(PSwitchTime > 0)
    {
        if(PSwitchTime == Physics.NPCPSwitch)
        {
            StopMusic();
            StartMusic(-1);
            PlaySound(SFX_PSwitch);
            PSwitch(true);
        }

        PSwitchTime--;

        if(PSwitchTime == 195)
            PlaySound(SFX_PSwitchTimeout);

        if(PSwitchTime <= 1)
        {
            PSwitch(false);
            StopMusic();
            StartMusic(Player[PSwitchPlayer].Section);
        }
    }
}

void PSwitch(bool enabled)
{
    int A = 0;
    // int B = 0;
    Block_t blankBlock;

    if(enabled)
    {
        for(A = 1; A <= numNPCs; A++)
        {
            bool transform = NPCIsACoin[NPC[A].Type] && NPC[A].Block == 0 && !NPC[A].Hidden && NPC[A].Special == 0.0;

            if(NPC[A].Type == NPCID_DRAGONCOIN && g_compatibility.fix_special_coin_switch)
                transform = false;

            if(transform)
            {
                if(numBlock < maxBlocks)
                {
                    numBlock++;
                    auto &nb = Block[numBlock];

                    if((NPC[A].Type == 251 || NPC[A].Type == 252 || NPC[A].Type == 253) && NPC[A].DefaultType != 0)
                        NPC[A].Type = NPC[A].DefaultType;

                    if(NPC[A].Type == 33 || NPC[A].Type == 258)
                        nb.Type = 89;
                    else if(NPC[A].Type == 88)
                        nb.Type = 188;
                    else if(NPC[A].Type == 103)
                        nb.Type = 280;
                    else if(NPC[A].Type == 138)
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
                    nb.Location.X += (NPC[A].Location.Width - nb.Location.Width) / 2.0;
                    nb.Location.SpeedX = 0;
                    nb.Location.SpeedY = 0;
                    nb.Special = 0;
                    nb.Kill = false;
                    nb.NPC = NPC[A].Type;
                    syncLayersTrees_Block(numBlock);
                }
                NPC[A].Killed = 9;
            }
        }

        // this has been made more complicated because the Blocks are no longer sorted like they were previously

        // use one of the pre-allocated result vectors
        TreeResult_Sentinel<BlockRef_t> sent;
        std::vector<BaseRef_t>& PSwitchBlocks = *sent.i_vec;

        // fill it with the PSwitch-affected blocks
        for(A = numBlock; A >= 1; A--)
        {
            if(BlockPSwitch[Block[A].Type] && Block[A].Special == 0 && Block[A].NPC == 0 && !Block[A].Hidden)
            {
                PSwitchBlocks.push_back(A);
            }
        }

        // sort them in reverse location order
        if(CompatGetLevel() == 1 || CompatGetLevel() == 2)
        {
            std::sort(PSwitchBlocks.begin(), PSwitchBlocks.end(),
                [](BaseRef_t a, BaseRef_t b) {
                    return (((BlockRef_t)a)->Location.X > ((BlockRef_t)b)->Location.X
                        || (((BlockRef_t)a)->Location.X == ((BlockRef_t)b)->Location.X
                            && ((BlockRef_t)a)->Location.Y > ((BlockRef_t)b)->Location.Y));
                });
        }

        // make the NPCs
        size_t numConverted = 0;
        for(; numConverted < PSwitchBlocks.size(); numConverted++)
        {
            A = PSwitchBlocks[numConverted];
            {
                if(numNPCs < maxNPCs)
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];
                    nn = NPC_t();
                    nn.Active = true;
                    nn.TimeLeft = 1;

                    if(Block[A].Type == 89)
                        nn.Type = 33;
                    else if(Block[A].Type == 188 || Block[A].Type == 60)
                        nn.Type = 88;
                    else if(Block[A].Type == 280)
                        nn.Type = 103;
                    else if(Block[A].Type == 293)
                        nn.Type = 138;
                    else
                        nn.Type = 10;

                    nn.Layer = Block[A].Layer;
                    nn.TriggerDeath = Block[A].TriggerDeath;
                    nn.TriggerLast = Block[A].TriggerLast;
                    nn.Block = Block[A].Type;
                    nn.Hidden = false;
                    nn.Location = Block[A].Location;
                    nn.Location.SpeedX = 0;
                    nn.Location.SpeedY = 0;
                    nn.Location.Width = NPCWidth[nn.Type];
                    nn.Location.Height = NPCHeight[nn.Type];
                    nn.Location.X += (Block[A].Location.Width - nn.Location.Width) / 2.0;
                    nn.DefaultLocation = nn.Location;
                    nn.DefaultType = nn.Type;

                    if(NPCFrame[nn.Type] > 0)
                    {
                        nn.Direction = 1;
                        nn.Frame = EditorNPCFrame(nn.Type, nn.Direction);
                        nn.DefaultDirection = nn.Direction;
                    }

                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);
                }
                else
                    break;
            }
        }

        // remove blocks that didn't get converted from the list
        PSwitchBlocks.resize(numConverted);

        // return them to reverse index order
        if(CompatGetLevel() == 1 || CompatGetLevel() == 2)
        {
            std::sort(PSwitchBlocks.begin(), PSwitchBlocks.end(),
                [](BaseRef_t a, BaseRef_t b) {
                    return a > b;
                });
        }

        // kill the blocks
        for(int A : PSwitchBlocks)
        {
            {
                {
                    Block[A] = Block[numBlock];
                    Block[numBlock] = blankBlock;
                    numBlock--;
                    syncLayersTrees_Block(A);
                    syncLayersTrees_Block(numBlock + 1);
                }
            }
        }

        ProcEvent(EVENT_PSWITCH_START, true);
    }
    else
    {
        for(A = 1; A <= numNPCs; A++)
        {
            if(NPC[A].Block > 0)
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
                    nb.Type = NPC[A].Block;
                    nb.Location = NPC[A].Location;
                    nb.Location.SpeedX = 0;
                    nb.Location.SpeedY = 0;
                    nb.Location.Width = BlockWidth[nb.Type];
                    nb.Location.Height = BlockHeight[nb.Type];
                    nb.Location.X += (NPC[A].Location.Width - nb.Location.Width) / 2.0;
                    nb.Special = 0;
                    nb.Kill = false;
                    syncLayersTrees_Block(numBlock);
                }
                NPC[A].Killed = 9;
            }
        }


        // this has been made more complicated because the Blocks are no longer sorted like they were previously

        // use one of the pre-allocated result vectors
        TreeResult_Sentinel<BlockRef_t> sent;
        std::vector<BaseRef_t>& PSwitchBlocks = *sent.i_vec;

        // fill it with the PSwitch-affected blocks
        for(A = numBlock; A >= 1; A--)
        {
            if(Block[A].NPC > 0 && !Block[A].Hidden)
            {
                PSwitchBlocks.push_back(A);
            }
        }

        // sort them in reverse location order
        if(CompatGetLevel() == 1 || CompatGetLevel() == 2)
        {
            std::sort(PSwitchBlocks.begin(), PSwitchBlocks.end(),
                [](BaseRef_t a, BaseRef_t b) {
                    return (((BlockRef_t)a)->Location.X > ((BlockRef_t)b)->Location.X
                        || (((BlockRef_t)a)->Location.X == ((BlockRef_t)b)->Location.X
                            && ((BlockRef_t)a)->Location.Y > ((BlockRef_t)b)->Location.Y));
                });
        }

        // restore the NPCs
        size_t numConverted = 0;
        for(; numConverted < PSwitchBlocks.size(); numConverted++)
        {
            A = PSwitchBlocks[numConverted];
            {
                if(numNPCs < maxNPCs)
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
                    nn.Type = Block[A].NPC;
                    nn.Location = Block[A].Location;
                    nn.Location.SpeedX = 0;
                    nn.Location.SpeedY = 0;
                    nn.Location.Width = NPCWidth[nn.Type];
                    nn.Location.Height = NPCHeight[nn.Type];
                    nn.Location.X += (Block[A].Location.Width - nn.Location.Width) / 2.0;
                    nn.DefaultLocation = nn.Location;
                    nn.DefaultType = nn.Type;

                    if(NPCFrame[nn.Type] > 0)
                    {
                        nn.Direction = 1;
                        nn.Frame = EditorNPCFrame(nn.Type, nn.Direction);
                        nn.DefaultDirection = nn.Direction;
                    }

                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);
                    nn.Killed = 0;
                }
                else
                    break;
            }
        }

        // remove blocks that didn't get converted from the list
        PSwitchBlocks.resize(numConverted);

        // shouldn't actually return them to reverse index order: KillBlock might internally call events, so it should be called in reverse location order

        // kill the blocks
        for(int A : PSwitchBlocks)
        {
            {
                {
                    KillBlock(A, false);
                    Block[A].Layer = LAYER_USED_P_SWITCH;
                    syncLayersTrees_Block(A);
                    // this is as close to a permanent death as blocks get in the game,
                    // because this layer usually doesn't exist
                }
            }
        }

        ProcEvent(EVENT_PSWITCH_END, true);
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

    if(g_compatibility.emulate_classic_block_order)
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
    }

    iBlocks = numBlock;
    for(A = 1; A <= numBlock; A++)
        iBlock[A] = A;

    resetFrameTimer();
}

void PowBlock()
{
    int numScreens = 0;
    int A = 0;
    int Z = 0;

    PlaySound(SFX_Twomp);
    numScreens = 1;

    if(!LevelEditor)
    {
        if(ScreenType == 1)
            numScreens = 2;
        if(ScreenType == 4)
            numScreens = 2;

        if(ScreenType == 5)
        {
            DynamicScreen();
            if(vScreen[2].Visible)
                numScreens = 2;
            else
                numScreens = 1;
        }

        if(ScreenType == 8)
            numScreens = 1;
    }

    for(Z = 1; Z <= numScreens; Z++)
    {
        for(A = 1; A <= numBlock; A++)
        {
            if(vScreenCollision(Z, Block[A].Location))
            {
                if(!Block[A].Hidden)
                {
                    BlockShakeUpPow(A);
                }
            }
        }
    }

    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].Active)
        {
            if(NPCIsACoin[NPC[A].Type])
            {
                NPC[A].Special = 1;
                NPC[A].Location.SpeedX = (dRand() * 1.0) - 0.5;
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
