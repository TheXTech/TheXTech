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
#include "../graphics.h"
#include "../player.h"
#include "../game_main.h"

#include <Utils/maths.h>
#include <Logger/logger.h>


void DropBonus(int A)
{
    int B = 0;
    //    if(nPlay.Online == true && nPlay.MySlot + 1 != A)
    //        return;
    if(A == 1 || numPlayers == 2 /*|| nPlay.Online == true*/)
    {
        if(Player[A].Character == 3 || Player[A].Character == 4)
        {
            Player[A].HeldBonus = 0;
            return;
        }
        if(Player[A].HeldBonus > 0)
        {
            PlaySound(11);
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = Player[A].HeldBonus;
            NPC[numNPCs].Location.Width = NPCWidth[Player[A].HeldBonus];
            NPC[numNPCs].Location.Height = 32;
            if(ScreenType == 5 && !vScreen[2].Visible /*&& nPlay.Online == false*/)
            {
                if(A == 1)
                    B = -40;
                if(A == 2)
                    B = 40;
                GetvScreenAverage();
                NPC[numNPCs].Location.X = -vScreenX[1] + vScreen[1].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0 + B;
                NPC[numNPCs].Location.Y = -vScreenY[1] + 16 + 12;
            }
                //            else if(nPlay.Online == true)
                //            {
                //                GetvScreen 1;
                //                NPC[numNPCs].Location.X = -vScreenX[1] + vScreen[1].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                //                NPC[numNPCs].Location.Y = -vScreenY[1] + 16 + 12;
                //            }
            else
            {
                GetvScreen(A);
                NPC[numNPCs].Location.X = -vScreenX[A] + vScreen[A].Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = -vScreenY[A] + 16 + 12;
            }
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Effect = 2;
            NPC[numNPCs].Effect2 = 1;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            CheckSectionNPC(numNPCs);
            Player[A].HeldBonus = 0;
            //            if(nPlay.Online == true)
            //                Netplay::sendData "M" + NPC[numNPCs].Type + "|" + NPC[numNPCs].Location.X + "|" + NPC[numNPCs].Location.Y + LB;
        }
    }
    else
        Player[A].HeldBonus = 0;
}

