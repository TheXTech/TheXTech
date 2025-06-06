/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"
#include "sound.h"
#include "graphics.h"
#include "collision.h"
#include "effect.h"
#include "player.h"
#include "npc.h"
#include "layers.h"
#include "controls.h"
#include "game_main.h"
#include "change_res.h"
#include "config.h"
#ifdef ENABLE_ANTICHEAT_TRAP
#include "editor.h" // For the `Backup_FullFileName` only
#endif
#include "frm_main.h"

#include "core/render.h"

#include "npc/npc_queues.h"

#include "main/game_info.h"
#include "main/screen_quickreconnect.h"
#include "main/screen_textentry.h"

#include "main/trees.h"

#include "main/cheat_code.h"

#include "npc_id.h"
#include "npc_traits.h"
#include "eff_id.h"

bool g_ForceBitmaskMerge = false;
bool g_CheatLogicScreen = false;
int g_CheatEditYourFriends = 0;

static void redigitIsCool()
{
    PlaySound(SFX_SMCry);
    Cheater = false;
}


#ifdef ENABLE_ANTICHEAT_TRAP
static void dieCheater()
{
    if(LevelEditor || WorldEditor || BattleMode || TestLevel || !Backup_FullFileName.empty())
    {
        // Don't perform the punish when running a level test or the editor
        redigitIsCool();
        return;
    }

    pLogCritical("redigitiscool code was been used, player got a punish!");
    PlaySound(SFX_SMExplosion);
    Score = 0; // Being very evil here, mu-ha-ha-ha-ha! >:D
    Lives = 0;
    g_100s = 0;
    Coins = 0;
    GodMode = false;
    ClearGame(true); // As a penalty, remove the saved game
    Cheater = true;
    CheaterMustDie = true;
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

        if(!g_config.unlimited_framerate)
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
 * \brief Opens all exits for current world level
 */
static void openSesame()
{
    bool found = false;

    for(WorldLevel_t &lev : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_ID))
    {
        if(CheckCollision(WorldPlayer[1].Location, lev.Location))
        {
            found = true;

            for(int A = 1; A <= 4; A++)
                LevelPath(lev, A);
        }
    }

    if(!found)
        PlaySound(SFX_BlockHit);
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
        Player[B].HeldBonus = NPCID_GRN_SHELL_S4;
}

static void fairyMagic()
{
    if(Player[1].Fairy)
    {
        for(int B = 1; B <= numPlayers; B++)
        {
            PlaySound(SFX_HeroFairy);
            Player[B].Immune = 10;
            Player[B].Effect = PLREFF_WAITING;
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
            Player[B].Effect = PLREFF_WAITING;
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
    for(int C : NPCQueues::Active.may_insert_erase)
    {
        if(NPC[C].Active)
        {
            if(!NPC[C]->NoIceBall && NPC[C].Type != NPCID_ICE_CUBE && !NPC[C]->IsABonus)
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

    for(int B = 0; B < numSections; B++)
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

    for(int i = 0; i < l_screen->player_count; i++)
    {
        int B = l_screen->players[i];

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
        Player[B].HeldBonus = NPCID_RED_SHELL_S4;
}

static void needABlueShell()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = NPCID_BLU_SHELL_S4;
}

static void needAYellowShell()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = NPCID_YEL_SHELL_S4;
}

static void needATurnip()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = NPCID_VEGGIE_1;
}

static void needA1Up()
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = NPCID_LIFE_S3;
}

static void grantPowerup(NPCID npcid, PlayerState state)
{
    PlaySound(SFX_GotItem);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = npcid;

        if(Player[B].Character >= 3 && Player[B].State != state)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = PLREFF_WAITING;
            Player[B].Effect2 = 4;
            Player[B].State = state;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3_CENTER, Player[B].Location);
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needATanookiSuit()
{
    grantPowerup(NPCID_STATUE_POWER, PLR_STATE_STATUE);
}

static void needAHammerSuit()
{
    grantPowerup(NPCID_HEAVY_POWER, PLR_STATE_HEAVY);
}

static void needAMushroom()
{
    PlaySound(SFX_GotItem);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].HeldBonus = NPCID_POWER_S3;

        if(Player[B].Character >= 3 && Player[B].State == 1)
        {
            PlaySound(SFX_Transform);
            Player[B].Immune = 30;
            Player[B].Effect = PLREFF_WAITING;
            Player[B].Effect2 = 4;
            Player[B].State = 2;
            SizeCheck(Player[B]);
            NewEffect(EFFID_SMOKE_S3_CENTER, Player[B].Location);
        }

        if(Player[B].Character >= 3 && Player[B].Hearts < 3)
            Player[B].Hearts += 1;
    }
}

