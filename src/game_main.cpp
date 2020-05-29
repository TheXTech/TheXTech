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

#include <Logger/logger.h>
#include <Utils/files.h>
#include <InterProcess/intproc.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>

#include "globals.h"
#include "game_main.h"

#include "blocks.h"
#include "change_res.h"
#include "collision.h"
#include "effect.h"
#include "graphics.h"
#include "joystick.h"
#include "layers.h"
#include "load_gfx.h"
#include "player.h"
#include "sound.h"
#include "editor.h"
#include "main/level_file.h"

#include "pseudo_vb.h"

void CheckActive();
// set up sizable blocks
void SizableBlocks();

// game_main_setupphysics.cpp

int GameMain(const CmdLineSetup_t &setup)
{
    Player_t blankPlayer;
//    int A = 0;
//    int B = 0;
//    int C = 0;
    bool tempBool = false;
    LB = "\n";
    EoT = "";

    FrameSkip = setup.frameSkip;
    noSound = setup.noSound;
    neverPause = setup.neverPause;

    // [ !Here was a starting dialog! ]

    //    frmLoader.Show 'show the Splash screen
    //    Do
    //        DoEvents
    //    Loop While StartMenu = False 'wait until the player clicks a button

    StartMenu = true;

    initAll();

//    Unload frmLoader
    gfxLoaderTestMode = setup.testLevelMode;

    if(!GFX.load()) // Load UI graphics
        return 1;

//    If LevelEditor = False Then
//        frmMain.Show // Show window a bit later
//    frmMain.show();
//        GameMenu = True
    GameMenu = true;
//    Else
//        frmSplash.Show
//        BlocksSorted = True
//    End If

    LoadingInProcess = true;

    InitControls(); // init player's controls
    DoEvents();

#ifdef __EMSCRIPTEN__ // Workaround for a recent Chrome's policy to avoid sudden sound without user's interaction
    frmMain.show(); // Don't show window until playing an initial sound

    while(!MenuMouseDown)
    {
        frmMain.clearBuffer();
        SuperPrint("Click to start a game", 3, 230, 280);
        frmMain.repaint();
        DoEvents();
        PGE_Delay(10);
    }
#endif

    if(!noSound)
    {
        InitMixerX();
        if(!setup.testLevelMode)
            PlayInitSound();
    }

    frmMain.show(); // Don't show window until playing an initial sound

    InitSound(); // Setup sound effects
    LevelSelect = true; // world map is to be shown
    DoEvents();
    SetupPhysics(); // Setup Physics
    SetupGraphics(); // setup graphics
//    Load GFX 'load the graphics form
//    GFX.load(); // load the graphics form // Moved to before sound load
    SizableBlocks();
    LoadGFX(); // load the graphics from file
    SetupVars(); //Setup Variables

    if(setup.interprocess)
        IntProc::init();

    LoadingInProcess = false;

    ShowFPS = setup.testShowFPS;
    MaxFPS = setup.testMaxFPS;

    if(!setup.testLevel.empty() || setup.interprocess) // Start level testing immediately!
    {
        GameMenu = false;
        LevelSelect = false;
        FullFileName = setup.testLevel;
        if(setup.testBattleMode)
        {
            numPlayers = 2;
            BattleMode = true;
            BattleIntro = 150;
        }
        else
        {
            numPlayers = setup.testNumPlayers;
            BattleMode = false;
            BattleIntro = 0;
        }
        GodMode = setup.testGodMode;
        GrabAll = setup.testGrabAll;
        zTestLevel(setup.testMagicHand, setup.interprocess);
    }

    do
    {
        if(GameMenu || MagicHand || LevelEditor)
        {
            frmMain.MousePointer = 99;
            showCursor(0);
        }
        else if(!resChanged)
        {
            frmMain.MousePointer = 0;
            showCursor(1);
        }

//        If LevelEditor = True Then 'Load the level editor
//            [USELESS!]

        // TheXTech Credits
        if(GameOutro)
        {
            ShadowMode = false;
            GodMode = false;
            GrabAll= false;
            CaptainN = false;
            FlameThrower = false;
            FreezeNPCs = false;
            WalkAnywhere = false;
            MultiHop = false;
            SuperSpeed = false;
            FlyForever = false;
            GoToLevelNoGameThing = false;

            for(int A = 1; A <= maxPlayers; A++)
            {
                Player[A] = blankPlayer;
            }

            numPlayers = 5;
            GameMenu = false;
            StopMusic();

            auto outroPath = AppPath + "outro.lvlx";
            if(!Files::fileExists(outroPath))
                outroPath = AppPath + "outro.lvl";
            OpenLevel(outroPath);

            ScreenType = 7;
            SetupScreens();
            ClearBuffer = true;

            for(int A = 1; A <= numPlayers; ++A)
            {
                Player_t &p = Player[A];

                if(A == 1)
                    p.State = 4;
                else if(A == 2)
                    p.State = 7;
                else if(A == 3)
                    p.State = 5;
                else if(A == 4)
                    p.State = 3;
                else
                    p.State = 6;

                if(A == 4)
                {
                    p.Mount = 1;
                    p.MountType = int(iRand() % 3) + 1;
                }

                p.Character = A;
                if(A == 2)
                {
                    p.Mount = 3;
                    p.MountType = int(iRand() % 8) + 1;
                }

                p.HeldBonus = 0;
                p.Section = 0;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            }

            SetupPlayers();
            CreditChop = 300; // 100
            EndCredits = 0;
            SetupCredits();
            overTime = 0;
            GoalTime = SDL_GetTicks() + 1000;
            fpsCount = 0;
            fpsTime = 0;
            cycleCount = 0;
            gameTime = 0;

            // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
            UpdateGraphics();

            do
            {
                DoEvents();
                tempTime = SDL_GetTicks();
                ScreenType = 0;
                SetupScreens();
                if(tempTime >= gameTime + frameRate || tempTime < gameTime)
                {
                    CheckActive();
                    OutroLoop();

                    if(fpsCount >= 32000)
                        fpsCount = 0; // Fixes Overflow bug

                    if(cycleCount >= 32000)
                        cycleCount = 0; // Fixes Overflow bug

                    overTime = overTime + (tempTime - (gameTime + frameRate));

                    if(gameTime == 0.0)
                        overTime = 0;

                    if(overTime <= 1)
                        overTime = 0;
                    else if(overTime > 1000)
                        overTime = 1000;

                    gameTime = tempTime - overTime;
                    overTime = (overTime - (tempTime - gameTime));
                    DoEvents();
                    if(SDL_GetTicks() > fpsTime)
                    {
                        if(cycleCount >= 65)
                        {
                            overTime = 0;
                            gameTime = 0;
                        }

                        cycleCount = 0;
                        fpsTime = SDL_GetTicks() + 1000;
                        GoalTime = fpsTime;
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount

                        if(ShowFPS)
                        {
                            PrintFPS = fpsCount;
                        }
                        fpsCount = 0;
                    }
                }

                PGE_Delay(1);
                if(!GameIsActive) break;// Break on quit
            } while(GameOutro);
        }

        // The Game Menu
        else if(GameMenu)
        {
            BattleIntro = 0;
            BattleOutro = 0;
            AllCharBlock = 0;
            Cheater = false;

            // in a main menu, reset this into initial state
            GoToLevelNoGameThing = false;

            for(int A = 1; A <= maxPlayers; ++A)
            {
                OwedMount[A] = 0;
                OwedMountType[A] = 0;
            }

            MenuMouseMove = false;
            MenuMouseRelease = false;
            MenuMouseClick = false;
            MenuMouseBack = false;
            BattleMode = false;

            if(MenuMode != 4)
            {
                PlayerCharacter = 0;
                PlayerCharacter2 = 0;
            }

            pLogDebug("Clear check-points at Game Menu start");
            Checkpoint.clear();
            CheckpointsList.clear();
            WorldPlayer[1].Frame = 0;
            CheatString = "";
            LevelBeatCode = 0;
            curWorldLevel = 0;

            ClearWorld();

            ReturnWarp = 0;
            ReturnWarpSaved = 0;
            ShadowMode = false;
            GodMode = false;
            GrabAll = false;
            CaptainN = false;
            FlameThrower = false;
            FreezeNPCs = false;
            WalkAnywhere = false;
            MultiHop = false;
            SuperSpeed = false;
            FlyForever = false;
            BeatTheGame = false;
            ScreenType = 2;

            SetupScreens();

            BattleOutro = 0;
            BattleIntro = 0;

            for(int A = 1; A <= maxPlayers; ++A)
            {
                Player[A] = blankPlayer;
            }

            numPlayers = 6;

            auto introPath = AppPath + "intro.lvlx";
            if(!Files::fileExists(introPath))
                introPath = AppPath + "intro.lvl";
            OpenLevel(introPath);
            vScreenX[1] = -level[0].X;

            StartMusic(0);
            SetupPlayers();

            For(A, 1, numPlayers)
            {
                Player_t &p = Player[A];
                p.State = (iRand() % 6) + 2;
                p.Character = (iRand() % 5) + 1;

                if(A >= 1 && A <= 5)
                    p.Character = A;

                p.HeldBonus = 0;
                p.Section = 0;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.X = level[p.Section].X + ((128 + dRand() * 64) * A);
                p.Location.Y = level[p.Section].Height - p.Location.Height - 65;

                do
                {
                    tempBool = true;
                    for(int B = 1; B <= numBlock; ++B)
                    {
                        if(CheckCollision(p.Location, Block[B].Location))
                        {
                            p.Location.Y = Block[B].Location.Y - p.Location.Height - 0.1;
                            tempBool = false;
                        }
                    }
                } while(!tempBool);
                p.Dead = true;
            }

            ProcEvent("Level - Start", true);
            For(A, 2, maxEvents)
            {
                if(Events[A].AutoStart)
                    ProcEvent(Events[A].Name, true);
            }

            overTime = 0;
            GoalTime = SDL_GetTicks() + 1000;
            fpsCount = 0;
            fpsTime = 0;
            cycleCount = 0;
            gameTime = 0;

            // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
            UpdateGraphics();

            do
            {
                DoEvents();
                tempTime = SDL_GetTicks();
                if(tempTime >= gameTime + frameRate || tempTime < gameTime)
                {
                    CheckActive();
                    MenuLoop();   // Run the menu loop

                    if(fpsCount >= 32000)
                        fpsCount = 0; // Fixes Overflow bug

                    if(cycleCount >= 32000)
                        cycleCount = 0; // Fixes Overflow bug

                    overTime = overTime + (tempTime - (gameTime + frameRate));
                    if(gameTime == 0.0)
                        overTime = 0;

                    if(overTime <= 1)
                        overTime = 0;
                    else if(overTime > 1000)
                        overTime = 1000;

                    gameTime = tempTime - overTime;
                    overTime = (overTime - (tempTime - gameTime));

                    DoEvents();

                    if(SDL_GetTicks() > fpsTime)
                    {
                        if(cycleCount >= 65)
                        {
                            overTime = 0;
                            gameTime = 0;
                        }

                        cycleCount = 0;
                        fpsTime = SDL_GetTicks() + 1000;
                        GoalTime = fpsTime;
//                        If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount

                        if(ShowFPS)
                        {
                            PrintFPS = fpsCount;
                        }
                        fpsCount = 0;
                    }
                }

                PGE_Delay(1);
                if(!GameIsActive) return 0;// Break on quit

            } while(GameMenu);
        }

        // World Map
        else if(LevelSelect)
        {
            CheatString.clear();
            For(A, 1, numPlayers)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 2)
                {
                    if(OwedMount[A] > 0)
                    {
                        Player[A].Mount = OwedMount[A];
                        if(OwedMountType[A] > 0)
                            Player[A].MountType = OwedMountType[A];
                        else
                            Player[A].MountType = 1;
                    }
                }
                OwedMount[A] = 0;
                OwedMountType[A] = 0;
            }

            LoadCustomGFX();
            LoadCustomSound();
            SetupPlayers();

            if((!StartLevel.empty() && NoMap) || !GoToLevel.empty())
            {
                if(NoMap)
                    SaveGame();

                Player[1].Vine = 0;
                Player[2].Vine = 0;

                if(!GoToLevelNoGameThing)
                    PlaySound(28);
                SoundPause[26] = 2000;

                LevelSelect = false;

                if(!GoToLevelNoGameThing)
                    GameThing();
                else
                {
                    frmMain.clearBuffer();
                    frmMain.repaint();
                }
                ClearLevel();
                PGE_Delay(1000);

                std::string levelPath;
                if(GoToLevel.empty())
                    levelPath = SelectWorld[selWorld].WorldPath + StartLevel;
                else
                {
                    levelPath = SelectWorld[selWorld].WorldPath + GoToLevel;
                    GoToLevel.clear();
                }

                if(!OpenLevel(levelPath))
                {
                    MessageText = fmt::format_ne("ERROR: Can't open \"{0}\": file doesn't exist or corrupted.", levelPath);
                    PauseGame(1);
                    ErrorQuit = true;
                }
            }
            else
            {
                if(curWorldMusic > 0)
                    StartMusic(curWorldMusic);
                overTime = 0;
                GoalTime = SDL_GetTicks() + 1000;
                fpsCount = 0;
                fpsTime = 0;
                cycleCount = 0;
                gameTime = 0;

                // On a world map, reset this into default state
                GoToLevelNoGameThing = false;

                // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
                UpdateGraphics2();

                do // 'level select loop
                {
                    FreezeNPCs = false;
                    DoEvents();
                    tempTime = SDL_GetTicks();
                    if(tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS)
                    {
                        if(fpsCount >= 32000)
                            fpsCount = 0; // Fixes Overflow bug

                        if(cycleCount >= 32000)
                            cycleCount = 0; // Fixes Overflow bug

                        overTime = overTime + (tempTime - (gameTime + frameRate));

                        if(gameTime == 0.0)
                            overTime = 0;

                        if(overTime <= 1)
                            overTime = 0;
                        else if(overTime > 1000)
                            overTime = 1000;

                        gameTime = tempTime - overTime;
                        overTime = (overTime - (tempTime - gameTime));

                        CheckActive();
                        WorldLoop();
                        DoEvents();

                        if(SDL_GetTicks() > fpsTime)
                        {
                            if(cycleCount >= 65)
                            {
                                overTime = 0;
                                gameTime = 0;
                            }
                            cycleCount = 0;
                            fpsTime = SDL_GetTicks() + 1000;
                            GoalTime = fpsTime;
//                            If Debugger = True Then frmLevelDebugger.lblFPS = fpsCount

                            if(ShowFPS)
                                PrintFPS = fpsCount;

                            fpsCount = 0;
                        }
                    }

                    PGE_Delay(1);
                    if(!GameIsActive)
                        return 0;// Break on quit
                } while(LevelSelect);
            }
        }

        // MAIN GAME
        else
        {
            CheatString.clear();
            EndLevel = false;

            if(numPlayers == 1)
                ScreenType = 0; // Follow 1 player
            else if(numPlayers == 2)
                ScreenType = 5; // Dynamic screen
            else
            {
                // ScreenType = 3 'Average, no one leaves the screen
                ScreenType = 2; // Average
            }

            if(SingleCoop > 0)
                ScreenType = 6;
//            If nPlay.Online = True Then ScreenType = 8 'Online

            for(int A = 1; A <= numPlayers; ++A)
            {
                if(Player[A].Mount == 2)
                    Player[A].Mount = 0; // take players off the clown car
            }

            SetupPlayers(); // Setup Players for the level
            qScreen = false;

// for warp entrances
            if((ReturnWarp > 0 && IsEpisodeIntro/*FileName == StartLevel*/) || (StartWarp > 0))
            {
                for(int numPlayersMax = numPlayers, A = 1; A <= numPlayersMax; ++A)
                {
                    Player_t &p = Player[A];

                    if(StartWarp > 0)
                        p.Warp = StartWarp;
                    else
                        p.Warp = ReturnWarp;

                    if(Warp[p.Warp].Effect == 1)
                    {
                        if(Warp[p.Warp].Direction2 == 1) // DOWN
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = Warp[p.Warp].Exit.X + (Warp[p.Warp].Exit.Width / 2) - (p.Location.Width / 2);
//                                .Location.Y = Warp(.Warp).Exit.Y - .Location.Height - 8
                            p.Location.Y = Warp[p.Warp].Exit.Y - p.Location.Height - 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 3 Then
                        if(Warp[p.Warp].Direction2 == 3) // UP
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = Warp[p.Warp].Exit.X + (Warp[p.Warp].Exit.Width / 2) - (p.Location.Width / 2);
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height + 8
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height + 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 2 Then
                        if(Warp[p.Warp].Direction2 == 2) // RIGHT
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X - .Location.Width - 8
                            p.Location.X = Warp[p.Warp].Exit.X - p.Location.Width - 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height - p.Location.Height - 2;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 4 Then
                        if(Warp[p.Warp].Direction2 == 4) // LEFT
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width + 8
                            p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width + 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height - p.Location.Height - 2;
//                            End If
                        }

                        PlayerFrame(A);
                        CheckSection(A);
                        SoundPause[17] = 0;
                        p.Effect = 8;
                        p.Effect2 = 950;
                    }
                    else if(Warp[p.Warp].Effect == 2)
                    {
//                            .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                        p.Location.X = Warp[p.Warp].Exit.X + Warp[p.Warp].Exit.Width / 2 - p.Location.Width / 2;
//                            .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height
                        p.Location.Y = Warp[p.Warp].Exit.Y + Warp[p.Warp].Exit.Height - p.Location.Height;

                        CheckSection(A);
                        p.Effect = 8;
                        p.Effect2 = 2000;
                    }
                }

                if(StartWarp > 0)
                    StartWarp = 0;
                else
                    ReturnWarp = 0;
            }
