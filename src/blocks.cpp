/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "globals.h"
#include "blocks.h"
#include "sound.h"
#include "graphics.h"
#include "effect.h"
#include "collision.h"
#include "npc.h"
#include "player.h"
#include "sorting.h"
#include "layers.h"

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
    if(BattleMode == true && b.RespawnDelay == 0)
    {
        b.RespawnDelay = 1;
    }
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
                PlaySound(34);
                // UnDuck whatPlayer
                if(b.Type == 622)
                {
                    Player[whatPlayer].Character = 1;
                }
                if(b.Type == 623)
                {
                    Player[whatPlayer].Character = 2;
                }
                if(b.Type == 624)
                {
                    Player[whatPlayer].Character = 3;
                }
                if(b.Type == 625)
                {
                    Player[whatPlayer].Character = 4;
                }
                if(b.Type == 631)
                {
                    Player[whatPlayer].Character = 5;
                }

                auto &p = Player[whatPlayer];
                p.State = SavedChar[p.Character].State;
                p.HeldBonus = SavedChar[p.Character].HeldBonus;
                p.Mount = SavedChar[p.Character].Mount;
                p.MountType = SavedChar[p.Character].MountType;
                p.Hearts = SavedChar[p.Character].Hearts;
                if(p.State == 0)
                {
                    p.State = 1;
                }
                p.FlySparks = false;
                p.Immune = 50;
                p.Effect = 8;
                p.Effect2 = 14;
                if(p.Mount <= 1)
                {
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                    if(p.Mount == 1 && p.State == 1)
                    {
                        p.Location.Height = Physics.PlayerHeight[1][2];
                    }
                    p.StandUp = true;
                }
                tempLocation = p.Location;
                tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - 16;
                tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
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
        if(b.RapidHit > 0 && Player[whatPlayer].Character == 4 && whatPlayer > 0)
        {
            b.RapidHit = (iRand() % 3) + 1;
        }
        return;
    }
    b.Invis = false;
    if(HitDown == true && b.Special > 0)
    {
        tempBool = false;
        auto tmpNumBlocks = numBlock;
        for(auto B = 1; B <= tmpNumBlocks; B++)
        {
            if(B != A)
            {
                if(CheckCollision(b.Location, newLoc(Block[B].Location.X + 4, Block[B].Location.Y - 16, Block[B].Location.Width - 8, Block[B].Location.Height)))
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
        if(HitDown == false)
            BlockShakeUp(A);
        else
            BlockShakeDown(A);
    }


    if(Block[A].Type == 169)
    {
        PlaySound(32);
        BeltDirection = -BeltDirection; // for the blet direction changing block
    }


    if(b.Type == 170) // smw switch blocks
    {
        PlaySound(32);
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
        PlaySound(32);
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
        PlaySound(32);
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
        PlaySound(32);
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
            b.RapidHit = (iRand() % 3) + 1;
        }
        if(HitDown == false)
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
                if(B != A && Block[B].Hidden == false && (BlockOnlyHitspot1[Block[B].Type] & !BlockIsSizable[Block[B].Type]) == 0)
                {
                    if(CheckCollision(Block[B].Location, newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30)))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                for(auto B = 1; B <= b.Special; B++)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    if(newBlock == 89)
                    {
                        NPC[numNPCs].Type = 33;
                    }
                    else if(newBlock == 192)
                    {
                        NPC[numNPCs].Type = 88;
                    }
                    else
                    {
                        NPC[numNPCs].Type = 10;
                    }
                    if(Player[whatPlayer].Character == 5)
                    {
                        NPC[numNPCs].Type = 251;
                        if(dRand() * 20 <= 3.0)
                            NPC[numNPCs].Type = 252;
                        if(dRand() * 60 <= 3.0)
                            NPC[numNPCs].Type = 253;
                        PlaySound(81);
                    }
                    else
                    {
                        PlaySound(14);
                    }
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.X = b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                    NPC[numNPCs].Location.Y = b.Location.Y - NPC[numNPCs].Location.Height - 0.01;
                    NPC[numNPCs].Location.SpeedX = dRand() * 3 - 1.5;
                    NPC[numNPCs].Location.SpeedY = -(dRand() * 4) - 3;
                    if(HitDown == true)
                    {
                        NPC[numNPCs].Location.SpeedY = -NPC[numNPCs].Location.SpeedY * 0.5;
                        NPC[numNPCs].Location.Y = b.Location.Y + b.Location.Height;
                    }
                    NPC[numNPCs].Special = 1;
                    NPC[numNPCs].Immune = 20;
                    CheckSectionNPC(numNPCs);
                    if(B > 20 || (Player[whatPlayer].Character == 5 && B > 5))
                    {
                        break;
                    }
                }
                b.Special = 0;
            }
            else
            {
                Coins = Coins + 1;
                if(Coins >= 100)
                {
                    if(Lives < 99)
                    {
                        Lives = Lives + 1;
                        PlaySound(15);
                        Coins = Coins - 100;
                    }
                    else
                    {
                        Coins = 99;
                    }
                }
                PlaySound(14);
                NewEffect(11, b.Location);
                b.Special = b.Special - 1;
            }
        }
        else if(b.RapidHit > 0) // (whatPlayer > 0 And Player(whatPlayer).Character = 3)
        {
            tempBool = false;
            for(auto B = 1; B <= numBlock; B++)
            {
                if(B != A && Block[B].Hidden == false && (BlockOnlyHitspot1[Block[B].Type] & !BlockIsSizable[Block[B].Type]) == 0)
                {
                    if(CheckCollision(Block[B].Location, newLoc(b.Location.X + 1, b.Location.Y - 31, 30, 30)))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                if(newBlock == 89)
                {
                    NPC[numNPCs].Type = 33;
                }
                else if(newBlock == 192)
                {
                    NPC[numNPCs].Type = 88;
                }
                else
                {
                    NPC[numNPCs].Type = 10;
                }
                NPC[numNPCs].Type = 138;
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                NPC[numNPCs].Location.X = b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = b.Location.Y - NPC[numNPCs].Location.Height - 0.01;
                NPC[numNPCs].Location.SpeedX = dRand() * 3.0 - 1.5;
                NPC[numNPCs].Location.SpeedY = -(dRand() * 4) - 3;
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].Immune = 20;
                PlaySound(14);
                CheckSectionNPC(numNPCs);
                b.Special = b.Special - 1;
            }
            else
            {
                Coins = Coins + 1;
                if(Coins >= 100)
                {
                    if(Lives < 99)
                    {
                        Lives = Lives + 1;
                        PlaySound(15);
                        Coins = Coins - 100;
                    }
                    else
                    {
                        Coins = 99;
                    }
                }
                PlaySound(14);
                NewEffect(11, b.Location);
                b.Special = b.Special - 1;
            }

        }
        else
        {
            Coins = Coins + 1;
            if(Coins >= 100)
            {
                if(Lives < 99)
                {
                    Lives = Lives + 1;
                    PlaySound(15);
                    Coins = Coins - 100;
                }
                else
                {
                    Coins = 99;
                }
            }
            PlaySound(14);
            NewEffect(11, b.Location);
            b.Special = b.Special - 1;
        }

        if(b.Special == 0 && !(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }




    }
    else if(b.Special >= 1000) // New spawn code
    {
        C = b.Special - 1000; // this finds the NPC type and puts in the variable C
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55)) // 55 is the bouncy note block
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
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
        {
            numNPCs++; // create a new NPC
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 1000;
            if(NPCIsYoshi[C])
            {
                NPC[numNPCs].Type = 96;
                NPC[numNPCs].Special = C;
            }
            else if(numPlayers > 2)
            {
                NPC[numNPCs].Type = C;
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
                    NPC[numNPCs].Type = C;
                }
                else
                {
                    NPC[numNPCs].Type = 9;
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
                    NPC[numNPCs].Type = C;
                }
                else
                {
                    NPC[numNPCs].Type = 185;
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
                    NPC[numNPCs].Type = C;
                }
                else
                {
                    NPC[numNPCs].Type = 184;
                }
            }
            else
            {
                NPC[numNPCs].Type = C;
            }

            if(makeShroom && whatPlayer > 0 &&
               (Player[whatPlayer].State > 1 || Player[whatPlayer].Character == 5)) // set the NPC type if the conditions are met
            {
                NPC[numNPCs].Type = C;
            }

            if(makeShroom && BattleMode) // always spawn the item in battlemode
            {
                NPC[numNPCs].Type = C;
            }

            if(NPC[numNPCs].Type == 287)
            {
                NPC[numNPCs].Type = RandomBonus();
            }

            CharStuff(numNPCs);
            NPC[numNPCs].Location.Width = NPCWidth[C];
            // Make block a bit smaller to allow player take a bonus easier (Redigit's idea)
            if(fEqual(b.Location.Width, 32) && !b.wasShrinkResized)
            {
                b.Location.Width = b.Location.Width - 0.1;
                b.Location.X = b.Location.X + 0.05;
                b.wasShrinkResized = true; // Don't move it!!!
            }

            NPC[numNPCs].Location.Height = 0;
            NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;

            if(NPCIsYoshi[C]) // if the npc is yoshi then set the color of the egg
            {
                if(C == 98)
                {
                    NPC[numNPCs].Frame = 1;
                }
                else if(C == 99)
                {
                    NPC[numNPCs].Frame = 2;
                }
                else if(C == 100)
                {
                    NPC[numNPCs].Frame = 3;
                }
                else if(C == 148)
                {
                    NPC[numNPCs].Frame = 4;
                }
                else if(C == 149)
                {
                    NPC[numNPCs].Frame = 5;
                }
                else if(C == 150)
                {
                    NPC[numNPCs].Frame = 6;
                }
            }

            if(HitDown == false)
            {
                NPC[numNPCs].Location.Y = b.Location.Y; // - 0.1
                NPC[numNPCs].Location.Height = 0;
                if(NPCIsYoshi[C])
                {
                    NPC[numNPCs].Effect = 0;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Y = b.Location.Y - 32;
                }
                else if(NPC[numNPCs].Type == 34)
                {
                    NPC[numNPCs].Effect = 0;
                    NPC[numNPCs].Location.Y = b.Location.Y - 32;
                    NPC[numNPCs].Location.SpeedY = -6;
                    NPC[numNPCs].Location.Height = NPCHeight[C];
                    // PlaySound(7); // Don't play mushroom sound on leaf, like in original SMB3
                }
                else
                {
                    NPC[numNPCs].Effect = 1;
                    PlaySound(7);
                }
            }
            else
            {
                NPC[numNPCs].Location.Y = b.Location.Y + 4;
                NPC[numNPCs].Location.Height = NPCHeight[C];
                NPC[numNPCs].Effect = 3;
                PlaySound(7);
            }

            NPC[numNPCs].Effect2 = 0;
            CheckSectionNPC(numNPCs);
            if(NPCIsYoshi[NPC[numNPCs].Type] ||
               NPCIsBoot[NPC[numNPCs].Type] || NPC[numNPCs].Type == 9 ||
               NPC[numNPCs].Type == 14 || NPC[numNPCs].Type == 22 ||
               NPC[numNPCs].Type == 90 || NPC[numNPCs].Type == 153 ||
               NPC[numNPCs].Type == 169 || NPC[numNPCs].Type == 170 ||
               NPC[numNPCs].Type == 182 || NPC[numNPCs].Type == 183 ||
               NPC[numNPCs].Type == 184 || NPC[numNPCs].Type == 185 ||
               NPC[numNPCs].Type == 186 || NPC[numNPCs].Type == 187 ||
               NPC[numNPCs].Type == 188 || NPC[numNPCs].Type == 195)
            {
                NPC[numNPCs].TimeLeft = Physics.NPCTimeOffScreen * 20;
            }
        }
        else // Spawn the player
        {
            PlaySound(7);
            Player[tempPlayer].State = 1;
            Player[tempPlayer].Location.Width = Physics.PlayerWidth[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.Height = Physics.PlayerHeight[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Frame = 1;
            Player[tempPlayer].Dead = false;
            Player[tempPlayer].Location.X = b.Location.X + b.Location.Width * 0.5 - Player[tempPlayer].Location.Width * 0.5;
            if(HitDown == false)
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

    }
    else if(b.Special == 100) // Block contains a mushroom
    {
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
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
        PlaySound(7);
        if(tempPlayer == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 1000;
            NPC[numNPCs].Type = 9;
            NPC[numNPCs].Location.Width = NPCWidth[9];
            NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            if(HitDown == false)
            {
                NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
                NPC[numNPCs].Location.Height = 0;
                NPC[numNPCs].Effect = 1;
            }
            else
            {
                NPC[numNPCs].Location.Y = b.Location.Y + 4;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Effect = 3;
            }
            NPC[numNPCs].Effect2 = 0;
            CheckSectionNPC(numNPCs);
        }
        else
        {
            Player[tempPlayer].Location.Width = Physics.PlayerWidth[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.Height = Physics.PlayerHeight[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Frame = 1;
            Player[tempPlayer].Dead = false;
            Player[tempPlayer].Location.X = b.Location.X + b.Location.Width * 0.5 - Player[tempPlayer].Location.Width * 0.5;
            if(HitDown == false)
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
    }
    else if(b.Special == 102) // Block contains a fire flower
    {
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
        if(HitDown == false)
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
        PlaySound(7);
        if(tempPlayer == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 1000;
            for(auto B = 1; B <= numPlayers; B++)
            {
                if(Player[B].State == 1)
                {
                    makeShroom = true;
                }
            }
            if(!makeShroom)
            {
                NPC[numNPCs].Type = 14;
            }
            else
            {
                NPC[numNPCs].Type = 9;
            }
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            if(HitDown == false)
            {
                NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
                NPC[numNPCs].Location.Height = 0;
                NPC[numNPCs].Effect = 1;
            }
            else
            {
                NPC[numNPCs].Location.Y = b.Location.Y + 4;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Effect = 3;
            }
            NPC[numNPCs].Effect2 = 0;
            CheckSectionNPC(numNPCs);
        }
        else // Rez player
        {
            Player[tempPlayer].Frame = 1;
            Player[tempPlayer].Dead = false;
            Player[tempPlayer].Location.Width = Physics.PlayerWidth[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.Height = Physics.PlayerHeight[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.X = b.Location.X + b.Location.Width * 0.5 - Player[tempPlayer].Location.Width * 0.5;
            if(HitDown == false)
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
    }
    else if(b.Special == 103) // Block contains a Leaf
    {
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[b.Type];
            b.Location.Width = BlockWidth[b.Type];
        }
        if(HitDown == false)
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
        PlaySound(7);
        if(tempPlayer == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 1000;
            for(auto B = 1; B <= numPlayers; B++)
            {
                if(Player[B].State == 1)
                {
                    makeShroom = true;
                }
            }
            if(!makeShroom)
            {
                NPC[numNPCs].Type = 34;
            }
            else
            {
                NPC[numNPCs].Type = 9;
            }
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
            NPC[numNPCs].Location.SpeedX = 0;
            if(HitDown == false)
            {
                if(NPC[numNPCs].Type == 34)
                {
                    NPC[numNPCs].Location.Y = b.Location.Y - 32;
                    NPC[numNPCs].Location.SpeedY = -6;
                    NPC[numNPCs].Location.Height = NPCHeight[34];
                }
                else
                {
                    NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
                    NPC[numNPCs].Location.Height = 0;
                    NPC[numNPCs].Effect = 1;
                }
            }
            else
            {
                NPC[numNPCs].Location.Y = b.Location.Y + 4;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Effect = 3;
            }
            NPC[numNPCs].Effect2 = 0;
            CheckSectionNPC(numNPCs);
        }
        else // Rez player
        {
            Player[tempPlayer].Location.Width = Physics.PlayerWidth[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Location.Height = Physics.PlayerHeight[Player[tempPlayer].Character][Player[tempPlayer].State];
            Player[tempPlayer].Frame = 1;
            Player[tempPlayer].Dead = false;
            Player[tempPlayer].Location.X = b.Location.X + b.Location.Width * 0.5 - Player[tempPlayer].Location.Width * 0.5;
            if(HitDown == false)
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
    }
    else if(b.Special == 104) // Block contains a Shoe
    {
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        PlaySound(7);
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = 35;
        NPC[numNPCs].Direction = -1;
        NPC[numNPCs].Location.Width = NPCWidth[35];
        NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
        NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        if(HitDown == false)
        {
            NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
            NPC[numNPCs].Location.Height = 0;
            NPC[numNPCs].Effect = 1;
        }
        else
        {
            NPC[numNPCs].Location.Y = b.Location.Y + 4;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Effect = 3;
        }
        NPC[numNPCs].Effect2 = 0;
        CheckSectionNPC(numNPCs);
    }
    else if(b.Special == 105) // Block contains a Green Yoshi
    {
        SoundPause[2] = 2;
        PlaySound(7);

        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = 96;
        NPC[numNPCs].Special = 95;
        NPC[numNPCs].Direction = 1;
        NPC[numNPCs].Location.Width = NPCWidth[96];
        NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
        NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        if(HitDown == false)
        {
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Y = b.Location.Y - 32;
            NPC[numNPCs].Effect = 0;
        }
        else
        {
            PlaySound(7);
            NPC[numNPCs].Location.Y = b.Location.Y + 4;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Effect = 3;
        }
        NPC[numNPCs].Effect2 = 0;
        CheckSectionNPC(numNPCs);
    }
    else if(b.Special == 101) // Block contains a Goomba
    {
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        PlaySound(7);
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = 1;
        NPC[numNPCs].Location.Width = NPCWidth[9];
        NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
        NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        if(HitDown == false)
        {
            NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
            NPC[numNPCs].Location.Height = 0;
            NPC[numNPCs].Effect = 1;
        }
        else
        {
            NPC[numNPCs].Location.Y = b.Location.Y + 4;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Effect = 3;
        }
        NPC[numNPCs].Effect2 = 0;
        CheckSectionNPC(numNPCs);
    }
    else if(b.Special == 201) // Block contains a 1-up
    {
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        b.Special = 0;
        if(!(b.Type == 55))
        {
            b.Type = newBlock;
            b.Location.Height = BlockHeight[newBlock];
            b.Location.Width = BlockWidth[newBlock];
        }
        if(HitDown == false)
        {
            BlockShakeUp(A);
        }
        else
        {
            BlockShakeDown(A);
        }
        PlaySound(7);
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = 90;
        NPC[numNPCs].Location.Width = NPCWidth[90];
        NPC[numNPCs].Location.X = (b.Location.X + b.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0);
        NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        if(HitDown == false)
        {
            NPC[numNPCs].Location.Y = b.Location.Y - 0.1;
            NPC[numNPCs].Location.Height = 0;
            NPC[numNPCs].Effect = 1;
        }
        else
        {
            NPC[numNPCs].Location.Y = b.Location.Y + 4;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Effect = 3;
        }
        NPC[numNPCs].Effect2 = 0;
        CheckSectionNPC(numNPCs);
    }

    if(PSwitchTime > 0 && newBlock == 89 && b.Special == 0 && oldSpecial > 0)
    {
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 1;
        NPC[numNPCs].Type = 33;
        NPC[numNPCs].Block = 89;
        NPC[numNPCs].Location = b.Location;
        NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
        NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
        NPC[numNPCs].Location.X = NPC[numNPCs].Location.X + (b.Location.Width - NPC[numNPCs].Location.Width) / 2.0;
        NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 0.01;
        NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
        NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
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
    if(Block[A].Hidden == false)
    {
        Block[A].ShakeY = -12; // Go up
        Block[A].ShakeY2 = 12; // Come back down
        Block[A].ShakeY3 = 0;
        if(A != iBlock[iBlocks])
        {
            iBlocks += 1;
            iBlock[iBlocks] = A;
        }
    }
}

void BlockShakeUpPow(int A)
{
    if(Block[A].Hidden == false)
    {
        Block[A].ShakeY = -6; // Go up
        Block[A].ShakeY2 = 6; // Come back down
        Block[A].ShakeY3 = 0;
        if(A != iBlock[iBlocks])
        {
            iBlocks += 1;
            iBlock[iBlocks] = A;
        }
    }
}

void BlockShakeDown(int A)
{
    if(Block[A].Hidden == false)
    {
        Block[A].ShakeY = 12; // Go down
        Block[A].ShakeY2 = -12; // Come back up
        Block[A].ShakeY3 = 0;
        if(A != iBlock[iBlocks])
        {
            iBlocks = iBlocks + 1;
            iBlock[iBlocks] = A;
        }
    }
}

void BlockHitHard(int A)
{
    if(Block[A].Hidden == false)
    {
        if(Block[A].Type == 90)
        {
            // Block(A).Hidden = True
            // NewEffect 82, Block(A).Location, , A
            // PlaySound 3
        }
        else
        {
            Block[A].Kill = true;
            iBlocks = iBlocks + 1;
            iBlock[iBlocks] = A;
        }
    }
}

void KillBlock(int A, bool Splode)
{
    Block_t blankBlock;
    bool tempBool = false;
    int C = 0;
    if(Block[A].Hidden == true)
        return;
    if(BattleMode == true && Block[A].RespawnDelay == 0)
        Block[A].RespawnDelay = 1;
    if(Splode == true)
    {
        if(Block[A].Type == 526)
            PlaySound(64);
        else if(Block[A].Type == 186)
            PlaySound(43);
        else
            PlaySound(4); // Block smashed
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
    if(LevelEditor == true)
    {
        if(numBlock > 0)
        {
            Block[A] = Block[numBlock];
            Block[numBlock] = blankBlock;
            numBlock = numBlock - 1;
        }
    }
    else
    {
        Score += 50;
        if(Block[A].TriggerDeath != "")
        {
            ProcEvent(Block[A].TriggerDeath);
        }
        if(Block[A].TriggerLast != "")
        {
            tempBool = false;
            for(C = 1; C <= numNPCs; C++)
            {
                if(NPC[C].Layer == Block[A].Layer && NPC[C].Generator == false)
                    tempBool = true;
            }
            for(C = 1; C <= numBlock; C++)
            {
                if(C != A)
                {
                    if(Block[A].Layer == Block[C].Layer)
                        tempBool = true;
                }
            }
            if(tempBool == false)
            {
                ProcEvent(Block[A].TriggerLast);
            }
        }
        Block[A].Hidden = true;
        Block[A].Layer = "Destroyed Blocks";
        Block[A].Kill = false;
    }

}

void BlockFrames()
{
    int A = 0;

    bool pChar[maxPlayers] = {false};
    bool tempBool = false;
    SDL_memset(pChar, 0, sizeof(pChar));

    if(FreezeNPCs == true)
        return;
    // Update block frame counter
    BlockFrame2[4] = BlockFrame2[4] + 1;
    if(BlockFrame2[4] == 8)
        BlockFrame2[4] = 0;
    BlockFrame2[5] = BlockFrame2[5] + 1;
    if(BlockFrame2[5] == 8)
        BlockFrame2[5] = 0;
    BlockFrame2[30] = BlockFrame2[30] + 1;
    if(BlockFrame2[30] == 8)
        BlockFrame2[30] = 0;
    BlockFrame2[55] = BlockFrame2[55] + 1;
    if(BlockFrame2[55] == 8)
        BlockFrame2[55] = 0;
    BlockFrame2[88] = BlockFrame2[88] + 1;
    if(BlockFrame2[88] == 8)
        BlockFrame2[88] = 0;
    BlockFrame2[109] = BlockFrame2[109] + 1;
    if(BlockFrame2[109] == 4)
        BlockFrame2[109] = 0;
    BlockFrame2[371] = BlockFrame2[371] + 1;
    if(BlockFrame2[371] == 8)
        BlockFrame2[371] = 0;
    BlockFrame2[379] = BlockFrame2[379] + 1;
    if(BlockFrame2[379] >= 12)
        BlockFrame2[379] = 0;
    // Check if the block type is ready for the next frame
    if(BlockFrame2[4] == 0)
    {
        BlockFrame[4] = BlockFrame[4] + 1;
        if(BlockFrame[4] == 4)
            BlockFrame[4] = 0;
    }
    if(BlockFrame2[5] == 0)
    {
        BlockFrame[5] = BlockFrame[5] + 1;
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

    BlockFrame2[626] = BlockFrame2[626] + 1;
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

    if(pChar[1] == false)
        BlockFrame[626] = 0;
    if(pChar[2] == false)
        BlockFrame[627] = 0;
    if(pChar[3] == false)
        BlockFrame[628] = 0;
    if(pChar[4] == false)
        BlockFrame[629] = 0;
    if(pChar[5] == false)
        BlockFrame[632] = 0;

    if(BlockFrame2[30] == 0)
    {
        BlockFrame[30] = BlockFrame[30] + 1;
        if(BlockFrame[30] == 4)
            BlockFrame[30] = 0;
    }
    if(BlockFrame2[55] == 0)
    {
        BlockFrame[55] = BlockFrame[55] + 1;
        if(BlockFrame[55] == 4)
            BlockFrame[55] = 0;
    }
    if(BlockFrame2[88] == 0)
    {
        BlockFrame[88] = BlockFrame[88] + 1;
        if(BlockFrame[88] == 4)
            BlockFrame[88] = 0;
    }
    BlockFrame[170] = BlockFrame[88];
    if(BlockFrame2[109] == 0)
    {
        BlockFrame[109] = BlockFrame[109] + 1;
        if(BlockFrame[109] == 8)
            BlockFrame[109] = 0;
    }
    if(BlockFrame2[371] == 0)
    {
        BlockFrame[371] = BlockFrame[371] + 1;
        if(BlockFrame[371] == 8)
            BlockFrame[371] = 0;
    }
    if(BlockFrame2[379] == 0)
    {
        BlockFrame[379] = BlockFrame[379] + 1;
        if(BlockFrame[379] == 4)
            BlockFrame[379] = 0;
    }
    BlockFrame[380] = BlockFrame[379];
    BlockFrame[381] = BlockFrame[379];
    BlockFrame[382] = BlockFrame[379];

    BlockFrame2[530] = BlockFrame2[530] + 1;
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
            if(Player[A].Stoned == true)
            {
                tempBool = true;
                break;
            }
        }
        if(BlockFrame[458] < 5 && tempBool == true)
        {
            BlockFrame2[458] = BlockFrame2[458] + 1;
            if(BlockFrame2[458] >= 4)
            {
                BlockFrame2[458] = 0;
                BlockFrame[458] = BlockFrame[458] + 1;
            }
        }
        else if(BlockFrame[458] > 0 && tempBool == false)
        {
            BlockFrame2[458] = BlockFrame2[458] + 1;
            if(BlockFrame2[458] >= 4)
            {
                BlockFrame2[458] = 0;
                BlockFrame[458] = BlockFrame[458] - 1;
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
    if(FreezeNPCs == true)
        return;
    BlockFrames();
    if(BattleMode == true)
    {
        for(A = 1; A <= numBlock; A++)
        {
            // respawn
            if(Block[A].RespawnDelay > 0)
            {
                Block[A].RespawnDelay = Block[A].RespawnDelay + 1;
                if(Block[A].RespawnDelay >= 65 * 60)
                {
                    if(Block[A].DefaultType > 0 || Block[A].DefaultSpecial > 0 || Block[A].Layer == "Destroyed Blocks")
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(CheckCollision(Block[A].Location, newLoc(Player[B].Location.X - 64, Player[B].Location.Y - 64, 128, 128)))
                            {
                                B = 0;
                                break;
                            }
                        }
                        if(B > 0)
                        {
                            if(Block[A].Layer == "Destroyed Blocks")
                                Block[A].Layer = "Default";
                            if(Block[A].Hidden == true)
                            {
                                for(B = 0; B <= maxLayers; B++)
                                {
                                    if(Layer[B].Name == Block[A].Layer)
                                        Block[A].Hidden = Layer[B].Hidden;
                                }
                                if(Block[A].Hidden == false)
                                    NewEffect(10, newLoc(Block[A].Location.X + Block[A].Location.Width / 2.0 - EffectWidth[10] / 2, Block[A].Location.Y + Block[A].Location.Height / 2.0 - EffectHeight[10] / 2));
                            }
                            if(Block[A].Type != Block[A].DefaultType || Block[A].Special != Block[A].DefaultSpecial)
                            {
                                if(Block[A].Type != Block[A].DefaultType)
                                    NewEffect(10, newLoc(Block[A].Location.X + Block[A].Location.Width / 2.0 - EffectWidth[10] / 2, Block[A].Location.Y + Block[A].Location.Height / 2.0 - EffectHeight[10] / 2));
                                Block[A].Special = Block[A].DefaultSpecial;
                                Block[A].Type = Block[A].DefaultType;
                            }
                            Block[A].RespawnDelay = 0;
                        }
                        else
                            Block[A].RespawnDelay = 65 * 30;
                    }
                    else
                        Block[A].RespawnDelay = 0;
                }
            }
        }
    }

    for(auto A = 1; A <= iBlocks; A++)
    {
        // Update the shake effect
        if(Block[iBlock[A]].Hidden == true)
        {
            Block[iBlock[A]].ShakeY = 0;
            Block[iBlock[A]].ShakeY2 = 0;
            Block[iBlock[A]].ShakeY3 = 0;
        }
        if(Block[iBlock[A]].ShakeY < 0) // Block Shake Up
        {
            Block[iBlock[A]].ShakeY = Block[iBlock[A]].ShakeY + 2;
            Block[iBlock[A]].ShakeY3 = Block[iBlock[A]].ShakeY3 - 2;
            if(Block[iBlock[A]].ShakeY == 0)
            {
                if(Block[iBlock[A]].TriggerHit != "")
                {
                    ProcEvent(Block[iBlock[A]].TriggerHit);
                }
                if(Block[iBlock[A]].Type == 282)
                    Block[iBlock[A]].Type = 283;
                else if(Block[iBlock[A]].Type == 283)
                    Block[iBlock[A]].Type = 282;
                if(Block[iBlock[A]].Type == 90 && Block[iBlock[A]].Special == 0)
                {
                    Block[iBlock[A]].Hidden = true;
                    NewEffect(82, Block[iBlock[A]].Location, 1, iBlock[A]);
                    Block[iBlock[A]].ShakeY = 0;
                    Block[iBlock[A]].ShakeY2 = 0;
                    Block[iBlock[A]].ShakeY3 = 0;
                }
            }
        }
        else if(Block[iBlock[A]].ShakeY > 0) // Block Shake Down
        {
            Block[iBlock[A]].ShakeY = Block[iBlock[A]].ShakeY - 2;
            Block[iBlock[A]].ShakeY3 = Block[iBlock[A]].ShakeY3 + 2;
            if(Block[iBlock[A]].ShakeY == 0)
            {
                if(Block[iBlock[A]].TriggerHit != "")
                {
                    ProcEvent(Block[iBlock[A]].TriggerHit);
                }
                if(Block[iBlock[A]].Type == 282)
                    Block[iBlock[A]].Type = 283;
                else if(Block[iBlock[A]].Type == 283)
                    Block[iBlock[A]].Type = 282;
                if(Block[iBlock[A]].Type == 90)
                {
                    Block[iBlock[A]].Hidden = true;
                    NewEffect(82, Block[iBlock[A]].Location, 1, iBlock[A]);
                    Block[iBlock[A]].ShakeY = 0;
                    Block[iBlock[A]].ShakeY2 = 0;
                    Block[iBlock[A]].ShakeY3 = 0;
                }
            }
        }
        else if(Block[iBlock[A]].ShakeY2 > 0) // Come back down
        {
            Block[iBlock[A]].ShakeY2 = Block[iBlock[A]].ShakeY2 - 2;
            Block[iBlock[A]].ShakeY3 = Block[iBlock[A]].ShakeY3 + 2;

            if(Block[iBlock[A]].RapidHit > 0 && Block[iBlock[A]].Special > 0 && Block[iBlock[A]].ShakeY3 == 0)
            {
                BlockHit(iBlock[A]);
                Block[iBlock[A]].RapidHit = Block[iBlock[A]].RapidHit - 1;
            }
        }
        else if(Block[iBlock[A]].ShakeY2 < 0) // Go back up
        {
            Block[iBlock[A]].ShakeY2 = Block[iBlock[A]].ShakeY2 + 2;
            Block[iBlock[A]].ShakeY3 = Block[iBlock[A]].ShakeY3 - 2;
        }

        if(Block[iBlock[A]].ShakeY3 != 0)
        {
            for(auto B = 1; B <= numNPCs; B++)
            {
                if(NPC[B].Active == true)
                {
                    if(NPC[B].Killed == 0 && NPC[B].Effect == 0 && NPC[B].HoldingPlayer == 0 && (NPCNoClipping[NPC[B].Type] == false || NPCIsACoin[NPC[B].Type] == true))
                    {
                        if(!(Block[iBlock[A]].ShakeY3 > 0) || NPCIsACoin[NPC[B].Type])
                        {
                            if(ShakeCollision(NPC[B].Location, Block[iBlock[A]].Location, Block[iBlock[A]].ShakeY3) == true)
                            {
                                if(iBlock[A] != NPC[B].tempBlock)
                                {
                                    if(Block[iBlock[A]].IsReally != B)
                                    {
                                        if(BlockIsSizable[Block[iBlock[A]].Type] == false && BlockOnlyHitspot1[Block[iBlock[A]].Type] == false)
                                        {
                                            NPCHit(B, 2, iBlock[A]);
                                        }
                                        else
                                        {
                                            if(Block[iBlock[A]].Location.Y + 1 >= NPC[B].Location.Y + NPC[B].Location.Height - 1)
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
                if(Player[B].Dead == false)
                {
                    if(Player[B].Effect == 0 && Block[iBlock[A]].Type != 55)
                    {
                        if(ShakeCollision(Player[B].Location, Block[iBlock[A]].Location, Block[iBlock[A]].ShakeY3) == true)
                        {
                            if(BlockIsSizable[Block[iBlock[A]].Type] == false && BlockOnlyHitspot1[Block[iBlock[A]].Type] == false)
                            {
                                Player[B].Location.SpeedY = double(Physics.PlayerJumpVelocity);
                                Player[B].StandUp = true;
                                PlaySound(2);
                            }
                            else
                            {
                                if(Block[iBlock[A]].Location.Y + 1 >= Player[B].Location.Y + Player[B].Location.Height - 1)
                                {
                                    Player[B].Location.SpeedY = double(Physics.PlayerJumpVelocity);
                                    Player[B].StandUp = true;
                                    PlaySound(2);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(Block[iBlock[A]].Kill == true) // See if block should be broke
        {
            Block[iBlock[A]].Kill = false;
            if(Block[iBlock[A]].Special == 0)
            {
                if(Block[iBlock[A]].Type == 4 || Block[iBlock[A]].Type == 60 ||
                   Block[iBlock[A]].Type == 90 || Block[iBlock[A]].Type == 188 ||
                   Block[iBlock[A]].Type == 226 || Block[iBlock[A]].Type == 293 ||
                   Block[iBlock[A]].Type == 526) // Check to see if it is breakable
                    KillBlock(iBlock[A]); // Destroy the block
            }
        }
    }

    for(auto A = iBlocks; A >= 1; A--)
    {
        if(Block[iBlock[A]].ShakeY == 0)
        {
            if(Block[iBlock[A]].ShakeY2 == 0)
            {
                if(Block[iBlock[A]].ShakeY3 == 0)
                {
                    iBlock[A] = iBlock[iBlocks];
                    iBlocks = iBlocks - 1;
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
            PlaySound(32);
            PSwitch(true);
        }

        PSwitchTime--;

        if(PSwitchTime == 195)
            PlaySound(92);

        if(PSwitchTime <= 1)
        {
            PSwitch(false);
            StopMusic();
            StartMusic(Player[PSwitchPlayer].Section);
        }
    }
}

void PSwitch(bool Bool)
{
    int A = 0;
    int B = 0;
    Block_t blankBlock;
    if(Bool == true)
    {
        for(A = 1; A <= numNPCs; A++)
        {
            if(NPCIsACoin[NPC[A].Type] == true && NPC[A].Block == 0 && NPC[A].Hidden == false && NPC[A].Special == 0.0)
            {
                if(numBlock < maxBlocks)
                {
                    numBlock = numBlock + 1;
                    if((NPC[A].Type == 251 || NPC[A].Type == 252 || NPC[A].Type == 253) && NPC[A].DefaultType != 0)
                        NPC[A].Type = NPC[A].DefaultType;
                    if(NPC[A].Type == 33 || NPC[A].Type == 258)
                        Block[numBlock].Type = 89;
                    else if(NPC[A].Type == 88)
                        Block[numBlock].Type = 188;
                    else if(NPC[A].Type == 103)
                        Block[numBlock].Type = 280;
                    else if(NPC[A].Type == 138)
                        Block[numBlock].Type = 293;
                    else
                        Block[numBlock].Type = 4;
                    Block[numBlock].TriggerDeath = NPC[A].TriggerDeath;
                    Block[numBlock].TriggerLast = NPC[A].TriggerLast;
                    Block[numBlock].Layer = NPC[A].Layer;
                    Block[numBlock].Invis = false;
                    Block[numBlock].Hidden = false;
                    Block[numBlock].Location = NPC[A].Location;
                    Block[numBlock].Location.Width = BlockWidth[Block[numBlock].Type];
                    Block[numBlock].Location.Height = BlockHeight[Block[numBlock].Type];
                    Block[numBlock].Location.X = Block[numBlock].Location.X + (NPC[A].Location.Width - Block[numBlock].Location.Width) / 2.0;
                    Block[numBlock].Location.SpeedX = 0;
                    Block[numBlock].Location.SpeedY = 0;
                    Block[numBlock].Special = 0;
                    Block[numBlock].Kill = false;
                    Block[numBlock].NPC = NPC[A].Type;
                }
                NPC[A].Killed = 9;
            }
        }
        for(A = numBlock; A >= 1; A--)
        {
            if(BlockPSwitch[Block[A].Type] == true && Block[A].Special == 0 && Block[A].NPC == 0 && Block[A].Hidden == false)
            {
                if(numNPCs < maxNPCs)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 1;
                    if(Block[A].Type == 89)
                        NPC[numNPCs].Type = 33;
                    else if(Block[A].Type == 188 || Block[A].Type == 60)
                        NPC[numNPCs].Type = 88;
                    else if(Block[A].Type == 280)
                        NPC[numNPCs].Type = 103;
                    else if(Block[A].Type == 293)
                        NPC[numNPCs].Type = 138;
                    else
                        NPC[numNPCs].Type = 10;
                    NPC[numNPCs].Layer = Block[A].Layer;
                    NPC[numNPCs].TriggerDeath = Block[A].TriggerDeath;
                    NPC[numNPCs].TriggerLast = Block[A].TriggerLast;
                    NPC[numNPCs].Block = Block[A].Type;
                    NPC[numNPCs].Hidden = false;
                    NPC[numNPCs].Location = Block[A].Location;
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.X = NPC[numNPCs].Location.X + (Block[A].Location.Width - NPC[numNPCs].Location.Width) / 2.0;
                    NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
                    NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
                    CheckSectionNPC(numNPCs);
                    Block[A] = Block[numBlock];
                    Block[numBlock] = blankBlock;
                    numBlock = numBlock - 1;
                }
            }
        }
        ProcEvent("P Switch - Start", true);
    }
    else
    {
        for(A = 1; A <= numNPCs; A++)
        {
            if(NPC[A].Block > 0)
            {
                if(numBlock < maxBlocks)
                {
                    numBlock = numBlock + 1;
                    Block[numBlock].Layer = NPC[A].Layer;
                    Block[numBlock].TriggerDeath = NPC[A].TriggerDeath;
                    Block[numBlock].TriggerLast = NPC[A].TriggerLast;
                    Block[numBlock].Hidden = NPC[A].Hidden;
                    Block[numBlock].Invis = false;
                    Block[numBlock].Type = NPC[A].Block;
                    Block[numBlock].Location = NPC[A].Location;
                    Block[numBlock].Location.SpeedX = 0;
                    Block[numBlock].Location.SpeedY = 0;
                    Block[numBlock].Location.Width = BlockWidth[Block[numBlock].Type];
                    Block[numBlock].Location.Height = BlockHeight[Block[numBlock].Type];
                    Block[numBlock].Location.X = Block[numBlock].Location.X + (NPC[A].Location.Width - Block[numBlock].Location.Width) / 2.0;
                    Block[numBlock].Special = 0;
                    Block[numBlock].Kill = false;
                }
                NPC[A].Killed = 9;
            }
        }


        // Stop
        for(A = numBlock; A >= 1; A--)
        {
            if(Block[A].NPC > 0 && Block[A].Hidden == false)
            {
                if(numNPCs < maxNPCs)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Layer = Block[A].Layer;
                    NPC[numNPCs].TriggerDeath = Block[A].TriggerDeath;
                    NPC[numNPCs].TriggerLast = Block[A].TriggerLast;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 1;
                    NPC[numNPCs].Hidden = Block[A].Hidden;
                    NPC[numNPCs].Type = Block[A].NPC;
                    NPC[numNPCs].Location = Block[A].Location;
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.X = NPC[numNPCs].Location.X + (Block[A].Location.Width - NPC[numNPCs].Location.Width) / 2.0;
                    NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
                    NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
                    CheckSectionNPC(numNPCs);
                    NPC[numNPCs].Killed = 0;
                    KillBlock(A, false);
                    Block[A].Layer = "Used P Switch";
                }
            }
        }

        ProcEvent("P Switch - End", true);
    }
    qSortBlocksX(1, numBlock);
    B = 1;
    for(A = 2; A <= numBlock; A++)
    {
        if(Block[A].Location.X > Block[B].Location.X)
        {
            qSortBlocksY(B, A - 1);
            B = A;
        }
    }
    qSortBlocksY(B, A - 1);
    FindSBlocks();
    FindBlocks();
    iBlocks = numBlock;
    for(A = 1; A <= numBlock; A++)
        iBlock[A] = A;
    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    fpsTime = 0;
    cycleCount = 0;
    gameTime = 0;
}

void PowBlock()
{
    int numScreens = 0;
    int A = 0;
    int Z = 0;
    PlaySound(37);
    numScreens = 1;
    if(LevelEditor == false)
    {
        if(ScreenType == 1)
            numScreens = 2;
        if(ScreenType == 4)
            numScreens = 2;
        if(ScreenType == 5)
        {
            DynamicScreen();
            if(vScreen[2].Visible == true)
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
                if(Block[A].Hidden == false)
                {
                    BlockShakeUpPow(A);
                }
            }
        }
    }
    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].Active == true)
        {
            if(NPCIsACoin[NPC[A].Type])
            {
                NPC[A].Special = 1;
                NPC[A].Location.SpeedX = (dRand() * 1.0) - 0.5;
            }
        }
    }
    if(GameMenu == false)
        ScreenShake = 20;
}
