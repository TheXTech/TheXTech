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
#include "npc.h"
#include "sound.h"
#include "graphics.h"
#include "effect.h"
#include "game_main.h"
#include "player.h"
#include "collision.h"
#include "editor.h"
#include "blocks.h"

#include <Utils/maths.h>

// UpdateNPCs at npc/npc_update.cpp

// NpcHit at npc/npc_hit.cpp

void CheckSectionNPC(int A)
{
    int B = 0;
    if(GameMenu == true)
        return;

    if(NPC[A].HoldingPlayer > 0)
    {
        if(NPC[A].TimeLeft < 10)
            NPC[A].TimeLeft = 10;
        NPC[A].Section = Player[NPC[A].HoldingPlayer].Section;
    }

    if(NPC[A].Location.X >= level[B].X)
    {
        if(NPC[A].Location.X + NPC[A].Location.Width <= level[B].Width)
        {
            if(NPC[A].Location.Y >= level[B].Y)
            {
                if(NPC[A].Location.Y + NPC[A].Location.Height <= level[B].Height)
                {
                    NPC[A].Section = B;
                    return;
                }
            }
        }
    }
    for(B = 0; B <= numSections; B++)
    {
        if(NPC[A].Location.X >= level[B].X)
        {
            if(NPC[A].Location.X + NPC[A].Location.Width <= level[B].Width)
            {
                if(NPC[A].Location.Y >= level[B].Y)
                {
                    if(NPC[A].Location.Y + NPC[A].Location.Height <= level[B].Height)
                    {
                        NPC[A].Section = B;
                        return;
                    }
                }
            }
        }
    }
}

void Deactivate(int A)
{
    if(NPC[A].DefaultType > 0)
    {
        if(NPC[A].TurnBackWipe == true && NoTurnBack[NPC[A].Section] == true)
            NPC[A].Killed = 9;
        else
        {
            if(NPC[A].Type == 189 && NPC[A].Special > 0)
            {
                NPC[A].Inert = false;
                NPC[A].Stuck = false;
            }
            // reset variables back to default
            NPC[A].Quicksand = 0;
            NPC[A].NoLavaSplash = false;
            NPC[A].Active = false;
            NPC[A].Location = NPC[A].DefaultLocation;
            NPC[A].Direction = NPC[A].DefaultDirection;
            NPC[A].Stuck = NPC[A].DefaultStuck;
            NPC[A].TimeLeft = 0;
            NPC[A].Projectile = false;
            NPC[A].Effect = 0;
            NPC[A].Effect2 = 0;
            NPC[A].Effect3 = 0;
            NPC[A].Type = NPC[A].DefaultType;
            NPC[A].BeltSpeed = 0;
            NPC[A].standingOnPlayer = 0;
            NPC[A].standingOnPlayerY = 0;
            NPC[A].Frame = 0;
            NPC[A].Killed = 0;
            NPC[A].Shadow = false;
            NPC[A].oldAddBelt = 0;
            NPC[A].Reset[1] = false;
            NPC[A].Reset[2] = false;
            NPC[A].Special = NPC[A].DefaultSpecial;
            NPC[A].Special2 = NPC[A].DefaultSpecial2;
            NPC[A].Special3 = 0;
            NPC[A].Special4 = 0;
            NPC[A].Special5 = 0;
            NPC[A].Special6 = 0;
            NPC[A].Damage = 0;
            NPC[A].HoldingPlayer = 0;
            NPC[A].Pinched1 = 0;
            NPC[A].Pinched2 = 0;
            NPC[A].Pinched3 = 0;
            NPC[A].Pinched4 = 0;
            NPC[A].Pinched = 0;
            NPC[A].MovingPinched = 0;
        }
    }
    else if(NPCIsAnExit[NPC[A].Type])
        NPC[A].TimeLeft = 100;
    else
        NPC[A].Killed = 9;
}

void Bomb(Location_t Location, int Game, int ImmunePlayer)
{
    float Radius = 0;
    int i = 0;
    double X = 0;
    double Y = 0;
    double A = 0;
    double B = 0;
    double C = 0;

    NPC[0].Multiplier = 0;
    if(Game == 0)
    {
        NewEffect(148, Location);
        PlaySound(22);
        Radius = 32;
    }
    if(Game == 2)
    {
        NewEffect(69, Location);
        PlaySound(43);
        Radius = 52;
    }
    if(Game == 3)
    {
        NewEffect(70, Location);
        PlaySound(43);
        Radius = 64;
    }

    X = Location.X + Location.Width / 2.0;
    Y = Location.Y + Location.Height / 2.0;

    for(i = 1; i <= numNPCs; i++)
    {
        if(NPC[i].Hidden == false && NPC[i].Active == true && NPC[i].Inert == false && NPC[i].Generator == false && NPCIsABonus[NPC[i].Type] == false)
        {
            if(NPC[i].Type != 13 && NPC[i].Type != 291)
            {
                A = std::abs(NPC[i].Location.X + NPC[i].Location.Width / 2.0 - X);
                B = std::abs(NPC[i].Location.Y + NPC[i].Location.Height / 2.0 - Y);
                C = std::sqrt(std::pow(A, 2) + std::pow(B, 2));

                if(static_cast<float>(C) <= static_cast<float>(Radius) + static_cast<float>(NPC[i].Location.Width / 4.0 + NPC[i].Location.Height / 4.0))
                {
                    NPC[0].Location = NPC[i].Location;
                    NPCHit(i, 3, 0);
                    if(NPCIsVeggie[NPC[i].Type])
                    {
                        NPC[i].Projectile = true;
                        NPC[i].Location.SpeedY = -5;
                        NPC[i].Location.SpeedX = dRand() * 4 - 2;
                    }
                }
            }
        }
    }

    for(i = 1; i <= numBlock; i++)
    {
        if(Block[i].Hidden == false && BlockNoClipping[Block[i].Type] == false)
        {
            A = std::abs(Block[i].Location.X + Block[i].Location.Width / 2.0 - X);
            B = std::abs(Block[i].Location.Y + Block[i].Location.Height / 2.0 - Y);
            C = std::sqrt(std::pow(A, 2) + std::pow(B, 2));
            if((float)C <= Radius + (Block[i].Location.Width / 4.0 + Block[i].Location.Height / 4.0))
            {
                BlockHit(i);
                BlockHitHard(i);
                if(Game == 0 && Block[i].Type == 457)
                    KillBlock(i);
            }
        }
    }

    if(Game != 0)
    {
        for(i = 1; i <= numPlayers; i++)
        {
            A = std::abs(Player[i].Location.X + Player[i].Location.Width / 2.0 - X);
            B = std::abs(Player[i].Location.Y + Player[i].Location.Height / 2.0 - Y);
            C = std::sqrt(std::pow(A, 2) + std::pow(B, 2));
            if((float)C <= Radius + (Player[i].Location.Width / 4.0 + Player[i].Location.Height / 4.0))
                PlayerHurt(i);
        }
    }
    else if(BattleMode == true)
    {
        for(i = 1; i <= numPlayers; i++)
        {
            if(i != ImmunePlayer)
            {
                A = std::abs(Player[i].Location.X + Player[i].Location.Width / 2.0 - X);
                B = std::abs(Player[i].Location.Y + Player[i].Location.Height / 2.0 - Y);
                C = std::sqrt(std::pow(A, 2) + std::pow(B, 2));
                if((float)C <= Radius + (Player[i].Location.Width / 4.0 + Player[i].Location.Height / 4.0))
                    PlayerHurt(i);
            }
        }
    }
}

void DropNPC(int A, int NPCType)
{
    int B = 0;
    if(A == 1 || numPlayers == 2)
    {
        PlaySound(11);
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Type = NPCType;
        NPC[numNPCs].Location.Width = NPCWidth[NPCType];
        NPC[numNPCs].Location.Height = NPCHeight[NPCType];
        if(ScreenType == 5 && vScreen[2].Visible == false)
        {
            if(A == 1)
                B = -40;
            if(A == 2)
                B = 40;
            NPC[numNPCs].Location.X = -vScreenX[1] + vScreen[1].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0 + B;
            NPC[numNPCs].Location.Y = -vScreenY[1] + 16 + 12;
        }
        else
        {
            NPC[numNPCs].Location.X = -vScreenX[A] + vScreen[A].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
            NPC[numNPCs].Location.Y = -vScreenY[A] + 16 + 12;
        }
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        NPC[numNPCs].Effect = 2;
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 200;
    }
}

void TurnNPCsIntoCoins()
{
    int A = 0;

    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].Active == true && NPC[A].Generator == false)
        {
            if(NPC[A].Hidden == false && NPC[A].Killed == 0 && NPCIsAnExit[NPC[A].Type] == false && NPC[A].Inert == false)
            {
                if(NPCIsYoshi[NPC[A].Type] == false && NPCIsBoot[NPC[A].Type] == false && NPCIsABonus[NPC[A].Type] == false && NPC[A].Type != 265 && NPC[A].Type != 13 && NPC[A].Type != 108 && NPC[A].Type != 26 && NPCIsVeggie[NPC[A].Type] == false && NPC[A].Type != 91 && NPC[A].Type != 171 && NPCIsAVine[NPC[A].Type] == false && NPC[A].Type != 56 && NPC[A].Type != 60 && NPC[A].Type != 62 && NPC[A].Type != 64 && NPC[A].Type != 66 && NPC[A].Type != 104 && !(NPC[A].Projectile == true && NPC[A].Type == 30) && !(NPC[A].Projectile == true && NPC[A].Type == 17) && NPC[A].Type != 291 && NPC[A].Type != 292 && NPC[A].Type != 266 && NPC[A].Type != 57 && NPC[A].Type != 58 && !(NPC[A].Type >= 78 && NPC[A].Type <= 83) && NPC[A].Type != 91 && NPC[A].Type != 260 && NPC[A].Type != 259)
                {
                    NPC[A].Location.Y = NPC[A].Location.Y + 32;
                    NewEffect(11, NPC[A].Location);
                    PlaySound(14);
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
                    NPC[A].Killed = 9;
                    NPC[A].Location.Height = 0;
                    NPC[A].Active = false;
                }
                else if(NPC[A].Type == 197 || NPC[A].Type == 260 || NPC[A].Type == 259)
                    NPC[A].Active = false;
            }
        }
        else if(NPC[A].Generator == true)
        {
            NPC[A].Killed = 9;
            NPC[A].Hidden = true;
        }
    }
}

void SkullRide(int A)
{
    int B = 0;
    Location_t tempLocation;
    tempLocation = NPC[A].Location;
    tempLocation.Width = tempLocation.Width + 16;
    tempLocation.X = tempLocation.X - 8;

    for(B = 1; B <= numNPCs; B++) // Recursively activate all neihbour skull-ride segments
    {
        if(NPC[B].Type == 190)
        {
            if(NPC[B].Active)
            {
                if(NPC[B].Special == 0.0)
                {
                    if(CheckCollision(tempLocation, NPC[B].Location) == true)
                    {
                        NPC[B].Special = 1;
                        SkullRide(B);
                    }
                }
            }
        }
    }
}