//'--------------------------------------------
            ProcEvent("Level - Start", true);

            for(int A = 2; A <= maxEvents; ++A)
            {
                if(Events[A].AutoStart)
                    ProcEvent(Events[A].Name, true);
            }

            overTime = 0;
            GoalTime = SDL_GetTicks() + 1000;
            fpsCount = 0;
            fpsTime = 0;
            cycleCount = 0;
            gameTime = 0;

            // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
            UpdateGraphics();

            do // MAIN GAME LOOP
            {
                DoEvents();
                tempTime = SDL_GetTicks();

                if(tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS)
                {
                    CheckActive();
                    if(fpsCount >= 32000) fpsCount = 0; // Fixes Overflow bug
                    if(cycleCount >= 32000) cycleCount = 0; // Fixes Overflow bug
                    overTime = overTime + (tempTime - (gameTime + frameRate));
                    if(gameTime == 0.0)
                        overTime = 0;
                    if(overTime <= 1)
                        overTime = 0;
                    else if(overTime > 1000)
                        overTime = 1000;

                    gameTime = tempTime - overTime;
                    overTime = (overTime - (tempTime - gameTime));

                    GameLoop(); // Run the game loop
                    DoEvents();

                    if(SDL_GetTicks() > fpsTime)
                    {
                        if(cycleCount >= 65)
                        {
                            overTime = 0;
                            gameTime = tempTime;
                        }

                        cycleCount = 0;
                        fpsTime = SDL_GetTicks() + 1000;
                        GoalTime = fpsTime;


                        if(ShowFPS)
                        {
                            PrintFPS = fpsCount;
                        }
                        fpsCount = 0;
                    }

                    if(!LivingPlayers())
                    {
                        EveryonesDead();
                    }
                }

                PGE_Delay(1);
                if(!GameIsActive) return 0;// Break on quit
            }
            while(!LevelSelect && !GameMenu);

            // TODO: Utilize this and any TestLevel/MagicHand related code to allow PGE Editor integration
            // (do any code without interaction of no more existnig Editor VB forms, keep IPS with PGE Editor instead)

