/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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


#include <Logger/logger.h>

#include "../globals.h"
#include "../game_main.h"
#include "../core/render.h"
#include "../controls.h"
#include "../npc.h"
#include "../blocks.h"
#include "../collision.h"
#include "../effect.h"
#include "../player.h"
#include "../graphics.h"
#include "../sound.h"
#include "../main/trees.h"
#include "game_info.h"


void DoCredits(bool quit)
{
    if(GameMenu)
        return;

    int screenH_half = ScreenH / 2;

    CreditOffsetY -= 0.8;
//    if(CreditOffsetY > ScreenH || CreditOffsetY + CreditTotalHeight < 0)
//    {}

    // Closing screen
    if((CreditOffsetY + CreditTotalHeight) < -100)
    {
        if(musicPlaying)
        {
            FadeOutMusic(11000);
            musicPlaying = false;
        }

        CreditChop += 0.4f;
        if(CreditChop >= static_cast<float>(screenH_half))
        {
            CreditChop = static_cast<float>(screenH_half);
            EndCredits++;
            if(EndCredits == screenH_half)
            {
                SetupCredits();
                GameOutroDoQuit = true;
                GameMenu = true;
            }
        }
        else
            EndCredits = 0;
    }

        // Opening screen
    else if(CreditChop > 100 && CreditOffsetY + CreditTotalHeight > 0)
    {
        CreditChop -= 2.0f;
        if(CreditChop < 100)
            CreditChop = 100;

        if(CreditChop < 250 && !musicPlaying)
        {
            if(bgMusic[0] <= 0) // Play default music if no music set in outro level
            {
                musicName = "tmusic";
                PlayMusic("tmusic", 2000);
                musicPlaying = true;
            }
            else // Otherwise, play the music that set by level
                StartMusic(0, 2000);
        }
    }

    if(CreditChop <= 100 || EndCredits > 0)
    {
//        for(A = 1; A <= 2; A++) // Useless loop
//        {
        if(quit)
        {
            CreditChop = static_cast<float>(screenH_half);
            EndCredits = 0;
            XRender::clearBuffer();
            SetupCredits();
            GameMenu = true;
            GameOutroDoQuit = true;
        }
//        }
    }
}

void OutroLoop()
{
    Controls::Update();
    bool quit = SharedControls.QuitCredits;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        quit |= Player[i+1].Controls.Start;
    }

    if(g_gameInfo.outroDeadMode)
    {
        UpdateNPCs();
        UpdateBlocks();
        UpdateEffects();
        // UpdatePlayer();
        DoCredits(quit);
        UpdateGraphics();
        UpdateSound();

        if(GameOutroDoQuit) // Don't unset the GameOutro before GFX update, otherwise a glitch will happen
        {
            GameOutro = false;
            GameOutroDoQuit = false;
        }
        return;
    }

    for(int A = 1; A <= numPlayers; A++)
    {
        auto &pp = Player[A];
        pp.Controls = Controls_t();

        if(g_gameInfo.outroWalkDirection < 0)
            pp.Controls.Left = true;
        else if(g_gameInfo.outroWalkDirection > 0)
            pp.Controls.Right = true;

        if(pp.Controls.Left)
            pp.Direction = -1;
        if(pp.Controls.Right)
            pp.Direction = 1;

        Location_t tempLocation = pp.Location;
        // tempLocation = pp.Location; // Why here was the duplicated location assignment?
        tempLocation.SpeedX = 0;
        tempLocation.SpeedY = 0;
        double pp_bottom = pp.Location.Y + pp.Location.Height;
        tempLocation.Y = pp_bottom - 8;
        tempLocation.Height = pp.Mount == 1 ? 50 : 25;
        tempLocation.Width = 16;

        if(pp.Location.SpeedX > 0)
            tempLocation.X = (pp.Location.X + pp.Location.Width) + 20;
        else
            tempLocation.X = pp.Location.X - (tempLocation.Width + 20);

        if(g_gameInfo.outroAutoJump)
        {
            // int64_t fBlock = 0;
            // int64_t lBlock = 0;
            // fBlock = FirstBlock[long(tempLocation.X / 32) - 1];
            // lBlock = LastBlock[long((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            // blockTileGet(tempLocation, fBlock, lBlock);
            // if(!BlocksSorted)
            // {
            //     fBlock = 1;
            //     lBlock = numBlock;
            // }

            bool doJump = true;

            for(const Block_t& bb : treeBlockQuery(tempLocation, SORTMODE_NONE))
            {
//                if(tempLocation.X + tempLocation.Width >= Block[B].Location.X &&
//                   tempLocation.X <= Block[B].Location.X + Block[B].Location.Width &&
//                   tempLocation.Y + tempLocation.Height >= Block[B].Location.Y &&
//                   tempLocation.Y <= Block[B].Location.Y + Block[B].Location.Height)
                if(CheckCollision(tempLocation, bb.Location))
                {
                    if(!BlockNoClipping[bb.Type] &&
                       !bb.Invis && !bb.Hidden &&
                       !(BlockIsSizable[bb.Type] && bb.Location.Y < pp_bottom - 3))
                    {
                        doJump = false;
                        break;
                    }
                }
            }

            // D_pLogDebug("Player %d jumped: doJump %d, ppJump: %d", A, doJump ? 1 : 0, pp.Jump);
            if(doJump || pp.Jump > 0)
                pp.Controls.Jump = true;
        }
    }

    UpdateNPCs();
    UpdateBlocks();
    UpdateEffects();
    UpdatePlayer();
    DoCredits(quit);
    UpdateGraphics();
    UpdateSound();
    if(GameOutroDoQuit) // Don't unset the GameOutro before GFX update, otherwise a glitch will happen
    {
        GameOutro = false;
        GameOutroDoQuit = false;
    }
}

