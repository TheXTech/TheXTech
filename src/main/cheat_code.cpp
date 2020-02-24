/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include <Logger/logger.h>
#include <SDL2/SDL.h>

#include "../globals.h"
#include "../sound.h"
#include "../graphics.h"
#include "../collision.h"
#include "../effect.h"
#include "../player.h"
#include "../npc.h"
#include "../layers.h"


void CheatCode(char NewKey)
{
    std::string newCheat;
//    int A = 0;
    int B = 0;
    int C = 0;
    Location_t tempLocation;
    std::string oldString;
    bool cheated = false;

    if(LevelEditor == true || GameMenu == true || /*nPlay.Online == true ||*/ BattleMode == true)
    {
        CheatString.clear();
        return;
    }

    CheatString.push_back(NewKey);
    if(CheatString.size() > 23)
    {
        for(size_t A = 2; A <= CheatString.size(); A++)
            newCheat += CheatString.substr(A - 1, 1);
        CheatString = newCheat;
    }
    oldString = CheatString;

//    std::printf("Cheat string: [%s]\n", CheatString.c_str());
//    std::fflush(stdout);

    if(SDL_strstr(CheatString.c_str(), "redigitiscool"))
    {
        PlaySound(69);
        Cheater = false;
        CheatString.clear();
        return;
    }

    if(LevelSelect) // On world map
    {
        if(SDL_strstr(CheatString.c_str(), "imtiredofallthiswalking"))
        {
            for(B = 1; B <= numWorldPaths; B++)
            {
                tempLocation = WorldPath[B].Location;
                tempLocation.X = tempLocation.X + 4;
                tempLocation.Y = tempLocation.Y + 4;
                tempLocation.Width = tempLocation.Width - 8;
                tempLocation.Height = tempLocation.Height - 8;
                WorldPath[B].Active = true;
                for(C = 1; C <= numScenes; C++)
                {
                    if(CheckCollision(tempLocation, Scene[C].Location))
                        Scene[C].Active = false;
                }
            }
            for(B = 1; B <= numWorldLevels; B++)
                WorldLevel[B].Active = true;
            PlaySound(27);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "illparkwhereiwant"))
        {
            if(WalkAnywhere == true)
            {
                WalkAnywhere = false;
                PlaySound(5);
            }
            else
            {
                WalkAnywhere = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
    }
    else // On level
    {
        if(SDL_strstr(CheatString.c_str(), "needashell"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 113;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "fairymagic"))
        {
            if(Player[1].Fairy == true)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    PlaySound(87);
                    Player[B].Immune = 10;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].Fairy = false;
                    Player[B].FairyTime = 0;
                    SizeCheck(B);
                    NewEffect(63, Player[B].Location);
                }
            }
            else
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    PlaySound(87);
                    Player[B].Immune = 10;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].Fairy = true;
                    Player[B].FairyTime = -1;
                    SizeCheck(B);
                    NewEffect(63, Player[B].Location);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "iceage"))
        {
            for(C = 1; C <= numNPCs; C++)
            {
                if(NPC[C].Active == true)
                {
                    if(NPCNoIceBall[NPC[C].Type] == false && NPC[C].Type != 263 && NPCIsABonus[NPC[C].Type] == false)
                    {
                        NPC[0].Type = 265;
                        NPCHit(C, 3, 0);
                    }
                }
            }
            PlaySound(34);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "istillplaywithlegos"))
        {
            ShowLayer("Destroyed Blocks");
            PlaySound(34);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "itsrainingmen"))
        {
            for(C = 1; C <= numPlayers; C++)
            {
                for(B = -100; B <= 900; B += 34)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 90;
                    NPC[numNPCs].Location.Y = Player[C].Location.Y - 600;
                    NPC[numNPCs].Location.X = Player[C].Location.X - 400 + B;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 32;
                    NPC[numNPCs].Stuck = true;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].Section = Player[C].Section;
                }
            }
            PlaySound(34);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "donttypethis"))
        {
            for(C = 1; C <= numPlayers; C++)
            {
                for(B = -100; B <= 900; B += 34)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 134;
                    NPC[numNPCs].Location.Y = Player[C].Location.Y - 600;
                    NPC[numNPCs].Location.X = Player[C].Location.X - 400 + B;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 32;
                    NPC[numNPCs].Stuck = true;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].Section = Player[C].Section;
                }
            }
            PlaySound(34);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "wetwater"))
        {
            for(B = 1; B <= numEffects; B++)
            {
                if(Effect[B].Type == 113)
                    Effect[B].Life = 0;
            }
            for(B = 0; B <= maxSections; B++)
            {
                if(UnderWater[B] == true)
                {
                    UnderWater[B] = false;
                    if(Background2REAL[B] == 55)
                        Background2[B] = 30;
                    else if(Background2REAL[B] == 56)
                        Background2[B] = 39;
                    else
                        Background2[B] = Background2REAL[B];
                    if(bgMusicREAL[B] == 46)
                    {
                        Background2[B] = 8;
                        bgMusic[B] = 7;
                    }
                    else if(bgMusicREAL[B] == 47)
                    {
                        Background2[B] = 39;
                        bgMusic[B] = 4;
                    }
                    else if(bgMusicREAL[B] == 48)
                    {
                        Background2[B] = 30;
                        bgMusic[B] = 29;
                    }
                    else if(bgMusicREAL[B] == 49)
                    {
                        Background2[B] = 30;
                        bgMusic[B] = 50;
                    }
                    else
                        bgMusic[B] = bgMusicREAL[B];
                }
                else
                {
                    UnderWater[B] = true;
                    if(Background2REAL[B] != 55 && Background2REAL[B] != 56)
                    {
                        if(Background2REAL[B] == 12 || Background2REAL[B] == 13 || Background2REAL[B] == 19 || Background2REAL[B] == 29 || Background2REAL[B] == 30 || Background2REAL[B] == 31 || Background2REAL[B] == 32 || Background2REAL[B] == 33 || Background2REAL[B] == 34 || Background2REAL[B] == 42 || Background2REAL[B] == 43)
                            Background2[B] = 55;
                        else
                            Background2[B] = 56;
                    }
                    else
                        Background2[B] = Background2REAL[B];
                    if(bgMusicREAL[B] < 46 || bgMusicREAL[B] > 49)
                    {
                        if(bgMusic[B] == 7 || bgMusic[B] == 9 || bgMusic[B] == 42)
                            bgMusic[B] = 46;
                        else if(bgMusic[B] == 1 || bgMusic[B] == 2 || bgMusic[B] == 3 || bgMusic[B] == 4 || bgMusic[B] == 6 || bgMusic[B] == 54)
                            bgMusic[B] = 47;
                        else if(bgMusic[B] == 10 || bgMusic[B] == 17 || bgMusic[B] == 28 || bgMusic[B] == 29 || bgMusic[B] == 41 || bgMusic[B] == 51)
                            bgMusic[B] = 48;
                        else if(bgMusic[B] == 14 || bgMusic[B] == 26 || bgMusic[B] == 27 || bgMusic[B] == 35 || bgMusic[B] == 36 || bgMusic[B] == 50)
                            bgMusic[B] = 49;
                        else
                            bgMusic[B] = 18;
                    }
                    else
                        bgMusic[B] = bgMusicREAL[B];
                }
            }
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Dead == false && Player[B].TimeToLive == 0)
                {
                    StopMusic();
                    StartMusic(Player[B].Section);
                    break;
                }
            }
            CheatString.clear();
            cheated = true;
        }




        else if(SDL_strstr(CheatString.c_str(), "needaredshell"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 114;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needablueshell"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 115;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needayellowshell"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 116;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaturnip"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 92;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needa1up"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 90;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needatanookisuit"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 169;
                if(Player[B].Character >= 3 && Player[B].State != 5)
                {
                    PlaySound(34);
                    Player[B].Immune = 30;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].State = 5;
                    SizeCheck(B);
                    NewEffect(10,
                              newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                              Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
                }
                if(Player[B].Character >= 3 && Player[B].Hearts < 3)
                    Player[B].Hearts = Player[B].Hearts + 1;
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needahammersuit"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 170;
                if(Player[B].Character >= 3 && Player[B].State != 6)
                {
                    PlaySound(34);
                    Player[B].Immune = 30;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].State = 6;
                    SizeCheck(B);
                    NewEffect(10,
                              newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                              Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
                }
                if(Player[B].Character >= 3 && Player[B].Hearts < 3)
                    Player[B].Hearts = Player[B].Hearts + 1;
            }
            CheatString.clear();
            cheated = true;
        }

        else if(SDL_strstr(CheatString.c_str(), "needamushroom"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 9;
                if(Player[B].Character >= 3 && Player[B].State == 1)
                {
                    PlaySound(34);
                    Player[B].Immune = 30;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].State = 2;
                    SizeCheck(B);
                    NewEffect(10,
                              newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                              Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
                }
                if(Player[B].Character >= 3 && Player[B].Hearts < 3)
                    Player[B].Hearts = Player[B].Hearts + 1;
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaflower"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 14;
                if(Player[B].Character >= 3 && Player[B].State != 3)
                {
                    PlaySound(34);
                    Player[B].Immune = 30;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].State = 3;
                    SizeCheck(B);
                    NewEffect(10,
                              newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                              Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
                }
                if(Player[B].Character >= 3 && Player[B].Hearts < 3)
                    Player[B].Hearts = Player[B].Hearts + 1;
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaniceflower"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 264;
                if(Player[B].Character >= 3 && Player[B].State != 7)
                {
                    PlaySound(34);
                    Player[B].Immune = 30;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].State = 7;
                    SizeCheck(B);
                    NewEffect(10,
                              newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                              Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
                }
                if(Player[B].Character >= 3 && Player[B].Hearts < 3)
                    Player[B].Hearts = Player[B].Hearts + 1;
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaleaf"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 34;
                if(Player[B].Character >= 3 && Player[B].State != 4)
                {
                    PlaySound(34);
                    Player[B].Immune = 30;
                    Player[B].Effect = 8;
                    Player[B].Effect2 = 4;
                    Player[B].State = 4;
                    SizeCheck(B);
                    NewEffect(10,
                              newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                              Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
                }
                if(Player[B].Character >= 3 && Player[B].Hearts < 3)
                    Player[B].Hearts = Player[B].Hearts + 1;
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needanegg"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 96;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaplant"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 49;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needagun"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 22;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaswitch"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 32;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaclock"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 248;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needabomb"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 135;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needashoe"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 35;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaredshoe"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 191;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needablueshoe"))
        {
            PlaySound(12);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 193;
            CheatString.clear();
            cheated = true;
        }


        else if(SDL_strstr(CheatString.c_str(), "shadowstar"))
        {
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].Immune = 50;
                tempLocation = Player[B].Location;
                tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
                tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
            }
            if(ShadowMode == true)
                ShadowMode = false;
            else
                ShadowMode = true;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "ibakedacakeforyou"))
        {
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].Character = 3;
                Player[B].Immune = 50;
                if(Player[B].Mount <= 1)
                {
                    Player[B].Location.Y = Player[B].Location.Y + Player[B].Location.Height;
                    Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
                    if(Player[B].Mount == 1 && Player[B].State == 1)
                        Player[B].Location.Height = Physics.PlayerHeight[1][2];
                    Player[B].Location.Y = Player[B].Location.Y - Player[B].Location.Height;
                    Player[B].StandUp = true;
                }
                tempLocation = Player[B].Location;
                tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
                tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "anothercastle"))
        {
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].Character = 4;
                Player[B].Immune = 50;
                if(Player[B].Mount <= 1)
                {
                    Player[B].Location.Y = Player[B].Location.Y + Player[B].Location.Height;
                    Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
                    if(Player[B].Mount == 1 && Player[B].State == 1)
                        Player[B].Location.Height = Physics.PlayerHeight[1][2];
                    Player[B].Location.Y = Player[B].Location.Y - Player[B].Location.Height;
                    Player[B].StandUp = true;
                }
                tempLocation = Player[B].Location;
                tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
                tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "iamerror"))
        {
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].Character = 5;
                Player[B].Immune = 50;
                if(Player[B].Mount <= 1)
                {
                    Player[B].Location.Y = Player[B].Location.Y + Player[B].Location.Height;
                    Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
                    if(Player[B].Mount == 1 && Player[B].State == 1)
                        Player[B].Location.Height = Physics.PlayerHeight[1][2];
                    Player[B].Location.Y = Player[B].Location.Y - Player[B].Location.Height;
                    Player[B].StandUp = true;
                }
                tempLocation = Player[B].Location;
                tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
                tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
            }
            CheatString.clear();
            cheated = true;
        }

        else if(SDL_strstr(CheatString.c_str(), "itsamemario"))
        {
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].Character = 1;
                Player[B].Immune = 50;
                if(Player[B].Mount <= 1)
                {
                    Player[B].Location.Y = Player[B].Location.Y + Player[B].Location.Height;
                    Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
                    if(Player[B].Mount == 1 && Player[B].State == 1)
                        Player[B].Location.Height = Physics.PlayerHeight[1][2];
                    Player[B].Location.Y = Player[B].Location.Y - Player[B].Location.Height;
                    Player[B].StandUp = true;
                }
                tempLocation = Player[B].Location;
                tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
                tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "itsameluigi"))
        {
            PlaySound(34);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].Character = 2;
                Player[B].Immune = 50;
                if(Player[B].Mount <= 1)
                {
                    Player[B].Location.Y = Player[B].Location.Y + Player[B].Location.Height;
                    Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
                    if(Player[B].Mount == 1 && Player[B].State == 1)
                        Player[B].Location.Height = Physics.PlayerHeight[1][2];
                    Player[B].Location.Y = Player[B].Location.Y - Player[B].Location.Height;
                    Player[B].StandUp = true;
                }
                tempLocation = Player[B].Location;
                tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
                tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
                NewEffect(10, tempLocation);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario128"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 128;
                ScreenType = 2;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario64"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 64;
                ScreenType = 2;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario32"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 32;
                ScreenType = 2;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario16"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 16;
                ScreenType = 2;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario8"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 8;
                ScreenType = 2;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario4"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 4;
                ScreenType = 2;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "supermario2"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 2;
                ScreenType = 6;
                SingleCoop = 1;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * 24 - 12;
                    }
                    if(C == 1)
                    {
                        Player[C].Character = 1;
                        if(Player[C].Mount <= 1)
                        {
                            Player[C].Location.Y = Player[C].Location.Y + Player[C].Location.Height;
                            Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                            if(Player[C].Mount == 1 && Player[C].State == 1)
                                Player[C].Location.Height = Physics.PlayerHeight[1][2];
                            Player[C].Location.Y = Player[C].Location.Y - Player[C].Location.Height;
                            Player[C].StandUp = true;
                        }
                    }
                    else
                    {
                        Player[C].Character = 2;
                        if(Player[C].Mount <= 1)
                        {
                            Player[C].Location.Y = Player[C].Location.Y + Player[C].Location.Height;
                            Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                            if(Player[C].Mount == 1 && Player[C].State == 1)
                                Player[C].Location.Height = Physics.PlayerHeight[1][2];
                            Player[C].Location.Y = Player[C].Location.Y - Player[C].Location.Height;
                            Player[C].StandUp = true;
                        }
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "1player"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                for(C = 1; C <= numPlayers; C++)
                    Player[C].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Bomb(Player[C].Location, (iRand() % 2) + 2);
                    }
                }
                numPlayers = 1;
                ScreenType = 0;
                SingleCoop = 1;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                C = 1;
                Player[C] = Player[B];
                Player[C].Character = 1;
                if(Player[C].Mount <= 1)
                {
                    Player[C].Location.Y = Player[C].Location.Y + Player[C].Location.Height;
                    Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                    if(Player[C].Mount == 1 && Player[C].State == 1)
                        Player[C].Location.Height = Physics.PlayerHeight[1][2];
                    Player[C].Location.Y = Player[C].Location.Y - Player[C].Location.Height;
                    Player[C].StandUp = true;
                }
                Player[C].Immune = 1;
                Player[C].Immune2 = true;
            }
            CheatString.clear();
            cheated = true;
        }

        else if(SDL_strstr(CheatString.c_str(), "2player"))
        {
            B = CheckLiving();
            if(B > 0)
            {
                numPlayers = 2;
                ScreenType = 5;
                SingleCoop = 0;
                SetupScreens();
                if(Player[B].Effect == 9)
                    Player[B].Effect = 0;
                Player[B].Immune = 1;
                for(C = 1; C <= numPlayers; C++)
                {
                    if(C != B)
                    {
                        Player[C] = Player[B];
                        Player[C].Location.SpeedY = dRand() * -12;
                    }
                    if(C == 1)
                    {
                        Player[C].Character = 1;
                        if(Player[C].Mount <= 1)
                        {
                            Player[C].Location.Y = Player[C].Location.Y + Player[C].Location.Height;
                            Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                            if(Player[C].Mount == 1 && Player[C].State == 1)
                                Player[C].Location.Height = Physics.PlayerHeight[1][2];
                            Player[C].Location.Y = Player[C].Location.Y - Player[C].Location.Height;
                            Player[C].StandUp = true;
                        }
                    }
                    else
                    {
                        Player[C].Character = 2;
                        if(Player[C].Mount <= 1)
                        {
                            Player[C].Location.Y = Player[C].Location.Y + Player[C].Location.Height;
                            Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                            if(Player[C].Mount == 1 && Player[C].State == 1)
                                Player[C].Location.Height = Physics.PlayerHeight[1][2];
                            Player[C].Location.Y = Player[C].Location.Y - Player[C].Location.Height;
                            Player[C].StandUp = true;
                        }
                    }
                }
                Bomb(Player[B].Location, (iRand() % 2) + 2);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "wariotime"))
        {
            for(B = 1; B <= numNPCs; B++)
            {
                if(NPC[B].Active == true)
                {
                    if(!NPCWontHurt[NPC[B].Type] &&
                       !NPCIsABlock[NPC[B].Type] &&
                       !NPCIsABonus[NPC[B].Type] &&
                       !NPCIsACoin[NPC[B].Type] &&
                       !NPCIsAnExit[NPC[B].Type] &&
                        NPC[B].Type != 91 && !NPC[B].Generator &&
                        NPC[B].Inert == false
                    )
                    {
                        PlaySound(34);
                        NPC[B].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height / 2.0;
                        NPC[B].Location.X = NPC[B].Location.X + NPC[B].Location.Width / 2.0;
                        tempLocation = NPC[B].Location;
                        tempLocation.Y = tempLocation.Y - 16;
                        tempLocation.X = tempLocation.X - 16;
                        NewEffect(10, tempLocation);
                        NPC[B].Type = 10;
                        NPC[B].Location.Width = NPCWidth[NPC[B].Type];
                        NPC[B].Location.Height = NPCHeight[NPC[B].Type];
                        NPC[B].Location.Y = NPC[B].Location.Y - NPC[B].Location.Height / 2.0;
                        NPC[B].Location.X = NPC[B].Location.X - NPC[B].Location.Width / 2.0;
                        NPC[B].Location.SpeedX = 0;
                        NPC[B].Location.SpeedY = 0;
                    }
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "wherearemycarkeys"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 31;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    CheckSectionNPC(numNPCs);
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "boingyboing"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 26;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    CheckSectionNPC(numNPCs);
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "bombsaway"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 134;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.X = Player[B].Location.X;
                    NPC[numNPCs].Location.Y = Player[B].Location.Y;
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    CheckSectionNPC(numNPCs);
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "firemissiles"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 17;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    CheckSectionNPC(numNPCs);
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "burnthehousedown"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 279;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    NPC[numNPCs].Location.X = Player[B].Location.X;
                    NPC[numNPCs].Location.Y = Player[B].Location.Y;
                    NPC[numNPCs].Section = Player[B].Section;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "upandout"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 278;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    NPC[numNPCs].Location.X = Player[B].Location.X;
                    NPC[numNPCs].Location.Y = Player[B].Location.Y;
                    NPC[numNPCs].Section = Player[B].Section;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "powhammer"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 241;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    NPC[numNPCs].Location.X = Player[B].Location.X;
                    NPC[numNPCs].Location.Y = Player[B].Location.Y;
                    NPC[numNPCs].Section = Player[B].Section;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "hammerinmypants"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 29;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    NPC[numNPCs].Location.X = Player[B].Location.X;
                    NPC[numNPCs].Location.Y = Player[B].Location.Y;
                    NPC[numNPCs].Section = Player[B].Section;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "rainbowrider"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 195;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }


        else if(SDL_strstr(CheatString.c_str(), "greenegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 95;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "blueegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Frame = 1;
                    NPC[numNPCs].Special = 98;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "yellowegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 99;
                    NPC[numNPCs].Frame = 2;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }

        else if(SDL_strstr(CheatString.c_str(), "redegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 100;
                    NPC[numNPCs].Frame = 3;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "blackegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 148;
                    NPC[numNPCs].Frame = 4;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "purpleegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 149;
                    NPC[numNPCs].Frame = 5;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }

        else if(SDL_strstr(CheatString.c_str(), "pinkegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 150;
                    NPC[numNPCs].Frame = 6;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "coldegg"))
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Mount == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = 96;
                    NPC[numNPCs].Special = 228;
                    NPC[numNPCs].Frame = 6;
                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                    NPC[numNPCs].Location.SpeedX = 0;
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Effect = 2;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 200;
                    NPC[numNPCs].HoldingPlayer = B;
                    Player[B].HoldingNPC = numNPCs;
                    Player[B].ForceHold = 60;
                    PlaySound(23);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "stophittingme"))
        {
            bool tempBool;
            tempBool = GodMode;
            GodMode = false;
            for(B = 1; B <= numPlayers; B++)
            {
                PlayerHurt(B);
            }
            GodMode = tempBool;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "stickyfingers"))
        {
            if(GrabAll == true)
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGrabAll.Value = 0;
                GrabAll = false;
                PlaySound(5);
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGrabAll.Value = 1;
                GrabAll = true;
                PlaySound(6);
            }
            for(B = 1; B <= 128; B++)
            {
                if(GrabAll == true)
                    Player[B].CanGrabNPCs = true;
                else
                    Player[B].CanGrabNPCs = false;
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "captainn"))
        {
            if(CaptainN == true)
            {
                CaptainN = false;
                PlaySound(5);
            }
            else
            {
                CaptainN = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "flamethrower"))
        {
            if(FlameThrower == true)
            {
                FlameThrower = false;
                PlaySound(5);
            }
            else
            {
                FlameThrower = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "moneytree"))
        {
            if(CoinMode == true)
            {
                CoinMode = false;
                PlaySound(5);
            }
            else
            {
                CoinMode = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "donthurtme"))
        {
            if(GodMode == true)
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGodMode.Value = 0;
                GodMode = false;
                PlaySound(5);
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGodMode.Value = 1;
                GodMode = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "wingman"))
        {
            if(FlyForever == true)
            {
                FlyForever = false;
                PlaySound(5);
            }
            else
            {
                FlyForever = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "sonicstooslow"))
        {
            if(SuperSpeed == true)
            {
                SuperSpeed = false;
                PlaySound(5);
            }
            else
            {
                SuperSpeed = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "ahippinandahoppin"))
        {
            if(MultiHop == true)
            {
                MultiHop = false;
                PlaySound(5);
            }
            else
            {
                MultiHop = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "framerate"))
        {
            if(ShowFPS)
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkShowFPS.Value = 0;
                ShowFPS = false;
                PlaySound(5);
                PrintFPS = 0;
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkShowFPS.Value = 1;
                ShowFPS = true;
                PlaySound(6);
            }
            CheatString.clear();
            return;
        }
        else if(SDL_strstr(CheatString.c_str(), "speeddemon"))
        {
            if(MaxFPS == true)
            {
                MaxFPS = false;
//                if(TestLevel == true)
//                    frmTestSettings::chkMaxFPS.Value = 0;
                PlaySound(5);
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkMaxFPS.Value = 1;
                MaxFPS = true;
                PlaySound(6);
            }
            CheatString.clear();
            cheated = true;
        }
    }

    if(cheated)
    {
        pLogDebug("Cheating detected!!! [%s]\n", newCheat.c_str());
        Cheater = true;
    }
}