static void needAFlower()
{
    grantPowerup(NPCID_FIRE_POWER_S3, PLR_STATE_FIRE);
}

static void needAnIceFlower()
{
    grantPowerup(NPCID_ICE_POWER_S3, PLR_STATE_ICE);
}

static void needALeaf()
{
    grantPowerup(NPCID_LEAF_POWER, PLR_STATE_LEAF);
}

static void needACyclone()
{
    grantPowerup(NPCID_CYCLONE_POWER, PLR_STATE_CYCLONE);
}

static void goodTimesRoll()
{
    grantPowerup(NPCID_SHELL_POWER, PLR_STATE_SHELL);
}

static void wayPastCool()
{
    grantPowerup(NPCID_POLAR_POWER, PLR_STATE_POLAR);
}

static void sushiTime()
{
    grantPowerup(NPCID_AQUATIC_POWER, PLR_STATE_AQUATIC);
}

static void grantItemBox(NPCID npcid)
{
    PlaySound(SFX_GotItem);
    for(int B = 1; B <= numPlayers; B++)
        Player[B].HeldBonus = npcid;
}

static void needANegg()
{
    grantItemBox(NPCID_ITEM_POD);
}

static void needAPlant()
{
    grantItemBox(NPCID_TOOTHYPIPE);
}

static void needAGun()
{
    grantItemBox(NPCID_CANNONITEM);
}

static void needASwitch()
{
    grantItemBox(NPCID_COIN_SWITCH);
}

static void needAClock()
{
    grantItemBox(NPCID_TIMER_S3);
}

static void needABomb()
{
    grantItemBox(NPCID_WALK_BOMB_S2);
}

static void needAShoe()
{
    grantItemBox(NPCID_GRN_BOOT);
}

static void redShoe()
{
    grantItemBox(NPCID_RED_BOOT);
}

static void blueShoe()
{
    grantItemBox(NPCID_BLU_BOOT);
}

static void shadowStar()
{
    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Immune = 50;
        NewEffect(EFFID_SMOKE_S3_CENTER, Player[B].Location);
    }

    ShadowMode = !ShadowMode;

    //if(ShadowMode)
    //    ShadowMode = false;
    //else
    //    ShadowMode = true;
}

static void s_heightFix(Player_t& p)
{
    if(p.Mount <= 1)
    {
        int new_height = Physics.PlayerHeight[p.Character][p.State];
        if(p.Mount == 1 && p.State == 1)
            new_height = Physics.PlayerHeight[1][2];

        p.Location.set_height_floor(new_height);
        p.StandUp = true;
    }
}

static void changeCharCheat(int new_char)
{
    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Character = new_char;
        Player[B].Immune = 50;

        s_heightFix(Player[B]);

        NewEffect(EFFID_SMOKE_S3_CENTER, Player[B].Location);
    }

    UpdateYoshiMusic();
}

static void becomeAsPeach()
{
    changeCharCheat(3);
}

static void becomeAsToad()
{
    changeCharCheat(4);
}

static void becomeAsLink()
{
    changeCharCheat(5);
}

static void becomeAsMario()
{
    changeCharCheat(1);
}

static void becomeAsLuigi()
{
    changeCharCheat(2);
}

static int findLivingForCheat()
{
    if(!g_ClonedPlayerMode && numPlayers != Screens[0].player_count)
    {
        PlaySound(SFX_BlockHit);
        return false;
    }

    return CheckLiving();
}

static void setScreenPlayers(int count)
{
    Screens[0].player_count = count;
    Screens[0].players = {1, 2, 3, 4};
    UpdateScreenPlayers();
}

