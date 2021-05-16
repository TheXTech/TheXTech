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

#ifdef __3DS__
#error "Primary `main/cheat_code.cpp` only for SDL clients. Build `3ds/cheat_code.cpp` instead."
#endif

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_messagebox.h>

#include <Logger/logger.h>
#include <pge_delay.h>

#include "../globals.h"
#include "../sound.h"
#include "../graphics.h"
#include "../collision.h"
#include "../effect.h"
#include "../player.h"
#include "../npc.h"
#include "../layers.h"
#include "../game_main.h"


void CheatCode(char NewKey)
{
    std::string newCheat;
//    int A = 0;
    int B = 0;
    int C = 0;
    Location_t tempLocation;
    std::string oldString;
    bool cheated = false;

    if(LevelEditor || GameMenu || /*nPlay.Online ||*/ BattleMode)
    {
        CheatString.clear();
        return;
    }

    CheatString.push_back(NewKey);
    if(CheatString.size() > 23)
        CheatString.erase(0, 1);
    oldString = CheatString;

//    D_pLogDebug("Cheat string: [%s]\n", CheatString.c_str());

#ifdef ENABLE_ANTICHEAT_TRAP
    if(SDL_strstr(CheatString.c_str(), "redigitiscool"))
    {
        pLogCritical("redigitiscool code was been used, player got a punish!");
        PlaySound(SFX_SMExplosion);
        Score = 0; // Being very evil here, mu-ha-ha-ha-ha! >:D
        Lives = 0;
        Coins = 0;
        GodMode = false;
        ClearGame(true); // As a penalty, remove the saved game
        Cheater = true;
        CheatString.clear();

        if(!LevelSelect)
        {
            MessageText = "       Die, cheater!       "
                          "Now play the game all over "
                          "    from the beginning!    "
                          "                           "
                          "     Time to be evil!      "
                          "      Mu-ha-ha-ha-ha!      ";
            PauseGame(1);
            MessageText.clear();
        }
        else
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                     "Die, cheater!",
                                     "       Die, cheater!       \n"
                                     "Now play the game all over \n"
                                     "    from the beginning!    \n"
                                     "                           \n"
                                     "     Time to be evil!      \n"
                                     "      Mu-ha-ha-ha-ha!      ",
                                     frmMain.getWindow());
        }

        for(int A = 1; A <= numPlayers; ++A)
        {
            Player[A].State = 0;
            Player[A].Hearts = 1;
            if(!LevelSelect)
                PlayerHurt(A);
            else
                KillPlayer(A);
        }

        if(LevelSelect)
        {
            LevelSelect = false;
            GameMenu = true;
            MenuMode = 0;
            MenuCursor = 0;
            frmMain.clearBuffer();
            frmMain.repaint();
            StopMusic();
            DoEvents();
            PGE_Delay(500);
        }
        return;
    }
    else if(SDL_strstr(CheatString.c_str(), "\x77\x6f\x68\x6c\x73\x74\x61\x6e\x64\x69\x73\x74\x73\x65\x68\x72\x67\x75\x74"))
#else
    if(SDL_strstr(CheatString.c_str(), "redigitiscool") || SDL_strstr(CheatString.c_str(), "\x77\x6f\x68\x6c\x73\x74\x61\x6e\x64\x69\x73\x74\x73\x65\x68\x72\x67\x75\x74"))