void NPCSpecial(int A)
{
    double B = 0;
    double C = 0;
    double D = 0;
    double E = 0;
    double F = 0;
    double fBlock = 0;
    double lBlock = 0;
    bool straightLine = false; // SET BUT NOT USED
    bool tempBool = false;
    bool tempBool2 = false;
    Location_t tempLocation;
    NPC_t tempNPC;

    // dont despawn
    if(NPC[A].Type == 200 || NPC[A].Type == 201 || NPC[A].Type == 209 || NPC[A].Type == 208 || NPC[A].Type == 262)
    {
        if(NPC[A].TimeLeft > 1)
            NPC[A].TimeLeft = 100;
    }
    // '''''''''''''

    if(NPC[A].Type == 225 || NPC[A].Type == 226 || NPC[A].Type == 227) // Vine Maker
    {
        NPC[A].Location.SpeedY = -2;
        tempLocation.Height = 28;
        tempLocation.Width = 30;
        tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
        tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
        tempBool = false;
        for(B = 1; B <= numNPCs; B++)
        {
            if(NPCIsAVine[NPC[B].Type] == true)
            {
                if(NPC[B].Hidden == false)
                {
                    if(CheckCollision(tempLocation, NPC[B].Location) == true)
                    {
                        tempBool = true;
                        break;
                    }
                }
            }
        }
        if(tempBool == false)
        {
            fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            for(B = fBlock; B <= lBlock; B++)
            {
                if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                {
                    if(CheckCollision(tempLocation, Block[B].Location) == true && BlockSlope[Block[B].Type] == 0)
                    {
                        // tempBool = True
                    }
                }
            }
        }

        if(tempBool == false || NPC[A].Special == 1.0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            if(NPC[A].Type == 225)
                NPC[numNPCs].Type = 214;
            else if(NPC[A].Type == 226)
                NPC[numNPCs].Type = 213;
            else if(NPC[A].Type == 227)
                NPC[numNPCs].Type = 224;
            NPC[numNPCs].Location.Y = static_cast<int>(floor(static_cast<double>(NPC[A].Location.Y / 32))) * 32;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
            NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
            NPC[numNPCs].Layer = NPC[A].Layer;
            NPC[numNPCs].Shadow = NPC[A].Shadow;
        }
        if(NPC[A].Special == 1.0)
            NPC[A].Killed = 9;

        // driving block
    }
    else if(NPC[A].Type == 290)
    {
        if(NPC[A].Special4 > 0)
            NPC[A].Special4 = 0;
        else
        {
            NPC[A].Special5 = 0;
            NPC[A].Special6 = 0;
        }
        if(NPC[A].Special5 > 0)
        {
            if(NPC[A].Location.SpeedX < 0)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1;
        }
        else if(NPC[A].Special5 < 0)
        {
            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.1;
        }
        else
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95;
            if(NPC[A].Location.SpeedX > -0.1 && NPC[A].Location.SpeedX < 0.1)
                NPC[A].Location.SpeedX = 0;
        }
        if(NPC[A].Special6 > 0)
        {
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.1;
        }
        else if(NPC[A].Special6 < 0)
        {
            if(NPC[A].Location.SpeedY > 0)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.1;
        }
        else
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
            if(NPC[A].Location.SpeedY > -0.1 && NPC[A].Location.SpeedY < 0.1)
                NPC[A].Location.SpeedY = 0;
        }
        if(NPC[A].Location.SpeedY > 4)
            NPC[A].Location.SpeedY = 4;
        if(NPC[A].Location.SpeedY < -4)
            NPC[A].Location.SpeedY = -4;
        if(NPC[A].Location.SpeedX > 6)
            NPC[A].Location.SpeedX = 6;
        if(NPC[A].Location.SpeedX < -6)
            NPC[A].Location.SpeedX = -6;

    }
    else if(NPC[A].Type == 291) // heart bomb
    {
        if(NPC[A].Special4 != 0.0)
        {
            NPC[A].Killed = 9;
            C = NPC[A].BattleOwner;
            if(NPC[A].CantHurtPlayer > 0)
                C = NPC[A].CantHurtPlayer;
            Bomb(NPC[A].Location, 0, floor(C));
            for(B = 1; B <= 5; B++)
            {
                NewEffect(80, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 4, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 6), 1, 0, NPC[A].Shadow);
                Effect[numEffects].Location.SpeedX = dRand() * 6 - 3;
                Effect[numEffects].Location.SpeedY = dRand() * 6 - 3;
                Effect[numEffects].Frame = (iRand() % 3);
            }
        }
    }
    else if(NPC[A].Type == 272)
    {
        if(NPC[A].Projectile == false)
        {
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Special = -1;
            else
                NPC[A].Special = 1;
            tempLocation = NPC[A].Location;
            if(int(NPC[A].Special) == -1)
                tempLocation.Y = tempLocation.Y - 1;
            else
                tempLocation.Y = tempLocation.Y + tempLocation.Height + 1;
            tempLocation.Height = 1;
            tempBool = false;
            for(B = 1; B <= numNPCs; B++)
            {
                if(NPC[B].Active == true)
                {
                    if(NPC[B].Hidden == false)
                    {
                        if(NPCIsAVine[NPC[B].Type])
                        {
                            if(CheckCollision(tempLocation, NPC[B].Location))
                            {
                                tempBool = true;
                                break;
                            }
                        }
                    }
                }
            }
            if(tempBool == false)
            {
                for(B = 1; B <= numBackground; B++)
                {
                    if(Background[B].Hidden == false)
                    {
                        if((Background[B].Type >= 174 && Background[B].Type <= 186) || Background[B].Type == 63)
                        {
                            if(CheckCollision(tempLocation, Background[B].Location))
                            {
                                tempBool = true;
                                break;
                            }
                        }
                    }
                }
            }
            if(tempBool == true)
            {
                if(NPC[A].Special == 1)
                    NPC[A].Location.SpeedY = 2;
                else
                    NPC[A].Location.SpeedY = -1;
            }
            else
            {
                if(NPC[A].Special == -1)
                {
                    NPC[A].Location.SpeedY = 2;
                    NPC[A].Special = 2;
                }
                else
                {
                    NPC[A].Location.SpeedY = -1;
                    NPC[A].Special = -1;
                }
            }
        }
        else
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.987;
            if(NPC[A].Location.SpeedX > -0.1 && NPC[A].Location.SpeedX < 0.1)
                NPC[A].Location.SpeedX = 0;

        }
    }
    else if(NPC[A].Type == 287)
        NPC[A].Type = RandomBonus();
    else if(NPC[A].Type == 286) // falling spiney
    {
        if(NPC[A].Special != 0.0)
        {
            NPC[A].Type = 285;
            NPC[A].Special = 0;
            C = 0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Dead == false && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0)
                {
                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                    }
                }
            }
            NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * NPC[A].Direction;
        }
    }
    else if(NPC[A].Type == 283) // bubble
    {
        if(fEqual(NPC[A].Special, 287))
        {
            NPC[A].Special = RandomBonus();
            NPC[A].DefaultSpecial = NPC[A].Special;
        }
        NPC[A].Location.SpeedX = 0.75 * NPC[A].DefaultDirection;
        if(NPC[A].Special2 == 0.0)
            NPC[A].Special2 = -1;
        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05 * NPC[A].Special2;
        if(NPC[A].Location.SpeedY > 1)
        {
            NPC[A].Location.SpeedY = 1;
            NPC[A].Special2 = -1;
        }
        else if(NPC[A].Location.SpeedY < -1)
        {
            NPC[A].Location.SpeedY = -1;
            NPC[A].Special2 = 1;
        }
        if(NPC[A].Special3 > 0)
        {
            NewEffect(144, NPC[A].Location);
            PlaySound(91);
            NPC[A].Type = NPC[A].Special;
            NPC[A].Special3 = 0;
            NPC[A].Special2 = 0;
            NPC[A].Special = 0;
            NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            NPC[A].FrameCount = 0;
            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0;
            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
            NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height / 2.0;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
            NPC[A].Direction = NPC[A].DefaultDirection;

            if(NPCIsACoin[NPC[A].Type])
            {
                NPC[A].Special = 1;
                NPC[A].Location.SpeedX = dRand() * 1 - 0.5;
            }

            if(Maths::iRound(NPC[A].Direction) == 0)
            {
                if(iRand() % 2 == 1)
                    NPC[A].Direction = 1;
                else
                    NPC[A].Direction = -1;
            }

            NPC[A].TurnAround = false;
            if(NPC[A].Type == 134)
                NPC[A].Projectile = true;
        }

    }
    else if(NPC[A].Type == 275) // fire plant thing
    {
        if(NPC[A].Special == 0.0)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 170)
            {
                NPC[A].Special2 = 0;
                NPC[A].Special = 1;
            }
        }
        else if(fEqual(NPC[A].Special, 1))
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 70)
            {
                NPC[A].Special2 = 0;
                NPC[A].Special = 2;
            }
        }
        else if(NPC[A].Special == 2)
        {
            if(NPC[A].Special2 == 0.0) // spit fireballs
            {
                for(B = 1; B <= 4; B++)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 276;
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].TimeLeft = NPC[A].TimeLeft;
                    NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[numNPCs].Location.Height;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Layer = NPC[A].Layer;
                    if(B == 1 || B == 4)
                    {
                        NPC[numNPCs].Location.SpeedX = -2.5;
                        NPC[numNPCs].Location.SpeedY = -1.5;
                    }
                    else
                    {
                        NPC[numNPCs].Location.SpeedX = -1;
                        NPC[numNPCs].Location.SpeedY = -2;
                    }
                    if(B == 3 || B == 4)
                        NPC[numNPCs].Location.SpeedX = -NPC[numNPCs].Location.SpeedX;
                    NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.6;
                    NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.6;
                }
            }
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 50)
            {
                NPC[A].Special2 = 0;
                NPC[A].Special = 0;
            }
        }
    }
    else if(NPC[A].Type == 276) // plant fireballs
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.98;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98;
            if(NPC[A].Location.SpeedY > -0.5)
            {
                NPC[A].Location.SpeedX = 0;
                NPC[A].Location.SpeedY = 0;
                NPC[A].Special2 = 0;
                NPC[A].Special = 1;
            }
        }
        else
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.02;
            if(NPC[A].Location.SpeedY > 2)
                NPC[A].Location.SpeedY = 2;

            if(NPC[A].Location.SpeedY > 0.25)
            {
                NPC[A].Special2 = NPC[A].Special2 + 1;
                if(NPC[A].Special2 < 7)
                    NPC[A].Location.SpeedX = -0.8;
                else if(NPC[A].Special2 < 13)
                    NPC[A].Location.SpeedX = 0.8;
                else
                {
                    NPC[A].Special2 = 0;
                    NPC[A].Location.SpeedX = 0;
                }
            }

        }
    }
    else if(NPC[A].Type == 271) // bat thing
    {
        if(NPC[A].Special == 0)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Section == NPC[A].Section && Player[B].Dead == false && Player[B].TimeToLive == 0)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.Width = 400;
                    tempLocation.Height = 800;
                    tempLocation.X = tempLocation.X - tempLocation.Width / 2.0;
                    tempLocation.Y = tempLocation.Y - tempLocation.Height / 2.0;
                    if(CheckCollision(tempLocation, Player[B].Location))
                    {
                        NPC[A].Special = 1;
                        if(Player[B].Location.X < NPC[A].Location.X)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                        NPC[A].Location.SpeedX = 0.01 * NPC[A].Direction;

                        if(Player[B].Location.Y > NPC[A].Location.Y)
                        {
                            NPC[A].Location.SpeedY = 2.5;
                            NPC[A].Special2 = Player[B].Location.Y - 130;
                        }
                        else
                        {
                            NPC[A].Location.SpeedY = -2.5;
                            NPC[A].Special2 = Player[B].Location.Y + 130;
                        }

                    }
                }
            }
        }
        else
        {
            NPC[A].Location.SpeedX = (3 - std::abs(NPC[A].Location.SpeedY)) * NPC[A].Direction;
            if((NPC[A].Location.SpeedY > 0 && NPC[A].Location.Y > NPC[A].Special2) || (NPC[A].Location.SpeedY < 0 && NPC[A].Location.Y < NPC[A].Special2))
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.98;
                if(NPC[A].Location.SpeedY > -0.1 && NPC[A].Location.SpeedY < 0.1)
                    NPC[A].Location.SpeedY = 0;
            }
        }
    }
    else if(NPC[A].Type == 269) // larry magic
    {
        if(NPC[A].Special < 2)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 30 && NPC[A].Special != 2)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[A].Location.Width = 16;
                NPC[A].Location.Height = 32;
                NPC[A].Special = 2;
                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width;
                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                NPC[A].Special2 = 21;
            }
            else if(NPC[A].Special2 >= 15 && NPC[A].Special != 1)
            {
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[A].Location.Width = 10;
                NPC[A].Location.Height = 20;
                NPC[A].Special = 1;
                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width;
                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
            }
        }
    }
    else if(NPC[A].Type == 268 || NPC[A].Type == 281) // larry/ludwig shell
    {
        if(NPC[A].Special5 == 0) // Target a Random Player
        {
            C = 0;
            do
            {
                B = (iRand() % numPlayers) + 1;
                if(Player[B].Dead == false && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0)
                    NPC[A].Special5 = B;
                C = C + 1;
                if(C >= 20)
                    NPC[A].Special5 = 1;
            }
            while(!(NPC[A].Special5 > 0));
        }

        if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
            NPC[A].Direction = -1;
        else
            NPC[A].Direction = 1;
        if(NPC[A].Special == 0)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 60)
            {
                NPC[A].Special = 1;
                NPC[A].Special2 = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            if(NPC[A].Type == 281 && NPC[A].Location.SpeedY == Physics.NPCGravity)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1 * NPC[A].Direction;
            else
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2 * NPC[A].Direction;
            if(NPC[A].Type == 281 && NPC[A].Damage >= 5)
            {
                if(NPC[A].Location.SpeedX > 5.5)
                    NPC[A].Location.SpeedX = 5.5;
                else if(NPC[A].Location.SpeedX < -5.5)
                    NPC[A].Location.SpeedX = -5.5;
            }
            else if(NPC[A].Type == 281 && NPC[A].Damage >= 10)
            {
                if(NPC[A].Location.SpeedX > 6)
                    NPC[A].Location.SpeedX = 6;
                else if(NPC[A].Location.SpeedX < -6)
                    NPC[A].Location.SpeedX = -6;
            }
            else
            {
                if(NPC[A].Location.SpeedX > 5)
                    NPC[A].Location.SpeedX = 5;
                else if(NPC[A].Location.SpeedX < -5)
                    NPC[A].Location.SpeedX = -5;
            }

            if(NPC[A].Type == 281 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special3 = NPC[A].Special3 + 1;
                if((NPC[A].Location.SpeedX < -2 && NPC[A].Direction < 0) || (NPC[A].Location.SpeedX > 2 && NPC[A].Direction > 0))
                {
                    if(NPC[A].Special3 >= 20 - NPC[A].Damage * 2)
                    {
                        NPC[A].Special3 = 0;
                        NPC[A].Location.SpeedY = -3 - dRand() * 2;
                    }
                }
            }

            NPC[A].Special2 = NPC[A].Special2 + 1;

            if(NPC[A].Special2 >= 300 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special = 2;
                NPC[A].Special2 = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Location.SpeedY = -5 - dRand() * 3;
            if(NPC[A].Type == 281)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 2;
            NPC[A].Special = 3;
        }
        else if(NPC[A].Special == 3)
        {
            if(NPC[A].Location.SpeedX > 2.5)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
            else if(NPC[A].Location.SpeedX < -2.5)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
            NPC[A].Special2 = NPC[A].Special2 + 1;

            if(NPC[A].Type == 281)
            {
                NPC[A].Special2 = 20;
                PlaySound(24);
            }

            if(NPC[A].Special2 == 20)
            {
                NPC[A].Special = 0;
                NPC[A].Special2 = NPC[A].Direction;
                NPC[A].Special3 = 0;
                NPC[A].Special4 = 0;
                NPC[A].Special5 = 0;
                NPC[A].Special6 = 0;
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[A].Type = NPC[A].Type - 1;
                NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
            }
        }
        else
            NPC[A].Special = 0;

    }
    else if(NPC[A].Type == 267) // larry koopa
    {
        // special is phase
        // special5 is targetted player
        // special3 is jump counter
        // special2 is direction

        // special4 is attack timer
        // special3 is attack count
        if(NPC[A].Inert == true)
        {
            NPC[A].Special4 = 0;
            NPC[A].Special3 = 0;
        }
        if(NPC[A].Special5 == 0) // Target a Random Player
        {
            C = 0;
            do
            {
                B = (iRand() % numPlayers) + 1;
                if(Player[B].Dead == false && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0)
                    NPC[A].Special5 = B;
                C = C + 1;
                if(C >= 20)
                    NPC[A].Special5 = 1;
            }
            while(!(NPC[A].Special5 > 0));
        }

        if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
            NPC[A].Direction = -1;
        else
            NPC[A].Direction = 1;
        if(NPC[A].Special2 == 0)
            NPC[A].Special2 = NPC[A].Direction;

        if(NPC[A].Special == 0)
        {
            if(NPC[A].Special2 == -1)
                NPC[A].Location.SpeedX = -2.5;
            else
                NPC[A].Location.SpeedX = 2.5;
            // movement
            if(NPC[A].Location.X < Player[NPC[A].Special5].Location.X - 400)
                NPC[A].Special2 = 1;
            else if(NPC[A].Location.X > Player[NPC[A].Special5].Location.X + 400)
                NPC[A].Special2 = -1;
            // jumping
            if(NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special3 = NPC[A].Special3 + 1;
                if(NPC[A].Special3 >= 30 + dRand() * 100)
                {
                    NPC[A].Special3 = 0;
                    NPC[A].Location.SpeedY = -5 - dRand() * 4;
                }
            }
            else
                NPC[A].Special3 = 0;
            // attack timer
            NPC[A].Special4 = NPC[A].Special4 + 1;
            if(NPC[A].Special4 >= 100 + dRand() * 100 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special = 1;
                NPC[A].Special5 = 0;
                NPC[A].Special3 = 0;
                NPC[A].Special4 = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                NPC[A].Direction = -1;
            else
                NPC[A].Direction = 1;
            NPC[A].Special2 = NPC[A].Direction;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 >= 10)
            {
                NPC[A].Special3 = 0;
                NPC[A].Special = 2;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Special3 == 0 || NPC[A].Special3 == 6 || NPC[A].Special3 == 12) // shoot
            {

                if(NPC[A].Special3 == 0)
                {
                    NPC[A].Special6 = Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0;
                    NPC[A].Special4 = Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0 + 16;
                }

                if(NPC[A].Special3 == 0)
                    PlaySound(34);
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Direction = NPC[A].Direction;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].Type = 269;
                NPC[numNPCs].Location.Width = 10;
                NPC[numNPCs].Location.Height = 8;
                NPC[numNPCs].Frame = 3;
                NPC[numNPCs].Special2 = NPC[A].Special3;
                if(Maths::iRound(NPC[numNPCs].Direction) == -1)
                    NPC[numNPCs].Location.X = NPC[A].Location.X - 20;
                else
                    NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width - NPC[numNPCs].Location.Width + 20;
                NPC[numNPCs].Location.Y = NPC[A].Location.Y + 47;
                NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;
                C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - NPC[A].Special6;
                D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - NPC[A].Special4;
                NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                if(NPC[numNPCs].Location.SpeedY > 3)
                    NPC[numNPCs].Location.SpeedY = 3;
                else if(NPC[numNPCs].Location.SpeedY < -3)
                    NPC[numNPCs].Location.SpeedY = -3;
            }
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 >= 30)
            {
                NPC[A].Special = 0;
                NPC[A].Special4 = 0;
                NPC[A].Special5 = 0;
                NPC[A].Special6 = 0;
            }
        }

        // ludwig koopa
    }
    else if(NPC[A].Type == 280)
    {
        // special is phase
        // special5 is targetted player
        // special3 is jump counter
        // special2 is direction

        // special4 is attack timer
        // special3 is attack count

        if(NPC[A].Inert == true)
        {
            NPC[A].Special4 = 0;
            NPC[A].Special3 = 0;
            NPC[A].Special6 = 0;
        }
        if(NPC[A].Special5 == 0) // Target a Random Player
        {
            C = 0;
            do
            {
                B = (iRand() % numPlayers) + 1;
                if(Player[B].Dead == false && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0)
                    NPC[A].Special5 = B;
                C = C + 1;
                if(C >= 20)
                    NPC[A].Special5 = 1;
            }
            while(!(NPC[A].Special5 > 0));
        }

        if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
            NPC[A].Direction = -1;
        else
            NPC[A].Direction = 1;
        if(NPC[A].Special2 == 0)
            NPC[A].Special2 = NPC[A].Direction;

        if(NPC[A].Special == 0)
        {
            if((NPC[A].Damage < 5 && NPC[A].Special6 > 60) || (NPC[A].Damage < 10 && NPC[A].Special6 > 80) || NPC[A].Inert == true)
            {
                if(NPC[A].Special2 == -1)
                    NPC[A].Location.SpeedX = -1.5;
                else
                    NPC[A].Location.SpeedX = 1.5;
                // movement
                if(NPC[A].Location.X < Player[NPC[A].Special5].Location.X - 400)
                    NPC[A].Special2 = 1;
                else if(NPC[A].Location.X > Player[NPC[A].Special5].Location.X + 400)
                    NPC[A].Special2 = -1;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98;
                if(NPC[A].Location.SpeedY == Physics.NPCGravity)
                    NPC[A].Location.SpeedX = 0;

            }
            // attack timer
            if(NPC[A].Location.SpeedY == Physics.NPCGravity)
                NPC[A].Special6 = NPC[A].Special6 + 1;
            if(NPC[A].Special6 == 20 || NPC[A].Special6 == 40 || NPC[A].Special6 == 60 || (NPC[A].Damage >= 5 && NPC[A].Special6 == 80) || (NPC[A].Damage >= 10 && NPC[A].Special6 == 100))
            {
                NPC[A].Special = 1;
                NPC[A].Special5 = 0;
                NPC[A].Special3 = 0;
                NPC[A].Special4 = 0;
            }
            if(NPC[A].Damage >= 10 && NPC[A].Special == 0 && NPC[A].Special6 >= 100)
                NPC[A].Special6 = 200;
            if(NPC[A].Special6 >= 160 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special6 = 0;
                NPC[A].Special = 3;
                PlaySound(24);
                NPC[A].Location.SpeedY = -7 - dRand() * 2;
            }
        }
        else if(NPC[A].Special == 3)
        {
            if(NPC[A].Location.SpeedY > 0) // turn into shell
            {
                NPC[A].Special = 1;
                NPC[A].Special2 = 0;
                NPC[A].Special3 = 0;
                NPC[A].Special4 = 0;
                NPC[A].Special5 = 0;
                NPC[A].Special6 = 0;
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                NPC[A].Type = 281;
                NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2 * NPC[A].Special2;
                if(NPC[A].Location.SpeedX > 5)
                    NPC[A].Location.SpeedX = 5;
                else if(NPC[A].Location.SpeedX < -5)
                    NPC[A].Location.SpeedX = -5;
            }
        }
        else if(NPC[A].Special == 1 || NPC[A].Special == 2)
        {
            if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                NPC[A].Direction = -1;
            else
                NPC[A].Direction = 1;
            NPC[A].Special2 = NPC[A].Direction;
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Special3 == 20) // shoot
            {
                PlaySound(42);
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Direction = NPC[A].Direction;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].Type = 282;
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                NPC[numNPCs].Frame = 0;
                if(Maths::iRound(NPC[numNPCs].Direction) == -1)
                    NPC[numNPCs].Location.X = NPC[A].Location.X - 24;
                else
                    NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width - NPC[numNPCs].Location.Width + 24;
                NPC[numNPCs].Location.Y = NPC[A].Location.Y + 4;
                NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
                C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0);
                D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0);
                NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                if(NPC[numNPCs].Location.SpeedY > 2)
                    NPC[numNPCs].Location.SpeedY = 2;
                else if(NPC[numNPCs].Location.SpeedY < -2)
                    NPC[numNPCs].Location.SpeedY = -2;
            }
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 < 20)
                NPC[A].Special = 1;
            else
                NPC[A].Special = 2;

            if(NPC[A].Special3 >= 40)
            {
                NPC[A].Special = 0;
                NPC[A].Special5 = 0;
                NPC[A].Special3 = 0;
                NPC[A].Special4 = 0;
            }
        }



    }
    else if(NPC[A].Type == 266) // link sword beam
    {
        NPC[A].Special = NPC[A].Special + 1;
        if(NPC[A].Special == 40)
        {
            NPC[A].Killed = 9;
            for(B = 1; B <= 4; B++)
            {
                tempLocation.Height = EffectHeight[80];
                tempLocation.Width = EffectWidth[80];
                tempLocation.SpeedX = 0;
                tempLocation.SpeedY = 0;
                tempLocation.X = NPC[A].Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4; // + .Location.SpeedX
                tempLocation.Y = NPC[A].Location.Y + dRand() * 4 - EffectHeight[80] / 2.0 - 2;
                NewEffect(80, tempLocation);
                Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.3 + dRand() * 2 - 1;
                Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
                Effect[numEffects].Frame = (iRand() % 3);
            }
        }

        if(dRand() * 10 > 5.0)
        {
            tempLocation.Height = EffectHeight[80];
            tempLocation.Width = EffectWidth[80];
            tempLocation.SpeedX = 0;
            tempLocation.SpeedY = 0;
            tempLocation.X = NPC[A].Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4; // + .Location.SpeedX
            tempLocation.Y = NPC[A].Location.Y + dRand() * 4 - EffectHeight[80] / 2.0 - 2;
            NewEffect(80, tempLocation);
            Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.15;
            Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY; // + Rnd * 2 - 1
            Effect[numEffects].Frame = (iRand() % 2) + 1;
        }


    }
    else if(NPC[A].Type == 262) // mouser
    {
        if(NPC[A].Immune == 0)
        {
            C = 0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Dead == false && Player[B].Section == NPC[A].Section)
                {
                    if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                    }
                }
            }
        }
        else
        {
            if(dRand() * 100 > 90)
                NPC[A].Direction = -NPC[A].Direction;
            NPC[A].Special3 = 0;
            NPC[A].Special4 = 0;
        }
        if(NPC[A].Immune != 0)
            NPC[A].Location.SpeedX = 0;
        else if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                if(NPC[A].Special2 == 0)
                    NPC[A].Special2 = NPC[A].Direction;
                NPC[A].Location.SpeedX = 2 * NPC[A].Special2;
                if(NPC[A].Location.X < NPC[A].DefaultLocation.X - 64)
                    NPC[A].Special2 = 1;
                else if(NPC[A].Location.X > NPC[A].DefaultLocation.X + 64)
                    NPC[A].Special2 = -1;
                NPC[A].Special3 = NPC[A].Special3 + 1;
                NPC[A].Special4 = NPC[A].Special4 + 1;
                if(NPC[A].Special3 >= 100 + dRand() * 200)
                {
                    NPC[A].Special3 = 0;
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.SpeedY = -5;
                }
                else if(NPC[A].Special4 >= 20 + dRand() * 200)
                {
                    NPC[A].Special4 = 0;
                    NPC[A].Special = -10;
                }
            }
            else
                NPC[A].Location.SpeedX = 0;
        }
        else
        {
            if(NPC[A].Special == -1)
            {
                NPC[A].Special = 20;
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Layer = "Spawned NPCs";
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Direction = NPC[A].Direction;
                NPC[numNPCs].Type = 134;
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 48;
                NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0 - 12 * NPC[numNPCs].Direction;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].Location.SpeedX = (5 + dRand() * 3) * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedY = -5 - (dRand() * 3);
            }
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Special < 0)
                NPC[A].Special = NPC[A].Special + 1;
            else
                NPC[A].Special = NPC[A].Special - 1;
        }

    }
    else if(NPC[A].Type == 261) // muncher thing
    {
        if(NPC[A].Special == 0)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Section == NPC[A].Section)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.Height = 256;
                    tempLocation.Y = NPC[A].Location.Y - tempLocation.Height;
                    if(CheckCollision(tempLocation, Player[B].Location))
                    {
                        NPC[A].Special = 1;
                        NPC[A].Location.SpeedY = -7;
                        NPC[A].Location.SpeedX = 0;
                    }
                }
            }
        }
        else if(NPC[A].Location.SpeedY == Physics.NPCGravity)
            NPC[A].Special = 0;

        if(NPC[A].Stuck == false && NPC[A].Special == 0.0)
        {
            if(NPC[A].Special2 == 0.0)
            {
                if(NPC[A].Location.X < NPC[A].DefaultLocation.X - 128 && NPC[A].Direction == -1)
                    NPC[A].Special2 = 60;
                else if(NPC[A].Location.X > NPC[A].DefaultLocation.X + 128 && NPC[A].Direction == 1)
                    NPC[A].Special2 = 60;
                NPC[A].Location.SpeedX = 1.4 * NPC[A].Direction;
                if(fEqual(float(NPC[A].Location.SpeedY), Physics.NPCGravity))
                    NPC[A].Location.SpeedY = -1.5;
            }
            else
            {
                NPC[A].Special2 = NPC[A].Special2 - 1;
                if(fEqual(float(NPC[A].Location.SpeedY), Physics.NPCGravity))
                    NPC[A].Location.SpeedX = 0;
                if(NPC[A].Special2 == 0.0)
                {
                    if(NPC[A].Location.X < NPC[A].DefaultLocation.X)
                        NPC[A].Direction = 1;
                    else
                        NPC[A].Direction = -1;
                }
            }
        }




    }
    else if(NPC[A].Type == 260) // Firebar
    {
        C = 0.03 * NPC[A].DefaultSpecial;
        B = 0.98 * NPC[A].DefaultSpecial;
        if(NPC[A].Special2 == 0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + C;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + C * NPC[A].DefaultDirection;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedX = -B;
                NPC[A].Location.SpeedY = 0;
            }
            if(NPC[A].Location.SpeedX >= -0.001)
            {
                NPC[A].Special2 = NPC[A].Special2 + 1 * NPC[A].DefaultDirection;
                if(NPC[A].Special2 <= 0)
                    NPC[A].Special2 = 3;
                NPC[A].Special5 = 0;
            }
        }
        else if(NPC[A].Special2 == 1)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + C * NPC[A].DefaultDirection;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - C;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedY = B;
                NPC[A].Location.SpeedX = 0;
            }
            if(NPC[A].Location.SpeedY <= 0.001)
            {
                NPC[A].Special2 = NPC[A].Special2 + 1 * NPC[A].DefaultDirection;
                NPC[A].Special5 = 0;
            }
        }
        else if(NPC[A].Special2 == 2)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - C;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - C * NPC[A].DefaultDirection;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedX = B;
                NPC[A].Location.SpeedY = 0;
            }
            if(NPC[A].Location.SpeedX <= 0.001)
            {
                NPC[A].Special2 = NPC[A].Special2 + 1 * NPC[A].DefaultDirection;
                NPC[A].Special5 = 0;
            }
        }
        else if(NPC[A].Special2 == 3)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - C * NPC[A].DefaultDirection;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + C;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedY = -B;
                NPC[A].Location.SpeedX = 0;
            }
            if(NPC[A].Location.SpeedY >= -0.001)
            {
                NPC[A].Special2 = NPC[A].Special2 + 1 * NPC[A].DefaultDirection;
                if(NPC[A].Special2 > 3)
                    NPC[A].Special2 = 0;
                NPC[A].Special5 = 0;
            }
        }

    }
    else if(NPC[A].Type == 259) // Roto-Disk
    {
        C = 0.2; // * .DefaultDirection
        B = 6.05; // * .DefaultDirection
        NPC[A].Special4 = NPC[A].Special4 + 1;
        if(NPC[A].Special4 == 4)
        {
            NewEffect(136, NPC[A].Location);
            Effect[numEffects].Frame = NPC[A].Frame;
            Effect[numEffects].Location.SpeedX = 0;
            Effect[numEffects].Location.SpeedY = 0;
            NPC[A].Special4 = 0;
        }
        if(NPC[A].Special == 0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + C;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + C * NPC[A].DefaultDirection;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedX = -B;
                NPC[A].Location.SpeedY = 0;
            }
            if(NPC[A].Location.SpeedX >= 0)
            {
                NPC[A].Special = NPC[A].Special + 1 * NPC[A].DefaultDirection;
                if(NPC[A].Special < 0)
                    NPC[A].Special = 3;
                NPC[A].Special5 = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + C * NPC[A].DefaultDirection;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - C;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedY = B;
                NPC[A].Location.SpeedX = 0;
            }
            if(NPC[A].Location.SpeedY <= 0)
            {
                NPC[A].Special = NPC[A].Special + 1 * NPC[A].DefaultDirection;
                NPC[A].Special5 = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - C;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - C * NPC[A].DefaultDirection;
            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedX = B;
                NPC[A].Location.SpeedY = 0;
            }
            if(NPC[A].Location.SpeedX <= 0)
            {
                NPC[A].Special = NPC[A].Special + 1 * NPC[A].DefaultDirection;
                NPC[A].Special5 = 0;
            }
        }
        else if(NPC[A].Special == 3)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - C * NPC[A].DefaultDirection;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + C;

            if(NPC[A].Special5 == 0)
            {
                NPC[A].Special5 = 1;
                NPC[A].Location.SpeedY = -B;
                NPC[A].Location.SpeedX = 0;
            }
            if(NPC[A].Location.SpeedY >= 0)
            {
                NPC[A].Special = NPC[A].Special + 1 * NPC[A].DefaultDirection;
                if(NPC[A].Special > 3)
                    NPC[A].Special = 0;
                NPC[A].Special5 = 0;
            }
        }


    }
    else if(NPC[A].Type == 255)
    {
        for(B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Type == 31)
            {
                if(NPC[B].Active == true)
                {
                    if(NPC[B].HoldingPlayer != 0)
                    {
                        if(CheckCollision(NPC[A].Location, NPC[B].Location))
                        {
                            NPC[B].Killed = 9;
                            NewEffect(10, NPC[B].Location);
                            NPC[A].Killed = 3;
                        }
                    }
                }
            }
        }
    }
    else if(NPC[A].Type == 209) // Mother Brain
    {
        if(NPC[A].Special >= 1)
        {
            B = 1 - (NPC[A].Special / 45);
            C = B * 0.5;
            B = B * 15;
            C = C * 15;
            NPC[A].Location.X = NPC[A].DefaultLocation.X + dRand() * B - dRand() * C;
            NPC[A].Location.Y = NPC[A].DefaultLocation.Y + dRand() * B - dRand() * C;
            NPC[A].Special = NPC[A].Special + 1;
            if(NPC[A].Special >= 45)
                NPC[A].Special = 0;
        }
        else
            NPC[A].Location = NPC[A].DefaultLocation;
    }
    else if(NPC[A].Type == 210) // O thing
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 80 + dRand() * 20)
            {
                NPC[A].Special = 1;
                C = 0;
                for(B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].Dead == false && Player[B].Section == NPC[A].Section)
                    {
                        if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                        {
                            C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                            D = B;
                        }
                    }
                }
                B = D;
                C = (NPC[A].Location.X + NPC[A].Location.Width / 2.0) - (Player[B].Location.X + Player[B].Location.Width / 2.0);
                D = (NPC[A].Location.Y + NPC[A].Location.Height / 2.0) - (Player[B].Location.Y + Player[B].Location.Height / 2.0);
                E = std::sqrt(std::pow(C, 2) + std::pow(D, 2));
                C = -C / E;
                D = -D / E;
                NPC[A].Location.SpeedX = C * 3;
                NPC[A].Location.SpeedY = D * 3;
            }
        }

    }
    else if(NPC[A].Type == 211) // Metroid O shooter thing
    {
        NPC[A].Special = NPC[A].Special + 1 + dRand();
        if(NPC[A].Special >= 200 + dRand() * 200)
        {
            NPC[A].Special = 0;
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Inert = NPC[A].Inert;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 28;
            NPC[numNPCs].Location.X = NPC[A].Location.X + 2;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].Layer = "Spawned NPCs";
            NPC[numNPCs].Type = 210;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 50;
        }
    }
    else if(NPC[A].Type == 206 || NPC[A].Type == 205 || NPC[A].Type == 207) // sparky
    {
        if(NPC[A].Type == 206)
            F = 2;
        else
            F = 1;
        tempBool = false;
        tempBool2 = false;
        if(NPC[A].Special == 0.0)
        {
            if(Maths::iRound(NPC[A].Direction) == 0)
            {
                if(iRand() % 2 == 1)
                    NPC[A].Direction = 1;
                else
                    NPC[A].Direction = -1;
            }

            NPC[A].Special = 1;
            NPC[A].Special2 = NPC[A].Direction;
        }
        if(NPC[A].Slope > 0)
        {
            if(NPC[A].Special == 2)
            {
                NPC[A].Special2 = 1;
                NPC[A].Special = 1;
            }
            else if(NPC[A].Special == 4)
            {
                NPC[A].Special2 = -1;
                NPC[A].Special = 1;
            }
        }

        NPC[A].Special5 = NPC[A].Special5 + 1;
        if(NPC[A].Special5 >= 8 && NPC[A].Special == 1)
        {
            NPC[A].Special5 = 8;
            NPC[A].Special = 0;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            if(NPC[A].Location.SpeedY > 8)
                NPC[A].Location.SpeedY = 8;
        }
        if(NPC[A].Special == 1)
        {
            NPC[A].Location.SpeedY = F * std::abs(NPC[A].Special2);
            NPC[A].Location.SpeedX = F * NPC[A].Special2;
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 1 + 18 * NPC[A].Special2;
            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 1;
            fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            for(B = fBlock; B <= lBlock; B++)
            {
                if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                {
                    if(CheckCollision(tempLocation, Block[B].Location) == true && BlockSlope[Block[B].Type] == 0)
                    {
                        if(NPC[A].Special2 == 1)
                        {
                            NPC[A].Location.SpeedY = 0;
                            NPC[A].Special = 2;
                            NPC[A].Special2 = -1;
                        }
                        else
                        {
                            NPC[A].Location.SpeedY = 0;
                            NPC[A].Special = 4;
                            NPC[A].Special2 = -1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                tempLocation.Width = NPC[A].Location.Width + 2;
                tempLocation.Height = 8;
                tempLocation.X = NPC[A].Location.X;
                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                for(B = fBlock; B <= lBlock; B++)
                {
                    if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                    {
                        if(CheckCollision(tempLocation, Block[B].Location) == true)
                        {
                            NPC[A].Special3 = B;
                            tempBool2 = true;
                            break;
                        }
                    }
                }
                if(tempBool2 == false)
                {
                    if(NPC[A].Special2 == 1)
                    {
                        if(NPC[A].Special3 > 0)
                        {
                            NPC[A].Location.X = Block[NPC[A].Special3].Location.X + Block[NPC[A].Special3].Location.Width + 2;
                            NPC[A].Location.Y = NPC[A].Location.Y + 2;
                        }
                        NPC[A].Special = 4;
                        NPC[A].Special2 = 1;
                    }
                    else
                    {
                        NPC[A].Special = 2;
                        NPC[A].Special2 = 1;
                    }
                }
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Location.SpeedY = F * NPC[A].Special2;
            NPC[A].Location.SpeedX = std::abs(NPC[A].Special2);
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 1;
            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 1 + 18 * NPC[A].Special2;
            fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            for(B = fBlock; B <= lBlock; B++)
            {
                if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                {
                    if(CheckCollision(tempLocation, Block[B].Location) == true)
                    {
                        if(NPC[A].Special2 == 1)
                        {
                            NPC[A].Special = 1;
                            NPC[A].Special2 = -1;
                        }
                        else
                        {
                            NPC[A].Special = 3;
                            NPC[A].Special2 = -1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                tempLocation.Width = 8;
                tempLocation.Height = NPC[A].Location.Height;
                tempLocation.Y = NPC[A].Location.Y;
                tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width;
                fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                for(B = fBlock; B <= lBlock; B++)
                {
                    if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                    {
                        if(CheckCollision(tempLocation, Block[B].Location) == true)
                        {
                            tempBool2 = true;
                            break;
                        }
                    }
                }
                if(tempBool2 == false)
                {
                    if(NPC[A].Special2 == 1)
                    {
                        NPC[A].Special = 3;
                        NPC[A].Special2 = 1;
                    }
                    else
                    {
                        NPC[A].Special = 1;
                        NPC[A].Special2 = 1;
                    }
                }
            }
        }
        else if(NPC[A].Special == 3)
        {
            NPC[A].Location.SpeedY = -std::abs(NPC[A].Special2);
            NPC[A].Location.SpeedX = F * NPC[A].Special2;
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 1 + 18 * NPC[A].Special2;
            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 1;
            fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            for(B = fBlock; B <= lBlock; B++)
            {
                if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                {
                    if(CheckCollision(tempLocation, Block[B].Location) == true && BlockSlope2[Block[B].Type] == 0)
                    {
                        if(NPC[A].Special2 == 1)
                        {
                            NPC[A].Special = 2;
                            NPC[A].Special2 = 1;
                        }
                        else
                        {
                            NPC[A].Special = 4;
                            NPC[A].Special2 = 1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                tempLocation.Width = NPC[A].Location.Width;
                tempLocation.Height = 8;
                tempLocation.X = NPC[A].Location.X;
                tempLocation.Y = NPC[A].Location.Y - 8;
                fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                for(B = fBlock; B <= lBlock; B++)
                {
                    if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                    {
                        if(CheckCollision(tempLocation, Block[B].Location) == true)
                        {
                            tempBool2 = true;
                            if(BlockSlope2[Block[B].Type] != 0)
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * F;
                        }
                    }
                }
                if(tempBool2 == false)
                {
                    if(NPC[A].Special2 == 1)
                    {
                        NPC[A].Special = 4;
                        NPC[A].Special2 = -1;
                    }
                    else
                    {
                        NPC[A].Special = 2;
                        NPC[A].Special2 = -1;
                    }
                }
            }
        }
        else if(NPC[A].Special == 4)
        {
            NPC[A].Location.SpeedY = F * NPC[A].Special2;
            NPC[A].Location.SpeedX = -std::abs(NPC[A].Special2);
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 1;
            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 1 + 18 * NPC[A].Special2;
            fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            for(B = fBlock; B <= lBlock; B++)
            {
                if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                {
                    if(CheckCollision(tempLocation, Block[B].Location) == true && BlockSlope2[Block[B].Type] == 0)
                    {
                        if(NPC[A].Special2 == 1)
                        {
                            NPC[A].Special = 1;
                            NPC[A].Special2 = 1;
                        }
                        else
                        {
                            NPC[A].Special = 3;
                            NPC[A].Special2 = 1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                tempLocation.Width = 8;
                tempLocation.Height = NPC[A].Location.Height;
                tempLocation.Y = NPC[A].Location.Y;
                tempLocation.X = NPC[A].Location.X - 8;
                fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                for(B = fBlock; B <= lBlock; B++)
                {
                    if(Block[B].Hidden == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false)
                    {
                        if(CheckCollision(tempLocation, Block[B].Location) == true)
                        {
                            tempBool2 = true;
                            break;
                        }
                    }
                }
                if(tempBool2 == false)
                {
                    if(NPC[A].Special2 == 1)
                    {
                        NPC[A].Special = 3;
                        NPC[A].Special2 = -1;
                    }
                    else
                    {
                        NPC[A].Special = 1;
                        NPC[A].Special2 = -1;
                    }
                }
            }
        }



    }
    else if(NPC[A].Type == 201) // Wart
    {
        NPC[A].Direction = NPC[A].DefaultDirection;
        if(NPC[A].Immune > 0)
            NPC[A].Special = 2;
        if(NPC[A].Damage >= 30)
        {
            if(NPC[A].Special != 3)
                PlaySound(63);
            NPC[A].Special = 3;
        }
        if(NPC[A].Special == 0)
        {
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 >= 160 + dRand() * 140)
            {
                NPC[A].Special = 1;
                NPC[A].Special3 = 0;
            }

            if(NPC[A].Special2 == 1)
            {
                NPC[A].Location.SpeedX = 1;
                if(NPC[A].Location.X > NPC[A].DefaultLocation.X + NPC[A].Location.Width * 1)
                    NPC[A].Special2 = -1;
            }
            else if(NPC[A].Special2 == -1)
            {
                NPC[A].Location.SpeedX = -1;
                if(NPC[A].Location.X < NPC[A].DefaultLocation.X - NPC[A].Location.Width * 1)
                    NPC[A].Special2 = 1;
            }
            else
                NPC[A].Special2 = NPC[A].Direction;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Special3 == 0)
                PlaySound(62);
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if((int(NPC[A].Special3) % 10) == 0)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Inert = NPC[A].Inert;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 16 + (32 * NPC[A].Direction);
                NPC[numNPCs].Location.Y = NPC[A].Location.Y + 18;
                NPC[numNPCs].Direction = NPC[A].Direction;
                NPC[numNPCs].Type = 202;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 50;
                NPC[numNPCs].Location.SpeedY = -7;
                NPC[numNPCs].Location.SpeedX = 7 * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY + dRand() * 6 - 3;
                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * (1 - (NPC[A].Special3 / 140));
            }
            if(NPC[A].Special3 >= 120 + dRand() * 40)
            {
                NPC[A].Special = 0;
                NPC[A].Special3 = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Special4 = NPC[A].Special4 + 1;
            if(NPC[A].Special4 >= 120)
            {
                NPC[A].Special4 = 0;
                NPC[A].Special = 0;
            }
        }
        else if(NPC[A].Special == 3)
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Special4 = NPC[A].Special4 + 1;
            if(NPC[A].Special4 >= 120)
                NPC[A].Killed = 3;
        }

        // Platform movement
    }
    else if(NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 ||
            NPC[A].Type == 66 || NPC[A].Type == 104 || NPC[A].Type == 179)
    {
        straightLine = false; // SET BUT NOT USED
        UNUSED(straightLine);
        tempBool = false;
        for(B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Section == NPC[A].Section)
                tempBool = true;
        }
        if(NPC[A].Type == 179)
        {
            NPC[A].Location.X = NPC[A].Location.X - 24;
            NPC[A].Location.Width = 96;
            NPC[A].Location.Y = NPC[A].Location.Y + 8;
            NPC[A].Location.Height = 32;
        }
        if((NPC[A].Direction == 1 && tempBool) || NPC[A].Type == 179)
        {
            NPC[A].Location.SpeedY = NPC[A].Special;
            NPC[A].Location.SpeedX = NPC[A].Special2;
            tempBool = false;
            tempBool2 = false;
            tempLocation = NPC[A].Location;
            tempLocation.Y = NPC[A].Location.Y + 15;
            tempLocation.Height = 2;
            tempLocation.X = NPC[A].Location.X + 47;
            tempLocation.Width = 2;
            C = 2; // The Speed
            D = 0;
            E = 0;
            F = 0;
            tempNPC = NPC[A];
            for(B = 1; B <= numBackground; B++)
            {
                if((Background[B].Type >= 70 && Background[B].Type <= 74) || Background[B].Type == 100)
                {
                    if(Background[B].Hidden == false)
                    {
                        if(CheckCollision(tempLocation, Background[B].Location))
                        {
                            if(F > 0)
                            {
                                if(Background[B].Type == NPC[A].Special5 || Background[B].Type == 70 || Background[B].Type == 100)
                                {
                                    F = 0;
                                    E = 0;
                                    D = 0;
                                    NPC[A] = tempNPC;
                                }
                            }
                            if(F == 0)
                            {
                                if(Background[B].Type == 72)
                                {
                                    if(NPC[A].Location.SpeedY <= 0)
                                        NPC[A].Location.SpeedY = -C;
                                    else
                                        NPC[A].Location.SpeedY = C;
                                    NPC[A].Location.SpeedX = 0;
                                    E = -NPC[A].Location.X + Background[B].Location.X - 32;
                                }
                                else if(Background[B].Type == 71)
                                {
                                    if(NPC[A].Location.SpeedX >= 0)
                                        NPC[A].Location.SpeedX = C;
                                    else
                                        NPC[A].Location.SpeedX = -C;
                                    NPC[A].Location.SpeedY = 0;
                                    D = -NPC[A].Location.Y + Background[B].Location.Y;
                                }
                                else if(Background[B].Type == 73)
                                {
                                    if(NPC[A].Location.SpeedY < 0)
                                        NPC[A].Location.SpeedX = C;
                                    else if(NPC[A].Location.SpeedY > 0)
                                        NPC[A].Location.SpeedX = -C;
                                    if(NPC[A].Location.SpeedX > 0)
                                        NPC[A].Location.SpeedY = -C;
                                    else if(NPC[A].Location.SpeedX < 0)
                                        NPC[A].Location.SpeedY = C;
                                }
                                else if(Background[B].Type == 74)
                                {
                                    if(NPC[A].Location.SpeedY < 0)
                                        NPC[A].Location.SpeedX = -C;
                                    else if(NPC[A].Location.SpeedY > 0)
                                        NPC[A].Location.SpeedX = C;
                                    if(NPC[A].Location.SpeedX > 0)
                                        NPC[A].Location.SpeedY = C;
                                    else if(NPC[A].Location.SpeedX < 0)
                                        NPC[A].Location.SpeedY = -C;
                                }
                                else if(Background[B].Type == 70 || Background[B].Type == 100)
                                {
                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                    NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY;
                                    tempBool = true;
                                    break;
                                }
                                tempBool = true;
                                F = Background[B].Type;
                            }
                        }
                    }
                }
            }
            NPC[A].Special5 = F;
            if(tempBool == false)
            {

                if(NPC[A].Type == 104 && NPC[A].Wet == 2)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - Physics.NPCGravity * 0.25;
                else
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + E;
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + D;
            }
            NPC[A].Special = NPC[A].Location.SpeedY;
            NPC[A].Special2 = NPC[A].Location.SpeedX;
            for(B = 1; B <= numPlayers; B++)
            {
                if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                {
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.SpeedY = 0;
                }
            }
        }
        else
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
        }
        Block[NPC[A].tempBlock].Location = NPC[A].Location;
        Block[NPC[A].tempBlock].Location.X = Block[NPC[A].tempBlock].Location.X + NPC[A].Location.SpeedX;
        if(NPC[A].Location.SpeedY < 0)
            Block[NPC[A].tempBlock].Location.Y = Block[NPC[A].tempBlock].Location.Y + NPC[A].Location.SpeedY;
        if(NPC[A].Type == 179)
        {
            NPC[A].Location.X = NPC[A].Location.X + 24;
            NPC[A].Location.Width = 48;
            NPC[A].Location.Y = NPC[A].Location.Y - 8;
            NPC[A].Location.Height = 48;

            if(NPC[A].Location.SpeedX == 0 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Location.SpeedX = C * NPC[A].Direction;
                NPC[A].Special2 = NPC[A].Location.SpeedX;
            }

        }
    }
    else if(NPC[A].Type == 200) // King Koopa
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Dead == false && Player[B].Section == NPC[A].Section)
            {
                if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    D = B;
                }
            }
        }
        if(Player[D].Location.X + Player[D].Location.Width / 2.0 > NPC[A].Location.X + 16)
            NPC[A].Direction = 1;
        else
            NPC[A].Direction = -1;
        NPC[A].Special5 = D;

        if(dRand() * 300 > 297 && NPC[A].Special == 0.0)
            NPC[A].Special = 1;
        NPC[A].Special4 = NPC[A].Special4 + 1;
        if(NPC[A].Inert == true)
            NPC[A].Special4 = 150;

        if(NPC[A].Special4 <= 80 + dRand() * 40)
        {
            if((dRand() * 100 > 40) && int(NPC[A].Special4) % 16 == 0)
            {
                PlaySound(25);
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Inert = NPC[A].Inert;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 16;
                NPC[numNPCs].Location.Y = NPC[A].Location.Y - 32;
                NPC[numNPCs].Direction = NPC[A].Direction;
                NPC[numNPCs].Type = 30;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 50;
                NPC[numNPCs].Layer = "Spawned NPCs";
                NPC[numNPCs].Location.SpeedY = -8;
                NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;
            }
        }
        else if(NPC[A].Special4 > 300 + dRand() * 50)
            NPC[A].Special4 = 0;
        if(NPC[A].Inert == true)
            NPC[A].Special = 0;
        if(NPC[A].Special > 0)
        {
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 < 40)
                NPC[A].Special = 1;
            else if(NPC[A].Special3 < 70)
            {
                if(NPC[A].Special3 == 40)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].Layer = "Spawned NPCs";
                    NPC[numNPCs].Type = 87;
                    if(NPC[numNPCs].Direction == 1)
                        NPC[numNPCs].Frame = 4;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    if(NPC[numNPCs].Direction == -1)
                        NPC[numNPCs].Location.X = NPC[A].Location.X - 40;
                    else
                        NPC[numNPCs].Location.X = NPC[A].Location.X + 54;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 19;
                    NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
                    C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0);
                    D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0);
                    NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                    if(NPC[numNPCs].Location.SpeedY > 1)
                        NPC[numNPCs].Location.SpeedY = 1;
                    else if(NPC[numNPCs].Location.SpeedY < -1)
                        NPC[numNPCs].Location.SpeedY = -1;
                    PlaySound(42);
                }
                NPC[A].Special = 2;
            }
            else
            {
                NPC[A].Special = 0;
                NPC[A].Special3 = 0;
            }
        }
        else if(NPC[A].Special == 0)
        {
            if(NPC[A].Special2 == 0)
            {
                NPC[A].Location.SpeedX = -0.5;
                if(NPC[A].Location.X < NPC[A].DefaultLocation.X - NPC[A].Location.Width * 1.5)
                    NPC[A].Special2 = 1;
            }
            else
            {
                NPC[A].Location.SpeedX = 0.5;
                if(NPC[A].Location.X > NPC[A].DefaultLocation.X + NPC[A].Location.Width * 1.5)
                    NPC[A].Special2 = 0;
            }
            if(fEqual(NPC[A].Location.SpeedY, double(Physics.NPCGravity)) || NPC[A].Slope > 0)
            {
                if(dRand() * 200 >= 198)
                    NPC[A].Location.SpeedY = -8;
            }
        }


    }
    else if(NPC[A].Type == 197) // SMW Exit
    {
        if(NPC[A].Special == 0)
            NPC[A].Location.SpeedY = 2;
        else
            NPC[A].Location.SpeedY = -2;
        if(NPC[A].Location.Y <= NPC[A].DefaultLocation.Y)
            NPC[A].Special = 0;
        if(NPC[A].Special2 == 0)
        {
            tempLocation = NPC[A].Location;
            tempLocation.Height = 8000;
            C = 0;
            for(B = 1; B <= numBlock; B++)
            {
                if(CheckCollision(tempLocation, Block[B].Location) == true)
                {
                    if(C == 0)
                        C = B;
                    else
                    {
                        if(Block[B].Location.Y < Block[C].Location.Y)
                            C = B;
                    }
                }
            }
            if(C > 0)
                NPC[A].Special2 = Block[C].Location.Y + 4;
        }
        for(B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Section == NPC[A].Section)
            {
                if(Player[B].Location.Y + NPC[A].Location.Height <= NPC[A].Special2)
                {
                    if(Player[B].Location.X + Player[B].Location.Width >= NPC[A].Location.X + NPC[A].Location.Width - 8)
                    {
                        if(Player[B].Location.X <= NPC[A].Location.X + 80)
                        {
                            if(CheckCollision(Player[B].Location, NPC[A].Location) == true)
                            {
                                MoreScore(static_cast<int>(floor(static_cast<double>((1 - (NPC[A].Location.Y - NPC[A].DefaultLocation.Y) / (NPC[A].Special2 - NPC[A].DefaultLocation.Y)) * 10))) + 1, NPC[A].Location);
                                NPC[A].Killed = 9;
                                PlaySound(37);
                            }
                            FreezeNPCs = false;
                            TurnNPCsIntoCoins();
                            if(numPlayers > 2 /*&& nPlay.Online == false*/)
                                Player[1] = Player[A];
                            LevelMacro = 7;
                            for(C = 1; C <= numPlayers; C++)
                            {
                                if(!fEqual(B, C)) // And DScreenType <> 5 Then
                                {
                                    Player[C].Section = Player[B].Section;
                                    Player[C].Location.Y = Player[B].Location.Y + Player[B].Location.Height - Player[B].Location.Height;
                                    Player[C].Location.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                                    Player[C].Location.SpeedX = 0;
                                    Player[C].Location.SpeedY = 0;
                                    Player[C].Effect = 8;
                                    Player[C].Effect2 = -B;
                                }
                            }
                            StopMusic();
                            DoEvents();
                            PlaySound(60);
                            break;
                        }
                    }
                }
            }
        }

    }
    else if(NPC[A].Type == 105)
    {
        if(fEqual(NPC[A].Special, 1))
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity / 4;
            NPC[A].Special = 0;
        }
        else
            NPC[A].Location.SpeedY = 0;
    }
    else if(NPC[A].Type == 106)
        NPC[A].Location.SpeedY = NPC[A].Direction * 2;

    else if(NPC[A].Type == 199)
    {
        if(NPC[A].Special == 0.0)
        {
            tempLocation = NPC[A].Location;
            tempLocation.Height = 400;
            tempLocation.Y = tempLocation.Y - tempLocation.Height;
            tempLocation.X = tempLocation.X + tempLocation.Width / 2.0;
            tempLocation.Width = 600;
            tempLocation.X = tempLocation.X - tempLocation.Width / 2.0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(CheckCollision(tempLocation, Player[B].Location) == true)
                {
                    if(Player[B].Location.X + Player[B].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                    NPC[A].Special = 1;
                    NewEffect(104, NPC[A].Location, NPC[A].Direction);
                    break;
                }
            }
        }
        else if(Maths::iRound(NPC[A].Special) == 1)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 == 90)
            {
                NPC[A].Location.SpeedX = 1 * NPC[A].Direction;
                NPC[A].Location.SpeedY = -4.2;
                NPC[A].Special = 2;
                PlaySound(61);
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 0.4;
            if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y + NPC[A].Location.Height + 48)
                Deactivate(A);
        }


        // End If
        // Sniffits
    }
    else if(NPC[A].Type >= 130 && NPC[A].Type <= 132)
    {
        if(NPC[A].Projectile == true)
        {
            NPC[A].Special = 0;
            NPC[A].Special2 = 0;
        }
        tempBool = false;
        if(NPC[A].Type < 132)
        {
            NPC[A].Special = NPC[A].Special + 1;
            if(NPC[A].Special > 120)
            {
                NPC[A].FrameCount = NPC[A].FrameCount - 1;
                if(NPC[A].Special2 == 0)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Location.X = NPC[A].Location.X - 2;
                }
                else
                {
                    NPC[A].Special2 = 0;
                    NPC[A].Location.X = NPC[A].Location.X + 2;
                }
                NPC[A].Location.SpeedX = 0;
                if(NPC[A].Special >= 150)
                {
                    tempBool = true;
                    NPC[A].Special = 0;
                }
            }
        }
        else
        {
            NPC[A].Special = NPC[A].Special + 1;
            if(NPC[A].Special > 160)
            {
                tempBool = true;
                if(NPC[A].Special3 != 2)
                {
                    NPC[A].Special3 = NPC[A].Special3 - 1;
                    NPC[A].Special = 0;
                }
                else
                {
                    NPC[A].Special = 140;
                    NPC[A].Special3 = NPC[A].Special3 - 1;
                    if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
                        NPC[A].Special2 = 90;
                }
                if(NPC[A].Special3 < 0)
                    NPC[A].Special3 = 2;
            }
            if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
            {
                NPC[A].Special2 = NPC[A].Special2 + 1;
                if(NPC[A].Special2 >= 100)
                {
                    NPC[A].Special2 = 0;
                    NPC[A].Location.SpeedY = -3.9;
                    NPC[A].Location.Y = NPC[A].Location.Y - Physics.NPCGravity;
                }
            }
            else
                NPC[A].FrameCount = NPC[A].FrameCount - 1;
        }
        if(tempBool == true)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Type = 133;
            NPC[numNPCs].Layer = NPC[A].Layer;
            NPC[numNPCs].Inert = NPC[A].Inert;
            NPC[numNPCs].Direction = NPC[A].Direction;
            NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
            NPC[numNPCs].Location.Width = 16;
            NPC[numNPCs].Location.Height = 16;
            NPC[numNPCs].Location.X = NPC[A].Location.X + 8 + 16 * NPC[numNPCs].Direction;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y + 13;
        }
    }

}