static void superbDemo200()
{
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 200;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;
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
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 128;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;
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
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 64;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;
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
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 32;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

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
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 16;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

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
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 8;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

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
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 4;
        g_ClonedPlayerMode = true;
        setScreenPlayers(1);

        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

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
    int B = findLivingForCheat();

    if(B > 0)
    {
        g_ClonedPlayerMode = false;
        numPlayers = 2;
        SingleCoop = 1;
        setScreenPlayers(2);

        SetupScreens();
        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;
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
                s_heightFix(Player[C]);
            }
            else
            {
                Player[C].Character = 2;
                s_heightFix(Player[C]);
            }
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void onePlayer()
{
    int B = findLivingForCheat();

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
        if(!g_ClonedPlayerMode && !SingleCoop && B - 1 < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[B-1])
            std::swap(Controls::g_InputMethods[0], Controls::g_InputMethods[B-1]);

        // delete other control methods
        while(Controls::g_InputMethods.size() > 1)
            Controls::DeleteInputMethodSlot(1);

        numPlayers = 1;
        SingleCoop = 1;
        g_ClonedPlayerMode = false;

        setScreenPlayers(1);
        SetupScreens();
        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

        // move the living player into slot 1
        int C = 1;
        Player[C] = Player[B];
        Player[C].Character = 1;
        s_heightFix(Player[C]);

        Player[C].Immune = 1;
        Player[C].Immune2 = true;
    }
}

static void twoPlayer()
{
    int B = findLivingForCheat();
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
        g_ClonedPlayerMode = false;
        setScreenPlayers(2);
        SetupScreens();

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

        Player[B].Immune = 1;
        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * -12;
            }

            Player[C].Character = C;
            s_heightFix(Player[C]);
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void fourPlayer()
{
    int B = findLivingForCheat();
    if(B > 0)
    {
        numPlayers = 4;

        // setup so there are exactly two controller slots,
        // activate quick-reconnect if needed
        while(Controls::g_InputMethods.size() > 4)
        {
            Controls::DeleteInputMethodSlot(4);
        }
        while(Controls::g_InputMethods.size() < 4)
        {
            Controls::g_InputMethods.push_back(nullptr);
            QuickReconnectScreen::g_active = true;
        }

        SingleCoop = 0;
        g_ClonedPlayerMode = false;
        setScreenPlayers(4);

        if(Player[B].Effect == PLREFF_NO_COLLIDE)
            Player[B].Effect = PLREFF_NORMAL;

        Player[B].Immune = 1;
        for(int C = 1; C <= numPlayers; C++)
        {
            if(C != B)
            {
                Player[C] = Player[B];
                Player[C].Location.SpeedY = dRand() * -12;
            }

            Player[C].Character = C;

            if(Player[C].Character > 2 && Player[C].Mount == 3)
            {
                Player[C].Mount = 1;
                Player[C].MountType = 1;
            }

            s_heightFix(Player[C]);
        }

        Bomb(Player[B].Location, iRand(2) + 2);
    }
}

static void fourShared()
{
    fourPlayer();

    {
        ConfigChangeSentinel sent(ConfigSetLevel::cheat);
        g_config.four_screen_mode = MultiplayerPrefs::Shared;
    }
}

static void fourSplit()
{
    fourPlayer();

    {
        ConfigChangeSentinel sent(ConfigSetLevel::cheat);
        g_config.four_screen_mode = MultiplayerPrefs::Split;
    }
}

static void warioTime()
{
    for(int B : NPCQueues::Active.no_change)
    {
        if(NPC[B].Active)
        {
            if(!NPC[B]->WontHurt &&
               !NPC[B]->IsABlock &&
               !NPC[B]->IsABonus &&
               !NPC[B]->IsACoin &&
               !NPCIsAnExit(NPC[B]) &&
                NPC[B].Type != NPCID_ITEM_BURIED && !NPC[B].Generator &&
               !NPC[B].Inert
            )
            {
                PlaySound(SFX_Transform);
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[B].Location);
                NPC[B].Type = NPCID_COIN_S3;
                NPC[B].Location.set_width_center(NPC[B]->TWidth);
                NPC[B].Location.set_height_center(NPC[B]->THeight);
                NPC[B].Location.SpeedX = 0;
                NPC[B].Location.SpeedY = 0;

                treeNPCUpdate(B);
                NPCQueues::Unchecked.push_back(B);
            }
        }
    }
}