//            If TestLevel = True Then
            if(TestLevel)
            {
//                TestLevel = False
//                TestLevel = false;
//                LevelEditor = True
//                LevelEditor = true;
//                LevelEditor = true; //FIXME: Restart level testing or quit a game instead of THIS

                GameThing();
                PGE_Delay(500);
                zTestLevel(setup.testMagicHand, setup.interprocess); // Restart level

//                If nPlay.Online = False Then
//                    OpenLevel FullFileName
//                OpenLevel(FullFileName);
//                Else
//                    If nPlay.Mode = 1 Then
//                        Netplay.sendData "H0" & LB
//                        If Len(FullFileName) > 4 Then
//                            If LCase(Right(FullFileName, 4)) = ".lvl" Then
//                                OpenLevel FullFileName
//                            Else
//                                For A = 1 To 15
//                                    If nPlay.ClientCon(A) = True Then Netplay.InitSync A
//                                Next A
//                            End If
//                        Else
//                            For A = 1 To 15
//                                If nPlay.ClientCon(A) = True Then Netplay.InitSync A
//                            Next A
//                        End If
//                    End If
//                End If

//                LevelSelect = False
                LevelSelect = false;
            }
//            Else
            else
            {
                ClearLevel();
//            End If
            } // TestLevel
        }

    } while(GameIsActive);

    return 0;
}