void AddCredit(const std::string &newCredit)
{
    numCredits += 1;
    if(numCredits > maxCreditsLines)
    {
        numCredits = maxCreditsLines;
        pLogWarning("Can't add more credits lines: max limit has been excited ({0} linex maximum)", maxCreditsLines);
        return;
    }

    auto &c = Credit[numCredits];
    c.Text = STRINGINDEX_NONE;

    SetS(c.Text, newCredit);
}

void SetupCredits()
{
    int A = 0;
    numCredits = 0;

    AddCredit(g_gameInfo.title);

    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
#ifdef ENABLE_OLD_CREDITS
    AddCredit("Created By:");
#else
    if(!g_gameInfo.creditsGame.empty())
    {
        AddCredit("Game credits:");
        for(auto &s : g_gameInfo.creditsGame)
            AddCredit(s);
        AddCredit("");
        AddCredit("");
        AddCredit("Engine credits:");
        AddCredit("");
    }

    AddCredit("Original VB6 code By:");
#endif
    AddCredit("");
    AddCredit("Andrew Spinks");
    AddCredit("'Redigit'");
    AddCredit("");
    AddCredit("");
#ifndef ENABLE_OLD_CREDITS
    AddCredit("C++ port By:");
    AddCredit("");
    AddCredit("Vitaly Novichkov");
    AddCredit("'Wohlstand'");
    AddCredit("");
    AddCredit("");
#endif

    if(!WorldCredits[1].empty())
    {
        AddCredit("Level Design:");
        AddCredit("");
        for(A = 1; A <= maxWorldCredits; A++)
        {
            if(WorldCredits[A].empty())
                break;
            AddCredit(WorldCredits[A]);
        }
        AddCredit("");
        AddCredit("");
        AddCredit("");
    }

    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("Custom Sprites:");
    AddCredit("");
    AddCredit("Blue");
    AddCredit("Iceman404");
    AddCredit("LuigiFan");
    AddCredit("NameUser");
    AddCredit("Redigit");
    AddCredit("Valtteri");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("Special Thanks:");
#ifndef ENABLE_OLD_CREDITS
    AddCredit("");
    AddCredit("Kevsoft");
    AddCredit("Rednaxela");
    AddCredit("Aero");
    AddCredit("Kley");
    AddCredit("ShadowYoshi (Joey)");
    AddCredit("ZeZeinzer"); // Android testing, touchscreen controller button pictures
    AddCredit("LucyZocker"); // Android testing
    AddCredit("DavFar Gamers"); // Spanish translation of the readme
    AddCredit("Yingchun Soul"); // Idea for individual iceball shooting SFX and contribution with the "frozen NPC breaking" SFX
    AddCredit("MrDoubleA"); // Contribution with the "NPC got frozen" SFX
    AddCredit("Slash-18"); // Contribution with the better iceball shooting SFX
    AddCredit("ds-sloth"); // For the major contribution to the code and becoming a co-developer
    AddCredit("0lhi"); // Significant contribution to TheXTech development, testing, improvement process
    AddCredit("Eclipsed"); // For the help to verify and polish the speed-run mode
#endif
    AddCredit("");
    AddCredit("4matsy");
    AddCredit("AndyDark");
    AddCredit("Bikcmp");
    AddCredit("Blue");
    AddCredit("Captain Obvious");
    AddCredit("CaptainTrek");
    AddCredit("Chase");
    AddCredit("Coldwin");
    AddCredit("CrystalMike");
    AddCredit("DarkMatt");
    AddCredit("FallingSnow");
    AddCredit("Garro");
    AddCredit("Knuckles96");
    AddCredit("Kuribo");
    AddCredit("Kyasarin");
    AddCredit("Luminous");
    AddCredit("m4sterbr0s");
    AddCredit("NameUser");
    AddCredit("Namyrr");
    AddCredit("Qig");
    AddCredit("Quill");
    AddCredit("Red_Yoshi");
    AddCredit("Spitfire");
    AddCredit("Valtteri");
    AddCredit("Vandarx");
    AddCredit("Zephyr");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
#ifndef ENABLE_OLD_CREDITS
    AddCredit("'It just works!'");
    AddCredit("");
    AddCredit("Todd Howard");
#else
    AddCredit("'He has delivered us from the power");
    AddCredit("of darkness and conveyed us into");
    AddCredit("the kingdom of the Son of His love.'");
    AddCredit("");
    AddCredit("Colossians 1:13");
#endif
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit(g_gameInfo.creditsHomePage);

    CreditOffsetY = (ScreenH + 40);
    CreditTotalHeight = 32.0;

    for(A = 1; A <= numCredits; A++)
    {
        auto &cr = Credit[A];
        cr.Location.Width = GetS(cr.Text).size() * 18;
        cr.Location.Height = 16;
        cr.Location.X = (double(ScreenW) / 2) - (cr.Location.Width / 2.0);
        cr.Location.Y = 32 * A;
        CreditTotalHeight += 32.0;
    }
}