static void grantItemHeld(NPCID npcid, NPCEffect eff = NPCEFF_NORMAL, int special = 0)
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 0 && !Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Effect == PLREFF_NORMAL)
        {
            numNPCs++;
            NPC[numNPCs].Type = npcid;
            NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
            NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
            NPC[numNPCs].Location.X = Player[B].Location.X;
            NPC[numNPCs].Location.Y = Player[B].Location.Y;
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
            NPC[numNPCs].Effect = eff;
            NPC[numNPCs].Special = special;

            // for the pods
            if(special)
                NPCFrames(numNPCs);

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

static void carKeys()
{
    grantItemHeld(NPCID_KEY);
}

static void boingyBoing()
{
    grantItemHeld(NPCID_SPRING);
}

static void bombsAway()
{
    grantItemHeld(NPCID_BOMB);
}

static void fireMissiles()
{
    grantItemHeld(NPCID_BULLET);
}

static void hellFire()
{
    grantItemHeld(NPCID_FLY_CANNON);
}

static void upAndOut()
{
    grantItemHeld(NPCID_FLY_BLOCK);
}

static void powHammer()
{
    grantItemHeld(NPCID_EARTHQUAKE_BLOCK);
}

static void hammerInMyPants()
{
    grantItemHeld(NPCID_HEAVY_THROWER);
}

static void rainbowRider()
{
    grantItemHeld(NPCID_FLIPPED_RAINBOW_SHELL, NPCEFF_DROP_ITEM);
}

static void cantTouchThis()
{
    grantItemHeld(NPCID_INVINCIBILITY_POWER);
}

static void greenEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_GREEN);
}

static void blueEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_BLUE);
}

static void yellowEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_YELLOW);
}

static void redEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_RED);
}

static void blackEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_BLACK);
}

static void purpleEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_PURPLE);
}

static void pinkEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_PINK);
}

