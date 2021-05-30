/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "editor/editor.h"
#include "blocks.h"
#include "compat.h"
#include "control/joystick.h"
#include "main/trees.h"
#include "npc_id.h"
#include "layers.h"

#include <Utils/maths.h>

// UpdateNPCs at npc/npc_update.cpp

// NpcHit at npc/npc_hit.cpp

void CheckSectionNPC(int A)
{
    int B = 0;
    if(GameMenu)
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
        if(NPC[A].TurnBackWipe && NoTurnBack[NPC[A].Section])
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
        PlaySound(SFX_Bullet);
        Radius = 32;
    }
    if(Game == 2)
    {
        joyRumbleAllPlayers(150, 1.0);
        NewEffect(69, Location);
        PlaySound(SFX_Fireworks);
        Radius = 52;
    }
    if(Game == 3)
    {
        joyRumbleAllPlayers(200, 1.0);
        NewEffect(70, Location);
        PlaySound(SFX_Fireworks);
        Radius = 64;
    }

    X = Location.X + Location.Width / 2.0;
    Y = Location.Y + Location.Height / 2.0;

    for(i = 1; i <= numNPCs; i++)
    {
        if(!NPC[i].Hidden && NPC[i].Active && !NPC[i].Inert && !NPC[i].Generator && !NPCIsABonus[NPC[i].Type])
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

    // for(i = 1; i <= numBlock; i++)
    // {
    for(Block_t* block : treeBlockQuery(X, Y, X, Y, false, Radius*2))
    {
        i = block - &Block[1] + 1;
        if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type])
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
    else if(BattleMode)
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
        PlaySound(SFX_DropItem);
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Type = NPCType;
        NPC[numNPCs].Location.Width = NPCWidth[NPCType];
        NPC[numNPCs].Location.Height = NPCHeight[NPCType];
        if(ScreenType == 5 && !vScreen[2].Visible)
        {
            if(A == 1)
                B = -40;
            if(A == 2)
                B = 40;
            NPC[numNPCs].Location.X = -vScreenX[1] + vScreen[1].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0 + B;
            double ScreenTop = -vScreenY[1];
            if (vScreen[1].Height > 600)
                ScreenTop += vScreen[1].Height / 2 - 300;
            NPC[numNPCs].Location.Y = ScreenTop + 16 + 12;
        }
        else
        {
            NPC[numNPCs].Location.X = -vScreenX[A] + vScreen[A].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
            double ScreenTop = -vScreenY[A];
            if (vScreen[A].Height > 600)
                ScreenTop += vScreen[A].Height / 2 - 300;
            NPC[numNPCs].Location.Y = ScreenTop + 16 + 12;
        }
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        NPC[numNPCs].Effect = 2;
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 200;
        syncLayers_NPC(numNPCs);
    }
}