// game_main_setupvars.cpp

// game_loop.cpp

// menu_loop.cpp

void EditorLoop()
{
    // DUMMY
}

void KillIt()
{
    GameIsActive = false;
    frmMain.hide();
    if(resChanged)
        SetOrigRes();
    QuitMixerX();
    UnloadGFX();
    showCursor(1);
}


void NextLevel()
{
    int A = 0;
    for(A = 1; A <= numPlayers; A++)
        Player[A].HoldingNPC = 0;
    LevelMacro = 0;
    LevelMacroCounter = 0;
    StopMusic();
    ClearLevel();
    frmMain.clearBuffer();
    frmMain.repaint();
    DoEvents();
    if(!TestLevel && GoToLevel.empty() && !NoMap)
        PGE_Delay(500);
    if(BattleMode && !LevelEditor && !TestLevel)
    {
        EndLevel = false;
        GameMenu = true;
        MenuMode = 4;
        MenuCursor = selWorld - 1;
        PlayerCharacter = Player[1].Character;
        PlayerCharacter2 = Player[2].Character;
    }
    else
    {
        LevelSelect = true;
        EndLevel = false;
        if(TestLevel && BattleMode)
        {
            BattleIntro = 150;
            GameIsActive = false; // Quit game
        }
    }
}

// macros mainly used for end of level stuffs. takes over the players controls
void UpdateMacro()
{
    int A = 0;
    bool OnScreen = false;
    if(LevelMacro == 1) // SMB3 Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Location.X < level[Player[A].Section].Width && Player[A].Dead == false)
            {
                OnScreen = true;
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
                if(Player[A].Wet > 0 && Player[A].CanJump == true)
                {
                    if(Player[A].Location.SpeedY > 1)
                        Player[A].Controls.Jump = true;
                }
            }
            else
            {
                Player[A].Location.SpeedY = -Physics.PlayerGravity;
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
            }
        }

        if(!OnScreen)
        {
            LevelMacroCounter = LevelMacroCounter + 1;
            if(LevelMacroCounter >= 100)
            {
                LevelBeatCode = 1;
                LevelMacro = 0;
                LevelMacroCounter = 0;
                EndLevel = true;
            }
        }
    }
    else if(LevelMacro == 2)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 460)
        {
            LevelBeatCode = 2;
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            frmMain.clearBuffer();
        }
    }
    else if(LevelMacro == 3)
    {
        float tempTime = 0;
        float gameTime = 0;

        do
        {
            // tempTime = Timer - Int(Timer)
            tempTime = (float(SDL_GetTicks()) / 1000.0f) - std::floor(float(SDL_GetTicks()) / 1000.0f);
            if(tempTime > (float)(gameTime + 0.01f) || tempTime < gameTime)
            {
                gameTime = tempTime;
                DoEvents();
                UpdateGraphics();
                UpdateSound();
                BlockFrames();
                LevelMacroCounter = LevelMacroCounter + 1;
                if(LevelMacroCounter >= 300)
                    break;
            }

            if(!GameIsActive)
                return;

            PGE_Delay(1);
        } while(true);

        LevelBeatCode = 4;
        EndLevel = true;
        LevelMacro = 0;
        LevelMacroCounter = 0;
        frmMain.clearBuffer();
    }
    else if(LevelMacro == 4)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 300)
        {
            LevelBeatCode = 5;
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            frmMain.clearBuffer();
        }
    }
    else if(LevelMacro == 5)
    {
        // numNPCs = 0
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter == 250)
            PlaySound(45);
        if(LevelMacroCounter >= 800)
        {
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            if(!TestLevel)
            {
                GameOutro = true;
                BeatTheGame = true;
                SaveGame();
                MenuMode = 0;
                MenuCursor = 0;
            }
            frmMain.clearBuffer();
        }
    }
    else if(LevelMacro == 6) // Star Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Controls.Down = false;
            Player[A].Controls.Drop = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            Player[A].Controls.Run = false;
            Player[A].Controls.Up = false;
            Player[A].Controls.Start = false;
            Player[A].Controls.AltJump = false;
            Player[A].Controls.AltRun = false;
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 300)
        {
            LevelBeatCode = 7;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
    else if(LevelMacro == 7) // SMW Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Location.X < level[Player[A].Section].Width && Player[A].Dead == false)
            {
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
            }
            else
            {
                Player[A].Location.SpeedY = -Physics.PlayerGravity;
                Player[A].Controls.Down = false;
                Player[A].Controls.Drop = false;
                Player[A].Controls.Jump = false;
                Player[A].Controls.Left = false;
                Player[A].Controls.Right = true;
                Player[A].Controls.Run = false;
                Player[A].Controls.Up = false;
                Player[A].Controls.Start = false;
                Player[A].Controls.AltJump = false;
                Player[A].Controls.AltRun = false;
            }
        }
        LevelMacroCounter = LevelMacroCounter + 1;
        if(LevelMacroCounter >= 630)
        {
            LevelBeatCode = 8;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
}

