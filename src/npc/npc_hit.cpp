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
#include "../collision.h"
#include "../effect.h"
#include "../editor.h"
#include "../game_main.h"
#include "../blocks.h"
#include "../graphics.h"

#include <Logger/logger.h>

void NPCHit(int A, int B, int C)
{
    NPC_t tempNPC;
    Location_t tempLocation;
    NPC_t oldNPC = NPC[A];
    // ------+  HIT CODES  +-------
    // B = 1      Jumped on by a player (or kicked)
    // B = 2      Hit by a shaking block
    // B = 3      Hit by projectile
    // B = 4      Hit something as a projectile
    // B = 5      Hit something while being held
    // B = 6      Touched a lava block
    // B = 7      Hit by a tail
    // B = 8      Stomped by Boot
    // B = 9      Fell of a cliff
    // B = 10     Link stab
    // Frost Bolt check
    if(B == 3 && NPC[A].Type != 263 && NPC[A].Type != 265)
    {
        if(NPC[C].Type == 265 && NPC[A].Location.Width > 8 && NPC[A].Location.Height > 8)
        {
            if(NPC[A].Type == 283)
                NPCHit(A, 3, B);
            if(NPCNoIceBall[NPC[A].Type] == true || NPC[A].Location.Width > 128 || NPC[A].Location.Height > 128)
                return;
            if(NPC[A].Type == 3)
            {
                NPC[A].Type = 2;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 4)
            {
                NPC[A].Type = 5;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 6 || NPC[A].Type == 161)
            {
                NPC[A].Type = 7;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 72)
            {
                NPC[A].Type = 73;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 76)
            {
                NPC[A].Type = 5;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 109 || NPC[A].Type == 121)
            {
                NPC[A].Type = 113;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 110 || NPC[A].Type == 122)
            {
                NPC[A].Type = 114;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 111 || NPC[A].Type == 123)
            {
                NPC[A].Type = 115;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 112 || NPC[A].Type == 124)
            {
                NPC[A].Type = 116;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 175 || NPC[A].Type == 177)
            {
                NPC[A].Type = 174;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 173 || NPC[A].Type == 176)
            {
                NPC[A].Type = 172;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 243)
            {
                NPC[A].Type = 242;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == 244)
            {
                NPC[A].Type = 1;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            NPC[A].Special = NPC[A].Type;
            NPC[A].Special2 = NPC[A].Frame;
            // If .Type = 52 Or .Type = 51 Then
            NPC[A].Special3 = 1;
            NPC[A].Location.SpeedY = 0;
            // End If
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Type == 52)
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Location.Width = static_cast<int>(floor(static_cast<double>(NPC[A].Location.Width))) - 0.01;
                else
                    NPC[A].Location.X = static_cast<int>(floor(static_cast<double>(NPC[A].Location.X))) + 0.01;
            }
            NPC[A].Location.Height = static_cast<int>(floor(static_cast<double>(NPC[A].Location.Height)));
            NPC[A].Type = 263;
            NPC[A].BeltSpeed = 0;
            NPC[A].Projectile = false;
            NPC[A].RealSpeedX = 0;

            NewEffect(10, NPC[A].Location);
            for(C = 1; C <= 20; C++)
            {
                tempLocation.Height = EffectHeight[80];
                tempLocation.Width = EffectWidth[80];
                tempLocation.SpeedX = 0;
                tempLocation.SpeedY = 0;
                tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                NewEffect(80, tempLocation);
                Effect[numEffects].Location.SpeedX = dRand() * 2 - 1;
                Effect[numEffects].Location.SpeedY = dRand() * 2 - 1;
                Effect[numEffects].Frame = iRand() % 3;
            }

            PlaySound(9);
            // NPCHit C, 3, C
            return;
        }
    }

    // Online code
    //    if(nPlay.Online == true)
    //    {
    //        if(B == 1 || B == 7 || B == 8)
    //        {
    //            if(C == nPlay.MySlot + 1 || nPlay.Allow == true || C <= 0)
    //            {
    //                if(C == nPlay.MySlot + 1)
    //                    Netplay::sendData "2d" + std::to_string(A) + "|" + std::to_string(B) + "|" + std::to_string(C) + "|" + NPC[A].Type + LB;
    //            }
    //            else
    //                return;
    //        }
    //    }

    if(NPC[A].Active == false)
        return;


    // Safety
    StopHit = StopHit + 1;
    if(NPC[A].Killed > 0)
        return;
    if(B == 3 || B == 4)
    {
        if(NPC[C].Generator == true)
            return;
    }
    if((NPC[A].Type == 235 || NPC[A].Type == 231 || NPCIsCheep[NPC[A].Type] == true) && B == 1)
    {
        if(Player[C].Wet > 0)
            return;
    }
    if(NPC[A].Inert == true || StopHit > 2 || NPC[A].Immune > 0 || NPC[A].Killed > 0 || NPC[A].Effect == 208 || NPC[A].Generator == true)
        return;
    if(B == 6 && NPC[A].Killed == 6)
        return;
    if(B == 3 || B == 4 || B == 5) // Things immune to fire
    {
        if(NPC[C].Type == 13)
        {
            if(NPCNoFireBall[NPC[A].Type])
                return;
        }
    }


    if(B == 1 && NPCJumpHurt[NPC[A].Type] && NPC[A].Type != 283) // Things that don't die from jumping
        return;

    if(B == 10 && NPC[A].Type == 31)
    {
        if(Player[C].Character == 5 && Player[C].HasKey == false)
        {
            NPC[A].Killed = 9;
            Player[C].HasKey = true;
            PlaySound(84);
            return;
        }
    }

    // Yoshi Ice
    if(NPC[A].Type == 237)
    {
        if(B != 1 && B != 7 && B != 8 && B != 4)
            NPC[A].Killed = B;
        // Frozen NPC
    }
    else if(NPC[A].Type == 263)
    {
        if(B == 3 && NPC[C].Type == 13)
        {

            NPC[A].Type = NPC[A].Special;
            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Direction = 1;
            else if(NPC[A].Location.SpeedX < 0)
                NPC[A].Direction = -1;
            else
                NPC[A].Direction = NPC[A].DefaultDirection;
            NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            NPC[A].Special = NPC[A].DefaultSpecial;
            NPC[A].Special2 = 0;
            NPC[A].Special3 = 0;
            NPC[A].Special4 = 0;
            NPC[A].Special5 = 0;
            NPC[A].Special6 = 0;


        }
        else if(B == 3 || B == 5)
        {
            if(A == C || B == 6 || B == 5)
                NPC[A].Killed = B;
            if(B == 3 && NPC[C].Type == NPC[A].Type)
                NPC[A].Killed = B;
        }
        else if(B == 10 || B == 2)
            NPC[A].Killed = B;


        // Things that link can move with his sword
    }
    else if(B == 10 && NPC[A].Type == 134 && NPC[A].CantHurt == 0 && NPC[A].Projectile == false) // link picks up bombs
    {
        if(Player[C].Bombs < 9)
            Player[C].Bombs = Player[C].Bombs + 1;
        // .Location.X = .Location.X + .Location.Width / 2 - EffectWidth(10) / 2
        // .Location.Y = .Location.Y + .Location.Height / 2 - EffectHeight(10) / 2
        // NewEffect 10, .Location
        NPC[A].Killed = 9;
        PlaySound(79);
    }
    else if(B == 10 && ((NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 26 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 241 || NPC[A].Type == 96 || NPC[A].Type == 22 || NPC[A].Type == 134))
    {
        PlaySound(9);
        NPC[A].Bouce = true;
        if((NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 22)
        {
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Location.SpeedY = -5;
        }
        else
        {
            NPC[A].Location.SpeedX = 4 * Player[C].Direction;
            NPC[A].Location.SpeedY = -4;
        }
        NPC[A].Projectile = true;
        NPC[A].CantHurt = 30;
        NPC[A].CantHurtPlayer = C;
        NPC[A].BattleOwner = C;
        if(NPC[A].Type == 22)
            NPC[A].Direction = Player[C].Direction;
        // SMB2 Grass
    }
    else if(B == 10 && (NPC[A].Type == 91 || NPCIsVeggie[NPC[A].Type]))
    {
        if(NPC[A].Type == 91)
        {
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
            NPC[A].Type = NPC[A].Special;
        }
        PlaySound(88);
        NewEffect(63, NPC[A].Location);
        if(NPC[A].Type == 17)
        {
            PlaySound(22);
            NPC[A].Location.SpeedX = 5 * Player[C].Direction;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
        }
        NPC[A].Direction = Player[C].Direction;
        NPC[A].Generator = false;
        NPC[A].Frame = 0;
        NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
        CharStuff(A);
        NPC[A].Special = 0;
        if(NPCIsYoshi[NPC[A].Type])
        {
            NPC[A].Special = NPC[A].Type;
            NPC[A].Type = 96;
        }
        if(!(NPC[A].Type == 21 || NPC[A].Type == 22 || NPC[A].Type == 26 || NPC[A].Type == 31 || NPC[A].Type == 32 || NPC[A].Type == 35 || NPC[A].Type == 191 || NPC[A].Type == 193 || NPC[A].Type == 49 || NPCIsAnExit[NPC[A].Type]))
        {
            if(BattleMode == false)
                NPC[A].DefaultType = 0;
        }
        NPC[A].Location.Height = NPCHeight[NPC[A].Type];
        NPC[A].Location.Width = NPCWidth[NPC[A].Type];
        NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
        NPC[A].Location.SpeedX = (3 + dRand() * 1) * Player[C].Direction;
        if(NPC[A].Type == 17)
            NPC[A].Location.SpeedX = 5 * Player[C].Direction;
        NPC[A].Location.SpeedY = -4;
        NPC[A].CantHurtPlayer = C;
        NPC[A].CantHurt = 30;
        if(NPCIsVeggie[NPC[A].Type] || NPC[A].Type == 29 || NPC[A].Type == 251 || NPC[A].Type == 252)
        {
            if(NPC[A].Type != 252)
                NPC[A].Type = 251;
            NPC[A].Location.SpeedX = (1 + dRand() * 0.5) * Player[C].Direction;
            NPC[A].Location.SpeedY = -5;
            if(dRand() * 20 < 3)
                NPC[A].Type = 252;
            if(dRand() * 40 < 3)
                NPC[A].Type = 253;
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
            NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
        }
        if(NPCIsAShell[NPC[A].Type])
            NPC[A].Location.SpeedX = Physics.NPCShellSpeed * Player[C].Direction;
        NPCFrames(A);
        if(NPCIsACoin[NPC[A].Type] == false)
            NPC[A].Projectile = true;
        else
            NPC[A].Special = 1;
        NPC[A].Immune = 10;
        if(NPC[A].Type == 134)
        {
            // .Location.SpeedX = 5 * Player(C).Direction + Player(C).Location.SpeedX
            // .Location.SpeedY = -5
            NPC[A].Projectile = false;
        }
        if(Player[C].StandingOnNPC == A)
            Player[C].StandingOnNPC = 0;
        // bubble
    }
    else if(NPC[A].Type == 283)
        NPC[A].Special3 = 1;

    // Larry Koopa
    else if(NPC[A].Type == 267 || NPC[A].Type == 280)
    {
        if(B != 7)
            NPC[A].Immune = 10;
        if(B == 1 || B == 2 || B == 8)
        {
            NPC[A].Damage += 5;
            PlaySound(2);
            NPC[A].Special = 5;
        }
        else if(B == 3 || B == 4 || B == 5)
        {
            if(NPC[C].Type == 13 || NPC[C].Type == 108)
            {
                NPC[A].Damage = NPC[A].Damage + 1;
                PlaySound(9);
            }
            else
            {
                NPCHit(C, 3, B);
                NPC[A].Special = 5;
                NPC[A].Damage += 5;
                PlaySound(39);
            }
        }
        else if(B == 10)
        {
            NPC[A].Damage += 2;
            PlaySound(89);
        }
        else if(B == 6)
            NPC[A].Killed = B;
        if(NPC[A].Damage >= 15)
            NPC[A].Killed = B;
        else if(fEqual(NPC[A].Special, 5) && !(NPC[A].Type == 268 || NPC[A].Type == 281))
        {
            NPC[A].Special = 0;
            NPC[A].Special2 = 0;
            NPC[A].Special3 = 0;
            NPC[A].Special4 = 0;
            NPC[A].Special5 = 0;
            NPC[A].Special6 = 0;
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
            if(NPC[A].Type == 267)
                NPC[A].Type = 268;
            else
                NPC[A].Type = 281;
            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
            NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
            oldNPC = NPC[A];
        }

        // Larry Koop Shell
    }
    else if(NPC[A].Type == 268 || NPC[A].Type == 281)
    {
        if(B != 7 && B != 1 && B != 2 && B != 8)
            NPC[A].Immune = 10;
        if(B == 1 || B == 2 || B == 8)
        {
            if(B == 1 || B == 8)
            {

                if(Player[C].Location.X + Player[C].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                    Player[C].Location.SpeedX = Player[C].Location.SpeedX - 3;
                else
                    Player[C].Location.SpeedX = Player[C].Location.SpeedX + 3;

            }
            PlaySound(2);
        }
        else if(B == 3 || B == 4 || B == 5)
        {
            if(NPC[C].Type == 13 || NPC[C].Type == 108)
            {
                NPC[A].Damage = NPC[A].Damage + 1;
                PlaySound(9);
            }
            else
            {
                NPCHit(C, 3, B);
                NPC[A].Special = 5;
                NPC[A].Damage = NPC[A].Damage + 5;
                PlaySound(39);
            }
        }
        else if(B == 10)
        {
            NPC[A].Damage = NPC[A].Damage + 2;
            PlaySound(89);
        }
        else if(B == 6)
            NPC[A].Killed = B;
        if(NPC[A].Damage >= 15)
            NPC[A].Killed = B;
        // Zelda 2 Locked Door
    }
    else if(NPC[A].Type == 255)
    {
        if(B == 10)
        {
            if(Player[C].HasKey == true)
            {
                Player[C].HasKey = false;
                NPC[A].Killed = 3;
            }
        }
        // Goomba / Nekkid Koopa
    }
    else if(NPC[A].Type == 1 || NPC[A].Type == 2 || NPC[A].Type == 242 || NPC[A].Type == 27 ||
            NPC[A].Type == 55 || NPC[A].Type == 59 || NPC[A].Type == 61 || NPC[A].Type == 63 ||
            NPC[A].Type == 65 || NPC[A].Type == 71 || NPC[A].Type == 77 || NPC[A].Type == 271 ||
            NPC[A].Type == 89 || (NPC[A].Type >= 117 && NPC[A].Type <= 120) || NPC[A].Type == 162 ||
            NPC[A].Type == 163 || NPC[A].Type == 229 || NPC[A].Type == 236 || NPC[A].Type == 230 ||
            NPC[A].Type == 232 || NPC[A].Type == 233 || NPC[A].Type == 234)
    {
        if(B == 1)
        {
            if(NPC[A].Type == 162)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[A].Location.Height = 32;
                NPC[A].Type = 163;
                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                PlaySound(2);
            }
            else if(NPC[A].Type != 234)
                NPC[A].Killed = B;
        }
        else if(NPC[A].Type >= 117 && NPC[A].Type <= 120)
        {
            if(B == 3 && NPC[A].CantHurt > 0)
            {
            }
            else
                NPC[A].Killed = B;
        }
        else
        {
            if(NPC[A].Type == 234 && B == 3)
            {
                if(NPC[C].Type != 13 && NPC[C].Type != 108)
                    NPC[A].Killed = B;
            }
            else
                NPC[A].Killed = B;
        }
        // Mother Brain
    }
    else if(NPC[A].Type == 209)
    {
        int D = 0;
        bool tempBool;
        tempBool = false;
        for(D = 1; D <= numNPCs; D++)
        {
            if(NPC[D].Type == 208)
            {
                if(NPC[D].Active == true)
                {
                    if(NPC[D].Section == NPC[A].Section)
                    {
                        if(CheckCollision(NPC[A].Location, NPC[D].Location))
                        {
                            tempBool = true;
                            NPC[A].Immune = 65;
                            break;
                        }
                    }
                }
            }
        }
        if(tempBool == false)
        {
            if(B == 3 || B == 10)
            {
                if(NPC[C].Type == 13)
                {
                }
                else
                {
                    if(NPC[C].Type == 171)
                        NPC[A].Immune = 60;
                    else
                        NPC[A].Immune = 20;
                    NPC[A].Special = 1;
                    PlaySound(68);
                    NPC[A].Damage = NPC[A].Damage + 1;
                    if(NPC[A].Damage >= 10)
                        NPC[A].Killed = 3;
                }
            }
        }
        // Metroid Cherrio
    }
    else if(NPC[A].Type == 210)
    {
        if(B == 3 || B == 4 || B == 5 || B == 7 || B == 9 || B == 10)
            NPC[A].Killed = B;
        // Metroid Glass
    }
    else if(NPC[A].Type == 208)
    {
        if(B == 3)
        {
            PlaySound(64);
            if(NPC[C].Type == 13)
            {
            }
            else
            {
                if(NPC[C].Type == 171)
                    NPC[A].Immune = 60;
                else
                    NPC[A].Immune = 20;
                NPC[A].Damage = NPC[A].Damage + 3;
            }
        }
        else if(B == 10)
        {
            NPC[A].Damage = NPC[A].Damage + 3;
            NPC[A].Immune = 10;
        }
        if(NPC[A].Damage >= 15)
            NPC[A].Killed = B;
        // Metroid Floating Things
    }
    else if(NPC[A].Type == 203 || NPC[A].Type == 204)
    {
        if(B == 3)
        {
            if(NPC[C].Type != 13 && NPC[C].Type != 108)
                NPC[A].Killed = B;
        }
        else if(B == 2 || B == 4 || B == 5 || B == 6 || B == 9 || B == 10)
            NPC[A].Killed = B;
        // Spike Top
    }
    else if(NPC[A].Type == 207)
    {
        if(B == 3)
        {
            if(NPC[C].Type != 13 && NPC[C].Type != 108)
                NPC[A].Killed = B;
        }
        else if(!(B == 1))
            NPC[A].Killed = B;
        // Metroid Crawler
    }
    else if(NPC[A].Type == 205)
    {
        if(B == 3)
        {
            if(NPC[C].Type == 13)
                NPC[A].Damage = NPC[A].Damage + 1;
            else
                NPC[A].Damage = NPC[A].Damage + 3;
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
            else
                PlaySound(66);
        }
        else if(B == 8)
        {
            NPC[A].Damage = NPC[A].Damage + 1;
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
            else
                PlaySound(66);
        }
        else if(B == 10)
            NPC[A].Killed = B;
        else if(B == 4 || B == 2 || B == 9 || B == 6)
            NPC[A].Killed = B;
        // mouser
    }
    else if(NPC[A].Type == 262)
    {
        if(B == 1)
        {
        }
        else if(B == 3)
        {
            if(NPC[C].Type != 13)
            {
                NPC[A].Damage = NPC[A].Damage + 5;
                NPC[A].Immune = 60;
            }
            else
                NPC[A].Damage = NPC[A].Damage + 1;
        }
        else if(B == 6)
        {
            NPC[A].Killed = B;
            PlaySound(63);
        }
        else if(B == 10)
        {
            NPC[A].Damage = NPC[A].Damage + 2;
            NPC[A].Immune = 20;
        }
        if(NPC[A].Damage >= 20)
        {
            NPC[A].Killed = 3;
            PlaySound(41);
        }
        else if(B == 3)
        {
            if(NPC[C].Type == 13)
                PlaySound(9);
            else
                PlaySound(39);
        }
        else if(B == 10)
            PlaySound(39);
        // Wart
    }
    else if(NPC[A].Type == 201)
    {
        if(B == 1)
        {
        }
        else if(B == 3)
        {
            if(NPCIsVeggie[NPC[C].Type])
            {
                if(NPC[A].Special == 1)
                {
                    PlaySound(39);
                    NPC[A].Damage = NPC[A].Damage + 5;
                    NPC[A].Immune = 20;
                    NPC[C].Killed = 9;
                }
            }
            else
            {
                if(NPC[C].Type != 13)
                {
                    PlaySound(39);
                    NPC[A].Damage = NPC[A].Damage + 5;
                    NPC[A].Immune = 20;
                }
                else
                {
                    PlaySound(9);
                    NPC[A].Damage = NPC[A].Damage + 1;
                }
            }
        }
        else if(B == 6)
        {
            NPC[A].Killed = B;
            PlaySound(63);
        }
        else if(B == 10)
        {
            PlaySound(39);
            NPC[A].Damage = NPC[A].Damage + 5;
            NPC[A].Immune = 20;
        }
        // King Koopa
    }
    else if(NPC[A].Type == 200)
    {
        if(B == 1)
        {
        }
        else if(B == 3)
        {
            NPC[A].Immune = 20;
            if(NPC[C].Type != 13)
            {
                PlaySound(39);
                NPC[A].Damage = NPC[A].Damage + 3;
            }
            else
            {
                PlaySound(9);
                NPC[A].Damage = NPC[A].Damage + 1;
            }
        }
        else if(B == 10)
        {
            PlaySound(39);
            NPC[A].Immune = 20;
            NPC[A].Damage = NPC[A].Damage + 1;
        }
        else if(B == 6)
            NPC[A].Killed = B;
        if(NPC[A].Damage >= 12)
            NPC[A].Killed = B;
        // SMW Dry Bones
    }
    else if(NPC[A].Type == 189)
    {
        if(B == 1 || B == 8 || B == 10)
        {
            PlaySound(57);
            PlaySound(2);
            NPC[A].Special = 1;
            NPC[A].Special2 = 0;
            NPC[A].Inert = true;
            NPC[A].Stuck = true;
        }
        else if(B == 3)
        {
            if(NPC[C].Type == 13 || NPC[C].Type == 108)
            {
            }
            else
                NPC[A].Killed = B;
        }
        else
            NPC[A].Killed = B;

        // Big Boo
    }
    else if(NPC[A].Type == 44)
    {
        if((B == 3 && NPC[C].Type != 13) || B == 4)
        {
            if(B == 3 && NPC[C].Type == 45)
                NPCHit(C, 3, C);
            NPC[A].Damage = NPC[A].Damage + 1;
            NPC[A].Immune = 30;
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
            else
                PlaySound(39);
        }
        else if(B == 6)
            NPC[A].Killed = B;

        // Projectile Only Death (Ghosts, Thwomps, Etc.)
    }
    else if(NPC[A].Type == 37 || NPC[A].Type == 38 || NPC[A].Type == 42 || NPC[A].Type == 43 || NPC[A].Type == 180 || NPC[A].Type == 179 || NPC[A].Type == 206 || NPC[A].Type == 259)
    {
        if((B == 3 && NPC[C].Type != 13) || B == 4)
        {
            if(!(NPC[A].Type == 259) && NPC[C].Type != 58) // roto disks don't die form falling blocks
            {
                if(NPC[A].Type != 179)
                    NPC[A].Killed = B;
                if(B == 3 && (NPC[A].Type == 37 || NPC[A].Type == 180 || NPC[A].Type == 259))
                {
                    if(NPC[C].Location.SpeedX > 0)
                    {
                        NPC[A].Direction = 1;
                        NPC[A].Location.SpeedX = 2;
                    }
                    else
                    {
                        NPC[A].Direction = -1;
                        NPC[A].Location.SpeedX = -2;
                    }
                }
            }
        }
        else if(B == 6)
            NPC[A].Killed = B;
        else if(NPC[A].Type == 206 && B == 10)
            NPC[A].Killed = B;
        // Mega Mole
    }
    else if(NPC[A].Type == 164)
    {
        if(B == 6 || B == 7 || B == 9 || B == 2 || B == 10)
            NPC[A].Killed = B;
        else if(B == 3)
        {
            if(NPC[C].Type != 13)
                NPC[A].Killed = B;
        }
        // SMW Goombas
    }
    else if(NPC[A].Type == 165 || NPC[A].Type == 166 || NPC[A].Type == 167 || NPC[A].Type == 168)
    {
        if(B == 1)
        {
            if(NPC[A].Type == 167)
            {
                NPC[A].Type = 165;
                if(NPC[A].Projectile == false)
                {
                    PlaySound(9);
                    NPC[A].Projectile = true;
                }
            }
            else if(NPC[A].Type == 168)
            {
                PlaySound(2);
                if(NPC[A].Special2 == 0.0)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Location.SpeedX = 4 * Player[C].Direction;
                    NPC[A].Location.SpeedY = -2;
                }
            }
            else if(NPC[A].Type == 165)
            {
                PlaySound(9);
                NPC[A].Projectile = true;
                NPC[A].Type = 166;
            }
            else if(NPC[A].Type == 166)
            {
                if(NPC[A].CantHurt == 0)
                {
                    NPC[A].CantHurtPlayer = C;
                    NPC[A].CantHurt = 10;
                    PlaySound(9);
                    NPC[A].Projectile = true;
                    if(Player[C].Location.X + Player[C].Location.Width / 2.0 > NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                    NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                    NPC[A].Location.SpeedY = -2.5;
                }
            }
            NPC[A].Special = 0;
        }
        else if(B == 2)
        {
            if(NPC[A].Type != 168)
                NPC[A].Type = 166;
            NPC[A].Special = 0;
            if(NPC[A].Location.SpeedY > -4)
            {
                PlaySound(2);
                if(NPC[A].Type == 168)
                    NPC[A].Special2 = 1;
                else
                    NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
        }
        else if(B == 7)
        {
            if(NPC[A].Type != 168)
                NPC[A].Type = 166;
            NPC[A].Special = 0;
            PlaySound(2);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            if(NPC[A].Type == 168)
                NPC[A].Special2 = 1;
            else
                NPC[A].Projectile = true;
        }
        else
        {
            if(NPC[A].Type == 168 && (B == 3 || B == 4 || B == 8))
            {
                if(B == 3 && NPC[C].Type != 13 && NPC[C].Type != 108)
                {
                    NPC[A].Location.SpeedY = -7;
                    NPC[A].Killed = B;
                }
            }
            else
                NPC[A].Killed = B;
        }
        // SMB2 Bomb
    }
    else if(NPC[A].Type == 134)
    {
        if(B == 9)
            NPC[A].Killed = B;
        else if(B != 8 && B != 7)
            NPC[A].Special = 10000;
        // Heart bomb
    }
    else if(NPC[A].Type == 291)
    {
        if(B == 9)
            NPC[A].Killed = B;
        else if(B != 1 && B != 8 && B != 7 && B != 10)
        {
            if(NPC[A].HoldingPlayer > 0)
            {
                Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                NPC[A].HoldingPlayer = 0;
            }
            NPC[A].Special4 = 1;
        }
        // SMB2 Bob-omb
    }
    else if(NPC[A].Type == 135)
    {
        if(B == 9)
            NPC[A].Killed = B;
        else if(B == 7)
        {
            NPC[A].Projectile = true;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Location.SpeedY = -5;
            PlaySound(2);
            if(NPC[A].Special < 520)
                NPC[A].Special = 520;
        }
        else if(B != 8)
            NPC[A].Special = 10000;
        // Thwomps
    }
    else if(NPC[A].Type == 84 || NPC[A].Type == 181 || NPC[A].Type == 180)
    {
        if(B == 6)
            NPC[A].Killed = B;
        // Zelda NPCs
    }
    else if(NPC[A].Type == 125)
        NPC[A].Killed = B;
    // Zelda Bots
    else if(NPCIsABot[NPC[A].Type])
        NPC[A].Killed = B;
    // Switch Platforms
    else if(NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 || NPC[A].Type == 66)
    {
        if(B == 9)
            NPC[A].Killed = B;
        // Veggies
    }
    else if(NPCIsVeggie[NPC[A].Type])
    {
        if(B == 5)
        {
            Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
            NPC[A].CantHurtPlayer = NPC[A].HoldingPlayer;
            NPC[A].HoldingPlayer = 0;
            NPC[A].CantHurt = 1000;
            NPC[A].Location.SpeedX = double(3 * -NPC[A].Direction);
            NPC[A].Location.SpeedY = -3;
            NPC[A].Projectile = true;
        }
        // Because C++, second part of this condition never gets checked
        // in VB6, it does check and causes a crash, because C is an index of block when B is 4
        else if(B == 4 && (NPC[C].Type != NPC[A].Type || A == C))
        {
            if(!(NPC[C].Type == 202) && !(NPC[C].Type == 201))
            {
                if(NPC[A].Location.SpeedY > -4)
                    NPC[A].Location.SpeedY = -4;
                if(NPC[A].Location.SpeedX == 0.0)
                {
                    if(iRand() % 2 == 1)
                        NPC[A].Location.SpeedX = 2;
                    else
                        NPC[A].Location.SpeedX = -2;
                }
            }
        }
        else if(B == 6)
            NPC[A].Killed = B;
        // SMB3 Bomb
    }
    else if(NPC[A].Type == 137)
    {
        if(B == 1)
        {
            if(NPC[A].CantHurt == 0)
            {
                NPC[A].CantHurt = 10;
                NPC[A].CantHurtPlayer = C;
                PlaySound(2);
                NPC[A].Projectile = true;
                if(Player[C].Location.X + Player[C].Location.Width / 2.0 > NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                    NPC[A].Direction = -1;
                else
                    NPC[A].Direction = 1;
                NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                NPC[A].Location.SpeedY = -2;
            }
        }
        else if(B == 2)
        {
            if(NPC[A].Location.SpeedY > -4)
            {
                PlaySound(2);
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
        }
        else if(B == 7 || B == 10)
        {
            NPC[A].Type = 137;
            NPC[A].Special = 0;
            PlaySound(2);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Projectile = true;
        }
        else
        {
            if(B == 3)
            {
                if(NPC[C].Type != 13 && NPC[C].Type != 108)
                    NPC[A].Killed = B;
            }
            else if(B != 8)
                NPC[A].Killed = B;
        }
        // SMB3 Bob-om
    }
    else if(NPC[A].Type == 136)
    {
        if(B == 1)
        {
            NPC[A].CantHurt = 11;
            NPC[A].Type = 137;
            NPC[A].Special = 0;
            PlaySound(2);
            NPC[A].Projectile = true;
        }
        else if(B == 2)
        {
            if(NPC[A].Location.SpeedY > -4)
            {
                PlaySound(2);
                NPC[A].Location.SpeedY = -5;
                NPC[A].Projectile = true;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
            NPC[A].Type = 137;
            NPC[A].Special = 0;
        }
        else if(B == 7 || B == 10)
        {
            NPC[A].Type = 137;
            NPC[A].Special = 0;
            PlaySound(2);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Projectile = true;
        }
        else
        {
            if(B == 3 || B == 5)
            {
                if(NPC[C].Type != 13 && NPC[C].Type != 108)
                    NPC[A].Killed = B;
            }
            else if(B != 8)
                NPC[A].Killed = B;
        }
        if(NPC[A].Type == 137)
            NPC[A].Location.Height = 28;
        // Friendly NPCs (Toad, Peach, Link, Luigi, Etc.)
    }
    else if(NPCIsToad[NPC[A].Type] == true)
    {
        if(B == 2 && NPC[A].Location.SpeedY > -4)
        {
            PlaySound(2);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
        }
        else if(B == 3 || B == 4 || B == 5 || B == 6 || B == 9)
        {
            NPC[A].Killed = B;
            if(B == 3)
                NPC[A].Location.SpeedX = 2 * NPC[B].Direction;
        }
        // SMB3 Red Paragoomba
    }
    else if(NPC[A].Type == 3)
    {
        if(B == 1)
        {
            PlaySound(2);
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
            NPC[A].Type = 2;
            NPC[A].Frame = 0;
        }
        else
            NPC[A].Killed = B;
        // SML2 Paragoomba
    }
    else if(NPC[A].Type == 243)
    {
        if(B == 1)
        {
            PlaySound(2);
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
            NPC[A].Type = 242;
            NPC[A].Frame = 0;
        }
        else
            NPC[A].Killed = B;
        // SMB3 Brown Paragoomba
    }
    else if(NPC[A].Type == 244)
    {
        if(B == 1)
        {
            PlaySound(2);
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
            NPC[A].Type = 1;
            NPC[A].Frame = 0;
        }
        else
            NPC[A].Killed = B;
        // SMB3 Ice Block
    }
    else if(NPC[A].Type == 45)
    {
        if(B == 1 || (B == 10 && NPC[A].Projectile == false))
        {
            NPC[A].Special = 1;
            PlaySound(9);
            NPC[A].Location.SpeedX = Physics.NPCShellSpeed * Player[C].Direction;
            NPC[A].CantHurt = Physics.NPCCanHurtWait;
            NPC[A].CantHurtPlayer = C;
        }
        else if(B == 6 || B == 2 || B == 5 || B == 4 || (B == 3 && NPC[A].Special == 1) || B == 9 || (B == 10 && NPC[A].Projectile == true))
        {
            if(B == 10)
                B = 3;
            if(B == 4)
            {
                if(C == A)
                {
                    NPC[A].Killed = B;
                    NewEffect(75, NPC[A].Location);
                    Effect[numEffects].Location.X = Effect[numEffects].Location.X + NPC[A].Location.SpeedX;
                    Effect[numEffects].Location.Y = Effect[numEffects].Location.Y + NPC[A].Location.SpeedY;
                }
            }
            else if(B == 3)
            {
                if(!(NPC[C].Type == 13))
                    NPC[A].Killed = B;
                if(NPC[C].Killed == 0)
                    NPCHit(C, 3, A);
            }
            else
                NPC[A].Killed = B;
        }
        // Bower Statues
    }
    else if(NPC[A].Type == 84 || NPC[A].Type == 181)
    {
        if(B == 2)
        {
            NPC[A].Location.Y = NPC[A].Location.Y - 1;
            NPC[A].Location.SpeedY = -1;
        }
        // Things With Shells (Koopa Troopa, Buzzy Beetle, Etc.)
    }
    else if(NPC[A].Type == 4 || NPC[A].Type == 6 || NPC[A].Type == 23 || NPC[A].Type == 72 || NPCIsAParaTroopa[NPC[A].Type] || (NPC[A].Type >= 109 && NPC[A].Type <= 112) || NPC[A].Type == 173 || NPC[A].Type == 175 || NPC[A].Type == 176 || NPC[A].Type == 177)
    {
        if(B == 1)
        {
            PlaySound(2);
            NPC[A].Location.Y += NPC[A].Location.Height;
            NPC[A].Location.X += NPC[A].Location.Width / 2.0;
            if(NPC[A].Type == 4)
                NPC[A].Type = 5;
            else if(NPC[A].Type == 6)
                NPC[A].Type = 7;
            else if(NPC[A].Type == 72)
                NPC[A].Type = 73;
            else if(NPC[A].Type == 76) // winged green koopa
                NPC[A].Type = 4;
            else if(NPC[A].Type == 161) // winged red koopa
                NPC[A].Type = 6;
            else if(NPC[A].Type == 176) // smb1 winged green koopa
                NPC[A].Type = 173;
            else if(NPC[A].Type == 177) // smb winged red koopa
                NPC[A].Type = 175;
            else if(NPC[A].Type == 173) // smb1 green koopa
            {
                NPC[A].Type = 172;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == 175) // smb red koopa
            {
                NPC[A].Type = 174;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == 23)
                NPC[A].Type = 24;
            else if(NPC[A].Type >= 121 && NPC[A].Type <= 124) // smw winged koopas
            {
                NPC[A].Type = NPC[A].Type - 12;
                NPC[A].Special = 0;
            }
            else
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Location = NPC[A].Location;
                NPC[numNPCs].Location.Y -= 32.0;
                NPC[numNPCs].Type = NPC[A].Type + 8;
                NPC[numNPCs].Projectile = true;
                NPC[numNPCs].Direction = Player[C].Direction;
                NPC[numNPCs].Location.SpeedY = 0;
                NPC[numNPCs].Location.SpeedX = double(Physics.NPCShellSpeed * NPC[numNPCs].Direction);
                NPC[numNPCs].Location.X = NPC[numNPCs].Location.X - 16.0 + NPC[numNPCs].Location.SpeedX;
                CheckSectionNPC(numNPCs);
                NPC[numNPCs].CantHurtPlayer = C;
                NPC[numNPCs].CantHurt = 6;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[A].Type +=  4;
            }
            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
            NPC[A].Location.Y -= NPC[A].Location.Height;
            NPC[A].Location.X = NPC[A].Location.X - (NPC[A].Location.Width / 2.0) - double(NPC[A].Direction * 2.f);
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
            NPC[A].RealSpeedX = 0;
            NPC[A].Special = 0;
            NPC[A].Frame = 0;
            pLogDebug("Shell stomp, X distance: [%g], Y=[%g]", std::abs(NPC[numNPCs].Location.X - NPC[A].Location.X), NPC[numNPCs].Location.Y);
            if(NPC[A].Type >= 109 && NPC[A].Type <= 120)
                NewEffect(10, NPC[A].Location);
        }
        else if(B == 2 || B == 7)
        {
            PlaySound(9);
            NPC[A].Projectile = true;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
            if(NPC[A].Type == 4 || NPC[A].Type == 76)
                NPC[A].Type = 5;
            else if(NPC[A].Type == 6 || NPC[A].Type == 161)
                NPC[A].Type = 7;
            else if(NPC[A].Type == 72)
                NPC[A].Type = 73;
            else if(NPC[A].Type == 161) // winged red koopa
                NPC[A].Type = 6;
            else if(NPC[A].Type == 176) // smb1 winged green koopa
            {
                NPC[A].Type = 172;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == 177) // smb winged red koopa
            {
                NPC[A].Type = 174;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == 173) // smb1 green koopa
            {
                NPC[A].Type = 172;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == 175) // smb red koopa
            {
                NPC[A].Type = 174;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == 23)
                NPC[A].Type = 24;
            else if(NPC[A].Type >= 121 && NPC[A].Type <= 124)
            {
                NPC[A].Type = NPC[A].Type - 12;
                NPC[A].Special = 0;
            }
            else
                NPC[A].Type = NPC[A].Type + 4;
            if(B == 7 && NPC[A].Type >= 113 && NPC[A].Type <= 117)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Location = NPC[A].Location;
                NPC[numNPCs].Location.Y -= 32.0;
                NPC[numNPCs].Type = NPC[A].Type + 4;
                NPC[numNPCs].Projectile = true;
                NPC[numNPCs].Direction = Player[C].Direction;
                NPC[numNPCs].Location.SpeedY = 0;
                NPC[numNPCs].Location.SpeedX = double(Physics.NPCShellSpeed * NPC[numNPCs].Direction);
                NPC[numNPCs].Location.X = NPC[numNPCs].Location.X - (16.0 + double(32.0f * NPC[numNPCs].Direction));
                CheckSectionNPC(numNPCs);
                NPC[numNPCs].CantHurtPlayer = C;
                NPC[numNPCs].CantHurt = 6;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
            }
            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
            NPC[A].Location.Y -= NPC[A].Location.Height;
            NPC[A].Location.X = NPC[A].Location.X - (NPC[A].Location.Width / 2.0) - double(NPC[A].Direction * 2.f);
            NPC[A].Location.SpeedX = 0;
            NPC[A].Special = 0;
            NPC[A].Frame = 0;
            NPC[A].Location.SpeedY = -5;
            if(B == 2)
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
        }
        else
        {
            if(B == 3)
            {
                if(!(NPC[A].Type == 23 && (NPC[C].Type == 13 || NPC[C].Type == 108)))
                    NPC[A].Killed = B;
            }
            else
                NPC[A].Killed = B;
        }
        if(NPCIsAShell[NPC[A].Type])
            NPC[A].Stuck = false;
        // SMB3 Bowser
    }
    else if(NPC[A].Type == 86)
    {
        if(B == 9)
        {
            NPC[A].Killed = 6;
            if(NPC[A].Legacy == true)
            {
                bgMusic[NPC[A].Section] = 0;
                StopMusic();
            }
        }
        if(B == 1)
        {
            // PlaySound 2
        }
        else if(B == 3)
        {
            if(NPC[C].Type != 86)
            {
                NPC[A].Immune = 10;
                if(NPC[C].Type == 17)
                {
                    NPC[C].Location.SpeedX = -NPC[C].Location.SpeedX;
                    NPCHit(C, 4, C);
                }
                else
                    NPCHit(C, 3, A);
                if(NPC[C].Type != 13)
                {
                    PlaySound(39);
                    NPC[A].Damage = NPC[A].Damage + 10;
                }
                else
                {
                    PlaySound(9);
                    NPC[A].Damage = NPC[A].Damage + 1;
                }
            }
        }
        else if(B == 6)
            NPC[A].Killed = B;
        else if(B == 10)
        {
            NPC[A].Immune = 10;
            PlaySound(39);
            NPC[A].Damage = NPC[A].Damage + 10;
        }
        if(NPC[A].Damage >= 200)
        {
            NPC[A].Location.SpeedY = -13;
            NPC[A].Location.SpeedX = double(4 * NPC[C].Direction);
            NPC[A].Killed = B;
            if(NPC[A].Legacy == true)
            {
                bgMusic[NPC[A].Section] = 0;
                StopMusic();
            }
        }
        // SMW Rainbow Shell
    }
    else if(NPC[A].Type == 194)
    {
        if(B == 1)
            PlaySound(2);
        else if(B == 2 || B == 7)
        {
            PlaySound(9);
            NPC[A].Location.SpeedY = -5;
            if(B == 7)
            {
                NPC[A].Type = 195;
                NPC[A].Location.SpeedX = 0;
                MoreScore(6, NPC[A].Location, Player[C].Multiplier);
            }
        }
        else if(B == 6)
        {
            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 2;
            tempLocation.X = NPC[A].Location.X - 4 + dRand() * (NPC[A].Location.Width + 8) - 4;
            NewEffect(74, tempLocation);
        }
        else if(B == 8)
        {
            NPC[A].Killed = 8;
            PlaySound(36);
        }
        // Shells
    }
    else if(NPC[A].Type == 5 || NPC[A].Type == 7 || NPC[A].Type == 24 || NPC[A].Type == 73 || (NPC[A].Type >= 113 && NPC[A].Type <= 116) || NPC[A].Type == 174 || NPC[A].Type == 172 || NPC[A].Type == 195)
    {
        if(B == 1)
        {
            if(NPC[A].Effect == 2)
                NPC[A].Effect = 0;
            if(Player[C].Dismount <= 0 && Player[C].Mount != 2)
            {
                if(NPC[A].Location.SpeedX == 0.0 && NPC[A].CantHurtPlayer != C)
                {
                    PlaySound(9);
                    NPC[A].Location.SpeedX = double(Physics.NPCShellSpeed * Player[C].Direction);
                    NPC[A].CantHurt = Physics.NPCCanHurtWait;
                    NPC[A].CantHurtPlayer = C;
                    NPC[A].Projectile = true;
                    NPC[A].Location.SpeedY = 0;
                }
                else if(NPC[A].CantHurtPlayer != C || (NPC[A].Slope == 0 && Player[C].Vine == 0))
                {
                    PlaySound(2);
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.SpeedY = 0;
                    if(NPC[A].Wet > 0)
                    {
                        NPC[A].RealSpeedX = 0;
                        NPC[A].Projectile = false;
                    }
                }
            }
        }
        else if(B == 2 || B == 7)
        {
            PlaySound(9);
            NPC[A].Projectile = true;
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 0;
        }
        else if(B == 6)
        {
            if(NPC[A].Type == 195)
            {
                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 2;
                tempLocation.X = NPC[A].Location.X - 4 + dRand() * (NPC[A].Location.Width + 8) - 4;
                NewEffect(74, tempLocation);
            }
            else
                NPC[A].Killed = B;
        }
        else if(!(B == 4))
        {
            if(B == 3)
            {
                if(!(NPC[A].Type == 24 && (NPC[C].Type == 13 || NPC[C].Type == 108)))
                    NPC[A].Killed = B;
            }
            else
                NPC[A].Killed = B;
        }
        else if(B == 4)
        {
            if(NPC[C].Projectile == true && !(NPC[C].Type >= 117 && NPC[C].Type <= 120))
            {
                if(!(NPC[A].Type == 24 && NPC[C].Type == 13))
                    NPC[A].Killed = B;
                else
                    NPC[A].Killed = B;
            }
        }
        if(NPC[A].Type == 195)
        {
            NPC[A].Killed = 0;
            if(B == 5)
            {
                PlaySound(9);
                NPC[A].Location.SpeedX = Physics.NPCShellSpeed * -NPC[A].Direction;
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.SpeedX;
                NPC[A].CantHurt = Physics.NPCCanHurtWait;
                NPC[A].CantHurtPlayer = C;
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = 0;
                Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                NPC[A].HoldingPlayer = 0;
            }
        }
        // Big Piranha Plant
    }
    else if(NPC[A].Type == 256 || NPC[A].Type == 257)
    {
        if(!(B == 1 || B == 2 || B == 6))
        {
            if(NPC[A].Special3 == 0.0)
            {
                if(B == 3)
                {
                    if(NPC[C].Type == 13)
                    {
                        PlaySound(39);
                        NPC[A].Damage = NPC[A].Damage + 1;
                        NPC[A].Special3 = 10;
                        if(NPC[A].Special2 == 2)
                            NPC[A].Special = 50;
                    }
                    else
                    {
                        NPC[A].Damage = NPC[A].Damage + 3;
                        PlaySound(39);
                        NPC[A].Special3 = 30;
                        if(NPC[A].Special2 == 2)
                            NPC[A].Special = 50;
                    }
                }
                else if(B == 10)
                {
                    NPC[A].Damage = NPC[A].Damage + 2;
                    PlaySound(39);
                    NPC[A].Special3 = 10;
                    if(NPC[A].Special2 == 2)
                        NPC[A].Special = 50;
                }
            }
            if(NPC[A].Damage >= 6)
                NPC[A].Killed = B;
        }

        // Piranha Plants
    }
    else if(NPC[A].Type == 8 || NPC[A].Type == 275 || NPC[A].Type == 93 || NPC[A].Type == 51 || NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 245 || NPC[A].Type == 270)
    {
        if(!(B == 1 || B == 2 || B == 6))
        {
            if(B != 8 && B != 10)
                PlaySound(9);
            NPC[A].Killed = B;
        }
        // Podoboo
    }
    else if(NPC[A].Type == 12)
    {
        if(B == 9)
        {
        }
        else if(B == 8)
            NPC[A].Killed = B;
        else if(B == 3 || B == 5 || B == 7)
        {
            PlaySound(9);
            NPC[A].Killed = B;
        }
        else if(B == 4 && C > 0)
        {
            if(NPC[C].Projectile == true)
            {
                PlaySound(9);
                NPC[A].Killed = 3;
            }
        }
        // Player Fireball
    }
    else if(NPC[A].Type == 13 || NPC[A].Type == 265)
    {
        if(!(B == 7) && !(B == 9) && !(B == 2))
        {
            if(B == 3 || B == 4)
            {
                if(NPC[C].Type != 171)
                {
                    if(B != 6)
                        PlaySound(3);
                    NPC[A].Killed = B;
                }
            }
            else
            {
                if(B != 6)
                    PlaySound(3);
                NPC[A].Killed = B;
            }
        }
        // Yoshi Fireball
    }
    else if(NPC[A].Type == 108)
    {
        if(B == 3 || B == 5)
        {
            PlaySound(3);
            NPC[A].Killed = B;
        }
        // Hammer Bros.
    }
    else if(NPC[A].Type == 29)
        NPC[A].Killed = B;
    // Hammer Bros. Hammer
    else if(NPC[A].Type == 30)
    {
        if(B == 3)
            NPC[A].Killed = B;
        // Boom Boom
    }
    else if(NPC[A].Type == 15)
    {
        if(!(NPC[A].Special == 4))
        {
            if((B == 1 || B == 10) && NPC[A].Special == 0.0)
            {
                NPC[A].Damage = NPC[A].Damage + 3;
                if(B == 1)
                    PlaySound(2);
                else
                    PlaySound(39);
                NPC[A].Special = 4;
                NPC[A].Special2 = 0;
                NPC[A].Location.SpeedX = 0;
                NPC[A].Location.SpeedY = 0;
            }
            else if(B == 3)
            {
                NPC[A].Immune = 20;
                if(C > 0)
                    NPCHit(C, 3, A);
                NPC[A].Damage = NPC[A].Damage + 1;
                PlaySound(9);
            }
            else if(B == 6)
            {
                NPC[A].Killed = B;
                PlaySound(9);
            }
            else if(B == 10)
            {
                UpdateGraphics(true);
                NPC[A].Immune = 10;
                NPC[A].Damage = NPC[A].Damage + 1;
                PlaySound(39);
            }
        }
        if(NPC[A].Damage >= 9)
            NPC[A].Killed = B;
        // Bullet Bills
    }
    else if(NPC[A].Type == 17 || NPC[A].Type == 18)
    {
        if(B == 1 || B == 3 || B == 4 || B == 5 || B == 7 || B == 8 || B == 10)
        {
            if(!((B == 3 || B == 4) && (NPC[C].Type == 13 || NPC[C].Type == 12)))
            {
                if(!(B == 7 && NPC[A].Projectile == true))
                {
                    if(!(B == 3 && NPC[A].CantHurt > 0) && !(B == 3 && NPC[C].Type == 108))
                    {
                        if(!(NPC[A].Type == 18 && B == 4))
                        {
                            if(!(B == 10 && NPC[A].Projectile == true)) // Link can't stab friendly bullets
                            {
                                NPC[A].Killed = B;
                                if(A != C && B != 8 && B != 10)
                                    PlaySound(9);
                            }
                        }
                    }
                }
            }
        }
        // Birdo
    }
    else if(NPC[A].Type == 39)
    {
        if(NPC[A].Special >= 0)
        {
            if(B == 3)
            {
                if(NPC[C].Type != 13)
                {
                    NPC[A].Special = -30;
                    NPC[A].Damage = NPC[A].Damage + 1;
                    NPC[A].Direction = -NPC[A].Direction;
                    PlaySound(39);
                }
            }
            else if(B == 4)
                NPC[A].Damage = 3;
            else if(B == 10)
            {
                NPC[A].Special = -30;
                NPC[A].Damage = NPC[A].Damage + 1;
                NPC[A].Direction = -NPC[A].Direction;
                PlaySound(39);
            }
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
        }
        if(B == 6)
        {
            NPC[A].Killed = B;
            PlaySound(39);
        }
        // Eggs
    }
    else if(NPC[A].Type == 40)
    {
        if(B == 3)
        {
            if(NPC[C].Type == 13)
                B = 0;
        }
        if(B == 3 || B == 4 || B == 5 || B == 7 || B == 10)
        {
            if(C == A)
                NPC[A].Special = 1;
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.6;
                PlaySound(9);
            }
            NPC[A].Killed = B;
        }
        // Indestructable Objects
    }
    else if(NPC[A].Type == 21 || NPC[A].Type == 22 || NPC[A].Type == 26 || NPC[A].Type == 31 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 239 || NPC[A].Type == 35 || NPC[A].Type == 191 || NPC[A].Type == 193 || NPC[A].Type == 49 || NPCIsYoshi[NPC[A].Type] || NPC[A].Type == 96 || (NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 240 || NPC[A].Type == 241 || NPC[A].Type == 278 || NPC[A].Type == 279)
    {
        if(NPC[A].Type == 241 && (B == 4 || B == 5 || B == 10))
        {
            NPC[A].Killed = 4;
            PowBlock();
        }
        if(NPC[A].Type == 96 && (B == 4 || B == 5))
        {
            if(NPC[C].Type != NPC[A].Type)
                NPC[A].Killed = B;
        }
        else if(NPC[A].Type == 96 && B == 10)
        {
            PlaySound(53);
            NPC[A].Special2 = 1;
        }
        else
        {
            if(B == 1 && NPC[A].Type == 26)
                PlaySound(24);
            if(B == 1 && (NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 239))
            {
                NPC[A].Killed = 1;
                if(NPC[A].Type == 32)
                {
                    PSwitchTime = Physics.NPCPSwitch;
                    PSwitchPlayer = C;
                }
                else if(NPC[A].Type == 238)
                {
                    PSwitchStop = Physics.NPCPSwitch;
                    FreezeNPCs = true;
                    PSwitchPlayer = C;
                }
            }
            if(B == 2 && NPC[A].Location.SpeedY > -4 && NPC[A].CantHurt == 0)
            {
                NPC[A].CantHurt = 10;
                PlaySound(2);
                NPC[A].Location.SpeedY = -5;
                // If .Type = 96 Then .Location.SpeedY = -4
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
            else if(B == 6)
            {
                if(NPC[A].Type == 191)
                {
                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 2;
                    // tempLocation.X = .Location.X + .Location.Width / 2 - 4 '+ 4 * .Direction
                    tempLocation.X = NPC[A].Location.X - 4 + dRand() * (NPC[A].Location.Width + 8) - 4;
                    NewEffect(74, tempLocation);
                }
                else
                {
                    if(NPC[A].Type == 26)
                        NPC[A].Location.Y = NPC[A].Location.Y - 16;
                    NewEffect(10, NPC[A].Location);
                    if(NPC[A].NoLavaSplash == false)
                        NewEffect(13, NPC[A].Location);
                    PlaySound(16);
                    NPC[A].Location = NPC[A].DefaultLocation;
                    NPC[A].Active = false;
                    NPC[A].TimeLeft = 0;
                    NPC[A].Projectile = false;
                    NPC[A].Direction = NPC[A].DefaultDirection;
                    NPC[A].CantHurt = 0;
                    NPC[A].CantHurtPlayer = 0;
                    NPC[A].Reset[1] = false;
                    NPC[A].Reset[2] = false;
                }
            }
        }
        // Misc. Things With No Jump Death (SMB2 Shy Guys, SMB2 Ninji, SMB2 Pokey)
    }
    else if(NPC[A].Type == 19 || NPC[A].Type == 20 || NPC[A].Type == 247 || NPC[A].Type == 25 || NPC[A].Type == 28 || NPC[A].Type == 36 || NPC[A].Type == 285 || NPC[A].Type == 286 || NPC[A].Type == 47 || NPC[A].Type == 284 || NPC[A].Type == 48 || NPC[A].Type == 53 || NPC[A].Type == 54 || (NPC[A].Type >= 129 && NPC[A].Type <= 132) || NPC[A].Type == 158 || NPC[A].Type == 231 || NPC[A].Type == 235 || NPC[A].Type == 261 || NPC[A].Type == 272)
    {
        if(B == 10 && NPC[A].Type != 158)
            NPC[A].Killed = B;
        else if(B != 1)
        {
            if(B == 6)
                NPC[A].Killed = B;
            else if(B == 2 && NPC[A].Type == 158)
            {
                if(NPC[A].CantHurt == 0)
                {
                    NPC[A].CantHurt = 10;
                    PlaySound(2);
                    NPC[A].Location.SpeedY = -5;
                    NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
                    NPC[A].Projectile = true;
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.5;
                }
            }
            else if(NPC[A].Type == 158 && B == 5)
            {
                Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedX = 3 * -Player[NPC[A].HoldingPlayer].Direction;
                NPC[A].Location.SpeedY = -4;
                NPC[A].WallDeath = 0;
                NPC[A].HoldingPlayer = 0;
            }
            else if(NPC[A].Type == 158 && B == 3)
            {
                if(NPC[C].HoldingPlayer == 0 && NPC[C].Type != NPC[A].Type)
                {
                    NPC[A].Immune = 30;
                    NPC[A].Projectile = true;
                    NPC[A].Location.SpeedY = -5;
                    NPC[A].Location.SpeedX = (NPC[C].Location.SpeedX + NPC[A].Location.SpeedX) * 0.5;
                    if(NPC[A].Location.SpeedX < 1.2 && NPC[A].Location.SpeedX > -1.2)
                    {
                        if(NPC[C].Direction == -1)
                            NPC[A].Location.SpeedX = 3;
                        else
                            NPC[A].Location.SpeedX = -3;
                    }
                    PlaySound(9);
                }
            }
            else if(NPC[A].Type == 158 && B == 10)
            {
                NPC[A].Immune = 30;
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Location.SpeedX = Player[C].Location.SpeedX + 4 * Player[C].Direction;
                PlaySound(9);
            }
            else if(!(NPC[A].Type == 158 && (B == 4 || B == 8 || (B == 3 && NPC[C].Type == 13))))
            {
                if(NPC[A].Type == 158 && B == 7)
                {
                    NPC[A].Direction = Player[C].Direction;
                    NPC[A].Location.SpeedX = std::abs(NPC[A].Location.SpeedX) * NPC[A].Direction;
                    NPC[A].TurnAround = false;
                    NPC[A].Location.SpeedY = -6;
                    NPC[A].Projectile = true;
                    PlaySound(2);
                }
                else
                    NPC[A].Killed = B;
            }
        }
        else if(B == 1 && NPC[A].Type == 28)
        {
            NPC[A].Killed = B;
            NPC[A].Location.SpeedY = 0;
            NPC[A].Location.SpeedX = 0;
        }
        else if(B == 1 && NPCCanWalkOn[NPC[A].Type] == false && NPCJumpHurt[NPC[A].Type] == false)
        {
            NPC[A].Killed = B;
            NPC[A].Location.SpeedY = 0.123;
            NPC[A].Location.SpeedX = 0;
        }
        if(B == 1 && NPC[A].Type == 158)
            PlaySound(2);
        if((B == 1 || B == 8) && (NPC[A].Type == 47 || NPC[A].Type == 284))
            NPC[A].Killed = B;
        // Exits
    }
    else if(NPCIsAnExit[NPC[A].Type])
    {
        if(B == 6)
        {
            NewEffect(10, NPC[A].Location);
            NewEffect(13, NPC[A].Location);
            PlaySound(16);
            NPC[A].Location = NPC[A].DefaultLocation;
        }
        // Coins
    }
    else if(NPCIsACoin[NPC[A].Type])
    {
        if(LevelEditor == true)
            PlaySound(9);
        if(B == 2)
        {
            if(NPC[A].Type == 251 || NPC[A].Type == 252 || NPC[A].Type == 253)
            {
                PlaySound(81);
                NewEffect(78, NPC[A].Location);
                MoreScore(1, NPC[A].Location);
            }
            else if(NPC[A].Type == 258 || NPC[A].Type == 103)
            {
                PlaySound(14);
                NewEffect(78, NPC[A].Location);
                MoreScore(1, NPC[A].Location);
            }
            else
            {
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[A].Location.X = NPC[A].Location.X; // - (32 - .Location.Width) / 2
                NPC[A].Location.Height = 0;
                PlaySound(14);
                NewEffect(11, NPC[A].Location);
            }
            NPC[A].Killed = 9;
            if(NPC[A].Type == 252 || NPC[A].Type == 258)
                Coins = Coins + 5;
            else if(NPC[A].Type == 253)
                Coins = Coins + 20;
            else
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
                    Coins = 99;
            }
        }
        else if(B == 5 || B == 3 || B == 4 || B == 6)
            NPC[A].Killed = B;
        else if(B == 10)
        {
            if(C > 0)
                TouchBonus(C, A);
        }
        // Bonus Items
    }
    else if(NPCIsABonus[NPC[A].Type])
    {
        if(B == 2 && NPC[A].Location.SpeedY > -4)
        {
            PlaySound(2);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
        }
        else if(B == 6)
        {
            NPC[A].Killed = B;
        }
        else if(B == 5 || B == 4)
        {
            // B == 6 - touched a lava block, C is a block, not NPC!!!
            if(!(NPC[C].Type == 13 || NPC[C].Type == 108 || NPC[C].Type == 171 || NPCIsVeggie[NPC[C].Type]))
                NPC[A].Killed = B;
        }
        else if(B == 7)
        {
            if(NPC[A].Type == 9 || NPC[A].Type == 273 || NPC[A].Type == 14 ||
               NPC[A].Type == 90 || NPC[A].Type == 153 || NPC[A].Type == 169 ||
               (NPC[A].Type >= 182 && NPC[A].Type <= 188) || NPC[A].Type == 170)
            {
                NPC[A].Direction = Player[C].Direction;
                NPC[A].Location.SpeedX = std::abs(NPC[A].Location.SpeedX) * double(NPC[A].Direction);
                NPC[A].TurnAround = false;
                NPC[A].Location.SpeedY = -6;
                PlaySound(2);
            }
        }
        else if(B == 10 && NPC[A].Type != 153)
        {
            if(C > 0 && NPC[A].Effect == 0)
                TouchBonus(C, A);
        }
    }

    if(NPC[A].Killed == 10)
        NPC[A].Direction = Player[C].Direction;

    bool tempBool = false;
    if(NPC[A].Killed == 3)
    {
        if(NPC[C].Type == 13 && NPC[C].Special == 5.0 && NPC[A].Type != 13)
        {
            NPC[A].Direction = NPC[C].Direction;
            NPC[A].Killed = 10;
            tempBool = true;
        }
    }
    if(NPC[A].Killed == 10)
    {
        if(tempBool == true)
            MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, NPC[C].Multiplier);
        else
            MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, Player[C].Multiplier);
    }
    // Calculate Score
    Player[0].Multiplier = 0;
    if((B == 1 || B == 8) && C <= numPlayers && NPCIsABonus[NPC[A].Type] == false)
    {
        if(NPC[A].Type == 13 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 239)
        {
        }
        else if(oldNPC.Type == 137 || NPC[A].Type == 45 || oldNPC.Type == 166 || NPC[A].Type == 168 || (NPCIsAShell[oldNPC.Type] && !(B == 8)))
        {
            if(!(B == 8)) // (.Type = 45 And B = 8) Then
            {
                if(NPC[A].Type != 195)
                {
                    // MoreScore 1, .Location
                    if(Player[C].Multiplier > NPC[A].Multiplier)
                        NPC[A].Multiplier = Player[C].Multiplier;
                }
            }
        }
        else if(NPC[A].Location.SpeedX != oldNPC.Location.SpeedX || NPC[A].Location.SpeedY != oldNPC.Location.SpeedY || NPC[A].Projectile != NPC[A].Projectile || NPC[A].Killed != oldNPC.Killed || NPC[A].Type != oldNPC.Type || NPC[A].Inert != oldNPC.Inert)
        {
            if(NPC[A].Type == 15 && NPC[A].Killed == 0)
            {
                MoreScore(2, NPC[A].Location, Player[C].Multiplier);
                if(Player[C].Multiplier > NPC[A].Multiplier)
                    NPC[A].Multiplier = Player[C].Multiplier;
            }
            else
            {
                MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, Player[C].Multiplier);
                if(Player[C].Multiplier > NPC[A].Multiplier)
                    NPC[A].Multiplier = Player[C].Multiplier;
            }
        }
    }
    if((B == 2 || B == 7) && NPCIsABonus[NPC[A].Type] == false && oldNPC.Type != 194 && NPC[A].Type != 241)
    {
        if(NPC[A].Killed != 0 || NPC[A].Type != oldNPC.Type)
        {
            MoreScore(NPCScore[NPC[A].Type], NPC[A].Location);
            if(B == 2)
                NewEffect(75, newLoc(NPC[A].Location.X, NPC[A].Location.Y + NPC[A].Location.Height - 16));
        }
        else if(NPC[A].Location.SpeedX != oldNPC.Location.SpeedX || NPC[A].Location.SpeedY != oldNPC.Location.SpeedY || NPC[A].Projectile != NPC[A].Projectile)
        {
            // MoreScore 1, .Location
            if(B == 2)
                NewEffect(75, newLoc(NPC[A].Location.X, NPC[A].Location.Y + NPC[A].Location.Height - 16));
        }
    }
    if(B == 4 && NPC[A].Killed == 4 && NPCIsACoin[NPC[A].Type] == false && C != A && NPC[A].Type != 13 && NPC[A].Type != 265 && NPC[A].Type != 108 && NPC[A].Type != 241)
    {
        if(NPC[C].Type != 208)
        {
            if(!(NPC[A].Type == 17 && NPC[A].CantHurt > 0))
            {
                if(NPC[C].Multiplier < NPC[A].Multiplier)
                    NPC[C].Multiplier = NPC[A].Multiplier;
                MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, NPC[C].Multiplier);
            }
        }
    }
    if(B == 5 && NPC[A].Killed == 5)
    {
        if(NPC[A].Multiplier < NPC[C].Multiplier)
            NPC[A].Multiplier = NPC[C].Multiplier;
        MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, NPC[A].Multiplier);
    }
    if(B == 6 && NPC[A].Killed == 6 && (NPC[A].Type == 209 || NPC[A].Type == 200 || NPC[A].Type == 201 || NPC[A].Type == 15 || NPC[A].Type == 39 || NPC[A].Type == 86))
    {
        if(NPCWontHurt[NPC[A].Type] == false && NPCIsABonus[NPC[A].Type] == false && NPC[A].Type != 13)
            MoreScore(NPCScore[NPC[A].Type], NPC[A].Location);
    }
    if(!NPCIsACoin[NPC[A].Type] && B == 3 && C != A &&
       (NPC[A].Killed == B || !fEqual(NPC[A].Damage, oldNPC.Damage)) &&
        NPC[A].Type != 13 && NPC[A].Type != 108 && NPC[A].Type != 45 &&
        NPC[A].Type != 210 && NPC[A].Type != 241)
    {
        if(NPC[A].Killed == B)
        {
            if(NPC[C].Multiplier < NPC[A].Multiplier)
                NPC[C].Multiplier = NPC[A].Multiplier;
            MoreScore(NPCScore[NPC[A].Type], NPC[A].Location, NPC[C].Multiplier);
        }
        if(NPC[A].Type != 208 && NPC[A].Type != 209)
        {
            if(NPC[A].Location.Width >= 64 || NPC[A].Location.Height >= 64)
            {
                tempLocation = NPC[C].Location;
                tempLocation.X = NPC[C].Location.X + NPC[C].Location.Width / 2.0 - 16 + NPC[C].Location.SpeedX;
                tempLocation.Y = NPC[C].Location.Y + NPC[C].Location.Height / 2.0 - 16 + NPC[C].Location.SpeedY;
            }
            else
            {
                tempLocation = NPC[A].Location;
                tempLocation.Y = (NPC[C].Location.Y + tempLocation.Y + NPC[C].Location.Height / 2.0 + tempLocation.Height / 2.0) / 2 - 16;
                tempLocation.X = (NPC[C].Location.X + tempLocation.X + NPC[C].Location.Width / 2.0 + tempLocation.Width / 2.0) / 2 - 16;
            }
            NewEffect(75, tempLocation);
        }
    }
    if(NPC[A].Killed == 6)
    {
        if(BlockKills2[Block[C].Type] == true)
            NPC[A].NoLavaSplash = true;
    }
    if(NPC[A].Killed == 0 && NPC[A].Location.SpeedX == 0.0 && oldNPC.Location.SpeedX != 0.0)
        NPC[A].RealSpeedX = 0;

    if(NPC[A].Type != oldNPC.Type)
    {
        NPC[A].Location.Y += NPC[A].Location.Height;
        NPC[A].Location.Height = NPCHeight[NPC[A].Type];
        NPC[A].Location.Y -= NPC[A].Location.Height;
        NPC[A].Location.X += (NPC[A].Location.Width / 2.0);
        NPC[A].Location.Width = NPCWidth[NPC[A].Type];
        NPC[A].Location.X -= (NPC[A].Location.Width / 2.0);
    }

    StopHit = 0;
}