void SpecialNPC(int A)
{
    int B = 0;
    float C = 0;
    float D = 0;
    float E = 0;
    float F = 0;
    bool tempTurn = false;
    Location_t tempLocation;
    Location_t tempLocation2;
    NPC_t tempNPC;

    if(NPC[A].Type == 87 || NPC[A].Type == 276 || NPC[A].Type == 85 ||
       NPC[A].Type == 133 || NPC[A].Type == 246 || NPC[A].Type == 30 ||
       NPC[A].Type == 202 || NPC[A].Type == 210 ||
       (BattleMode && (NPC[A].Type == 13 || NPC[A].Type == 171 || NPC[A].Type == 265))) // Link shield block
    {
        for(B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Character == 5 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0 && Player[B].SwordPoke == 0 && Player[B].Fairy == false && !(NPC[A].Type == 13 && NPC[A].CantHurtPlayer == B) && !(NPC[A].Type == 171 && NPC[A].CantHurtPlayer == B))
            {
                if(!Player[B].Duck)
                    tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height - 52;
                else
                    tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height - 28;
                tempLocation.Height = 24;
                tempLocation.Width = 6;
                if(Player[B].Direction == 1)
                    tempLocation.X = Player[B].Location.X + Player[B].Location.Width - 2;
                else
                    tempLocation.X = Player[B].Location.X - tempLocation.Width + 4;
                if(CheckCollision(NPC[A].Location, tempLocation))
                {
                    UpdateGraphics(true);
                    PlaySound(85);
                    if(NPC[A].Type == 133)
                        NPC[A].Killed = 3;
                    else
                    {
                        NPC[A].Killed = 9;
                        if(NPC[A].Type == 13 || NPC[A].Type == 265)
                            NPC[A].Killed = 3;
                        if(NPC[A].Type != 30 && NPC[A].Type != 202 && NPC[A].Type != 210 && NPC[A].Type != 171 && NPC[A].Type != 13 && NPC[A].Type != 265)
                        {
                            for(int Ci = 1; Ci <= 10; Ci++)
                            {
                                NewEffect(77, NPC[A].Location, static_cast<float>(NPC[A].Special));
                                Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5 + NPC[A].Location.SpeedX * 0.1;
                                Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5 - NPC[A].Location.SpeedY * 0.1;
                                if(Effect[numEffects].Frame == 0)
                                    Effect[numEffects].Frame = -(iRand() % 3);
                                else
                                    Effect[numEffects].Frame = 5 + (iRand() % 3);
                            }
                        }
                        if(NPC[A].Type != 13 && NPC[A].Type != 265)
                        {
                            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                            NewEffect(10, NPC[A].Location);
                        }
                    }
                }
            }
        }
    }

    if(NPC[A].Type == 196 || NPC[A].Type == 97)
    {
        if(NPC[A].Projectile != 0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
            if(NPC[A].Location.SpeedY < 1 && NPC[A].Location.SpeedY > -1)
            {
                if(NPC[A].Location.SpeedX < 1 && NPC[A].Location.SpeedX > -1)
                    NPC[A].Projectile = false;
            }
        }
    }
    if(NPC[A].Type == 292) // Toad Boomerang
    {
        if(NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;
        if(NPC[A].Location.SpeedY > 8)
            NPC[A].Location.SpeedY = 8;
        if(NPC[A].Location.SpeedY < -8)
            NPC[A].Location.SpeedY = -8;
        if(NPC[A].Location.SpeedX > 12 + Player[NPC[A].Special5].Location.SpeedX)
            NPC[A].Location.SpeedX = 12 + Player[NPC[A].Special5].Location.SpeedX;
        if(NPC[A].Location.SpeedX < -12 + Player[NPC[A].Special5].Location.SpeedX)
            NPC[A].Location.SpeedX = -12 + Player[NPC[A].Special5].Location.SpeedX;
        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
            if(NPC[A].Location.SpeedX > -4 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.5;

        }
        else if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
            if(NPC[A].Location.SpeedX > -4 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.5;
        }
        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + (Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 - NPC[A].Location.X + NPC[A].Location.Width / 2.0) * 0.0005;
        if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 > Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.2;
            if(NPC[A].Location.SpeedY > 0 && NPC[A].Direction != NPC[A].Special6)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - std::abs(NPC[A].Location.SpeedY) * 0.04;
        }
        else if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 < Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.2;
            if(NPC[A].Location.SpeedY < 0 && NPC[A].Direction != NPC[A].Special6)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + std::abs(NPC[A].Location.SpeedY) * 0.04;
        }
        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + (Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0 - NPC[A].Location.Y + NPC[A].Location.Height / 2.0) * 0.004;


        for(B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Active)
            {
                if(NPCIsACoin[NPC[B].Type])
                {
                    if(CheckCollision(NPC[A].Location, NPC[B].Location))
                    {
                        NPC[B].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[B].Location.Width / 2.0;
                        NPC[B].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - NPC[B].Location.Height / 2.0;
                        NPC[B].Special = 0;
                        NPC[B].Projectile = false;
                    }
                }
            }
        }

        if((NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 && NPC[A].Special6 == 1) || (NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 && NPC[A].Special6 == -1))
        {
        }
        else
        {
            NPC[A].Special2 = 1;
            if(CheckCollision(NPC[A].Location, Player[NPC[A].Special5].Location))
            {
                NPC[A].Killed = 9;
                Player[NPC[A].Special5].FrameCount = 115;
                PlaySound(73);
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Active)
                    {
                        if(NPCIsACoin[NPC[B].Type])
                        {
                            if(CheckCollision(NPC[A].Location, NPC[B].Location))
                            {
                                NPC[B].Location.X = Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 - NPC[B].Location.Width / 2.0;
                                NPC[B].Location.Y = Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0 - NPC[B].Location.Height / 2.0;
                                TouchBonus(static_cast<int>(floor(static_cast<double>(NPC[A].Special5))), B);
                            }
                        }
                    }
                }
            }
        }

        if(NPC[A].Special2 == 1)
        {
            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0)
            {
                if(NPC[A].Location.SpeedX > 0)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.1;
            }
            else if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0)
            {
                if(NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1;
            }
            if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 > Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0)
            {
                if(NPC[A].Location.SpeedY > 0)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.3;
            }
            else if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 < Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0)
            {
                if(NPC[A].Location.SpeedY < 0)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.1;
            }
        }

    }
    else if(NPC[A].Type == 96) // yoshi egg
    {
        if(NPC[A].Location.SpeedY > 2)
            NPC[A].Projectile = true;
        if(NPC[A].Special2 == 1)
            NPC[A].Killed = 1;
    }
    else if(NPC[A].Type == 251 || NPC[A].Type == 252 || NPC[A].Type == 253) // Rupee
    {
        if(NPC[A].Location.SpeedX < -0.02)
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.02;
        else if(NPC[A].Location.SpeedX > 0.02)
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.02;
        else
            NPC[A].Location.SpeedX = 0;
    }
    else if(NPC[A].Type == 237 || NPC[A].Type == 263) // Yoshi Ice
    {
        if(dRand() * 100 > 93)
        {
            tempLocation.Height = EffectHeight[80];
            tempLocation.Width = EffectWidth[80];
            tempLocation.SpeedX = 0;
            tempLocation.SpeedY = 0;
            tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
            tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
            NewEffect(80, tempLocation);
        }
        if(NPC[A].Projectile != 0)
        {
            if(dRand() * 100 > 80)
            {
                tempLocation.Height = EffectHeight[80];
                tempLocation.Width = EffectWidth[80];
                tempLocation.SpeedX = 0;
                tempLocation.SpeedY = 0;
                tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                NewEffect(80, tempLocation);
            }
        }
    }
    else if(NPC[A].Type == 231 || NPC[A].Type == 235) // Blooper
    {
        if(NPC[A].Wet == 2 && NPC[A].Quicksand == 0)
        {
            if(NPC[A].Special == 0)
            {
                C = 0;
                D = 1;
                for(B = 1; B <= numPlayers; B++)
                {
                    if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                    {
                        if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                        {
                            C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                            D = B;
                        }
                    }
                }
                NPC[A].Special = D;
            }
            if(NPC[A].Special2 == 0 || NPC[A].Special4 == 1)
            {
                if(NPC[A].Location.Y + NPC[A].Location.Height >= Player[NPC[A].Special].Location.Y - 24 || NPC[A].Special4 == 1)
                {
                    NPC[A].Special2 = 60;
                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[NPC[A].Special].Location.X + Player[NPC[A].Special].Location.Width / 2.0)
                        NPC[A].Location.SpeedX = 4;
                    else
                        NPC[A].Location.SpeedX = -4;
                    if(NPC[A].Special4 == 1)
                    {
                        NPC[A].Special4 = 0;
                        NPC[A].Location.Y = NPC[A].Location.Y - 0.1;
                    }
                }
            }
            if(NPC[A].Special2 > 0)
            {
                NPC[A].Special2 = NPC[A].Special2 - 1;
                NPC[A].Location.SpeedY = -1.75;
                NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Location.SpeedY = 1;
                NPC[A].Frame = 1;
            }
            if(NPC[A].Special2 == 0)
                NPC[A].Special2 = -20;
            if(NPC[A].Special2 < 0)
                NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Location.SpeedY >= 0)
                NPC[A].Location.SpeedX = 0;
        }
        else
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.7;
            if(NPC[A].Location.SpeedY < -1)
                NPC[A].Location.SpeedY = -1;
            NPC[A].Special2 = -60;
        }

        if(Player[NPC[A].Special].Dead || Player[NPC[A].Special].Section != NPC[A].Section)
            NPC[A].Special = 0;
    }
    else if(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 1) // Red SMB3 Cheep
    {
        if(NPC[A].Projectile == 0)
        {
            if(NPC[A].Wet == 2)
                NPC[A].Special5 = 0;
            C = 0;
            D = 1;
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = B;
                    }
                }
            }
            B = int(D);
            if(!Player[B].WetFrame && Player[B].Location.Y + Player[B].Location.Height < NPC[A].Location.Y)
            {
                if((NPC[A].Direction == 1 && Player[D].Location.X > NPC[A].Location.X) ||
                   (NPC[A].Direction == -1 && Player[B].Location.X < NPC[A].Location.X))
                {
                    if(NPC[A].Location.X > Player[B].Location.X - 200 && NPC[A].Location.X + NPC[A].Location.Width < Player[B].Location.X + Player[B].Location.Width + 200)
                    {
                        if(NPC[A].Wet == 2)
                        {
                            if(NPC[A].Location.SpeedY > -3)
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.1;
                            NPC[A].Special3 = 1;
                        }
                    }
                    else
                        NPC[A].Special3 = 0;
                }
                else
                    NPC[A].Special3 = 0;
                if(NPC[A].Special3 == 1 && NPC[A].Wet == 0)
                {
                    NPC[A].Location.SpeedY = -(NPC[A].Location.Y - Player[B].Location.Y + Player[B].Location.Height / 2.0) * 0.05 + dRand() * 4 - 2;
                    if(NPC[A].Location.SpeedY < -9)
                        NPC[A].Location.SpeedY = -9;
                    NPC[A].Special3 = 0;
                    NPC[A].Special5 = 1;
                    NPC[A].WallDeath = 10;
                }
            }
        }
    }
    else if(NPC[A].Type == 288)
    {
        if(NPC[A].Special3 == 1)
        {
            tempLocation = NPC[A].Location;
            tempLocation.Y = tempLocation.Y - 32;
            NewEffect(147, NPC[A].Location);
            NewEffect(147, tempLocation);
            NPC[A].Frame = 0;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
            NPC[A].Special3 = 0;
            NPC[A].Effect = 8;
            NPC[A].Projectile = false;
            NPC[A].Type = 289;
            NPC[A].Effect2 = 16;
            PlaySound(41);
        }
    // firespitting plant
    }
    else if(NPC[A].Type == 245)
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
            {
                if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                    NPC[A].Special4 = B;
                }
            }
        }

        if(NPC[A].Location.X != NPC[A].DefaultLocation.X)
        {
            NPC[A].Killed = 2;
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.SpeedY;
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + NPCHeight[NPC[A].Type] + 1.5;
                NPC[A].Special2 = 4;
                NPC[A].Special = 70;
            }
            if(NPC[A].Special2 == 1)
            {
                NPC[A].Special = NPC[A].Special + 1;
                NPC[A].Location.Y = NPC[A].Location.Y - 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(fEqual(NPC[A].Special2, 2))
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 100)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
                else if(fEqual(NPC[A].Special, 50))
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].Type = 246;
                    NPC[numNPCs].Frame = 1;
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];

                    if(fEqual(NPC[numNPCs].Location.Width, 16))
                    {
                        NPC[numNPCs].Location.X = NPC[A].Location.X + 8;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + 8;
                    }
                    else // modified fireball
                    {
                        NPC[numNPCs].Location.X = NPC[A].Location.X;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                    }

                    NPC[numNPCs].Location.SpeedX = double(3.f * NPC[numNPCs].Direction);
                    C = float(NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) -
                        float(Player[NPC[A].Special4].Location.X + Player[NPC[A].Special4].Location.Width / 2.0);
                    D = float(NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) -
                        float(Player[NPC[A].Special4].Location.Y + Player[NPC[A].Special4].Location.Height / 2.0);

                    if(C == 0.0f)
                        C = -0.00001f;
                    NPC[numNPCs].Location.SpeedY = (double(D) / double(C)) * NPC[numNPCs].Location.SpeedX;

                    if(NPC[numNPCs].Location.SpeedY > 2)
                        NPC[numNPCs].Location.SpeedY = 2;
                    else if(NPC[numNPCs].Location.SpeedY < -2)
                        NPC[numNPCs].Location.SpeedY = -2;

                    NPC[numNPCs].Location.X = NPC[numNPCs].Location.X + NPC[numNPCs].Location.SpeedX * 4;
                    NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y + NPC[numNPCs].Location.SpeedY * 4;
                }
            }
            else if(fEqual(NPC[A].Special2, 3))
            {
                NPC[A].Special = NPC[A].Special + 1;
                NPC[A].Location.Y = NPC[A].Location.Y + 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                    NPC[A].Special2 = 4;
            }
            else if(fEqual(NPC[A].Special2, 4))
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 150)
                {
                    tempTurn = true;
                    if(!NPC[A].Inert)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                            {
                                if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                {
                                    tempTurn = false;
                                    break;
                                }
                            }
                        }
                    }

                    if(tempTurn)
                    {
                        NPC[A].Special2 = 1;
                        NPC[A].Special = 0;
                    }
                    else
                        NPC[A].Special = 140;
                }
            }
            NPC[A].Location.Height = NPCHeight[NPC[A].Type] - (NPC[A].Location.Y - NPC[A].DefaultLocation.Y);
            if(NPC[A].Location.Height < 0)
                NPC[A].Location.Height = 0;
            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }

    // jumping plant
    }
    else if(NPC[A].Type == 270)
    {
        if(NPC[A].Projectile != 0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98;
        }
        else
        {
            if(NPC[A].Special == 0) // hiding
            {
                NPC[A].Location.Y = NPC[A].DefaultLocation.Y + NPCHeight[NPC[A].Type] + 1.5;
                NPC[A].Location.Height = 0;
                NPC[A].Special2 = NPC[A].Special2 - 1;
                if(NPC[A].Special2 <= -30)
                {
                    tempTurn = true;
                    if(!NPC[A].Inert)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                            {
                                if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                {
                                    tempTurn = false;
                                    break;
                                }
                            }
                        }
                    }

                    if(tempTurn)
                    {
                        NPC[A].Special = 1;
                        NPC[A].Special2 = 0;
                    }
                    else
                        NPC[A].Special2 = 1000;
                }
            }
            else if(NPC[A].Special == 1) // jumping
            {
                NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                if(NPC[A].Special2 == 0)
                    NPC[A].Location.SpeedY = -6;
                else if(NPC[A].Location.SpeedY < -4)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.2;
                else if(NPC[A].Location.SpeedY < -3)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.15;
                else if(NPC[A].Location.SpeedY < -2)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.1;
                else if(NPC[A].Location.SpeedY < -1)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05;
                else
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.02;
                NPC[A].Special2 = NPC[A].Special2 + 1;
                if(NPC[A].Location.SpeedY >= 0)
                {
                    NPC[A].Special = 2;
                    NPC[A].Special2 = 0;
                }

            }
            else if(NPC[A].Special == 2) // falling
            {
                NPC[A].Location.Height = NPCHeight[NPC[A].Type];

                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.01;
                if(NPC[A].Location.SpeedY >= 0.75)
                    NPC[A].Location.SpeedY = 0.75;

                if(NPC[A].Location.Y + NPCHeight[NPC[A].Type] >= NPC[A].DefaultLocation.Y + NPCHeight[NPC[A].Type])
                {
                    NPC[A].Location.Height = (NPC[A].DefaultLocation.Y + NPCHeight[NPC[A].Type]) - (NPC[A].Location.Y);
                    if(NPC[A].Location.Y >= NPC[A].DefaultLocation.Y + NPCHeight[NPC[A].Type])
                    {
                        NPC[A].Location.Height = 0;
                        NPC[A].Location.Y = NPC[A].DefaultLocation.Y + NPCHeight[NPC[A].Type];
                        NPC[A].Special = 0;
                        NPC[A].Special2 = 60;
                    }
                }
            }
            if(NPC[A].Location.Height < 0)
                NPC[A].Location.Height = 0;
            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }

    // Piranha Plant code
    }
    else if(NPC[A].Type == 8 || NPC[A].Type == 74 || NPC[A].Type == 93 || NPC[A].Type == 256)
    {
        if(NPC[A].Special3 > 0)
            NPC[A].Special3 = NPC[A].Special3 - 1;
        if(NPC[A].Location.X != NPC[A].DefaultLocation.X)
        {
            NPC[A].Killed = 2;
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.SpeedY;
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                NPC[A].Location.Y += NPCHeight[NPC[A].Type] + 1.5;
                NPC[A].Special2 = 4;
                NPC[A].Special = 70;
            }
            if(NPC[A].Special2 == 1)
            {
                NPC[A].Special = NPC[A].Special + 1;
                NPC[A].Location.Y -= 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Location.Y = std::round(NPC[A].Location.Y);
                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                if(NPC[A].Type != 256)
                    NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 50)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special += 1;
                NPC[A].Location.Y += 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Special2 = 4;
                    if(NPC[A].Type == 256)
                        NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 75)
                {
                    tempTurn = true;
                    if(!NPC[A].Inert)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                            {
                                if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                {
                                    tempTurn = false;
                                    break;
                                }
                            }
                        }
                    }
                    if(NPC[A].Type == 256)
                        tempTurn = true;
                    if(tempTurn)
                    {
                        NPC[A].Special2 = 1;
                        NPC[A].Special = 0;
                    }
                    else
                        NPC[A].Special = 140;
                }
            }
            NPC[A].Location.Height = NPCHeight[NPC[A].Type] - (NPC[A].Location.Y - NPC[A].DefaultLocation.Y);
            if(NPC[A].Location.Height < 0)
                NPC[A].Location.Height = 0;
            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }
    // down piranha plant
    }
    else if(NPC[A].Type == 51 || NPC[A].Type == 257)
    {
        if(NPC[A].Special3 > 0)
            NPC[A].Special3 -= 1;
        if(NPC[A].Location.X != NPC[A].DefaultLocation.X)
        {
            NPC[A].Killed = 2;
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.SpeedY;
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                // .Location.Y = .Location.Y - NPCHeight(.Type) - 1.5
                NPC[A].Location.Height = 0;
                NPC[A].Special2 = 1;
                NPC[A].Special = 0;
            }
            else if(NPC[A].Special2 == 1)
            {
                NPC[A].Special += 1;
                // .Location.Y = .Location.Y + 1.5
                NPC[A].Location.Height += 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Location.Height = std::floor(NPC[A].Location.Height);
                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                if(NPC[A].Type != 257)
                    NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 50)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special += 1;
                // .Location.Y = .Location.Y - 1.5
                NPC[A].Location.Height -= 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Location.Height = 0;
                    NPC[A].Special2 = 4;
                }
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special += 1;
                if(NPC[A].Special >= 110)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Special = 0;
                }
            }
            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }
    // left/right piranha plant
    }
    else if(NPC[A].Type == 52)
    {
        NPC[A].Direction = NPC[A].DefaultDirection;
        if(NPC[A].Location.Y != NPC[A].DefaultLocation.Y)
        {
            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.SpeedY;
            NPCHit(A, 4);
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                if(NPC[A].Direction == 1)
                {
                    // .Location.x = .Location.X - NPCWidth(.Type) - 1.5
                    NPC[A].Location.Width = NPC[A].Location.Width - NPCWidth[NPC[A].Type] - 1.5;
                }
                else
                    NPC[A].Location.X = NPC[A].Location.X + NPCWidth[NPC[A].Type] + 1.5;
                NPC[A].Special2 = 1;
                NPC[A].Special = 0;
            }
            else if(NPC[A].Special2 == 1)
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Direction == -1)
                    NPC[A].Location.X += 1.5 * NPC[A].Direction;
                else
                    NPC[A].Location.Width += 1.5 * NPC[A].Direction;
                if(NPC[A].Special >= NPCWidth[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Location.Width = std::floor(NPC[A].Location.Width);
                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 50)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Direction == -1)
                    NPC[A].Location.X -= 1.5 * NPC[A].Direction;
                else
                    NPC[A].Location.Width -= 1.5 * NPC[A].Direction;
                if(NPC[A].Special >= NPCWidth[NPC[A].Type] * 0.65 + 1)
                {
                    NPC[A].Special2 = 4;
                    NPC[A].Location.Width = 0;
                }
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 110)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Special = 0;
                }
            }
            if(NPC[A].Direction == -1)
            {
                NPC[A].Location.Width = NPCWidth[NPC[A].Type] - (NPC[A].Location.X - NPC[A].DefaultLocation.X);
                if(NPC[A].Location.Width < 0)
                    NPC[A].Location.Width = 0;
            }
            if(NPC[A].Location.Width == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;

        }
    // smb3 belt code
    }
    else if(NPC[A].Type == 57)
    {
        NPC[A].Location.SpeedX = 0.8 * NPC[A].DefaultDirection * BeltDirection;
        NPC[A].Location.X = NPC[A].DefaultLocation.X;
        NPC[A].Location.Y = NPC[A].DefaultLocation.Y;
        NPC[A].Direction = NPC[A].DefaultDirection * BeltDirection;
    }
    else if(NPC[A].Type == 75)
    {
        if(NPC[A].Location.SpeedY == Physics.NPCGravity)
        {
            NPC[A].Special = NPC[A].Special + 1;
            NPC[A].Frame = 0;
            if(NPC[A].Special >= 100)
                NPC[A].Special = 1;
            else if(NPC[A].Special >= 10)
            {
                NPC[A].Special = 0;
                NPC[A].Frame = 1;
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
                NPC[A].Location.SpeedY = -4.6;
            }
        }
        else
        {
            if(NPC[A].Special <= 8)
            {
                NPC[A].Special = NPC[A].Special + 1;
                NPC[A].Frame = 1;
            }
            else
            {
                NPC[A].Frame = 2;
                NPC[A].Special = 100;
            }

        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 3;
    }
    // Fireball code (Podoboo)
    else if(NPC[A].Type == 12)
    {
        if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y + NPC[A].Location.Height + 16)
            NPC[A].Location.Y = NPC[A].DefaultLocation.Y + NPC[A].DefaultLocation.Height + 16;

        NPC[A].Projectile = true;

        // If .Location.X <> .DefaultLocation.X Then .Killed = 2
        if(NPC[A].Special2 == 0.0)
        {
            NPC[A].Location.Y = NPC[A].DefaultLocation.Y + NPC[A].Location.Height + 1.5;
            NPC[A].Special2 = 1;
            NPC[A].Special = 0;
            PlaySound(16);
            tempLocation = NPC[A].Location;
            tempLocation.Y -= 32;
            NewEffect(13, tempLocation);
        }
        else if(fEqual(NPC[A].Special2, 1))
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -6;

            if(NPC[A].Location.Y < NPC[A].DefaultLocation.Y - 10)
            {
                if(int(NPC[A].Special) % 5 == 0) {
                    NewEffect(12, NPC[A].Location);
                }
            }

            if(NPC[A].Special >= 30)
            {
                NPC[A].Special2 = 2;
                NPC[A].Special = 0;
            }
        }
        else if(fEqual(NPC[A].Special2, 2))
        {
            NPC[A].Special += 1;

            if(fEqual(NPC[A].Special, 61))
            {
                tempLocation = NPC[A].Location;
                tempLocation.Y = tempLocation.Y + 2;
                NewEffect(13, tempLocation);
                PlaySound(16);
            }

            if(NPC[A].Special >= 150)
            {
                NPC[A].Special2 = 0;
                NPC[A].Special = 0;
            }
        }

        if(NPC[A].Location.Y > level[NPC[A].Section].Height + 1)
            NPC[A].Location.Y = level[NPC[A].Section].Height;
    }
    else if((NPC[A].Type == 46 || NPC[A].Type == 212) && LevelMacro == 0)
    {
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Special2 == 1)
            {
                NPC[A].Special3 = NPC[A].Special3 + 1;
                NPC[A].Special2 = 0;
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Direction * 2;
                if(NPC[A].Location.X >= NPC[A].DefaultLocation.X + 2)
                    NPC[A].Direction = -1;
                if(NPC[A].Location.X <= NPC[A].DefaultLocation.X - 2)
                    NPC[A].Direction = 1;
            }
            else
            {
                if(NPC[A].Special3 > 0)
                    NPC[A].Special3 = NPC[A].Special3 - 1;
                NPC[A].Location.X = NPC[A].DefaultLocation.X;
            }
            if((NPC[A].Special3 >= 5 && NPC[A].Type == 46) || (NPC[A].Special3 >= 30 && NPC[A].Type == 212))
            {
                NPC[A].Special = 1;
                NPC[A].Location.X = NPC[A].DefaultLocation.X;
            }
        }
    // Big Koopa Code
    }
    else if(NPC[A].Type == 15)
    {
        if(NPC[A].Legacy)
        {
            if(NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;
            if(bgMusic[NPC[A].Section] != 6 && bgMusic[NPC[A].Section] != 15 && bgMusic[NPC[A].Section] != 21 && NPC[A].TimeLeft > 1)
            {
                bgMusic[NPC[A].Section] = 6;
                StopMusic();
                StartMusic(NPC[A].Section);
            }
        }
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.Height != 54)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 54;
                NPC[A].Location.Height = 54;
            }
            NPC[A].Special2 = NPC[A].Special2 + dRand() * 2;
            if(NPC[A].Special2 >= 250 + (iRand() % 250))
            {
                NPC[A].Special = 2;
                NPC[A].Special2 = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 10)
            {
                NPC[A].Special = 1;
                NPC[A].Special2 = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            if(NPC[A].Location.Height != 40)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 40;
                NPC[A].Location.Height = 40;
            }
            NPC[A].Special2 = NPC[A].Special2 + dRand() * 2;
            if(NPC[A].Special2 >= 100 + (iRand() % 100))
            {
                NPC[A].Special = 3;
                NPC[A].Special2 = 0;
            }
        }
        else if(NPC[A].Special == 3)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 10)
            {
                NPC[A].Special = 0;
                NPC[A].Special2 = 0;
            }
        }
        else if(NPC[A].Special == 4)
        {
            if(NPC[A].Location.Height != 34)
            {
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - 34;
                NPC[A].Location.Height = 34;
            }
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 100)
            {
                NPC[A].Special = 1;
                NPC[A].Special2 = 0;
            }
        }
    }
    else if(NPCIsAParaTroopa[NPC[A].Type]) // para-troopas
    {
        if(NPC[A].Special == 0) // chase
        {
            if(NPC[A].CantHurt > 0)
                NPC[A].CantHurt = 100;
            NPC[A].Projectile = false;
            C = 0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0 && NPC[A].CantHurtPlayer != B)
                {
                    if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = B;
                    }
                }
            }
            C = D;
            if(C > 0)
            {
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[C].Location.X + Player[C].Location.Width / 2.0)
                    D = -1;
                else
                    D = 1;
                NPC[A].Direction = D;
                E = 0; // X
                F = -1; // Y
                if(NPC[A].Location.Y > Player[C].Location.Y)
                    F = -1;
                else if(NPC[A].Location.Y < Player[C].Location.Y - 128)
                    F = 1;
                if(NPC[A].Location.X > Player[C].Location.X + Player[C].Location.Width + 64)
                    E = -1;
                else if(NPC[A].Location.X + NPC[A].Location.Width + 64 < Player[C].Location.X)
                    E = 1;
                if(NPC[A].Location.X + NPC[A].Location.Width + 150 > Player[C].Location.X && NPC[A].Location.X - 150 < Player[C].Location.X + Player[C].Location.Width)
                {
                    if(NPC[A].Location.Y > Player[C].Location.Y + Player[C].Location.Height)
                    {

                        // If Player(C).Location.SpeedX + NPC(Player(C).StandingOnNPC).Location.SpeedX > 0 And .Location.X + .Location.Width / 2 > Player(C).Location.X + Player(C).Location.Width / 2 Then
                            // E = -D
                        // ElseIf Player(C).Location.SpeedX + NPC(Player(C).StandingOnNPC).Location.SpeedX <= 0 And .Location.X + .Location.Width / 2 < Player(C).Location.X + Player(C).Location.Width / 2 Then
                            E = -D;
                        // End If
                        if(NPC[A].Location.Y < Player[C].Location.Y + Player[C].Location.Height + 160)
                        {
                            if(NPC[A].Location.X + NPC[A].Location.Width + 100 > Player[C].Location.X && NPC[A].Location.X - 100 < Player[C].Location.X + Player[C].Location.Width)
                                F = 0.2F;
                        }
                    }
                    else
                    {
                        if(NPC[A].Direction != D)
                            E = D;
                        E = D;
                        F = 1;
                    }
                }
                if(NPC[A].Wet == 2)
                {
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.025 * E;
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.025 * F;
                }
                else
                {
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05 * E;
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05 * F;
                }
                if(NPC[A].Location.SpeedX > 4)
                    NPC[A].Location.SpeedX = 4;
                else if(NPC[A].Location.SpeedX < -4)
                    NPC[A].Location.SpeedX = -4;
                if(NPC[A].Location.SpeedY > 3)
                    NPC[A].Location.SpeedY = 3;
                else if(NPC[A].Location.SpeedY < -3)
                    NPC[A].Location.SpeedY = -3;
            }
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;
        }
        else if(NPC[A].Special == 2)
        {

            if(NPC[A].Special3 == 0)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05;
                if(NPC[A].Location.SpeedY > 1)
                    NPC[A].Special3 = 1;
            }
            else
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.05;
                if(NPC[A].Location.SpeedY < -1)
                    NPC[A].Special3 = 0;
            }

            if(NPC[A].Location.X == NPC[A].DefaultLocation.X && NPC[A].Location.SpeedX == 0)
                NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
            if(NPC[A].Location.X < NPC[A].DefaultLocation.X - 64)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.02;
            else if(NPC[A].Location.X > NPC[A].DefaultLocation.X + 64)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.02;
            else if(NPC[A].Direction == -1)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.02;
            else if(NPC[A].Direction == 1)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.02;
            if(NPC[A].Location.SpeedX > 2)
                NPC[A].Location.SpeedX = 2;
            if(NPC[A].Location.SpeedX < -2)
                NPC[A].Location.SpeedX = -2;
        }
        else if(NPC[A].Special == 3)
        {
            NPC[A].Location.SpeedX = 0;
            if(NPC[A].Location.Y == NPC[A].DefaultLocation.Y && NPC[A].Location.SpeedY == 0)
                NPC[A].Location.SpeedY = 2 * NPC[A].Direction;
            if(NPC[A].Location.Y < NPC[A].DefaultLocation.Y - 64)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.02;
            else if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y + 64)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.02;
            else if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.02;
            else
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.02;
            if(NPC[A].Location.SpeedY > 2)
                NPC[A].Location.SpeedY = 2;
            if(NPC[A].Location.SpeedY < -2)
                NPC[A].Location.SpeedY = -2;
            C = 0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = B;
                    }
                }
            }
            if(Player[D].Location.X + Player[D].Location.Width / 2.0 > NPC[A].Location.X + 16)
                NPC[A].Direction = 1;
            else
                NPC[A].Direction = -1;
        }
        if(NPC[A].Stuck && NPC[A].Projectile == 0)
            NPC[A].Location.SpeedX = 0;
        NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.SpeedX;
        NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.SpeedY;
    // Jumpy bee thing
    }
    else if(NPC[A].Type == 54)
    {
        if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Special = NPC[A].Special + 1;
            if(NPC[A].Special == 30)
            {
                NPC[A].Special = 0;
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
                NPC[A].Location.SpeedY = -6;
                NPC[A].Location.SpeedX = 1.4 * NPC[A].Direction;
            }
        }
    // Bouncy Star thing code
    }
    else if(NPC[A].Type == 25)
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
            {
                if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                }
            }
        }
        if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
        {
            NPC[A].Special = NPC[A].Special + 1;
            if(NPC[A].Special == 8)
            {
                NPC[A].Location.SpeedY = -7;
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
                NPC[A].Special = 0;
            }
        }
        else
            NPC[A].Special = 0;
    // bowser statue
    }
    else if(NPC[A].Type == 84 || NPC[A].Type == 181)
    {
        NPC[A].Special = NPC[A].Special + 1;
        if(NPC[A].Special >= 200 + dRand() * 200)
        {
            NPC[A].Special = 0;
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Inert = NPC[A].Inert;
            NPC[numNPCs].Type = 85;
            NPC[numNPCs].Direction = NPC[A].Direction;
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y + 16;
            NPC[numNPCs].Location.X = NPC[A].Location.X + 24 * NPC[numNPCs].Direction;
            if(NPC[A].Type == 181)
            {
                NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 5;
                NPC[numNPCs].Location.X = NPC[A].Location.X + 6 + 30 * NPC[numNPCs].Direction;
            }
            NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
            if(NPC[numNPCs].Direction == 1)
                NPC[numNPCs].Frame = 4;
            NPC[numNPCs].FrameCount = iRand() % 8;
            PlaySound(42);
        }
    // Hammer Bro
    }
    else if(NPC[A].Type == 29 && NPC[A].Projectile == 0)
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
            {
                if(C == 0.f || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                }
            }
        }
        if(NPC[A].Special > 0)
        {
            NPC[A].Special = NPC[A].Special + 1;
            NPC[A].Location.SpeedX = 0.6;
            if(NPC[A].Special >= 100 && NPC[A].Location.SpeedY == Physics.NPCGravity)
                NPC[A].Special = -1;
        }
        else
        {
            NPC[A].Special = NPC[A].Special - 1;
            NPC[A].Location.SpeedX = -0.6;
            if(NPC[A].Special <= -100 && NPC[A].Location.SpeedY == Physics.NPCGravity)
                NPC[A].Special = 1;
        }
        if(NPC[A].Location.SpeedY == Physics.NPCGravity)
        {
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 >= 250)
            {
                NPC[A].Location.SpeedY = -7;
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
                NPC[A].Special2 = 0;
            }
        }
        NPC[A].Special3 = NPC[A].Special3 + dRand() * 2;
        if(NPC[A].Special3 >= 50 + dRand() * 1000)
        {
            if(NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Location.SpeedY = -3;
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
            }
            PlaySound(25);
            NPC[A].Special3 = -15;
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Inert = NPC[A].Inert;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 32;
            NPC[numNPCs].Location.X = NPC[A].Location.X;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y;
            NPC[numNPCs].Direction = NPC[A].Direction;
            NPC[numNPCs].Type = 30;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 50;
            NPC[numNPCs].Location.SpeedY = -8;
            NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;
        }
    // leaf
    }
    else if(NPC[A].Type == 34) // Leaf
    {
        if(NPC[A].Stuck && NPC[A].Projectile == 0)
            NPC[A].Location.SpeedX = 0;
        else if(NPC[A].Stuck)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            if(NPC[A].Location.SpeedY >= 8)
                NPC[A].Location.SpeedY = 8;
        }
        else if(NPC[A].Special == 0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
            if(NPC[A].Projectile != 0)
            {
                if(NPC[A].Location.SpeedY >= 2)
                {
                    NPC[A].Location.SpeedX = 1.2;
                    NPC[A].Special = 1;
                    NPC[A].Projectile = false;
                }

            }
            else if(NPC[A].Location.SpeedY >= 0)
                NPC[A].Special = 6;
        }
        else
        {
            if(NPC[A].Special == 1)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.25;
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.3;
                if(NPC[A].Location.SpeedY <= 0)
                    NPC[A].Special = 2;
            }
            else if(NPC[A].Special == 2)
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.3;
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.02;
                if(NPC[A].Location.SpeedX <= 0)
                {
                    NPC[A].Special = 3;
                    NPC[A].Location.SpeedX = 0;
                }
            }
            else if(NPC[A].Special == 3)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.4;
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.1;
                if(NPC[A].Location.SpeedY >= 3)
                    NPC[A].Special = 4;
            }
            else if(NPC[A].Special == 4)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.25;
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.3;
                if(NPC[A].Location.SpeedY <= 0)
                    NPC[A].Special = 5;
            }
            else if(NPC[A].Special == 5)
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.3;
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.02;
                if(NPC[A].Location.SpeedX >= 0)
                {
                    NPC[A].Special = 6;
                    NPC[A].Location.SpeedX = 0;
                }
            }
            else if(NPC[A].Special == 6)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.4;
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1;
                if(NPC[A].Location.SpeedY >= 3)
                    NPC[A].Special = 1;
            }
        }
    }
    else if(NPC[A].Type == 47) // lakitu
    {
        NPC[A].Projectile = false;
        if(NPC[A].TimeLeft > 1)
            NPC[A].TimeLeft = 100;
        if(NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && B != NPC[A].CantHurtPlayer)
            {
                if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    D = B;
                }
            }
        }
        C = D;
        if(C > 0)
        {
            if(NPC[A].Special == 0)
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
                D = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D = D + std::abs(Player[C].Location.SpeedX) / 2;
                if(NPC[A].Location.SpeedX < -5 - D)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[C].Location.X + Player[C].Location.Width / 2.0 - 50 + (Player[C].Location.SpeedX * 15))
                    NPC[A].Special = 1;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                D = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D = D + std::abs(Player[C].Location.SpeedX) / 2;
                if(NPC[A].Location.SpeedX > 5 + D)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[C].Location.X + Player[C].Location.Width / 2.0 + 50 + (Player[C].Location.SpeedX * 15))
                    NPC[A].Special = 0;
            }
            D = 1;
            if(numPlayers == 2)
            {
                if(ScreenType == 5)
                {
                    if(DScreenType != 5)
                        D = 2;
                }
            }
            if(NPC[A].Location.Y + NPC[A].Location.Height > Player[C].Location.Y - 248)
                NPC[A].Special2 = 1;
            if(NPC[A].Location.Y + NPC[A].Location.Height < Player[C].Location.Y - 256 || NPC[A].Location.Y < -vScreenY[D])
                NPC[A].Special2 = 0;
            if(NPC[A].Location.Y > -vScreenY[D] + 64)
                NPC[A].Special2 = 1;
            if(NPC[A].Location.Y < -vScreenY[D] + 72)
                NPC[A].Special2 = 0;
            if(NPC[A].Special2 == 0)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05;
                if(NPC[A].Location.SpeedY > 2)
                    NPC[A].Location.SpeedY = 2;
            }
            else
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.05;
                if(NPC[A].Location.SpeedY < -2)
                    NPC[A].Location.SpeedY = -2;
            }
            if(NPC[A].Inert)
            {
                if(NPC[A].Special3 > 1)
                    NPC[A].Special3 = 0;
            }
            if(NPC[A].Special3 == 0)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 10)
                {
                    NPC[A].FrameCount = 0;
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 2)
                        NPC[A].Special3 = 1;
                }
            }
            else if(NPC[A].Special3 == 1)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 10)
                {
                    NPC[A].FrameCount = 0;
                    NPC[A].Frame = NPC[A].Frame - 1;
                    if(NPC[A].Frame <= 0)
                        NPC[A].Special3 = 0;
                }
            }
            else if(NPC[A].Special3 == 2)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 16)
                {
                    NPC[A].FrameCount = 10;
                    if(NPC[A].Frame < 5)
                        NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame <= 5)
                        NPC[A].Special5 = NPC[A].Special5 + 1;
                }
            }
            else if(NPC[A].Special3 == 3)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 2)
                {
                    NPC[A].FrameCount = 0;
                    NPC[A].Frame = NPC[A].Frame - 1;
                    if(NPC[A].Frame <= 0)
                    {
                        NPC[A].Special3 = 0;
                        NPC[A].Frame = 0;
                    }
                }
            }
            if(std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) < 100)
            {
                if(NPC[A].Special4 == 0)
                {
                    NPC[A].Special3 = 2;
                    NPC[A].Special4 = 100;
                }
            }
        }
        if(NPC[A].Special4 > 0)
            NPC[A].Special4 = NPC[A].Special4 - 1;
        if(NPC[A].Special5 >= 20)
        {
            NPC[A].Special5 = 20;
            tempLocation = NPC[A].Location;
            tempLocation.X = tempLocation.X - 16;
            tempLocation.Y = tempLocation.Y - 16;
            tempLocation.Width = tempLocation.Width + 32;
            tempLocation.Height = tempLocation.Height + 32;
            D = 0;

            if(NPC[A].Location.Y + NPC[A].Location.Height > Player[C].Location.Y)
                D = 1;
            else
            {
                for(int Ei = 1; Ei <= numBlock; Ei++)
                {
                    if(!BlockNoClipping[Block[E].Type] &&
                       !BlockIsSizable[Block[E].Type] &&
                       !BlockOnlyHitspot1[Block[E].Type])
                    {
                        if(CheckCollision(tempLocation, Block[Ei].Location))
                            D = 1;
                    }
                }
            }

            if(D == 0)
            {
                NPC[A].Special3 = 3;
                NPC[A].FrameCount = 0;
                NPC[A].Special5 = 0;
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[C].Location.X + Player[C].Location.Width / 2.0)
                    NPC[numNPCs].Direction = -1;
                else
                    NPC[numNPCs].Direction = 1;
                NPC[numNPCs].Location = NPC[A].Location;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                if(NPC[A].CantHurt > 0)
                {
                    NPC[numNPCs].CantHurt = 100;
                    NPC[numNPCs].CantHurtPlayer = NPC[A].CantHurtPlayer;
                }
                NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y + 8;
                NPC[numNPCs].Location.SpeedX = (1.5 + std::abs(Player[C].Location.SpeedX) * 0.75) * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedY = -8;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Type = 48;
                tempNPC = NPC[A];
                NPC[A] = NPC[numNPCs];
                NPC[numNPCs] = tempNPC;
                PlaySound(25);
            }
        }
    }
    else if(NPC[A].Type == 166) // smw goomba
    {
        NPC[A].Special = NPC[A].Special + 1;
        if(NPC[A].Special >= 400)
        {
            if(NPC[A].Slope > 0 || NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Location.SpeedY == 0)
            {
                NPC[A].Location.SpeedY = -5;
                NPC[A].Type = 165;
                NPC[A].Special = 0;
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
            }
        }
    }
    else if(NPC[A].Type == 37 || NPC[A].Type == 180) // thwomp
    {
            if(NPC[A].Special == 0)
            {
                NPC[A].Location.SpeedY = 0;
                NPC[A].Location.Y = NPC[A].DefaultLocation.Y;
                C = 0;
                for(B = 1; B <= numPlayers; B++)
                {
                    if(!CanComeOut(NPC[A].Location, Player[B].Location) && Player[B].Location.Y >= NPC[A].Location.Y)
                        C = B;
                }
                if(C > 0)
                    NPC[A].Special = 1;
            }
            else if(NPC[A].Special == 1)
                NPC[A].Location.SpeedY = 6;
            else if(NPC[A].Special == 2)
            {
                if(NPC[A].Special2 == 0)
                {
                    PlaySound(37);
                    tempLocation.Width = 32;
                    tempLocation.Height = 32;
                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 16;


                    tempLocation.X = NPC[A].Location.X;
                    NewEffect(10, tempLocation);
                    Effect[numEffects].Location.SpeedX = -1.5;

                    tempLocation.X = tempLocation.X + tempLocation.Width - EffectWidth[10];
                    NewEffect(10, tempLocation);
                    Effect[numEffects].Location.SpeedX = 1.5;

                }
                NPC[A].Location.SpeedY = 0;
                if(NPC[A].Slope > 0)
                    NPC[A].Location.Y = NPC[A].Location.Y - 0.1;
                NPC[A].Special2 = NPC[A].Special2 + 1;
                if(NPC[A].Special2 >= 100)
                {
                    NPC[A].Location.Y = NPC[A].Location.Y - 1;
                    NPC[A].Special = 3;
                    NPC[A].Special2 = 0;
                }
            }
            else if(NPC[A].Special == 3)
            {
                NPC[A].Location.SpeedY = -2;
                if(NPC[A].Location.Y <= NPC[A].DefaultLocation.Y + 1)
                {
                    NPC[A].Location.Y = NPC[A].DefaultLocation.Y;
                    NPC[A].Location.SpeedY = 0;
                    NPC[A].Special = 0;
                    NPC[A].Special2 = 0;
                }
            }
        // End If
    }
    else if(NPC[A].Type == 38 || NPC[A].Type == 43 || NPC[A].Type == 44) // boo
    {
            if(BattleMode && NPC[A].CantHurt > 0)
                NPC[A].CantHurt = 100;
            if(NPC[A].Projectile != 0)
            {
                if(NPC[A].CantHurtPlayer > 0)
                    NPC[A].BattleOwner = NPC[A].CantHurtPlayer;
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95;
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
                if(NPC[A].Location.SpeedX > -2 && NPC[A].Location.SpeedX < 2)
                {
                    if(NPC[A].Location.SpeedY > -2 && NPC[A].Location.SpeedY < 2)
                        NPC[A].Projectile = false;
                }
            }
            C = 0;
            D = 0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section && B != NPC[A].CantHurtPlayer)
                {
                    if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = B;
                    }
                }
            }
            C = D;
            if(C > 0)
            {
                D = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                E = Player[C].Location.X + Player[C].Location.Width / 2.0;
                if((D <= E && Player[C].Direction == -1) || Player[C].SpinJump)
                {
                    NPC[A].Special = 0;
                    if(NPC[A].Type == 38)
                    {
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.9;
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.9;
                    }
                    else if(NPC[A].Type == 43)
                    {
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.85;
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.85;
                    }
                    else if(NPC[A].Type == 44)
                    {
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.8;
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.8;
                    }
                    if(NPC[A].Location.SpeedX < 0.1 && NPC[A].Location.SpeedX > -0.1)
                        NPC[A].Location.SpeedX = 0;
                    if(NPC[A].Location.SpeedY < 0.1 && NPC[A].Location.SpeedY > -0.1)
                        NPC[A].Location.SpeedY = 0;
                }
                else if((D >= E && Player[C].Direction == 1) || Player[C].SpinJump)
                {
                    NPC[A].Special = 0;
                    if(NPC[A].Type == 38)
                    {
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.9;
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.9;
                    }
                    else if(NPC[A].Type == 43)
                    {
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.85;
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.85;
                    }
                    else if(NPC[A].Type == 44)
                    {
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.8;
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.8;
                    }
                    if(NPC[A].Location.SpeedX < 0.1 && NPC[A].Location.SpeedX > -0.1)
                        NPC[A].Location.SpeedX = 0;
                    if(NPC[A].Location.SpeedY < 0.1 && NPC[A].Location.SpeedY > -0.1)
                        NPC[A].Location.SpeedY = 0;
                }
                else
                {
                    NPC[A].Special = 1;
                    NPC[A].Direction = Player[C].Direction;
                    if(NPC[A].Type == 38)
                        F = 0.03F;
                    else if(NPC[A].Type == 43)
                        F = 0.025F;
                    else if(NPC[A].Type == 44)
                        F = 0.02F;
                    if(D <= E && NPC[A].Location.SpeedX < 1.5)
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + F;
                    else if(NPC[A].Location.SpeedX > -1.5)
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - F;
                    D = NPC[A].Location.Y + NPC[A].Location.Height / 2.0;
                    E = Player[C].Location.Y + Player[C].Location.Height / 2.0;
                    if(D <= E && NPC[A].Location.SpeedY < 1.5)
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + F;
                    else if(NPC[A].Location.SpeedY > -1.5)
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - F;
                }
            }
            else
            {
                NPC[A].Special = 0;
                NPC[A].Location.SpeedX = 0;
                NPC[A].Location.SpeedY = 0;
            }
        // End If
    }
    else if(NPC[A].Type == 97 || NPC[A].Type == 196)
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].Special4 = NPC[A].Special4 + 1;
            if(NPC[A].Special4 >= 5)
            {
                NPC[A].Special4 = 0;
                NewEffect(80, newLoc(NPC[A].Location.X + dRand() * NPC[A].Location.Width - 2, NPC[A].Location.Y + dRand() * NPC[A].Location.Height));
                Effect[numEffects].Location.SpeedX = dRand() * 1.0 - 0.5;
                Effect[numEffects].Location.SpeedY = dRand() * 1.0 - 0.5;
            }
        }
        else
        {
            NPC[A].Special4 = NPC[A].Special4 + 1;
            if(NPC[A].Special4 >= 10)
            {
                NPC[A].Special4 = 0;
                NewEffect(80, newLoc(NPC[A].Location.X + dRand() * NPC[A].Location.Width - 2, NPC[A].Location.Y + dRand() * NPC[A].Location.Height));
                Effect[numEffects].Location.SpeedX = dRand() * 1.0 - 0.5;
                Effect[numEffects].Location.SpeedY = dRand() * 1.0 - 0.5;
                Effect[numEffects].Frame = 1;
            }
        }
        if(NPC[A].Special2 == 0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.04;
            if(NPC[A].Location.SpeedY <= -1.4)
                NPC[A].Special2 = 1;
        }
        else
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.04;
            if(NPC[A].Location.SpeedY >= 1.4)
                NPC[A].Special2 = 0;
        }
        if(NPC[A].Special3 == 0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.03;
            if(NPC[A].Location.SpeedX <= -0.6)
                NPC[A].Special3 = 1;
        }
        else
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.03;
            if(NPC[A].Location.SpeedX >= 0.6)
                NPC[A].Special3 = 0;
        }
    }
    else if(NPC[A].Type == 39 && NPC[A].Projectile == 0) // birdo
    {
        if(NPC[A].Legacy)
        {
            if(NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;
            if(bgMusic[NPC[A].Section] != 6 && bgMusic[NPC[A].Section] != 15 && bgMusic[NPC[A].Section] != 21 && NPC[A].TimeLeft > 1)
            {
                bgMusic[NPC[A].Section] = 15;
                StopMusic();
                StartMusic(NPC[A].Section);
            }
        }
        if(NPC[A].Special >= 0)
        {
            C = 0;
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                    }
                }
            }
            NPC[A].Special2 = NPC[A].Special2 + 1;
            if(NPC[A].Special2 == 125)
            {
                NPC[A].Location.Y = NPC[A].Location.Y - 1;
                NPC[A].Location.SpeedY = -5;
                if(NPC[A].Inert)
                    NPC[A].Special2 = 0;
            }
            else if(NPC[A].Special2 >= 240)
            {
                if(NPC[A].Special2 == 260)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].Type = 40;

                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 14 - NPC[numNPCs].Location.Height / 2.0;



                    if(NPC[numNPCs].Direction == 1)
                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                    else
                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
                    PlaySound(38);
                }
                NPC[A].Special = 1;
                if(NPC[A].Special2 > 280)
                {
                    NPC[A].Special2 = 0;
                    NPC[A].Special = 0;
                }
            }
            if(NPC[A].Special == 0 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special3 = NPC[A].Special3 + 1;
                if(NPC[A].Special3 <= 200)
                    NPC[A].Location.SpeedX = -1;
                else if(NPC[A].Special3 > 500)
                    NPC[A].Special3 = 0;
                else if(NPC[A].Special3 > 250 && NPC[A].Special3 <= 450)
                    NPC[A].Location.SpeedX = 1;
                else
                    NPC[A].Location.SpeedX = 0;
            }
            else
                NPC[A].Location.SpeedX = 0;
        }
        else
        {
            NPC[A].Special = NPC[A].Special + 1;
            NPC[A].Location.SpeedX = 0;
        }
        if(NPC[A].Stuck)
            NPC[A].Location.SpeedX = 0;


    }
    else if(NPC[A].Type == 284) // smw lakitu
    {
        if(NPC[A].Special == 0)
            NPC[A].Special = NPC[A].Type;
        NPC[A].Projectile = false;
        if(NPC[A].TimeLeft > 1)
            NPC[A].TimeLeft = 100;
        if(NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && B != NPC[A].CantHurtPlayer && Player[B].TimeToLive == 0)
            {
                if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    D = B;
                }
            }
        }
        C = D;
        if(C > 0)
        {
            if(NPC[A].Special6 == 0)
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
                D = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D = D + std::abs(Player[C].Location.SpeedX) / 2;
                if(NPC[A].Location.SpeedX < -5 - D)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[C].Location.X + Player[C].Location.Width / 2.0 - 50 + (Player[C].Location.SpeedX * 15))
                    NPC[A].Special6 = 1;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                D = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D = D + std::abs(Player[C].Location.SpeedX) / 2;
                if(NPC[A].Location.SpeedX > 5 + D)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[C].Location.X + Player[C].Location.Width / 2.0 + 50 + (Player[C].Location.SpeedX * 15))
                    NPC[A].Special6 = 0;
            }
            D = 1;
            if(numPlayers == 2)
            {
                if(ScreenType == 5)
                {
                    if(DScreenType != 5)
                        D = 2;
                }
            }
            if(NPC[A].Location.Y + NPC[A].Location.Height > Player[C].Location.Y - 248)
                NPC[A].Special2 = 1;
            if(NPC[A].Location.Y + NPC[A].Location.Height < Player[C].Location.Y - 256 || NPC[A].Location.Y < -vScreenY[D])
                NPC[A].Special2 = 0;
            if(NPC[A].Location.Y > -vScreenY[D] + 64)
                NPC[A].Special2 = 1;
            if(NPC[A].Location.Y < -vScreenY[D] + 72)
                NPC[A].Special2 = 0;
            if(NPC[A].Special2 == 0)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05;
                if(NPC[A].Location.SpeedY > 2)
                    NPC[A].Location.SpeedY = 2;
            }
            else
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.05;
                if(NPC[A].Location.SpeedY < -2)
                    NPC[A].Location.SpeedY = -2;
            }
            if(NPC[A].Inert)
            {
                if(NPC[A].Special3 > 1)
                    NPC[A].Special3 = 0;
            }
            if(std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) < 100)
            {
                if(NPC[A].Special4 == 0)
                {
                    NPC[A].Special3 = 2;
                    NPC[A].Special4 = 100;
                }
            }
        }
        if(NPC[A].Special4 > 0)
            NPC[A].Special4 = NPC[A].Special4 - 1;




        NPC[A].Frame = 0;
        if(NPC[A].FrameCount < 100)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 24)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 32)
                NPC[A].Frame = 1;
            else
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 108)
                NPC[A].Frame = 6;
            else if(NPC[A].FrameCount < 116)
                NPC[A].Frame = 7;
            else if(NPC[A].FrameCount < 124)
                NPC[A].Frame = 8;
            else if(NPC[A].FrameCount < 132)
                NPC[A].Frame = 7;
            else
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 3;
        NPC[A].Special5 = NPC[A].Special5 + 1;
        if(NPC[A].Special5 >= 150)
        {
            NPC[A].Special5 = 150;
            tempLocation = NPC[A].Location;
            tempLocation.X = tempLocation.X - 16;
            tempLocation.Y = tempLocation.Y - 16;
            tempLocation.Width = tempLocation.Width + 32;
            tempLocation.Height = tempLocation.Height + 32;
            D = 0;
            if(NPC[A].Location.Y + NPC[A].Location.Height > Player[C].Location.Y)
                D = 1;
            else
            {
                for(int Ei = 1; Ei <= numBlock; Ei++)
                {
                    if(CheckCollision(tempLocation, Block[Ei].Location) && !BlockNoClipping[Block[E].Type])
                        D = 1;
                }
            }
            if(D == 0)
            {
                NPC[A].FrameCount = 100;
                NPC[A].Special3 = 3;
                NPC[A].Special5 = 0;
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[C].Location.X + Player[C].Location.Width / 2.0)
                    NPC[numNPCs].Direction = -1;
                else
                    NPC[numNPCs].Direction = 1;
                NPC[numNPCs].Type = NPC[A].Special;
                if(NPC[numNPCs].Type == 287)
                    NPC[numNPCs].Type = RandomBonus();
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                if(NPC[A].CantHurt > 0)
                {
                    NPC[numNPCs].CantHurt = 100;
                    NPC[numNPCs].CantHurtPlayer = NPC[A].CantHurtPlayer;
                }
                NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y + 8;
                NPC[numNPCs].Location.SpeedX = (1 + dRand() * 2) * double(NPC[numNPCs].Direction);
                NPC[numNPCs].Location.SpeedY = -7;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].TimeLeft = 100;
                if(NPCIsACoin[NPC[numNPCs].Type])
                {
                    NPC[numNPCs].Special = 1;
                    NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.5;
                }
                // tempNPC = NPC(A)
                // NPC(A) = NPC(numNPCs)
                // NPC(numNPCs) = tempNPC