void InitControls()
{
    int A = 0;
//    int B = 0;
    bool newJoystick = false;

    int joysticksCount = InitJoysticks();

    for(int i = 0; i < joysticksCount; ++i)
    {
        newJoystick = StartJoystick(i);
        if(newJoystick) {
            A += 1;
        } else {
            break;
        }
    }
    numJoysticks = A;

    /* // Crazy Redigit's solution, useless
//    If numJoysticks = 0 Then
    if(numJoysticks == 0) {
//        useJoystick(1) = 0
        useJoystick[1] = 0;
//        useJoystick(2) = 0
        useJoystick[2] = 0;
//    ElseIf numJoysticks = 1 Then
    } else if(numJoysticks == 1) {
//        useJoystick(1) = 1
        useJoystick[1] = 1;
//        useJoystick(2) = 0
        useJoystick[2] = 0;
//    Else
    } else {
//        useJoystick(1) = 1
        useJoystick[1] = 1;
//        useJoystick(2) = 2
        useJoystick[2] = 2;
//    End If
    }
    */

//    '
    useJoystick[1] = 0;
    useJoystick[2] = 0;
//    '

    For(A, 1, 2)
    {
        {
            auto &j = conJoystick[A];
            j.Up.val = SDL_HAT_UP;
            j.Up.type = ConJoystick_t::JoyHat;
            j.Up.id = 0;
            j.Down.val = SDL_HAT_DOWN;
            j.Down.type = ConJoystick_t::JoyHat;
            j.Down.id = 0;
            j.Left.val = SDL_HAT_LEFT;
            j.Left.id = 0;
            j.Left.type = ConJoystick_t::JoyHat;
            j.Right.val = SDL_HAT_RIGHT;
            j.Right.type = ConJoystick_t::JoyHat;
            j.Right.id = 0;

            j.Run.id = 2;
            j.Run.val = 1;
            j.Run.type = ConJoystick_t::JoyButton;

            j.AltRun.id = 3;
            j.AltRun.val = 1;
            j.AltRun.type = ConJoystick_t::JoyButton;

            j.Jump.id = 0;
            j.Jump.val = 1;
            j.Jump.type = ConJoystick_t::JoyButton;

            j.AltJump.id = 1;
            j.AltJump.val = 1;
            j.AltJump.type = ConJoystick_t::JoyButton;

            j.Drop.id = 6;
            j.Drop.val = 1;
            j.Drop.type = ConJoystick_t::JoyButton;

            j.Start.id = 7;
            j.Start.val = 1;
            j.Start.type = ConJoystick_t::JoyButton;
        }
    }

    conKeyboard[1].Down = vbKeyDown;
    conKeyboard[1].Left = vbKeyLeft;
    conKeyboard[1].Up = vbKeyUp;
    conKeyboard[1].Right = vbKeyRight;
    conKeyboard[1].Jump = vbKeyZ;
    conKeyboard[1].Run = vbKeyX;
    conKeyboard[1].Drop = vbKeyShift;
    conKeyboard[1].Start = vbKeyEscape;
    conKeyboard[1].AltJump = vbKeyA;
    conKeyboard[1].AltRun = vbKeyS;

    conKeyboard[2].Down = vbKeyDown;
    conKeyboard[2].Left = vbKeyLeft;
    conKeyboard[2].Up = vbKeyUp;
    conKeyboard[2].Right = vbKeyRight;
    conKeyboard[2].Jump = vbKeyZ;
    conKeyboard[2].Run = vbKeyX;
    conKeyboard[2].Drop = vbKeyShift;
    conKeyboard[2].Start = vbKeyEscape;
    conKeyboard[2].AltJump = vbKeyA;
    conKeyboard[2].AltRun = vbKeyS;

    OpenConfig();

    if(useJoystick[1] > numJoysticks)
        useJoystick[1] = 0;
    if(useJoystick[2] > numJoysticks)
        useJoystick[2] = 0;
}


