/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <vector>

#ifdef ENABLE_ANTICHEAT_TRAP
#include "core/msgbox.h"
#include "core/render.h"
#include "core/events.h"
#endif

#include "sdl_proxy/sdl_stdinc.h"

#include <Logger/logger.h>
#ifdef ENABLE_ANTICHEAT_TRAP
#include <pge_delay.h>
#endif

#include "../globals.h"
#include "../sound.h"
#include "../graphics.h"
#include "../collision.h"
#include "../effect.h"
#include "../player.h"
#include "../npc.h"
#include "../layers.h"
#include "../controls.h"
#include "../game_main.h"
#include "game_info.h"
#include "screen_quickreconnect.h"
#include "frm_main.h"

#include "npc/npc_queues.h"
#include "main/trees.h"

#include "core/render.h"

#include "cheat_code.h"

#include "npc_id.h"
#include "eff_id.h"

bool g_ForceBitmaskMerge = false;

static void redigitIsCool()
{
    PlaySound(SFX_SMCry);
    Cheater = false;
}


#ifdef ENABLE_ANTICHEAT_TRAP
static void dieCheater()
{
    pLogCritical("redigitiscool code was been used, player got a punish!");
    PlaySound(SFX_SMExplosion);
    Score = 0; // Being very evil here, mu-ha-ha-ha-ha! >:D
    Lives = 0;
    Coins = 0;
    GodMode = false;
    ClearGame(true); // As a penalty, remove the saved game
    Cheater = true;
    cheats_clearBuffer();

    if(!LevelSelect)
    {
        MessageText = "       Die, cheater!       "
                      "Now play the game all over "
                      "    from the beginning!    "
                      "                           "
                      "     Time to be evil!      "
                      "      Mu-ha-ha-ha-ha!      ";
        PauseGame(PauseCode::Message);
        MessageText.clear();
    }
    else
    {
        XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR,
                              "Die, cheater!",
                              "       Die, cheater!       \n"
                              "Now play the game all over \n"
                              "    from the beginning!    \n"
                              "                           \n"
                              "     Time to be evil!      \n"
                              "      Mu-ha-ha-ha-ha!      ");
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
        XRender::clearBuffer();
        XRender::repaint();
        StopMusic();
        XEvents::doEvents();

        if(!MaxFPS)
            PGE_Delay(500);
    }
}
#endif



/* -----------------------------------*
 *      World map only cheats         *
 *------------------------------------*/

/*!
 * \brief Open all paths
 */
static void moonWalk()
{
    TinyLocation_t tempLocation;

    for(int B = 1; B <= numWorldPaths; B++)
    {
        tempLocation = WorldPath[B].Location;
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        WorldPath[B].Active = true;

        for(int C = 1; C <= numScenes; C++)
        {
            if(CheckCollision(tempLocation, Scene[C].Location))
                Scene[C].Active = false;
        }
    }

    for(int B = 1; B <= numWorldLevels; B++)
        WorldLevel[B].Active = true;

    PlaySound(SFX_NewPath);
}

/*!
 * \brief Allow player walk everywhere
 */
static void illParkWhereIWant()
{
    if(WalkAnywhere)
    {
        WalkAnywhere = false;
        PlaySound(SFX_PlayerShrink);
    }
    else
    {
        WalkAnywhere = true;
        PlaySound(SFX_PlayerGrow);
    }
}


/* -----------------------------------*
 *        Level only cheats           *
 *------------------------------------*/


static void needAShell()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 113;
}

static void fairyMagic()
{
    if(Player[1].Fairy)
    {
        for(int B = 1; B <= numPlayers; B++)
        {
            PlaySound(SFX_HeroFairy);
            Player[B].Immune = 10;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].Fairy = false;
            Player[B].FairyTime = 0;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S5, Player[B].Location);
        }
    }
    else
    {
        for(int B = 1; B <= numPlayers; B++)
        {
            PlaySound(SFX_HeroFairy);
            Player[B].Immune = 10;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].Fairy = true;
            Player[B].FairyTime = -1;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S5, Player[B].Location);
        }
    }
}

static void iceAge()
{
    for(int C = 1; C <= numNPCs; C++)
    {
        if(NPC[C].Active)
        {
            if(!NPCNoIceBall[NPC[C].Type] && NPC[C].Type != NPCID_ICE_CUBE && !NPCIsABonus[NPC[C].Type])
            {
                NPC[0].Type = NPCID_PLR_ICEBALL;
                NPCHit(C, 3, 0);
            }
        }
    }

    PlaySound(SFX_Transform);
}

static void iStillPlayWithLegos()
{
    ShowLayer(LAYER_DESTROYED_BLOCKS);
    PlaySound(SFX_Transform);
}

static void itsRainingMen()
{
    for(int C = 1; C <= numPlayers; C++)
    {
        for(int B = -100; B <= 900; B += 34)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_LIFE_S3;
            NPC[numNPCs].Location.Y = Player[C].Location.Y - 600;
            NPC[numNPCs].Location.X = Player[C].Location.X - 400 + B;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 32;
            NPC[numNPCs].Stuck = true;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].Section = Player[C].Section;
            syncLayers_NPC(numNPCs);
        }
    }

    PlaySound(SFX_Transform);
}

