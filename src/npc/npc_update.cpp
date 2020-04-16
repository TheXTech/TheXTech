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
#include "../layers.h"
#include "../editor.h"
#include "../blocks.h"
#include "../sorting.h"

#include <Utils/maths.h>

static RangeArr<int, 0, maxNPCs> newAct;
// Why this array is here? to don't reallocate it every call of UpdateNPCs()

void UpdateNPCs()
{
    // this is 1 of the 2 clusterfuck subs in the code, be weary

    // misc variables used mainly for arrays
    int A = 0;
    int B = 0;
//    float C = 0;
//    float D = 0;
//    double E = 0;
//    float F = 0;

//    std::string tempStr;
    int oldSlope = 0; // previous sloped block the npc was on
//    NPC_t tempNPC;
    int HitSpot = 0; // used for collision detection
    double tempHit = 0;
    Block_t tmpBlock;
    int tempHitBlock = 0;
    float tempSpeedA = 0;
//    float tempSpeedB = 0;
    bool tempTurn = false; // used for turning the npc around
    Location_t tempLocation;
    Location_t tempLocation2;
    Location_t preBeltLoc;
    float beltCount = 0;
    int tempBlockHit[3] = {0}; // Hit block from below code
    int winningBlock = 0; // More code for hitting the block from below
    int numTempBlock = 0;
    float speedVar = 0; // percent of the NPC it should actually moved. this helps when underwater

    bool tempBool = false;
//    bool tempBool2 = false;
//    bool tempBool3 = false;
    float newY = 0;
//    bool straightLine = false;
    Block_t blankBlock;
//    bool noBelt = false;
    float oldBeltSpeed = 0;
//    float beltFixX = 0;
    int oldDirection = 0;

    // used for collision detection
    long long fBlock = 0;
    long long lBlock = 0;
    long long fBlock2 = 0;
    long long lBlock2 = 0;
    int bCheck2 = 0;
    int bCheck = 0;
    float addBelt = 0;
    int numAct = 0;
    bool beltClear = false; // stops belt movement when on a wall
    bool resetBeltSpeed = false;
    double PlrMid = 0;
    double Slope = 0;
    bool SlopeTurn = false;
//    std::string timeStr;

    double lyrX = 0; // for attaching to layers
    double lyrY = 0; // for attaching to layers

    newAct.fill(0);

    NPC[0].Location.SpeedX = 0;
    NPC[0].Location.SpeedY = 0;

    if(LevelMacro > 0)
    {
        if(PSwitchTime > 0)
            PSwitchTime = 100;
        if(PSwitchStop > 0)
            PSwitchStop = 100;
    }

    // used for the p switches
    if(PSwitchStop > 0) // time stop
    {
        if(PSwitchStop == Physics.NPCPSwitch)
        {
            StopMusic();
            StartMusic(-1);
            PlaySound(32);
        }
        if(PSwitchTime > 2)
            PSwitchTime = 2;
        PSwitchStop = PSwitchStop - 1;
        if(PSwitchStop <= 0)
        {
            FreezeNPCs = false;
            StopMusic();
            StartMusic(Player[PSwitchPlayer].Section);
        }
    }

    if(FreezeNPCs == true) // When time is paused
    {
        StopHit = 0;
        for(A = numNPCs; A >= 1; A--) // check to see if NPCs should be killed
        {
            if(NPCIsBoot[NPC[A].Type] || NPCIsYoshi[NPC[A].Type])
            {
                if(NPC[A].CantHurt > 0)
                {
                    NPC[A].CantHurt = NPC[A].CantHurt - 1;
                    if(NPC[A].CantHurt == 0)
                        NPC[A].CantHurtPlayer = 0;
                }
            }
            if(NPC[A].TimeLeft > 0)
                NPC[A].TimeLeft = NPC[A].TimeLeft - 1;
            if(NPC[A].Immune > 0)
                NPC[A].Immune = NPC[A].Immune - 1;
            NPC[A].JustActivated = 0;
            NPC[A].Chat = false;
            if(NPC[A].TimeLeft == 0)
                Deactivate(A);
            if(NPC[A].Killed > 0)
            {
                if(NPC[A].Location.SpeedX == 0.0)
                {
                    NPC[A].Location.SpeedX = dRand() * 2 - 1;
                    if(NPC[A].Location.SpeedX < 0)
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.5;
                    else
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.5;
                }
                KillNPC(A, NPC[A].Killed);
            }
        }
        CharStuff();
        return;
    }


    if(CoinMode == true) // this is a cheat code
    {
        if(Lives >= 99 && Coins >= 99)
            CoinMode = false;
        else
        {
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
        }
    }



    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].RespawnDelay > 0)
        {
            NPC[A].Reset[1] = false;
            NPC[A].Reset[2] = false;
            NPC[A].RespawnDelay = NPC[A].RespawnDelay - 1;
        }

        if(NPC[A].Hidden == true)
            Deactivate(A);

        if(NPC[A].TailCD > 0)
            NPC[A].TailCD = NPC[A].TailCD - 1;

        if(A > maxNPCs - 100)
            NPC[A].Killed = 9;

        // generator code
        if(NPC[A].Generator == true)
        {
            NPC[A].Active = false;
            if(NPC[A].Hidden == false)
            {
                NPC[A].TimeLeft = 0;
                NPC[A].GeneratorTime = NPC[A].GeneratorTime + 1;

                if(NPC[A].GeneratorTime >= NPC[A].GeneratorTimeMax * 6.5f)
                    NPC[A].GeneratorTime = NPC[A].GeneratorTimeMax * 6.5f;

                if(NPC[A].GeneratorActive == true)
                {
                    NPC[A].GeneratorActive = false;
                    if(NPC[A].GeneratorTime >= NPC[A].GeneratorTimeMax * 6.5f)
                    {
                        tempBool = false;
                        for(B = 1; B <= numNPCs; B++)
                        {
                            if(B != A && NPC[B].Active == true && NPC[B].Type != 57)
                            {
                                if(CheckCollision(NPC[A].Location, NPC[B].Location))
                                    tempBool = true;
                            }
                        }

                        if(NPC[A].Type != 91)
                        {
                            for(B = 1; B <= numBlock; B++)
                            {
                                if(!Block[B].Hidden && !BlockIsSizable[Block[B].Type])
                                {
                                    if(CheckCollision(NPC[A].Location,
                                                      newLoc(Block[B].Location.X + 0.1, Block[B].Location.Y + 0.1,
                                                             Block[B].Location.Width - 0.2, Block[B].Location.Height - 0.2)))
                                        tempBool = true;
                                }
                            }
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].TimeToLive == 0)
                                {
                                    if(CheckCollision(NPC[A].Location, Player[B].Location) == true)
                                        tempBool = true;
                                }
                            }
                        }

                        if(numNPCs == maxNPCs - 100)
                            tempBool = true;

                        if(tempBool == true)
                            NPC[A].GeneratorTime = NPC[A].GeneratorTimeMax;
                        else
                        {
                            NPC[A].GeneratorTime = 0;
                            numNPCs++;
                            NPC[numNPCs] = NPC[A];

                            if(NPC[A].GeneratorEffect == 1) // Warp NPC
                            {
                                NPC[numNPCs].Layer = NPC[A].Layer;
                                NPC[numNPCs].Effect3 = NPC[A].GeneratorDirection;
                                NPC[numNPCs].Effect = 4;
                                NPC[numNPCs].Location.SpeedX = 0;
                                NPC[numNPCs].TimeLeft = 100;
                                if(NPC[A].GeneratorDirection == 1)
                                {
                                    if(NPCHeightGFX[NPC[A].Type] > NPC[A].Location.Height)
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPCHeightGFX[NPC[A].Type];
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y - (NPCHeightGFX[NPC[A].Type] - NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y;
                                    }
                                }
                                else if(NPC[A].GeneratorDirection == 3)
                                {
                                    if(NPCHeightGFX[NPC[A].Type] > NPC[A].Location.Height)
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y + NPC[A].Location.Height + (NPCHeightGFX[NPC[A].Type] - NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y + NPC[A].Location.Height;
                                    }
                                }
                                else if(NPC[A].GeneratorDirection == 2)
                                {
                                    NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 4;
                                    NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width;
                                    NPC[numNPCs].Effect2 = NPC[numNPCs].Location.X;
                                }
                                else if(NPC[A].GeneratorDirection == 4)
                                {
                                    NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 4;
                                    NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[A].Location.Width;
                                    NPC[numNPCs].Effect2 = NPC[numNPCs].Location.X + NPC[A].Location.Width;
                                }
                            }
                            else if(NPC[A].GeneratorEffect == 2) // projectile
                            {
                                NPC[numNPCs].Layer = "Spawned NPCs";
                                PlaySound(22);
                                NPC[numNPCs].Projectile = true;
                                if(NPC[numNPCs].Type == 17) // Normal Bullet Bills
                                    NPC[numNPCs].Projectile = false;

                                if(NPC[numNPCs].Type == 45)
                                    NPC[numNPCs].Special = 1;
                                if(NPC[A].GeneratorDirection == 1)
                                {
                                    NPC[numNPCs].Location.SpeedY = -10;
                                    NPC[numNPCs].Location.SpeedX = 0;
                                    NewEffect(10, newLoc(NPC[A].Location.X, NPC[A].Location.Y + 16, 32, 32));
                                    if(NPCIsVeggie[NPC[numNPCs].Type] == true)
                                        NPC[numNPCs].Location.SpeedX = dRand() * 2 - 1;
                                    // NPC(numNPCs).Location.SpeedY = -1
                                }
                                else if(NPC[A].GeneratorDirection == 2)
                                {
                                    NPC[numNPCs].Location.SpeedX = -Physics.NPCShellSpeed;
                                    NewEffect(10, newLoc(NPC[A].Location.X + 16, NPC[A].Location.Y, 32, 32));
                                }
                                else if(NPC[A].GeneratorDirection == 3)
                                {
                                    NPC[numNPCs].Location.SpeedY = 8;
                                    NPC[numNPCs].Location.SpeedX = 0;
                                    NewEffect(10, newLoc(NPC[A].Location.X, NPC[A].Location.Y - 16, 32, 32));
                                }
                                else
                                {
                                    NPC[numNPCs].Location.SpeedX = Physics.NPCShellSpeed;
                                    SoundPause[3] = 1;
                                    NewEffect(10, newLoc(NPC[A].Location.X - 16, NPC[A].Location.Y, 32, 32));
                                }
                            }

                            NPC[numNPCs].Direction = NPC[numNPCs].DefaultDirection;
                            NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
                            NPC[numNPCs].DefaultDirection = NPC[numNPCs].Direction;
                            NPC[numNPCs].DefaultType = 0;
                            NPC[numNPCs].Generator = false;
                            NPC[numNPCs].Active = true;
                            NPC[numNPCs].TimeLeft = 100;
                            NPC[numNPCs].TriggerActivate = NPC[A].TriggerActivate;
                            NPC[numNPCs].TriggerDeath = NPC[A].TriggerDeath;
                            NPC[numNPCs].TriggerLast = NPC[A].TriggerLast;
                            NPC[numNPCs].TriggerTalk = NPC[A].TriggerTalk;
                            CheckSectionNPC(numNPCs);
                            if(NPC[numNPCs].TriggerActivate != "")
                                ProcEvent(NPC[numNPCs].TriggerActivate);
                            if(NPC[numNPCs].Type == 287)
                                NPC[numNPCs].Type = RandomBonus();
                        }
                    }
                }
            }
        }

        if(NPC[A].Type == 57 && !NPC[A].Hidden)
        {
            CheckSectionNPC(A);
            bool sameSection = false;
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Section == NPC[A].Section)
                    sameSection = true;
            }

            if(sameSection)
            {
                NPC[A].TimeLeft = 100;
                NPC[A].Active = true;
                NPC[A].JustActivated = 0;
            }
        }
        numAct = 0;

        if(NPC[A].TimeLeft == 1 || NPC[A].JustActivated != 0)
        {
            if(NPC[A].Type == 169 || NPC[A].Type == 170)
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction, A);
        }

        if(NPC[A].JustActivated != 0)
        {
            if(NPC[A].Active && NPC[A].TimeLeft > 1 &&
               NPC[A].Type != 57 && NPC[A].Type != 46 &&
               NPC[A].Type != 212 && !NPCIsACoin[NPC[A].Type]) // And .Type <> 47
            {
                if(NPC[A].TriggerActivate != "")
                    ProcEvent(NPC[A].TriggerActivate);
                tempLocation = NPC[A].Location;
                tempLocation.Y = tempLocation.Y - 32;
                tempLocation.X = tempLocation.X - 32;
                tempLocation.Width = tempLocation.Width + 64;
                tempLocation.Height = tempLocation.Height + 64;
                for(B = 1; B <= numNPCs; B++)
                {
                    if((NPC[B].Active == false) && B != A && NPC[B].Reset[1] == true && NPC[B].Reset[2] == true)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location) == true)
                        {
                            numAct++;
                            SDL_assert_release(numAct <= maxNPCs);
                            newAct[numAct] = B;
                            NPC[B].Active = true;
                            NPC[B].TimeLeft = NPC[A].TimeLeft;
                            NPC[B].JustActivated = 1;
                            NPC[B].Section = NPC[A].Section;
                            if(B < A)
                            {
                                if(NPC[B].TriggerActivate != "")
                                    ProcEvent(NPC[B].TriggerActivate);
                            }
                        }
                    }
                    else if(B != A && NPC[B].Active == true && NPC[B].TimeLeft < NPC[A].TimeLeft - 1)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location) == true)
                            NPC[B].TimeLeft = NPC[A].TimeLeft - 1;
                    }
                }

                int C = 0;

                while(numAct > C)
                {
                    C++;
                    if(NPC[newAct[C]].Type != 57 && NPC[newAct[C]].Type != 46 &&
                       NPC[newAct[C]].Type != 212 && NPC[newAct[C]].Type != 47 &&
                       NPCIsACoin[NPC[newAct[C]].Type] == false)
                    {
                        tempLocation = NPC[newAct[C]].Location;
                        tempLocation.Y = tempLocation.Y - 32;
                        tempLocation.X = tempLocation.X - 32;
                        tempLocation.Width = tempLocation.Width + 64;
                        tempLocation.Height = tempLocation.Height + 64;
                        for(B = 1; B <= numNPCs; B++)
                        {
                            if((NPC[B].Active == false) && B != A && NPC[B].Reset[1] == true && NPC[B].Reset[2] == true)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location) == true)
                                {
                                    numAct = numAct + 1;
                                    SDL_assert_release(numAct <= maxNPCs);
                                    newAct[numAct] = B;
                                    NPC[B].Active = true;
                                    NPC[B].TimeLeft = NPC[newAct[C]].TimeLeft;
                                    NPC[B].JustActivated = 1;
                                    NPC[B].Section = NPC[newAct[C]].Section;
                                    if(B < A)
                                    {
                                        if(NPC[B].TriggerActivate != "")
                                            ProcEvent(NPC[B].TriggerActivate);
                                    }

                                }
                            }
                        }
                    }
                }
            } // NPC[A].Active ...

            if(NPC[A].Type == 208)
            {
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Type != 208 && NPC[B].Effect == 0 && NPC[B].Active == true)
                    {
                        if(NPCNoClipping[NPC[B].Type] == false)
                        {
                            if(NPC[A].Location.Y < NPC[B].Location.Y)
                            {
                                if(NPC[A].Location.Y + NPC[A].Location.Height > NPC[B].Location.Y + NPC[B].Location.Height)
                                {
                                    if(NPC[A].Location.Y < NPC[B].Location.Y)
                                    {
                                        if(NPC[A].Location.Y + NPC[A].Location.Height > NPC[B].Location.Y + NPC[B].Location.Height)
                                        {
                                            NPC[B].Frame = EditorNPCFrame(NPC[B].Type, NPC[B].Direction);
                                            NPC[B].Effect = 208;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } // 208
        } // .JustActivated != 0

        NPC[A].tempBlock = 0;
        if(NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 || NPC[A].Type == 66)
        {
            NPC[A].Active = true;
            NPC[A].TimeLeft = 100;
        }

        if(fEqual(NPC[A].Location.Width, 32.0))
        {
            if(NPC[A].Type != 57 && NPC[A].Type != 84)
            {
                // If .Type = 58 Or .Type = 21 Then
                if(!(NPCIsAnExit[NPC[A].Type] || NPC[A].Type == 8 || NPC[A].Type == 51 ||
                     NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 256 ||
                     NPC[A].Type == 257 || NPC[A].Type == 93 || NPC[A].Type == 245))
                {
                    NPC[A].Location.X = NPC[A].Location.X + 0.015;
                }

                NPC[A].Location.Width = NPC[A].Location.Width - 0.03;
            }
        }
        else if(fEqual(NPC[A].Location.Width, 256.0))
            NPC[A].Location.Width = 255.9;
        else if(fEqual(NPC[A].Location.Width, 128.0))
            NPC[A].Location.Width = 127.9;

        if(NPC[A].Active == true && NPC[A].TimeLeft > 1)
        {

            if(NPC[A].Type == 45 && NPC[A].Special == 1)
            {
                if(NPC[A].Projectile == true)
                    NPC[A].Special2 = 0;
                else
                {
                    NPC[A].Special2 = NPC[A].Special2 + 1;
                    if(NPC[A].Special2 >= 450)
                    {
                        NewEffect(10, NPC[A].Location);
                        NPC[A].Killed = 9;
                    }
                }
            }

            if(NPCIsABlock[NPC[A].Type] || NPCIsAHit1Block[NPC[A].Type] || (NPCCanWalkOn[NPC[A].Type] == true && !(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2)))
            {
                if(
                    (
                        NPC[A].Projectile == false && NPC[A].HoldingPlayer == 0 &&
                        NPC[A].Effect == 0 && !(NPC[A].Type == 45 && NPC[A].Special == 1) &&
                       !((NPC[A].Type == 46 || NPC[A].Type == 212) && NPC[A].Special == 1)
                    ) || NPC[A].Type == 58 || NPC[A].Type == 67 || NPC[A].Type == 68 ||
                    NPC[A].Type == 69 || NPC[A].Type == 70
                )
                {
                    numBlock = numBlock + 1;
                    Block[numBlock] = blankBlock;
                    Block[numBlock].Type = 0;
                    Block[numBlock].Location = NPC[A].Location;
                    Block[numBlock].Location.Y = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.Y + 0.02)));
                    Block[numBlock].IsPlayer = NPC[A].standingOnPlayer;
                    Block[numBlock].standingOnPlayerY = NPC[A].standingOnPlayerY;
                    Block[numBlock].IsReally = A;
                    if(NPC[A].Type == 56)
                        Block[numBlock].Type = 25;
                    if(NPCIsAHit1Block[NPC[A].Type] == true || (NPCCanWalkOn[NPC[A].Type] == true && !NPCIsABlock[NPC[A].Type]))
                        Block[numBlock].Type = 26;
                    if(NPCCanWalkOn[NPC[A].Type] == true && NPCIsAHit1Block[NPC[A].Type] == false && NPCIsABlock[NPC[A].Type] == false)
                        Block[numBlock].noProjClipping = true;
                    if(NPC[A].Type == 26 && Block[numBlock].Location.Height != 32)
                    {
                        Block[numBlock].Location.Y = Block[numBlock].Location.Y - 16;
                        Block[numBlock].Location.Height = Block[numBlock].Location.Height + 16;
                    }
                    Block[numBlock].Location.SpeedX = Block[numBlock].Location.SpeedX + NPC[A].BeltSpeed;
                    Block[numBlock].IsNPC = NPC[A].Type;
                    numTempBlock = numTempBlock + 1;
                    NPC[A].tempBlock = numBlock;
                }
            }
        }
    }
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
        {
            numBlock = numBlock + 1;
            Block[numBlock] = blankBlock;
            Block[numBlock].Type = 25;
            Block[numBlock].Location = Player[A].Location;
            Block[numBlock].Location.X = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.X))) + 1;
            Block[numBlock].Location.Y = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.Y))) + 1;
            Block[numBlock].Location.Width = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.Width))) + 1;
            Block[numBlock].IsPlayer = A;
            numTempBlock = numTempBlock + 1;
        }
    }
    if(numTempBlock > 1)
        qSortBlocksX(numBlock + 1 - numTempBlock, numBlock);
    for(A = numBlock + 1 - numTempBlock; A <= numBlock; A++)
        NPC[Block[A].IsReally].tempBlock = A;

    for(A = 1; A <= numNPCs; A++)
    {
        Physics.NPCGravity = Physics.NPCGravityReal;

        lyrX = NPC[A].Location.X;
        lyrY = NPC[A].Location.Y;

        if(NPC[A].RealSpeedX != 0)
        {
            NPC[A].Location.SpeedX = NPC[A].RealSpeedX;
            NPC[A].RealSpeedX = 0;
        }


        StopHit = 0;
        if(NPC[A].Projectile == false || NPC[A].Type == 50 || NPC[A].Type == 78)
            NPC[A].Multiplier = 0;
        if(NPC[A].Immune > 0)
            NPC[A].Immune = NPC[A].Immune - 1;
        if(NPC[A].Type == 56 && NPC[A].TimeLeft > 1)
            NPC[A].TimeLeft = 100;
        if(NPC[A].JustActivated != 0)
        {
            if(NPC[A].Active == true)
            {
                if(NPC[A].Type == 197)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.Height = 8000;
                    int C = 0;
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
                    {
                        NPC[A].Special2 = Block[C].Location.Y + 4;
                        NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height;
                        NPC[A].Special = 1;
                    }
                }
                else if(NPC[A].Type == 199) // blaarg
                    NPC[A].Location.Y = NPC[A].DefaultLocation.Y + NPC[A].Location.Height + 36;



                else if(NPC[A].Type == 17 || NPC[A].Type == 18 || (NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2) || NPC[A].Type == 42) // Special Start for Jumping Fish and Bullet Bills
                {
                    if(NPC[A].TimeLeft <= 1)
                    {
                        NPC[A].Active = false;
                        NPC[A].TimeLeft = 0;
                    }
                    else if(Maths::iRound(NPC[A].Direction) == -1 && NPC[A].Location.X < Player[NPC[A].JustActivated].Location.X)
                    {
                        NPC[A].Active = false;
                        NPC[A].TimeLeft = 0;
                    }
                    else if(Maths::iRound(NPC[A].Direction) == 1 && NPC[A].Location.X > Player[NPC[A].JustActivated].Location.X)
                    {
                        NPC[A].Active = false;
                        NPC[A].TimeLeft = 0;
                    }
                    else if(NPCIsCheep[NPC[A].Type] && Maths::iRound(NPC[A].Special) == 2)
                    {
                        NPC[A].Location.Y = level[Player[NPC[A].JustActivated].Section].Height - 0.1;
                        NPC[A].Location.SpeedX = (1 + (NPC[A].Location.Y - NPC[A].DefaultLocation.Y) * 0.005) * NPC[A].Direction;
                        NPC[A].Special5 = 1;
                    }
                    else if(!(NPC[A].Type == 42))
                        PlaySound(22);
                }
                else if(NPC[A].Type == 21)
                    NPC[A].Special = 100;
            }
            if(NPC[A].Type == 84 || NPC[A].Type == 181)
                NPC[A].Special = iRand() % 200;
            NPC[A].JustActivated = 0;
            NPC[A].CantHurt = 0;
            NPC[A].CantHurtPlayer = 0;
            if(NPC[A].Type == 21)
                NPC[A].Projectile = false;
            if(NPC[A].Type == 22)
                NPC[A].Projectile = false;
        }
        else if(!(NPCIsCheep[NPC[A].Type] && Maths::iRound(NPC[A].Special) == 2) && !(NPC[A].Type == 12))
        {
            if(GameMenu == false && NPC[A].Location.Y > level[NPC[A].Section].Height + 16)
                NPCHit(A, 9);
        }



        // Normal operations start here



        if(NPCIsAVine[NPC[A].Type])
        {
            // .Location.SpeedX = 0
            // .Location.SpeedY = 0
            if(NPC[A].Type == 213 || NPC[A].Type == 214)
                NPC[A].Frame = BlockFrame[5];
            else if(NPC[A].Type >= 215 && NPC[A].Type <= 220)
                NPC[A].Frame = SpecialFrame[7];

        }
        else if(NPC[A].Active == true && NPC[A].Killed == 0 && NPC[A].Generator == false)
        {


            speedVar = 1;
            if(NPC[A].Slope > 0 && !(NPCIsAShell[NPC[A].Type] || (NPC[A].Type == 45 && NPC[A].Special == 1)))
            {
                if((NPC[A].Location.SpeedX > 0 && BlockSlope[Block[NPC[A].Slope].Type] == -1) ||
                   (NPC[A].Location.SpeedX < 0 && BlockSlope[Block[NPC[A].Slope].Type] == 1))
                {
                    if(!NPCCanWalkOn[NPC[A].Type] || NPCIsABlock[NPC[A].Type] || NPC[A].Type == 78)
                        speedVar = (float)(1 - Block[NPC[A].Slope].Location.Height / Block[NPC[A].Slope].Location.Width * 0.4);
                }
            }
            speedVar = 1;

            if(NPC[A].Projectile == false)
                speedVar = speedVar * NPCSpeedvar[NPC[A].Type];

            // water check

            // Things immune to water's effects
            if(NPC[A].Type == 12 || NPC[A].Type == 17 || NPC[A].Type == 18 || NPC[A].Type == 30 || NPC[A].Type == 38 || NPC[A].Type == 42 || NPC[A].Type == 43 || NPC[A].Type == 44 || NPC[A].Type == 85 || NPC[A].Type == 87 || NPC[A].Type == 108 || NPC[A].Type == 171 || NPC[A].Type == 292 || NPC[A].Type == 197 || NPC[A].Type == 202 || NPC[A].Type == 210 || NPC[A].Type == 225 || NPC[A].Type == 226 || NPC[A].Type == 227 || NPC[A].Type == 47 || NPC[A].Type == 284 || NPC[A].Type == 179 || NPC[A].Type == 270 || NPC[A].Type == 269 || (NPCIsACoin[NPC[A].Type] && NPC[A].Special == 0) || NPC[A].Type == 266 || NPC[A].Type == 259 || NPC[A].Type == 260)
            {
                NPC[A].Wet = 0;
                NPC[A].Quicksand = 0;
            }
            else
            {
                if(NPC[A].Wet > 0)
                    NPC[A].Wet = NPC[A].Wet - 1;
                if(NPC[A].Quicksand > 0)
                    NPC[A].Quicksand = NPC[A].Quicksand - 1;
                if(UnderWater[NPC[A].Section] == true)
                    NPC[A].Wet = 2;
                for(B = 1; B <= numWater; B++)
                {
                    if(Water[B].Hidden == false)
                    {
                        if(CheckCollision(NPC[A].Location, Water[B].Location) == true)
                        {
                            if(NPC[A].Wet == 0 && NPCIsACoin[NPC[A].Type] == false)
                            {
                                if(NPC[A].Location.SpeedY >= 1)
                                {
                                    tempLocation.Width = 32;
                                    tempLocation.Height = 32;
                                    tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - tempLocation.Height;
                                    NewEffect(114, tempLocation);
                                }
                                if(!(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 1) && NPC[A].Type != 34 && NPC[A].Type != 13)
                                {
                                    if(NPC[A].Location.SpeedY > 0.5)
                                        NPC[A].Location.SpeedY = 0.5;
                                    if(NPC[A].Location.SpeedY < -0.5)
                                        NPC[A].Location.SpeedY = -0.5;
                                }
                                else
                                {
                                    if(NPC[A].Location.SpeedY > 2)
                                        NPC[A].Location.SpeedY = 2;
                                    if(NPC[A].Location.SpeedY < -2)
                                        NPC[A].Location.SpeedY = -2;
                                }
                                if(NPC[A].Type == 104)
                                    NPC[A].Special = NPC[A].Location.SpeedY;
                            }
                            if(Water[B].Quicksand == true)
                                NPC[A].Quicksand = 2;
                            NPC[A].Wet = 2;
                        }
                    }
                }
            }
            if(NPC[A].Wet == 1 && NPC[A].Location.SpeedY < -1)
            {
                tempLocation.Width = 32;
                tempLocation.Height = 32;
                tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - tempLocation.Height;
                NewEffect(114, tempLocation);
            }


            if(NPC[A].Wet > 0)
            {
                if(NPC[A].Type == 263)
                {
                    NPC[A].Projectile = true;
                    Physics.NPCGravity = -Physics.NPCGravityReal * 0.2;
                }
                else
                    Physics.NPCGravity = Physics.NPCGravityReal * 0.2;
                if(NPC[A].Type == 195 && NPC[A].Special4 == 1)
                    NPC[A].Special5 = 0;
                else if(!NPCIsCheep[NPC[A].Type] && NPC[A].Type != 190 && NPC[A].Type != 205 && NPC[A].Type != 206 && NPC[A].Type != 207)
                    speedVar = (float)(speedVar * 0.5);
                else if(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2 && NPC[A].Location.SpeedY > 0)
                    speedVar = (float)(speedVar * 0.5);
                if(NPC[A].Location.SpeedY >= 3) // Terminal Velocity in water
                    NPC[A].Location.SpeedY = 3;
                if(NPC[A].Location.SpeedY < -3)
                    NPC[A].Location.SpeedY = -3;
            }
            else if(!(NPC[A].Type != 190 && NPCIsCheep[NPC[A].Type] == false))
            {
                NPC[A].WallDeath = NPC[A].WallDeath + 2;
                if(NPC[A].WallDeath >= 10)
                    NPC[A].WallDeath = 10;
            }
            if(NPC[A].Quicksand > 0 && NPCNoClipping[NPC[A].Type] == false)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 1;
                if(NPC[A].Location.SpeedY < -1)
                    NPC[A].Location.SpeedY = -1;
                else if(NPC[A].Location.SpeedY > 0.5)
                    NPC[A].Location.SpeedY = 0.5;
                speedVar = (float)(speedVar * 0.3);
            }




            if(NPC[A].Type == 147)
            {
                B = iRand() % 9;
                NPC[A].Type = 139 + B;
                if(NPC[A].Type == 147)
                    NPC[A].Type = 92;
                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0;
                NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height / 2.0;
            }
            if(NPC[A].Text != "")
            {
                NPC[A].Chat = false;
                tempLocation = NPC[A].Location;
                tempLocation.Y = tempLocation.Y - 25;
                tempLocation.Height = tempLocation.Height + 50;
                tempLocation.X = tempLocation.X - 25;
                tempLocation.Width = tempLocation.Width + 50;
                for(B = 1; B <= numPlayers; B++)
                {
                    if(CheckCollision(tempLocation, Player[B].Location) == true)
                        NPC[A].Chat = true;
                }
            }
            oldDirection = NPC[A].Direction;
            UNUSED(oldDirection);
            if(NPC[A].Type == 17 || NPC[A].Type == 18)
            {
                if(NPC[A].CantHurt > 0)
                {
                    NPC[A].CantHurt = 10000;
                    if(NPC[A].Type == 18)
                        NPC[A].Location.SpeedX = 4 * NPC[A].Direction;
                }
                if(NPC[A].TimeLeft > 3 && BattleMode == false)
                    NPC[A].TimeLeft = 3;
            }

            if(NPC[A].Type == 267 || NPC[A].Type == 268 || NPC[A].Type == 280 || NPC[A].Type == 281) // koopalings
            {
                if(NPC[A].TimeLeft > 1)
                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;

            }

            CheckSectionNPC(A);

            if((NPC[A].Type == 86 || NPC[A].Type == 259 || NPC[A].Type == 260) && NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;

            if(!(NPC[A].Type == 13 || (NPCIsCheep[NPC[A].Type] && fEqual(NPC[A].Special, 2)) ||
                 NPC[A].Type == 50 || NPC[A].Type == 56 || NPC[A].Type == 60 || NPC[A].Type == 62 ||
                 NPC[A].Type == 64 || NPC[A].Type == 66 || NPC[A].Type == 86 || NPCIsYoshi[NPC[A].Type]) &&
                 NPC[A].HoldingPlayer == 0)
            {
                int C = 0;
                for(B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].Section == NPC[A].Section)
                        C = 1;
                }
                if(C == 0 && NPC[A].TimeLeft > 1)
                    NPC[A].TimeLeft = 0;
            }
            if((NPC[A].Type == 225 || NPC[A].Type == 226 || NPC[A].Type == 227) && NPC[A].TimeLeft > 10)
                NPC[A].TimeLeft = 100;
            if(NPC[A].TimeLeft > 10 && NoTurnBack[NPC[A].Section] == true)
                NPC[A].TurnBackWipe = true;
            if(NPC[A].TimeLeft < 1)
                Deactivate(A);
            NPC[A].TimeLeft = NPC[A].TimeLeft - 1;
            if(NPC[A].Effect == 0)
            {



                // this code is for NPCs that are being held by a player

                if(NPC[A].HoldingPlayer > 0) // NPC is held
                {
                    NPC[A].standingOnPlayer = 0;
                    if(NPC[A].Type == 56)
                    {
                        Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                        NPC[A].HoldingPlayer = 0;
                    }
                    if(Player[NPC[A].HoldingPlayer].HoldingNPC == A && Player[NPC[A].HoldingPlayer].TimeToLive == 0 && Player[NPC[A].HoldingPlayer].Dead == false) // Player and NPC are on the same page
                    {
                        NPC[A].Multiplier = 0;
                        if(NPC[A].Type == 159)
                        {
                            Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                            NPC[A].HoldingPlayer = 0;
                            NPC[A].Killed = 9;
                            NewEffect(10, NPC[A].Location);
                        }
                        if(NPCIsYoshi[NPC[A].Type] == true)
                        {
                            NPC[A].Special = NPC[A].Type;
                            NPC[A].Type = 96;
                        }
                        if(NPC[A].Type == 91)
                        {
                            if(NPC[A].Special == 0.0)
                                NPC[A].Special = 147;
                            NPC[A].Generator = false;
                            NPC[A].Frame = 0;
                            NPC[A].Type = NPC[A].Special;
                            NPC[A].Special = 0;
                            if(NPCIsYoshi[NPC[A].Type])
                            {
                                NPC[A].Special = NPC[A].Type;
                                NPC[A].Type = 96;
                            }
                            if(!(NPC[A].Type == 21 || NPC[A].Type == 22 || NPC[A].Type == 26 || NPC[A].Type == 31 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 239 || NPC[A].Type == 193 || NPC[A].Type == 191 || NPC[A].Type == 35 || NPC[A].Type == 193 || NPC[A].Type == 49 || NPCIsAnExit[NPC[A].Type]))
                                NPC[A].DefaultType = 0;
                            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                            if(NPC[A].Type == 147)
                            {
                                B = iRand() % 9;
                                NPC[A].Type = 139 + B;
                                if(NPC[A].Type == 147)
                                    NPC[A].Type = 92;
                                NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                                NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0;
                                NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                                NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                                NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height / 2.0;
                            }
                        }
                        if(NPC[A].Type == 45)
                            NPC[A].Special = 1;
                        if(NPC[A].Type == 133)
                        {
                            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0;
                            NPC[A].Type = 138;
                            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                            NPC[A].Location.Width = NPCWidth[NPC[A].Type];
                            NPC[A].Location.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                            NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height / 2.0;
                        }
                        NPC[A].TimeLeft = 100;
                        NPC[A].BeltSpeed = 0;
                        if(NPC[A].Type == (NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2))
                            NPC[A].Special5 = 0;
                        NPC[A].Direction = Player[NPC[A].HoldingPlayer].Direction; // Face the player
                        NPC[A].Location.SpeedY = Player[NPC[A].HoldingPlayer].Location.SpeedY;
                        NPC[A].Location.SpeedX = 0;
                        if(!(NPC[A].Type == 195 || NPC[A].Type == 22 || NPC[A].Type == 26 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 239 || NPC[A].Type == 193 || NPC[A].Type == 35 || NPC[A].Type == 191 || NPC[A].Type == 193 || NPC[A].Type == 49 || NPC[A].Type == 134 || (NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 31 || NPC[A].Type == 240 || NPC[A].Type == 278 || NPC[A].Type == 279 || NPC[A].Type == 292))
                        {
                            for(B = 1; B <= numNPCs; B++)
                            {
                                if(B != A && NPC[B].Active == true &&
                                    (NPC[B].HoldingPlayer == 0 || (BattleMode == true && NPC[B].HoldingPlayer != NPC[A].HoldingPlayer)) &&
                                    !NPCIsABonus[NPC[B].Type] &&
                                    (NPC[B].Type != 13  || (BattleMode == true && NPC[B].CantHurtPlayer != NPC[A].HoldingPlayer)) &&
                                    (NPC[B].Type != 265 || (BattleMode == true && NPC[B].CantHurtPlayer != NPC[A].HoldingPlayer)) &&
                                    NPC[B].Type != 21 && NPC[B].Type != 22 &&  NPC[B].Type != 26 && NPC[B].Type != 31 &&
                                    NPC[B].Type != 32 && NPC[B].Type != 238 && NPC[B].Type != 239 && NPC[B].Type != 191 &&
                                    NPC[B].Type != 35 && !(NPC[B].Type == 193 && NPC[A].Type == 193) &&
                                    !(NPC[B].Type == 37) && !(NPC[B].Type == 180) && !(NPC[B].Type == 38) &&
                                    !(NPC[B].Type == 39) && !(NPC[B].Type == 45 && NPC[B].Special == 0.0) &&
                                    !(NPC[B].Type == 91) && !(NPC[B].Type == 159) && !(NPC[B].Type == 195) &&
                                    !(NPC[B].Type == 30 && NPC[B].Projectile == true) && NPC[B].Type != 241 && NPC[B].Type != 263 && NPC[B].Type != 291)
                                {
                                    if(NPC[A].CantHurtPlayer != NPC[B].CantHurtPlayer && NPC[B].Killed == 0 && (Player[NPC[A].HoldingPlayer].StandingOnNPC != B) && NPC[B].Inert == false)
                                    {
                                        if(CheckCollision(NPC[A].Location, NPC[B].Location) == true)
                                        {
                                            NPCHit(B, 3, A);
                                            if(NPC[B].Killed > 0)
                                            {
                                                NPC[B].Location.SpeedX = Physics.NPCShellSpeed * 0.5 * -Player[NPC[A].HoldingPlayer].Direction;
                                                NPCHit(A, 5, B);
                                            }
                                            if(NPC[A].Killed > 0)
                                                NPC[A].Location.SpeedX = Physics.NPCShellSpeed * 0.5 * Player[NPC[A].HoldingPlayer].Direction;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        if(NPC[A].Type == 237 || NPC[A].Type == 263) // Yoshi Ice
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
                        }
                    }
                    else // Player and NPC are not on the same page
                    {
                        Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                        if(NPC[A].Type == 272)
                            NPC[A].Projectile = true;
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                        if(NPC[A].Type == 29)
                        {
                            NPC[A].Killed = 3;
                            NPC[A].Direction = -NPC[A].Direction;
                        }
                        if(NPC[A].Type == 17)
                        {
                            PlaySound(22);
                            NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                            NPC[A].Projectile = true;
                            NPC[A].CantHurt = 1000;
                            NPC[A].CantHurtPlayer = NPC[A].HoldingPlayer;
                        }
                        NPC[A].HoldingPlayer = 0;
                    }
                }
                else // NPC is not held
                {
                    if(NPC[A].CantHurt <= 0)
                        NPC[A].CantHurtPlayer = 0;
                    tempHit = 0;
                    tempBlockHit[1] = 0;
                    tempBlockHit[2] = 0;
                    winningBlock = 0;

                    if((LevelWrap[NPC[A].Section] || LevelVWrap[NPC[A].Section]) && NPC[A].Type != 30 && NPC[A].Type != 108) // Level wraparound
                    {
                        if(LevelWrap[NPC[A].Section])
                        {
                            if(NPC[A].Location.X + NPC[A].Location.Width < level[NPC[A].Section].X)
                                NPC[A].Location.X = level[NPC[A].Section].Width - 1;
                            else if(NPC[A].Location.X > level[NPC[A].Section].Width)
                                NPC[A].Location.X = level[NPC[A].Section].X - NPC[A].Location.Width + 1;
                        }

                        if(LevelVWrap[NPC[A].Section])
                        {
                            if(NPC[A].Location.Y + NPC[A].Location.Height < level[NPC[A].Section].Y)
                                NPC[A].Location.Y = level[NPC[A].Section].Height - 1;
                            else if(NPC[A].Location.Y > level[NPC[A].Section].Height)
                                NPC[A].Location.Y = level[NPC[A].Section].Y - NPC[A].Location.Height + 1;
                        }
                    }

                    if(NoTurnBack[NPC[A].Section] && NPC[A].Location.X < level[NPC[A].Section].X - NPC[A].Location.Width - 32)
                        NPCHit(A, 9);

                    if(NPC[A].CantHurt > 0)
                    {
                        if(!(NPC[A].Type == 21))
                            NPC[A].CantHurt = NPC[A].CantHurt - 1;
                    }
                    else
                        NPC[A].CantHurtPlayer = 0;
                    if(NPC[A].Projectile == true)
                    {
                        if(NPC[A].CantHurtPlayer != 0)
                            NPC[A].BattleOwner = NPC[A].CantHurtPlayer;
                    }
                    else
                        NPC[A].BattleOwner = 0;
                    if(NPCIsAShell[NPC[A].Type])
                    {
                        NPC[A].Special4 = NPC[A].Special4 - 1;
                        if(NPC[A].Special4 < 0)
                            NPC[A].Special4 = 0;
                    }
                    if(NPC[A].TurnAround == true)
                    {
                        if((NPC[A].Type == 267 || NPC[A].Type == 280) && NPC[A].Special == 0) // larry koopa
                        {
                            if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                            {
                                if(NPC[A].Special2 < 0)
                                    NPC[A].Special3 = NPC[A].Special3 + 30;
                                NPC[A].Special2 = -1;
                            }
                            else
                            {
                                if(NPC[A].Special2 > 0)
                                    NPC[A].Special3 = NPC[A].Special3 + 30;
                                NPC[A].Special2 = 1;
                            }

                        }

                        if(NPC[A].Type == 265)
                            NPCHit(A, 3, A);
                        if(NPCIsAShell[NPC[A].Type] == true && NPC[A].Location.SpeedX != 0 && NPC[A].Special4 == 0)
                        {
                            NPC[A].Special4 = 5;
                            tempLocation.Height = 0;
                            tempLocation.Width = 0;
                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 16;
                            tempLocation.X = NPC[A].Location.X - 16;
                            if(NPC[A].Direction == 1)
                                tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width - 16;
                            NewEffect(132, tempLocation);
                        }
                        if(NPC[A].Type == 179)
                            NPC[A].Special2 = -NPC[A].Special2;
                        if(!NPCIsAShell[NPC[A].Type] && NPC[A].Type != 13 && NPC[A].Type != 78 &&
                           NPC[A].Type != 17 && NPC[A].Type != 86 && !NPCIsABot[NPC[A].Type] &&
                           NPC[A].Type != 40 && NPC[A].Type != 133 && !NPCIsVeggie[NPC[A].Type] &&
                           NPC[A].Type != 160 && NPC[A].Type != 206 && NPC[A].Type != 205 &&
                           NPC[A].Type != 207 && NPC[A].Type != 265 && NPC[A].Type != 266)  // Don't turn around if a shell or a fireball
                        {
                            NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                            if(NPC[A].tempBlock > 0)
                                Block[NPC[A].tempBlock].Location.SpeedX = -Block[NPC[A].tempBlock].Location.SpeedX;
                        }
                        NPC[A].TurnAround = false;
                    }

                    if(NPC[A].Type == 179) // play saw sound
                        PlaySound(74);
                    // NPC Movement Code



                    // Default Movement Code
                    if((NPCDefaultMovement[NPC[A].Type] == true || (NPCIsCheep[NPC[A].Type] == true && NPC[A].Special != 2)) && !((NPC[A].Type == 55 || NPC[A].Type == 119) && NPC[A].Special > 0) && NPC[A].Type != 91)
                    {
                        if(NPC[A].Direction == 0)
                        {
                            if(iRand() % 2 == 0)
                                NPC[A].Direction = -1;
                            else
                                NPC[A].Direction = 1;
                        }
                        if(NPCCanWalkOn[NPC[A].Type] == true)
                        {
                            if(NPC[A].Location.SpeedX < Physics.NPCWalkingOnSpeed && NPC[A].Location.SpeedX > -Physics.NPCWalkingOnSpeed)
                            {
                                if(NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * NPC[A].Direction;
                            }
                            if(NPC[A].Location.SpeedX > Physics.NPCWalkingOnSpeed)
                            {
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                                if(NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.1;
                            }
                            else if(NPC[A].Location.SpeedX < -Physics.NPCWalkingOnSpeed)
                            {
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                                if(NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1;
                            }
                        }
                        else if(NPC[A].Type == 125)
                        {
                            if(NPC[A].Location.SpeedX < 2 && NPC[A].Location.SpeedX > -2)
                            {
                                if(NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
                            }
                            if(NPC[A].Location.SpeedX > 2)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                            else if(NPC[A].Location.SpeedX < -2)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                        }
                        else if(!(NPC[A].Type >= 117 && NPC[A].Type <= 120 && NPC[A].Projectile == true))
                        {
                            if(NPC[A].Location.SpeedX < Physics.NPCWalkingSpeed && NPC[A].Location.SpeedX > -Physics.NPCWalkingSpeed)
                            {
                                if(NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;
                            }
                            if(NPC[A].Location.SpeedX > Physics.NPCWalkingSpeed)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                            else if(NPC[A].Location.SpeedX < -Physics.NPCWalkingSpeed)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                        }
                    }
                    else if(NPC[A].Type == 203)
                    {
                        if(NPC[A].Location.SpeedX > -2 && NPC[A].Location.SpeedX < 2)
                            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
                    }
                    else if(NPC[A].Type == 204)
                    {
                        if(NPC[A].Location.SpeedX > -2.5 && NPC[A].Location.SpeedX < 2.5)
                            NPC[A].Location.SpeedX = 2.5 * NPC[A].Direction;

                        // Slow things down that shouldnt move
                    }
                    else if(NPC[A].Type == 21 || NPC[A].Type == 22 || NPC[A].Type == 25 || NPC[A].Type == 26 || NPC[A].Type == 31 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 239 || NPC[A].Type == 35 || NPC[A].Type == 191 || NPC[A].Type == 193 || (NPC[A].Type == 40 && NPC[A].Projectile == true) || NPC[A].Type == 49 || NPC[A].Type == 58 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70 || (NPCIsVeggie[NPC[A].Type] && NPC[A].Projectile == false) || (NPC[A].Type == 29 && NPC[A].Projectile == true) || (NPC[A].Projectile == true && (NPC[A].Type == 54 && NPC[A].Type == 15)) || NPC[A].Type == 75 || NPC[A].Type == 84 || NPC[A].Type == 181 || NPC[A].Type == 94 || NPC[A].Type == 198 || NPC[A].Type == 96 || NPC[A].Type == 134 || NPC[A].Type == 137 || NPC[A].Type == 101 || NPC[A].Type == 102 || (NPCIsYoshi[NPC[A].Type] && NPC[A].Special == 0) || (NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 166 || (NPC[A].Type == 39 && NPC[A].Projectile == true) || NPC[A].Type == 170 || NPC[A].Type == 169 || NPC[A].Type == 183 || NPC[A].Type == 188 || NPC[A].Type == 97 || NPC[A].Type == 196 || NPC[A].Type == 182 || NPC[A].Type == 240 || NPC[A].Type == 241 || NPC[A].Type == 249 || NPC[A].Type == 250 || NPC[A].Type == 254 || NPC[A].Type == 255 || NPC[A].Type == 278 || NPC[A].Type == 279 || NPC[A].Type == 277 || NPC[A].Type == 264 || NPC[A].Type == 288 || NPC[A].Type == 275)
                    {
                        if(NPC[A].Location.SpeedX > 0)
                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                        else if(NPC[A].Location.SpeedX < 0)
                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                        if(NPC[A].Location.SpeedX >= -0.05 && NPC[A].Location.SpeedX <= 0.05)
                            NPC[A].Location.SpeedX = 0;
                        if(NPC[A].Location.SpeedY >= -Physics.NPCGravity && NPC[A].Location.SpeedY <= Physics.NPCGravity)
                        {
                            if(NPC[A].Location.SpeedX > 0)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.3;
                            else if(NPC[A].Location.SpeedX < 0)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.3;
                            if(NPC[A].Location.SpeedX >= -0.3 && NPC[A].Location.SpeedX <= 0.3)
                                NPC[A].Location.SpeedX = 0;
                        }
                    }
                    else if(NPC[A].Type == 78)
                    {
                        NPC[A].Projectile = true;
                        NPC[A].Direction = NPC[A].DefaultDirection;
                        NPC[A].Location.SpeedX = 1 * NPC[A].DefaultDirection;
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!(Player[B].Effect == 0 || Player[B].Effect == 3))
                            {
                                NPC[A].Location.SpeedX = 0;
                                NPC[A].Location.SpeedY = 0;
                            }
                        }
                    }
                    // Mushroom Movement Code
                    else if(NPC[A].Type == 9 || NPC[A].Type == 273 || NPC[A].Type == 90 ||
                            NPC[A].Type == 153 || NPC[A].Type == 184 || NPC[A].Type == 185 ||
                            NPC[A].Type == 186 || NPC[A].Type == 187 || NPC[A].Type == 163 ||
                            NPC[A].Type == 164)
                    {
                        if(NPC[A].Direction == 0.0f) // Move toward the closest player
                        {
                            double C = 0;
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                                {
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                        NPC[A].Direction = -Player[B].Direction;
                                    }
                                }
                            }
                        }

                        if(NPC[A].Location.SpeedX < Physics.NPCMushroomSpeed && NPC[A].Location.SpeedX > -Physics.NPCMushroomSpeed)
                        {
                            if(NPC[A].Projectile == false)
                                NPC[A].Location.SpeedX = Physics.NPCMushroomSpeed * NPC[A].Direction;
                        }
                        if(NPC[A].Location.SpeedX > Physics.NPCMushroomSpeed)
                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                        else if(NPC[A].Location.SpeedX < -Physics.NPCMushroomSpeed)
                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                    }
                    else if(NPC[A].Type == 194)
                    {
                        NPC[A].Projectile = true;

                        double C = 0;
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
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
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.1 * double(NPC[A].Direction);
                        if(NPC[A].Location.SpeedX < -4)
                            NPC[A].Location.SpeedX = -4;
                        if(NPC[A].Location.SpeedX > 4)
                            NPC[A].Location.SpeedX = 4;
                        // Yoshi Fireball
                    }
                    else if(NPC[A].Type == 108)
                    {
                        NPC[A].Projectile = true;
                        if(NPC[A].Location.SpeedX == 0)
                            NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                        // bully
                    }
                    else if(NPC[A].Type == 168)
                    {
                        if(!NPC[A].Projectile && NPC[A].Special2 == 0.0)
                        {
                            double C = 0;
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
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

                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05 * double(NPC[A].Direction);
                            if(NPC[A].Location.SpeedX >= 3)
                                NPC[A].Location.SpeedX = 3;
                            if(NPC[A].Location.SpeedX <= -3)
                                NPC[A].Location.SpeedX = -3;
                        }
                        else
                        {
                            if(NPC[A].Location.SpeedX > 0.1)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.075;
                            else if(NPC[A].Location.SpeedX < -0.1)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.075;
                            if(NPC[A].Location.SpeedX >= -0.1 && NPC[A].Location.SpeedX <= 0.1)
                                NPC[A].Special2 = 0;
                        }
                    }
                    else if(NPC[A].Type == 190)
                    {
                        if(NPC[A].Special == 1)
                            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;

                        // Big Koopa Movement Code
                    }
                    else if(NPC[A].Type == 15)
                    {
                        if(NPC[A].Location.SpeedX < 0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                        if(NPC[A].Special == 0 || NPC[A].Special == 3)
                        {
                            if(NPC[A].Location.SpeedX < 3.5 && NPC[A].Location.SpeedX > -3.5)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + (0.1 * NPC[A].Direction);
                            if(NPC[A].Location.SpeedX > 3.5)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                            else if(NPC[A].Location.SpeedX < -3.5)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                            if(NPC[A].Special == 3)
                                NPC[A].Location.SpeedY = -6;
                        }
                        else if(NPC[A].Special == 2)
                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + (0.2 * NPC[A].Direction);
                        else if(NPC[A].Special == 3)
                            NPC[A].Location.SpeedY = -6;
                        else
                        {
                            if(NPC[A].Location.SpeedX > 0)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                            else if(NPC[A].Location.SpeedX < 0)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                            if(NPC[A].Location.SpeedX > -0.5 && NPC[A].Location.SpeedX < 0.5)
                                NPC[A].Location.SpeedX = 0.0001 * NPC[A].Direction;
                        }
                        // spiney eggs
                    }
                    else if(NPC[A].Type == 48)
                    {
                        if(NPC[A].CantHurt > 0)
                        {
                            NPC[A].Projectile = true;
                            NPC[A].CantHurt = 100;
                        }
                        else
                        {
                            double C = 0;
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
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
                            if(NPC[A].Direction == 1 && NPC[A].Location.SpeedX < 4)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.04;
                            if(NPC[A].Direction == -1 && NPC[A].Location.SpeedX > -4)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.04;
                        }
                    }
                    else if(NPC[A].Type == 17 || NPC[A].Type == 18)
                    {
                        if(NPC[A].CantHurt < 1000)
                            NPC[A].Location.SpeedX = 4 * NPC[A].Direction;
                    }
                    else if(NPC[A].Type == 42)
                        NPC[A].Location.SpeedX = 2 * double(NPC[A].Direction);
                    // yoshi
                    if(NPCIsYoshi[NPC[A].Type])
                    {
                        if(NPC[A].Special == 0.0)
                        {
                            if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                            {
                                if(NPC[A].Wet == 0)
                                    NPC[A].Location.SpeedY = -2.1;
                                else
                                    NPC[A].Location.SpeedY = -1.1;
                            }
                        }
                        else
                        {
                            if(NPC[A].Location.SpeedX < 3 && NPC[A].Location.SpeedX > -3)
                            {
                                if(NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = 3 * NPC[A].Direction;
                            }
                        }
                    }
                    if(!(NPC[A].Type == 39) && !(NPC[A].Type == 46) && !(NPC[A].Type == 212) && !(NPC[A].Type == 56) && !(NPC[A].Type == 57) && !(NPC[A].Type == 60) && !(NPC[A].Type == 62) && !(NPC[A].Type == 64) && !(NPC[A].Type == 66) && !(NPC[A].Type == 84) && !(NPC[A].Type == 181) && !(NPC[A].Type == 85) && !(NPC[A].Type == 22) && !(NPC[A].Type == 49) && !(NPC[A].Type == 50) && !(NPC[A].Type >= 104 && NPC[A].Type <= 106))
                    {
                        if(NPC[A].Location.SpeedX < 0) // Find the NPCs direction
                            NPC[A].Direction = -1;
                        else if(NPC[A].Location.SpeedX > 0)
                            NPC[A].Direction = 1;
                    }
                    // Reset Speed when no longer a projectile
                    // If Not (NPCIsAShell(.Type) Or .Type = 8 Or .Type = 93 Or .Type = 74 Or .Type = 51 Or .Type = 52 Or .Type = 12 Or .Type = 14 Or .Type = 13 Or .Type = 15 Or NPCIsABonus(.Type) Or .Type = 17 Or .Type = 18 Or .Type = 21 Or .Type = 22 Or .Type = 25 Or .Type = 26 Or .Type = 29 Or .Type = 30 Or .Type = 31 Or .Type = 32 Or .Type = 35 Or .Type = 37 Or .Type = 38 Or .Type = 39 Or .Type = 40 Or .Type = 42 Or .Type = 43 Or .Type = 44 Or .Type = 45 Or .Type = 46 Or .Type = 47 Or .Type = 48 Or .Type = 76 Or .Type = 49 Or .Type = 54 Or .Type = 56 Or .Type = 57 Or .Type = 58 Or .Type = 60 Or .Type = 62 Or .Type = 64 Or .Type = 66 Or .Type = 67 Or .Type = 68 Or .Type = 69 Or .Type = 70 Or .Type = 78 Or .Type = 84 Or .Type = 85 Or .Type = 87 Or (.Type = 55 And .Special > 0) Or (.Type >= 79 And .Type <= 83) Or .Type = 86 Or .Type = 92 Or .Type = 94 Or NPCIsYoshi(.Type) Or .Type = 96 Or .Type = 101 Or .Type = 102) And .Projectile = False Then
                    if((NPCDefaultMovement[NPC[A].Type] == true || (NPCIsCheep[NPC[A].Type] == true && NPC[A].Special != 2)) && !((NPC[A].Type == 55 || NPC[A].Type == 119) && NPC[A].Special > 0) && NPC[A].Projectile == false)
                    {
                        if(NPCCanWalkOn[NPC[A].Type] == false)
                        {
                            if(NPCCanWalkOn[NPC[A].Type] == true)
                                NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * NPC[A].Direction;
                            else if(NPC[A].Type == 125)
                                NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
                            else
                                NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;


                            if((NPCIsCheep[NPC[A].Type] && NPC[A].Special != 1) && NPC[A].Projectile == false)
                            {
                                if(NPC[A].Wet == 0)
                                {
                                    if(NPC[A].Special5 >= 0)
                                        NPC[A].Special2 = NPC[A].Special2 - 1;
                                }
                                else
                                {
                                    NPC[A].Special2 = 6;
                                    NPC[A].Special3 = NPC[A].Location.SpeedX;
                                }
                                if(NPC[A].Special2 <= 0)
                                {
                                    NPC[A].Special3 = NPC[A].Special3 * 0.99;
                                    if(NPC[A].Special3 > -0.1 && NPC[A].Special3 < 0.1)
                                        NPC[A].Special3 = 0;
                                    NPC[A].Location.SpeedX = NPC[A].Special3;
                                }
                            }

                            if(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 1 && NPC[A].Projectile == false)
                                NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * 2 * NPC[A].Direction;


                        }
                    }
                    if(NPC[A].Type == 135 && NPC[A].Projectile == false && NPC[A].Special2 == 1)
                        NPC[A].Location.SpeedX = 0;



                    // NPC Gravity
                    if(NPCNoGravity[NPC[A].Type] == false)
                    {

                        if(NPC[A].Type == 13 || NPC[A].Type == 265)
                        {
                            NPC[A].CantHurt = 100;
                            if(NPC[A].Special < 2)
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 1.5;
                            else if(NPC[A].Special == 3)
                            {
                                // peach fireball changes
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 0.9;
                                if(NPC[A].Location.SpeedX > 3)
                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.04;
                                else if(NPC[A].Location.SpeedX < -3)
                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.04;
                            }
                            else if(NPC[A].Special == 4)
                            {

                                // toad fireball changes
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 1.3;
                                if(NPC[A].Location.SpeedX < 8 && NPC[A].Location.SpeedX > 0)
                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.03;
                                else if(NPC[A].Location.SpeedX > -8 && NPC[A].Location.SpeedX < 0)
                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.03;
                            }
                            else if(NPC[A].Special == 5) // link fireballs float
                            {
                            }
                            else
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 1.3;

                        }
                        else if(NPC[A].Type == 17 || NPC[A].Type == 18)
                            NPC[A].Location.SpeedY = 0;

                        else if((NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2) && NPC[A].Projectile == false)
                        {
                            if(Maths::iRound(NPC[A].Special5) == 1)
                            {
                                if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y)
                                    NPC[A].Location.SpeedY = -4 - (NPC[A].Location.Y - NPC[A].DefaultLocation.Y) * 0.02;
                                else
                                    NPC[A].Special5 = 0;
                            }
                            else
                            {
                                // If .Location.SpeedY < 2 + (.Location.Y - .DefaultLocation.Y) * 0.02 Then
                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 0.4;
                                // End If
                            }
                        }
                        else if(NPC[A].Type != 225 && NPC[A].Type != 226 && NPC[A].Type != 227 && !(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2) && NPC[A].Type != 210 && NPC[A].Type != 211 && NPC[A].Type != 133 && NPC[A].Type != 97 && NPC[A].Type != 196 && NPC[A].Type != 87 && NPC[A].Type != 8 && NPC[A].Type != 245 && NPC[A].Type != 246 && NPC[A].Type != 93 && NPC[A].Type != 74 && NPC[A].Type != 256 && NPC[A].Type != 257 && !NPCIsAParaTroopa[NPC[A].Type] && NPC[A].Type != 51 && NPC[A].Type != 52 && NPC[A].Type != 34 && NPC[A].Type != 37 && NPC[A].Type != 180 && NPC[A].Type != 38 && NPC[A].Type != 42 && NPC[A].Type != 43 && NPC[A].Type != 44 && !(NPC[A].Type == 47) && !(NPC[A].Type == 56) && !(NPC[A].Type == 57) && !(NPC[A].Type == 60) && !(NPC[A].Type == 62) && !(NPC[A].Type == 64) && !(NPC[A].Type == 66) && !(NPC[A].Type == 85) && !(NPCIsACoin[NPC[A].Type] && NPC[A].Special == 0) && !(NPC[A].Type == 105) && !(NPC[A].Type == 106) && !(NPC[A].Type == 108) && !(NPC[A].Type == 197) && !(NPC[A].Type == 199) && !(NPC[A].Type == 203) && !(NPC[A].Type == 204) && ((!(NPC[A].Type == 205) && !(NPC[A].Type == 206) && !(NPC[A].Type == 207))) && !(NPC[A].Type == 209) && !(NPC[A].Type == 91) && NPC[A].Type != 269 && NPC[A].Type != 270 && NPC[A].Type != 255)
                        {
                            if(NPC[A].Type != 271 && NPC[A].Type != 272 && NPC[A].Type != 276 && NPC[A].Type != 282 && NPC[A].Type != 283 && NPC[A].Type != 284 && NPC[A].Type != 289 && NPC[A].Type != 290 && NPC[A].Type != 291 && NPC[A].Type != 292) // no gravity
                            {
                                if(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 4 && NPC[A].Projectile == false)
                                    NPC[A].Location.SpeedX = 0;
                                if(NPC[A].Wet == 2 && (NPC[A].Type == 190))
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - Physics.NPCGravity * 0.5;
                                else if(NPC[A].Wet == 2 && NPCIsCheep[NPC[A].Type] && NPC[A].Special != 2 && NPC[A].Projectile == false) // Fish cheep
                                {
                                    if((NPC[A].Location.X < NPC[A].DefaultLocation.X - 100 && NPC[A].Direction == -1) || (NPC[A].Location.X > NPC[A].DefaultLocation.X + 100 && NPC[A].Direction == 1))
                                    {
                                        if(NPC[A].Special == 3)
                                            NPC[A].TurnAround = true;
                                    }
                                    if(NPC[A].Special == 4)
                                    {


                                        if(NPC[A].Location.SpeedY == 0)
                                            NPC[A].Special4 = 1;
                                        if(NPC[A].Location.SpeedY == 0.01)
                                            NPC[A].Special4 = 0;

                                        NPC[A].Location.SpeedX = 0;
                                        if(NPC[A].Location.SpeedY > 2)
                                            NPC[A].Location.SpeedY = 2;
                                        if(NPC[A].Location.SpeedY < -2)
                                            NPC[A].Location.SpeedY = -2;
                                        if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y + 25)
                                            NPC[A].Special4 = 1;
                                        else if(NPC[A].Location.Y < NPC[A].DefaultLocation.Y - 25)
                                            NPC[A].Special4 = 0;
                                        if(NPC[A].Special4 == 0)
                                            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05;
                                        else
                                            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.05;
                                    }
                                    else
                                    {
                                        if(NPC[A].Special4 == 0)
                                        {
                                            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.025;
                                            if(NPC[A].Location.SpeedY <= -1)
                                                NPC[A].Special4 = 1;
                                            if(NPC[A].Special == 3 && NPC[A].Location.SpeedY <= -0.5)
                                                NPC[A].Special4 = 1;
                                        }
                                        else
                                        {
                                            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.025;
                                            if(NPC[A].Location.SpeedY >= 1)
                                                NPC[A].Special4 = 0;
                                            if(NPC[A].Special == 3 && NPC[A].Location.SpeedY >= 0.5)
                                                NPC[A].Special4 = 0;
                                        }
                                    }
                                }
                                else if(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 1 && NPC[A].Special5 == 1)
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 0.6;
                                else if(NPC[A].Type == 278 || NPC[A].Type == 278)
                                {
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 0.75;
                                    if(NPC[A].Location.SpeedY > Physics.NPCGravity * 15)
                                        NPC[A].Location.SpeedY = Physics.NPCGravity * 15;
                                }
                                else if(NPC[A].Type != 259 && NPC[A].Type != 260)
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity;
                            }
                        }


                        if(NPC[A].Type == 291)
                        {
                            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + Physics.NPCGravity * 0.8;
                            // If .Location.SpeedY >= 5 Then .Location.SpeedY = 5
                            if(NPC[A].Location.SpeedX < -0.005)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.02;
                            else if(NPC[A].Location.SpeedX > 0.005)
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.02;
                            else
                                NPC[A].Location.SpeedX = 0;


                        }

                    }
                    else if(NPC[A].Projectile == true)
                    {
                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
                        if(NPC[A].Location.SpeedY > -0.1 && NPC[A].Location.SpeedY < 0.1)
                        {
                            NPC[A].Projectile = false;
                            NPC[A].Location.SpeedY = 0;
                        }
                    }

                    if(NPC[A].Location.SpeedY >= 8 && NPC[A].Type != 259 && NPC[A].Type != 260)
                        NPC[A].Location.SpeedY = 8;
                    if(NPC[A].Type == 40)
                    {
                        if(NPC[A].Projectile == false)
                        {
                            NPC[A].Location.SpeedY = 0; // egg code
                            if(NPC[A].Location.SpeedX == 0)
                                NPC[A].Projectile = true;
                        }
                    }
                    if((NPC[A].Type == 45 || NPC[A].Type == 46 || NPC[A].Type == 212) && NPC[A].Special == 0)
                        NPC[A].Location.SpeedY = 0;
                    if(NPC[A].Type == 50 || NPC[A].Type == 211)
                    {
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                    }
                    NPCSpecial(A);
                    // Dont move
                    if(NPC[A].Stuck == true && NPC[A].Projectile == false && NPC[A].Type != 34) // face closest player
                    {
                        NPC[A].Location.SpeedX = 0;
                        if(!(NPC[A].Type == 189 && NPC[A].Special > 0))
                        {
                            double C = 0;
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
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
                        }
                    }
                    // Actual Movement

                    if(NPC[A].Type == 78)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                            {
                                NPC[A].Location.SpeedX = 0;
                                break;
                            }
                        }
                    }
                    if(NPC[A].Type == 263)
                    {
                        if(NPC[A].Projectile == true || NPC[A].Wet > 0 || NPC[A].HoldingPlayer > 0)
                            NPC[A].Special3 = 0;
                        else if(NPC[A].Special3 == 1)
                        {
                            NPC[A].Location.SpeedX = 0;
                            NPC[A].Location.SpeedY = 0;
                        }
                    }
                    if((NPC[A].Type == 96 && NPC[A].Special2 == 1) || NPC[A].Type == 151 || NPC[A].Type == 159)
                    {
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                    }
                    else if(NPC[A].Type == 160 || NPC[A].Type == 188)
                        NPC[A].Location.SpeedY = 0;
                    if(NPC[A].Type == 192)
                    {
                        NPC[A].Projectile = false;
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                    }
                    if(NPC[A].Type == 190)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                            {
                                NPC[A].Location.SpeedX = 0;
                                NPC[A].Location.SpeedY = 0;
                            }
                        }
                    }


                    if(NPC[A].Type == 247 && NPC[A].Projectile == false)
                    {
                        speedVar = (float)(speedVar * 0.7);
                        if(NPC[A].Special2 < 2)
                        {
                            speedVar = (float)(speedVar * 0.7);
                            NPC[A].Special2 = NPC[A].Special2 + 1;
                        }
                    }

                    if((!NPCIsAnExit[NPC[A].Type] || NPC[A].Type == 97 || NPC[A].Type == 196) && !(NPC[A].Type == 14) && !(NPC[A].Type == 57))
                    {
                        if(!NPCIsAParaTroopa[NPC[A].Type] && !(NPC[A].Type == 91))
                        {
                            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.SpeedX * speedVar;
                            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.SpeedY;
                        }
                    }
                    else
                    {
                        if(!(NPC[A].Location.X == NPC[A].DefaultLocation.X && NPC[A].Location.Y == NPC[A].DefaultLocation.Y) || NPC[A].Type == 14)
                        {
                            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.99;
                            NPC[A].Location.X = NPC[A].Location.X + NPC[A].Location.SpeedX;
                            NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.SpeedY;
                            if(NPC[A].Projectile == false)
                                NPC[A].Location.SpeedX = 0;
                        }
                    }
                    // End If 'end of freezenpcs

                    if(NPC[A].Type == 263 && NPC[A].Special == 3)
                        NPC[A].BeltSpeed = 0;


                    // projectile check
                    if(NPC[A].Type == 13 || NPC[A].Type == 108 || NPC[A].Type == 265 || NPC[A].Type == 266 || NPC[A].Type == 171 || NPC[A].Type == 292)
                        NPC[A].Projectile = true;

                    // make things projectiles
                    if(NPC[A].Type == 58 || NPC[A].Type == 21 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70 || (NPC[A].Type >= 78 && NPC[A].Type <= 83))
                    {
                        if(NPC[A].Location.SpeedY > Physics.NPCGravity * 20)
                            NPC[A].Projectile = true;
                        else
                            NPC[A].Projectile = false;
                    }
                    if(NPC[A].Type == 78)
                        NPC[A].Projectile = true;
                    if(NPC[A].Type == 241 && (NPC[A].Location.SpeedY > 2 || NPC[A].Location.SpeedY < -2))
                        NPC[A].Projectile = true;
                    // Special NPCs code
                    SpecialNPC(A);
                    // Block Collision

                    if(NPC[A].Type == 179)
                        NPC[A].Location.Height = 24;

                    if(NPC[A].Pinched1 > 0)
                        NPC[A].Pinched1 = NPC[A].Pinched1 - 1;
                    if(NPC[A].Pinched2 > 0)
                        NPC[A].Pinched2 = NPC[A].Pinched2 - 1;
                    if(NPC[A].Pinched3 > 0)
                        NPC[A].Pinched3 = NPC[A].Pinched3 - 1;
                    if(NPC[A].Pinched4 > 0)
                        NPC[A].Pinched4 = NPC[A].Pinched4 - 1;
                    if(NPC[A].MovingPinched > 0)
                        NPC[A].MovingPinched = NPC[A].MovingPinched - 1;

                    newY = 0;
                    UNUSED(newY);
                    oldBeltSpeed = NPC[A].BeltSpeed;
                    resetBeltSpeed = false;
                    beltClear = false;
                    NPC[A].BeltSpeed = 0;
                    beltCount = 0;
                    addBelt = 0;
                    NPC[A].onWall = false;
                    tempSpeedA = 0;
                    oldSlope = NPC[A].Slope;
                    SlopeTurn = false;
                    NPC[A].Slope = 0;
                    if(NPC[A].Location.X < -(FLBlocks - 1) * 32)
                        NPC[A].Location.X = -(FLBlocks - 1) * 32;
                    if(NPC[A].Location.X + NPC[A].Location.Width > (FLBlocks + 1) * 32)
                        NPC[A].Location.X = (FLBlocks + 1) * 32 - NPC[A].Location.Width;

                    if(!(NPCIsACoin[NPC[A].Type] && NPC[A].Special == 0) && !(NPC[A].Type == 45 && NPC[A].Special == 0) && !(NPC[A].Type == 57) && !(NPC[A].Type == 85) && !(NPC[A].Type == 91) && !(NPC[A].Type == 97) && !(NPC[A].Type == 196) && !(NPC[A].Type >= 104 && NPC[A].Type <= 106) && !(NPCIsAnExit[NPC[A].Type] && ((NPC[A].DefaultLocation.X == NPC[A].Location.X && NPC[A].DefaultLocation.Y == NPC[A].Location.Y) || NPC[A].Inert == true)) && !(NPC[A].Type == 159) && !(NPC[A].Type == 192) && !(NPC[A].Type == 202) && !(NPC[A].Type == 246 || NPC[A].Type == 255 || NPC[A].Type == 259 || NPC[A].Type == 260))
                    {

                        if((NPCNoClipping[NPC[A].Type] == false || (NPC[A].Projectile == true)) && !(NPC[A].Type == 40 && NPC[A].Projectile == true) && !(NPC[A].Type == 50) && NPC[A].standingOnPlayer == 0 && !(NPCIsVeggie[NPC[A].Type] && NPC[A].Projectile == true) && !(NPC[A].Type == 30) && !(NPC[A].Type == 18) && !(NPC[A].Type == 108) && !(NPCIsCheep[NPC[A].Type] == true && NPC[A].Special == 2) && !(NPC[A].Type == 272))
                        {
                            for(bCheck = 1; bCheck <= 2; bCheck++)
                            {
                                if(bCheck == 1)
                                {
                                    fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(NPC[A].Location.X / 32))) - 1];
                                    lBlock = LastBlock[floor((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];
                                }
                                else
                                {
                                    fBlock = numBlock + 1 - numTempBlock;
                                    lBlock = numBlock;
                                }
                                for(B = (int)fBlock; B <= lBlock; B++)
                                {
                                    // If Not .Block = B And Not .tempBlock = B And Not (.Projectile = True And Block(B).noProjClipping = True) And BlockNoClipping(Block(B).Type) = False And Block(B).Hidden = False And Block(B).Hidden = False Then




                                    if(NPC[A].Location.X + NPC[A].Location.Width >= Block[B].Location.X)
                                    {
                                        if(NPC[A].Location.X <= Block[B].Location.X + Block[B].Location.Width)
                                        {
                                            if(NPC[A].Location.Y + NPC[A].Location.Height >= Block[B].Location.Y)
                                            {
                                                if(NPC[A].Location.Y <= Block[B].Location.Y + Block[B].Location.Height)
                                                {

                                                    // If CheckCollision(.Location, Block(B).Location) = True Then



                                                    if(!(NPC[A].Block == B) && !(NPC[A].tempBlock == B) && !(NPC[A].Projectile == true && Block[B].noProjClipping == true) && BlockNoClipping[Block[B].Type] == false && Block[B].Hidden == false)
                                                    {
                                                        if(Block[B].IsNPC == 78 && NPCNoClipping[NPC[A].Type] == false && NPC[A].Type != 17)
                                                            NPCHit(A, 8);

                                                        if(Block[B].IsNPC != 57 && (NPCIsABlock[Block[B].IsNPC] || NPCIsAHit1Block[Block[B].IsNPC] || NPCCanWalkOn[Block[B].IsNPC]))
                                                            HitSpot = NPCFindCollision(NPC[A].Location, Block[B].Location);
                                                        else
                                                            HitSpot = FindCollisionBelt(NPC[A].Location, Block[B].Location, oldBeltSpeed);
                                                        if(NPCIsCheep[NPC[A].Type])
                                                        {
                                                            if(NPC[A].Wet == 0)
                                                            {
                                                                if(NPC[A].WallDeath >= 9)
                                                                    HitSpot = 0;
                                                            }
                                                        }
                                                        if(NPC[A].Type == 171 || NPC[A].Type == 266 || NPC[A].Type == 292)
                                                        {
                                                            if(Block[B].Type == 457)
                                                                KillBlock(B);
                                                            HitSpot = 0;
                                                        }
                                                        if(NPC[A].Type == 266)
                                                            HitSpot = 0;
                                                        if(Block[B].IsPlayer > 0 && ((NPCStandsOnPlayer[NPC[A].Type] == false && NPC[A].Type != 13) || NPC[A].Inert == true))
                                                            HitSpot = 0;
                                                        if((NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2) && HitSpot != 3)
                                                            HitSpot = 0;
                                                        if(Block[B].Invis == true)
                                                        {
                                                            if(HitSpot != 3)
                                                                HitSpot = 0;
                                                        }
                                                        if(HitSpot == 5)
                                                        {
                                                            if(CheckHitSpot1(NPC[A].Location, Block[B].Location) == true)
                                                                HitSpot = 1;
                                                        }
                                                        if(NPC[A].Type == 58 || NPC[A].Type == 21 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70)
                                                        {
                                                            if(Block[B].IsPlayer > 0 || Block[B].IsNPC == 56)
                                                            {
                                                                HitSpot = 0;
                                                                NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                            }
                                                        }
                                                        if(NPC[A].Type >= 78 && NPC[A].Type <= 83 && HitSpot != 1)
                                                            HitSpot = 0;
                                                        if(NPC[A].Type == 48 && (Block[B].IsNPC == 22 || Block[B].IsNPC == 49)) // spiney eggs don't walk on special items
                                                            HitSpot = 0;
                                                        if(NPC[A].Type == 190 && Block[B].IsNPC > 0)
                                                            HitSpot = 0;
                                                        if(NPC[A].Type == 86)
                                                        {
                                                            if(HitSpot != 1 && NPC[A].Special > 0)
                                                            {
                                                                if(Block[B].Location.X < level[NPC[A].Section].X + 48 || Block[B].Location.X > level[NPC[A].Section].Width - 80)
                                                                    NPC[A].Special = 0;
                                                                else
                                                                {
                                                                    HitSpot = 0;
                                                                    resetBeltSpeed = true;
                                                                }
                                                            }
                                                            else if(HitSpot == 3)
                                                            {
                                                                if(fEqual(NPC[A].Special4, 3))
                                                                {
                                                                    NPC[A].Frame = 10;
                                                                    NPC[A].Special3 = 21;
                                                                    NPC[A].Special = 1;
                                                                    NPC[A].Location.SpeedX = 0;
                                                                }
                                                            }
                                                        }

                                                        if(NPC[A].Type == 13 || NPC[A].Type == 265)
                                                        {
                                                            if(Block[B].Type == 626 && fEqual(NPC[A].Special, 1))
                                                                HitSpot = 0;
                                                            if(Block[B].Type == 627 && fEqual(NPC[A].Special, 2))
                                                                HitSpot = 0;
                                                            if(Block[B].Type == 628 && fEqual(NPC[A].Special, 3))
                                                                HitSpot = 0;
                                                            if(Block[B].Type == 629 && fEqual(NPC[A].Special, 4))
                                                                HitSpot = 0;
                                                        }

                                                        if(NPC[A].Type == 13 || NPC[A].Type == 108)
                                                        {
                                                            if(Block[B].Type == 621 || Block[B].Type == 620)
                                                            {
                                                                NPCHit(A, 3, A);
                                                                if(Block[B].Type == 621)
                                                                    Block[B].Type = 109;
                                                                else
                                                                {
                                                                    Block[B].Layer = "Destroyed Blocks";
                                                                    Block[B].Hidden = true;
                                                                    numNPCs++;
                                                                    NPC[numNPCs] = NPC_t();
                                                                    NPC[numNPCs].Location.Width = 28;
                                                                    NPC[numNPCs].Location.Height = 32;
                                                                    NPC[numNPCs].Type = 10;
                                                                    NPC[numNPCs].Location.Y = Block[B].Location.Y;
                                                                    NPC[numNPCs].Location.X = Block[B].Location.X + 2;
                                                                    NPC[numNPCs].Active = true;
                                                                    NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
                                                                    NPC[numNPCs].DefaultLocation = NPC[numNPCs].Location;
                                                                    NPC[numNPCs].TimeLeft = 100;
                                                                    CheckSectionNPC(numNPCs);
                                                                }
                                                            }
                                                        }

                                                        if((NPC[A].Type == 37 || NPC[A].Type == 180) && HitSpot != 1)
                                                            HitSpot = 0;
                                                        if(Block[B].IsNPC == 57 && HitSpot == 5)
                                                        {
                                                            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width / 2.0)
                                                                HitSpot = 4;
                                                            else
                                                                HitSpot = 2;
                                                        }
                                                        if(NPC[A].Type == 13 || NPC[A].Type == 265)
                                                        {
                                                            if(NPCIsABonus[Block[B].IsNPC])
                                                                HitSpot = 0;
                                                        }
                                                        if(NPC[A].Type == 15 && HitSpot == 5)
                                                        {
                                                            if(NPC[A].WallDeath >= 5)
                                                                NPC[A].Killed = 3;
                                                            else
                                                                HitSpot = 3;
                                                        }
                                                        if(BlockIsSizable[Block[B].Type] && HitSpot != 1)
                                                            HitSpot = 0;
                                                        if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type] == true)
                                                        {
                                                            if(HitSpot != 1 || (NPCIsAParaTroopa[NPC[A].Type] && NPC[A].Special != 1))
                                                                HitSpot = 0;
                                                        }
                                                        if(NPC[A].Type == 133 && HitSpot > 0)
                                                            NPC[A].Killed = 4;
                                                        if(NPC[A].Type == 134 && NPC[A].Projectile == true && HitSpot != 0)
                                                            NPC[A].Special = 1000;
                                                        if(NPC[A].Shadow == true && HitSpot != 1 && !(Block[B].Special > 0 && NPC[A].Projectile == true))
                                                            HitSpot = 0;


                                                        // vine makers
                                                        if(NPC[A].Type == 225 || NPC[A].Type == 226 || NPC[A].Type == 227)
                                                        {
                                                            if(HitSpot == 3)
                                                                NPC[A].Special = 1;
                                                        }

                                                        if(NPC[A].Type == 197 && Block[B].IsNPC > 0)
                                                            HitSpot = 0;




                                                        if(NPC[A].Type == 205 || NPC[A].Type == 206 || NPC[A].Type == 207)
                                                        {
                                                            NPC[A].Special5 = 0;
                                                            if(HitSpot == 1)
                                                            {
                                                                if(NPC[A].Special == 4 && NPC[A].Location.X + 0.99 == Block[B].Location.X + Block[B].Location.Width)
                                                                    HitSpot = 0;
                                                                if(NPC[A].Special == 2 && NPC[A].Location.X + NPC[A].Location.Width - 0.99 == Block[B].Location.X)
                                                                    HitSpot = 0;
                                                            }
                                                            if(BlockIsSizable[Block[B].Type] == true || BlockOnlyHitspot1[Block[B].Type] == true)
                                                                HitSpot = 0;
                                                            if(BlockSlope2[Block[B].Type] != 0 && HitSpot == 3)
                                                            {
                                                                if(NPC[A].Special == 4 && NPC[A].Special2 == -1)
                                                                {
                                                                    if(NPC[A].Location.X - 0.01 == Block[B].Location.X)
                                                                    {
                                                                        NPC[A].Special = 3;
                                                                        NPC[A].Special2 = 1;
                                                                        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedY;
                                                                    }
                                                                }
                                                            }
                                                            if(BlockSlope2[Block[B].Type] != 0 && HitSpot == 1)
                                                            {
                                                                if(NPC[A].Special == 4)
                                                                    HitSpot = 2;
                                                                if(NPC[A].Special == 2)
                                                                    HitSpot = 4;
                                                            }
                                                            if(NPC[A].Special == 3)
                                                            {
                                                                if(BlockSlope2[Block[B].Type] != 0)
                                                                {
                                                                    if(HitSpot == 2 || HitSpot == 4)
                                                                        HitSpot = 0;
                                                                }
                                                                else if(HitSpot == 2 || HitSpot == 4)
                                                                    HitSpot = 0;
                                                            }
                                                        }



                                                        if(BlockSlope2[Block[B].Type] != 0 && HitSpot > 0 && ((NPC[A].Location.Y > Block[B].Location.Y) || ((NPC[A].Type == 205 || NPC[A].Type == 206 || NPC[A].Type == 207) && NPC[A].Special == 3)))
                                                        {

                                                            if(HitSpot == 5)
                                                            {
                                                                if(NPC[A].Special == 2 && NPC[A].Special2 == 1)
                                                                {
                                                                    NPC[A].Special2 = 1;
                                                                    NPC[A].Special = 3;
                                                                }
                                                                if(NPC[A].Special == 4 && NPC[A].Special2 == 1)
                                                                {
                                                                    NPC[A].Special2 = -1;
                                                                    NPC[A].Special = 3;
                                                                }
                                                            }

                                                            HitSpot = 0;
                                                            if(BlockSlope2[Block[B].Type] == 1)
                                                                PlrMid = NPC[A].Location.X + NPC[A].Location.Width;
                                                            else
                                                                PlrMid = NPC[A].Location.X;
                                                            Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;
                                                            if(BlockSlope2[Block[B].Type] > 0)
                                                                Slope = 1 - Slope;
                                                            if(Slope < 0)
                                                                Slope = 0;
                                                            if(Slope > 1)
                                                                Slope = 1;

                                                            if(NPC[A].Location.Y < Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope) - 0.1)
                                                            {

                                                                if(NPC[A].Type == 13 || NPC[A].Type == 17 || NPC[A].Type == 265)
                                                                    NPCHit(A, 3, A);

                                                                if(NPC[A].Type == 263)
                                                                {
                                                                    if(NPC[A].Location.SpeedY < -2)
                                                                        NPCHit(A, 3, A);
                                                                }



                                                                if(fEqual(NPC[A].Location.SpeedY, double(Physics.NPCGravity)) || NPC[A].Slope > 0 || oldSlope > 0)
                                                                {

                                                                    if((NPC[A].Special == 2 || NPC[A].Special == 4) && NPC[A].Special2 == -1)
                                                                    {
                                                                        if(NPC[A].Special == 4)
                                                                            NPC[A].Special2 = 1;
                                                                        if(NPC[A].Special == 2)
                                                                            NPC[A].Special2 = -1;
                                                                        NPC[A].Special = 3;

                                                                    }


                                                                    PlrMid = NPC[A].Location.Y;
                                                                    Slope = (PlrMid - Block[B].Location.Y) / Block[B].Location.Height;
                                                                    if(Slope < 0)
                                                                        Slope = 0;
                                                                    if(Slope > 1)
                                                                        Slope = 1;
                                                                    if(BlockSlope2[Block[B].Type] < 0)
                                                                        NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width - (Block[B].Location.Width * Slope);
                                                                    else
                                                                        NPC[A].Location.X = Block[B].Location.X + (Block[B].Location.Width * Slope) - NPC[A].Location.Width;
                                                                    SlopeTurn = true;
                                                                    if(NPC[A].Location.SpeedX < 0)
                                                                        HitSpot = 2;
                                                                    else
                                                                        HitSpot = 4;
                                                                }
                                                                else
                                                                {
                                                                    NPC[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope);
                                                                    if(NPC[A].Type == 205 || NPC[A].Type == 206 || NPC[A].Type == 207)
                                                                    {
                                                                        NPC[A].Location.Y = NPC[A].Location.Y + 1;
                                                                        tempBlockHit[1] = 0;
                                                                        tempBlockHit[2] = 0;
                                                                    }
                                                                    if(NPC[A].Location.SpeedY < -0.01)
                                                                        NPC[A].Location.SpeedY = -0.01 + Block[B].Location.SpeedY;

                                                                    if(NPCIsAParaTroopa[NPC[A].Type] == true)
                                                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 2;

                                                                }
                                                            }
                                                        }



                                                        if(BlockSlope[Block[B].Type] != 0 && HitSpot > 0)
                                                        {
                                                            HitSpot = 0;
                                                            if(NPC[A].Location.Y + NPC[A].Location.Height <= Block[B].Location.Y + Block[B].Location.Height + NPC[A].Location.SpeedY + 4)
                                                            {
                                                                if(NPC[A].Location.X < Block[B].Location.X + Block[B].Location.Width && NPC[A].Location.X + NPC[A].Location.Width > Block[B].Location.X)
                                                                {

                                                                    if(BlockSlope[Block[B].Type] == 1)
                                                                        PlrMid = NPC[A].Location.X;
                                                                    else
                                                                        PlrMid = NPC[A].Location.X + NPC[A].Location.Width;
                                                                    Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;
                                                                    if(BlockSlope[Block[B].Type] < 0)
                                                                        Slope = 1 - Slope;
                                                                    if(Slope < 0)
                                                                        Slope = 0;
                                                                    if(Slope > 100)
                                                                        Slope = 100;
                                                                    if(tempHitBlock > 0)
                                                                    {
                                                                        if(BlockIsSizable[Block[tempHitBlock].Type] == false)
                                                                        {
                                                                            if(Block[tempHitBlock].Location.Y != Block[B].Location.Y)
                                                                            {
                                                                                tempHitBlock = 0;
                                                                                tempHit = 0;
                                                                            }
                                                                        }
                                                                    }
                                                                    if(NPC[A].Location.Y >= Block[B].Location.Y + (Block[B].Location.Height * Slope) - NPC[A].Location.Height - 0.1)
                                                                    {
                                                                        if(NPC[A].Type == 241 && NPC[A].Location.SpeedY > 2)
                                                                            NPCHit(A, 4, A);
                                                                        if((NPC[A].Type == 205 || NPC[A].Type == 206 || NPC[A].Type == 207) && NPC[A].Special == 3)
                                                                        {
                                                                            NPC[A].Special = 1;
                                                                            NPC[A].Special2 = -NPC[A].Special2;
                                                                        }

                                                                        if(NPC[A].Type == 17 || NPC[A].Type == 40) // Bullet bills crash on slopes
                                                                        {
                                                                            NPC[A].Slope = 1;
                                                                            if(NPC[A].Location.SpeedX < 0)
                                                                                HitSpot = 2;
                                                                            else
                                                                                HitSpot = 4;
                                                                        }
                                                                        else
                                                                        {


                                                                            NPC[A].Location.Y = Block[B].Location.Y + (Block[B].Location.Height * Slope) - NPC[A].Location.Height - 0.1;
                                                                            if(NPCIsCheep[NPC[A].Type])
                                                                                NPC[A].TurnAround = true;
                                                                            NPC[A].Slope = B;
                                                                            HitSpot = 1;
                                                                            // Fireballs dont go up steep slopes
                                                                            if(Block[B].Location.Height / static_cast<double>(Block[B].Location.Width) >= 1 && ((BlockSlope[Block[B].Type] == -1 && NPC[A].Location.SpeedX > 0) || (BlockSlope[Block[B].Type] == 1 && NPC[A].Location.SpeedX < 0)))
                                                                            {
                                                                                if((NPC[A].Type == 13 && NPC[A].Special != 2 && NPC[A].Special != 3) || (NPC[A].Type == 265 && NPC[A].Special == 5))
                                                                                {
                                                                                    if(NPC[A].Location.SpeedX < 0)
                                                                                        HitSpot = 2;
                                                                                    else
                                                                                        HitSpot = 4;
                                                                                }
                                                                            }
                                                                            if(NPCIsAShell[NPC[A].Type] || (NPC[A].Type == 45 && NPC[A].Special == 1) || NPC[A].Type == 263)
                                                                            {
                                                                                if(NPC[A].Location.SpeedY > NPC[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type])
                                                                                {
                                                                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type];
                                                                                    HitSpot = 0;
                                                                                    if(NPC[A].Location.SpeedY > 0)
                                                                                        NPC[A].Location.SpeedY = 0;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }



                                                        if(NPC[A].Type == 206 || NPC[A].Type == 205 || NPC[A].Type == 207)
                                                        {
                                                            if(NPC[A].Special == 3 && (HitSpot == 2 || HitSpot == 4))
                                                            {
                                                                if(Block[B].Location.Y + Block[B].Location.Height <= NPC[A].Location.Y + 1)
                                                                    HitSpot = 3;
                                                            }
                                                            if(Block[B].IsNPC > 0)
                                                                HitSpot = 0;
                                                        }

                                                        if(BlockKills[Block[B].Type] && (HitSpot > 0 || NPC[A].Slope == B))
                                                            NPCHit(A, 6, B);

                                                        if(NPC[A].Type == 13 && Block[B].IsNPC == 263)
                                                            HitSpot = 0;

                                                        if(NPC[A].Type == 96 && HitSpot == 1)
                                                        {
                                                            if((NPC[A].Location.SpeedY > 2 && HitSpot == 1) || (NPC[A].Location.SpeedY < -2 && HitSpot == 3) || (NPC[A].Location.SpeedX > 2 && HitSpot == 4) || (NPC[A].Location.SpeedX < -2 && HitSpot == 2))
                                                                NPC[A].Special2 = 1;
                                                        }
                                                        if(HitSpot == 5)
                                                        {
                                                            if(NPC[A].Slope > 0 && Block[B].Location.Y + Block[B].Location.Height < NPC[A].Location.Y + 4)
                                                            {
                                                                if(Block[B].Location.X + Block[B].Location.Width < NPC[A].Location.X + 4 || Block[B].Location.X > NPC[A].Location.X + NPC[A].Location.Width - 4)
                                                                    HitSpot = 0;
                                                            }
                                                        }
                                                        // beech koopa kicking an ice block
                                                        if((NPC[A].Type == 55 || NPC[A].Type == 119) && Block[B].IsNPC == 45)
                                                        {
                                                            if(HitSpot == 2 || HitSpot == 4)
                                                            {
                                                                if(NPC[A].Location.SpeedY == Physics.NPCGravity ||
                                                                   NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0 ||
                                                                   (oldSlope > 0 && !NPC[Block[B].IsReally].Projectile))
                                                                {
                                                                    NPC[Block[B].IsReally].Special = 1;
                                                                    NPC[A].Special = 10;
                                                                    Player[numPlayers + 1].Direction = NPC[A].Direction;
                                                                    NPC[A].Location.X = NPC[A].Location.X - NPC[A].Direction;
                                                                    NPCHit(Block[B].IsReally, 1, numPlayers + 1);
                                                                    HitSpot = 0;
                                                                }
                                                            }
                                                        }
                                                        if(NPC[A].Type == 179 && Block[B].IsNPC > 0)
                                                            HitSpot = 0;
                                                        if(Block[B].IsNPC == 208 || Block[B].IsNPC == 209)
                                                        {
                                                            if(NPC[A].Projectile == true)
                                                            {
                                                                NPCHit(Block[B].IsReally, 3, A);
                                                                NPCHit(A, 4, Block[B].IsReally);
                                                            }
                                                        }



                                                        if((NPC[A].Type == 237 || NPC[A].Type == 263) && (HitSpot == 2 || HitSpot == 4 || HitSpot == 5))
                                                        {
                                                            if(Block[B].IsNPC == 263)
                                                            {
                                                                NPCHit(Block[B].IsReally, 3, Block[B].IsReally);
                                                                NPC[Block[B].IsReally].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                NPC[A].Multiplier = NPC[A].Multiplier + 1;
                                                            }
                                                            NPCHit(A, 3, A);
                                                        }

                                                        if(NPC[A].Type == 263 && (HitSpot == 1 || HitSpot == 3 || HitSpot == 5))
                                                        {
                                                            if(NPC[A].Location.SpeedX > -Physics.NPCShellSpeed * 0.8 && NPC[A].Location.SpeedX < Physics.NPCShellSpeed * 0.8)
                                                            {
                                                                if(NPC[A].Location.SpeedY > 5 || NPC[A].Location.SpeedY < -2)
                                                                    NPCHit(A, 3, A);
                                                            }
                                                        }

                                                        if(NPCIsACoin[NPC[A].Type] && NPC[A].Special == 0 && HitSpot > 0)
                                                            NPCHit(A, 3, A);

                                                        if(Block[B].Location.SpeedX != 0 && (HitSpot == 2 || HitSpot == 4))
                                                            NPC[A].MovingPinched = 2;
                                                        if(Block[B].Location.SpeedY != 0 && (HitSpot == 1 || HitSpot == 3))
                                                            NPC[A].MovingPinched = 2;

                                                        if(NPC[A].TimeLeft > 1)
                                                        {
                                                            if(HitSpot == 1)
                                                                NPC[A].Pinched1 = 2;
                                                            else if(HitSpot == 2)
                                                                NPC[A].Pinched2 = 2;
                                                            else if(HitSpot == 3)
                                                                NPC[A].Pinched3 = 2;
                                                            else if(HitSpot == 4)
                                                                NPC[A].Pinched4 = 2;
                                                            else if(HitSpot == 5)
                                                            {
                                                                double C = 0;
                                                                int D = 0;

                                                                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width / 2.0)
                                                                {
                                                                    C = (Block[B].Location.X + Block[B].Location.Width / 2.0) - (NPC[A].Location.X + NPC[A].Location.Width / 2.0);
                                                                    D = 2;
                                                                }
                                                                else
                                                                {
                                                                    C = (NPC[A].Location.X + NPC[A].Location.Width / 2.0) - (Block[B].Location.X + Block[B].Location.Width / 2.0);
                                                                    D = 4;
                                                                }

                                                                if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 < Block[B].Location.Y + Block[B].Location.Height / 2.0)
                                                                {
                                                                    if(C < (Block[B].Location.Y + Block[B].Location.Height / 2.0) - (NPC[A].Location.Y + NPC[A].Location.Height / 2.0))
                                                                        D = 1;
                                                                }
                                                                else
                                                                {
                                                                    if(C < (NPC[A].Location.Y + NPC[A].Location.Height / 2.0) - (Block[B].Location.Y + Block[B].Location.Height / 2.0))
                                                                        D = 3;
                                                                }

                                                                if(D == 1)
                                                                    NPC[A].Pinched1 = 2;
                                                                if(D == 2)
                                                                    NPC[A].Pinched2 = 2;
                                                                if(D == 3)
                                                                    NPC[A].Pinched3 = 2;
                                                                if(D == 4)
                                                                    NPC[A].Pinched4 = 2;

                                                                if(Block[B].Location.SpeedX != 0.0 && (D == 2 || D == 4))
                                                                    NPC[A].MovingPinched = 2;
                                                                if(Block[B].Location.SpeedY != 0.0 && (D == 1 || D == 3))
                                                                    NPC[A].MovingPinched = 2;



                                                                // If Not (.Location.Y + .Location.Height - .Location.SpeedY <= Block(B).Location.Y - Block(B).Location.SpeedY) Then .Pinched1 = 2
                                                                // If Not (.Location.Y - .Location.SpeedY >= Block(B).Location.Y + Block(B).Location.Height - Block(B).Location.SpeedY) Then .Pinched3 = 2
                                                                // If Not (.Location.X + .Location.Width - .Location.SpeedX <= Block(B).Location.X - Block(B).Location.SpeedX) Then .Pinched2 = 2
                                                                // If Not (.Location.X - .Location.SpeedX >= Block(B).Location.X + Block(B).Location.Width - Block(B).Location.SpeedX) Then .Pinched4 = 2
                                                            }
                                                            if(NPC[A].MovingPinched > 0)
                                                            {
                                                                if((NPC[A].Pinched1 > 0 && NPC[A].Pinched3 > 0) || (NPC[A].Pinched2 > 0 && NPC[A].Pinched4 > 0))
                                                                {
                                                                    if(HitSpot > 1)
                                                                        HitSpot = 0;
                                                                    NPC[A].Damage = NPC[A].Damage + 10000;
                                                                    NPC[A].Immune = 0;
                                                                    NPC[0].Multiplier = 0;
                                                                    NPCHit(A, 3, 0);
                                                                }
                                                            }
                                                        }

                                                        if(HitSpot == 1 && NPC[A].Type == 241 && NPC[A].Location.SpeedY > 2)
                                                            NPCHit(A, 4, A);
                                                        if(HitSpot == 1 && NPC[A].Type == 247 && Block[B].IsNPC == 247)
                                                        {
                                                            NPC[Block[B].IsReally].Special = -3;
                                                            NPC[A].Special2 = 0;
                                                        }
                                                        if((NPC[A].Type == 13 || NPC[A].Type == 265) && NPC[A].Special == 5 && HitSpot > 0)
                                                            NPCHit(A, 3, A);
                                                        if(NPC[A].Type == 265 && HitSpot > 1)
                                                            NPCHit(A, 3, A);
                                                        if(NPC[A].Type == 283 && BlockIsSizable[Block[B].Type] == false)
                                                            NPCHit(A, 3, A);
                                                        if(NPC[A].Type == 286 && HitSpot == 1)
                                                            NPC[A].Special = 1;
                                                        if(NPC[A].Type == 288 && HitSpot == 1)
                                                        {
                                                            NPC[A].Special3 = 1;
                                                            NPC[A].Projectile = false;
                                                        }
                                                        if(NPC[A].Type == 291 && HitSpot > 0)
                                                            NPCHit(A, 3, A);
                                                        // hitspot 1
                                                        if(HitSpot == 1) // Hitspot 1
                                                        {
                                                            if((NPC[A].Type == 13 || NPC[A].Type == 265) && NPC[A].Location.SpeedX == 0)
                                                                NPCHit(A, 4, A);


                                                            if(NPC[A].Type == 197)
                                                                NPC[A].Special = 1;
                                                            if(NPC[A].Type == 231 || NPC[A].Type == 235)
                                                                NPC[A].Special4 = 1;
                                                            tempSpeedA = Block[B].Location.SpeedY;
                                                            if(tempSpeedA < 0)
                                                                tempSpeedA = 0;
                                                            if(NPC[Block[B].IsReally].Type != 57 && NPC[Block[B].IsReally].Type != 60 && NPC[Block[B].IsReally].Type != 62 && NPC[Block[B].IsReally].Type != 64 && NPC[Block[B].IsReally].Type != 66 && Block[B].IsReally > 0)
                                                            {
                                                                if(NPC[Block[B].IsReally].TimeLeft < NPC[A].TimeLeft - 1)
                                                                    NPC[Block[B].IsReally].TimeLeft = NPC[A].TimeLeft - 1;
                                                                else if(NPC[Block[B].IsReally].TimeLeft - 1 > NPC[A].TimeLeft)
                                                                    NPC[A].TimeLeft = NPC[Block[B].IsReally].TimeLeft - 1;
                                                            }
                                                            if(NPC[A].Type == 45 && NPC[A].Special == 1 && NPC[A].Location.SpeedX == 0 && NPC[A].Location.SpeedY > 7.95)
                                                                NPCHit(A, 4, A);
                                                            if(NPC[A].Type == 37 || NPC[A].Type == 180)
                                                                NPC[A].Special = 2;

                                                            if((NPC[A].Type == 58 || NPC[A].Type == 21 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70) && NPC[A].Location.SpeedY > Physics.NPCGravity * 20)
                                                                PlaySound(37);
                                                            if(NPC[A].Type == 78 && NPC[A].Location.SpeedY > Physics.NPCGravity * 10)
                                                                PlaySound(37);

                                                            if(WalkingCollision3(NPC[A].Location, Block[B].Location, oldBeltSpeed) == true || NPC[A].Location.Width > 32)
                                                            {
                                                                resetBeltSpeed = true;

                                                                if(Block[B].IsNPC != 0)
                                                                {
                                                                    if(Block[B].Location.SpeedY > 0 && Block[B].IsNPC >= 60 && Block[B].IsNPC <= 66)
                                                                        tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01 + Block[B].Location.SpeedY;
                                                                    else
                                                                        tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                    tempHitBlock = B;
                                                                }
                                                                else
                                                                {
                                                                    tempHitBlock = B;
                                                                    tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                }
                                                                if(Block[B].IsNPC >= 60 && Block[B].IsNPC <= 66)
                                                                {
                                                                    NPC[A].BeltSpeed = 0;
                                                                    beltCount = 0;
                                                                }

                                                                double C = 0;

                                                                if(NPC[A].Location.X > Block[B].Location.X)
                                                                    C = NPC[A].Location.X - 0.01;
                                                                else
                                                                    C = Block[B].Location.X - 0.01;

                                                                if(NPC[A].Location.X + NPC[A].Location.Width < Block[B].Location.X + Block[B].Location.Width)
                                                                    C = (NPC[A].Location.X + NPC[A].Location.Width - C + 0.01);
                                                                else
                                                                    C = (Block[B].Location.X + Block[B].Location.Width - C + 0.01);

                                                                if(Block[B].IsPlayer == 0)
                                                                {
                                                                    if(Block[B].IsNPC > 0)
                                                                        NPC[A].BeltSpeed = NPC[A].BeltSpeed + float(Block[B].Location.SpeedX * C) * NPCSpeedvar[Block[B].IsNPC];
                                                                    else
                                                                        NPC[A].BeltSpeed = NPC[A].BeltSpeed + float(Block[B].Location.SpeedX * C);
                                                                    beltCount += static_cast<float>(C);
                                                                }
                                                            }

                                                            if(tempHitBlock == B)
                                                            {
                                                                if(NPC[A].Type == 48)
                                                                {
                                                                    if(NPC[A].Location.SpeedY > 2)
                                                                    {
                                                                        NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.7 + Block[B].Location.SpeedY;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                        tempHit = 0;
                                                                        tempHitBlock = 0;
                                                                    }
                                                                }

                                                                if(
                                                                    (
                                                                        (NPCStandsOnPlayer[NPC[A].Type] && NPC[A].Projectile == false) ||
                                                                        (NPCIsAShell[NPC[A].Type] && NPC[A].Location.SpeedX == 0.0)
                                                                    ) && Block[B].IsPlayer > 0
                                                                )
                                                                {
                                                                    NPC[A].standingOnPlayerY = Block[B].standingOnPlayerY + NPC[A].Location.Height;
                                                                    NPC[A].standingOnPlayer = Block[B].IsPlayer;
                                                                    if(NPC[A].standingOnPlayer == 0 && Block[B].IsNPC == 56)
                                                                        NPC[A].TimeLeft = 100;
                                                                }

                                                                if(NPC[A].Projectile)
                                                                {

                                                                    if(NPC[A].Type == 13)
                                                                    {
                                                                        if(NPC[A].Special == 4)
                                                                            NPC[A].Location.SpeedY = -3 + Block[B].Location.SpeedY;
                                                                        else
                                                                            NPC[A].Location.SpeedY = -5 + Block[B].Location.SpeedY;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                        tempHit = 0;
                                                                    }
                                                                    else if(NPC[A].Type == 265)
                                                                    {
                                                                        NPC[A].Location.SpeedY = -7 + Block[B].Location.SpeedY;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                        tempHit = 0;
                                                                        if(Block[B].Slippy == false)
                                                                            NPC[A].Special5 = NPC[A].Special5 + 1;
                                                                        if(NPC[A].Special5 >= 3)
                                                                            NPCHit(A, 3, A);
                                                                    }
                                                                    else if(NPC[A].Type >= 117 && NPC[A].Type <= 120)
                                                                    {
                                                                        // Yes, you aren't mistook, it's just a blank block, hehehe (made by Redigit originally)


                                                                    }
                                                                    else if(NPC[A].Bouce == true || NPC[A].Location.SpeedY > 5.8 || ((NPC[A].Type == 22 || NPC[A].Type == 49) && (NPC[A].Location.SpeedY > 2 || (NPC[A].Location.SpeedX > 1 || NPC[A].Location.SpeedX < -1))))
                                                                    {
                                                                        NPC[A].Bouce = false;
                                                                        if(NPCIsAShell[NPC[A].Type] || (NPC[A].Type == 45 && NPC[A].Special == 1) || NPC[A].Type == 263)
                                                                        {
                                                                            if(NPC[A].Slope == 0)
                                                                                NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.5;
                                                                            for(int C = 1; C <= numPlayers; C++)
                                                                            {
                                                                                if(Player[C].StandingOnNPC == A)
                                                                                {
                                                                                    NPC[A].Location.SpeedY = 0;
                                                                                    break;
                                                                                }
                                                                            }

                                                                        }
                                                                        else if(NPC[A].Type == 58 || NPC[A].Type == 21 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70 || (NPC[A].Type >= 78 && NPC[A].Type <= 83))
                                                                            NPC[A].Location.SpeedY = 0;
                                                                        else if(NPC[A].Type == 86 || NPC[A].Type == 96)
                                                                        {
                                                                            NPC[A].Projectile = false;
                                                                            NPC[A].Location.SpeedY = 0;
                                                                        }
                                                                        else
                                                                            NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.6;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                        tempHit = 0;
                                                                    }
                                                                    else if(NPC[A].Type != 78 && NPC[A].Type != 17 && NPC[A].Type != 13)
                                                                    {
                                                                        if(NPCMovesPlayer[NPC[A].Type] == true)
                                                                        {
                                                                            if(NPC[A].Location.SpeedX == 0)
                                                                            {
                                                                                tempBool = false;
                                                                                for(int C = 1; C <= numPlayers; C++)
                                                                                {
                                                                                    if(CheckCollision(NPC[A].Location, Player[C].Location) == true)
                                                                                    {
                                                                                        tempBool = true;
                                                                                        break;
                                                                                    }
                                                                                }
                                                                                if(tempBool == false)
                                                                                    NPC[A].Projectile = false;
                                                                            }
                                                                        }
                                                                        else if(NPC[A].Type == 168)
                                                                        {
                                                                            if(NPC[A].Location.SpeedX > -0.1 && NPC[A].Location.SpeedX < 0.1)
                                                                                NPC[A].Projectile = false;
                                                                        }
                                                                        else if(!(NPCIsAShell[NPC[A].Type] || (NPC[A].Type == 45 && NPC[A].Special == 1) || NPC[A].Type == 48))
                                                                            NPC[A].Projectile = false;
                                                                        else if(NPC[A].Location.SpeedX == 0)
                                                                            NPC[A].Projectile = false;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        else if(HitSpot == 2) // Hitspot 2
                                                        {
                                                            if(BlockSlope[Block[oldSlope].Type] == 1 && Block[oldSlope].Location.Y == Block[B].Location.Y)
                                                            {
                                                            }
                                                            else
                                                            {
                                                                beltClear = true;
                                                                if(NPC[A].Type == 86)
                                                                    NPC[A].Location.SpeedX = 0;
                                                                addBelt = NPC[A].Location.X;
                                                                resetBeltSpeed = true;
                                                                if(NPC[A].Type == 13 && NPC[A].Special == 3)
                                                                {
                                                                    if(NPC[A].Special2 == 0)
                                                                    {
                                                                        NPC[A].Special2 = 1;
                                                                        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                    }
                                                                    else
                                                                        NPCHit(A, 4, A);
                                                                }
                                                                else if(NPC[A].Type == 13 || NPC[A].Type == 40)
                                                                    NPCHit(A, 4, A);
                                                                if(NPC[A].Type == 45 && NPC[A].Special == 1)
                                                                    NPCHit(A, 4, A);
                                                                if(NPC[A].Slope == 0 && SlopeTurn == false)
                                                                    NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                                                                if(!(NPC[A].Type == 13 || NPC[A].Type == 78 || NPC[A].Type == 17))
                                                                    NPC[A].TurnAround = true;
                                                                if(NPCIsAParaTroopa[NPC[A].Type] == true)
                                                                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - Block[B].Location.SpeedX * 1.2;
                                                                if(NPCIsAShell[NPC[A].Type] == true)
                                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                addBelt = NPC[A].Location.X - addBelt;
                                                            }
                                                        }
                                                        else if(HitSpot == 4) // Hitspot 4
                                                        {
                                                            beltClear = true;
                                                            if(NPC[A].Type == 86)
                                                                NPC[A].Location.SpeedX = 0;
                                                            resetBeltSpeed = true;
                                                            addBelt = NPC[A].Location.X;
                                                            if(NPC[A].Type == 13 && NPC[A].Special == 3)
                                                            {
                                                                if(NPC[A].Special2 == 0)
                                                                {
                                                                    NPC[A].Special2 = 1;
                                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                }
                                                                else
                                                                    NPCHit(A, 4, A);
                                                            }
                                                            else if(NPC[A].Type == 13 || NPC[A].Type == 40)
                                                                NPCHit(A, 4, A);
                                                            if(NPC[A].Type == 45 && NPC[A].Special == 1)
                                                                NPCHit(A, 4, A);
                                                            if(NPC[A].Slope == 0 && SlopeTurn == false)
                                                                NPC[A].Location.X = Block[B].Location.X - NPC[A].Location.Width - 0.01;
                                                            if(!(NPC[A].Type == 13 || NPC[A].Type == 78 || NPC[A].Type == 17))
                                                                NPC[A].TurnAround = true;
                                                            if(NPCIsAParaTroopa[NPC[A].Type] == true)
                                                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - Block[B].Location.SpeedX * 1.2;
                                                            if(NPCIsAShell[NPC[A].Type] == true)
                                                                NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                            addBelt = NPC[A].Location.X - addBelt;
                                                        }
                                                        else if(HitSpot == 3) // Hitspot 3
                                                        {
                                                            if(NPC[A].Type == 45 && NPC[A].Special == 1)
                                                                NPCHit(A, 4, A);
                                                            if(NPC[A].Type == 15)
                                                                NPC[A].Special3 = 0;
                                                            if(tempBlockHit[1] == 0)
                                                                tempBlockHit[1] = B;
                                                            else
                                                                tempBlockHit[2] = B;
                                                            if(NPCIsAParaTroopa[NPC[A].Type])
                                                            {
                                                                NPC[A].Location.SpeedY = 2 + Block[B].Location.SpeedY;
                                                                NPC[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.1;
                                                            }
                                                        }
                                                        else if(HitSpot == 5) // Hitspot 5
                                                        {
                                                            if(NPC[A].Type == 13 || NPC[A].Type == 40)
                                                                NPCHit(A, 4, A);
                                                            beltClear = true;
                                                            if(NPC[A].Type == 86)
                                                                NPC[A].Location.SpeedX = 0;
                                                            NPC[A].onWall = true;
                                                            if(NPC[A].WallDeath >= 5 && !(NPCIsABonus[NPC[A].Type] == true) && NPC[A].Type != 278 && NPC[A].Type != 279 && NPC[A].Type != 191 && NPC[A].Type != 21 && NPC[A].Type != 22 && NPC[A].Type != 26 && NPC[A].Type != 29 && NPC[A].Type != 31 && NPC[A].Type != 32 && NPC[A].Type != 35 && NPC[A].Type != 191 && NPC[A].Type != 193 && NPC[A].Type != 49 && NPC[A].Type != 134 && !(NPC[A].Type == 158) && !(NPC[A].Type == 195) && !(NPC[A].Type == 241) && !((NPC[A].Type >= 154 && NPC[A].Type <= 157))) // walldeath stuff
                                                            {
                                                                NPC[A].Location.SpeedX = Physics.NPCShellSpeed * 0.5 * NPC[A].Direction;
                                                                if(NPCIsVeggie[NPC[A].Type])
                                                                    NPC[A].Projectile = true;
                                                                else if(NPC[A].Type == 135)
                                                                    NPCHit(A, 4, A);
                                                                else if(NPC[A].Type == 291)
                                                                    NPCHit(A, 3, A);
                                                                else
                                                                {
                                                                    NewEffect(75, NPC[A].Location);
                                                                    NPC[A].Killed = 3;
                                                                }
                                                            }
                                                            else if(NPC[A].Type != 48 && !(NPCIsABlock[NPC[A].Type] && Block[B].IsNPC > 0) && !(Block[B].IsNPC == 57))
                                                            {
                                                                addBelt = NPC[A].Location.X;
                                                                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width * 0.5)
                                                                    NPC[A].Location.X = Block[B].Location.X - NPC[A].Location.Width - 0.01;
                                                                else
                                                                    NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                                                                addBelt = NPC[A].Location.X - addBelt;
                                                                if(NPC[A].Type == 15)
                                                                {
                                                                    NPC[A].Location.SpeedY = 0;
                                                                    NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                }
                                                                if(NPC[A].Type != 13 && NPC[A].Type != 78 && NPC[A].Type != 265)
                                                                    NPC[A].TurnAround = true;
                                                                if(NPCIsAShell[NPC[A].Type] == true)
                                                                {
                                                                    if(NPC[A].Location.X < Block[B].Location.X && NPC[A].Location.SpeedX > 0)
                                                                        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                    else if(NPC[A].Location.X + NPC[A].Location.Width > Block[B].Location.X + Block[B].Location.Width && NPC[A].Location.SpeedX < 0)
                                                                        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                }
                                                            }
                                                        }
                                                        if((NPC[A].Projectile & !(NPC[A].Type == 13)) != 0 && !(NPC[A].Type == 265) && !(NPC[A].Type == 58) && !(NPC[A].Type == 21 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70)) // Hit the block if the NPC is a projectile
                                                        {
                                                            if(HitSpot == 2 || HitSpot == 4 || HitSpot == 5)
                                                            {
                                                                BlockHit(B);
                                                                PlaySound(3);
                                                                if(NPC[A].Type == 17) // Bullet Bills
                                                                {
                                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                    NPCHit(A, 4, A);
                                                                    BlockHitHard(B);
                                                                    break;
                                                                }
                                                                if(NPCIsAShell[NPC[A].Type] || (NPC[A].Type == 45 && NPC[A].Special == 1) || NPC[A].Type == 263)
                                                                {
                                                                    BlockHitHard(B);
                                                                    if(Block[B].Type == 4 || Block[B].Type == 188 || Block[B].Type == 60 || Block[B].Type == 90)
                                                                        NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    // End If



                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if((bCheck == 2 || BlocksSorted == true) && PSwitchTime == 0)
                                            break;
                                    }
                                }
                                if(numTempBlock == 0)
                                    break;
                            }
                        }

                        if(tempBlockHit[1] > 0) // find out which block was hit from below
                        {
                            if(tempBlockHit[2] == 0)
                                winningBlock = tempBlockHit[1];
                            else
                            {
                                double C = Block[tempBlockHit[1]].Location.X + Block[tempBlockHit[1]].Location.Width * 0.5;
                                double D = Block[tempBlockHit[2]].Location.X + Block[tempBlockHit[2]].Location.Width * 0.5;
                                C -= (NPC[A].Location.X + NPC[A].Location.Width * 0.5);
                                D -= (NPC[A].Location.X + NPC[A].Location.Width * 0.5);

                                if(C < 0)
                                    C = -C;
                                if(D < 0)
                                    D = -D;

                                if(C < D)
                                    winningBlock = tempBlockHit[1];
                                else
                                    winningBlock = tempBlockHit[2];
                            }
                            if(NPC[A].Type == 13 || NPC[A].Type == 265) // Kill the fireball
                                NPCHit(A, 4);
                            else if(NPC[A].Projectile || Block[winningBlock].Invis == true) // Hit the block hard if the NPC is a projectile
                            {
                                if(!(NPC[A].Type == 58 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70 || NPC[A].Type == 21))
                                {
                                    if(NPC[A].Location.SpeedY < -0.05)
                                    {
                                        BlockHit(winningBlock);
                                        PlaySound(3);
                                        if(NPCIsAShell[NPC[A].Type] || NPC[A].Type == 263)
                                            BlockHitHard(winningBlock);
                                    }
                                    else
                                        NPC[A].Projectile = false;
                                }
                            }
                            if(!NPCIsAParaTroopa[NPC[A].Type])
                            {
                                NPC[A].Location.Y = Block[winningBlock].Location.Y + Block[winningBlock].Location.Height + 0.01;
                                NPC[A].Location.SpeedY = 0.01 + Block[B].Location.SpeedY;
                            }
                        }
                        if(NPCIsAShell[NPC[A].Type])
                        {
                            if(NPC[A].Special > 0)
                            {
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.9;
                                NPC[A].Frame = 0;
                                NPC[A].FrameCount = 0;
                                if(NPC[A].Location.SpeedX > -0.3 && NPC[A].Location.SpeedX < 0.3)
                                {
                                    NPC[A].Location.SpeedX = 0;
                                    NPC[A].Special = 0;
                                    NPC[A].Projectile = false;
                                }
                            }
                        }
                        if(NPC[A].Type == 78 && NPC[A].Location.SpeedX != 0)
                            NPC[A].Location.SpeedX = 1 * NPC[A].DefaultDirection;
                        // beltspeed code
                        if(resetBeltSpeed == false)
                        {
                            if(NPC[A].Type == 86 && NPC[A].Special == 1)
                                NPC[A].Special = 0;
                            if(oldBeltSpeed >= 1 || oldBeltSpeed <= -1)
                            {
                                NPC[A].BeltSpeed = oldBeltSpeed - NPC[A].oldAddBelt;
                                beltCount = 1;
                                if(NPC[A].BeltSpeed >= 2.1)
                                    NPC[A].BeltSpeed = NPC[A].BeltSpeed - 0.1;
                                else if(NPC[A].BeltSpeed <= -2.1)
                                    NPC[A].BeltSpeed = NPC[A].BeltSpeed + 0.1;
                            }
                        }

                        if(NPC[A].BeltSpeed != 0.0f)
                        {
                            preBeltLoc = NPC[A].Location;
                            NPC[A].BeltSpeed = NPC[A].BeltSpeed / beltCount;
                            NPC[A].BeltSpeed = NPC[A].BeltSpeed * speedVar;
                            NPC[A].Location.X = NPC[A].Location.X + double(NPC[A].BeltSpeed);
//                            D = NPC[A].BeltSpeed; // Idk why this is needed as this value gets been overriden and never re-used
                            tempLocation = NPC[A].Location;
                            tempLocation.Y = tempLocation.Y + 1;
                            tempLocation.Height = tempLocation.Height - 2;
                            tempLocation.Width = tempLocation.Width / 2;
                            if(NPC[A].BeltSpeed > 0)
                                tempLocation.X = tempLocation.X + tempLocation.Width;
                            if(!(NPC[A].Type >= 79 && NPC[A].Type <= 83) && !NPC[A].Inert)
                            {
                                for(int C = 1; C <= numNPCs; C++)
                                {
                                    if(A != C && NPC[C].Active == true && !NPC[C].Projectile)
                                    {
                                        if(NPC[C].Killed == 0 && NPC[C].standingOnPlayer == 0 && NPC[C].HoldingPlayer == 0 && NPCNoClipping[NPC[C].Type] == false && NPC[C].Effect == 0 && NPC[C].Inert == false) // And Not NPCIsABlock(NPC(C).Type) Then
                                        {
                                            tempLocation2 = preBeltLoc;
                                            tempLocation2.Width = tempLocation2.Width - 4;
                                            tempLocation2.X = tempLocation2.X + 2;
                                            if(CheckCollision(tempLocation, NPC[C].Location))
                                            {
                                                if(!CheckCollision(tempLocation2, NPC[C].Location))
                                                {
                                                    if(NPC[A].TimeLeft - 1 > NPC[C].TimeLeft)
                                                        NPC[C].TimeLeft = NPC[A].TimeLeft - 1;
                                                    else if(NPC[A].TimeLeft < NPC[C].TimeLeft - 1)
                                                        NPC[A].TimeLeft = NPC[C].TimeLeft - 1;
                                                    NPC[A].onWall = true;
                                                    if((NPC[A].Location.SpeedX > 0 && NPC[C].Location.X > NPC[A].Location.X) || (NPC[A].Location.SpeedX < 0 && NPC[C].Location.X < NPC[A].Location.X))
                                                        NPC[A].TurnAround = true;
                                                    if((NPC[C].Location.SpeedX > 0 && NPC[A].Location.X > NPC[C].Location.X) || (NPC[C].Location.SpeedX < 0 && NPC[A].Location.X < NPC[C].Location.X))
                                                        NPC[C].TurnAround = true;
                                                    NPC[A].Location = preBeltLoc;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if(NPC[A].Location.X == preBeltLoc.X)
                            {
                                NPC[A].BeltSpeed = 0;
                                addBelt = 0;
                                if(NPC[A].tempBlock > 0)
                                    Block[NPC[A].tempBlock].Location.SpeedX = 0;
                            }
                        }
                        if(NPC[A].onWall == false)
                            NPC[A].BeltSpeed = NPC[A].BeltSpeed + addBelt;
                        NPC[A].oldAddBelt = addBelt;
                        if(beltClear == true)
                            NPC[A].BeltSpeed = 0;

                        if(NPC[A].Type == 37 || NPC[A].Type == 180)
                            NPC[A].BeltSpeed = 0;
                        // End Block Collision
                        // If .Type = 12 Then .Projectile = True 'Stop the big fireballs from getting killed from tha lava
                        if(NPC[A].Type == 194)
                            NPC[A].Projectile = true;

                        if(NPC[A].Type == 179)
                        {
                            NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                            NPC[A].Projectile = true;
                        }

                        // NPC Collision

                        if(NPC[A].Inert == false && NPC[A].Type != 159 && NPC[A].Type != 22 && NPC[A].Type != 26 && !(NPC[A].Type == 30 && NPC[A].Projectile == false) && NPC[A].Type != 32 && NPC[A].Type != 35 && !(NPC[A].Type == 40 && NPC[A].Projectile == false) && !((NPC[A].Type == 58 || NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 || NPC[A].Type == 66 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70 || NPC[A].Type == 21) && NPC[A].Projectile == false) && !(NPC[A].Type == 45 && NPC[A].Special == 0) && !(NPC[A].Type == 48 && NPC[A].Projectile == false) && NPC[A].Type != 49 && NPC[A].Type != 46 && NPC[A].Type != 56 && !(NPC[A].Type == 57) && !NPCIsYoshi[NPC[A].Type] && !(NPC[A].Type >= 78 && NPC[A].Type <= 83) && !(NPC[A].Type == 96 && NPC[A].Projectile == false) && !(NPC[A].Type >= 117 && NPC[A].Type <= 120 && NPC[A].Projectile == true && NPC[A].CantHurt > 0) && !(NPCIsAShell[NPC[A].Type] && NPC[A].Projectile == false) && !(NPC[A].Projectile == true && NPC[A].Type >= 117 && NPC[A].Type <= 120) && NPC[A].Type != 133 && !(NPCIsToad[NPC[A].Type] == false && NPC[A].Projectile == false && NPC[A].Location.SpeedX == 0 && (NPC[A].Location.SpeedY == 0 || NPC[A].Location.SpeedY == Physics.NPCGravity)))
                        {
                            if(!NPCIsACoin[NPC[A].Type] && NPC[A].Type != 240 && NPC[A].Type != 212 && NPC[A].Type != 205 && NPC[A].Type != 206 && NPC[A].Type != 207 && NPC[A].Type != 191 && NPC[A].Type != 193 && !(NPCIsCheep[NPC[A].Type] && NPC[A].Special == 2) && !(NPC[A].Generator == true) && NPC[A].Type != 246 && NPC[A].Type != 260 && NPC[A].Type != 276 && NPC[A].Type != 278 && NPC[A].Type != 279 && NPC[A].Type != 282 && NPC[A].Type != 288 && NPC[A].Type != 289)
                            {

                                for(B = 1; B <= numNPCs; B++)
                                {
                                    if(NPC[B].Active == true)
                                    {
                                        if(!NPCIsACoin[NPC[B].Type])
                                        {
                                            if(CheckCollision(NPC[A].Location, NPC[B].Location) == true)
                                            {
                                                if(B != A)
                                                {
                                                    if(!(NPC[B].Type == 15 && NPC[B].Special == 4) && !(NPCIsToad[NPC[B].Type]) && !(NPC[B].Type >= 104 && NPC[B].Type <= 106) && !(NPC[B].Type >= 154 && NPC[B].Type <= 157) && !(NPC[B].Type == 159) && !(NPC[B].Type == 202) && NPCIsAVine[NPC[B].Type] == false && NPC[B].Type != 265 && NPC[B].Type != 260 && NPC[B].Type != 291)
                                                    {
                                                        // If Not (NPC(B).Type = 133) And NPC(B).HoldingPlayer = 0 And .Killed = 0 And NPC(B).JustActivated = 0 And NPC(B).Inert = False And NPC(B).Killed = 0 Then
                                                        if(!(NPC[B].Type == 133) && !(NPCIsVeggie[NPC[B].Type] && NPCIsVeggie[NPC[A].Type]) && NPC[B].HoldingPlayer == 0 && NPC[A].Killed == 0 && NPC[B].JustActivated == 0 && NPC[B].Inert == false && NPC[B].Killed == 0)
                                                        {



                                                            if(NPC[B].Type != 22 && NPC[B].Type != 266 && NPC[B].Type != 49 && NPC[B].Type != 26 && NPC[B].Type != 30 && NPC[B].Type != 31 && NPC[B].Type != 32 && NPC[B].Type != 35 && NPC[B].Type != 56 && !(NPC[B].Type == 50) && !(NPC[B].Type == 57) && !(NPC[B].Type == 58) && NPC[B].Type != 191 && NPC[B].Type != 193 && !(NPC[B].Generator == true) && !((NPC[A].Type == 13 || NPC[A].Type == 265) && NPC[B].Type == 195) && NPC[B].Type != 240 && NPC[B].Type != 278 && NPC[B].Type != 279 && NPC[B].Type != 288 && NPC[B].Type != 289 && NPC[B].Type != 291 && NPC[B].Type != 171 && NPC[B].Type != 292)
                                                            {
                                                                if(!(NPC[B].Type == 67 || NPC[B].Type == 60 || NPC[B].Type == 62 || NPC[B].Type == 64 || NPC[B].Type == 66 || NPC[B].Type == 68 || NPC[B].Type == 69 || NPC[B].Type == 70) && !(NPC[A].Projectile == false && NPC[B].Type == 48) && NPCIsYoshi[NPC[B].Type] == false && NPC[B].Type != 46 && NPC[B].Type != 212 && !(NPC[B].Type == 45 && NPC[B].Special == 0) && !(NPC[B].Type == 57) && !(NPC[B].Type >= 78 && NPC[B].Type <= 83) && !(NPC[B].Type == 84) && !(NPC[B].Type == 85) && !(NPC[B].Type == 17 && NPC[B].CantHurt > 0) && !(NPC[B].Type == 91) && !(NPC[A].CantHurtPlayer == NPC[B].CantHurtPlayer && NPC[A].CantHurtPlayer > 0) && !(NPC[B].Type == 96 && NPC[B].Projectile == false) && !(NPC[B].Type == 108) && !(NPC[B].Type == 246) && NPC[B].Type != 276 && NPC[B].Type != 282 && NPC[B].Type != 225 && NPC[B].Type != 226 && NPC[B].Type != 227)
                                                                {

                                                                    if(NPC[A].Type == 269 || NPC[B].Type == 269 || NPC[A].Type == 282 || NPC[B].Type == 282)
                                                                        HitSpot = 0;

                                                                    if(NPC[A].Type == 283)
                                                                    {
                                                                        NPCHit(A, 3, B);
                                                                        HitSpot = 0;
                                                                    }
                                                                    else if(NPC[B].Type == 283)
                                                                        NPCHit(B, 3, A);


                                                                    if(NPC[A].Type == 266)
                                                                    {
                                                                        if(NPCIsABonus[NPC[B].Type] == false)
                                                                            NPCHit(B, 10, NPC[A].CantHurtPlayer);
                                                                        HitSpot = 0;
                                                                    }

                                                                    // toad code
                                                                    if(NPCIsToad[NPC[A].Type] == true)
                                                                    {
                                                                        if(!(NPCWontHurt[NPC[B].Type] && NPC[B].Projectile == false) && !NPCIsABonus[NPC[B].Type] && !(NPC[B].Type == 13) && !(NPC[B].Type == 265) && !(NPC[B].Type == 17 && NPC[B].CantHurt > 0) && !(NPC[B].Type == 50) && !(NPC[B].Type == 171) && !(NPC[B].Type == 292) && !(NPC[B].Type == 195))
                                                                        {
                                                                            NPCHit(A, 3, B);
                                                                            HitSpot = 0;
                                                                        }
                                                                    }
                                                                    // Koopa Code
                                                                    if((NPC[A].Type == 117 || NPC[A].Type == 118 || NPC[A].Type == 120) && NPC[A].Projectile == false && (NPC[B].Projectile == false && NPC[B].Type >= 113 && NPC[B].Type <= 116))
                                                                    {
                                                                        tempLocation = NPC[A].Location;
                                                                        tempLocation2 = NPC[B].Location;
                                                                        tempLocation.Width = 8;
                                                                        tempLocation.X = tempLocation.X + 12;
                                                                        tempLocation2.Width = 8;
                                                                        tempLocation2.X = tempLocation2.X + 12;
                                                                        if(CheckCollision(tempLocation, tempLocation2))
                                                                        {
                                                                            NPC[B].Type = NPC[B].Type - 4;
                                                                            if(NPC[B].Type == 112)
                                                                                NPC[B].Type = 194;
                                                                            NPC[A].Killed = 9;
                                                                            NPC[B].Direction = NPC[A].Direction;
                                                                            NPC[B].Frame = EditorNPCFrame(NPC[B].Type, NPC[B].Direction);
                                                                        }
                                                                        // NPC is a projectile
                                                                    }
                                                                    else if(NPC[A].Projectile && !(NPC[B].Type == 45 && NPC[B].Special == 0.0) && NPC[A].Type != 266)
                                                                    {
                                                                        if(!(NPC[A].Projectile && NPC[B].Projectile && NPC[A].Type == 17 && NPC[B].Type == 17 && NPC[A].CantHurtPlayer != NPC[B].CantHurtPlayer))
                                                                        {
                                                                            if(!((NPC[A].Type == 13 && NPCIsABonus[NPC[B].Type]) || NPC[B].Type == 13 || NPC[B].Type == 87))
                                                                            {
                                                                                if(NPCIsAShell[NPC[A].Type] &&
                                                                                        (NPC[B].Type == 55 || NPC[B].Type == 119) &&
                                                                                        (int(NPC[A].Direction) != int(NPC[B].Direction) || NPC[A].Special > 0) && !NPC[B].Projectile)
                                                                                {
                                                                                    if(int(NPC[A].Direction) == -1)
                                                                                    {
                                                                                        NPC[B].Frame = 3;
                                                                                        if(NPC[B].Type == 119)
                                                                                            NPC[B].Frame = 5;
                                                                                        NPC[B].FrameCount = 0;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        NPC[B].Frame = 0;
                                                                                        NPC[B].FrameCount = 0;
                                                                                    }
                                                                                    if(NPC[A].CantHurt < 25)
                                                                                        NPC[A].Special = 1;
                                                                                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > NPC[B].Location.X + NPC[B].Location.Width / 2.0)
                                                                                    {
                                                                                        NPC[B].Location.X = NPC[A].Location.X - NPC[B].Location.Width - 1;
                                                                                        NPC[B].Direction = 1;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        NPC[B].Location.X = NPC[A].Location.X + NPC[A].Location.Width + 1;
                                                                                        NPC[B].Direction = -1;
                                                                                    }
                                                                                    if(NPC[A].Location.SpeedY < NPC[B].Location.SpeedY)
                                                                                        NPC[A].Location.SpeedY = NPC[B].Location.SpeedY;
                                                                                    NPC[A].Frame = 0;
                                                                                    NPC[A].FrameCount = 0;
                                                                                    if(NPC[A].CantHurt < 25)
                                                                                        NPC[A].Special = 2;
                                                                                    NPC[B].Special = 0;
                                                                                    tempLocation = NPC[B].Location;
                                                                                    tempLocation.Y += 1;
                                                                                    tempLocation.Height -= 2;
                                                                                    for(bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                                                                    {
                                                                                        if(bCheck2 == 1)
                                                                                        {
                                                                                            fBlock2 = FirstBlock[(NPC[B].Location.X / 32) - 1];
                                                                                            lBlock2 = LastBlock[((NPC[B].Location.X + NPC[B].Location.Width) / 32.0) + 1];
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            fBlock2 = numBlock - numTempBlock;
                                                                                            lBlock2 = numBlock;
                                                                                        }

                                                                                        for(auto C = fBlock2; C <= lBlock2; C++)
                                                                                        {
                                                                                            if(!BlockIsSizable[Block[C].Type] && !BlockOnlyHitspot1[Block[C].Type] && Block[C].Hidden == false && BlockSlope[Block[C].Type] == 0)
                                                                                            {
                                                                                                if(CheckCollision(tempLocation, Block[C].Location) == true)
                                                                                                {
                                                                                                    if(int(NPC[A].Direction) == -1)
                                                                                                    {
                                                                                                        NPC[B].Location.X = Block[C].Location.X + Block[C].Location.Width + 0.1;
                                                                                                        NPC[A].Location.X = NPC[B].Location.X + NPC[B].Location.Width + 0.1;
                                                                                                    }
                                                                                                    else
                                                                                                    {
                                                                                                        NPC[B].Location.X = Block[C].Location.X - NPC[B].Location.Width - 0.1;
                                                                                                        NPC[A].Location.X = NPC[B].Location.X - NPC[A].Location.Width - 0.1;
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                                else if(NPC[A].Type == 78)
                                                                                    NPCHit(B, 8, A);
                                                                                else
                                                                                {
                                                                                    if(!NPCIsABonus[NPC[B].Type])
                                                                                    {
                                                                                        if(NPC[A].Type == 21 && NPC[B].Type == 17)
                                                                                            NPC[B].Projectile = true;
                                                                                        else
                                                                                        {
                                                                                            tempBool = false; // This whole cluster stops friendly projectiles form killing riddin shells
                                                                                            if(NPCIsAShell[NPC[A].Type] == true)
                                                                                            {
                                                                                                for(auto C = 1; C <= numPlayers; C++)
                                                                                                {
                                                                                                    if(Player[C].StandingOnNPC == A && NPC[B].CantHurtPlayer == C)
                                                                                                    {
                                                                                                        tempBool = true;
                                                                                                        break;
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                            if(NPCIsAShell[NPC[B].Type] == true)
                                                                                            {
                                                                                                for(auto C = 1; C <= numPlayers; C++)
                                                                                                {
                                                                                                    if(Player[C].StandingOnNPC == B && NPC[A].CantHurtPlayer == C)
                                                                                                    {
                                                                                                        tempBool = true;
                                                                                                        break;
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                            if(!(NPC[A].Type == 17 && NPC[A].Projectile == true))
                                                                                            {
                                                                                                if(NPCIsAShell[NPC[B].Type] && NPC[B].Projectile == true)
                                                                                                {
                                                                                                    if(tempBool == false)
                                                                                                        NPCHit(A, 3, B);
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    if(tempBool == false)
                                                                                                        NPCHit(A, 4, B);
                                                                                                }
                                                                                            }
                                                                                            if(tempBool == false) // end cluster
                                                                                                NPCHit(B, 3, A);
                                                                                            if(NPC[A].Type == 17)
                                                                                            {
                                                                                                if(NPC[B].Type == 15)
                                                                                                {
                                                                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                                                    NPCHit(A, 4, B);
                                                                                                }
                                                                                                else if(NPC[B].Type == 21)
                                                                                                {
                                                                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                                                    PlaySound(3);
                                                                                                    NPCHit(A, 4, A);
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    else if(!(NPC[B].Type == 40 && NPC[B].Projectile == false))
                                                                    {
                                                                        HitSpot = FindCollision(NPC[A].Location, NPC[B].Location);
                                                                        if(NPCIsToad[NPC[A].Type] && NPC[A].Killed > 0)
                                                                            HitSpot = 0;
                                                                        if(NPCIsAParaTroopa[NPC[A].Type] && NPCIsAParaTroopa[NPC[B].Type])
                                                                        {
                                                                            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > NPC[B].Location.X + NPC[B].Location.Width / 2.0)
                                                                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.05;
                                                                            else
                                                                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.05;
                                                                            if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 > NPC[B].Location.Y + NPC[B].Location.Height / 2.0)
                                                                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + 0.05;
                                                                            else
                                                                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedY - 0.05;
                                                                            HitSpot = 0;
                                                                        }

                                                                        if(NPC[B].Projectile == false && NPCNoClipping[NPC[A].Type] == false && NPCNoClipping[NPC[B].Type] == false)
                                                                        {
                                                                            if(((NPC[A].Type == 55 || NPC[A].Type == 119) && NPCIsAShell[NPC[B].Type]) || ((NPC[B].Type == 55 || NPC[B].Type == 119) && NPCIsAShell[NPC[A].Type])) // Nekkid koopa kicking a shell
                                                                            {
                                                                                if(NPC[A].Type == 55 || NPC[A].Type == 119)
                                                                                {
                                                                                    if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
                                                                                    {
                                                                                        // If .Direction = 1 And .Location.X + .Location.Width < NPC(B).Location.X + 3 Or (.Direction = -1 And .Location.X > NPC(B).Location.X + NPC(B).Location.Width - 3) Then
                                                                                        if((NPC[A].Direction == 1  && NPC[A].Location.X + NPC[A].Location.Width < NPC[B].Location.X + 4) ||
                                                                                           (NPC[A].Direction == -1 && NPC[A].Location.X > NPC[B].Location.X + NPC[B].Location.Width - 4))
                                                                                        {
                                                                                            if(NPC[B].Location.SpeedX == 0.0 && NPC[B].Effect == 0)
                                                                                            {
                                                                                                NPC[A].Special = 10;
                                                                                                Player[numPlayers + 1].Direction = NPC[A].Direction;
                                                                                                NPC[A].Location.X = NPC[A].Location.X - NPC[A].Direction;
                                                                                                NPCHit(B, 1, numPlayers + 1);
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                            else if((HitSpot == 2 || HitSpot == 4) && NPC[A].Type != 179 && NPC[B].Type != 179)
                                                                            {
                                                                                NPC[A].onWall = true;
                                                                                if(NPC[A].Direction == NPC[B].Direction)
                                                                                {
                                                                                    if(NPC[A].Location.SpeedX * NPC[A].Direction > NPC[B].Location.SpeedX * NPC[B].Direction)
                                                                                    {
                                                                                        if(NPC[A].Type != 17 && NPC[A].Type != 13 && NPC[A].Type != 265)
                                                                                            NPC[A].TurnAround = true;
                                                                                    }
                                                                                    else if(NPC[A].Location.SpeedX * NPC[A].Direction < NPC[B].Location.SpeedX * NPC[B].Direction)
                                                                                        NPC[B].TurnAround = true;
                                                                                    else
                                                                                    {
                                                                                        NPC[A].TurnAround = true;
                                                                                        NPC[B].TurnAround = true;
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    if(NPC[A].Type != 17 && NPC[A].Type != 13 && NPC[A].Type != 265)
                                                                                        NPC[A].TurnAround = true;
                                                                                    NPC[B].TurnAround = true;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if(NPC[A].WallDeath > 0)
                        {
                            if(NPCIsCheep[NPC[A].Type] == true)
                                NPC[A].WallDeath = NPC[A].WallDeath - 1;
                            else
                                NPC[A].WallDeath = 0;
                        }
                        if(tempHit != 0) // Walking
                        {
                            if(NPC[A].Type == 3) // Walking code for Flying Goomba
                            {
                                if(NPC[A].Special <= 30)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                    if(tempSpeedA != 0)
                                        NPC[A].Location.SpeedY = tempSpeedA;
                                }
                                else if(NPC[A].Special == 31)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -4;
                                }
                                else if(NPC[A].Special == 32)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -4;
                                }
                                else if(NPC[A].Special == 33)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -4;
                                }
                                else if(NPC[A].Special == 34)
                                {
                                    NPC[A].Special = 0;
                                    NPC[A].Location.SpeedY = -7;
                                }
                            }
                            else if(NPC[A].Type == 167) // Walking code for SMW Flying Goomba
                            {
                                if(NPC[A].Special <= 60)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                    if(tempSpeedA != 0)
                                        NPC[A].Location.SpeedY = tempSpeedA;
                                }
                                else if(NPC[A].Special == 61)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 62)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 63)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 64)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 65)
                                {
                                    NPC[A].Special = 0;
                                    NPC[A].Location.SpeedY = -7;
                                }
                            }
                            else if(NPCTurnsAtCliffs[NPC[A].Type] == true && NPC[A].Projectile == false) // Walking code NPCs that turn
                            {
                                tempTurn = true;
                                tempLocation = NPC[A].Location;
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 8;
                                tempLocation.Height = 16;
                                if(NPC[A].Slope > 0)
                                    tempLocation.Height = 32;
                                tempLocation.Width = 16;
                                // If .Location.SpeedX > 0 Then
                                if(NPC[A].Direction > 0)
                                {
                                    tempLocation.X = tempLocation.X + NPC[A].Location.Width - 20;
                                    if(NPC[A].Type == 247 && NPC[A].Special2 == 0)
                                        tempLocation.X = tempLocation.X + 16;

                                    // If .Type = 189 Then tempLocation.X = tempLocation.X - 10
                                }
                                else
                                {
                                    tempLocation.X = tempLocation.X - tempLocation.Width + 20;
                                    if(NPC[A].Type == 247 && NPC[A].Special2 == 0)
                                        tempLocation.X = tempLocation.X - 16;

                                    // If .Type = 189 Then tempLocation.X = tempLocation.X + 10
                                }
                                for(bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                {
                                    if(bCheck2 == 1)
                                    {
                                        fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
                                        lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    }
                                    else
                                    {
                                        fBlock2 = numBlock - numTempBlock;
                                        lBlock2 = numBlock;
                                    }
                                    for(B = (int)fBlock2; B <= lBlock2; B++)
                                    {
                                        //                                            If BlockNoClipping(Block(B).Type) = False And Block(B).Invis = False And Block(B).Hidden = False And Not (BlockIsSizable(Block(B).Type) And Block(B).Location.Y < .Location.Y + .Location.Height - 3) Then


                                        if(tempLocation.X + tempLocation.Width >= Block[B].Location.X)
                                        {
                                            if(tempLocation.X <= Block[B].Location.X + Block[B].Location.Width)
                                            {
                                                if(tempLocation.Y + tempLocation.Height >= Block[B].Location.Y)
                                                {
                                                    if(tempLocation.Y <= Block[B].Location.Y + Block[B].Location.Height)
                                                    {
                                                        if(BlockNoClipping[Block[B].Type] == false && Block[B].Invis == false && Block[B].Hidden == false && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 3))
                                                        {
                                                            // If CheckCollision(tempLocation, Block(B).Location) = True Then
                                                            tempTurn = false;
                                                            break;
                                                            // End If
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // not working
                                        }




                                        // End If
                                    }
                                }
                                if(tempTurn == true)
                                    NPC[A].TurnAround = true;
                                NPC[A].Location.SpeedY = 0;
                                if(NPC[A].Slope > 0)
                                {
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                    if(NPC[A].Location.SpeedY < 0)
                                        NPC[A].Location.SpeedY = 0;
                                }
                                if(tempSpeedA != 0)
                                    NPC[A].Location.SpeedY = tempSpeedA;
                            }
                            else if(NPC[A].Type == 77) // ninja code
                            {
                                tempTurn = true;
                                tempLocation = NPC[A].Location;
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 8;
                                tempLocation.Height = 16;
                                if(NPC[A].Slope > 0)
                                    tempLocation.Height = 32;
                                tempLocation.Width = 16;
                                if(NPC[A].Location.SpeedX > 0)
                                    tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width - 16;
                                else
                                    tempLocation.X = NPC[A].Location.X - tempLocation.Width + 16;
                                for(bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                {
                                    if(bCheck2 == 1)
                                    {
                                        fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
                                        lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    }
                                    else
                                    {
                                        fBlock2 = numBlock - numTempBlock;
                                        lBlock2 = numBlock;
                                    }
                                    for(B = (int)fBlock2; B <= lBlock2; B++)
                                    {
                                        if(BlockNoClipping[Block[B].Type] == false && Block[B].Invis == false && Block[B].Hidden == false && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 3))
                                        {
                                            if(CheckCollision(tempLocation, Block[B].Location) == true)
                                            {
                                                tempTurn = false;
                                                break;
                                            }
                                        }
                                    }
                                }
                                tempLocation = NPC[A].Location;
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.Y = NPC[A].Location.Y + 8;
                                tempLocation.Height = tempLocation.Height - 16;
                                tempLocation.Width = 32;
                                if(NPC[A].Location.SpeedX > 0)
                                    tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width;
                                else
                                    tempLocation.X = NPC[A].Location.X - tempLocation.Width;
                                for(bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                {
                                    if(bCheck2 == 1)
                                    {
                                        fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
                                        lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    }
                                    else
                                    {
                                        fBlock2 = numBlock - numTempBlock;
                                        lBlock2 = numBlock;
                                    }
                                    for(B = (int)fBlock2; B <= lBlock2; B++)
                                    {
                                        if(BlockNoClipping[Block[B].Type] == false && Block[B].Invis == false && Block[B].Hidden == false && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 1))
                                        {
                                            if(CheckCollision(tempLocation, Block[B].Location) == true)
                                            {
                                                if(NPC[A].Slope > 0)
                                                {

                                                }
                                                else if(BlockSlope[Block[B].Type] == 0)
                                                    tempTurn = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                                if(tempTurn == true)
                                {
                                    NPC[A].Location.Y = NPC[A].Location.Y - 0.1;
                                    NPC[A].Location.SpeedY = -6.55;
                                }
                                else
                                {
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                }
                                if(tempSpeedA != 0)
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY + tempSpeedA;
                            }
                            else // Walking code for everything else
                            {
                                if(NPCIsAParaTroopa[NPC[A].Type])
                                {
                                    if(NPC[A].Special == 1)
                                        NPC[A].Location.SpeedY = -9;
                                    else
                                    {
                                        if(NPC[A].Location.SpeedY > 0)
                                            NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY;
                                    }
                                }
                                else
                                {
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0 && NPCIsAShell[NPC[A].Type] == false && NPC[A].Type != 45)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                }
                                if(NPC[A].Type == 129)
                                {
                                    NPC[A].Special = NPC[A].Special + 1;
                                    if(NPC[A].Special <= 3)
                                        NPC[A].Location.SpeedY = -3.5;
                                    else
                                    {
                                        NPC[A].Location.SpeedY = -5.5;
                                        NPC[A].Special = 0;
                                    }
                                }
                                if(NPC[A].Type == 125)
                                {
                                    NPC[A].FrameCount = NPC[A].FrameCount + 1;
                                    if(NPC[A].FrameCount > 1)
                                        NPC[A].FrameCount = 0;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                if(tempSpeedA != 0)
                                    NPC[A].Location.SpeedY = tempSpeedA;
                                if(NPC[A].Type == 179)
                                    NPC[A].Special = NPC[A].Location.SpeedY;

                            }
                            if(NPC[A].Slope == 0)
                                NPC[A].Location.Y = tempHit;
                            tempHit = 0;
                            if(Block[tempHitBlock].IsNPC > 0 && NPC[Block[tempHitBlock].IsReally].Slope > 0)
                            {
                                // .Location.SpeedY = 0
                                NPC[A].Slope = NPC[Block[tempHitBlock].IsReally].Slope;
                                // Stop
                            }
                        }
                    }
                }
                if(NPC[A].tempBlock > 0 && (NPC[A].Type < 60 || NPC[A].Type > 66) && NPC[A].Type != 57)
                {
                    if((NPC[A].Type < 78 || NPC[A].Type > 83) && NPC[A].Type != 26)
                    {
                        Block[NPC[A].tempBlock].Location = NPC[A].Location;
                        if(NPC[A].Type == 26)
                        {
                            Block[NPC[A].tempBlock].Location.Y = Block[NPC[A].tempBlock].Location.Y - 16;
                            Block[NPC[A].tempBlock].Location.Height = Block[NPC[A].tempBlock].Location.Height + 16;
                        }
                        while(Block[NPC[A].tempBlock].Location.X < Block[NPC[A].tempBlock - 1].Location.X && NPC[A].tempBlock > numBlock + 1 - numTempBlock)
                        {

                            tmpBlock = Block[NPC[A].tempBlock - 1];
                            Block[NPC[A].tempBlock - 1] = Block[NPC[A].tempBlock];
                            Block[NPC[A].tempBlock] = tmpBlock;

                            NPC[Block[NPC[A].tempBlock].IsReally].tempBlock = NPC[A].tempBlock;
                            NPC[A].tempBlock = NPC[A].tempBlock - 1;

                        }
                        while(Block[NPC[A].tempBlock].Location.X > Block[NPC[A].tempBlock + 1].Location.X && NPC[A].tempBlock < numBlock)
                        {


                            tmpBlock = Block[NPC[A].tempBlock + 1];
                            Block[NPC[A].tempBlock + 1] = Block[NPC[A].tempBlock];
                            Block[NPC[A].tempBlock] = tmpBlock;

                            NPC[Block[NPC[A].tempBlock].IsReally].tempBlock = NPC[A].tempBlock;
                            NPC[A].tempBlock = NPC[A].tempBlock + 1;




                            // NPC(Block(.tempBlock).IsReally).tempBlock = .tempBlock
                            // NPC(Block(.tempBlock + 1).IsReally).tempBlock = .tempBlock + 1


                        }
                    }
                    Block[NPC[A].tempBlock].Location.SpeedX = NPC[A].Location.SpeedX + NPC[A].BeltSpeed;
                }
                if(NPC[A].Projectile == true)
                {
                    if(NPC[A].Type == 179 || NPC[A].Type == 58 || NPC[A].Type == 21 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 || NPC[A].Type == 70 || (NPC[A].Type >= 78 && NPC[A].Type <= 83))
                        NPC[A].Projectile = false;
                }
                // Pinched code
                // If .Direction <> oldDirection Then
                // .PinchCount = .PinchCount + 10
                // Else
                // If .PinchCount > 0 Then
                // .PinchCount = .PinchCount - 1
                // If .Pinched = False Then .PinchCount = .PinchCount - 1
                // End If
                // End If
                // If .PinchCount >= 14 And .Pinched = False Then
                // .Pinched = True
                // .PinchedDirection = .Direction
                // ElseIf .PinchCount >= 15 Then
                // .PinchCount = 15
                // ElseIf .PinchCount = 0 Then
                // .Pinched = False
                // End If
                NPC[A].Pinched = false;
                // Special Code for things that work while held
                if(NPC[A].Type == 134) // SMB2 Bomb
                {
                    // If .Location.SpeedX < -2 Or .Location.SpeedX > 2 Or .Location.SpeedY < -2 Or .Location.SpeedY > 5 Then .Projectile = True
                    NPC[A].Special = NPC[A].Special + 1;
                    if(NPC[A].Special > 250)
                        NPC[A].Special2 = 1;
                    if(NPC[A].Special >= 350 || NPC[A].Special < 0)
                    {
                        Bomb(NPC[A].Location, 2);
                        NPC[A].Killed = 9;
                    }
                }
                else if(NPC[A].Type == 135) // SMB2 Bob-om
                {
                    NPC[A].Special = NPC[A].Special + 1;
                    if(NPC[A].Special > 450)
                        NPC[A].Special2 = 1;
                    if(NPC[A].Special >= 550 || NPC[A].Special < 0)
                    {
                        Bomb(NPC[A].Location, 2);
                        NPC[A].Killed = 9;
                    }
                }
                else if(NPC[A].Type == 137) // SMB3 Bomb
                {
                    if(NPC[A].Inert == false)
                        NPC[A].Special = NPC[A].Special + 1;
                    if(NPC[A].Special > 250)
                        NPC[A].Special2 = 1;
                    if(NPC[A].Special >= 350 || NPC[A].Special < 0)
                        Bomb(NPC[A].Location, 3);
                }
                else if(NPC[A].Type == 189)
                {
                    if(NPC[A].Special > 0)
                    {
                        NPC[A].Special2 = NPC[A].Special2 + 1;
                        if(NPC[A].Special2 >= 400 && NPC[A].Special3 == 0)
                        {
                            NPC[A].Special = 0;
                            NPC[A].Special2 = 0;
                            NPC[A].Inert = false;
                            NPC[A].Stuck = false;
                        }
                        else if(NPC[A].Special2 >= 300)
                        {
                            if(NPC[A].Special3 == 0)
                            {
                                NPC[A].Location.X = NPC[A].Location.X + 2;
                                NPC[A].Special3 = 1;
                            }
                            else
                            {
                                NPC[A].Location.X = NPC[A].Location.X - 2;
                                NPC[A].Special3 = 0;
                            }
                        }
                    }
                }
                else if(NPC[A].Type == 86) // smb3 bowser
                {
                    // special5 - the player
                    // special4 - what bowser is doing
                    // special3 - counter for what bowser is doing
                    // special2 - counter for what bowser needs to do
                    if(NPC[A].Legacy)
                    {
                        if(NPC[A].TimeLeft > 1)
                        {
                            NPC[A].TimeLeft = 100;
                            if(bgMusic[NPC[A].Section] != 21)
                            {
                                StopMusic();
                                bgMusic[NPC[A].Section] = 21;
                                StartMusic(NPC[A].Section);
                            }
                        }
                    }

                    if(NPC[A].Special4 == 0.0)
                    {
                        NPC[A].Special3 = 0; // reset counter when done
                        if(NPC[A].Direction < 0)
                            NPC[A].Frame = 0;
                        else
                            NPC[A].Frame = 5;
                    }

                    if(NPC[A].Special5 == 0.0) // find player
                    {
                        tempBool = false;
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                                tempBool = true;
                        }

                        if(!tempBool)
                        {
                            NPC[A].Special5 = 0;
                            NPC[A].Special4 = 2;
                        }
                        else
                        {
                            do
                                B = static_cast<int>(floor(static_cast<double>(dRand() * numPlayers))) + 1;
                            while(Player[B].Dead == true || Player[B].TimeToLive > 0);
                            NPC[A].Special5 = B;
                        }
                    }

                    // see if facing the player
                    tempBool = false;
                    if(NPC[A].Special5 > 0)
                    {
                        if(Player[long(NPC[A].Special5)].Location.X + Player[long(NPC[A].Special5)].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                        {
                            if(NPC[A].Direction < 0)
                                tempBool = true;
                        }
                        else
                        {
                            if(NPC[A].Direction > 0)
                                tempBool = true;
                        }
                    }

                    NPC[A].Special2 = NPC[A].Special2 + dRand();
                    if(NPC[A].Special4 == 0.0 && tempBool == true)
                    {
                        if(NPC[A].Special2 >= 200 + dRand() * 400 - dRand() * 200) // hop on player
                        {
                            if(NPC[A].Special2 >= 200 + dRand() * 600)
                                NPC[A].Special2 = 0;
                            NPC[A].Special4 = 3;
                        }
                        else if((NPC[A].Special2 >= 80 && NPC[A].Special2 <= 130) || (NPC[A].Special2 >= 160 + dRand() * 300 && NPC[A].Special2 <= 180 + dRand() * 800)) // shoot fireball
                            NPC[A].Special4 = 4;
                    }

                    if(NPC[A].Inert)
                    {
                        if(fEqual(NPC[A].Special4, 4) || fEqual(NPC[A].Special4, 3))
                            NPC[A].Special4 = 0;
                    }

                    if(NPC[A].Special4 == 0.0) // when not doing anything turn to player
                    {
                        if(!tempBool)
                        {
                            if(NPC[A].Direction > 0)
                                NPC[A].Special4 = -1;
                            if(NPC[A].Direction < 0)
                                NPC[A].Special4 = 1;
                        }
                    }

                    if(NPC[A].Special4 == 0.0) // hop
                        NPC[A].Special4 = 2;

                    if(NPC[A].HoldingPlayer > 0)
                    {
                        if(fEqual(NPC[A].Direction, -1))
                            NPC[A].Frame = 0;
                        else
                            NPC[A].Frame = 5;
                        NPC[A].Special4 = 9000;
                    }
                    else if(fEqual(NPC[A].Special4, 9000))
                    {
                        NPC[A].Special5 = NPC[A].CantHurtPlayer;
                        NPC[A].Special4 = 0;
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                    }

                    if(fEqual(NPC[A].Special4, -1)) // turn left
                    {
                        NPC[A].Special3 = NPC[A].Special3 - 1;
                        if(NPC[A].Special3 > -5)
                            NPC[A].Frame = 9;
                        else if(NPC[A].Special3 > -10)
                            NPC[A].Frame = 8;
                        else if(NPC[A].Special3 > -15)
                            NPC[A].Frame = 12;
                        else if(NPC[A].Special3 > -20)
                            NPC[A].Frame = 3;
                        else if(NPC[A].Special3 > -25)
                            NPC[A].Frame = 4;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = -1;
                        }
                    }
                    else if(fEqual(NPC[A].Special4, 1)) // turn right
                    {
                        NPC[A].Special3 = NPC[A].Special3 + 1;
                        if(NPC[A].Special3 < 5)
                            NPC[A].Frame = 4;
                        else if(NPC[A].Special3 < 10)
                            NPC[A].Frame = 3;
                        else if(NPC[A].Special3 < 15)
                            NPC[A].Frame = 12;
                        else if(NPC[A].Special3 < 20)
                            NPC[A].Frame = 8;
                        else if(NPC[A].Special3 < 25)
                            NPC[A].Frame = 9;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = 1;
                        }
                    }
                    else if(fEqual(NPC[A].Special4, -10)) // look left
                    {
                        NPC[A].Special3 = NPC[A].Special3 - 1;
                        if(NPC[A].Special3 > -5)
                            NPC[A].Frame = 3;
                        else if(NPC[A].Special3 > -10)
                            NPC[A].Frame = 4;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = -1;
                        }
                    }
                    else if(fEqual(NPC[A].Special4, 10)) // look right
                    {
                        NPC[A].Special3 = NPC[A].Special3 + 1;
                        if(NPC[A].Special3 < 5)
                            NPC[A].Frame = 8;
                        else if(NPC[A].Special3 < 10)
                            NPC[A].Frame = 9;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = 1;
                        }
                    }
                    else if(fEqual(NPC[A].Special4, 2)) // hops
                    {
                        if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                        {
                            if(NPC[A].Special3 < 5)
                            {
                                NPC[A].Special3 = NPC[A].Special3 + 1;
                                if(NPC[A].Direction < 0)
                                    NPC[A].Frame = 1;
                                else
                                    NPC[A].Frame = 6;
                            }
                            else if(fEqual(NPC[A].Special3, 5))
                            {
                                NPC[A].Special3 = NPC[A].Special3 + 1;
                                NPC[A].Location.SpeedY = -3;
                                NPC[A].Location.Y = NPC[A].Location.Y - 0.1;
                                if(NPC[A].Direction < 0)
                                    NPC[A].Frame = 0;
                                else
                                    NPC[A].Frame = 5;
                            }
                            else if(NPC[A].Special3 < 10)
                            {
                                NPC[A].Special3 = NPC[A].Special3 + 1;
                                if(NPC[A].Direction < 0)
                                    NPC[A].Frame = 1;
                                else
                                    NPC[A].Frame = 6;
                            }
                            else
                                NPC[A].Special4 = 0;
                        }
                    }
                    else if(fEqual(NPC[A].Special4, 3)) // jump on player
                    {
                        if(NPC[A].Special3 < -1)
                        {
                            if(NPC[A].Special > 1)
                                NPC[A].Special = NPC[A].Special - 1;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                            if(fEqual(NPC[A].Special3, -1))
                                NPC[A].Special3 = 6;
                        }
                        else if(NPC[A].Special3 < 5)
                        {
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 1;
                            else
                                NPC[A].Frame = 6;
                        }
                        else if(fEqual(NPC[A].Special3, 5))
                        {
                            auto &sx = NPC[A].Location.SpeedX;
                            auto &pl = Player[int(NPC[A].Special5)].Location;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                            NPC[A].Location.SpeedY = -12;
                            NPC[A].BeltSpeed = 0;
                            NPC[A].Location.Y = NPC[A].Location.Y - 0.1;
                            // This formula got been compacted: If something will glitch, feel free to restore back this crap
                            //NPC[A].Location.SpeedX = (static_cast<int>(std::floor(static_cast<double>(((Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 - 16) + 1) / 32))) * 32 + 1 - NPC[A].Location.X) / 50;
                            double pCenter = pl.X + pl.Width / 2.0;
                            sx = std::floor((pCenter - 16.0 + 1.0) / 32.0) * 32.0 + 1.0;
                            sx -= NPC[A].Location.X;
                            sx /= 50;
                            if(sx > 15)
                                sx = 15;
                            else if(sx < -15)
                                sx = -15;
                            NPC[A].Special3 = -50;
                            NPC[A].Special = 10;
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 0;
                            else
                                NPC[A].Frame = 5;
                        }
                        else if(fEqual(NPC[A].Special3, 6))
                        {
                            if(NPC[A].Location.SpeedY > 0)
                            {
                                NPC[A].Frame = 10;
                                NPC[A].Location.SpeedX = 0;
                                NPC[A].Location.SpeedY = 0;
                                NPC[A].Special3 = 10;
                                NPC[A].Projectile = true;
                            }
                        }
                        else if(NPC[A].Special3 < 13)
                        {
                            NPC[A].Location.SpeedY = -2;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                        }
                        else if(NPC[A].Special3 < 16)
                        {
                            NPC[A].Location.SpeedY = 2;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                        }
                        else if(NPC[A].Special3 < 19)
                        {
                            NPC[A].Location.SpeedY = -2;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                        }
                        else if(NPC[A].Special3 < 21)
                        {
                            NPC[A].Location.SpeedY = 2;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                        }
                        else if(fEqual(NPC[A].Special3, 21))
                        {
                            if(NPC[A].Location.SpeedY != 0.0)
                                NPC[A].Location.SpeedY = 10;
                            else
                            {
                                PlaySound(37);
                                NPC[A].Special3 = 30;
                                NPC[A].Frame = 11;
                                NPC[A].Projectile = false;
                                tempLocation = NPC[A].Location;
                                tempLocation.X = tempLocation.X; // + 16
                                tempLocation.Width = tempLocation.Width; // - 32
                                tempLocation.Y = tempLocation.Y + tempLocation.Height - 8;
                                tempLocation.Height = 16;
                                fBlock = FirstBlock[long(NPC[A].Location.X / 32) - 1];
                                lBlock = LastBlock[long((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];

                                for(B = (int)fBlock; B <= lBlock; B++)
                                {
                                    if(Block[B].Type == 186 && CheckCollision(tempLocation, Block[B].Location) && !Block[B].Hidden)
                                        KillBlock(B);
                                }

                                if(NPC[A].Legacy && fEqual(NPC[A].Special7, 1.0)) // Classic SMBX 1.0's behavior when Bowser stomps a floor
                                {
                                    fBlock = FirstBlock[long(level[NPC[A].Section].X / 32) - 1];
                                    lBlock = LastBlock[long((level[NPC[A].Section].Width) / 32.0) + 2];

                                    // Shake all blocks up
                                    for(int B = (int)fBlock; B <= lBlock; B++)
                                        BlockShakeUp(B);

                                    // expand down a section at the bottom of destroyed blocks
                                    for(int B = 0; B <= numSections; B++)
                                    {
                                        auto &n = NPC[A];
                                        auto &s = level[B];

                                        if(n.Location.X >= s.X &&
                                           n.Location.X + n.Location.Width <= s.Width &&
                                           n.Location.Y + n.Location.Height + 48 >= s.Y &&
                                           n.Location.Y + n.Location.Height + 48 <= s.Height &&
                                           B != n.Section)
                                        {
                                            n.Special2 = 0;
                                            n.Special3 = 0;
                                            n.Special4 = 2;

                                            auto &ns = level[n.Section];
                                            if(s.X < ns.X)
                                                ns.X = s.X;
                                            if(s.Y < ns.Y)
                                                ns.Y = s.Y;
                                            if(s.Width > ns.Width)
                                                ns.Width = s.Width;
                                            if(s.Height > ns.Height)
                                                ns.Height = s.Height;

                                            s.X = 0;
                                            s.Y = 0;
                                            s.Width = 0;
                                            s.Height = 0;

                                            for(int C = 1; C <= numNPCs; C++)
                                            {
                                                auto &nc = NPC[C];
                                                if(nc.Section == B)
                                                    nc.Section = n.Section;
                                            }
                                        }
                                    } // for
                                    SoundPause[2] = 12;
                                }
                            }
                        }
                        else if(NPC[A].Special3 < 35)
                        {
                            NPC[A].Frame = 11;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                        }
                        else if(NPC[A].Special3 < 40)
                        {
                            NPC[A].Frame = 12;
                            NPC[A].Special3 = NPC[A].Special3 + 1;
                            NPC[A].Special5 = 0;
                        }
                        else
                        {
                            if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                            {
                                NPC[A].Special3 = 0;
                                if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                                    NPC[A].Special4 = -10;
                                else
                                    NPC[A].Special4 = 10;
                            }
                        }
                    }
                    else if(fEqual(NPC[A].Special4, 4)) // shoot a fireball
                    {
                        NPC[A].Special3 = NPC[A].Special3 + 1;
                        if(NPC[A].Special3 < 15)
                        {
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 13;
                            else
                                NPC[A].Frame = 14;
                        }
                        else if(NPC[A].Special3 < 30)
                        {
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 2;
                            else
                                NPC[A].Frame = 7;
                            if(fEqual(NPC[A].Special3, 29))
                            {
                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                NPC[numNPCs].Active = true;
                                NPC[numNPCs].TimeLeft = 100;
                                NPC[numNPCs].Direction = NPC[A].Direction;
                                NPC[numNPCs].Section = NPC[A].Section;
                                NPC[numNPCs].Type = 87;
                                if(NPC[numNPCs].Direction > 0)
                                    NPC[numNPCs].Frame = 4;
                                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                                if(NPC[numNPCs].Direction < 0)
                                    NPC[numNPCs].Location.X = NPC[A].Location.X - 40;
                                else
                                    NPC[numNPCs].Location.X = NPC[A].Location.X + 54;
                                NPC[numNPCs].Location.Y = NPC[A].Location.Y + 19;
                                NPC[numNPCs].Location.SpeedX = 4 * double(NPC[numNPCs].Direction);
                                double C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[int(NPC[A].Special5)].Location.X + Player[int(NPC[A].Special5)].Location.Width / 2.0);
                                double D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[int(NPC[A].Special5)].Location.Y + Player[int(NPC[A].Special5)].Location.Height / 2.0);
                                NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                                if(NPC[numNPCs].Location.SpeedY > 1)
                                    NPC[numNPCs].Location.SpeedY = 1;
                                else if(NPC[numNPCs].Location.SpeedY < -1)
                                    NPC[numNPCs].Location.SpeedY = -1;
                                PlaySound(42);
                            }
                        }
                        else if(NPC[A].Special3 < 45)
                        {
                            if(NPC[A].Direction == -1)
                                NPC[A].Frame = 0;
                            else
                                NPC[A].Frame = 5;
                        }
                        else
                            NPC[A].Special4 = 0;
                    }

                }
                else if(NPC[A].Type == 29 && NPC[A].HoldingPlayer > 0)
                {
                    if(Player[NPC[A].HoldingPlayer].Effect == 0)
                        NPC[A].Special3 = NPC[A].Special3 + 1;
                    if(NPC[A].Special3 >= 20)
                    {
                        PlaySound(25);
                        NPC[A].Special3 = 0; // -15
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[numNPCs].Location.Height = 32;
                        NPC[numNPCs].Location.Width = 32;
                        NPC[numNPCs].Location.X = NPC[A].Location.X;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                        NPC[numNPCs].Direction = NPC[A].Direction;
                        NPC[numNPCs].Type = 30;
                        NPC[numNPCs].Shadow = NPC[A].Shadow;
                        NPC[numNPCs].CantHurt = 200;
                        NPC[numNPCs].CantHurtPlayer = NPC[A].HoldingPlayer;
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].TimeLeft = 50;
                        NPC[numNPCs].Location.SpeedY = -8;
                        NPC[numNPCs].Location.SpeedX = 3 * Player[NPC[A].HoldingPlayer].Direction + Player[NPC[A].HoldingPlayer].Location.SpeedX * 0.8;
                    }
                }
                else if(NPC[A].Type == 21 || NPC[A].Type == 22) // Bullet Bill Shooter
                {
                    if(NPC[A].Type == 21)
                    {
                        NPC[A].Special = NPC[A].Special + 1;
                        if(NPC[A].HoldingPlayer > 0)
                        {
                            if(Player[NPC[A].HoldingPlayer].Effect == 0)
                                NPC[A].Special = NPC[A].Special + 6;
                        }
                    }
                    else
                    {
                        if(NPC[A].HoldingPlayer > 0)
                        {
                            if(Player[NPC[A].HoldingPlayer].SpinJump == true)
                            {
                                if(NPC[A].Direction != Player[NPC[A].HoldingPlayer].SpinFireDir)
                                {
                                    if(Player[NPC[A].HoldingPlayer].Effect == 0)
                                        NPC[A].Special = NPC[A].Special + 20;
                                }
                            }
                            else
                            {
                                if(Player[NPC[A].HoldingPlayer].Effect == 0)
                                    NPC[A].Special = NPC[A].Special + 10;
                            }
                        }
                        else if(NPC[A].standingOnPlayer > 0)
                            NPC[A].Special = NPC[A].Special + 5;
                        else if(NPC[A].Projectile == true)
                        {
                            // .Special = .Special + 10
                        }
                    }

                    if(NPC[A].Special >= 200)
                    {
                        double C = 0;

                        if(NPC[A].HoldingPlayer > 0)
                        {
                            if(Player[NPC[A].HoldingPlayer].SpinJump == true)
                                Player[NPC[A].HoldingPlayer].SpinFireDir = int(NPC[A].Direction);
                        }

                        if(NPC[A].HoldingPlayer == 0 && NPC[A].standingOnPlayer == 0 && NPC[A].Type == 21)
                        {
                            C = 0;
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                                {
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                                            NPC[A].Direction = -1;
                                        else
                                            NPC[A].Direction = 1;
                                        if(CanComeOut(NPC[A].Location, Player[B].Location) == false)
                                            C = -1;
                                    }
                                }
                            }
                        }

                        if(numNPCs < maxNPCs)
                        {
                            if(fEqual(C, -1) && NPC[A].HoldingPlayer == 0 && NPC[A].standingOnPlayer == 0)
                                NPC[A].Special = 0;
                            else if(Player[NPC[A].standingOnPlayer].Controls.Run == true || NPC[A].standingOnPlayer == 0)
                            {
                                NPC[A].Special = 0;
                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                NPC[numNPCs].Inert = NPC[A].Inert;
                                tempBool = false;
                                NPC[numNPCs].Direction = NPC[A].Direction;
                                if(NPC[A].HoldingPlayer > 0 || NPC[A].standingOnPlayer > 0 || (NPC[A].Type == 22 && NPC[A].Projectile == true))
                                {
                                    NPC[numNPCs].Projectile = true;
                                    NPC[numNPCs].CantHurt = 10000;
                                    NPC[numNPCs].CantHurtPlayer = NPC[A].HoldingPlayer;
                                    NPC[numNPCs].Location.SpeedX = 8 * NPC[numNPCs].Direction;
                                }
                                else if(NPC[A].CantHurtPlayer > 0)
                                {
                                    NPC[numNPCs].Projectile = true;
                                    NPC[numNPCs].CantHurt = 1000;
                                    NPC[numNPCs].CantHurtPlayer = NPC[A].CantHurtPlayer;
                                }
                                else if(NPC[A].Type == 22)
                                {
                                    tempBool = true;
                                    numNPCs = numNPCs - 1;
                                }
                                if(tempBool == false)
                                {
                                    NPC[numNPCs].Shadow = NPC[A].Shadow;
                                    NPC[numNPCs].Active = true;
                                    NPC[numNPCs].TimeLeft = 100;
                                    NPC[numNPCs].JustActivated = 0;
                                    NPC[numNPCs].Section = NPC[A].Section;
                                    NPC[numNPCs].Type = 17;
                                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];

                                    if(NPC[numNPCs].Direction > 0)
                                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                                    else
                                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width;

                                    if(NPC[numNPCs].Direction > 0)
                                        NPC[numNPCs].Frame = 1;
                                    else
                                        NPC[numNPCs].Frame = 0;
                                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - NPC[numNPCs].Location.Height / 2.0;

                                    tempLocation = NPC[numNPCs].Location;
                                    tempLocation.X = NPC[numNPCs].Location.X + (NPC[numNPCs].Location.Width / 2.0) * NPC[numNPCs].Direction;
                                    tempLocation.Y = NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                                    NewEffect(10, tempLocation);

                                    PlaySound(22);
                                }
                            }
                        }
                    }
                }
                else if(NPC[A].Type == 50)
                {
                    B = 0;
                    if(NPC[A].Special > 0)
                    {
                        if(Player[NPC[A].Special].HoldingNPC > 0)
                        {
                            if(NPC[Player[NPC[A].Special].HoldingNPC].Type == 49)
                                B = 1;
                        }
                    }
                    else if(NPC[NPC[A].Special2].Projectile == true && NPC[NPC[A].Special2].Active == true)
                    {
                        B = 1;
                        NPC[A].Projectile = true;
                        NPC[A].Direction = NPC[NPC[A].Special2].Direction;
                        if(NPC[A].Direction > 0)
                            NPC[A].Location.X = NPC[NPC[A].Special2].Location.X + 32;
                        else
                            NPC[A].Location.X = NPC[NPC[A].Special2].Location.X - NPC[A].Location.Width;
                        NPC[A].Location.Y = NPC[NPC[A].Special2].Location.Y;
                    }
                    if(Player[NPC[A].standingOnPlayer].Controls.Run == true)
                        B = 1;
                    if(NPC[A].Special2 > 0 && NPC[NPC[A].Special2].Special2 != A)
                        B = 0;
                    if(NPC[A].Special > 0)
                    {
                        if(Player[NPC[A].Special].Effect != 0)
                            B = 0;
                    }
                    if(B == 0)
                        NPC[A].Killed = 9;
                }
                else if(NPC[A].Type == 49)
                {
                    if(NPC[A].HoldingPlayer == 0 && NPC[A].standingOnPlayer == 0)
                        NPC[A].Special = 0;
                    if(NPC[A].HoldingPlayer > 0 && NPC[A].Special2 > 0)
                        NPC[NPC[A].Special2].Direction = NPC[A].Direction;
                    if(Player[NPC[A].HoldingPlayer].Effect != 0)
                        NPC[A].Special = 0;
                    // FIXME: In original game, this is a dead code because of "And 0" condition at end.
                    // In this sample, the "& false" was been commented, still idk for what this logic needs
                    // the NPC-49 is a "toothy's pipe"
                    if(NPC[A].Projectile && NPC[A].Special2 == 0.0 && NPC[A].Special == 0.0 /*&& false*/)
                    {
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[A].Special2 = numNPCs;
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].Section = NPC[A].Section;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Type = 50;
                        NPC[numNPCs].Location.Height = 32;
                        NPC[numNPCs].Location.Width = 48;
                        NPC[numNPCs].Special = 0;
                        NPC[numNPCs].Special2 = A;
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Direction = NPC[A].Direction;
                        if(NPC[numNPCs].Direction > 0)
                        {
                            NPC[numNPCs].Location.X = NPC[A].Location.X + 32;
                            NPC[numNPCs].Frame = 2;
                        }
                        else
                            NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[numNPCs].Location.Width;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                    }
                    if(NPC[NPC[A].Special2].Type == 50 && fEqual(NPC[NPC[A].Special2].Special2, A))
                    {
                        NPC[NPC[A].Special2].Projectile = true;
                        NPC[NPC[A].Special2].Direction = NPC[A].Direction;
                        if(NPC[A].Direction > 0)
                            NPC[NPC[A].Special2].Location.X = NPC[A].Location.X + 32;
                        else
                            NPC[NPC[A].Special2].Location.X = NPC[A].Location.X - NPC[NPC[A].Special2].Location.Width;
                        NPC[NPC[A].Special2].Location.Y = NPC[A].Location.Y;
                    }
                    if(NPC[A].standingOnPlayer > 0 && Player[NPC[A].standingOnPlayer].Controls.Run == false)
                        NPC[A].Special = 0;
                }
                else if(NPC[A].Type == 31)
                {
                    if(NPC[A].HoldingPlayer > 0)
                    {
                        for(B = 1; B <= numBackground; B++)
                        {
                            if(Background[B].Type == 35)
                            {
                                tempLocation = Background[B].Location;
                                tempLocation.Width = 16;
                                tempLocation.X = tempLocation.X + 8;
                                tempLocation.Height = 26;
                                tempLocation.Y = tempLocation.Y + 2;
                                if(CheckCollision(NPC[A].Location, tempLocation) == true)
                                {
                                    PlaySound(31);
                                    StopMusic();
                                    LevelMacro = 3;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if(NPCIsABot[NPC[A].Type])
                {
                    if(NPC[A].Projectile == true || NPC[A].HoldingPlayer > 0)
                    {
                        NPC[A].Special = -1;
                        NPC[A].Special2 = 0;
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98;
                    }
                    else
                    {
                        if(NPC[A].Special == 0.0)
                        {
                            double C = 0;
                            int D = 1;
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
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
                        }

                        if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                        {
                            NPC[A].Location.SpeedX = 0;
                            if(NPC[A].Special == 0.0)
                                NPC[A].Special = (iRand() % 3) + 1;
                            if(fEqual(NPC[A].Special, 1))
                            {
                                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                                NPC[A].Special2 = NPC[A].Special2 + 1;
                                NPC[A].Location.SpeedX = 0;
                                if(NPC[A].Special2 >= 90)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = -1;
                                    NPC[A].Location.SpeedY = -7;
                                    NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
                                }
                            }
                            else if(NPC[A].Special == 3)
                            {
                                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                                NPC[A].Special2 = NPC[A].Special2 + 30;
                                NPC[A].Location.SpeedX = 0;
                                if(NPC[A].Special2 >= 30)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = -1;
                                    NPC[A].Location.SpeedY = -3;
                                    NPC[A].Location.SpeedX = 2.5 * NPC[A].Direction;
                                }
                            }
                            else if(NPC[A].Special == 2)
                            {
                                NPC[A].Location.SpeedX = 0.5 * NPC[A].Direction;
                                NPC[A].Special2 = NPC[A].Special2 + 1;
                                if(NPC[A].Special2 == 120)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = -2;
                                }
                            }
                            else
                            {
                                NPC[A].Special2 = NPC[A].Special2 + 1;
                                if(NPC[A].Special2 == 30)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = 0;
                                }
                            }
                        }
                    }
                }

                // If FreezeNPCs = True Then
                // .Direction = .DefaultDirection
                // .Special = .DefaultSpecial
                // .Special2 = 0
                // .Special3 = 0
                // .Special4 = 0
                // .Special5 = 0
                // End If


                NPCFrames(A);




                // Effects
            }
            else if(NPC[A].Effect == 1) // Bonus coming out of a block effect
            {

                if(NPC[A].Direction == 0.0f) // Move toward the closest player
                {
                    double C = 0;
                    for(B = 1; B <= numPlayers; B++)
                    {
                        if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                        {
                            if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                            {
                                C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                NPC[A].Direction = -Player[B].Direction;
                            }
                        }
                    }
                }
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction, A);
                NPC[A].Effect2 = NPC[A].Effect2 + 1;
                NPC[A].Location.Y = NPC[A].Location.Y - 1; // .01
                NPC[A].Location.Height = NPC[A].Location.Height + 1;
                if(NPCHeightGFX[NPC[A].Type] > 0)
                {
                    if(NPC[A].Effect2 >= NPCHeightGFX[NPC[A].Type])
                    {
                        NPC[A].Effect = 0;
                        NPC[A].Effect2 = 0;
                        NPC[A].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                        NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                        NPC[A].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                    }
                }
                else
                {
                    if(NPC[A].Effect2 >= NPCHeight[NPC[A].Type])
                    {
                        NPC[A].Effect = 0;
                        NPC[A].Effect2 = 0;
                        NPC[A].Location.Height = NPCHeight[NPC[A].Type];
                    }
                }
            }
            else if(NPC[A].Effect == 208)
            {
                tempBool = false;
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Type == 208)
                    {
                        if(CheckCollision(NPC[A].Location, NPC[B].Location) == true)
                        {
                            tempBool = true;
                            break;
                        }
                    }
                }
                if(tempBool == false)
                    NPC[A].Effect = 0;
            }
            else if(NPC[A].Effect == 2) // Bonus item is falling from the players container effect
            {
                NPC[A].Location.Y = NPC[A].Location.Y + 2.2;
                NPC[A].Effect2 = NPC[A].Effect2 + 1;
                if(NPC[A].Effect2 == 5)
                    NPC[A].Effect2 = 1;
            }
            else if(NPC[A].Effect == 3) // Bonus falling out of a block
            {
                if(NPC[A].Type == 34)
                    NPC[A].Direction = 1;
                else if(NPC[A].Direction == 0.f) // Move toward the closest player
                {
                    double C = 0;
                    for(B = 1; B <= numPlayers; B++)
                    {
                        if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                        {
                            if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                            {
                                C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                NPC[A].Direction = -Player[B].Direction;
                            }
                        }
                    }
                }

                NPC[A].Effect2 = NPC[A].Effect2 + 1;
                NPC[A].Location.Y = NPC[A].Location.Y + 1;

                if(fEqual(NPC[A].Effect2, 32.0))
                {
                    NPC[A].Effect = 0;
                    NPC[A].Effect2 = 0;
                    NPC[A].Location.Height = 32;
                    for(bCheck = 1; bCheck <= 2; bCheck++)
                    {
                        if(bCheck == 1)
                        {
                            fBlock = FirstBlock[(NPC[A].Location.X / 32) - 1];
                            lBlock = LastBlock[((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];
                        }
                        else
                        {
                            fBlock = numBlock - numTempBlock;
                            lBlock = numBlock;
                        }
                        for(B = (int)fBlock; B <= lBlock; B++)
                        {
                            if(Block[B].Invis == false && !(BlockIsSizable[Block[B].Type] == true && NPC[A].Location.Y > Block[B].Location.Y) && Block[B].Hidden == false)
                            {
                                if(CheckCollision(NPC[A].Location, Block[B].Location) == true)
                                {
                                    NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else if(NPC[A].Effect == 4) // Warp Generator
            {
                if(NPC[A].Effect3 == 1)
                {
                    NPC[A].Location.Y -= 1;
                    if(NPC[A].Type == 106)
                        NPC[A].Location.Y = NPC[A].Location.Y - 1;
                    if(NPC[A].Location.Y + NPC[A].Location.Height <= NPC[A].Effect2)
                    {
                        NPC[A].Effect = 0;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }
                else if(NPC[A].Effect3 == 3)
                {
                    NPC[A].Location.Y += 1;
                    if(NPC[A].Type == 106)
                        NPC[A].Location.Y += 1;
                    if(NPC[A].Location.Y >= NPC[A].Effect2)
                    {
                        NPC[A].Effect = 0;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }
                else if(NPC[A].Effect3 == 2)
                {
                    if(NPC[A].Type == 9 || NPC[A].Type == 90 || NPC[A].Type == 153 || NPC[A].Type == 184 || NPC[A].Type == 185 || NPC[A].Type == 186 || NPC[A].Type == 187 || NPC[A].Type == 163 || NPC[A].Type == 164)
                        NPC[A].Location.X -= double(Physics.NPCMushroomSpeed);
                    else if(NPCCanWalkOn[NPC[A].Type] == true)
                        NPC[A].Location.X = NPC[A].Location.X - 1;
                    else
                        NPC[A].Location.X -= double(Physics.NPCWalkingSpeed);
                    if(NPC[A].Location.X + NPC[A].Location.Width <= NPC[A].Effect2)
                    {
                        NPC[A].Effect = 0;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }
                else if(NPC[A].Effect3 == 4)
                {
                    if(NPC[A].Type == 9 || NPC[A].Type == 90 || NPC[A].Type == 153 || NPC[A].Type == 184 || NPC[A].Type == 185 || NPC[A].Type == 186 || NPC[A].Type == 187 || NPC[A].Type == 163 || NPC[A].Type == 164)
                        NPC[A].Location.X += double(Physics.NPCMushroomSpeed);
                    else if(NPCCanWalkOn[NPC[A].Type] == true)
                        NPC[A].Location.X = NPC[A].Location.X + 1;
                    else
                        NPC[A].Location.X += double(Physics.NPCWalkingSpeed);
                    if(NPC[A].Location.X >= NPC[A].Effect2)
                    {
                        NPC[A].Effect = 0;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }

                NPCFrames(A);

                if(NPC[A].Effect == 0 && NPC[A].Type != 91)
                    NPC[A].Layer = "Spawned NPCs";

            }
            else if(NPC[A].Effect == 5) // Grabbed by Yoshi
            {
                NPC[A].TimeLeft = 100;
                NPC[A].Effect3 = NPC[A].Effect3 - 1;
                if(NPC[A].Effect3 <= 0)
                {
                    NPC[A].Effect = 0;
                    NPC[A].Effect2 = 0;
                    NPC[A].Effect3 = 0;
                }
            }
            else if(NPC[A].Effect == 6) // Held by Yoshi
            {
                NPC[A].TimeLeft = 100;
                if(Player[NPC[A].Effect2].YoshiNPC != A)
                {
                    NPC[A].Effect = 0;
                    NPC[A].Effect2 = 0;
                    NPC[A].Effect3 = 0;
                }
            }
            else if(NPC[A].Effect == 8) // Holding Pattern
            {
                NPC[A].Effect2 = NPC[A].Effect2 - 1;
                if(NPC[A].Effect2 <= 0)
                {
                    NPC[A].Effect = 0;
                    NPC[A].Effect2 = 0;
                    NPC[A].Effect3 = 0;
                }
            }
        }

        if(!fEqual(speedVar, 1) && !fEqual(speedVar, 0))
        {
            NPC[A].RealSpeedX = float(NPC[A].Location.SpeedX);
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * double(speedVar);
        }

        if(NPC[A].AttLayer != "" && NPC[A].HoldingPlayer == 0)
        {
            for(B = 1; B <= maxLayers; B++)
            {
                if(Layer[B].Name != "")
                {
                    if(Layer[B].Name == NPC[A].AttLayer)
                    {
                        if(NPC[A].Location.X - lyrX == 0.0 && NPC[A].Location.Y - lyrY == 0.0)
                        {
                            if(Layer[B].SpeedX != 0.0f || Layer[B].SpeedY != 0.0f)
                            {
                                Layer[B].EffectStop = true;
                                Layer[B].SpeedX = 0;
                                Layer[B].SpeedY = 0;

                                for(int C = 1; C <= numBlock; C++)
                                {
                                    if(Block[C].Layer == Layer[B].Name)
                                    {
                                        Block[C].Location.SpeedX = double(Layer[B].SpeedX);
                                        Block[C].Location.SpeedY = double(Layer[B].SpeedY);
                                    }
                                }

                                for(int C = 1; C <= numNPCs; C++)
                                {
                                    if(NPC[C].Layer == Layer[B].Name)
                                    {
                                        if(NPCIsAVine[NPC[C].Type] || NPC[C].Type == 91)
                                        {
                                            NPC[C].Location.SpeedX = 0;
                                            NPC[C].Location.SpeedY = 0;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            Layer[B].EffectStop = false;
                            Layer[B].SpeedX = float(NPC[A].Location.X - lyrX);
                            Layer[B].SpeedY = float(NPC[A].Location.Y - lyrY);
                        }
                    }
                }
                else
                    break;
            }
        }

    }

    numBlock = numBlock - numTempBlock; // clean up the temp npc blocks
    for(A = numNPCs; A >= 1; A--) // KILL THE NPCS <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><
    {
        if(NPC[A].Killed > 0)
        {
            if(NPC[A].Location.SpeedX == 0.0)
            {
                NPC[A].Location.SpeedX = dRand() * 2 - 1;
                if(NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX - 0.5;
                else
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX + 0.5;
            }
            KillNPC(A, NPC[A].Killed);
        }
    }
    //    if(nPlay.Online == true)
    //    {
    //        if(nPlay.Mode == 1)
    //        {
    //            nPlay.NPCWaitCount = nPlay.NPCWaitCount + 10;
    //            if(nPlay.NPCWaitCount >= 5)
    //            {
    //                tempStr = "L" + LB;
    //                for(A = 1; A <= numNPCs; A++)
    //                {
    //                    if(NPC[A].Active == true && NPC[A].TimeLeft > 1)
    //                    {
    //                        if(NPC[A].HoldingPlayer <= 1)
    //                        {
    //                            tempStr = tempStr + "K" + std::to_string(A) + "|" + NPC[A].Type + "|" + NPC[A].Location.X + "|" + NPC[A].Location.Y + "|" + std::to_string(NPC[A].Location.Width) + "|" + std::to_string(NPC[A].Location.Height) + "|" + NPC[A].Location.SpeedX + "|" + NPC[A].Location.SpeedY + "|" + NPC[A].Section + "|" + NPC[A].TimeLeft + "|" + NPC[A].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[A].Projectile)))) + "|" + NPC[A].Special + "|" + NPC[A].Special2 + "|" + NPC[A].Special3 + "|" + NPC[A].Special4 + "|" + NPC[A].Special5 + "|" + NPC[A].Effect + LB;
    //                            if(NPC[A].Effect != 0)
    //                                tempStr = tempStr + "2c" + std::to_string(A) + "|" + NPC[A].Effect2 + "|" + NPC[A].Effect3 + LB;
    //                        }
    //                    }
    //                }
    //                Netplay::sendData tempStr + "O" + std::to_string(numPlayers) + LB;
    //                nPlay.NPCWaitCount = 0;
    //            }
    //        }
    //    }
    CharStuff();
}