// main_config.cpp


void NPCyFix()
{
    int A = 0;
    float XnH = 0;
    float XnHfix = 0;
    for(A = 1; A <= numNPCs; A++)
    {
        XnH = NPC[A].Location.Y + NPC[A].Location.Height;
        if((int(XnH * 100) % 800) / 100 != 0)
        {
            if((int(XnH + std::abs((int(XnH * 100) % 800) / 100)) * 100) % 800 == 0)
                XnHfix = std::abs((int(XnH * 100) % 800) / 100);
            else
                XnHfix = std::abs(8 - ((int(XnH * 100) % 800) / 100));
            NPC[A].Location.Y = NPC[A].Location.Y + XnHfix;
        }
    }
}

void CheckActive()
{
#ifndef __EMSCRIPTEN__
    bool MusicPaused = false;
    bool focusLost = false;

    if(neverPause)
        return;

    if(!GameIsActive)
        return;

//    If nPlay.Online = True Then Exit Sub
    // If LevelEditor = False And TestLevel = False Then Exit Sub
    // If LevelEditor = False Then Exit Sub
    while(!frmMain.isWindowActive())
    {
        frmMain.waitEvents();
//        If LevelEditor = True Or MagicHand = True Then frmLevelWindow.vScreen(1).MousePointer = 0
        if(LevelEditor || MagicHand)
            showCursor(0);
        overTime = 0;
        GoalTime = SDL_GetTicks() + 1000;
        fpsCount = 0;
        fpsTime = 0;
        cycleCount = 0;
        gameTime = 0;
        tempTime = 0;
        //keyDownEnter = false;
        //keyDownAlt = false;

        if(!focusLost)
        {
            pLogDebug("Window Focus lost");
            focusLost = true;
        }

        if(musicPlaying && !MusicPaused)
        {
            MusicPaused = true;
            SoundPauseAll();
        }

        if(!GameIsActive)
            break;
    }

    if(focusLost)
        pLogDebug("Window Focus got back");

    if(MusicPaused)
        SoundResumeAll();

/* // Useless condition
    if(!noSound && MusicPaused)
    {
        if(MusicPaused)
        {
            SoundResumeAll();
            if(GameOutro == true)
                SoundResumeAll();
            else if(LevelSelect == true && GameMenu == false && LevelEditor == false)
                SoundResumeAll();
            else if(curMusic > 0)
                SoundResumeAll();
            else if(curMusic < 0)
            {
                SoundResumeAll();
                if(PSwitchStop > 0)
                {
                    // mciSendString "resume stmusic", 0, 0, 0
                    SoundResumeAll();
                }
                else
                {
                    // mciSendString "resume smusic", 0, 0, 0
                    SoundResumeAll();
                }
            }
        }
    }
    */
//    If LevelEditor = True Or MagicHand = True Then frmLevelWindow.vScreen(1).MousePointer = 99
#endif // not def __EMSCRIPTEN__
}