static void dontTypeThis()
{
    for(int C = 1; C <= numPlayers; C++)
    {
        for(int B = -100; B <= 900; B += 34)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_BOMB;
            NPC[numNPCs].Location.Y = Player[C].Location.Y - 600;
            NPC[numNPCs].Location.X = Player[C].Location.X - 400 + B;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 32;
            NPC[numNPCs].Stuck = true;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].Section = Player[C].Section;
            syncLayers_NPC(numNPCs);
        }
    }

    PlaySound(SFX_Transform);
}

static void wetWater()
{
    for(int B = 1; B <= numEffects; B++)
    {
        if(Effect[B].Type == EFFID_AIR_BUBBLE)
            Effect[B].Life = 0;
    }

    for(int B = 0; B <= numSections; B++)
    {
        if(UnderWater[B])
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
                if(Background2REAL[B] == 12 || Background2REAL[B] == 13 || Background2REAL[B] == 19 || Background2REAL[B] == 29 ||
                   Background2REAL[B] == 30 || Background2REAL[B] == 31 || Background2REAL[B] == 32 || Background2REAL[B] == 33 ||
                   Background2REAL[B] == 34 || Background2REAL[B] == 42 || Background2REAL[B] == 43)
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

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        if(!Player[B].Dead && Player[B].TimeToLive == 0)
        {
            StopMusic();
            StartMusic(Player[B].Section);
            break;
        }
    }
}

static void needARedShell()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 114;
}

static void needABlueShell()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 115;
}

static void needAYellowShell()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 116;
}

static void needATurnip()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 92;
}

static void needA1Up()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 90;
}

static void needATanookiSuit()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = 169;

        if(Player[B].Character >= 3 && Player[B].State != 5)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].State = 5;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3,
                      newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                      Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needAHammerSuit()
{
    PlaySound(SFX_GotItem);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = 170;

        if(Player[B].Character >= 3 && Player[B].State != 6)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].State = 6;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3,
                      newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                      Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needAMushroom()
{
    PlaySound(SFX_GotItem);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = 9;

        if(Player[B].Character >= 3 && Player[B].State == 1)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].State = 2;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3,
                      newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                      Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needAFlower()
{
    PlaySound(SFX_GotItem);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = 14;

        if(Player[B].Character >= 3 && Player[B].State != 3)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].State = 3;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3,
                      newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                      Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needAnIceFlower()
{
    PlaySound(SFX_GotItem);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = 264;

        if(Player[B].Character >= 3 && Player[B].State != 7)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].State = 7;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3,
                      newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                      Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needALeaf()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = 34;

        if(Player[B].Character >= 3 && Player[B].State != 4)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = 8;
            Player[B].Effect2 = 4;
            Player[B].State = 4;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3,
                      newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - EffectWidth[10] / 2.0,
                      Player[B].Location.Y + Player[B].Location.Height / 2.0 - EffectHeight[10] / 2.0));
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needANegg()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 96;
}

static void needAPlant()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 49;
}

static void needAGun()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 22;
}

static void needASwitch()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 32;
}

static void needAClock()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 248;
}

static void needABomb()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 135;
}

static void needAShoe()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 35;
}

static void redShoe()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 191;
}

static void blueShoe()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = 193;
}

static void shadowStar()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Immune = 50;
        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    ShadowMode = !ShadowMode;

    //if(ShadowMode)
    //    ShadowMode = false;
    //else
    //    ShadowMode = true;
}

static void becomeAsPeach()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Character = 3;
        Player[B].Immune = 50;

        if(Player[B].Mount <= 1)
        {
            Player[B].Location.Y += Player[B].Location.Height;
            Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
            if(Player[B].Mount == 1 && Player[B].State == 1)
                Player[B].Location.Height = Physics.PlayerHeight[1][2];
            Player[B].Location.Y += -Player[B].Location.Height;
            Player[B].StandUp = true;
        }

        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    UpdateYoshiMusic();
}

static void becomeAsToad()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Character = 4;
        Player[B].Immune = 50;

        if(Player[B].Mount <= 1)
        {
            Player[B].Location.Y += Player[B].Location.Height;
            Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
            if(Player[B].Mount == 1 && Player[B].State == 1)
                Player[B].Location.Height = Physics.PlayerHeight[1][2];
            Player[B].Location.Y += -Player[B].Location.Height;
            Player[B].StandUp = true;
        }

        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    UpdateYoshiMusic();
}

static void becomeAsLink()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Character = 5;
        Player[B].Immune = 50;

        if(Player[B].Mount <= 1)
        {
            Player[B].Location.Y += Player[B].Location.Height;
            Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
            if(Player[B].Mount == 1 && Player[B].State == 1)
                Player[B].Location.Height = Physics.PlayerHeight[1][2];
            Player[B].Location.Y += -Player[B].Location.Height;
            Player[B].StandUp = true;
        }

        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    UpdateYoshiMusic();
}