static void coldEgg()
{
    grantItemHeld(NPCID_ITEM_POD, NPCEFF_DROP_ITEM, NPCID_PET_CYAN);
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
    ConfigChangeSentinel sent(ConfigSetLevel::cheat);

    g_config.show_fps = !g_config.show_fps;
    PlaySound(g_config.show_fps ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void speedDemon()
{
    ConfigChangeSentinel sent(ConfigSetLevel::cheat);

    g_config.unlimited_framerate = !g_config.unlimited_framerate;
    PlaySound(g_config.unlimited_framerate ? SFX_PlayerGrow : SFX_PlayerShrink);

    if(g_config.compatibility_mode != Config_t::COMPAT_OFF && g_config.compatibility_mode != Config_t::COMPAT_MODERN)
    {
        pLogDebug("Marking Cheater by unlimited framerate cheat code");
        Cheater = true;
    }
}

static void gifs2png()
{
    PlaySound(SFX_Transform);

#ifdef __3DS__
    ConfigChangeSentinel sent(ConfigSetLevel::cheat);
    g_config.inaccurate_gifs = !g_config.inaccurate_gifs;
#else
    g_ForceBitmaskMerge = !g_ForceBitmaskMerge;
    XRender::unloadGifTextures();
#endif
}

static void logicScreen()
{
    PlaySound(SFX_PSwitch);
    g_CheatLogicScreen = !g_CheatLogicScreen;
}

static void editYourFriends()
{
    g_CheatEditYourFriends = 2;
    PauseGame(PauseCode::Options);
}

static void bornToClimb()
{
    CanWallJump = !CanWallJump;
    PlaySound(CanWallJump ? SFX_PlayerGrow : SFX_PlayerShrink);
}

static void newLeaf()
{
    if(g_config.show_fps.m_set == ConfigSetLevel::cheat || g_config.unlimited_framerate.m_set == ConfigSetLevel::cheat)
    {
        g_config.show_fps.unset();
        g_config.unlimited_framerate.unset();
        UpdateConfig();
    }

    GodMode = false;
    MultiHop = false;
    SuperSpeed = false;
    FlyForever = false;
    CoinMode = false;
    FlameThrower = false;
    CaptainN = false;
    GrabAll = false;
    ShadowMode = false;
    CanWallJump = false;
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
    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Immune = 50;
        NewEffect(EFFID_SMOKE_S3_CENTER, Player[B].Location);
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
    PlaySound(SFX_Transform);

    for(int B = 1; B <= numPlayers; B++)
    {
        Player[B].Immune = 50;
        NewEffect(EFFID_SMOKE_S3_CENTER, Player[B].Location);
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
        bgo.Location.X = pLoc.X + (pLoc.Width - bgo.Location.Width) / 2;
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

static void setRes(int w, int h)
{
    ConfigChangeSentinel sent(ConfigSetLevel::cheat);

    g_config.internal_res = {w, h};
}

static void setResGb()
{
    setRes(320, 288);
}

static void setResGba()
{
    setRes(480, 320);
}

static void setResNds()
{
    setRes(512, 384);
}

static void setResSnes()
{
    setRes(512, 448);
}

static void setResVga()
{
    setRes(640, 480);
}

static void setResHello()
{
    setRes(768, 432);
}

static void setRes3ds()
{
    setRes(800, 480);
}

static void setResClassic()
{
    setRes(800, 600);
}

static void setResHD()
{
    setRes(1280, 720);
}

static void setResDyn()
{
    setRes(0, 0);
}

static void setResCustom()
{
    int w, h;

    std::string s = TextEntryScreen::Run("Game width:");
    if(s.empty())
        return;

    w = 0; // just to suppress an unneeded warning

    while(s != "dyn" && (w = atol(s.c_str())) <= 0)
    {
        s = TextEntryScreen::Run("Invalid input. Game width:");
        if(s.empty())
            return;
    }

    if(s == "dyn")
        w = 0;

    s = TextEntryScreen::Run("Game height:");
    if(s.empty())
        return;

    while((h = atol(s.c_str())) <= 0)
    {
        s = TextEntryScreen::Run("Invalid input. Game height:");
        if(s.empty())
            return;
    }

    setRes(w, h);
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

    // cheat to show the logical screens
    {"logicscreen", logicScreen, false},

    // cheat to allow editing any setting
    {"edityourfriends", editYourFriends, false},

    // resolution cheats
    {"gameboyview", setResGb, false},
    {"tinyview", setResGba, false},
    {"gbaview", setResGba, false},
    {"superbdemoadvance", setResGba, false},
    {"ndsview", setResNds, false},
    {"snesview", setResSnes, false},
    {"vgaview", setResVga, false},
    {"helloview", setResHello, false},
    {"3dsview", setRes3ds, false},
    {"smbxview", setResClassic, false},
    {"aodview", setResClassic, false},
    {"classicview", setResClassic, false},
    {"hdview", setResHD, false},
    {"dynview", setResDyn, false},
    {"debugview", setResCustom, false},
    {"customview", setResCustom, false},

    {nullptr, nullptr, false}
};


static const CheatCodeDefault_t s_cheatsListWorldDefault[] =
{
    {"imtiredofallthiswalking", moonWalk, true}, {"moonwalk", moonWalk, true}, {"skywalk", moonWalk, true},
    {"illparkwhereiwant", illParkWhereIWant, true}, {"parkinglot", illParkWhereIWant, true},
    {"4shared", fourShared, true},
    {"4split", fourSplit, true},
    {"opensesame", openSesame, true},
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
    {"4shared", fourShared, true},
    {"4split", fourSplit, true},

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
    {"speeddemon", speedDemon, false},

    {"getmeouttahere", getMeOuttaHere, true},
    {"newleaf", newLeaf, true},
    {"borntoclimb", bornToClimb, true},
    {"needacyclone", needACyclone, true},
    {"goodtimesroll", goodTimesRoll, true},
    {"canttouchthis", cantTouchThis, true},
    {"waypastcool", wayPastCool, true},
    {"sushitime", sushiTime, true},

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

void run_cheat(XMessage::Message message)
{
    int set = message.player;

    const auto& cheat_list = (set == 0) ? s_cheatsListGlobal
                            : (set == 1) ? s_cheatsListLevel
                            : s_cheatsListWorld;

    if(set == 1 && LevelSelect)
        return;
    else if(set == 2 && !LevelSelect)
        return;

    if(message.message >= cheat_list.size())
        return;

    const auto& c = cheat_list[message.message];

    c.call();

    if(c.isCheat)
    {
        pLogDebug("Cheating detected!!! [%s]\n", c.key);
        Cheater = true;
    }
}

static void processCheats(bool instant)
{
    const char *buf = s_buffer.getString();
    auto bufLen = s_buffer.getBufLen();

    for(uint8_t set = 0; set < 3; set++)
    {
        const auto& cheat_list = (set == 0) ? s_cheatsListGlobal
                                : (set == 1) ? s_cheatsListLevel
                                : s_cheatsListWorld;

        for(size_t i = 0; i < cheat_list.size(); i++)
        {
            const auto& c = cheat_list[i];

            if(!cheatCompare(bufLen, buf, c.keyLen, c.key))
                continue;

            XMessage::Message cheat_message{XMessage::Type::enter_code, set, (uint8_t)i};

            if(instant)
                run_cheat(cheat_message);
            else
                XMessage::PushMessage(cheat_message);

            s_buffer.clear();
            break;
        }
    }
}

void cheats_setBuffer(const std::string &line, bool instant)
{
    s_buffer.setBuffer(line);
    processCheats(instant);
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

    processCheats(false);
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