void AddCredit(std::string newCredit)
{
    numCredits += 1;
    if(numCredits > maxCreditsLines)
    {
        numCredits = maxCreditsLines;
        pLogWarning("Can't add more credits lines: max limit has been excited ({0} linex maximum)", maxCreditsLines);
        return;
    }
    Credit[numCredits].Text = newCredit;
}

Location_t newLoc(double X, double Y, double Width, double Height)
{
    Location_t ret;
    ret.X = X;
    ret.Y = Y;
    ret.Width = Width;
    ret.Height = Height;
    return ret;
}

void MoreScore(int addScore, Location_t Loc)
{
    int mult = 0; // dummy
    MoreScore(addScore, Loc, mult);
}

void MoreScore(int addScore, Location_t Loc, int &Multiplier)
{
    //int oldM = 0;
    int A = 0;

    if(GameMenu || GameOutro || BattleMode)
        return;
    A = addScore + Multiplier;
    if(A == 0)
        return;
    Multiplier++;
    if(A > 13)
        A = 13;
    if(A < addScore)
        A = addScore;
    if(Multiplier > 9)
        Multiplier = 8;
    if(A > 13)
        A = 13;
    if(Points[A] <= 5)
    {
        Lives += Points[A];
        PlaySound(15, Points[A] - 1);
    }
    else
        Score += Points[A];
    NewEffect(79, Loc);
    Effect[numEffects].Frame = A - 1;
}