static void becomeAsMario()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Character = 1;
        Player[B].Immune = 50;

        if(Player[B].Mount <= 1)
        {
            Player[B].Location.Y += Player[B].Location.Height;
            Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
            if(Player[B].Mount == 1 && Player[B].State == 1)
                Player[B].Location.Height = Physics.PlayerHeight[1][2];
            Player[B].Location.Y += -Player[B].Location.Height;
            Player[B].StandUp = true;
        }

        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    UpdateYoshiMusic();
}

static void becomeAsLuigi()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Character = 2;
        Player[B].Immune = 50;

        if(Player[B].Mount <= 1)
        {
            Player[B].Location.Y += Player[B].Location.Height;
            Player[B].Location.Height = Physics.PlayerHeight[Player[B].Character][Player[B].State];
            if(Player[B].Mount == 1 && Player[B].State == 1)
                Player[B].Location.Height = Physics.PlayerHeight[1][2];
            Player[B].Location.Y += -Player[B].Location.Height;
            Player[B].StandUp = true;
        }

        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    UpdateYoshiMusic();
}

static void superbDemo200()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 200;

        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;
        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo128()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 128;

        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;
        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo64()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 64;
        SetupScreens();
        if(Player[B].Effect == 9)
            Player[B].Effect = 0;
        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo32()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 32;
        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;

        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo16()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 16;
        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;

        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo8()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 8;
        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;

        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo4()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 4;
        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;

        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * 24 - 12;
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void superbDemo2()
{
    int B = CheckLiving();

    if(B > 0)
    {
        numPlayers = 2;
        SingleCoop = 1;
        SetupScreens();
        if(Player[B].Effect == 9)
            Player[B].Effect = 0;
        Player[B].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
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
                    Player[C].Location.Y += Player[C].Location.Height;
                    Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                    if(Player[C].Mount == 1 && Player[C].State == 1)
                        Player[C].Location.Height = Physics.PlayerHeight[1][2];
                    Player[C].Location.Y += -Player[C].Location.Height;
                    Player[C].StandUp = true;
                }
            }
            else
            {
                Player[C].Character = 2;
                if(Player[C].Mount <= 1)
                {
                    Player[C].Location.Y += Player[C].Location.Height;
                    Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                    if(Player[C].Mount == 1 && Player[C].State == 1)
                        Player[C].Location.Height = Physics.PlayerHeight[1][2];
                    Player[C].Location.Y += -Player[C].Location.Height;
                    Player[C].StandUp = true;
                }
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void onePlayer()
{
    int B = CheckLiving();

    if(B > 0)
    {
        for(int C = 1; C <= numPlayers; C++)
            Player[C].Immune = 1;

        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Bomb(Player[C].Location, iRand(2) + 2);
            }
        }

        // set the living player to get the controls if not P1
        if(B-1 < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[B-1])
            std::swap(Controls::g_InputMethods[0], Controls::g_InputMethods[B-1]);

        // delete other control methods
        while(Controls::g_InputMethods.size() > 1)
            Controls::DeleteInputMethodSlot(1);

        numPlayers = 1;
        SingleCoop = 1;
        SetupScreens();
        if(Player[B].Effect == 9)
            Player[B].Effect = 0;

        // move the living player into slot 1
        int C = 1;
        Player[C] = Player[B];
        Player[C].Character = 1;
        if(Player[C].Mount <= 1)
        {
            Player[C].Location.Y += Player[C].Location.Height;
            Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
            if(Player[C].Mount == 1 && Player[C].State == 1)
                Player[C].Location.Height = Physics.PlayerHeight[1][2];
            Player[C].Location.Y += -Player[C].Location.Height;
            Player[C].StandUp = true;
        }

        Player[C].Immune = 1;
        Player[C].Immune2 = true;
    }
}