//                if(MagicHand == true)
//                {
//                    if(NPC[A].Special == NPC[A].Type)
//                        frmNPCs::ShowLak;
//                }
            }
        }
    // nekkid koopa
    }
    else if(NPC[A].Type == 55)
    {
        if(NPC[A].Special > 0)
        {
            NPC[A].Special = NPC[A].Special - 1;
            NPC[A].Location.SpeedX = 0;
        }
    // beach koopa
    }
    else if(NPC[A].Type >= 117 && NPC[A].Type <= 120)
    {
        if(NPC[A].Type == 119 && NPC[A].Special > 0)
        {
            NPC[A].Special = NPC[A].Special - 1;
            NPC[A].Location.SpeedX = 0;
        }
        if(NPC[A].Projectile != 0)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.96;
            if(NPC[A].Location.SpeedX > -0.003 && NPC[A].Location.SpeedX < 0.003)
            {
                NPC[A].Projectile = false;
                NPC[A].Location.Y = NPC[A].Location.Y - Physics.NPCGravity;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Direction = -NPC[A].Direction;
            }
        }
        else
        {
            if(NPC[A].Type != 119)
            {
                if(NPC[A].Location.SpeedY == Physics.NPCGravity)
                {
                    for(B = 1; B <= numNPCs; B++)
                    {
                        if(NPC[B].Active && NPC[B].Section == NPC[A].Section && !NPC[B].Hidden && NPC[B].HoldingPlayer == 0)
                        {
                            if(NPC[B].Type >= 113 && NPC[B].Type <= 116)
                            {
                                tempLocation = NPC[A].Location;
                                tempLocation2 = NPC[B].Location;
                                tempLocation.Width = tempLocation.Width + 32;
                                tempLocation.X = tempLocation.X - 16;
                                if(CheckCollision(tempLocation, tempLocation2))
                                {
                                    NPC[A].Location.Y = NPC[A].Location.Y - Physics.NPCGravity;
                                    NPC[A].Location.SpeedY = -4;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Projectile code
    if(NPCIsAShell[NPC[A].Type] || (NPC[A].Type == 45 && NPC[A].Special == 1))
    {
        if(NPC[A].Location.SpeedX != 0)
            NPC[A].Projectile = true;
    }
    if(NPC[A].Type == 13)
        NPC[A].Projectile = true;
    else if(NPC[A].Type == 17 && NPC[A].CantHurt > 0)
        NPC[A].Projectile = true;
    else if(NPC[A].Type == 12) // Stop the big fireballs from getting killed from tha lava
        NPC[A].Projectile = false;
    else if(NPC[A].Type == 50) // killer plant destroys blocks
    {
        for(B = 1; B <= numBlock; B++)
        {
            if(CheckCollision(NPC[A].Location, Block[B].Location))
            {
                BlockHitHard(B);
            }
        }
    }
}

void CharStuff(int WhatNPC, bool CheckEggs)
{
    bool SMBX = false;
    bool SMB2 = false;
    bool TLOZ = false;
    int A = 0;
    int NPCStart = 0;
    int NPCStop = 0;
    if(GameMenu)
        return;

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Character == 1 || Player[A].Character == 2)
            SMBX = true;
        if(Player[A].Character == 3 || Player[A].Character == 4)
            SMB2 = true;
        if(Player[A].Character == 5)
            TLOZ = true;
    }

    if(WhatNPC == 0)
    {
        NPCStart = 1;
        NPCStop = numNPCs;
    }
    else
    {
        NPCStart = WhatNPC;
        NPCStop = WhatNPC;
    }

    if(!SMBX && SMB2) // Turn SMBX stuff into SMB2 stuff
    {
        for(A = NPCStart; A <= NPCStop; A++)
        {
            if(NPC[A].Type == 96 && NPC[A].Special > 0 && CheckEggs) // Check Eggs
            {
                if(NPCIsYoshi[NPC[A].Special]) // Yoshi into mushroom (Egg)
                {
                    // NPC(A).Special = 249
                    NPC[A].Special = 35; // Yoshi into boot
                }
            }
        }
    }

    if(!SMBX && !SMB2 && TLOZ) // Turn SMBX stuff into Zelda stuff
    {
        for(A = NPCStart; A <= NPCStop; A++)
        {
            if(NPC[A].Active && !NPC[A].Generator && !NPC[A].Inert)
            {
                if(NPC[A].Type == 9 || NPC[A].Type == 184 || NPC[A].Type == 185 || NPCIsBoot[NPC[A].Type]) // turn mushrooms into hearts
                {
                    NPC[A].Frame = 0;
                    NPC[A].Type = 250;
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - NPCHeight[NPC[A].Type] - 1;
                    NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPCWidth[NPC[A].Type] / 2.0;
                    NPC[A].Location.Width = 32;
                    NPC[A].Location.Height = 32;
                }
                else if(NPC[A].Type == 10 || NPC[A].Type == 33 || NPC[A].Type == 88 || NPC[A].Type == 138 || NPC[A].Type == 258) // turn coins into rupees
                {
                    if(NPC[A].Type == 258)
                        NPC[A].Type = 252;
                    else
                        NPC[A].Type = 251;
                    NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - NPCHeight[NPC[A].Type];
                    NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPCWidth[NPC[A].Type] / 2.0;
                    NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                    NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                    NPC[A].Frame = 0;
                }
            }
            if(NPC[A].Type == 96 && NPC[A].Special > 0 && CheckEggs) // Check Eggs
            {
                if(NPCIsYoshi[NPC[A].Special] || NPCIsBoot[NPC[A].Special]) // Yoshi / boot into mushroom (Egg)
                    NPC[A].Special = 250;
                if(NPC[A].Special == 9 || NPC[A].Special == 184 || NPC[A].Special == 185) // mushrooms into hearts (eggs)
                    NPC[A].Special = 250;
                if(NPC[A].Special == 10 || NPC[A].Special == 33 || NPC[A].Special == 88 || (!SMB2 && NPC[A].Special == 138)) // coins into rupees (eggs)
                    NPC[A].Special = 251;
            }
        }
    }
}

int RandomBonus()
{
    int B = iRand() % 6;
    switch(B)
    {
    case 0:
        return 9;
    case 1:
        return 14;
    case 2:
        return 34;
    case 3:
        return 169;
    case 4:
        return 170;
    case 5:
        return 264;
    }
    return 0;
}