#endif
    {
        PlaySound(SFX_SMCry);
        Cheater = false;
        CheatString.clear();
        return;
    }


    if(LevelSelect) // On world map
    {
        if(SDL_strstr(CheatString.c_str(), "imtiredofallthiswalking") || SDL_strstr(CheatString.c_str(), "moonwalk") || SDL_strstr(CheatString.c_str(), "skywalk") || SDL_strstr(CheatString.c_str(), "skzwalk"))
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
            PlaySound(SFX_NewPath);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "illparkwhereiwant") || SDL_strstr(CheatString.c_str(), "parkinglot"))
        {
            if(WalkAnywhere == true)
            {
                WalkAnywhere = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                WalkAnywhere = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
    }
    else // On level
    {
        if(SDL_strstr(CheatString.c_str(), "needashell"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 113;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "fairymagic") || SDL_strstr(CheatString.c_str(), "fairzmagic"))
        {
            if(Player[1].Fairy == true)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    PlaySound(SFX_ZeldaFairy);
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
                    PlaySound(SFX_ZeldaFairy);
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
            PlaySound(SFX_Raccoon);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "istillplaywithlegos") || SDL_strstr(CheatString.c_str(), "istillplazwithlegos"))
        {
            ShowLayer("Destroyed Blocks");
            PlaySound(SFX_Raccoon);
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
            PlaySound(SFX_Raccoon);
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "donttypethis") || SDL_strstr(CheatString.c_str(), "donttzpethis"))
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
            PlaySound(SFX_Raccoon);
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
            for(B = 0; B <= numSections; B++)
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
            PlaySound(SFX_Raccoon);
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
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 114;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needablueshell"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 115;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needayellowshell") || SDL_strstr(CheatString.c_str(), "needazellowshell"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 116;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaturnip"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 92;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needa1up"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 90;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needatanookisuit"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 169;
                if(Player[B].Character >= 3 && Player[B].State != 5)
                {
                    PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "needahammersuit") || SDL_strstr(CheatString.c_str(), "hammertime"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 170;
                if(Player[B].Character >= 3 && Player[B].State != 6)
                {
                    PlaySound(SFX_Raccoon);
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
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 9;
                if(Player[B].Character >= 3 && Player[B].State == 1)
                {
                    PlaySound(SFX_Raccoon);
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
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 14;
                if(Player[B].Character >= 3 && Player[B].State != 3)
                {
                    PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "niceflower"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 264;
                if(Player[B].Character >= 3 && Player[B].State != 7)
                {
                    PlaySound(SFX_Raccoon);
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
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
            {
                Player[B].HeldBonus = 34;
                if(Player[B].Character >= 3 && Player[B].State != 4)
                {
                    PlaySound(SFX_Raccoon);
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
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 96;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaplant"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 49;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needagun"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 22;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaswitch"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 32;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needaclock"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 248;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needabomb"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 135;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "needashoe"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 35;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "redshoe"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 191;
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "blueshoe"))
        {
            PlaySound(SFX_GotItem);
            for(B = 1; B <= numPlayers; B++)
                Player[B].HeldBonus = 193;
            CheatString.clear();
            cheated = true;
        }


        else if(SDL_strstr(CheatString.c_str(), "shadowstar"))
        {
            PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "ibakedacakeforyou") || SDL_strstr(CheatString.c_str(), "ibakedacakeforzou") || SDL_strstr(CheatString.c_str(), "itsamepeach"))
        {
            PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "anothercastle") || SDL_strstr(CheatString.c_str(), "itsametoad"))
        {
            PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "iamerror") || SDL_strstr(CheatString.c_str(), "itsamelink"))
        {
            PlaySound(SFX_Raccoon);
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

        else if(SDL_strstr(CheatString.c_str(), "itsamemario") || SDL_strstr(CheatString.c_str(), "plumberboy") || SDL_strstr(CheatString.c_str(), "plumberboz") || SDL_strstr(CheatString.c_str(), "moustacheman"))
        {
            PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "itsameluigi") || SDL_strstr(CheatString.c_str(), "greenmario"))
        {
            PlaySound(SFX_Raccoon);
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
                        PlaySound(SFX_Raccoon);
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
        else if(SDL_strstr(CheatString.c_str(), "carkeys") || SDL_strstr(CheatString.c_str(), "carkezs"))
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
                    PlaySound(SFX_Grab);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "boingyboing") || SDL_strstr(CheatString.c_str(), "boingzboing"))
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "burnthehousedown") || SDL_strstr(CheatString.c_str(), "hellfire"))
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "hammerinmypants") || SDL_strstr(CheatString.c_str(), "hammerinmzpants"))
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "yellowegg") || SDL_strstr(CheatString.c_str(), "zellowegg"))
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
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
                    PlaySound(SFX_Grab);
                }
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "stophittingme") || SDL_strstr(CheatString.c_str(), "uncle"))
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
        else if(SDL_strstr(CheatString.c_str(), "stickyfingers") || SDL_strstr(CheatString.c_str(), "stickzfingers"))
        {
            if(GrabAll == true)
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGrabAll.Value = 0;
                GrabAll = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGrabAll.Value = 1;
                GrabAll = true;
                PlaySound(SFX_PlayerGrow);
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
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                CaptainN = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "flamethrower"))
        {
            if(FlameThrower == true)
            {
                FlameThrower = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                FlameThrower = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "moneytree") || SDL_strstr(CheatString.c_str(), "moneztree"))
        {
            if(CoinMode == true)
            {
                CoinMode = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                CoinMode = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "donthurtme") || SDL_strstr(CheatString.c_str(), "godmode"))
        {
            if(GodMode == true)
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGodMode.Value = 0;
                GodMode = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkGodMode.Value = 1;
                GodMode = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "wingman"))
        {
            if(FlyForever == true)
            {
                FlyForever = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                FlyForever = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "tooslow"))
        {
            if(SuperSpeed == true)
            {
                SuperSpeed = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                SuperSpeed = true;
                PlaySound(SFX_PlayerGrow);
            }
            CheatString.clear();
            cheated = true;
        }
        else if(SDL_strstr(CheatString.c_str(), "ahippinandahoppin") || SDL_strstr(CheatString.c_str(), "jumpman"))
        {
            if(MultiHop == true)
            {
                MultiHop = false;
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
                MultiHop = true;
                PlaySound(SFX_PlayerGrow);
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
                PlaySound(SFX_PlayerShrink);
                PrintFPS = 0;
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkShowFPS.Value = 1;
                ShowFPS = true;
                PlaySound(SFX_PlayerGrow);
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
                PlaySound(SFX_PlayerShrink);
            }
            else
            {
//                if(TestLevel == true)
//                    frmTestSettings::chkMaxFPS.Value = 1;
                MaxFPS = true;
                PlaySound(SFX_PlayerGrow);
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