void TouchBonus(int A, int B)
{
    int C = 0;
    int D = 0;
    int toadBool = 0;
    bool tempBool = false;
    Location_t tempLocation;

    if(NPC[B].CantHurtPlayer != A || (NPCIsACoin[NPC[B].Type] && Player[A].HoldingNPC != B && NPC[B].Killed == 0))
    {
        //        if(nPlay.Online == true && nPlay.MySlot + 1 == A)
        //            Netplay::sendData "1k" + std::to_string(A) + "|" + std::to_string(B) + "|" + NPC[B].Type + LB;
        if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5) // for link give hearts
        {
            if(NPC[B].Type == 34 || NPC[B].Type == 169 || NPC[B].Type == 170)
            {
                Player[A].Hearts = Player[A].Hearts + 1;
                if(Player[A].Hearts > 3)
                    Player[A].Hearts = 3;
            }
        }

        // If .Character = 3 Or .Character = 4 Then  'for peach and toad turn leaf and suits into a mushroom
        // If NPC(B).Type = 34 Or NPC(B).Type = 169 Or NPC(B).Type = 170 Then NPC(B).Type = 9
        // End If

        if(NPC[B].Type == 273 && numPlayers > 1) // ? mushroom
        {
            if(!Player[1].Dead && Player[1].TimeToLive == 0 && !Player[2].Dead &&
                Player[2].TimeToLive == 0 && Player[1].Immune == 0 && Player[2].Immune == 0)
            {
                tempLocation = Player[1].Location;
                Player[1].Location.X = Player[2].Location.X + Player[2].Location.Width / 2.0 - Player[1].Location.Width / 2.0;
                Player[1].Location.Y = Player[2].Location.Y + Player[2].Location.Height - Player[1].Location.Height;
                Player[2].Location.X = tempLocation.X + Player[1].Location.Width / 2.0 - Player[2].Location.Width / 2.0;
                Player[2].Location.Y = tempLocation.Y + Player[1].Location.Height - Player[2].Location.Height;
                C = Player[1].Direction;
                Player[1].Direction = Player[2].Direction;
                Player[2].Direction = C;
                C = Player[1].Slope;
                Player[1].Slope = Player[2].Slope;
                Player[2].Slope = C;
                C = Player[1].StandingOnNPC;
                Player[1].StandingOnNPC = Player[2].StandingOnNPC;
                Player[2].StandingOnNPC = C;
                if(Player[1].Immune < 10)
                    Player[1].Immune = 10;
                if(Player[2].Immune < 10)
                    Player[2].Immune = 10;
                NPC[B].Killed = 9;
                PlaySound(20);
                return;
            }
        }

        if(NPC[B].Type == 254) // Player is a fairy
        {
            if(Player[A].Mount == 2)
                return;
            else
            {
                if(!Player[A].Fairy)
                {
                    Player[A].Immune = 30;
                    Player[A].Effect = 8;
                    Player[A].Effect2 = 4;
                    Player[A].Fairy = true;
                    SizeCheck(A);
                    NewEffect(63, Player[A].Location);
                }
                PlaySound(87);
                Player[A].FairyTime = -1;
                NPC[B].Killed = 9;
            }
        }
        if(NPC[B].Type == 90 || NPC[B].Type == 187 || NPC[B].Type == 186) // player touched a 1up mushroom
        {
            NPC[B].Killed = 9;
            MoreScore(10, NPC[B].Location);
            return;
        }
        if(NPC[B].Type == 248 && NPC[B].Effect != 2 && (Player[A].Character == 1 || Player[A].Character == 2)) // send the clock to the item container
        {
            Player[A].HeldBonus = 248;
            NPC[B].Killed = 9;
            PlaySound(12);
            return;
        }
        if(NPC[B].Type == 240 || NPC[B].Type == 248) // player touched the clock
        {
            PSwitchStop = Physics.NPCPSwitch;
            FreezeNPCs = true;
            PSwitchPlayer = A;
            NPC[B].Killed = 9;
            return;
        }
        if(NPC[B].Type == 192) // player touched the chekpoint
        {
            if(Player[A].State == 1)
                Player[A].State = 2;
            if(Player[A].Hearts == 1)
                Player[A].Hearts = 2;
            SizeCheck(A);
            NPC[B].Killed = 9;
            PlaySound(58);
            Checkpoint = FullFileName;
            Checkpoint_t cp;
            cp.id = Maths::iRound(NPC[B].Special);
            CheckpointsList.push_back(cp);
            pLogDebug("Added checkpoint ID %d", cp.id);
            return;
        }
        if(NPC[B].Type == 188) // player touched the 3up moon
        {
            NPC[B].Killed = 9;
            MoreScore(12, NPC[B].Location);
            return;
        }
        if(NPC[B].Type == 178)
        {
            NPC[B].Killed = 9;
            return;
        }
        if(NPCIsToad[NPC[B].Type])
        {
            toadBool = NPC[B].Type;
            NPC[B].Type = 9;
        }
        if(NPC[B].Type == 153) // Bonus is a POISON mushroom
            PlayerHurt(A);
        else if(NPC[B].Type == 9 || NPC[B].Type == 184 || NPC[B].Type == 185 || NPC[B].Type == 249 || NPC[B].Type == 250) // Bonus is a mushroom
        {
            if(Player[A].Character == 5 && Player[A].State == 1)
                Player[A].State = 2;
            if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
            {
                Player[A].Hearts = Player[A].Hearts + 1;
                if(Player[A].Hearts > 3)
                    Player[A].Hearts = 3;
            }

            UpdatePlayerBonus(A, NPC[B].Type);
            if(Player[A].State == 1 && Player[A].Character != 5)
            {
                if(Player[A].Duck)
                    UnDuck(A);
                Player[A].StateNPC = NPC[B].Type;
                Player[A].Frame = 1;
                Player[A].Effect = 1;
                if(Player[A].Mount > 0)
                    UnDuck(A);
                PlaySound(6);
            }
            else if(NPC[B].Type == 250)
                PlaySound(79);
            else
                PlaySound(12);
            if(NPC[B].Effect != 2)
                MoreScore(6, NPC[B].Location);
        }
        else if(NPC[B].Type == 14 || NPC[B].Type == 182 || NPC[B].Type == 183) // Bonus is a fire flower
        {
            if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
            {
                Player[A].Hearts = Player[A].Hearts + 1;
                if(Player[A].Hearts > 3)
                    Player[A].Hearts = 3;
            }
            UpdatePlayerBonus(A, NPC[B].Type);
            Player[A].StateNPC = NPC[B].Type;
            if(Player[A].State != 3)
            {
                Player[A].Frame = 1;
                Player[A].Effect = 4;
                if(Player[A].Mount > 0)
                    UnDuck(A);
                if(Player[A].Character == 5)
                    PlaySound(83);
                else
                    PlaySound(6);
            }
            else
            {
                if(Player[A].Character == 5)
                    PlaySound(79);
                else
                    PlaySound(12);
            }
            if(NPC[B].Effect != 2)
                MoreScore(6, NPC[B].Location);
        }
        else if(NPC[B].Type == 264 || NPC[B].Type == 277) // Bonus is an ice flower
        {
            if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
            {
                Player[A].Hearts = Player[A].Hearts + 1;
                if(Player[A].Hearts > 3)
                    Player[A].Hearts = 3;
            }

            UpdatePlayerBonus(A, NPC[B].Type);
            Player[A].StateNPC = NPC[B].Type;
            if(Player[A].State != 7)
            {
                Player[A].Frame = 1;
                Player[A].Effect = 41;
                if(Player[A].Mount > 0)
                    UnDuck(A);
                if(Player[A].Character == 5)
                    PlaySound(83);
                else
                    PlaySound(6);
            }
            else
            {
                if(Player[A].Character == 5)
                    PlaySound(79);
                else
                    PlaySound(12);
            }
            if(NPC[B].Effect != 2)
                MoreScore(6, NPC[B].Location);
        }
        else if(NPC[B].Type == 34) // Bonus is a leaf
        {
            UpdatePlayerBonus(A, NPC[B].Type);
            Player[A].StateNPC = NPC[B].Type;
            if(Player[A].State != 4)
            {
                Player[A].Frame = 1;
                Player[A].Effect = 5;
                Player[A].Effect2 = 0;
                if(Player[A].Mount > 0)
                    UnDuck(A);
                if(Player[A].Character == 5)
                    PlaySound(83);
                else
                    PlaySound(34);
            }
            else
            {
                if(Player[A].Character == 5)
                    PlaySound(79);
                else
                    PlaySound(12);
            }
            if(NPC[B].Effect != 2)
                MoreScore(6, NPC[B].Location);
        }
        else if(NPC[B].Type == 169) // Bonus is a Tanooki Suit
        {
            UpdatePlayerBonus(A, NPC[B].Type);
            Player[A].StateNPC = NPC[B].Type;
            if(Player[A].State != 5)
            {
                Player[A].Frame = 1;
                Player[A].Effect = 11;
                Player[A].Effect2 = 0;
                if(Player[A].Mount > 0)
                    UnDuck(A);
                if(Player[A].Character == 5)
                    PlaySound(83);
                else
                    PlaySound(34);
            }
            else
            {
                if(Player[A].Character == 5)
                    PlaySound(79);
                else
                    PlaySound(12);
            }
            if(NPC[B].Effect != 2)
                MoreScore(6, NPC[B].Location);
        }
        else if(NPC[B].Type == 170) // Bonus is a Hammer Suit
        {
            UpdatePlayerBonus(A, NPC[B].Type);
            Player[A].StateNPC = NPC[B].Type;
            if(Player[A].State != 6)
            {
                Player[A].Frame = 1;
                Player[A].Effect = 12;
                Player[A].Effect2 = 0;
                if(Player[A].Mount > 0)
                    UnDuck(A);
                if(Player[A].Character == 5)
                    PlaySound(83);
                else
                    PlaySound(34);
            }
            else
            {
                if(Player[A].Character == 5)
                    PlaySound(79);
                else
                    PlaySound(12);
            }
            if(NPC[B].Effect != 2)
                MoreScore(6, NPC[B].Location);
        }
        else if(NPCIsACoin[NPC[B].Type]) // Bonus is a coin
        {
            if(NPC[B].Type == 152)
                PlaySound(56);
            else if(NPC[B].Type == 251 || NPC[B].Type == 252 || NPC[B].Type == 253)
                PlaySound(81);
            else if(NPC[B].Type != 274)
                PlaySound(14);
            if(NPC[B].Type == 252 || NPC[B].Type == 258)
                Coins = Coins + 5;
            else if(NPC[B].Type == 253)
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
            if(NPC[B].Type == 274)
            {
                PlaySound(59);
                MoreScore(NPCScore[NPC[B].Type], NPC[B].Location);
                NPCScore[274] = NPCScore[274] + 1;
                if(NPCScore[274] > 14)
                    NPCScore[274] = 14;
            }
            else
                MoreScore(1, NPC[B].Location);
            NewEffect(78, NPC[B].Location);
        }
        else if(NPCIsAnExit[NPC[B].Type] && LevelMacro == 0) // Level exit
        {
            if(NPC[B].Type != 196)
            {
                TurnNPCsIntoCoins();
                FreezeNPCs = false;
                if(numPlayers > 2 /*&& nPlay.Online == false*/)
                    Player[1] = Player[A];
            }
            if(NPC[B].Type == 11)
            {
                if(NPC[B].Frame == 0)
                    MoreScore(10, Player[A].Location);
                if(NPC[B].Frame == 1)
                    MoreScore(6, Player[A].Location);
                if(NPC[B].Frame == 2)
                    MoreScore(8, Player[A].Location);
                LevelMacro = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(A != C) // And DScreenType <> 5 Then
                    {
                        Player[C].Section = Player[A].Section;
                        Player[C].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[C].Location.Height;
                        Player[C].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[C].Location.Width / 2.0;
                        Player[C].Location.SpeedX = 0;
                        Player[C].Location.SpeedY = 0;
                        Player[C].Effect = 8;
                        Player[C].Effect2 = -A;
                    }
                }
                StopMusic();
                DoEvents();
                PlaySound(19);
            }
            else if(NPC[B].Type == 16)
            {
                LevelMacro = 2;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(A != C) // And DScreenType <> 5 Then
                    {
                        Player[C].Section = Player[A].Section;
                        Player[C].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[C].Location.Height;
                        Player[C].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[C].Location.Width / 2.0;
                        Player[C].Location.SpeedX = 0;
                        Player[C].Location.SpeedY = 0;
                        Player[C].Effect = 8;
                        Player[C].Effect2 = -A;
                    }
                }
                StopMusic();
                PlaySound(21);
            }
            else if(NPC[B].Type == 41)
            {
                LevelMacro = 4;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(A != C) // And DScreenType <> 5 Then
                    {
                        Player[C].Section = Player[A].Section;
                        Player[C].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[C].Location.Height;
                        Player[C].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[C].Location.Width / 2.0;
                        Player[C].Location.SpeedX = 0;
                        Player[C].Location.SpeedY = 0;
                        Player[C].Effect = 8;
                        Player[C].Effect2 = -A;
                    }
                }
                StopMusic();
                PlaySound(40);
            }
            else if(NPC[B].Type == 97 || NPC[B].Type == 196)
            {
                for(C = 1; C <= numStars; C++)
                {
                    if(Star[C].level == FileNameFull && (Star[C].Section == NPC[B].Section || Star[C].Section == -1))
                        tempBool = true;
                }

                if(!tempBool)
                {
                    numStars = numStars + 1;
                    Star[numStars].level = FileNameFull;
                    Star[numStars].Section = NPC[B].Section;
                    for(C = 1; C <= numWarps; C++)
                    {
                        if(Warp[C].Stars == numStars)
                        {
                            int allBGOs = numBackground + numLocked;
                            for(D = numBackground; D <= allBGOs; D++)
                            {
                                if(Background[D].Type == 160)
                                {
                                    if(CheckCollision(Warp[C].Entrance, Background[D].Location))
                                    {
                                        Background[D].Layer.clear();
                                        Background[D].Hidden = true;
                                    }
                                }
                            }
                        }
                    }
                }

                if(NPC[B].Type == 97)
                {
                    LevelMacro = 6;
                    for(C = 1; C <= numPlayers; C++)
                    {
                        if(A != C) // And DScreenType <> 5 Then
                        {
                            Player[C].Section = Player[A].Section;
                            Player[C].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[C].Location.Height;
                            Player[C].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[C].Location.Width / 2.0;
                            Player[C].Location.SpeedX = 0;
                            Player[C].Location.SpeedY = 0;
                            Player[C].Effect = 8;
                            Player[C].Effect2 = -A;
                        }
                    }
                    StopMusic();
                    PlaySound(52);
                }
                else
                    PlaySound(59);
            }
        }
        if(toadBool > 0)
            NPC[B].Type = toadBool;
        NPC[B].Killed = 9;
        //        if(nPlay.Online == true && A == nPlay.MySlot + 1)
        //            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1c" + std::to_string(A) + "|" + Player[A].Effect + "|" + Player[A].Effect2 + "1h" + std::to_string(A) + "|" + Player[A].State + LB;
    }
}
