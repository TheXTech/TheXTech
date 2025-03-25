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


#include <Logger/logger.h>
#include <Integrator/integrator.h>

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
#include "outro_loop.h"

OutroContent g_outroScreen;

void initOutroContent()
{
    g_outroScreen.gameCredits = "Game credits:";
    g_outroScreen.engineCredits = "Engine credits:";

    g_outroScreen.originalBy = "Original VB6 code By:";
    g_outroScreen.nameAndrewSpinks = "Andrew Spinks";

    g_outroScreen.cppPortDevelopers = "C++ port developers:";
    g_outroScreen.nameVitalyNovichkov = "Vitaly Novichkov";

    g_outroScreen.qualityControl = "Quality Control:";

    g_outroScreen.psVitaPortBy = "PS Vita Port By:";

    g_outroScreen.levelDesign = "Level Design:";

    g_outroScreen.customSprites = "Custom Sprites:";

    g_outroScreen.specialThanks = "Special Thanks:";
}


void DoCredits(bool quit)
{
    // CreditChop and CreditOffsetY were previously floats, now they are ints measured in tenths

    if(GameMenu)
        return;

    int TargetH_half = XRender::TargetH / 2;

    int shrink = vScreen[1].Top;

    CreditOffsetY -= 8;
//    if(CreditOffsetY > ScreenH || CreditOffsetY + CreditTotalHeight < 0)
//    {}

    // Closing screen
    if((CreditOffsetY + CreditTotalHeight * 10) < -shrink * 10)
    {
        if(musicPlaying)
        {
            FadeOutMusic(11000);
            musicPlaying = false;
        }

        // CreditChop += 0.4f;
        CreditChop += 4;
        if(CreditChop >= TargetH_half * 10)
        {
            CreditChop = TargetH_half * 10;
            EndCredits++;
            if(EndCredits == TargetH_half)
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
    else if(CreditChop > shrink * 10 && CreditOffsetY + CreditTotalHeight * 10 > 0)
    {
        // CreditChop -= 2.0f;
        CreditChop -= 20;
        if(CreditChop < shrink * 10)
            CreditChop = shrink * 10;

        if(CreditChop < TargetH_half * 10 - 500 && !musicPlaying)
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

    if(CreditChop <= shrink * 10 || EndCredits > 0)
    {
//        for(A = 1; A <= 2; A++) // Useless loop
//        {
        if(quit)
        {
            CreditChop = TargetH_half * 10;
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
    Controls::Update(false);
    Integrator::sync();
    bool quit = SharedControls.QuitCredits;

    for(int i = 0; i < l_screen->player_count; i++)
        quit |= Controls::g_RawControls[i].Start;

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
        num_t pp_bottom = pp.Location.Y + pp.Location.Height;
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

            for(const Block_t& bb : treeFLBlockQuery(tempLocation, SORTMODE_NONE))
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
        AddCredit(g_outroScreen.gameCredits);
        for(auto &s : g_gameInfo.creditsGame)
            AddCredit(s);
        AddCredit("");
        AddCredit("");
        AddCredit(g_outroScreen.engineCredits);
        AddCredit("");
    }

    AddCredit(g_outroScreen.originalBy);
#endif
    AddCredit("");
    AddCredit(g_outroScreen.nameAndrewSpinks);
    AddCredit("'Redigit'"); // Author of original VB6 Engine (2009-2011)
    AddCredit("");
    AddCredit("");
#ifndef ENABLE_OLD_CREDITS
    AddCredit(g_outroScreen.cppPortDevelopers);
    AddCredit("");
    AddCredit(g_outroScreen.nameVitalyNovichkov);
    AddCredit("'Wohlstand'");
    AddCredit("");
    AddCredit("ds-sloth"); // For the major contribution to the code and becoming a co-developer
    AddCredit("");
    AddCredit("");
    AddCredit(g_outroScreen.qualityControl);
    AddCredit("");
    AddCredit("0lhi"); // Significant contribution to TheXTech development, testing, improvement process (2021-2024)
    AddCredit("");
    AddCredit("");
#endif
#ifdef VITA
    AddCredit(g_outroScreen.psVitaPortBy);
    AddCredit("");
    AddCredit("Axiom");
    AddCredit("");
    AddCredit("");
#endif

    if(numWorldCredits > 0)
    {
        AddCredit(g_outroScreen.levelDesign);
        AddCredit("");
        for(A = 1; A <= numWorldCredits; A++)
            AddCredit(WorldCredits[A]);
        AddCredit("");
        AddCredit("");
        AddCredit("");
    }

    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit(g_outroScreen.customSprites);
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
    AddCredit(g_outroScreen.specialThanks);
#ifndef ENABLE_OLD_CREDITS
    AddCredit("");
    AddCredit("Kevsoft");
    AddCredit("Rednaxela");
    AddCredit("Aero");
    AddCredit("Kley");
    AddCredit("ShadowYoshi (Joey)");
    AddCredit("ZeZeinzer"); // Android testing, touchscreen controller button pictures
    AddCredit("LucyZocker"); // Android testing
    AddCredit("DavFar"); // Spanish translation of the ReadMe
    AddCredit("Yingchun Soul"); // Idea for individual iceball shooting SFX and contribution with the "frozen NPC breaking" SFX
    AddCredit("MrDoubleA"); // Contribution with the "NPC got frozen" SFX
    AddCredit("Slash-18"); // Contribution with the better iceball shooting SFX
    AddCredit("sl4cer");   // For reporting several issues while livestreaming (2021) (also known as draena)
    AddCredit("Eclipsed"); // For the help to verify and polish the speed-run mode (2021)
    AddCredit("RunninPigeon"); // For Nintendo Switch alpha testing on hardware (Formerly "ThatRoadRunnerfan727 (Matthew)")
    AddCredit("RMN Community"); // For Quality Episodes that allowed us to refine the Engine
    AddCredit("Savby"); // For important design work on the character select screen (ConnectScreen)
    AddCredit("Otabo"); /* For the "Sarasaland Adventure" projects (both 1 and 2) that was being widely used in
                           tests and debugs: and also, they introduced the SMBX to me (Vitaliy), in 2013
                           and inspired me to all my future projects (Moondust Project and TheXTech). */
    AddCredit("Krissy Silvermoon"); // For testing and design feedback since 2021 until at least 2024
    AddCredit("Sapphire Bullet Bill"); // For testing and gameplay feedback since 2022 until at least 2024
    AddCredit("Bubble"); // For bugtesting in 2021-2022
    AddCredit("Trickii"); // For extensive bugtesting in 2024
    AddCredit("AntonioGZZ96"); // For careful bugtesting in 2024
    AddCredit("ddrsoul"); // For sharing the engine with the PortMaster community
    // --- P.S. Put all next lines above THIS comment line. ---
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

    // CreditChop and CreditOffsetY were previously floats, now they are ints measured in tenths
    CreditChop = (XRender::TargetH / 2) * 10; // 100
    EndCredits = 0;
    CreditOffsetY = (XRender::TargetH + 40) * 10;
    CreditTotalHeight = 32;

    for(A = 1; A <= numCredits; A++)
    {
        auto &cr = Credit[A];
        cr.Location.Width = SuperTextPixLen(GetS(cr.Text), g_gameInfo.creditsFont);
        cr.Location.Height = 16;
        cr.Location.X = (XRender::TargetW / 2) - (cr.Location.Width / 2);
        cr.Location.Y = 32 * A;
        CreditTotalHeight += 32;
    }
}