static void twoPlayer()
{
    int B = CheckLiving();
    if(B > 0)
    {
        numPlayers = 2;

        // setup so there are exactly two controller slots,
        // activate quick-reconnect if needed
        while(Controls::g_InputMethods.size() > 2)
        {
            Controls::DeleteInputMethodSlot(2);
        }
        if(Controls::g_InputMethods.size() == 1)
        {
            Controls::g_InputMethods.push_back(nullptr);
            QuickReconnectScreen::g_active = true;
        }

        SingleCoop = 0;
        SetupScreens();

        if(Player[B].Effect == 9)
            Player[B].Effect = 0;

        Player[B].Immune = 1;
        for(int C = 1; C <= numPlayers; C++)
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
                    Player[C].Location.Y += Player[C].Location.Height;
                    Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                    if(Player[C].Mount == 1 && Player[C].State == 1)
                        Player[C].Location.Height = Physics.PlayerHeight[1][2];
                    Player[C].Location.Y += -Player[C].Location.Height;
                    Player[C].StandUp = true;
                }
            }
            else
            {
                Player[C].Character = 2;
                if(Player[C].Mount <= 1)
                {
                    Player[C].Location.Y += Player[C].Location.Height;
                    Player[C].Location.Height = Physics.PlayerHeight[Player[C].Character][Player[C].State];
                    if(Player[C].Mount == 1 && Player[C].State == 1)
                        Player[C].Location.Height = Physics.PlayerHeight[1][2];
                    Player[C].Location.Y += -Player[C].Location.Height;
                    Player[C].StandUp = true;
                }
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void warioTime()
{
    Location_t tempLocation;

    for(int B : NPCQueues::Active.no_change)
    {
        if(NPC[B].Active)
        {
            if(!NPCWontHurt[NPC[B].Type] &&
               !NPCIsABlock[NPC[B].Type] &&
               !NPCIsABonus[NPC[B].Type] &&
               !NPCIsACoin[NPC[B].Type] &&
               !NPCIsAnExit[NPC[B].Type] &&
                NPC[B].Type != NPCID_ITEM_BURIED && !NPC[B].Generator &&
               !NPC[B].Inert
            )
            {
                PlaySound(SFX_Transform);
                NPC[B].Location.Y += NPC[B].Location.Height / 2.0;
                NPC[B].Location.X += NPC[B].Location.Width / 2.0;
                tempLocation = NPC[B].Location;
                tempLocation.Y -= 16;
                tempLocation.X -= 16;
                NewEffect(EFFID_SMOKE_S3, tempLocation);
                NPC[B].Type = NPCID_COIN_S3;
                NPC[B].Location.Width = NPCWidth[NPC[B].Type];
                NPC[B].Location.Height = NPCHeight[NPC[B].Type];
                NPC[B].Location.Y += -NPC[B].Location.Height / 2.0;
                NPC[B].Location.X += -NPC[B].Location.Width / 2.0;
                NPC[B].Location.SpeedX = 0;
                NPC[B].Location.SpeedY = 0;

                treeNPCUpdate(B);
                NPCQueues::Unchecked.push_back(B);
            }
        }
    }
}

static void carKeys()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_KEY;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].HoldingPlayer = B;

            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void boingyBoing()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_SPRING;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].HoldingPlayer = B;

            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void bombsAway()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_BOMB;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.X = Player[B].Location.X;
            NPC[numNPCs].Location.Y = Player[B].Location.Y;
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].HoldingPlayer = B;

            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void fireMissiles()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead &&
           Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_BULLET;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].HoldingPlayer = B;

            syncLayers_NPC(numNPCs);
            CheckSectionNPC(numNPCs);
            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void hellFire()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_FLY_CANNON;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void upAndOut()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_FLY_BLOCK;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void powHammer()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_EARTHQUAKE_BLOCK;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void hammerInMyPants()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead &&
           Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_HEAVY_THROWER;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void rainbowRider()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead &&
           Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_FLIPPED_RAINBOW_SHELL;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Effect = 2;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].HoldingPlayer = B;
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void greenEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
            NPC[numNPCs].Special = 95;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Effect = 2;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 200;
            NPC[numNPCs].HoldingPlayer = B;
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void blueEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void yellowEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void redEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void blackEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead &&
           Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void purpleEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void pinkEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void coldEgg()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == 0)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Type = NPCID_ITEM_POD;
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
            syncLayers_NPC(numNPCs);

            Player[B].HoldingNPC = numNPCs;
            Player[B].ForceHold = 60;
            PlaySound(SFX_Grab);
        }
    }
}

static void stopHittingMe()
{
    bool ret = GodMode;
    GodMode = false;

    for(int B = 1; B <= numPlayers; B++)
        PlayerHurt(B);

    GodMode = ret;
}

static void stickyFingers()
{
    GrabAll = !GrabAll;
    PlaySound(GrabAll ? SFX_PlayerGrow : SFX_PlayerShrink);

    for(int B = 1; B <= maxPlayers; B++)
        Player[B].CanGrabNPCs = GrabAll;
}