void TurnNPCsIntoCoins()
{
    int A = 0;

    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].Active && !NPC[A].Generator)
        {
            if(!NPC[A].Hidden && NPC[A].Killed == 0 && !NPCIsAnExit[NPC[A].Type] && !NPC[A].Inert)
            {
                if(!NPCIsYoshi[NPC[A].Type] && !NPCIsBoot[NPC[A].Type] &&
                   !NPCIsABonus[NPC[A].Type] && NPC[A].Type != 265 && NPC[A].Type != 13 &&
                   NPC[A].Type != 108 && NPC[A].Type != 26 && !NPCIsVeggie[NPC[A].Type] &&
                   NPC[A].Type != 91 && NPC[A].Type != 171 && !NPCIsAVine[NPC[A].Type] &&
                   NPC[A].Type != 56 && NPC[A].Type != 60 && NPC[A].Type != 62 &&
                   NPC[A].Type != 64 && NPC[A].Type != 66 && NPC[A].Type != 104 &&
                   !(NPC[A].Projectile && NPC[A].Type == 30) &&
                   !(NPC[A].Projectile && NPC[A].Type == 17) &&
                   NPC[A].Type != 291 && NPC[A].Type != 292 && NPC[A].Type != 266 &&
                   NPC[A].Type != 57 && NPC[A].Type != 58 &&
                   !(NPC[A].Type >= 78 && NPC[A].Type <= 83) &&
                   NPC[A].Type != 91 && NPC[A].Type != 260 && NPC[A].Type != 259)
                {
                    NPC[A].Location.Y = NPC[A].Location.Y + 32;
                    NewEffect(11, NPC[A].Location);
                    PlaySound(SFX_Coin);
                    Coins = Coins + 1;
                    if(Coins >= 100)
                    {
                        if(Lives < 99)
                        {
                            Lives = Lives + 1;
                            PlaySound(SFX_1up);
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
        else if(NPC[A].Generator)
        {
            NPC[A].Killed = 9;
            NPC[A].Hidden = true;
        }
    }
}

void SkullRide(int A, bool reEnable)
{
    Location_t loc = NPC[A].Location;
    loc.Width += 16;
    loc.X -= 8;
    if(g_compatibility.fix_skull_raft) // Detect by height in condition skull ruft cells were on slopes
    {
        loc.Height += 30;
        loc.Y -= 15;
    }

    int spec = reEnable ? 2 : 0;

    for(int B = 1; B <= numNPCs; B++) // Recursively activate all neihbour skull-ride segments
    {
        auto &npc = NPC[B];
        if(npc.Type == 190)
        {
            if(npc.Active)
            {
                if(npc.Special == spec)
                {
                    if(CheckCollision(loc, npc.Location))
                    {
                        npc.Special = 1;
                        SkullRide(B, reEnable);
                    }
                }
            }
        }
    }
}

static void s_alignRuftCell(NPC_t &me, const Location_t &alignAt)
{
    double w = me.Location.Width;
    me.Location.SpeedX = 0.0;
    me.RealSpeedX = 0.0;

    if(me.Direction > 0)
    {
        auto p = alignAt.X;
        do
        {
            p -= w;
        } while(SDL_fabs(p - me.Location.X) >= w / 2 && p > me.Location.X);
        me.Location.X = p;
    }
    else
    {
        auto p = alignAt.X + alignAt.Width;
        while(SDL_fabs(p - me.Location.X) >= w / 2 && p < me.Location.X + me.Location.Width)
        {
            p += w;
        }
        me.Location.X = p;
    }

    me.Special3 = me.Location.X;
}

void SkullRideDone(int A, const Location_t &alignAt)
{
    auto &me = NPC[A];

    Location_t loc = me.Location;
    loc.Width += 16;
    loc.X -= 8;
    loc.Height += 30;
    loc.Y -= 15;

    for(int B = 1; B <= numNPCs; B++) // Recursively DE-activate all neighbour skull-ride segments
    {
        auto &npc = NPC[B];
        if(npc.Type == 190)
        {
            if(npc.Active)
            {
                if(npc.Special == 1.0)
                {
                    if(CheckCollision(loc , npc.Location))
                    {
                        npc.Special = 2;
                        npc.Location.SpeedX = 0.0;
                        s_alignRuftCell(npc, alignAt);
                        SkullRideDone(B, alignAt);
                    }
                }
            }
        }
    }
}

void NPCSpecial(int A)
{
    double C = 0;
    double D = 0;
    double E = 0;
    double F = 0;
    // int64_t fBlock = 0;
    // int64_t lBlock = 0;
    bool straightLine = false; // SET BUT NOT USED
    bool tempBool = false;
    bool tempBool2 = false;
    Location_t tempLocation;
    NPC_t tempNPC;
    auto &npc = NPC[A];

    // dont despawn
    if(npc.Type == NPCID_BOWSER_SMB || npc.Type == NPCID_WART || npc.Type == NPCID_MOTHERBRAIN ||
       npc.Type == NPCID_BOSSGLASS || npc.Type == NPCID_MOUSER)
    {
        if(npc.TimeLeft > 1)
            npc.TimeLeft = 100;
    }
    // '''''''''''''

    if(npc.Type == NPCID_VINEHEAD_RED_SMB3 || npc.Type == NPCID_VINEHEAD_GREEN_SMB3 || npc.Type == NPCID_VINEHEAD_SMW) // Vine Maker
    {
        npc.Location.SpeedY = -2;
        tempLocation.Height = 28;
        tempLocation.Width = 30;
        tempLocation.Y = npc.Location.Y + npc.Location.Height / 2.0 - tempLocation.Height / 2.0;
        tempLocation.X = npc.Location.X + npc.Location.Width / 2.0 - tempLocation.Width / 2.0;
        tempBool = false;

        for(int i = 1; i <= numNPCs; i++)
        {
            auto &n = NPC[i];
            if(NPCIsAVine[n.Type] && !n.Hidden && CheckCollision(tempLocation, n.Location))
            {
                tempBool = true;
                break;
            }
        }

        // ====== TODO: Is this dead code really needed? ==============
        if(!tempBool)
        {
            // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                auto &b = *block;
                if(!b.Hidden && !BlockNoClipping[b.Type] && !BlockIsSizable[b.Type] && !BlockOnlyHitspot1[b.Type])
                {
                    if(CheckCollision(tempLocation, b.Location) && BlockSlope[b.Type] == 0)
                    {
                        // tempBool = True
                    }
                }
            }
        }
        // ============================================================

        if(!tempBool || npc.Special == 1.0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            if(npc.Type == NPCID_VINEHEAD_RED_SMB3)
                NPC[numNPCs].Type = NPCID_REDVINE_SMB3;
            else if(npc.Type == NPCID_VINEHEAD_GREEN_SMB3)
                NPC[numNPCs].Type = NPCID_GRNVINE_SMB3;
            else if(npc.Type == NPCID_VINEHEAD_SMW)
                NPC[numNPCs].Type = NPCID_GRNVINE_SMW;
            NPC[numNPCs].Location.Y = vb6Round(npc.Location.Y / 32) * 32;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Section = npc.Section;
            NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
            NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
            NPC[numNPCs].Layer = npc.Layer;
            NPC[numNPCs].Shadow = npc.Shadow;
            syncLayers_NPC(numNPCs);
        }

        if(npc.Special == 1.0)
            npc.Killed = 9;

        // driving block
    }
    else if(npc.Type == NPCID_COCKPIT)
    {
        if(npc.Special4 > 0)
            npc.Special4 = 0;
        else
        {
            npc.Special5 = 0;
            npc.Special6 = 0;
        }

        if(npc.Special5 > 0)
        {
            if(npc.Location.SpeedX < 0)
                npc.Location.SpeedX = npc.Location.SpeedX * 0.95;
            npc.Location.SpeedX = npc.Location.SpeedX + 0.1;
        }
        else if(npc.Special5 < 0)
        {
            if(npc.Location.SpeedX > 0)
                npc.Location.SpeedX = npc.Location.SpeedX * 0.95;
            npc.Location.SpeedX = npc.Location.SpeedX - 0.1;
        }
        else
        {
            npc.Location.SpeedX = npc.Location.SpeedX * 0.95;
            if(npc.Location.SpeedX > -0.1 && npc.Location.SpeedX < 0.1)
                npc.Location.SpeedX = 0;
        }

        if(npc.Special6 > 0)
        {
            if(npc.Location.SpeedY < 0)
                npc.Location.SpeedY = npc.Location.SpeedY * 0.95;
            npc.Location.SpeedY = npc.Location.SpeedY + 0.1;
        }
        else if(npc.Special6 < 0)
        {
            if(npc.Location.SpeedY > 0)
                npc.Location.SpeedY = npc.Location.SpeedY * 0.95;
            npc.Location.SpeedY = npc.Location.SpeedY - 0.1;
        }
        else
        {
            npc.Location.SpeedY = npc.Location.SpeedY * 0.95;
            if(npc.Location.SpeedY > -0.1 && npc.Location.SpeedY < 0.1)
                npc.Location.SpeedY = 0;
        }

        if(npc.Location.SpeedY > 4)
            npc.Location.SpeedY = 4;

        if(npc.Location.SpeedY < -4)
            npc.Location.SpeedY = -4;

        if(npc.Location.SpeedX > 6)
            npc.Location.SpeedX = 6;

        if(npc.Location.SpeedX < -6)
            npc.Location.SpeedX = -6;

    }
    else if(npc.Type == NPCID_PEACHBOMB) // heart bomb
    {
        if(npc.Special4 != 0.0)
        {
            npc.Killed = 9;
            C = npc.BattleOwner;

            if(npc.CantHurtPlayer > 0)
                C = npc.CantHurtPlayer;

            Bomb(npc.Location, 0, vb6Round(C));

            for(int i = 1; i <= 5; i++)
            {
                NewEffect(80, newLoc(npc.Location.X + npc.Location.Width / 2.0 - 4, npc.Location.Y + npc.Location.Height / 2.0 - 6), 1, 0, npc.Shadow);
                Effect[numEffects].Location.SpeedX = dRand() * 6 - 3;
                Effect[numEffects].Location.SpeedY = dRand() * 6 - 3;
                Effect[numEffects].Frame = (iRand() % 3);
            }
        }
    }
    else if(npc.Type == NPCID_HOOPSTER)
    {
        if(!npc.Projectile)
        {
            npc.Location.SpeedX = 0;
            if(npc.Location.SpeedY < 0)
                npc.Special = -1;
            else
                npc.Special = 1;

            tempLocation = npc.Location;

            if(int(npc.Special) == -1)
                tempLocation.Y -= 1;
            else
                tempLocation.Y += tempLocation.Height + 1;

            tempLocation.Height = 1;
            tempBool = false;

            for(int i = 1; i <= numNPCs; i++)
            {
                auto &n = NPC[i];
                if(n.Active && !n.Hidden && NPCIsAVine[n.Type] && CheckCollision(tempLocation, n.Location))
                {
                    tempBool = true;
                    break;
                }
            }

            if(!tempBool)
            {
                for(int i = 1; i <= numBackground; i++)
                {
                    auto &b = Background[i];
                    if(!b.Hidden && ((b.Type >= 174 && b.Type <= 186) || b.Type == 63) && CheckCollision(tempLocation, b.Location))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }

            if(tempBool)
            {
                if(npc.Special == 1)
                    npc.Location.SpeedY = 2;
                else
                    npc.Location.SpeedY = -1;
            }
            else
            {
                if(npc.Special == -1)
                {
                    npc.Location.SpeedY = 2;
                    npc.Special = 2;
                }
                else
                {
                    npc.Location.SpeedY = -1;
                    npc.Special = -1;
                }
            }
        }
        else
        {
            npc.Location.SpeedY += Physics.NPCGravity;
            npc.Location.SpeedX *= 0.987;
            if(npc.Location.SpeedX > -0.1 && npc.Location.SpeedX < 0.1)
                npc.Location.SpeedX = 0;

        }
    }
    else if(npc.Type == NPCID_RANDOM_POWERUP)
        npc.Type = RandomBonus();

    else if(npc.Type == NPCID_SPINYEGG_SMW) // falling spiney
    {
        if(npc.Special != 0.0)
        {
            npc.Type = NPCID_SPINY_SMW;
            npc.Special = 0;
            C = 0;

            for(int i = 1; i <= numPlayers; i++)
            {
                auto &p = Player[i];
                if(!p.Dead && p.Section == npc.Section && p.TimeToLive == 0)
                {
                    if(C == 0.0 || std::abs(npc.Location.X + npc.Location.Width / 2.0 - (p.Location.X + p.Location.Width / 2.0)) < C)
                    {
                        C = std::abs(npc.Location.X + npc.Location.Width / 2.0 - (p.Location.X + p.Location.Width / 2.0));
                        if(npc.Location.X + npc.Location.Width / 2.0 > p.Location.X + p.Location.Width / 2.0)
                            npc.Direction = -1;
                        else
                            npc.Direction = 1;
                    }
                }
            }

            npc.Location.SpeedX = Physics.NPCWalkingOnSpeed * npc.Direction;
        }
    }
    else if(npc.Type == NPCID_BUBBLE) // bubble
    {
        if(fiEqual(npc.Special, 287))
        {
            npc.Special = RandomBonus();
            npc.DefaultSpecial = npc.Special;
        }

        npc.Location.SpeedX = 0.75 * npc.DefaultDirection;
        if(npc.Special2 == 0.0)
            npc.Special2 = -1;

        npc.Location.SpeedY = npc.Location.SpeedY + 0.05 * npc.Special2;
        if(npc.Location.SpeedY > 1)
        {
            npc.Location.SpeedY = 1;
            npc.Special2 = -1;
        }
        else if(npc.Location.SpeedY < -1)
        {
            npc.Location.SpeedY = -1;
            npc.Special2 = 1;
        }

        if(npc.Special3 > 0)
        {
            NewEffect(144, npc.Location);
            PlaySound(SFX_Bubble);
            npc.Type = npc.Special;
            npc.Special3 = 0;
            npc.Special2 = 0;
            npc.Special = 0;
            npc.Frame = EditorNPCFrame(npc.Type, npc.Direction);
            npc.FrameCount = 0;
            npc.Location.X = npc.Location.X + npc.Location.Width / 2.0;
            npc.Location.Y = npc.Location.Y + npc.Location.Height / 2.0;
            npc.Location.Width = NPCWidth[npc.Type];
            npc.Location.Height = NPCHeight[npc.Type];
            npc.Location.X = npc.Location.X - npc.Location.Width / 2.0;
            npc.Location.Y = npc.Location.Y - npc.Location.Height / 2.0;
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;
            npc.Direction = npc.DefaultDirection;

            if(NPCIsACoin[npc.Type])
            {
                npc.Special = 1;
                npc.Location.SpeedX = dRand() * 1 - 0.5;
            }

            if(Maths::iRound(npc.Direction) == 0)
            {
                if(iRand() % 2 == 1)
                    npc.Direction = 1;
                else
                    npc.Direction = -1;
            }

            npc.TurnAround = false;
            if(npc.Type == 134)
                npc.Projectile = true;
        }

    }
    else if(npc.Type == NPCID_VOLCANO_LOTUS) // fire plant thing
    {
        if(npc.Special == 0.0)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 170)
            {
                npc.Special2 = 0;
                npc.Special = 1;
            }
        }
        else if(fiEqual(npc.Special, 1))
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 70)
            {
                npc.Special2 = 0;
                npc.Special = 2;
            }
        }
        else if(npc.Special == 2)
        {
            if(npc.Special2 == 0.0) // spit fireballs
            {
                for(int i = 1; i <= 4; i++)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = NPCID_VOLCANO_LOTUS_FIREBALL;
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Section = npc.Section;
                    NPC[numNPCs].TimeLeft = npc.TimeLeft;
                    NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                    NPC[numNPCs].Location.Y = npc.Location.Y - NPC[numNPCs].Location.Height;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Layer = npc.Layer;

                    if(i == 1 || i == 4)
                    {
                        NPC[numNPCs].Location.SpeedX = -2.5;
                        NPC[numNPCs].Location.SpeedY = -1.5;
                    }
                    else
                    {
                        NPC[numNPCs].Location.SpeedX = -1;
                        NPC[numNPCs].Location.SpeedY = -2;
                    }

                    if(i == 3 || i == 4)
                        NPC[numNPCs].Location.SpeedX = -NPC[numNPCs].Location.SpeedX;

                    NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.6;
                    NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.6;
                    syncLayers_NPC(numNPCs);
                }
            }

            npc.Special2 += 1;
            if(npc.Special2 >= 50)
            {
                npc.Special2 = 0;
                npc.Special = 0;
            }
        }
    }
    else if(npc.Type == NPCID_VOLCANO_LOTUS_FIREBALL) // plant fireballs
    {
        if(npc.Special == 0)
        {
            npc.Location.SpeedY *= 0.98;
            npc.Location.SpeedX *= 0.98;
            if(npc.Location.SpeedY > -0.5)
            {
                npc.Location.SpeedX = 0;
                npc.Location.SpeedY = 0;
                npc.Special2 = 0;
                npc.Special = 1;
            }
        }
        else
        {
            npc.Location.SpeedY = npc.Location.SpeedY + 0.02;
            if(npc.Location.SpeedY > 2)
                npc.Location.SpeedY = 2;

            if(npc.Location.SpeedY > 0.25)
            {
                npc.Special2 = npc.Special2 + 1;
                if(npc.Special2 < 7)
                    npc.Location.SpeedX = -0.8;
                else if(npc.Special2 < 13)
                    npc.Location.SpeedX = 0.8;
                else
                {
                    npc.Special2 = 0;
                    npc.Location.SpeedX = 0;
                }
            }

        }
    }
    else if(npc.Type == NPCID_SWOOPER) // bat thing
    {
        if(npc.Special == 0)
        {
            for(int i = 1; i <= numPlayers; i++)
            {
                auto &p = Player[i];
                if(p.Section == npc.Section && !p.Dead && p.TimeToLive == 0)
                {
                    tempLocation = npc.Location;
                    tempLocation.Width = 400;
                    tempLocation.Height = 800;
                    tempLocation.X -= tempLocation.Width / 2.0;
                    tempLocation.Y -= tempLocation.Height / 2.0;

                    if(CheckCollision(tempLocation, p.Location))
                    {
                        npc.Special = 1;
                        if(p.Location.X < npc.Location.X)
                            npc.Direction = -1;
                        else
                            npc.Direction = 1;
                        npc.Location.SpeedX = 0.01 * npc.Direction;

                        if(p.Location.Y > npc.Location.Y)
                        {
                            npc.Location.SpeedY = 2.5;
                            npc.Special2 = p.Location.Y - 130;
                        }
                        else
                        {
                            npc.Location.SpeedY = -2.5;
                            npc.Special2 = p.Location.Y + 130;
                        }

                        PlaySound(SFX_SwooperFlap);
                    }
                }
            }
        }
        else
        {
            npc.Location.SpeedX = (3 - std::abs(npc.Location.SpeedY)) * npc.Direction;
            if((npc.Location.SpeedY > 0 && npc.Location.Y > npc.Special2) || (npc.Location.SpeedY < 0 && npc.Location.Y < npc.Special2))
            {
                npc.Location.SpeedY = npc.Location.SpeedY * 0.98;
                if(npc.Location.SpeedY > -0.1 && npc.Location.SpeedY < 0.1)
                    npc.Location.SpeedY = 0;
            }
        }
    }
    else if(npc.Type == NPCID_LARRY_MAGIC_RING) // larry magic
    {
        if(npc.Special < 2)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 30 && npc.Special != 2)
            {
                npc.Location.X = npc.Location.X + npc.Location.Width;
                npc.Location.Y = npc.Location.Y + npc.Location.Height;
                npc.Location.Width = 16;
                npc.Location.Height = 32;
                npc.Special = 2;
                npc.Location.X = npc.Location.X - npc.Location.Width;
                npc.Location.Y = npc.Location.Y - npc.Location.Height;
                npc.Special2 = 21;
            }
            else if(npc.Special2 >= 15 && npc.Special != 1)
            {
                npc.Location.X = npc.Location.X + npc.Location.Width;
                npc.Location.Y = npc.Location.Y + npc.Location.Height;
                npc.Location.Width = 10;
                npc.Location.Height = 20;
                npc.Special = 1;
                npc.Location.X = npc.Location.X - npc.Location.Width;
                npc.Location.Y = npc.Location.Y - npc.Location.Height;
            }
        }
    }
    else if(npc.Type == NPCID_LARRYSHELL || npc.Type == NPCID_LUDWIG_SHELL) // larry/ludwig shell
    {
        if(npc.Special5 == 0) // Target a Random Player
        {
            C = 0;
            do
            {
                int i = (iRand() % numPlayers) + 1;
                if(!Player[i].Dead && Player[i].Section == npc.Section && Player[i].TimeToLive == 0)
                    npc.Special5 = i;
                C += 1;
                if(C >= 20)
                    npc.Special5 = 1;
            }
            while(!(npc.Special5 > 0));
        }

        if(Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0 < npc.Location.X + npc.Location.Width / 2.0)
            npc.Direction = -1;
        else
            npc.Direction = 1;
        if(npc.Special == 0)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 60)
            {
                npc.Special = 1;
                npc.Special2 = 0;
            }
        }
        else if(npc.Special == 1)
        {
            if(npc.Type == 281 && fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
                npc.Location.SpeedX = npc.Location.SpeedX + 0.1 * npc.Direction;
            else
                npc.Location.SpeedX = npc.Location.SpeedX + 0.2 * npc.Direction;
            if(npc.Type == 281 && npc.Damage >= 5)
            {
                if(npc.Location.SpeedX > 5.5)
                    npc.Location.SpeedX = 5.5;
                else if(npc.Location.SpeedX < -5.5)
                    npc.Location.SpeedX = -5.5;
            }
            else if(npc.Type == 281 && npc.Damage >= 10)
            {
                if(npc.Location.SpeedX > 6)
                    npc.Location.SpeedX = 6;
                else if(npc.Location.SpeedX < -6)
                    npc.Location.SpeedX = -6;
            }
            else
            {
                if(npc.Location.SpeedX > 5)
                    npc.Location.SpeedX = 5;
                else if(npc.Location.SpeedX < -5)
                    npc.Location.SpeedX = -5;
            }

            if(npc.Type == 281 && fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special3 = npc.Special3 + 1;
                if((npc.Location.SpeedX < -2 && npc.Direction < 0) || (npc.Location.SpeedX > 2 && npc.Direction > 0))
                {
                    if(npc.Special3 >= 20 - npc.Damage * 2)
                    {
                        npc.Special3 = 0;
                        npc.Location.SpeedY = -3 - dRand() * 2;
                    }
                }
            }

            npc.Special2 += 1;

            if(npc.Special2 >= 300 && fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special = 2;
                npc.Special2 = 0;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedY = -5 - dRand() * 3;
            if(npc.Type == 281)
                npc.Location.SpeedY -= 2;
            npc.Special = 3;
        }
        else if(npc.Special == 3)
        {
            if(npc.Location.SpeedX > 2.5)
                npc.Location.SpeedX -= 0.2;
            else if(npc.Location.SpeedX < -2.5)
                npc.Location.SpeedX += 0.2;
            npc.Special2 += 1;

            if(npc.Type == 281)
            {
                npc.Special2 = 20;
                PlaySound(SFX_Spring);
            }

            if(npc.Special2 == 20)
            {
                npc.Special = 0;
                npc.Special2 = npc.Direction;
                npc.Special3 = 0;
                npc.Special4 = 0;
                npc.Special5 = 0;
                npc.Special6 = 0;
                npc.Location.X += npc.Location.Width / 2.0;
                npc.Location.Y += npc.Location.Height;
                npc.Type = npc.Type - 1;
                npc.Location.Width = NPCWidth[npc.Type];
                npc.Location.Height = NPCHeight[npc.Type];
                npc.Location.X -= npc.Location.Width / 2.0;
                npc.Location.Y -= npc.Location.Height;
            }
        }
        else
            npc.Special = 0;

    }
    else if(npc.Type == NPCID_LARRY) // larry koopa
    {
        // special is phase
        // special5 is targetted player
        // special3 is jump counter
        // special2 is direction

        // special4 is attack timer
        // special3 is attack count
        if(npc.Inert)
        {
            npc.Special4 = 0;
            npc.Special3 = 0;
        }

        if(npc.Special5 == 0) // Target a Random Player
        {
            C = 0;
            do
            {
                int i = (iRand() % numPlayers) + 1;
                if(Player[i].Dead == false && Player[i].Section == npc.Section && Player[i].TimeToLive == 0)
                    npc.Special5 = i;
                C += 1;
                if(C >= 20)
                    npc.Special5 = 1;
            }
            while(!(npc.Special5 > 0));
        }

        if(Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0 < npc.Location.X + npc.Location.Width / 2.0)
            npc.Direction = -1;
        else
            npc.Direction = 1;

        if(npc.Special2 == 0)
            npc.Special2 = npc.Direction;

        if(npc.Special == 0)
        {
            if(npc.Special2 == -1)
                npc.Location.SpeedX = -2.5;
            else
                npc.Location.SpeedX = 2.5;

            // movement
            if(npc.Location.X < Player[npc.Special5].Location.X - 400)
                npc.Special2 = 1;
            else if(npc.Location.X > Player[npc.Special5].Location.X + 400)
                npc.Special2 = -1;

            // jumping
            if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special3 += 1;
                if(npc.Special3 >= 30 + dRand() * 100)
                {
                    npc.Special3 = 0;
                    npc.Location.SpeedY = -5 - dRand() * 4;
                }
            }
            else
                npc.Special3 = 0;

            // attack timer
            npc.Special4 += 1;
            if(npc.Special4 >= 100 + dRand() * 100 && fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special = 1;
                npc.Special5 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
            }
        }
        else if(npc.Special == 1)
        {
            if(Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0 < npc.Location.X + npc.Location.Width / 2.0)
                npc.Direction = -1;
            else
                npc.Direction = 1;

            npc.Special2 = npc.Direction;
            npc.Location.SpeedX = 0;
            npc.Special3 += 1;

            if(npc.Special3 >= 10)
            {
                npc.Special3 = 0;
                npc.Special = 2;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedX = 0;
            if(npc.Special3 == 0 || npc.Special3 == 6 || npc.Special3 == 12) // shoot
            {

                if(npc.Special3 == 0)
                {
                    npc.Special6 = Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0;
                    npc.Special4 = Player[npc.Special5].Location.Y + Player[npc.Special5].Location.Height / 2.0 + 16;
                }

                if(npc.Special3 == 0)
                    PlaySound(SFX_Raccoon);

                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Section = npc.Section;
                NPC[numNPCs].Type = 269;
                NPC[numNPCs].Location.Width = 10;
                NPC[numNPCs].Location.Height = 8;
                NPC[numNPCs].Frame = 3;
                NPC[numNPCs].Special2 = npc.Special3;

                if(Maths::iRound(NPC[numNPCs].Direction) == -1)
                    NPC[numNPCs].Location.X = npc.Location.X - 20;
                else
                    NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width - NPC[numNPCs].Location.Width + 20;

                NPC[numNPCs].Location.Y = npc.Location.Y + 47;
                NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;
                C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - npc.Special6;
                D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - npc.Special4;
                NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;

                if(NPC[numNPCs].Location.SpeedY > 3)
                    NPC[numNPCs].Location.SpeedY = 3;
                else if(NPC[numNPCs].Location.SpeedY < -3)
                    NPC[numNPCs].Location.SpeedY = -3;
                syncLayers_NPC(numNPCs);
            }

            npc.Special3 += 1;
            if(npc.Special3 >= 30)
            {
                npc.Special = 0;
                npc.Special4 = 0;
                npc.Special5 = 0;
                npc.Special6 = 0;
            }
        }

        // ludwig koopa
    }
    else if(npc.Type == NPCID_LUDWIG)
    {
        // special is phase
        // special5 is targetted player
        // special3 is jump counter
        // special2 is direction

        // special4 is attack timer
        // special3 is attack count

        if(npc.Inert)
        {
            npc.Special4 = 0;
            npc.Special3 = 0;
            npc.Special6 = 0;
        }

        if(npc.Special5 == 0) // Target a Random Player
        {
            C = 0;
            do
            {
                int i = (iRand() % numPlayers) + 1;
                if(!Player[i].Dead && Player[i].Section == npc.Section && Player[i].TimeToLive == 0)
                    npc.Special5 = i;
                C += 1;
                if(C >= 20)
                    npc.Special5 = 1;
            }
            while(npc.Special5 <= 0); // TESTME: !(npc.Special5 > 0)
        }

        if(Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0 < npc.Location.X + npc.Location.Width / 2.0)
            npc.Direction = -1;
        else
            npc.Direction = 1;

        if(npc.Special2 == 0)
            npc.Special2 = npc.Direction;

        if(npc.Special == 0)
        {
            if((npc.Damage < 5 && npc.Special6 > 60) || (npc.Damage < 10 && npc.Special6 > 80) || npc.Inert)
            {
                if(npc.Special2 == -1)
                    npc.Location.SpeedX = -1.5;
                else
                    npc.Location.SpeedX = 1.5;
                // movement
                if(npc.Location.X < Player[npc.Special5].Location.X - 400)
                    npc.Special2 = 1;
                else if(npc.Location.X > Player[npc.Special5].Location.X + 400)
                    npc.Special2 = -1;
            }
            else
            {
                npc.Location.SpeedX = npc.Location.SpeedX * 0.98;
                if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
                    npc.Location.SpeedX = 0;

            }

            // attack timer
            if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
                npc.Special6 += 1;

            if(npc.Special6 == 20 || npc.Special6 == 40 || npc.Special6 == 60 || (npc.Damage >= 5 && npc.Special6 == 80) || (npc.Damage >= 10 && npc.Special6 == 100))
            {
                npc.Special = 1;
                npc.Special5 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
            }

            if(npc.Damage >= 10 && npc.Special == 0 && npc.Special6 >= 100)
                npc.Special6 = 200;

            if(npc.Special6 >= 160 && fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special6 = 0;
                npc.Special = 3;
                PlaySound(SFX_Spring);
                npc.Location.SpeedY = -7 - dRand() * 2;
            }
        }
        else if(npc.Special == 3)
        {
            if(npc.Location.SpeedY > 0) // turn into shell
            {
                npc.Special = 1;
                npc.Special2 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
                npc.Special5 = 0;
                npc.Special6 = 0;
                npc.Location.X += npc.Location.Width / 2.0;
                npc.Location.Y += npc.Location.Height;
                npc.Type = NPCID_LUDWIG_SHELL;
                npc.Location.Width = NPCWidth[npc.Type];
                npc.Location.Height = NPCHeight[npc.Type];
                npc.Location.X -= npc.Location.Width / 2.0;
                npc.Location.Y -= npc.Location.Height;
            }
            else
            {
                npc.Location.SpeedX += 0.2 * npc.Special2;
                if(npc.Location.SpeedX > 5)
                    npc.Location.SpeedX = 5;
                else if(npc.Location.SpeedX < -5)
                    npc.Location.SpeedX = -5;
            }
        }
        else if(npc.Special == 1 || npc.Special == 2)
        {
            if(Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0 < npc.Location.X + npc.Location.Width / 2.0)
                npc.Direction = -1;
            else
                npc.Direction = 1;

            npc.Special2 = npc.Direction;
            npc.Location.SpeedX = 0;

            if(npc.Special3 == 20) // shoot
            {
                PlaySound(SFX_BigFireball);
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Section = npc.Section;
                NPC[numNPCs].Type = NPCID_LUDWIG_FIRE;
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                NPC[numNPCs].Frame = 0;
                if(Maths::iRound(NPC[numNPCs].Direction) == -1)
                    NPC[numNPCs].Location.X = npc.Location.X - 24;
                else
                    NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width - NPC[numNPCs].Location.Width + 24;
                NPC[numNPCs].Location.Y = npc.Location.Y + 4;
                NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
                C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0);
                D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[npc.Special5].Location.Y + Player[npc.Special5].Location.Height / 2.0);
                NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                if(NPC[numNPCs].Location.SpeedY > 2)
                    NPC[numNPCs].Location.SpeedY = 2;
                else if(NPC[numNPCs].Location.SpeedY < -2)
                    NPC[numNPCs].Location.SpeedY = -2;
                syncLayers_NPC(numNPCs);
            }

            npc.Special3 += 1;
            if(npc.Special3 < 20)
                npc.Special = 1;
            else
                npc.Special = 2;

            if(npc.Special3 >= 40)
            {
                npc.Special = 0;
                npc.Special5 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
            }
        }



    }
    else if(npc.Type == NPCID_SWORDBEAM) // link sword beam
    {
        npc.Special += 1;
        if(npc.Special == 40)
        {
            npc.Killed = 9;
            for(int i = 1; i <= 4; i++)
            {
                tempLocation.Height = EffectHeight[80];
                tempLocation.Width = EffectWidth[80];
                tempLocation.SpeedX = 0;
                tempLocation.SpeedY = 0;
                tempLocation.X = npc.Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4; // + .Location.SpeedX
                tempLocation.Y = npc.Location.Y + dRand() * 4 - EffectHeight[80] / 2.0 - 2;
                NewEffect(80, tempLocation);
                Effect[numEffects].Location.SpeedX = npc.Location.SpeedX * 0.3 + dRand() * 2 - 1;
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
            tempLocation.X = npc.Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4; // + .Location.SpeedX
            tempLocation.Y = npc.Location.Y + dRand() * 4 - EffectHeight[80] / 2.0 - 2;
            NewEffect(80, tempLocation);
            Effect[numEffects].Location.SpeedX = npc.Location.SpeedX * 0.15;
            Effect[numEffects].Location.SpeedY = npc.Location.SpeedY; // + Rnd * 2 - 1
            Effect[numEffects].Frame = (iRand() % 2) + 1;
        }


    }
    else if(npc.Type == NPCID_MOUSER) // mouser
    {
        if(npc.Immune == 0)
        {
            C = 0;
            for(int i = 1; i <= numPlayers; i++)
            {
                auto &p = Player[i];
                if(!p.Dead && p.Section == npc.Section)
                {
                    if(C == 0 || std::abs(npc.Location.X + npc.Location.Width / 2.0 - (p.Location.X + p.Location.Width / 2.0)) < C)
                    {
                        C = std::abs(npc.Location.X + npc.Location.Width / 2.0 - (p.Location.X + p.Location.Width / 2.0));
                        if(npc.Location.X + npc.Location.Width / 2.0 > p.Location.X + p.Location.Width / 2.0)
                            npc.Direction = -1;
                        else
                            npc.Direction = 1;
                    }
                }
            }
        }
        else
        {
            if(dRand() * 100 > 90)
                npc.Direction = -npc.Direction;
            npc.Special3 = 0;
            npc.Special4 = 0;
        }

        if(npc.Immune != 0)
            npc.Location.SpeedX = 0;
        else if(npc.Special == 0)
        {
            if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                if(npc.Special2 == 0)
                    npc.Special2 = npc.Direction;

                npc.Location.SpeedX = 2 * npc.Special2;

                if(npc.Location.X < npc.DefaultLocation.X - 64)
                    npc.Special2 = 1;
                else if(npc.Location.X > npc.DefaultLocation.X + 64)
                    npc.Special2 = -1;

                npc.Special3 += 1;
                npc.Special4 += 1;

                if(npc.Special3 >= 100 + dRand() * 200)
                {
                    npc.Special3 = 0;
                    npc.Location.SpeedX = 0;
                    npc.Location.SpeedY = -5;
                }
                else if(npc.Special4 >= 20 + dRand() * 200)
                {
                    npc.Special4 = 0;
                    npc.Special = -10;
                }
            }
            else
                npc.Location.SpeedX = 0;
        }
        else
        {
            if(npc.Special == -1)
            {
                npc.Special = 20;
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Layer = "Spawned NPCs";
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Type = 134;
                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                NPC[numNPCs].Location.Y = npc.Location.Y + npc.Location.Height - 48;
                NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0 - 12 * NPC[numNPCs].Direction;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Section = npc.Section;
                NPC[numNPCs].Location.SpeedX = (5 + dRand() * 3) * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedY = -5 - (dRand() * 3);
                syncLayers_NPC(numNPCs);
            }

            npc.Location.SpeedX = 0;

            if(npc.Special < 0)
                npc.Special += 1;
            else
                npc.Special -= 1;
        }

    }
    else if(npc.Type == NPCID_NIPPER_PLANT) // muncher thing
    {
        if(npc.Special == 0)
        {
            for(int i = 1; i <= numPlayers; i++)
            {
                auto &p = Player[i];
                if(!p.Dead && p.TimeToLive == 0 && p.Section == npc.Section)
                {
                    tempLocation = npc.Location;
                    tempLocation.Height = 256;
                    tempLocation.Y -= tempLocation.Height;
                    if(CheckCollision(tempLocation, p.Location))
                    {
                        npc.Special = 1;
                        npc.Location.SpeedY = -7;
                        npc.Location.SpeedX = 0;
                    }
                }
            }
        }
        else if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            npc.Special = 0;

        if(!npc.Stuck && npc.Special == 0.0)
        {
            if(npc.Special2 == 0.0)
            {
                if(npc.Location.X < npc.DefaultLocation.X - 128 && npc.Direction == -1)
                    npc.Special2 = 60;
                else if(npc.Location.X > npc.DefaultLocation.X + 128 && npc.Direction == 1)
                    npc.Special2 = 60;

                npc.Location.SpeedX = 1.4 * npc.Direction;

                if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
                    npc.Location.SpeedY = -1.5;
            }
            else
            {
                npc.Special2 -= 1;

                if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
                    npc.Location.SpeedX = 0;

                if(npc.Special2 == 0.0)
                {
                    if(npc.Location.X < npc.DefaultLocation.X)
                        npc.Direction = 1;
                    else
                        npc.Direction = -1;
                }
            }
        }




    }
    else if(npc.Type == NPCID_FIREBAR) // Firebar
    {
        double C = 0.03 * npc.DefaultSpecial;
        double B = 0.98 * npc.DefaultSpecial;

        if(npc.Special2 == 0)
        {
            npc.Location.SpeedX += C;
            npc.Location.SpeedY += C * npc.DefaultDirection;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedX = -B;
                npc.Location.SpeedY = 0;
            }

            if(npc.Location.SpeedX >= -0.001)
            {
                npc.Special2 += 1 * npc.DefaultDirection;
                if(npc.Special2 <= 0)
                    npc.Special2 = 3;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special2 == 1)
        {
            npc.Location.SpeedX += C * npc.DefaultDirection;
            npc.Location.SpeedY -= C;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedY = B;
                npc.Location.SpeedX = 0;
            }

            if(npc.Location.SpeedY <= 0.001)
            {
                npc.Special2 += 1 * npc.DefaultDirection;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special2 == 2)
        {
            npc.Location.SpeedX -= C;
            npc.Location.SpeedY -= C * npc.DefaultDirection;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedX = B;
                npc.Location.SpeedY = 0;
            }

            if(npc.Location.SpeedX <= 0.001)
            {
                npc.Special2 += 1 * npc.DefaultDirection;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special2 == 3)
        {
            npc.Location.SpeedX -= C * npc.DefaultDirection;
            npc.Location.SpeedY += C;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedY = -B;
                npc.Location.SpeedX = 0;
            }

            if(npc.Location.SpeedY >= -0.001)
            {
                npc.Special2 += 1 * npc.DefaultDirection;
                if(npc.Special2 > 3)
                    npc.Special2 = 0;
                npc.Special5 = 0;
            }
        }

    }
    else if(npc.Type == NPCID_ROTODISK) // Roto-Disk
    {
        double C = 0.2; // * .DefaultDirection
        double B = 6.05; // * .DefaultDirection

        npc.Special4 += 1;

        if(npc.Special4 == 4)
        {
            NewEffect(136, npc.Location);
            Effect[numEffects].Frame = npc.Frame;
            Effect[numEffects].Location.SpeedX = 0;
            Effect[numEffects].Location.SpeedY = 0;
            npc.Special4 = 0;
        }

        if(npc.Special == 0)
        {
            npc.Location.SpeedX = npc.Location.SpeedX + C;
            npc.Location.SpeedY = npc.Location.SpeedY + C * npc.DefaultDirection;
            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedX = -B;
                npc.Location.SpeedY = 0;
            }
            if(npc.Location.SpeedX >= 0)
            {
                npc.Special = npc.Special + 1 * npc.DefaultDirection;
                if(npc.Special < 0)
                    npc.Special = 3;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special == 1)
        {
            npc.Location.SpeedX = npc.Location.SpeedX + C * npc.DefaultDirection;
            npc.Location.SpeedY = npc.Location.SpeedY - C;
            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedY = B;
                npc.Location.SpeedX = 0;
            }
            if(npc.Location.SpeedY <= 0)
            {
                npc.Special = npc.Special + 1 * npc.DefaultDirection;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedX = npc.Location.SpeedX - C;
            npc.Location.SpeedY = npc.Location.SpeedY - C * npc.DefaultDirection;
            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedX = B;
                npc.Location.SpeedY = 0;
            }
            if(npc.Location.SpeedX <= 0)
            {
                npc.Special = npc.Special + 1 * npc.DefaultDirection;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special == 3)
        {
            npc.Location.SpeedX = npc.Location.SpeedX - C * npc.DefaultDirection;
            npc.Location.SpeedY = npc.Location.SpeedY + C;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedY = -B;
                npc.Location.SpeedX = 0;
            }
            if(npc.Location.SpeedY >= 0)
            {
                npc.Special = npc.Special + 1 * npc.DefaultDirection;
                if(npc.Special > 3)
                    npc.Special = 0;
                npc.Special5 = 0;
            }
        }


    }
    else if(npc.Type == NPCID_LOCKDOOR)
    {
        for(int i = 1; i <= numNPCs; i++)
        {
            auto &n = NPC[i];
            if(n.Type == NPCID_KEY && n.Active && n.HoldingPlayer != 0 && CheckCollision(npc.Location, n.Location))
            {
                n.Killed = 9;
                NewEffect(10, n.Location);
                npc.Killed = 3;
            }
        }
    }
    else if(npc.Type == NPCID_MOTHERBRAIN) // Mother Brain
    {
        if(npc.Special >= 1)
        {
            double B = 1 - (npc.Special / 45);
            double C = B * 0.5;
            B *= 15;
            C *= 15;
            npc.Location.X = npc.DefaultLocation.X + dRand() * B - dRand() * C;
            npc.Location.Y = npc.DefaultLocation.Y + dRand() * B - dRand() * C;
            npc.Special = npc.Special + 1;
            if(npc.Special >= 45)
                npc.Special = 0;
        }
        else
            npc.Location = npc.DefaultLocation;
    }
    else if(npc.Type == NPCID_RINKA) // O thing
    {
        if(npc.Special == 0)
        {
            double C = 0;
            double D = 0.0;
            double E;
            npc.Special2 += 1;

            if(npc.Special2 >= 80 + dRand() * 20)
            {
                npc.Special = 1;
                C = 0;
                for(int i = 1; i <= numPlayers; i++)
                {
                    if(!Player[i].Dead && Player[i].Section == npc.Section)
                    {
                        if(C == 0 || std::abs(npc.Location.X + npc.Location.Width / 2.0 - (Player[i].Location.X + Player[i].Location.Width / 2.0)) < C)
                        {
                            C = std::abs(npc.Location.X + npc.Location.Width / 2.0 - (Player[i].Location.X + Player[i].Location.Width / 2.0));
                            D = i;
                        }
                    }
                }
                int ip = D;
                C = (npc.Location.X + npc.Location.Width / 2.0) - (Player[ip].Location.X + Player[ip].Location.Width / 2.0);
                D = (npc.Location.Y + npc.Location.Height / 2.0) - (Player[ip].Location.Y + Player[ip].Location.Height / 2.0);
                E = std::sqrt(std::pow(C, 2) + std::pow(D, 2));
                C = -C / E;
                D = -D / E;
                npc.Location.SpeedX = C * 3;
                npc.Location.SpeedY = D * 3;
            }
        }

    }
    else if(npc.Type == NPCID_RINKAGEN) // Metroid O shooter thing
    {
        npc.Special = npc.Special + 1 + dRand();
        if(npc.Special >= 200 + dRand() * 200)
        {
            npc.Special = 0;
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Inert = npc.Inert;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 28;
            NPC[numNPCs].Location.X = npc.Location.X + 2;
            NPC[numNPCs].Location.Y = npc.Location.Y;
            NPC[numNPCs].Section = npc.Section;
            NPC[numNPCs].Layer = "Spawned NPCs";
            NPC[numNPCs].Type = 210;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 50;
            syncLayers_NPC(numNPCs);
        }
    }
    else if(npc.Type == NPCID_SPARK || npc.Type == NPCID_METROID_ZOOMER || npc.Type == NPCID_SPIKE_TOP) // sparky
    {
        if(npc.Type == NPCID_SPARK)
            F = 2;
        else
            F = 1;

        tempBool = false;
        tempBool2 = false;

        if(npc.Special == 0.0)
        {
            if(Maths::iRound(npc.Direction) == 0)
            {
                if(iRand() % 2 == 1)
                    npc.Direction = 1;
                else
                    npc.Direction = -1;
            }

            npc.Special = 1;
            npc.Special2 = npc.Direction;
        }

        if(npc.Slope > 0)
        {
            if(npc.Special == 2)
            {
                npc.Special2 = 1;
                npc.Special = 1;
            }
            else if(npc.Special == 4)
            {
                npc.Special2 = -1;
                npc.Special = 1;
            }
        }

        npc.Special5 += 1;
        if(npc.Special5 >= 8 && npc.Special == 1)
        {
            npc.Special5 = 8;
            npc.Special = 0;
            npc.Location.SpeedY += Physics.NPCGravity;
            if(npc.Location.SpeedY > 8)
                npc.Location.SpeedY = 8;
        }

        if(npc.Special == 1)
        {
            npc.Location.SpeedY = F * std::abs(npc.Special2);
            npc.Location.SpeedX = F * npc.Special2;
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = npc.Location.X + npc.Location.Width / 2.0 - 1 + 18 * npc.Special2;
            tempLocation.Y = npc.Location.Y + npc.Location.Height / 2.0 - 1;
            // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            // blockTileGet(tempLocation, fBlock, lBlock);

            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                int i = block - &Block[1] + 1;
                if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                {
                    if(CheckCollision(tempLocation, Block[i].Location) && BlockSlope[Block[i].Type] == 0)
                    {
                        if(npc.Special2 == 1)
                        {
                            npc.Location.SpeedY = 0;
                            npc.Special = 2;
                            npc.Special2 = -1;
                        }
                        else
                        {
                            npc.Location.SpeedY = 0;
                            npc.Special = 4;
                            npc.Special2 = -1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                tempLocation.Width = npc.Location.Width + 2;
                tempLocation.Height = 8;
                tempLocation.X = npc.Location.X;
                tempLocation.Y = npc.Location.Y + npc.Location.Height;

                // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                // blockTileGet(tempLocation, fBlock, lBlock);

                for(Block_t* block : treeBlockQuery(tempLocation, false))
                {
                    int i = block - &Block[1] + 1;
                    if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                    {
                        if(CheckCollision(tempLocation, Block[i].Location))
                        {
                            npc.Special3 = i;
                            tempBool2 = true;
                            break;
                        }
                    }
                }

                if(!tempBool2)
                {
                    if(npc.Special2 == 1)
                    {
                        if(npc.Special3 > 0)
                        {
                            npc.Location.X = Block[npc.Special3].Location.X + Block[npc.Special3].Location.Width + 2;
                            npc.Location.Y = npc.Location.Y + 2;
                        }
                        npc.Special = 4;
                        npc.Special2 = 1;
                    }
                    else
                    {
                        npc.Special = 2;
                        npc.Special2 = 1;
                    }
                }
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedY = F * npc.Special2;
            npc.Location.SpeedX = std::abs(npc.Special2);
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = npc.Location.X + npc.Location.Width / 2.0 - 1;
            tempLocation.Y = npc.Location.Y + npc.Location.Height / 2.0 - 1 + 18 * npc.Special2;

            // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            // blockTileGet(tempLocation, fBlock, lBlock);

            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                int i = block - &Block[1] + 1;
                if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                {
                    if(CheckCollision(tempLocation, Block[i].Location))
                    {
                        if(npc.Special2 == 1)
                        {
                            npc.Special = 1;
                            npc.Special2 = -1;
                        }
                        else
                        {
                            npc.Special = 3;
                            npc.Special2 = -1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                tempLocation.Width = 8;
                tempLocation.Height = npc.Location.Height;
                tempLocation.Y = npc.Location.Y;
                tempLocation.X = npc.Location.X + npc.Location.Width;
                // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                // blockTileGet(tempLocation, fBlock, lBlock);

                for(Block_t* block : treeBlockQuery(tempLocation, false))
                {
                    int i = block - &Block[1] + 1;
                    if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                    {
                        if(CheckCollision(tempLocation, Block[i].Location))
                        {
                            tempBool2 = true;
                            break;
                        }
                    }
                }

                if(!tempBool2)
                {
                    if(npc.Special2 == 1)
                    {
                        npc.Special = 3;
                        npc.Special2 = 1;
                    }
                    else
                    {
                        npc.Special = 1;
                        npc.Special2 = 1;
                    }
                }
            }
        }
        else if(npc.Special == 3)
        {
            npc.Location.SpeedY = -std::abs(npc.Special2);
            npc.Location.SpeedX = F * npc.Special2;
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = npc.Location.X + npc.Location.Width / 2.0 - 1 + 18 * npc.Special2;
            tempLocation.Y = npc.Location.Y + npc.Location.Height / 2.0 - 1;
            // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            // blockTileGet(tempLocation, fBlock, lBlock);

            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                int i = block - &Block[1] + 1;
                if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                {
                    if(CheckCollision(tempLocation, Block[i].Location) && BlockSlope2[Block[i].Type] == 0)
                    {
                        if(npc.Special2 == 1)
                        {
                            npc.Special = 2;
                            npc.Special2 = 1;
                        }
                        else
                        {
                            npc.Special = 4;
                            npc.Special2 = 1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                tempLocation.Width = npc.Location.Width;
                tempLocation.Height = 8;
                tempLocation.X = npc.Location.X;
                tempLocation.Y = npc.Location.Y - 8;
                // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                // blockTileGet(tempLocation, fBlock, lBlock);

                for(Block_t* block : treeBlockQuery(tempLocation, false))
                {
                    int i = block - &Block[1] + 1;
                    if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                    {
                        if(CheckCollision(tempLocation, Block[i].Location))
                        {
                            tempBool2 = true;
                            if(BlockSlope2[Block[i].Type] != 0)
                                npc.Location.SpeedY = npc.Location.SpeedY * F;
                        }
                    }
                }

                if(!tempBool2)
                {
                    if(npc.Special2 == 1)
                    {
                        npc.Special = 4;
                        npc.Special2 = -1;
                    }
                    else
                    {
                        npc.Special = 2;
                        npc.Special2 = -1;
                    }
                }
            }
        }
        else if(npc.Special == 4)
        {
            npc.Location.SpeedY = F * npc.Special2;
            npc.Location.SpeedX = -std::abs(npc.Special2);
            tempBool = false;
            tempLocation.Width = 2;
            tempLocation.Height = 2;
            tempLocation.X = npc.Location.X + npc.Location.Width / 2.0 - 1;
            tempLocation.Y = npc.Location.Y + npc.Location.Height / 2.0 - 1 + 18 * npc.Special2;
            // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            // blockTileGet(tempLocation, fBlock, lBlock);

            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                int i = block - &Block[1] + 1;
                if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                {
                    if(CheckCollision(tempLocation, Block[i].Location) && BlockSlope2[Block[i].Type] == 0)
                    {
                        if(npc.Special2 == 1)
                        {
                            npc.Special = 1;
                            npc.Special2 = 1;
                        }
                        else
                        {
                            npc.Special = 3;
                            npc.Special2 = 1;
                        }
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                tempLocation.Width = 8;
                tempLocation.Height = npc.Location.Height;
                tempLocation.Y = npc.Location.Y;
                tempLocation.X = npc.Location.X - 8;
                // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
                // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                // blockTileGet(tempLocation, fBlock, lBlock);

                for(Block_t* block : treeBlockQuery(tempLocation, false))
                {
                    int i = block - &Block[1] + 1;
                    if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type] && !BlockIsSizable[Block[i].Type] && !BlockOnlyHitspot1[Block[i].Type])
                    {
                        if(CheckCollision(tempLocation, Block[i].Location))
                        {
                            tempBool2 = true;
                            break;
                        }
                    }
                }

                if(!tempBool2)
                {
                    if(npc.Special2 == 1)
                    {
                        npc.Special = 3;
                        npc.Special2 = -1;
                    }
                    else
                    {
                        npc.Special = 1;
                        npc.Special2 = -1;
                    }
                }
            }
        }



    }
    else if(npc.Type == NPCID_WART) // Wart
    {
        npc.Direction = npc.DefaultDirection;

        if(npc.Immune > 0)
            npc.Special = 2;

        if(npc.Damage >= 30)
        {
            if(npc.Special != 3)
                PlaySound(SFX_WartKilled);
            npc.Special = 3;
        }

        if(npc.Special == 0)
        {
            npc.Special3 += 1;

            if(npc.Special3 >= 160 + dRand() * 140)
            {
                npc.Special = 1;
                npc.Special3 = 0;
            }

            if(npc.Special2 == 1)
            {
                npc.Location.SpeedX = 1;
                if(npc.Location.X > npc.DefaultLocation.X + npc.Location.Width * 1)
                    npc.Special2 = -1;
            }
            else if(npc.Special2 == -1)
            {
                npc.Location.SpeedX = -1;
                if(npc.Location.X < npc.DefaultLocation.X - npc.Location.Width * 1)
                    npc.Special2 = 1;
            }
            else
                npc.Special2 = npc.Direction;
        }
        else if(npc.Special == 1)
        {
            npc.Location.SpeedX = 0;
            if(npc.Special3 == 0)
                PlaySound(SFX_WartBubbles);

            npc.Special3 = npc.Special3 + 1;
            if((int(npc.Special3) % 10) == 0)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Inert = npc.Inert;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2.0 - 16 + (32 * npc.Direction);
                NPC[numNPCs].Location.Y = npc.Location.Y + 18;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Type = 202;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 50;
                NPC[numNPCs].Location.SpeedY = -7;
                NPC[numNPCs].Location.SpeedX = 7 * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY + dRand() * 6 - 3;
                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * (1 - (npc.Special3 / 140));
                syncLayers_NPC(numNPCs);
            }

            if(npc.Special3 >= 120 + dRand() * 40)
            {
                npc.Special = 0;
                npc.Special3 = 0;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedX = 0;
            npc.Special4 += 1;
            if(npc.Special4 >= 120)
            {
                npc.Special4 = 0;
                npc.Special = 0;
            }
        }
        else if(npc.Special == 3)
        {
            npc.Location.SpeedX = 0;
            npc.Special4 += 1;
            if(npc.Special4 >= 120)
                npc.Killed = 3;
        }

    }
    // Platform movement
    else if(npc.Type == NPCID_YELBLOCKS || npc.Type == NPCID_BLUBLOCKS || npc.Type == NPCID_GRNBLOCKS ||
            npc.Type == NPCID_REDBLOCKS || npc.Type == NPCID_PLATFORM_SMB3 || npc.Type == NPCID_SAW)
    {
        straightLine = false; // SET BUT NOT USED
        UNUSED(straightLine);
        tempBool = false;

        for(int B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Section == npc.Section)
                tempBool = true;
        }

        if(npc.Type == NPCID_SAW) // Grinder
        {
            npc.Location.X -= 24;
            npc.Location.Width = 96;
            npc.Location.Y += 8;
            npc.Location.Height = 32;
        }

        if((npc.Direction == 1 && tempBool) || npc.Type == 179) // Player in same section, enabled, or, grinder
        {
            bool pausePlatforms = false;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                {
                    pausePlatforms = true;
                }
            }

            if(!g_compatibility.fix_platforms_acceleration || !pausePlatforms) // Keep zeroed speed when player required the pause of the move effect
            {
                npc.Location.SpeedY = npc.Special;
                npc.Location.SpeedX = npc.Special2;
            }

            tempBool = false;
            tempBool2 = false;
            tempLocation = npc.Location;
            tempLocation.Y = npc.Location.Y + 15;
            tempLocation.Height = 2;
            tempLocation.X = npc.Location.X + 47;
            tempLocation.Width = 2;

            C = 2; // The Speed
            D = 0;
            E = 0;
            F = 0;
            tempNPC = npc;

            for(int i = 1; i <= numBackground; i++)
            {
                // Any nearest BGO touched? (rails and reverse buffers)
                if((Background[i].Type >= 70 && Background[i].Type <= 74) || Background[i].Type == 100)
                {
                    // Not hidden
                    if(!Background[i].Hidden)
                    {
                        if(CheckCollision(tempLocation, Background[i].Location))
                        {
                            if(F > 0)
                            {
                                if(Background[i].Type == npc.Special5 || Background[i].Type == 70 || Background[i].Type == 100)
                                {
                                    F = 0;
                                    E = 0;
                                    D = 0;
                                    npc = tempNPC;
                                }
                            }

                            if(F == 0)
                            {
                                // Vertical rail
                                if(Background[i].Type == 72)
                                {
                                    if(npc.Location.SpeedY <= 0)
                                        npc.Location.SpeedY = -C;
                                    else
                                        npc.Location.SpeedY = C;
                                    npc.Location.SpeedX = 0;
                                    E = -npc.Location.X + Background[i].Location.X - 32;
                                }
                                // Horizontal rail
                                else if(Background[i].Type == 71)
                                {
                                    if(npc.Location.SpeedX >= 0)
                                        npc.Location.SpeedX = C;
                                    else
                                        npc.Location.SpeedX = -C;
                                    npc.Location.SpeedY = 0;
                                    D = -npc.Location.Y + Background[i].Location.Y;
                                }
                                // Diagonal rail left-bottom, right-top
                                else if(Background[i].Type == 73)
                                {
                                    if(npc.Location.SpeedY < 0)
                                        npc.Location.SpeedX = C;
                                    else if(npc.Location.SpeedY > 0)
                                        npc.Location.SpeedX = -C;
                                    if(npc.Location.SpeedX > 0)
                                        npc.Location.SpeedY = -C;
                                    else if(npc.Location.SpeedX < 0)
                                        npc.Location.SpeedY = C;
                                }
                                // Diagonal rail left-top, right-bottom
                                else if(Background[i].Type == 74)
                                {
                                    if(npc.Location.SpeedY < 0)
                                        npc.Location.SpeedX = -C;
                                    else if(npc.Location.SpeedY > 0)
                                        npc.Location.SpeedX = C;
                                    if(npc.Location.SpeedX > 0)
                                        npc.Location.SpeedY = C;
                                    else if(npc.Location.SpeedX < 0)
                                        npc.Location.SpeedY = -C;
                                }
                                // Reverse buffer
                                else if(Background[i].Type == 70 || Background[i].Type == 100)
                                {
                                    npc.Location.SpeedX = -npc.Location.SpeedX;
                                    npc.Location.SpeedY = -npc.Location.SpeedY;
                                    tempBool = true;
                                    break;
                                }

                                tempBool = true;
                                F = Background[i].Type;
                            }
                        }
                    }//Not hidden
                } // any important BGO?
            } // for BGOs

            npc.Special5 = F;

            if(!tempBool)
            {
                if(npc.Type == NPCID_PLATFORM_SMB3 && npc.Wet == 2)
                    npc.Location.SpeedY -= Physics.NPCGravity * 0.25;
                else
                    npc.Location.SpeedY += Physics.NPCGravity;
            }
            else
            {
                npc.Location.SpeedX += E;
                npc.Location.SpeedY += D;
            }

            if(!g_compatibility.fix_platforms_acceleration || !pausePlatforms)
            {
                npc.Special = npc.Location.SpeedY;
                npc.Special2 = npc.Location.SpeedX;
            }

            if(pausePlatforms) // Or zero the speed and don't change special values
            {
                npc.Location.SpeedX = 0;
                npc.Location.SpeedY = 0;
            }

//            for(B = 1; B <= numPlayers; B++) // Move this code to up
//            {
//                if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
//                {
//                    npc.Location.SpeedX = 0;
//                    npc.Location.SpeedY = 0;
//                }
//            }
        }
        else
        {
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;
        }

        Block[npc.tempBlock].Location = npc.Location;
        Block[npc.tempBlock].Location.X = Block[npc.tempBlock].Location.X + npc.Location.SpeedX;

        if(npc.Location.SpeedY < 0)
            Block[npc.tempBlock].Location.Y += npc.Location.SpeedY;

        // necessary for tree update
        Block[npc.tempBlock].LocationInLayer = Block[npc.tempBlock].Location;
        if(Block[npc.tempBlock].LayerIndex != -1)
        {
            Block[npc.tempBlock].LocationInLayer.X -= Layer[Block[npc.tempBlock].LayerIndex].OffsetX;
            Block[npc.tempBlock].LocationInLayer.Y -= Layer[Block[npc.tempBlock].LayerIndex].OffsetY;
        }
        treeBlockUpdateLayer(Block[npc.tempBlock].LayerIndex, &Block[npc.tempBlock]);

        if(npc.Type == 179)
        {
            npc.Location.X += 24;
            npc.Location.Width = 48;
            npc.Location.Y -= 8;
            npc.Location.Height = 48;

            if(npc.Location.SpeedX == 0 && fEqual((float)npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Location.SpeedX = C * npc.Direction;
                npc.Special2 = npc.Location.SpeedX;
            }
        }
    }
    else if(npc.Type == NPCID_BOWSER_SMB) // King Koopa
    {
        C = 0;
        for(int i = 1; i <= numPlayers; i++)
        {
            auto &p = Player[i];
            if(!p.Dead && p.Section == npc.Section)
            {
                if(C == 0.0 || std::abs(npc.Location.X + npc.Location.Width / 2.0 - (p.Location.X + p.Location.Width / 2.0)) < C)
                {
                    C = std::abs(npc.Location.X + npc.Location.Width / 2.0 - (p.Location.X + p.Location.Width / 2.0));
                    D = i;
                }
            }
        }

        if(Player[D].Location.X + Player[D].Location.Width / 2.0 > npc.Location.X + 16)
            npc.Direction = 1;
        else
            npc.Direction = -1;

        npc.Special5 = D;

        if(dRand() * 300 > 297 && npc.Special == 0.0)
            npc.Special = 1;

        npc.Special4 += 1;

        if(npc.Inert)
            npc.Special4 = 150;

        if(npc.Special4 <= 80 + dRand() * 40)
        {
            if((dRand() * 100 > 40) && int(npc.Special4) % 16 == 0)
            {
                PlaySound(SFX_HammerToss);
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Inert = npc.Inert;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2.0 - 16;
                NPC[numNPCs].Location.Y = npc.Location.Y - 32;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Type = NPCID_ENEMYHAMMER;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 50;
                NPC[numNPCs].Layer = "Spawned NPCs";
                NPC[numNPCs].Location.SpeedY = -8;
                NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;
                syncLayers_NPC(numNPCs);
            }
        }
        else if(npc.Special4 > 300 + dRand() * 50)
            npc.Special4 = 0;

        if(npc.Inert)
            npc.Special = 0;

        if(npc.Special > 0)
        {
            npc.Special3 = npc.Special3 + 1;
            if(npc.Special3 < 40)
                npc.Special = 1;
            else if(npc.Special3 < 70)
            {
                if(npc.Special3 == 40)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Direction = npc.Direction;
                    NPC[numNPCs].Section = npc.Section;
                    NPC[numNPCs].Layer = "Spawned NPCs";
                    NPC[numNPCs].Type = NPCID_EXT_FIRE_A;
                    if(NPC[numNPCs].Direction == 1)
                        NPC[numNPCs].Frame = 4;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    if(NPC[numNPCs].Direction == -1)
                        NPC[numNPCs].Location.X = npc.Location.X - 40;
                    else
                        NPC[numNPCs].Location.X = npc.Location.X + 54;
                    NPC[numNPCs].Location.Y = npc.Location.Y + 19;
                    NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
                    C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2.0);
                    D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[npc.Special5].Location.Y + Player[npc.Special5].Location.Height / 2.0);
                    NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                    if(NPC[numNPCs].Location.SpeedY > 1)
                        NPC[numNPCs].Location.SpeedY = 1;
                    else if(NPC[numNPCs].Location.SpeedY < -1)
                        NPC[numNPCs].Location.SpeedY = -1;
                    syncLayers_NPC(numNPCs);
                    PlaySound(SFX_BigFireball);
                }
                npc.Special = 2;
            }
            else
            {
                npc.Special = 0;
                npc.Special3 = 0;
            }
        }
        else if(npc.Special == 0)
        {
            if(npc.Special2 == 0)
            {
                npc.Location.SpeedX = -0.5;
                if(npc.Location.X < npc.DefaultLocation.X - npc.Location.Width * 1.5)
                    npc.Special2 = 1;
            }
            else
            {
                npc.Location.SpeedX = 0.5;
                if(npc.Location.X > npc.DefaultLocation.X + npc.Location.Width * 1.5)
                    npc.Special2 = 0;
            }
            if(fEqual(npc.Location.SpeedY, double(Physics.NPCGravity)) || npc.Slope > 0)
            {
                if(dRand() * 200 >= 198)
                    npc.Location.SpeedY = -8;
            }
        }


    }
    else if(npc.Type == NPCID_GOALTAPE) // SMW Exit
    {
        if(npc.Special == 0)
            npc.Location.SpeedY = 2;
        else
            npc.Location.SpeedY = -2;

        if(npc.Location.Y <= npc.DefaultLocation.Y)
            npc.Special = 0;

        if(npc.Special2 == 0)
        {
            tempLocation = npc.Location;
            tempLocation.Height = 8000;
            C = 0;
            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                int i = block - &Block[1] + 1;
                if(CheckCollision(tempLocation, Block[i].Location))
                {
                    if(C == 0)
                        C = i;
                    else
                    {
                        if(Block[i].Location.Y < Block[C].Location.Y)
                            C = i;
                    }
                }
            }
            if(C > 0)
                npc.Special2 = Block[C].Location.Y + 4;
        }

        for(int i = 1; i <= numPlayers; i++)
        {
            auto &p = Player[i];
            if(p.Section == npc.Section)
            {
                if(p.Location.Y + npc.Location.Height <= npc.Special2)
                {
                    if(p.Location.X + p.Location.Width >= npc.Location.X + npc.Location.Width - 8)
                    {
                        if(p.Location.X <= npc.Location.X + 80)
                        {
                            if(CheckCollision(p.Location, npc.Location))
                            {
                                MoreScore(vb6Round((1 - (npc.Location.Y - npc.DefaultLocation.Y) / (npc.Special2 - npc.DefaultLocation.Y)) * 10) + 1, npc.Location);
                                npc.Killed = 9;
                                PlaySound(SFX_Twomp);
                            }

                            FreezeNPCs = false;
                            TurnNPCsIntoCoins();

                            if(numPlayers > 2 /*&& nPlay.Online == false*/)
                                Player[1] = Player[A];

                            LevelMacro = LEVELMACRO_GOAL_TAPE_EXIT;

                            for(int j = 1; j <= numPlayers; j++)
                            {
                                if(i == j) // And DScreenType <> 5 Then
                                    continue;
                                Player[j].Section = p.Section;
                                Player[j].Location.Y = p.Location.Y + p.Location.Height - p.Location.Height;
                                Player[j].Location.X = p.Location.X + p.Location.Width / 2.0 - p.Location.Width / 2.0;
                                Player[j].Location.SpeedX = 0;
                                Player[j].Location.SpeedY = 0;
                                Player[j].Effect = 8;
                                Player[j].Effect2 = -i;
                            }

                            StopMusic();
                            DoEvents();
                            PlaySound(SFX_TapeExit);
                            break;
                        }
                    }
                }
            }
        }

    }
    else if(npc.Type == NPCID_CHECKERPLATFORM)
    {
        if(fiEqual(npc.Special, 1))
        {
            npc.Location.SpeedY += Physics.NPCGravity / 4;
            npc.Special = 0;
        }
        else
            npc.Location.SpeedY = 0;
    }
    else if(npc.Type == NPCID_PLATFORM_SMB)
        npc.Location.SpeedY = npc.Direction * 2;

    else if(npc.Type == NPCID_BLARGG)
    {
        if(Maths::iRound(npc.Special) == 0.0)
        {
            tempLocation = npc.Location;
            tempLocation.Height = 400;
            tempLocation.Y -= tempLocation.Height;
            tempLocation.X += tempLocation.Width / 2.0;
            tempLocation.Width = 600;
            tempLocation.X -= tempLocation.Width / 2.0;

            for(int i = 1; i <= numPlayers; i++)
            {
                if(CheckCollision(tempLocation, Player[i].Location))
                {
                    if(Player[i].Location.X + Player[i].Location.Width / 2.0 < npc.Location.X + npc.Location.Width / 2.0)
                        npc.Direction = -1;
                    else
                        npc.Direction = 1;
                    npc.Special = 1;
                    NewEffect(104, npc.Location, npc.Direction);
                    break;
                }
            }
        }
        else if(Maths::iRound(npc.Special) == 1)
        {
            npc.Special2 += 1;
            if(npc.Special2 == 90)
            {
                npc.Location.SpeedX = 1 * npc.Direction;
                npc.Location.SpeedY = -4.2;
                npc.Special = 2;
                PlaySound(SFX_Blaarg);
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedY += Physics.NPCGravity * 0.4;
            if(npc.Location.Y > npc.DefaultLocation.Y + npc.Location.Height + 48)
                Deactivate(A);
        }


        // End If
        // Sniffits
    }
    else if(npc.Type >= 130 && npc.Type <= 132)
    {
        if(npc.Projectile)
        {
            npc.Special = 0;
            npc.Special2 = 0;
        }

        tempBool = false;

        if(npc.Type < 132)
        {
            npc.Special += 1;
            if(npc.Special > 120)
            {
                npc.FrameCount -= 1;

                if(npc.Special2 == 0)
                {
                    npc.Special2 = 1;
                    npc.Location.X -= 2;
                }
                else
                {
                    npc.Special2 = 0;
                    npc.Location.X += 2;
                }

                npc.Location.SpeedX = 0;

                if(npc.Special >= 150)
                {
                    tempBool = true;
                    npc.Special = 0;
                }
            }
        }
        else
        {
            npc.Special += 1;

            if(npc.Special > 160)
            {
                tempBool = true;

                if(npc.Special3 != 2)
                {
                    npc.Special3 -= 1;
                    npc.Special = 0;
                }
                else
                {
                    npc.Special = 140;
                    npc.Special3 -= 1;
                    if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity) || npc.Slope > 0)
                        npc.Special2 = 90;
                }

                if(npc.Special3 < 0)
                    npc.Special3 = 2;
            }

            if(fEqual((float)npc.Location.SpeedY, Physics.NPCGravity) || npc.Slope > 0)
            {
                npc.Special2 += 1;
                if(npc.Special2 >= 100)
                {
                    npc.Special2 = 0;
                    npc.Location.SpeedY = -3.9;
                    npc.Location.Y -= Physics.NPCGravity;
                }
            }
            else
                npc.FrameCount -= 1;
        }

        if(tempBool)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Section = npc.Section;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Type = 133;
            NPC[numNPCs].Layer = npc.Layer;
            NPC[numNPCs].Inert = npc.Inert;
            NPC[numNPCs].Direction = npc.Direction;
            NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
            NPC[numNPCs].Location.Width = 16;
            NPC[numNPCs].Location.Height = 16;
            NPC[numNPCs].Location.X = npc.Location.X + 8 + 16 * NPC[numNPCs].Direction;
            NPC[numNPCs].Location.Y = npc.Location.Y + 13;
            syncLayers_NPC(numNPCs);
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
            if(Player[B].Character == 5 && !Player[B].Dead && Player[B].TimeToLive == 0 &&
               Player[B].Effect == 0 && Player[B].SwordPoke == 0 && !Player[B].Fairy &&
               !(NPC[A].Type == 13 && NPC[A].CantHurtPlayer == B) &&
               !(NPC[A].Type == 171 && NPC[A].CantHurtPlayer == B))
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
#if XTECH_ENABLE_WEIRD_GFX_UPDATES
                    UpdateGraphics(true);
#endif
                    PlaySound(SFX_ZeldaShield);
                    if(NPC[A].Type == 133)
                        NPC[A].Killed = 3;
                    else
                    {
                        NPC[A].Killed = 9;

                        if(NPC[A].Type == 13 || NPC[A].Type == 265)
                            NPC[A].Killed = 3;

                        if(NPC[A].Type != 30 && NPC[A].Type != 202 && NPC[A].Type != 210 &&
                           NPC[A].Type != 171 && NPC[A].Type != 13 && NPC[A].Type != 265)
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
        if(NPC[A].Projectile)
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
        double npcHCenter = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
        double npcVCenter = NPC[A].Location.Y + NPC[A].Location.Height / 2.0;
        double playerHCenter = Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0;
        double playerVCenter = Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0;

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

        if(npcHCenter > playerHCenter)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
            if(NPC[A].Location.SpeedX > -4 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.5;

        }
        else if(npcHCenter < playerHCenter)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
            if(NPC[A].Location.SpeedX > -4 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.5;
        }

        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + (playerHCenter - npcHCenter) * 0.0005;

        if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 > playerVCenter)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.2;
            if(NPC[A].Location.SpeedY > 0 && NPC[A].Direction != NPC[A].Special6)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - std::abs(NPC[A].Location.SpeedY) * 0.04;
        }
        else if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 < playerVCenter)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.2;
            if(NPC[A].Location.SpeedY < 0 && NPC[A].Direction != NPC[A].Special6)
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + std::abs(NPC[A].Location.SpeedY) * 0.04;
        }
        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + (playerVCenter - NPC[A].Location.Y + NPC[A].Location.Height / 2.0) * 0.004;


        for(B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Active)
            {
                if(NPCIsACoin[NPC[B].Type])
                {
                    if(CheckCollision(NPC[A].Location, NPC[B].Location))
                    {
                        NPC[B].Location.X = npcHCenter - NPC[B].Location.Width / 2.0;
                        NPC[B].Location.Y = npcVCenter - NPC[B].Location.Height / 2.0;
                        NPC[B].Special = 0;
                        NPC[B].Projectile = false;
                    }
                }
            }
        }

        bool atCenter = (npcHCenter > playerHCenter && fiEqual(NPC[A].Special6, 1)) ||
                        (npcHCenter < playerHCenter && fiEqual(NPC[A].Special6, -1));
        if(!atCenter)
        {
            NPC[A].Special2 = 1;
            if(CheckCollision(NPC[A].Location, Player[NPC[A].Special5].Location))
            {
                NPC[A].Killed = 9;
                Player[NPC[A].Special5].FrameCount = 115;
                PlaySound(SFX_Grab2);
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Active)
                    {
                        if(NPCIsACoin[NPC[B].Type])
                        {
                            if(CheckCollision(NPC[A].Location, NPC[B].Location))
                            {
                                NPC[B].Location.X = playerHCenter - NPC[B].Location.Width / 2.0;
                                NPC[B].Location.Y = playerVCenter - NPC[B].Location.Height / 2.0;
                                TouchBonus(vb6Round(NPC[A].Special5), B);
                            }
                        }
                    }
                }
            }
        }

        if(NPC[A].Special2 == 1)
        {
            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > playerHCenter)
            {
                if(NPC[A].Location.SpeedX > 0)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.1;
            }
            else if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < playerHCenter)
            {
                if(NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1;
            }
            if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 > playerVCenter)
            {
                if(NPC[A].Location.SpeedY > 0)
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.3;
            }
            else if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 < playerVCenter)
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
        if(NPC[A].Projectile)
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
                            C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                            D = (float)B;
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
        if(!NPC[A].Projectile)
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
                        C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = (float)B;
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
            PlaySound(SFX_BirdoBeat);
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
                    C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
            else if(fiEqual(NPC[A].Special2, 2))
            {
                NPC[A].Special = NPC[A].Special + 1;
                if(NPC[A].Special >= 100)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
                else if(fiEqual(NPC[A].Special, 50))
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
                    syncLayers_NPC(numNPCs);
                }
            }
            else if(fiEqual(NPC[A].Special2, 3))
            {
                NPC[A].Special = NPC[A].Special + 1;
                NPC[A].Location.Y = NPC[A].Location.Y + 1.5;
                if(NPC[A].Special >= NPCHeight[NPC[A].Type] * 0.65 + 1)
                    NPC[A].Special2 = 4;
            }
            else if(fiEqual(NPC[A].Special2, 4))
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
        if(NPC[A].Projectile)
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

                NPC[A].Special2 += 1;

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
                    NPC[A].Location.Y = vb6Round(NPC[A].Location.Y);
                    NPC[A].Location.Height = NPCHeight[NPC[A].Type];
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
            NPC[A].Location.Y -= NPC[A].Location.SpeedY;
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
                    NPC[A].Location.Height = NPCHeight[NPC[A].Type];
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
                    NPC[A].Location.Width = NPCWidth[NPC[A].Type];
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
        NPC[A].Location.SpeedX = 0.8 * NPC[A].DefaultDirection * (float)BeltDirection;
        NPC[A].Location.X = NPC[A].DefaultLocation.X;
        NPC[A].Location.Y = NPC[A].DefaultLocation.Y;
        NPC[A].Direction = NPC[A].DefaultDirection * (float)BeltDirection;
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
            PlaySound(SFX_Lava);
            tempLocation = NPC[A].Location;
            tempLocation.Y -= 32;
            NewEffect(13, tempLocation);
        }
        else if(fiEqual(NPC[A].Special2, 1))
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
        else if(fiEqual(NPC[A].Special2, 2))
        {
            NPC[A].Special += 1;

            if(fiEqual(NPC[A].Special, 61))
            {
                tempLocation = NPC[A].Location;
                tempLocation.Y = tempLocation.Y + 2;
                NewEffect(13, tempLocation);
                PlaySound(SFX_Lava);
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
    else if((NPC[A].Type == 46 || NPC[A].Type == 212) && LevelMacro == LEVELMACRO_OFF)
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
                        C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = (float)B;
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
                        C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = (float)B;
                    }
                }
            }

            if(Player[D].Location.X + Player[D].Location.Width / 2.0 > NPC[A].Location.X + 16)
                NPC[A].Direction = 1;
            else
                NPC[A].Direction = -1;
        }

        if(NPC[A].Stuck && !NPC[A].Projectile)
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
                if(C == 0 || SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
            NPC[numNPCs].FrameCount = (float)(iRand() % 8);
            syncLayers_NPC(numNPCs);
            PlaySound(SFX_BigFireball);
        }
    // Hammer Bro
    }
    else if(NPC[A].Type == 29 && !NPC[A].Projectile)
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
            {
                if(C == 0.f || SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
            PlaySound(SFX_HammerToss);
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
            syncLayers_NPC(numNPCs);
        }
    // leaf
    }
    else if(NPC[A].Type == 34) // Leaf
    {
        if(NPC[A].Stuck && !NPC[A].Projectile)
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
            if(NPC[A].Projectile)
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
                if(C == 0 || (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    D = (float)B;
                }
            }
        }
        C = D;
        if(C > 0)
        {
            if(NPC[A].Special == 0)
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
                D = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D += (float)SDL_fabs(Player[C].Location.SpeedX) / 2;
                if(NPC[A].Location.SpeedX < -5 - D)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[C].Location.X + Player[C].Location.Width / 2.0 - 50 + (Player[C].Location.SpeedX * 15))
                    NPC[A].Special = 1;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                D = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D += (float)SDL_fabs(Player[C].Location.SpeedX) / 2;
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

            if(SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) < 100)
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
                for(Block_t* block : treeBlockQuery(tempLocation, false))
                {
                    Block_t& b = *block;
                    if(!BlockNoClipping[b.Type] &&
                       !BlockIsSizable[b.Type] &&
                       !BlockOnlyHitspot1[b.Type])
                    {
                        if(CheckCollision(tempLocation, b.Location))
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
                syncLayers_NPC(numNPCs);
                syncLayers_NPC(A);
                PlaySound(SFX_HammerToss);
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
                    PlaySound(SFX_Twomp);
                    if(GameplayShakeScreenThwomp)
                        doShakeScreen(0, 4, SHAKE_SEQUENTIAL, 5, 0.2);
                    tempLocation.Width = 32;
                    tempLocation.Height = 32;
                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 16;


//                    tempLocation.X = NPC[A].Location.X;
                    tempLocation.X = (NPC[A].Location.X + NPC[A].Location.Width / 8);
                    NewEffect(10, tempLocation);
                    Effect[numEffects].Location.SpeedX = -1.5;

//                    tempLocation.X = tempLocation.X + tempLocation.Width - EffectWidth[10];
                    tempLocation.X = (NPC[A].Location.X + NPC[A].Location.Width - EffectWidth[10]) - (NPC[A].Location.Width / 8);
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
            if(NPC[A].Projectile)
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
                    if(C == 0 || SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                        D = (float)B;
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
    else if(NPC[A].Type == 39 && !NPC[A].Projectile) // birdo
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
                    if(C == 0 || SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                    {
                        C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
                    syncLayers_NPC(numNPCs);
                    PlaySound(SFX_BirdoSpit);
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
                if(C == 0 || SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                {
                    C = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                    D = (float)B;
                }
            }
        }

        C = D;

        if(C > 0)
        {
            if(NPC[A].Special6 == 0)
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.2;
                D = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D += (float)SDL_fabs(Player[C].Location.SpeedX) / 2;
                if(NPC[A].Location.SpeedX < -5 - D)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Player[C].Location.X + Player[C].Location.Width / 2.0 - 50 + (Player[C].Location.SpeedX * 15))
                    NPC[A].Special6 = 1;
            }
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.2;
                D = (float)SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) / 100;
                D += (float)SDL_fabs(Player[C].Location.SpeedX) / 2;
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

            if(SDL_fabs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[C].Location.X + Player[C].Location.Width / 2.0) < 100)
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
                for(Block_t* block : treeBlockQuery(tempLocation, false))
                {
                    Block_t& b = *block;
                    if(CheckCollision(tempLocation, b.Location) && !BlockNoClipping[b.Type])
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
                syncLayers_NPC(numNPCs);

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
            NPC[A].Special -= 1;
            NPC[A].Location.SpeedX = 0;
        }
    // beach koopa
    }
    else if(NPC[A].Type >= 117 && NPC[A].Type <= 120)
    {
        if(NPC[A].Type == 119 && NPC[A].Special > 0)
        {
            NPC[A].Special -= 1;
            NPC[A].Location.SpeedX = 0;
        }

        if(NPC[A].Projectile)
        {
            NPC[A].Location.SpeedX *= 0.96;
            if(NPC[A].Location.SpeedX > -0.003 && NPC[A].Location.SpeedX < 0.003)
            {
                NPC[A].Projectile = false;
                NPC[A].Location.Y -= Physics.NPCGravity;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Direction = -NPC[A].Direction;
            }
        }
        else
        {
            if(NPC[A].Type != 119)
            {
                if(fEqual((float)NPC[A].Location.SpeedY, Physics.NPCGravity))
                {
                    for(B = 1; B <= numNPCs; B++)
                    {
                        if(NPC[B].Active && NPC[B].Section == NPC[A].Section && !NPC[B].Hidden && NPC[B].HoldingPlayer == 0)
                        {
                            if(NPC[B].Type >= 113 && NPC[B].Type <= 116)
                            {
                                tempLocation = NPC[A].Location;
                                tempLocation2 = NPC[B].Location;
                                tempLocation.Width += 32;
                                tempLocation.X -= 16;
                                if(CheckCollision(tempLocation, tempLocation2))
                                {
                                    NPC[A].Location.Y -= Physics.NPCGravity;
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

//    if(NPC[A].Type == 13)
//        NPC[A].Projectile = true;
//    else if(NPC[A].Type == 17 && NPC[A].CantHurt > 0)
    if(NPC[A].Type == 13 || (NPC[A].Type == 17 && NPC[A].CantHurt > 0))
        NPC[A].Projectile = true;
    else if(NPC[A].Type == 12) // Stop the big fireballs from getting killed from tha lava
        NPC[A].Projectile = false;
    else if(NPC[A].Type == 50) // killer plant destroys blocks
    {
        for(Block_t* block : treeBlockQuery(NPC[A].Location, false))
        {
            B = block - &Block[1] + 1;
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
    default:
    case 0:
        return NPCID_SHROOM_SMB3;
    case 1:
        return NPCID_FIREFLOWER_SMB3;
    case 2:
        return NPCID_LEAF;
    case 3:
        return NPCID_TANOOKISUIT;
    case 4:
        return NPCID_HAMMERSUIT;
    case 5:
        return NPCID_ICEFLOWER_SMB3;
    }

    return 0;
}
