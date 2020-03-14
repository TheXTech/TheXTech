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

#include "../globals.h"
#include "../npc.h"
#include "../sound.h"
#include "../effect.h"
#include "../layers.h"
#include "../game_main.h"


void KillNPC(int A, int B)
{
    // ------+  KILL CODES  +-------
    // B = 1      Jumped on by a player (or kicked)
    // B = 2      Hit by a shaking block
    // B = 3      Hit by projectile
    // B = 4      Hit something as a projectile
    // B = 5      Hit something while being held
    // B = 6      Touched a lava block
    // B = 8      Stomped by boot
    // B = 9      Time to DIE
    // B = 10     Zelda Stab
    bool DontSpawnExit = false;
    bool DontResetMusic = false;
    bool tempBool = false;
    NPC_t blankNPC;
    int C = 0;
    Location_t tempLocation;

    if(NPC[A].Type == 263 && NPC[A].Special > 0 && NPC[A].Killed != 9)
    {
        NewEffect(10, NPC[A].Location);
        for(C = 1; C <= 50; C++)
        {
            tempLocation.Height = EffectHeight[80];
            tempLocation.Width = EffectWidth[80];
            tempLocation.SpeedX = 0;
            tempLocation.SpeedY = 0;
            tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width;
            tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height;
            NewEffect(80, tempLocation);
            Effect[numEffects].Location.SpeedX = dRand() * 4 - 2 - NPC[A].Location.SpeedX * 0.2;
            Effect[numEffects].Location.SpeedY = dRand() * 4 - 2 + NPC[A].Location.SpeedY * 0.2;
            Effect[numEffects].Frame = (iRand() % 3);
        }
        PlaySound(9);
        NPC[A].Type = NPC[A].Special;
        if(B != 10)
        {
            MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, NPC[A].Multiplier);
        }
        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.4;
        if(NPC[A].Location.SpeedX < 2 && NPC[A].Location.SpeedX > -2)
            NPC[A].Location.SpeedX = double(2 * NPC[A].Direction);
    }
    if(NPC[A].Killed == 8 && NPC[A].Type != 13 && NPC[A].Type != 125 && NPCIsABot[NPC[A].Type] == false && NPC[A].Type != 203 && NPC[A].Type != 204 && NPC[A].Type != 205 && NPC[A].Type != 210)
        NewEffect(76, NPC[A].Location);
    if(NPC[A].Type == 59 || NPC[A].Type == 61 || NPC[A].Type == 63 || NPC[A].Type == 65 || NPC[A].DefaultType == 59 || NPC[A].DefaultType == 61 || NPC[A].DefaultType == 63 || NPC[A].DefaultType == 65)
    {
        tempBool = false;
        for(C = 1; C <= numNPCs; C++)
        {
            if(NPC[C].Type == NPC[A].Type && NPC[C].Section == NPC[A].Section && C != A)
                tempBool = true;
        }
        if(LevelEditor == true)
            tempBool = true;
        if(tempBool == false)
        {
            if(NPC[A].Type == 59 || NPC[A].DefaultType == 59)
            {
                PlaySound(32);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 171)
                        Block[C].Type = 172;
                    else if(Block[C].Type == 172)
                        Block[C].Type = 171;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == 60)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
            else if(NPC[A].Type == 61 || NPC[A].DefaultType == 61)
            {
                PlaySound(32);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 174)
                        Block[C].Type = 175;
                    else if(Block[C].Type == 175)
                        Block[C].Type = 174;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == 62)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
            else if(NPC[A].Type == 63 || NPC[A].DefaultType == 63)
            {
                PlaySound(32);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 177)
                        Block[C].Type = 178;
                    else if(Block[C].Type == 178)
                        Block[C].Type = 177;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == 64)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
            else if(NPC[A].Type == 65 || NPC[A].DefaultType == 65)
            {
                PlaySound(32);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 180)
                        Block[C].Type = 181;
                    else if(Block[C].Type == 181)
                        Block[C].Type = 180;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == 66)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
        }
    }
    if(NPC[A].TriggerDeath != "" && LevelEditor == false)
    {
        ProcEvent(NPC[A].TriggerDeath);
    }
    if(NPC[A].TriggerLast != "")
    {
        tempBool = false;
        for(C = 1; C <= numNPCs; C++)
        {
            if(C != A)
            {
                if(NPC[C].Layer == NPC[A].Layer && NPC[C].Generator == false)
                    tempBool = true;
            }
        }
        for(C = 1; C <= numBlock; C++)
        {
            if(NPC[A].Layer == Block[C].Layer)
                tempBool = true;
        }
        if(tempBool == false)
        {
            ProcEvent(NPC[A].TriggerLast);
        }
    }
    if(NPC[A].HoldingPlayer > 0)
    {
        if(!NPCIsAnExit[NPC[A].Type]) // Tell the player the NPC he was holding is dead
            Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
    }
    if(NPC[A].Killed == 10) // Things that die by Link's sword
    {
        if(!(NPC[A].Type == 15 || NPC[A].Type == 39 || NPC[A].Type == 86 || NPC[A].Type == 209 || NPC[A].Type == 200 || NPC[A].Type == 201 || NPC[A].Type == 203 || NPC[A].Type == 204 || NPC[A].Type == 205 || NPC[A].Type == 210 || NPC[A].Type == 208))
        {
            PlaySound(53);
            NewEffect(63 , NPC[A].Location);
            B = 9;
            if(dRand() * 10 <= 3)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Type = 251;
                if(dRand() * 15 <= 3)
                    NPC[numNPCs].Type = 252;
                if(dRand() * 40 <= 3)
                    NPC[numNPCs].Type = 253;
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                if(NPC[A].Location.Height >= 32)
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - NPC[numNPCs].Location.Height / 2.0;
                else if(NPC[A].Type == 51 || NPC[A].Type == 257) // Stops the rupees from spawning in blocks
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 1;
                else // Stops the rupees from spawning in blocks
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - NPC[numNPCs].Location.Height - 1;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].TimeLeft = Physics.NPCTimeOffScreen;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TailCD = 10;
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].Location.SpeedY = -5;
                NPC[numNPCs].Location.SpeedX = (1 + std::fmod(dRand(), 0.5)) * NPC[A].Direction;
            }
        }
    }
    if(!(B == 9))
    {
        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
        if(NPC[A].Type == 235 || NPC[A].Type == 231 || NPC[A].Type == 1 || NPC[A].Type == 242 || NPC[A].Type == 243 || NPC[A].Type == 244 || NPC[A].Type == 162 || NPC[A].Type == 163 || NPC[A].Type == 164 || NPC[A].Type == 165 || NPC[A].Type == 166 || NPC[A].Type == 167 || NPC[A].Type == 229 || NPC[A].Type == 236 || NPC[A].Type == 230 || NPC[A].Type == 232 || NPC[A].Type == 233 || NPC[A].Type == 234 || NPC[A].Type == 237 || NPC[A].Type == 263) // Goomba / Rex
        {
            if(B == 1 && NPC[A].Type != 229 && NPC[A].Type != 236 && NPC[A].Type != 230 && NPC[A].Type != 232 && NPC[A].Type != 233 && NPC[A].Type != 234)
            {
                if(NPC[A].Type == 1)
                    NewEffect(2, NPC[A].Location);
                else if(NPC[A].Type == 242)
                    NewEffect(126, NPC[A].Location);
                else
                    NewEffect(84, NPC[A].Location, NPC[A].Direction);
            }
            else if(B == 6)
            {
                PlaySound(16);
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 16;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NewEffect(10, NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 16;
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10, NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                if(NPC[A].Type == 1 || NPC[A].Type == 244)
                    NewEffect(4, NPC[A].Location);
                else if(NPC[A].Type == 242 || NPC[A].Type == 243)
                    NewEffect(127, NPC[A].Location);
                else if(NPC[A].Type == 231)
                    NewEffect(117, NPC[A].Location);
                else if(NPC[A].Type == 235)
                    NewEffect(121, NPC[A].Location);
                else if(NPC[A].Type == 162)
                    NewEffect(86, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 164)
                    NewEffect(87, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 163)
                    NewEffect(85, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 229)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(115, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 236)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(122, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 230)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(116, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 232)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(118, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 233)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(119, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 234)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(120, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 237 || NPC[A].Type == 263)
                {
                    NewEffect(10, NPC[A].Location);
                    for(C = 1; C <= 50; C++)
                    {
                        tempLocation.Height = EffectHeight[80];
                        tempLocation.Width = EffectWidth[80];
                        tempLocation.SpeedX = 0;
                        tempLocation.SpeedY = 0;
                        tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width;
                        tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height;
                        NewEffect(80, tempLocation);
                        Effect[numEffects].Location.SpeedX = dRand() * 2 - 1 - NPC[A].Location.SpeedX * 0.3;
                        Effect[numEffects].Location.SpeedY = dRand() * 2 - 1 + NPC[A].Location.SpeedY * 0.3;
                        Effect[numEffects].Frame = iRand() % 3;
                    }
                }
                else
                    NewEffect(88, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == 203 || NPC[A].Type == 204 || NPC[A].Type == 205 || NPC[A].Type == 210)
        {
            PlaySound(65);
            NewEffect(108, NPC[A].Location);
        }
        else if(NPC[A].Type == 208)
        {
            PlaySound(67);
            for(C = 1; C <= 100; C++)
                NewEffect(111, newLoc(NPC[A].Location.X + dRand() * NPC[A].Location.Width - 16, NPC[A].Location.Y + dRand() * NPC[A].Location.Height - 16));
        }
        else if(NPC[A].Type == 209)
        {
            PlaySound(69);
            PlaySound(70);
            NewEffect(112, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == 38 || NPC[A].Type == 42 || NPC[A].Type == 43 || NPC[A].Type == 44 || NPC[A].Type == 206 || NPC[A].Type == 259)
        {
            PlaySound(9); // Shell hit sound
            NPC[A].Location.SpeedY = -10;
            if(NPC[A].Type == 38)
                NewEffect(94, NPC[A].Location, NPC[A].Direction);
            else if(NPC[A].Type == 42)
                NewEffect(93, NPC[A].Location, NPC[A].Direction);
            else if(NPC[A].Type == 43)
                NewEffect(92, NPC[A].Location, NPC[A].Direction);
            else if(NPC[A].Type == 206)
                NewEffect(110, NPC[A].Location);
            else if(NPC[A].Type == 259)
            {
                NewEffect(136, NPC[A].Location);
                Effect[numEffects].Frame = NPC[A].Frame;
                Effect[numEffects].Life = 100;
            }
            else
                NewEffect(91, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == 168) // bully
        {
            NPC[A].Location.SpeedY = -8;
            if(B == 6)
            {
                PlaySound(16);
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - EffectHeight[10];
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(10, NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9);
                NewEffect(89, NPC[A].Location, NPC[A].Direction);
            }




        // turnips
        }
        else if(NPCIsVeggie[NPC[A].Type])
        {
            NPC[A].Location.Y = NPC[A].Location.Y - (32 - NPC[A].Location.Height);
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
            NPC[A].Location.Height = 32;
            NPC[A].Location.Width = 32;
            if(NPC[A].Killed == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 32) // P Switch
        {
            if(B == 1)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + 2;
                NewEffect(81 , NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySound(9);
                NewEffect(10 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 238) // P Switch Time
        {
            if(B == 1)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + 2;
                NewEffect(123 , NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySound(9);
                NewEffect(10 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 239) // Red Switch
        {
            if(B == 1)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + 2;
                NewEffect(124 , NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySound(9);
                NewEffect(10 , NPC[A].Location);
            }
        }
        else if((NPC[A].Type == 84 || NPC[A].Type == 181) && B == 6) // lava only
        {
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32;
            NPC[A].Location.Height = 32;
            NPC[A].Location.Width = 32;
            PlaySound(16);
            NewEffect(10 , NPC[A].Location);
            if(NPC[A].NoLavaSplash == false)
                NewEffect(13 , NPC[A].Location);
        }
        else if(NPC[A].Type == 37 || NPC[A].Type == 179 || NPC[A].Type == 180) // thwomp
        {
            if(B == 3 || B == 4 || B == 2)
            {
                PlaySound(9); // Shell hit sound
                NPC[A].Location.SpeedY = -10;
                if(NPC[A].Type == 37)
                    NewEffect(90 , NPC[A].Location);
                else if(NPC[A].Type == 179)
                {
                    NPC[A].Location.Width = 64;
                    NPC[A].Location.Height = 64;
                    NPC[A].Location.X = NPC[A].Location.X - 8;
                    NPC[A].Location.Y = NPC[A].Location.Y - 8;
                    NewEffect(99 , NPC[A].Location);
                }
                else if(NPC[A].Type == 180)
                    NewEffect(98 , NPC[A].Location);
            }
            else if(B == 6)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(NPC[A].Type == 179)
            {
                PlaySound(9); // Shell hit sound
                NPC[A].Location.SpeedY = -10;
                NPC[A].Location.Width = 64;
                NPC[A].Location.Height = 64;
                NPC[A].Location.X = NPC[A].Location.X - 8;
                NPC[A].Location.Y = NPC[A].Location.Y - 8;
                NewEffect(99 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 136 || NPC[A].Type == 137)
        {
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9);
                NPC[A].Location.SpeedY = -11;
                NewEffect(72, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == 89) // SMB1 Goomba
        {
            if(B == 1)
                NewEffect(52 , NPC[A].Location);
            else if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(53 , NPC[A].Location);
            }
        // Zelda NPCs
        }
        else if(NPC[A].Type == 125 || NPCIsABot[NPC[A].Type] || NPC[A].Type == 255)
        {
            PlaySound(53);
            NewEffect(63 , NPC[A].Location);
        }
        else if(NPC[A].Type == 133)
        {
            NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX * 0.3;
            NewEffect(68 , NPC[A].Location);
        }
        else if(NPC[A].Type == 59 || NPC[A].Type == 61 || NPC[A].Type == 63 || NPC[A].Type == 65) // switch goombas
        {
            if(B == 1)
            {
                NewEffect(NPC[A].Type - 22, NPC[A].Location);
            }
            else if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(NPC[A].Type - 21, NPC[A].Location);
            }

        }
        else if(NPC[A].Type == 267 || NPC[A].Type == 268 || NPC[A].Type == 280 || NPC[A].Type == 281) // larry koopa
        {
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(NPC[A].Type == 280 || NPC[A].Type == 281)
                NewEffect(143 , NPC[A].Location);
            else
                NewEffect(140 , NPC[A].Location);

        }
        else if(NPC[A].Type == 201 || NPC[A].Type == 262) // wart, smb2 bosses
        {
            if(NPC[A].Type == 262 && NPC[A].Killed != 3 && NPC[A].Killed != 6 && NPC[A].Killed != 10)
                PlaySound(41);
            // If B <> 6 Then MoreScore NPCScore(.Type), .Location
            if(B == 6)
            {
                PlaySound(16);
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + 24;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else
            {
                NPC[A].Location.SpeedY = -7;
                if(NPC[A].Type == 262)
                {
                    NewEffect(138, NPC[A].Location, NPC[A].Direction);
                    Effect[numEffects].Location.SpeedX = 0;
                    Effect[numEffects].Location.SpeedY = -8;
                }
                else
                    NewEffect(106, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == 200) // king koopa
        {
            if(B == 6)
            {
                PlaySound(16);
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + 24;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else
            {
                // .Location.Width = .Location.Width + 2
                // .Location.X = .Location.X - 1
                NewEffect(105, NPC[A].Location, NPC[A].Direction);
            }
            PlaySound(44);
        }
        else if(NPC[A].Type == 86) // bowser
        {
            if(B == 6)
            {
                PlaySound(16);
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + 24;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else
            {
                NPC[A].Location.Width = NPC[A].Location.Width + 2;
                NPC[A].Location.X = NPC[A].Location.X - 1;
                NewEffect(50, NPC[A].Location, NPC[A].Direction);
            }
            PlaySound(44);
            if(NPC[A].Legacy == true)
            {
                tempBool = false;
                for(B = 1; B <= numNPCs; B++)
                {
                    if(B != A && NPC[B].Type == 86)
                        tempBool = true;
                }
                if(tempBool == false)
                {
                    LevelMacroCounter = 0;
                    LevelMacro = 5;
                }
            }
        }
        else if(NPC[A].Type == 2 || NPC[A].Type == 3) // Red goomba
        {
            if(B == 1)
                NewEffect(6 , NPC[A].Location);
            else if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(7 , NPC[A].Location);
            }
        }
        else if(NPCIsToad[NPC[A].Type] == true) // toad
        {
            if(B != 2)
            {
                if(B == 6)
                {
                    NPC[A].Location.Y = NPC[A].Location.Y + (NPC[A].Location.Height - 32);
                    PlaySound(16);
                    NewEffect(10 , NPC[A].Location);
                    if(NPC[A].NoLavaSplash == false)
                        NewEffect(13 , NPC[A].Location);
                }
                else
                {
                    PlaySound(9); // Shell hit sound
                    NewEffect(48, NPC[A].Location, NPC[A].Direction);
                }
            }
        }
        else if(NPC[A].Type == 96) // yoshi egg
        {
            C = NPC[A].Special;
            CharStuff(A, true);
            if(NPC[A].Special == 287)
                NPC[A].Special = RandomBonus();
            NewEffect(56, NPC[A].Location, 1, static_cast<int>(floor(static_cast<double>(NPC[A].Special))));
            if(C == 98)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 3;
            else if(C == 99)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 5;
            else if(C == 100)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 7;
            else if(C == 148)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 9;
            else if(C == 149)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 11;
            else if(C == 150)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 13;
            else if(C == 228)
                Effect[numEffects].Frame = Effect[numEffects].Frame + 15;
        }
        else if(NPC[A].Type == 71) // giagnormous goomba
        {
            if(B == 1)
                NewEffect(45 , NPC[A].Location);
            else if(B == 6)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(46 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 27) // Grey goomba
        {
            if(B == 1)
                NewEffect(23 , NPC[A].Location);
            else if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                if(!(B == 5))
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(22 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 55) // nekkid koopa
        {
            if(B == 1)
                NewEffect(35 , NPC[A].Location);
            else if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                if(!(B == 5))
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(36, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == 189) // Dry Bones
        {
            NPC[A].Location.Width = 48;
            NPC[A].Location.X = NPC[A].Location.X - 8;
            if(B == 6)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else
            {
                PlaySound(9);
                NewEffect(97, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type >= 117 && NPC[A].Type <= 120) // SMW Beach Koopas
        {
            if(B == 1)
            {
                NewEffect(62 , NPC[A].Location);
                Effect[numEffects].Frame = NPC[A].Type - 117;
            }
            else if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9);
                if(!(B == 5))
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(61, NPC[A].Location, NPC[A].Direction);
                Effect[numEffects].Frame = (NPC[A].Type - 117) * 4;
                if(NPC[A].Direction == 1)
                    Effect[numEffects].Frame = Effect[numEffects].Frame + 2;
            }
        }
        else if(NPC[A].Type == 4 || NPC[A].Type == 5 || NPC[A].Type == 76) // Green Koopa
        {
             NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(8 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 207) // spike top
        {
            if(B == 6)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(109, NPC[A].Location, static_cast<double>(NPC[A].Frame));
            }
        }
        else if(NPC[A].Type == 172 || NPC[A].Type == 173 || NPC[A].Type == 176) // smb1 Green Koopa
        {
             NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(95 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 174 || NPC[A].Type == 175 || NPC[A].Type == 177) // smb1 red Koopa
        {
             NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(96 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 241)
        {
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else
                NewEffect(125 , NPC[A].Location);
        }
        else if(NPC[A].Type == 72 || NPC[A].Type == 73) // giant Green Koopa
        {
             NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(47 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 39) // Birdo
        {
            PlaySound(39);
            if(NPC[A].Legacy == true && LevelEditor == false)
            {
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == 39 && C != A)
                        DontSpawnExit = true;
                }
                if(DontSpawnExit == false)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 41;
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.X = NPC[A].Location.X;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                    NPC[numNPCs].Location.SpeedY = -6;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Frame = 0;
                    CheckSectionNPC(numNPCs);
                    PlaySound(41);
                }
                else
                {
                    for(C = 1; C <= numNPCs; C++)
                    {
                        if(NPC[C].Type == 39 && NPC[C].Active == true && C != A)
                            DontResetMusic = true;
                    }
                    if(DontResetMusic == false)
                    {
                        bgMusic[NPC[A].Section] = bgMusicREAL[NPC[A].Section];
                        StopMusic();
                        StartMusic(NPC[A].Section);
                    }
                }
            }
             NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height / 2.0 + 32;
             NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0 + 20;
             NewEffect(29, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == 40) // Egg
        {
            if(NPC[A].Special == 1)
                NPC[A].Location.SpeedY = -5.1;
            NewEffect(28 , NPC[A].Location);
        }
        else if(NPC[A].Type == 6 || NPC[A].Type == 7 || NPC[A].Type == 161) // Red Koopa
        {
             NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[9] / 2.0;
             NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[9] / 2.0;
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                NewEffect(9 , NPC[A].Location);
            }

        }
        else if((NPC[A].Type >= 109 && NPC[A].Type <= 116) || (NPC[A].Type >= 121 && NPC[A].Type <= 124)) // SMW Koopas
        {
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9);
                NewEffect(60 , NPC[A].Location);
                if(NPC[A].Type <= 112)
                    Effect[numEffects].Frame = NPC[A].Type - 109;
                else if(NPC[A].Type <= 116)
                    Effect[numEffects].Frame = NPC[A].Type - 113;
                else
                    Effect[numEffects].Frame = NPC[A].Type - 121;
            }
        }
        else if(NPC[A].Type == 23 || NPC[A].Type == 24 || NPC[A].Type == 36 || NPC[A].Type == 53 || NPC[A].Type == 54 || NPC[A].Type == 285 || NPC[A].Type == 286) // Hard thing / Spiney
        {
             NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[9] / 2.0;
             NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[9] / 2.0;
            if(B == 6)
            {
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                PlaySound(9); // Shell hit sound
                if(NPC[A].Type == 36)
                    NewEffect(27, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 285 || NPC[A].Type == 286)
                    NewEffect(146, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 53)
                    NewEffect(33 , NPC[A].Location);
                else if(NPC[A].Type == 54)
                    NewEffect(34 , NPC[A].Location);
                else
                    NewEffect(19 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 30) // Hammer
        {
            if(B == 3)
            {
                PlaySound(9);
                NewEffect(10 , NPC[A].Location);
            }
        }
        else if(NPC[A].Type == 256 || NPC[A].Type == 257)
        {
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            tempLocation = NPC[A].Location;
            if(NPC[A].Type == 257)
            {
                int npcH = NPC[A].Location.Height;
                for(C = 0; C <= npcH; C += 32)
                {
                    if(NPC[A].Location.Height - C > 16)
                    {
                        tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32 - C;
                        NewEffect(10, tempLocation);
                    }
                }
            }
            else
            {
                int npcH = NPC[A].Location.Height;
                for(C = 0; C <= npcH; C += 32)
                {
                    if(NPC[A].Location.Height - C > 16)
                    {
                        tempLocation.Y = NPC[A].Location.Y + C;
                        NewEffect(10, tempLocation);
                    }
                }
            }
            PlaySound(9); // Shell hit sound
        }
        else if(NPC[A].Type == 8 || NPC[A].Type == 275 || NPC[A].Type == 93 || NPC[A].Type == 12 || NPC[A].Type == 51 || NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 37 || NPC[A].Type == 38 || NPC[A].Type == 42 || NPC[A].Type == 43 || NPC[A].Type == 44 || NPC[A].Type == 245 || NPC[A].Type == 270) // Piranha Plant / Fireball
        {
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
            NewEffect(10 , NPC[A].Location);
            if(B == 8)
                PlaySound(36);
            else
                PlaySound(9); // Shell hit sound
        }
        else if(NPC[A].Type == 13 || NPC[A].Type == 108 || NPC[A].Type == 265) // Small Fireball / Yoshi Fireball
        {
            if(B == 6)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[9] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[9] / 2.0;
                NPC[A].Location.Width = 32;
                PlaySound(16);
                NewEffect(10, NPC[A].Location, 1, 0, NPC[A].Shadow);
                NPC[A].Location.X = NPC[A].Location.X + 2;
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else
            {
                for(C = 1; C <= 10; C++)
                {
                    if(NPC[A].Type == 265)
                        NewEffect(139, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
                    else
                        NewEffect(77, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
                    Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5 + NPC[A].Location.SpeedX * 0.1;
                    Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5 - NPC[A].Location.SpeedY * 0.1;
                }
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                if((NPC[A].Type == 13 && NPC[A].Special == 5) || NPC[A].Type == 108)
                    NewEffect(10, NPC[A].Location, 1, 0, NPC[A].Shadow);
                else
                    NewEffect(131, NPC[A].Location, 1, 0, NPC[A].Shadow);
            }
        }
        else if(NPC[A].Type == 15) // Big Koopa
        {
            NPC[A].Location.Y = NPC[A].Location.Y - (NPCHeight[NPC[A].Type] - NPC[A].Location.Height);
            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
            if(NPC[A].Legacy == true)
            {
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Type == 15 && NPC[B].Killed == 0 && B != A)
                        DontSpawnExit = true;
                }
                if(LevelEditor == true)
                    DontSpawnExit = true;
                if(DontSpawnExit == false)
                    NewEffect(14, NPC[A].Location, 1, 16);
                else
                {
                    NewEffect(14 , NPC[A].Location);
                    for(B = 1; B <= numNPCs; B++)
                    {
                        if(NPC[B].Type == 15 && NPC[B].Active == true && B != A && NPC[B].Killed == 0)
                            DontResetMusic = true;
                    }
                    if(DontResetMusic == false)
                    {
                        bgMusic[NPC[A].Section] = bgMusicREAL[NPC[A].Section];
                        StopMusic();
                        StartMusic(Player[1].Section);
                    }
                }
            }
            else
                NewEffect(14 , NPC[A].Location);
         }
         else if(NPC[A].Type == 17 || NPC[A].Type == 18) // Bullet Bills
         {
             NPC[A].Location.SpeedX = NPC[A].Location.SpeedX / 2;
            if(B == 1)
                 NPC[A].Location.SpeedX = 0.0001 * NPC[A].Direction;
            else if(B == 5)
                 NPC[A].Location.SpeedX = 3 * NPC[A].Direction;
            else
                 NPC[A].Location.SpeedX = NPC[A].Location.SpeedX / 2;
            if(!(B == 1))
                NPC[A].Location.SpeedY = -9;
            if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                if(B == 3 || B == 2)
                    PlaySound(9);
                if(NPC[A].Type == 17)
                    NewEffect(15, NPC[A].Location, NPC[A].Direction);
                else
                    NewEffect(16, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == 247 || NPC[A].Type == 19 || NPC[A].Type == 20 || NPC[A].Type == 25 || NPC[A].Type == 28 || NPC[A].Type == 29 || NPC[A].Type == 47 || NPC[A].Type == 284 || NPC[A].Type == 48 || NPC[A].Type == 77 || NPC[A].Type == 271 || (NPC[A].Type >= 129 && NPC[A].Type <= 132) || NPC[A].Type == 158 || NPC[A].Type == 261 || NPC[A].Type == 272) // Misc Things
        {
            if(B == 6)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + (NPC[A].Location.Height - 32);
                PlaySound(16);
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
            else
            {
                if(NPC[A].Type >= 129 && NPC[A].Type <= 132)
                    NewEffect(64 + NPC[A].Type - 129, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 19)
                    NewEffect(17, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 20)
                    NewEffect(18, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 261)
                    NewEffect(137, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 25)
                    NewEffect(20, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 28)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(NPC[A].Killed == 1)
                        NPC[A].Location.SpeedY = 0;
                    NewEffect(24, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 158)
                    NewEffect(83, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == 247)
                    NewEffect(128 , NPC[A].Location);
                else if(NPC[A].Type == 272)
                    NewEffect(142 , NPC[A].Location);
                else if(NPC[A].Type == 77)
                {
                    if(B == 1)
                    {
                         NPC[A].Location.SpeedY = 0;
                         NPC[A].Location.SpeedX = 0;
                        PlaySound(2);
                    }
                    else
                    {
                         NPC[A].Location.SpeedY = -11;
                        PlaySound(9);
                    }
                    NewEffect(49, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 271)
                {
                    if(B == 1)
                    {
                         NPC[A].Location.SpeedY = 0;
                         NPC[A].Location.SpeedX = 0;
                    }
                    else
                    {
                         NPC[A].Location.SpeedY = -11;
                        PlaySound(9);
                    }
                    NewEffect(141, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 29)
                {
                    if(B == 1)
                    {
                         NPC[A].Location.SpeedY = 0;
                         NPC[A].Location.SpeedX = 0;
                        PlaySound(2);
                    }
                    else
                    {
                         NPC[A].Location.SpeedY = -11;
                        PlaySound(9);
                    }
                    NewEffect(25, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == 48)
                    NewEffect(31 , NPC[A].Location);
                else if(NPC[A].Type == 47 || NPC[A].Type == 284)
                {
                    if(NPC[A].Location.SpeedX > 4)
                        NPC[A].Location.SpeedX = 4;
                    if(NPC[A].Location.SpeedX < -4)
                        NPC[A].Location.SpeedX = -4;
                    if(B == 1)
                    {
                         NPC[A].Location.SpeedY = 0;
                         NPC[A].Location.SpeedX = 0;
                        PlaySound(2);
                    }
                    else
                    {
                         NPC[A].Location.SpeedY = -11;
                        PlaySound(9);
                    }
                    if(NPC[A].Type == 284)
                        NewEffect(145 , NPC[A].Location);
                    else
                    {
                        NPC[A].Location.Y = NPC[A].Location.Y - 14;
                        NewEffect(32 , NPC[A].Location);
                    }
                }
                if(!(NPC[A].Type == 29) && !(NPC[A].Type == 47) && !(NPC[A].Type == 77))
                    PlaySound(9);
            }
        }
        else if(NPC[A].Type == 45) // ice block
        {
            if(B == 6)
            {
                NewEffect(10 , NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(13 , NPC[A].Location);
                PlaySound(16);
            }
            else
            {
                NewEffect(30 , NPC[A].Location);
                PlaySound(4);
            }
        }
        else if(NPCIsABonus[NPC[A].Type]) // NPC is a bonus
        {
            if(B == 3 || B == 4 || B == 5)
            {
                if(!NPCIsACoin[NPC[A].Type] || LevelEditor || TestLevel) // Shell hit sound
                    PlaySound(9);
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
            }
            else if(B == 6)
            {
                PlaySound(16);
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                if(NPC[A].NoLavaSplash == false)
                    NewEffect(13 , NPC[A].Location);
            }
        }
        else if(LevelEditor || MagicHand)
        {
            if(!(NPC[A].Type == 32 && B == 1))
            {
                 NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10 , NPC[A].Location);
                PlaySound(36);
            }
        }
    }
    if(BattleMode == true)
        NPC[A].RespawnDelay = 65 * 30;

    if(NPC[A].AttLayer != "")
    {
        for(C = 1; C <= maxLayers; C++)
        {
            if(NPC[A].AttLayer == Layer[C].Name)
            {
                Layer[C].SpeedX = 0;
                Layer[C].SpeedY = 0;
            }
        }
    }

    if((!GameMenu && !BattleMode) || NPC[A].DefaultType == 0)
    {
        for(B = 1; B <= numPlayers; B++) // Tell the player to stop standing on me because im dead kthnx
        {
            if(Player[B].StandingOnNPC == A)
            {
                Player[B].StandingOnNPC = 0;
                if(NPC[A].Type != 56)
                    Player[B].Location.SpeedY = NPC[A].Location.SpeedY;
            }
            else if(Player[B].StandingOnNPC == numNPCs)
                Player[B].StandingOnNPC = A;
            if(Player[B].YoshiNPC == numNPCs)
                Player[B].YoshiNPC = A;
            if(fEqual(Player[B].VineNPC, numNPCs))
                Player[B].VineNPC = A;
        }
        NPC[A] = NPC[numNPCs];
        NPC[numNPCs] = blankNPC;
        numNPCs = numNPCs - 1;
        if(NPC[A].HoldingPlayer > 0)
        {
            Player[NPC[A].HoldingPlayer].HoldingNPC = A;
//            if(nPlay.Online == true && nPlay.Mode == 1)
//                Netplay::sendData "K" + std::to_string(C) + "|" + NPC[A].Type + "|" + NPC[A].Location.X + "|" + NPC[A].Location.Y + "|" + std::to_string(NPC[A].Location.Width) + "|" + std::to_string(NPC[A].Location.Height) + "|" + NPC[A].Location.SpeedX + "|" + NPC[A].Location.SpeedY + "|" + NPC[A].Section + "|" + NPC[A].TimeLeft + "|" + NPC[A].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[A].Projectile)))) + "|" + NPC[A].Special + "|" + NPC[A].Special2 + "|" + NPC[A].Special3 + "|" + NPC[A].Special4 + "|" + NPC[A].Special5 + "|" + NPC[A].Effect + LB + "1n" + NPC[A].HoldingPlayer + "|" + std::to_string(A) + "|" + NPC[A].Type + LB;
        }

        if(NPC[A].Effect == 5 || NPC[A].Effect == 6)
            Player[NPC[A].Effect2].YoshiNPC = A;
        if(NPC[A].Type == 49 && NPC[A].Special == 1)
        {
            for(C = 1; C <= numNPCs; C++)
            {
                if(NPC[C].Type == 50 && NPC[C].Special2 == numNPCs + 1)
                    NPC[C].Special2 = A;
            }
        }
        else if(NPC[A].Type == 49 && NPC[A].Special2 > 0)
            NPC[NPC[A].Special2].Special2 = A;
        else if(NPC[A].Type == 50 && NPC[A].Special2 > 0)
            NPC[NPC[A].Special2].Special2 = A;
    }
    else
    {
        Deactivate(A);
    }
}