static void capitanN()
{
    CaptainN = !CaptainN;
    PlaySound(CaptainN ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void flameThrower()
{
    FlameThrower = !FlameThrower;
    PlaySound(FlameThrower ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void moneyTree()
{
    CoinMode = !CoinMode;
    PlaySound(CoinMode ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void godMode()
{
    GodMode = !GodMode;
    PlaySound(GodMode ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void wingMan()
{
    FlyForever = !FlyForever;
    PlaySound(FlyForever ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void tooSlow()
{
    SuperSpeed = !SuperSpeed;
    PlaySound(SuperSpeed ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void ahippinAndAHopping()
{
    MultiHop = !MultiHop;
    PlaySound(MultiHop ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void frameRate()
{
    ShowFPS = !ShowFPS;
    PlaySound(ShowFPS ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void speedDemon()
{
    MaxFPS = !MaxFPS;
    PlaySound(MaxFPS ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void gifs2png()
{
    PlaySound(SFX_Transform);
    g_ForceBitmaskMerge = !g_ForceBitmaskMerge;
    XRender::unloadGifTextures();
}

static void newLeaf()
{
    GodMode = false;
    MaxFPS = false;
    ShowFPS = false;
    MultiHop = false;
    SuperSpeed = false;
    FlyForever = false;
    CoinMode = false;
    FlameThrower = false;
    CaptainN = false;
    GrabAll = false;
    ShadowMode = false;
    PlaySound(SFX_VillainKilled);
}

static void getMeOuttaHere()
{
    // in non-cheat variant, LevelMacroCounter is stuck at 0 if player never leaves section
    bool possible_softlock = (LevelMacro == LEVELMACRO_CARD_ROULETTE_EXIT && LevelMacroCounter == 0);

    if(LevelMacro != LEVELMACRO_OFF && !possible_softlock)
        return;

    LevelBeatCode = 0;
    LevelMacro = LEVELMACRO_OFF;
    LevelMacroCounter = 0;
    EndLevel = true;
}

static void holyTrinity()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Immune = 50;
        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    if(ShadowMode && GodMode && MultiHop)
    {
        ShadowMode = false;
        GodMode = false;
        MultiHop = false;
    }
    else
    {
        ShadowMode = true;
        GodMode = true;
        MultiHop = true;
    }
}

static void essentials()
{
    Location_t tempLocation;

    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Immune = 50;
        tempLocation = Player[B].Location;
        tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height / 2.0 - 16;
        tempLocation.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    if(ShadowMode && GodMode && MultiHop && SuperSpeed)
    {
        SuperSpeed = false;
        ShadowMode = false;
        GodMode = false;
        MultiHop = false;
    }
    else
    {
        SuperSpeed = true;
        ShadowMode = true;
        GodMode = true;
        MultiHop = true;
    }
}

static void foundMyCarKey()
{
    if(LevelMacro != LEVELMACRO_OFF)
        return;

    PlaySound(SFX_Key);
    StopMusic();
    LevelMacro = LEVELMACRO_KEYHOLE_EXIT;

    int player = CheckLiving();
    if(player && numBackground + numLocked < maxBackgrounds)
    {
        const Location_t& pLoc = Player[player].Location;

        // can't properly add a background because they need to be sorted, but can at least trigger the animation.
        Background_t& bgo = Background[numBackground + numLocked + 1];
        bgo = Background_t();

        bgo.Type = 35;

        bgo.Location.Width = BackgroundWidth[35];
        bgo.Location.Height = BackgroundHeight[35];
        bgo.Location.X = pLoc.X + pLoc.Width / 2 - bgo.Location.Width / 2;
        bgo.Location.Y = pLoc.Y + pLoc.Height - bgo.Location.Height;

        LevelMacroWhich = numBackground + numLocked + 1;
    }
}

static void lifeGoals()
{
    if(LevelMacro != LEVELMACRO_OFF)
        return;

    LevelMacro = LEVELMACRO_GOAL_TAPE_EXIT;
    StopMusic();
    PlaySound(SFX_TapeExit);
}

static void mysteryBall()
{
    if(LevelMacro != LEVELMACRO_OFF)
        return;

    LevelMacro = LEVELMACRO_QUESTION_SPHERE_EXIT;
    StopMusic();
    PlaySound(SFX_DungeonClear);
}

static void itsVegas()
{
    // in non-cheat variant, LevelMacroCounter is stuck at 0 if player never leaves section
    bool possible_softlock = (LevelMacro == LEVELMACRO_CARD_ROULETTE_EXIT && LevelMacroCounter == 0);

    if(LevelMacro != LEVELMACRO_OFF && !possible_softlock)
        return;

    LevelMacro = LEVELMACRO_CARD_ROULETTE_EXIT;
    // mark as cheat variant to prevent softlocks
    LevelMacroWhich = -1;
    StopMusic();
    PlaySound(SFX_CardRouletteClear);
}






struct CheatCodeDefault_t
{
    const char*key;
    void (*call)();
    bool isCheat;
};

struct CheatCode_t
{
    char key[25];
    size_t keyLen;
    void (*call)();
    bool isCheat;
};

static const CheatCodeDefault_t s_cheatsListGlobalDefault[] =
{
#ifdef ENABLE_ANTICHEAT_TRAP
    {"redigitiscool", dieCheater, false},
#else
    {"redigitiscool", redigitIsCool, false},
#endif
    {"\x77\x6f\x68\x6c\x73\x74\x61\x6e\x64\x69\x73\x74\x73\x65\x68\x72\x67\x75\x74", redigitIsCool, false},

    {"gifs2png", gifs2png, false},

    {nullptr, nullptr, false}
};


static const CheatCodeDefault_t s_cheatsListWorldDefault[] =
{
    {"imtiredofallthiswalking", moonWalk, true}, {"moonwalk", moonWalk, true}, {"skywalk", moonWalk, true},
    {"illparkwhereiwant", illParkWhereIWant, true}, {"parkinglot", illParkWhereIWant, true},
    {nullptr, nullptr, false}
};

static const CheatCodeDefault_t s_cheatsListLevelDefault[] =
{
    {"needashell", needAShell, true},
    {"fairymagic", fairyMagic, true},
    {"iceage", iceAge, true},
    {"istillplaywithlegos", iStillPlayWithLegos, true},
    {"itsrainingmen", itsRainingMen, true},
    {"donttypethis", dontTypeThis, true},
    {"wetwater", wetWater, true},
    {"needaredshell", needARedShell, true},
    {"needablueshell", needABlueShell, true},
    {"needayellowshell", needAYellowShell, true},
    {"needaturnip", needATurnip, true},
    {"needa1up", needA1Up, true},
    {"needatanookisuit", needATanookiSuit, true},
    {"needahammersuit", needAHammerSuit, true}, {"hammertime", needAHammerSuit, true},
    {"needamushroom", needAMushroom, true},
    {"needaflower", needAFlower, true},
    {"niceflower", needAnIceFlower, true},
    {"needaleaf", needALeaf, true},
    {"needanegg", needANegg, true},
    {"needaplant", needAPlant, true},
    {"needagun", needAGun, true},
    {"needaswitch", needASwitch, true},
    {"needaclock", needAClock, true},
    {"needabomb", needABomb, true},
    {"needashoe", needAShoe, true},
    {"redshoe", redShoe, true},
    {"blueshoe", blueShoe, true},
    {"shadowstar", shadowStar, true},

    {"ibakedacakeforyou", becomeAsPeach, true}, {"itsamepeach", becomeAsPeach, true},
    {"anothercastle", becomeAsToad, true}, {"itsametoad", becomeAsToad, true},
    {"iamerror", becomeAsLink, true}, {"itsamelink", becomeAsLink, true},
    {"itsamemario", becomeAsMario, true}, {"plumberboy", becomeAsMario, true}, {"moustacheman", becomeAsMario, true},
    {"itsameluigi", becomeAsLuigi, true}, {"greenmario", becomeAsLuigi, true},

    {"supermario200", superbDemo200, true},
    {"supermario128", superbDemo128, true},
    {"supermario64", superbDemo64, true},
    {"supermario32", superbDemo32, true},
    {"supermario16", superbDemo16, true},
    {"supermario8", superbDemo8, true},
    {"supermario4", superbDemo4, true},
    {"supermario2", superbDemo2, true},
    {"1player", onePlayer, true},
    {"2player", twoPlayer, true},

    {"wariotime", warioTime, true},
    {"carkeys", carKeys, true},
    {"boingyboing", boingyBoing, true},
    {"bombsaway", bombsAway, true},
    {"firemissiles", fireMissiles, true},
    {"burnthehousedown", hellFire, true}, {"hellfire", hellFire, true},
    {"upandout", upAndOut, true},
    {"powhammer", powHammer, true},
    {"hammerinmypants", hammerInMyPants, true},
    {"rainbowrider", rainbowRider, true},

    {"greenegg", greenEgg, true},
    {"blueegg", blueEgg, true},
    {"yellowegg", yellowEgg, true},
    {"redegg", redEgg, true},
    {"blackegg", blackEgg, true},
    {"purpleegg", purpleEgg, true},
    {"pinkegg", pinkEgg, true},
    {"coldegg", coldEgg, true},
    {"stophittingme", stopHittingMe, true}, {"uncle", stopHittingMe, true},
    {"stickyfingers", stickyFingers, true},
    {"captainn", capitanN, true},
    {"flamethrower", flameThrower, true},
    {"moneytree", moneyTree, true},
    {"donthurtme", godMode, true}, {"godmode", godMode, true},
    {"wingman", wingMan, true},
    {"tooslow", tooSlow, true},
    {"ahippinandahoppin", ahippinAndAHopping, true}, {"jumpman", ahippinAndAHopping, true},
    {"framerate", frameRate, false},
    {"speeddemon", speedDemon, true},

    {"getmeouttahere", getMeOuttaHere, true},
    {"newleaf", newLeaf, true},

    {"holytrinity", holyTrinity, true}, {"passerby", holyTrinity, true},
    {"essentials", essentials, true}, {"holyfour", essentials, true},

    {"foundmycarkey", foundMyCarKey, true},
    {"lifegoals", lifeGoals, true},
    {"mysteryball", mysteryBall, true},
    {"itsvegas", itsVegas, true},

    {nullptr, nullptr, false}
};


//! Current list of global cheats
static std::vector<CheatCode_t> s_cheatsListGlobal;
//! Current list of world map specific cheats
static std::vector<CheatCode_t> s_cheatsListWorld;
//! Current list of level specific cheats
static std::vector<CheatCode_t> s_cheatsListLevel;


/*!
 * \brief hasQWERTZ
 * \param s Checks does string is suitable for conversion into QWERTZ
 * \return true if string can be converted into QWERTZ
 */
SDL_FORCE_INLINE bool hasQWERTZ(const std::string &s)
{
    for(const char &c : s)
    {
        switch(c)
        {
        case 'y':
        case 'z':
            return true;
        default:
            continue;
        }
    }

    return false;
}

/*!
 * \brief convert string into QWERTZ
 * \param s Source string in QWERTY
 * \return QWERTZ string
 */
SDL_FORCE_INLINE std::string toQWERTZ(std::string s)
{
    for(char &c : s)
    {
        switch(c)
        {
        case 'y':
            c = 'z';
            break;
        case 'z':
            c = 'y';
            break;
        default:
            continue;
        }
    }

    return s;
}

/*!
 * \brief hasAZERTY
 * \param s Checks does string is suitable for conversion into AZERTY
 * \return true if string can be converted into AZERTY
 */
SDL_FORCE_INLINE bool hasAZERTY(const std::string &s)
{
    for(const char &c : s)
    {
        switch(c)
        {
        case 'q':
        case 'w':
        case 'a':
        case ';':
        case 'z':
        case 'm':
            return true;
        default:
            continue;
        }
    }

    return false;
}

/*!
 * \brief convert string into AZERTY
 * \param s Source string in AZERTY
 * \return AZERTY string
 */
SDL_FORCE_INLINE std::string toAZERTY(std::string s)
{
    for(char &c : s)
    {
        switch(c)
        {
        case 'q':
            c = 'a';
            break;
        case 'w':
            c = 'z';
            break;
        case 'a':
            c = 'q';
            break;
        case 'z':
            c = 'w';
            break;
        case 'm':
            c = ';';
            break;
        default:
            continue;
        }
    }

    return s;
}

SDL_FORCE_INLINE void convertArray(std::vector<CheatCode_t> &dst, const CheatCodeDefault_t *src)
{
    dst.clear();

    while(src->key && src->call)
    {
        CheatCode_t cd = {};
        SDL_memset(cd.key, 0, sizeof(cd.key));
        SDL_strlcpy(cd.key, src->key, sizeof(cd.key));
        cd.keyLen = SDL_strlen(cd.key);
        cd.call = src->call;
        cd.isCheat = src->isCheat;
        dst.push_back(cd);

        if(hasQWERTZ(src->key)) // Automatically add QWERTZ alias
        {
            std::string z = toQWERTZ(src->key);
            SDL_memset(cd.key, 0, sizeof(cd.key));
            SDL_strlcpy(cd.key, z.c_str(), SDL_min(sizeof(cd.key), z.size() + 1));
            cd.keyLen = SDL_strlen(cd.key);
            dst.push_back(cd);
        }

        if(hasAZERTY(src->key)) // Automatically add AZERTY alias
        {
            std::string z = toAZERTY(src->key);
            SDL_memset(cd.key, 0, sizeof(cd.key));
            SDL_strlcpy(cd.key, z.c_str(), SDL_min(sizeof(cd.key), z.size() + 1));
            cd.keyLen = SDL_strlen(cd.key);
            dst.push_back(cd);
        }

        ++src;
    }
}

SDL_FORCE_INLINE void addAliasCheats(CheatsScope scope, std::vector<GameInfo::CheatAlias> &list)
{
    for(const auto &c : list)
    {
        cheats_addAlias(scope, c.first, c.second);
        if(hasQWERTZ(c.second)) // Add QWERTZ version for new string
            cheats_addAlias(scope, c.first, toQWERTZ(c.second));
        if(hasAZERTY(c.second)) // Add AZERTY version for new string
            cheats_addAlias(scope, c.first, toAZERTY(c.second));
    }
}

SDL_FORCE_INLINE void addRenameCheats(CheatsScope scope, std::vector<GameInfo::CheatAlias> &list)
{
    for(const auto &c : list)
    {
        cheats_rename(scope, c.first, c.second);

        if(hasQWERTZ(c.first)) // Remove no longer relevant QWERTZ version of string
            cheats_erase(scope, toQWERTZ(c.first));
        if(hasAZERTY(c.first)) // Remove no longer relevant AZERTY version of string
            cheats_erase(scope, toAZERTY(c.first));

        if(hasQWERTZ(c.second)) // Add QWERTZ version for new string if needed
            cheats_addAlias(scope, c.second, toQWERTZ(c.second));
        if(hasAZERTY(c.second)) // Add AZERTY version for new string if needed
            cheats_addAlias(scope, c.second, toAZERTY(c.second));
    }
}

void cheats_reset()
{
    convertArray(s_cheatsListGlobal, s_cheatsListGlobalDefault);
    convertArray(s_cheatsListWorld, s_cheatsListWorldDefault);
    convertArray(s_cheatsListLevel, s_cheatsListLevelDefault);

    addAliasCheats(CHEAT_SCOPE_GLOBAL,  g_gameInfo.cheatsGlobalAliases);
    addRenameCheats(CHEAT_SCOPE_GLOBAL, g_gameInfo.cheatsGlobalRenames);

    addAliasCheats(CHEAT_SCOPE_WORLD,  g_gameInfo.cheatsWorldAliases);
    addRenameCheats(CHEAT_SCOPE_WORLD, g_gameInfo.cheatsWorldRenames);

    addAliasCheats(CHEAT_SCOPE_LEVEL,  g_gameInfo.cheatsLevelAliases);
    addRenameCheats(CHEAT_SCOPE_LEVEL, g_gameInfo.cheatsLevelRenames);
}

void cheats_addAlias(CheatsScope scope, const std::string &source, const std::string &alias)
{
    std::vector<CheatCode_t> *dst = nullptr;

    switch(scope)
    {
    case CHEAT_SCOPE_GLOBAL:
        dst = &s_cheatsListGlobal;
        break;
    case CHEAT_SCOPE_WORLD:
        dst = &s_cheatsListWorld;
        break;
    case CHEAT_SCOPE_LEVEL:
        dst = &s_cheatsListLevel;
        break;
    }

    SDL_assert(dst);

    for(auto &c : *dst)
    {
        if(source == c.key)
        {
            auto cc = c;
            SDL_memset(cc.key, 0, sizeof(cc.key));
            SDL_strlcpy(cc.key, alias.c_str(), SDL_min(sizeof(cc.key), alias.size() + 1));
            cc.keyLen = SDL_strlen(cc.key);
            dst->push_back(cc);
            break;
        }
    }
}

void cheats_rename(CheatsScope scope, const std::string &source, const std::string &alias)
{
    std::vector<CheatCode_t> *dst = nullptr;

    switch(scope)
    {
    case CHEAT_SCOPE_GLOBAL:
        dst = &s_cheatsListGlobal;
        break;
    case CHEAT_SCOPE_WORLD:
        dst = &s_cheatsListWorld;
        break;
    case CHEAT_SCOPE_LEVEL:
        dst = &s_cheatsListLevel;
        break;
    }

    SDL_assert(dst);

    for(auto &c : *dst)
    {
        if(source == c.key)
        {
            SDL_memset(c.key, 0, sizeof(c.key));
            SDL_strlcpy(c.key, alias.c_str(), SDL_min(sizeof(c.key), alias.size() + 1));
            c.keyLen = SDL_strlen(c.key);
            break;
        }
    }

}

void cheats_erase(CheatsScope scope, const std::string &source)
{
    std::vector<CheatCode_t> *dst = nullptr;

    switch(scope)
    {
    case CHEAT_SCOPE_GLOBAL:
        dst = &s_cheatsListGlobal;
        break;
    case CHEAT_SCOPE_WORLD:
        dst = &s_cheatsListWorld;
        break;
    case CHEAT_SCOPE_LEVEL:
        dst = &s_cheatsListLevel;
        break;
    }

    SDL_assert(dst);

    for(auto it = dst->begin(); it != dst->end(); ++it)
    {
        if(source == it->key)
        {
            dst->erase(it);
            break;
        }
    }
}


struct CheatBuffer_t
{
    static const size_t maxLen = 25;
    char buffer[2][maxLen + 1] = {};
    size_t t = 0;
    size_t bufLen = 0;

    void clear()
    {
        buffer[0][0] = 0;
        buffer[1][0] = 0;
        bufLen = 0;
    }

    void setBuffer(const std::string &line)
    {
        bufLen = SDL_min(maxLen, line.size());
        SDL_memcpy(buffer[t], line.c_str(), bufLen);
        buffer[t][bufLen] = 0;
    }

    void addSym(char c)
    {
        if(bufLen < maxLen)
        {
            buffer[t][bufLen++] = c;
            buffer[t][bufLen] = 0;
        }
        else
        {
            size_t ts = t,
                   td = !t;
            SDL_memcpy(buffer[td], buffer[ts] + 1, bufLen - 1);
            t = td;
            buffer[t][bufLen - 1] = c;
            buffer[t][bufLen] = 0;
        }
    }

    const char *getString()
    {
        if(bufLen == 0)
            return "";
        return buffer[t];
    }

    size_t getBufLen() const
    {
        return bufLen;
    }
};

static CheatBuffer_t s_buffer;


SDL_FORCE_INLINE bool cheatCompare(size_t bufLen, const char *buf,
                                   size_t keyLen, const char *key)
{
    if(bufLen < keyLen)
        return false;

    return SDL_memcmp(buf + (bufLen - keyLen), key, keyLen) == 0;
}

static void processCheats()
{
    std::string oldString;
    const char *buf = s_buffer.getString();
    auto bufLen = s_buffer.getBufLen();
    bool cheated = false;

    // D_pLogDebug("Cheat buffer [%s]\n", buf);

    for(const auto &c : s_cheatsListGlobal)
    {
        if(!cheatCompare(bufLen, buf, c.keyLen, c.key))
            continue;

        c.call();
        oldString = buf;
        s_buffer.clear();
        cheated = c.isCheat;
        break;
    }

    for(const auto &c : (LevelSelect ? s_cheatsListWorld : s_cheatsListLevel))
    {
        if(!cheatCompare(bufLen, buf, c.keyLen, c.key))
            continue;

        c.call();
        oldString = buf;
        s_buffer.clear();
        cheated = c.isCheat;
        break;
    }

    if(cheated)
    {
        pLogDebug("Cheating detected!!! [%s]\n", oldString.c_str());
        Cheater = true;
    }
}

void cheats_setBuffer(const std::string &line)
{
    s_buffer.setBuffer(line);
    processCheats();
}

void cheats_clearBuffer()
{
    s_buffer.clear();
}

/*!
 * \brief Process the cheat buffer
 * \param sym New key symbol
 */
void CheatCode(char sym)
{
    if(LevelEditor || GameMenu || /*nPlay.Online ||*/ BattleMode)
    {
        s_buffer.clear();
        return;
    }

    s_buffer.addSym(sym);

    processCheats();
}

bool cheats_contains(const std::string &needle)
{
    const char *buf = s_buffer.getString();
    auto bufLen = s_buffer.getBufLen();
    return cheatCompare(bufLen, buf, needle.size(), needle.c_str());
}

bool cheats_contains(const char *needle)
{
    const char *buf = s_buffer.getString();
    auto bufLen = s_buffer.getBufLen();
    return cheatCompare(bufLen, buf, SDL_strlen(needle), needle);
}

std::string cheats_get()
{
    return std::string(s_buffer.getString(), s_buffer.getBufLen());
}