void SizableBlocks()
{
    BlockIsSizable[568] = true;
    BlockIsSizable[579] = true;
    BlockIsSizable[575] = true;
    BlockIsSizable[25] = true;
    BlockIsSizable[26] = true;
    BlockIsSizable[27] = true;
    BlockIsSizable[28] = true;
    BlockIsSizable[38] = true;
    BlockIsSizable[79] = true;
    BlockIsSizable[108] = true;
    BlockIsSizable[130] = true;
    BlockIsSizable[161] = true;
    BlockIsSizable[240] = true;
    BlockIsSizable[241] = true;
    BlockIsSizable[242] = true;
    BlockIsSizable[243] = true;
    BlockIsSizable[244] = true;
    BlockIsSizable[245] = true;
    BlockIsSizable[259] = true;
    BlockIsSizable[260] = true;
    BlockIsSizable[261] = true;
    BlockIsSizable[287] = true;
    BlockIsSizable[288] = true;
    BlockIsSizable[437] = true;
    BlockIsSizable[441] = true;
    BlockIsSizable[442] = true;
    BlockIsSizable[443] = true;
    BlockIsSizable[444] = true;
    BlockIsSizable[438] = true;
    BlockIsSizable[439] = true;
    BlockIsSizable[440] = true;
    BlockIsSizable[445] = true;
}

void StartBattleMode()
{
    int A = 0;
    Player_t blankPlayer;
    numPlayers = 2;
    for(A = 1; A <= numCharacters; A++)
    {
        SavedChar[A] = blankPlayer;
        SavedChar[A].Character = A;
        SavedChar[A].State = 1;
    }
    Player[1].State = 2;
    Player[1].Mount = 0;
    Player[1].Character = 1;
    Player[1].HeldBonus = 0;
    Player[1].CanFly = false;
    Player[1].CanFly2 = false;
    Player[1].TailCount = 0;
    Player[1].YoshiBlue = false;
    Player[1].YoshiRed = false;
    Player[1].YoshiYellow = false;
    Player[1].Hearts = 2;
    Player[2].State = 2;
    Player[2].Mount = 0;
    Player[2].Character = 2;
    Player[2].HeldBonus = 0;
    Player[2].CanFly = false;
    Player[2].CanFly2 = false;
    Player[2].TailCount = 0;
    Player[2].YoshiBlue = false;
    Player[2].YoshiRed = false;
    Player[2].YoshiYellow = false;
    Player[2].Hearts = 2;
    Player[1].Character = PlayerCharacter;
    Player[2].Character = PlayerCharacter2;
    numStars = 0;
    Coins = 0;
    Score = 0;
    Lives = 99;
    BattleLives[1] = 3;
    BattleLives[2] = 3;
    LevelSelect = false;
    GameMenu = false;
    BattleMode = true;
    frmMain.clearBuffer();
    frmMain.repaint();
    StopMusic();
    DoEvents();
    PGE_Delay(500);
    ClearLevel();

    if(NumSelectWorld <= 1)
    {
        MessageText = "Can't start battle because of no levels available";
        PauseGame(1);
        ErrorQuit = true;
    }
    else
    {
        if(selWorld == 1)
            selWorld = (iRand() % (NumSelectWorld - 1)) + 2;
    }

    std::string levelPath = SelectWorld[selWorld].WorldPath + SelectWorld[selWorld].WorldFile;
    if(!OpenLevel(levelPath))
    {
        MessageText = fmt::format_ne("ERROR: Can't open \"{0}\": file doesn't exist or corrupted.", SelectWorld[selWorld].WorldFile);
        PauseGame(1);
        ErrorQuit = true;
    }
    SetupPlayers();

    BattleIntro = 150;
    BattleWinner = 0;
    BattleOutro = 0;
}
