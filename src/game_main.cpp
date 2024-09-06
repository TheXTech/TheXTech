/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef PGE_NO_THREADING
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_thread.h>
#endif

#include <Logger/logger.h>
#include <Utils/files.h>
#include <AppPath/app_path.h>
#include <Integrator/integrator.h>
#include <PGE_File_Formats/file_formats.h>
#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif
#include <pge_delay.h>
#include <fmt_format_ne.h>

#ifdef ENABLE_XTECH_LUA
#include "xtech_lua_main.h"
#endif

#include "sdl_proxy/sdl_timer.h"

#include "globals.h"
#include "game_main.h"
#include "gfx.h"

#include "config.h"
#include "frame_timer.h"
#include "blocks.h"
#include "change_res.h"
#include "collision.h"
#include "effect.h"
#include "eff_id.h"
#include "graphics.h"
#include "layers.h"
#include "load_gfx.h"
#include "player.h"
#include "sound.h"
#include "editor.h"
#include "editor/new_editor.h"
#include "custom.h"
#include "main/world_globals.h"
#include "main/cheat_code.h"
#include "main/game_globals.h"
#include "main/level_file.h"
#include "main/world_file.h"
#include "main/speedrunner.h"
#include "main/menu_main.h"
#include "main/game_info.h"
#include "main/outro_loop.h"
#include "editor/editor_strings.h"
#include "main/game_strings.h"
#include "main/translate.h"
#include "main/record.h"
#include "main/asset_pack.h"
#include "core/render.h"
#include "core/window.h"
#include "core/events.h"
#include "core/msgbox.h"
#include "core/language.h"
#include "script/luna/luna.h"
#include "fontman/font_manager.h"

#include "pseudo_vb.h"

#include "controls.h"

#include "main/screen_connect.h"
#include "main/screen_quickreconnect.h"
#include "main/screen_asset_pack.h"

#include "main/level_medals.h"

#include "main/trees.h"

void CheckActive();
// set up sizable blocks
void SizableBlocks();

// game_main_setupphysics.cpp

static int loadingThread(void *waiter_ptr)
{
#ifndef PGE_NO_THREADING
    auto *waiter = (SDL_atomic_t *)waiter_ptr;
#else
    UNUSED(waiter_ptr);
#endif

    LoaderUpdateDebugString("Game info");
    initGameInfo();
    cheats_reset();

    LoaderUpdateDebugString("Translations");
    XLanguage::findLanguages(); // find present translations
    ReloadTranslations(); // load translations

    LoaderUpdateDebugString("Asset packs");
    GetAssetPacks();

    SetupPhysics(); // Setup Physics
    SetupGraphics(); // setup graphics
//    Load GFX 'load the graphics form
//    GFX.load(); // load the graphics form // Moved to before sound load
    SizableBlocks();
    LoadGFX(); // load the graphics from file

    Controls::LoadTouchScreenGFX();

    SetupVars(); //Setup Variables

#ifdef THEXTECH_PRELOAD_LEVELS
    LoaderUpdateDebugString("Worlds preload");
    FindWorlds();
    LoaderUpdateDebugString("Levels preload");
    FindLevels();
#endif

    InitSound(); // Setup sound effects

#ifndef PGE_NO_THREADING
    if(waiter)
        SDL_AtomicSet(waiter, 0);
#endif

    return 0;
}

static std::string getIntrFile()
{
    auto introPath = AppPath + "intro.lvlx";

    if(!Files::fileExists(introPath))
        introPath = AppPath + "intro.lvl";

    if(!Files::fileExists(introPath))
        return std::string();

    pLogDebug("Found root intro level file: %s", introPath.c_str());

    return introPath;
}

static std::string findIntroLevel()
{
    std::string introPath;
    std::string introSetDir = AppPath + "introset/";

    if(!DirMan::exists(introSetDir))
        return getIntrFile();

    DirMan introSet(introSetDir);
    std::vector<std::string> intros;

    if(!introSet.getListOfFiles(intros, {".lvl", "lvlx"}) || intros.empty())
        return getIntrFile();

    std::sort(intros.begin(), intros.end());

    for(auto &i : intros)
    {
        i.insert(0, introSetDir);
        pLogDebug("Found introset intro level: %s", i.c_str());
    }

    auto rootIntro = getIntrFile();
    if(!rootIntro.empty())
        intros.push_back(rootIntro);

    const std::string &selected = intros[iRand2T(intros.size())];

    pLogDebug("Selected intro level to start: %s", selected.c_str());

    return selected;
}

// expand the section vertically if the top 8px of the level are empty
static void s_ExpandSectionForMenu()
{
    SpeedlessLocation_t& menu_section = level[0];

    // check current section top for expandability
    Location_t tempLocation = newLoc(menu_section.X, menu_section.Y, menu_section.Width - menu_section.X, 8);

    for(int A : treeBlockQuery(tempLocation, SORTMODE_NONE))
    {
        if(CheckCollision(Block[A].Location, tempLocation))
            return;
    }

    for(int A : treeBackgroundQuery(tempLocation, SORTMODE_NONE))
    {
        if(CheckCollision(Background[A].Location, tempLocation))
            return;
    }

    // expand level height to a maximum of 2160px
    if(menu_section.Y > menu_section.Height - 2160)
        menu_section.Y = menu_section.Height - 2160;
}

void MainLoadAll()
{
    LoadingInProcess = true;

    if(g_AssetsLoaded)
    {
        StopAllSounds();
        StopMusic();

        UnloadSound();
        UnloadGFX(true);
    }

    if(ScreenAssetPack::g_LoopActive)
        PlayInitSound();

    if(FontManager::isInitied())
        FontManager::quit();

    LoaderInit();

    LoaderUpdateDebugString("Fonts");

    FontManager::initFull();

#ifndef PGE_NO_THREADING
    {
        gfxLoaderThreadingMode = true;

        SDL_Thread*     loadThread;
        SDL_atomic_t    loadWaiter;
        int             loadWaiterState = 1;
        int             threadReturnValue;

        SDL_AtomicSet(&loadWaiter, loadWaiterState);
        loadThread = SDL_CreateThread(loadingThread, "Loader", &loadWaiter);

        if(!loadThread)
        {
            gfxLoaderThreadingMode = false;
            pLogCritical("Failed to create the loading thread! Do running the load directly");
            loadingThread(nullptr);
        }
        else
        {
            do
            {
                UpdateLoadREAL();
                PGE_Delay(15);
                loadWaiterState = SDL_AtomicGet(&loadWaiter);
            } while(loadWaiterState);

            SDL_WaitThread(loadThread, &threadReturnValue);
            pLogDebug("Loading thread was exited with %d code.", threadReturnValue);
        }
    }
#else
    loadingThread(nullptr);
#endif

    Integrator::setGameName(g_gameInfo.title, g_gameInfo.statusIconName);
    XWindow::setTitle(g_gameInfo.titleWindow().c_str());
    XWindow::updateWindowIcon();

    LoaderFinish();
    LoadingInProcess = false;

    g_AssetsLoaded = true;
}


int GameMain(const CmdLineSetup_t &setup)
{
    Player_t blankPlayer;
//    int A = 0;
//    int B = 0;
//    int C = 0;
    bool tempBool = false;
    int lastWarpEntered = 0;

//    LB = "\n";
//    EoT = "";

    // moved into MainLoadAll
    // cheats_reset();

    // [ !Here was a starting dialog! ]

    //    frmLoader.Show 'show the Splash screen
    //    Do
    //        DoEvents
    //    Loop While StartMenu = False 'wait until the player clicks a button

    // Set global SMBX64 behaviour at PGE-FL
    FileFormats::SetSMBX64LvlFlags(FileFormats::F_SMBX64_KEEP_LEGACY_NPC_IN_BLOCK_CODES);

    StartMenu = true;
    MenuMode = MENU_INTRO;

    // strings and translation initialization moved into MainLoadAll
#if 0
    initOutroContent();
    initMainMenu();
    initEditorStrings();
    initGameStrings();

    if(!CurrentLanguage.empty())
    {
        XTechTranslate translator;
        if(translator.translate())
        {
            pLogDebug("Loaded translation for language %s-%s",
                      CurrentLanguage.c_str(),
                      CurrentLangDialect.empty() ? "??" : CurrentLangDialect.c_str());
        }
    }
#endif

    initAll();

//    Unload frmLoader
    gfxLoaderTestMode = setup.testLevelMode;

    // find asset pack and load required UI graphics
    if(!InitUIAssetsFrom(setup.assetPack))
        return 1;

//    If LevelEditor = False Then
//        frmMain.Show // Show window a bit later
//    XWindow::show();
//        GameMenu = True
    GameMenu = true;
//    Else
//        frmSplash.Show
//        BlocksSorted = True
//    End If

    LoadingInProcess = true;

    XEvents::doEvents();

#ifdef __EMSCRIPTEN__ // Workaround for a recent Chrome's policy to avoid sudden sound without user's interaction
    FontManager::initFull();

    XWindow::show(); // Don't show window until playing an initial sound

    while(!SharedCursor.Primary)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();
        SuperPrint("Click to start a game", 3, 230, 280);
        XRender::repaint();
        XRender::setTargetScreen();
        XEvents::doEvents();
        Controls::Update(false);
        PGE_Delay(10);
    }
#endif

    InitMixerX();

#ifndef PGE_NO_THREADING
    gfxLoaderThreadingMode = true;
#endif
    XWindow::show(); // Don't show window until playing an initial sound

    if(!setup.testLevelMode)
        PlayInitSound();

#ifdef THEXTECH_INTERPROC_SUPPORTED
    if(setup.interprocess)
        IntProc::init();
#endif

    Integrator::initIntegrations();

    // want to go directly to game content
    bool cmdline_content = (!setup.testLevel.empty() || !setup.testReplay.empty() || setup.interprocess);

    // special case: go straight to asset pack menu
    if(g_config.pick_assets_on_start && !cmdline_content && setup.assetPack.empty() && GetAssetPacks().size() > 1)
    {
        FontManager::initFull();
        Controls::LoadTouchScreenGFX();
        GameMenu = false;
        ScreenAssetPack::g_LoopActive = true;
    }
    // normal case: load everything and go to menu
    else
        MainLoadAll();

    LevelSelect = true; // world map is to be shown

    LoadingInProcess = false;

    // Clear the screen
    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    XEvents::doEvents();

    if(!g_config.background_work && !XWindow::hasWindowInputFocus())
        SoundPauseEngine(1);

    if(cmdline_content) // Start level testing immediately!
    {
        bool is_world = (Files::hasSuffix(setup.testLevel, ".wld") || Files::hasSuffix(setup.testLevel, ".wldx"));

        GameMenu = false;
        LevelSelect = is_world;

        if(!setup.testReplay.empty())
            Record::LoadReplay(setup.testReplay, setup.testLevel);
        else
            FullFileName = setup.testLevel;

        if(setup.testBattleMode && !is_world)
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

        if(!is_world)
        {
            GodMode = setup.testGodMode;
            GrabAll = setup.testGrabAll;

            if(GodMode || GrabAll)
                Cheater = true;
        }

        editorScreen.ResetCursor();

        if(setup.testReplay.empty() && setup.testEditor)
        {
            editorScreen.active = false;
            MouseRelease = false;
            LevelEditor = true;
            WorldEditor = is_world;
            OpenLevel(FullFileName);
            editorScreen.ResetCursor();
            EditorBackup();
        }
        else if(is_world)
        {
            LoadSingleWorld(setup.testLevel);

            selWorld = 1;

            if(SelectWorld[selWorld].WorldPath.empty())
            {
                LevelSelect = false;
                TestLevel = true;
                EndLevel = false;
                MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, setup.testLevel);
                PauseGame(PauseCode::Message);
                ErrorQuit = true;
            }
            else
            {
                if(setup.testSave >= 0 && setup.testSave <= maxSaveSlots)
                {
                    selSave = setup.testSave;
                    FindSaves();
                }

                if(numPlayers < 1)
                    numPlayers = 1;

                for(int A = 1; A <= numCharacters; A++)
                    blockCharacter[A] = (g_forceCharacter) ? false : SelectWorld[selWorld].blockChar[A];

                // prepare for StartEpisode(): set player characters
                for(int i = 0; i < numPlayers; i++)
                {
                    if(testPlayer[i + 1].Character != 0)
                        l_screen->charSelect[i] = testPlayer[i + 1].Character;
                    else
                        l_screen->charSelect[i] = i + 1;

                    // replace blocked characters
                    if(blockCharacter[l_screen->charSelect[i]])
                    {
                        for(int new_char = 1; new_char <= numCharacters; new_char++)
                        {
                            // check it's unblocked
                            if(blockCharacter[new_char])
                                continue;

                            // check no other player has the character first
                            int j = 0;
                            for(; j < i; j++)
                            {
                                if(l_screen->charSelect[j] == new_char)
                                    break;
                            }

                            // if loop ended naturally, character is unused
                            if(j == i)
                            {
                                l_screen->charSelect[i] = new_char;
                                break;
                            }
                        }
                    }

                    if(i <= (int)Controls::g_InputMethods.size())
                        Controls::g_InputMethods.push_back(nullptr);
                }

                QuickReconnectScreen::g_active = true;

                StartEpisode();
            }
        }
        else
        {
            zTestLevel(setup.testMagicHand, setup.interprocess);

            if(!LevelName.empty())
                Integrator::setLevelName(LevelName);
            else
                Integrator::setLevelName(FileName);
        }
    }

    do
    {
        if(GameMenu || MagicHand || LevelEditor || ScreenAssetPack::g_LoopActive)
        {
            XWindow::setCursor(CURSOR_NONE);
            XWindow::showCursor(0);
        }
        else if(!g_config.fullscreen)
        {
            XWindow::setCursor(CURSOR_DEFAULT);
            XWindow::showCursor(1);
        }


        if(ScreenAssetPack::g_LoopActive)
        {
            // make sure that controllers can connect properly
            numPlayers = maxLocalPlayers;

            // Run the frame-loop
            runFrameLoop(&ScreenAssetPack::Loop,
                         nullptr,
                        []()->bool{ return ScreenAssetPack::g_LoopActive;}, nullptr,
                        nullptr,
                        nullptr);
        }
        else if(LevelEditor) // Load the level editor
        {
            // if(resChanged)
            //     ChangeScreen();
            // BattleMode = false;
            SingleCoop = 0;
            numPlayers = 0;
            // ScreenType = 0; // set in SetupScreens()
            XEvents::doEvents();
            SetupEditorGraphics(); //Set up the editor graphics
            InitScreens();

            for(int i = 0; i < maxLocalPlayers; i++)
                Screens_AssignPlayer(i + 1, *l_screen);

            SetupScreens();
            MagicHand = false;
            MouseRelease = false;
            ScrollRelease = false;

            // coming back from a level test
            if(!WorldEditor)
                EditorRestore();

            // Run the frame-loop
            runFrameLoop(&EditorLoop,
                         nullptr,
                        []()->bool{ return LevelEditor || WorldEditor;}, nullptr,
                        nullptr,
                        nullptr);

            MenuMode = MENU_INTRO;
            LevelEditor = false;
            WorldEditor = false;

            if(GameIsActive)
            {
                XRender::clearBuffer();
                XRender::repaint();
#ifdef THEXTECH_PRELOAD_LEVELS
                FindWorlds();
#endif
            }
        }

        // TheXTech Credits
        else if(GameOutro)
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
                Player[A] = blankPlayer;

            numPlayers = g_gameInfo.outroMaxPlayersCount;
#ifdef __16M__
            if(numPlayers > 3)
                numPlayers = 3;
#endif
            if(g_gameInfo.outroDeadMode)
                numPlayers = 1; // Deadman mode
            GameMenu = false;
            StopMusic();

            auto outroPath = AppPath + "outro.lvlx";
            if(!Files::fileExists(outroPath))
                outroPath = AppPath + "outro.lvl";
            OpenLevel(outroPath);

            SetupScreens();
            ClearBuffer = true;

            for(int A = 1; A <= numPlayers; ++A)
            {
                Player_t &p = Player[A];

                if(A <= (int)g_gameInfo.outroStates.size())
                    p.State = g_gameInfo.outroStates[A - 1];
                else if(A == 1)
                    p.State = 4;
                else if(A == 2)
                    p.State = 7;
                else if(A == 3)
                    p.State = 5;
                else if(A == 4)
                    p.State = 3;
                else
                    p.State = 6;

                p.Character = g_gameInfo.outroCharacterNext();

                if(A <= (int)g_gameInfo.outroMounts.size())
                {
                    p.Mount = g_gameInfo.outroMounts[A - 1];
                    switch(p.Mount)
                    {
                    case 1:
                        p.MountType = iRand(3) + 1;
                        break;
                    case 3:
                        p.MountType = iRand(8) + 1;
                        break;
                    default:
                        p.MountType = 0;
                    }
                }
                else if(A == 4)
                {
                    p.Mount = 1;
                    p.MountType = iRand(3) + 1;
                }
                else if(A == 2)
                {
                    p.Mount = 3;
                    p.MountType = iRand(8) + 1;
                }

                p.HeldBonus = NPCID(0);
                p.Section = 0;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            }

            SetupPlayers();
            CreditChop = XRender::TargetH / 2; // 100
            EndCredits = 0;
            GameOutroDoQuit = false;
            SetupCredits();

            // Update graphics before loop begin (to process an initial lazy-unpacking of used sprites)
            GraphicsLazyPreLoad();
            resetFrameTimer();

            for(int A = 1; A <= numPlayers; ++A)
            {
                if(g_gameInfo.outroWalkDirection == 0 && A <= (int)g_gameInfo.outroInitialDirections.size())
                {
                    if(g_gameInfo.outroInitialDirections[A - 1] < 0)
                        Player[A].Direction = -1;
                    else if(g_gameInfo.outroInitialDirections[A - 1] > 0)
                        Player[A].Direction = 1;
                }
            }

            if(g_gameInfo.outroDeadMode)
            {
                CheckSection(1);
                for(int A = 1; A <= numPlayers; ++A)
                    Player[A].Dead = true;
            }

            lunaLoad();

            clearScreenFaders();

            // Run the frame-loop
            runFrameLoop(&OutroLoop,
                         nullptr,
                        []()->bool{ return GameOutro;}, nullptr,
                        nullptr,
                        []()->void
                        {
                            SetupScreens();
                        });
        }

        // quickly exit if returned to menu from world test
        else if(setup.testLevelMode && GameMenu)
        {
            GameIsActive = false;
        }

        // The Game Menu
        else if(GameMenu)
        {
            {
                ConfigChangeSentinel sent(ConfigSetLevel::ep_config);

                g_config.playstyle = Config_t::MODE_MODERN;
                if(g_config.speedrun_mode.m_set != ConfigSetLevel::cmdline)
                    g_config.speedrun_mode = 0;
            }

            Integrator::clearEpisodeName();
            Integrator::clearLevelName();
            Integrator::clearEditorFile();
            FontManager::clearAllCustomFonts();

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

            MenuMouseRelease = false;
            MenuMouseClick = false;
            MenuCursorCanMove = false;
            BattleMode = false;

            if(MenuMode != MENU_BATTLE_MODE)
            {
                PlayerCharacter = 0;
                PlayerCharacter2 = 0;
            }

            pLogDebug("Clear check-points at Game Menu start");
            Checkpoint.clear();
            CheckpointsList.clear();
            g_curLevelMedals.reset_checkpoint();
            WorldPlayer[1].Frame = 0;
            cheats_clearBuffer();
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
            g_ForceBitmaskMerge = false;
            g_ClonedPlayerMode = false;
            g_CheatLogicScreen = false;
            g_CheatEditYourFriends = false;
            XRender::unloadGifTextures();

            // reset l_screen to index 0
            Screens[0] = *l_screen;
            l_screen = &Screens[0];

            // reset non-local screens' charSelect
            for(int s = 1; s < maxNetplayClients; s++)
                Screens[s].charSelect = {};

            // reinitialize the screens (resets multiplayer preferences and restores state disrupted by reassigning Screens[0])
            InitScreens();

            for(int i = 0; i < maxLocalPlayers; i++)
                Screens_AssignPlayer(i + 1, *l_screen);

            SetupScreens();

            BattleOutro = 0;
            BattleIntro = 0;

            for(int A = 1; A <= maxPlayers; ++A)
                Player[A] = blankPlayer;

            numPlayers = g_gameInfo.introMaxPlayersCount;
#ifdef __16M__
            if(numPlayers > 3)
                numPlayers = 3;
#endif
            if(g_gameInfo.introDeadMode)
                numPlayers = 1;// one deadman should be

            auto introPath = findIntroLevel();
            if(introPath.empty())
            {
                XMsgBox::errorMsgBox("Fatal error",
                                     "Can't find any intro level file to start the main menu.\n"
                                     "The game will be closed.\n"
                                     "\n"
                                     "Please make sure the intro.lvlx or intro.lvl file is exist\n"
                                     "in the game assets directory, or make sure the \"introset\" directory\n"
                                     "contains any valid level files.");
                return 1;// Fatal error happen
            }

            OpenLevel(introPath);
            Screens[0].vScreen(1).X = -level[0].X;
            Screens[0].canonical_screen().vScreen(1).X = -level[0].X;
            s_ExpandSectionForMenu();

            if(g_config.EnableInterLevelFade)
                g_levelScreenFader.setupFader(3, 65, 0, ScreenFader::S_FADE);
            else
                clearScreenFaders();

            setMusicStartDelay(); // Don't start music until all gfx will be loaded

            StartMusic(0);
            SetupPlayers();

            For(A, 1, numPlayers)
            {
                Player_t &p = Player[A];
                p.State = iRand(6) + 2;
                // p.Character = (iRand() % 5) + 1;

                // if(A >= 1 && A <= 5)
                p.Character = g_gameInfo.introCharacterNext();

                p.HeldBonus = NPCID(0);
                p.Section = 0;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.X = level[p.Section].X + ((128 + dRand() * 64) * A);
                p.Location.Y = level[p.Section].Height - p.Location.Height - 65;

                do
                {
                    tempBool = true;
                    for(int B : treeBlockQuery(p.Location, SORTMODE_NONE))
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

            // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
            GraphicsLazyPreLoad();
            resetFrameTimer();
            // Clear the speed-runner timer
            speedRun_resetTotal();

            lunaLoad();

            delayedMusicStart(); // Allow music being started

            ProcEvent(EVENT_LEVEL_START, 0, true);
            For(A, 2, maxEvents)
            {
                if(Events[A].AutoStart)
                    ProcEvent(A, 0, true);
            }

            // Main menu loop
            runFrameLoop(&MenuLoop, nullptr, []()->bool{ return GameMenu;});
            if(!GameIsActive)
            {
                speedRun_saveStats();
                return 0;// Break on quit
            }
        }

        // World Map
        else if(LevelSelect)
        {
            cheats_clearBuffer();

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

            if(!NoMap)
            {
                // Restore the previously preserved world map paths
                FileNameFull = FileNameFullWorld;
                FileName = FileNameWorld;
                FileNamePath = FileNamePathWorld;
                FileFormat = FileFormatWorld;

                // Recalculate the FullFileName
                FullFileName = FileNamePath + FileNameFull;
            }

            LoadCustomConfig();
            FindCustomPlayers();
            LoadCustomGFX();
            LoadCustomSound();
            SetupPlayers();
            FontManager::loadCustomFonts();

            if(!NoMap)
                FindWldStars();

            if((!StartLevel.empty() && NoMap) || !GoToLevel.empty() || !FileRecentSubHubLevel.empty())
            {
                if(NoMap)
                    SaveGame();

                Player[1].Vine = 0;
                Player[2].Vine = 0;

//                if(!GoToLevelNoGameThing)
//                    PlaySound(SFX_LevelSelect);
                SoundPause[26] = 2000;

                LevelSelect = false;

                XRender::setTargetTexture();
                XRender::clearBuffer();
                XRender::repaint();

                lunaReset();
                ResetSoundFX();
                ClearLevel();

                std::string levelPath;
                if(GoToLevel.empty())
                {
                    if(FileRecentSubHubLevel.empty())
                        levelPath = SelectWorld[selWorld].WorldPath + StartLevel;
                    else
                        levelPath = SelectWorld[selWorld].WorldPath + FileRecentSubHubLevel;
                }
                else
                {
                    levelPath = SelectWorld[selWorld].WorldPath + GoToLevel;
                    GoToLevel.clear();
                }

                if(!OpenLevel(levelPath))
                {
                    MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, levelPath);
                    PauseGame(PauseCode::Message);
                    ErrorQuit = true;
                }

                if(!GoToLevelNoGameThing)
                {
                    GameThing(1000, 3);
                }
                else
                {
                    XRender::setTargetTexture();
                    XRender::clearBuffer();
                    XRender::repaint();
                }
            }
            else
            {
                ResetSoundFX();
                setMusicStartDelay(); // Don't start music until all gfx will be loaded

                worldResetSection();

                if(curWorldMusic > 0)
                    StartMusic(curWorldMusic);

                resetFrameTimer();
                speedRun_resetCurrent();

                // On a world map, reset this into default state
                GoToLevelNoGameThing = false;

                // Clear the recent hub level when entering a world map
                FileRecentSubHubLevel.clear();

                // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
                UpdateGraphics2(true);
                resetFrameTimer();

                if(g_config.EnableInterLevelFade)
                    g_worldScreenFader.setupFader(4, 65, 0, ScreenFader::S_FADE);
                else
                    g_worldScreenFader.clearFader();

                // WorldLoop will automatically resume the music as needed
                // delayedMusicStart(); // Allow music being started
                worldResetSection();

                // 'level select loop
                runFrameLoop(nullptr, &WorldLoop,
                             []()->bool{return LevelSelect;},
                             nullptr,
                             []()->void{FreezeNPCs = false;});

                if(!GameIsActive)
                {
                    speedRun_saveStats();
                    return 0;// Break on quit
                }
            }
        }

        // MAIN GAME
        else
        {
            cheats_clearBuffer();
            EndLevel = false;

            Record::InitRecording(); // initializes level data recording

            for(int A = 1; A <= numPlayers; ++A)
            {
                if(Player[A].Mount == 2)
                    Player[A].Mount = 0; // take players off the clown car
            }

            setMusicStartDelay(); // Don't start music until all gfx will be loaded

            SetupPlayers(); // Setup Players for the level

            if(LevelRestartRequested && Checkpoint.empty())
                StartWarp = lastWarpEntered; // When restarting a level (after death), don't restore an entered warp on checkpoints

            if(IsHubLevel && StartWarp > 0) // Save the warp where player entered the hub
                ReturnWarpSaved = StartWarp;
            else if(IsHubLevel && ReturnWarp == 0)
                ReturnWarpSaved = 0;

            qScreen = false;
            qScreen_canonical = false;
            LevelRestartRequested = false;

            if(lastWarpEntered != StartWarp)
                lastWarpEntered = StartWarp; // Re-use it when player re-enters a level after death (when option is toggled on)

// for warp entrances
            if((ReturnWarp > 0 && (IsEpisodeIntro || IsHubLevel)/*FileName == StartLevel*/) || (StartWarp > 0))
            {
                for(int numPlayersMax = numPlayers, A = 1; A <= numPlayersMax; ++A)
                {
                    Player_t &p = Player[A];

                    if(StartWarp > 0)
                        p.Warp = StartWarp;
                    else
                        p.Warp = ReturnWarp;

                    p.WarpBackward = false;
                    auto &warp = Warp[p.Warp];

                    if(warp.Effect == 1)
                    {
                        if(warp.Direction2 == 1) // DOWN
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = warp.Exit.X + (warp.Exit.Width / 2) - (p.Location.Width / 2);
//                                .Location.Y = Warp(.Warp).Exit.Y - .Location.Height - 8
                            p.Location.Y = warp.Exit.Y - p.Location.Height - 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 3 Then
                        if(warp.Direction2 == 3) // UP
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = warp.Exit.X + (warp.Exit.Width / 2) - (p.Location.Width / 2);
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height + 8
                            p.Location.Y = warp.Exit.Y + warp.Exit.Height + 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 2 Then
                        if(warp.Direction2 == 2) // RIGHT
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X - .Location.Width - 8
                            p.Location.X = warp.Exit.X - p.Location.Width - 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.Location.Y = warp.Exit.Y + warp.Exit.Height - p.Location.Height - 2;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 4 Then
                        if(warp.Direction2 == 4) // LEFT
                        {
//                                If .Mount = 3 Then .Duck = True
                            if(p.Mount == 3) p.Duck = true;
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width + 8
                            p.Location.X = warp.Exit.X + warp.Exit.Width + 8;
//                                .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height - 2
                            p.Location.Y = warp.Exit.Y + warp.Exit.Height - p.Location.Height - 2;
//                            End If
                        }

                        PlayerFrame(p);
                        CheckSection(A);
                        SoundPause[17] = 0;
                        p.Effect = PLREFF_WAITING;
                        p.Effect2 = 950;
                    }
                    else if(warp.Effect == 2)
                    {
//                            .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                        p.Location.X = warp.Exit.X + warp.Exit.Width / 2 - p.Location.Width / 2;
//                            .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height
                        p.Location.Y = warp.Exit.Y + warp.Exit.Height - p.Location.Height;

                        CheckSection(A);
                        p.Effect = PLREFF_WAITING;
                        p.Effect2 = 2000;
                    }
                    else if(warp.Effect == 3) // Portal warp
                    {
                        p.Location.X = warp.Exit.X + warp.Exit.Width / 2 - p.Location.Width / 2;
                        p.Location.Y = warp.Exit.Y + warp.Exit.Height - p.Location.Height;
                        CheckSection(A);
                        p.WarpCD = 50;
                    }
                }

                if(StartWarp > 0)
                {
                    StartWarp = 0;

                    // fix a bug where ReturnWarp (from different level) would be used at hub after death if StartWarp was set for hub
                    if(IsHubLevel && g_config.enable_last_warp_hub_resume)
                        ReturnWarp = 0;
                }
                else
                    ReturnWarp = 0;
            }

            // ---------------------------------------
            //    Verify if level can run or not
            // ---------------------------------------
            bool hasPlayerPoint = false;
            bool hasStartWarp = (Player[1].Warp > 0);
            bool hasValidStartWarp = (Player[1].Warp > 0 && Player[1].Warp <= numWarps);
            bool startError = false;

            for(int i = 1; i <= numPlayers && i <= 2; ++i)
                hasPlayerPoint |= !PlayerStart[i].isNull();

            if(hasStartWarp && !hasValidStartWarp)
            {
                MessageText = fmt::format_ne(g_gameStrings.errorInvalidEnterWarp,
                                             FullFileName,
                                             Player[1].Warp,
                                             numWarps);
                startError = true;
            }
            else if(!hasPlayerPoint && !hasStartWarp)
            {
                MessageText = fmt::format_ne(g_gameStrings.errorNoStartPoint, FullFileName);
                startError = true;
            }

            if(startError) // Quit the level because of error
            {
                // Mark all players as dead
                for(int A = 1; A <= numPlayers; A++)
                    Player[A].Dead = true;

                PauseGame(PauseCode::Message);

                if(g_config.modern_lives_system)
                    ++g_100s;
                else
                    ++Lives;

                EveryonesDead();
                clearScreenFaders();

                if(BattleMode && !LevelEditor && !setup.testLevelMode)
                {
                    BattleMode = false;
                    GameMenu = true;
                    MenuMode = MENU_BATTLE_MODE;
                    MenuCursor = selWorld - 1;
                }
            }
            else // Run the level normally
            {
                speedRun_resetCurrent();
//'--------------------------------------------

                // Update graphics before loop begin (to process inital lazy-unpacking of used sprites)
                GraphicsLazyPreLoad();
                resetFrameTimer();

                speedRun_triggerEnter();

                clearScreenFaders(); // Reset all faders
                if(g_config.EnableInterLevelFade)
                    g_levelScreenFader.setupFader(2, 65, 0, ScreenFader::S_FADE);

                lunaLoad();

                delayedMusicStart(); // Allow music being started

                ProcEvent(EVENT_LEVEL_START, 0, true);
                for(int A = 2; A <= maxEvents; ++A)
                {
                    if(Events[A].AutoStart)
                        ProcEvent(A, 0, true);
                }

                // MAIN GAME LOOP
                runFrameLoop(nullptr, &GameLoop,
                []()->bool{return !LevelSelect && !GameMenu;},
                []()->bool
                {
                    if(!LivingPlayers())
                    {
                        EveryonesDead();
                        return true;
                    }
                    return false;
                });
            }

            // store to level save info if level won
            if(LevelBeatCode > 0 || !GoToLevel.empty())
            {
                CommitBeatCode(LevelBeatCode);
                g_curLevelMedals.commit();
            }
            // otherwise, reset the medal count
            else
                g_curLevelMedals.on_all_dead();

            Record::EndRecording();

            StopAllSounds();
            UnloadExtSounds();

            if(!GameIsActive)
            {
                speedRun_saveStats();
                return 0;// Break on quit
            }

            // TODO: Utilize this and any TestLevel/MagicHand related code to allow PGE Editor integration
            // (do any code without interaction of no more existnig Editor VB forms, keep IPS with PGE Editor instead)

//            If TestLevel = True Then
            if(TestLevel)
            {
                // provide option to restart (was previously restricted to fails and command-line runs)
                // if(LevelBeatCode == 0 || (setup.testLevelMode && LevelBeatCode >= 0))
                if(LevelBeatCode >= 0)
                {
                    LevelSelect = false;
                    LevelBeatCode = -2; // checked in PauseScreen::Init()
                    PauseGame(PauseCode::PauseScreen);
                }

                // check that we are still restarting (it could have been canceled above)
                if(LevelBeatCode == 0 || LevelBeatCode == -2)
                {
                    GameThing();
                    zTestLevel(setup.testMagicHand || editorScreen.test_magic_hand, setup.interprocess); // Restart level
                }
                // from editor, return to editor
                else if(!Backup_FullFileName.empty())
                {
                    // printf("returning to editor...\n");

                    TestLevel = false;
                    LevelEditor = true;
                    GameMenu = false;
                    SetupPlayers();

                    // reopen the temporary level (FullFileName)
                    OpenLevel(FullFileName);

                    // reset FullFileName to point to the real level (Backup_FullFileName)
                    Files::deleteFile(FullFileName);
                    FullFileName = Backup_FullFileName;
                    // this is needed because the temporary levels are currently saved as ".lvl(x)tst"
                    if(FileNameFull.size() > 3 && FileNameFull.substr(FileNameFull.size() - 3) == "tst")
                        FileNameFull.resize(FileNameFull.size() - 3);

                    Backup_FullFileName = "";

                    Integrator::setEditorFile(FileName);

                    editorScreen.active = false;
                    MouseRelease = false;

                    if(g_config.EnableInterLevelFade)
                        g_levelScreenFader.setupFader(3, 65, 0, ScreenFader::S_FADE);
                }
                // from command line, close (if player has requested to stop testing)
                else if(setup.testLevelMode)
                {
                    GameIsActive = false;
                }

                LevelBeatCode = 0;

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
            else if(!LevelRestartRequested)
            {
                lunaReset();
                ResetSoundFX();
                ClearLevel();
//            End If
            } // TestLevel
        }

    } while(GameIsActive);

    Integrator::quitIntegrations();

    return 0;
}

// game_main_setupvars.cpp

// game_loop.cpp

// menu_loop.cpp

void EditorLoop()
{
    Controls::Update();
    Integrator::sync();
    UpdateEditor();
    UpdateBlocks();
    UpdateEffects();
    if(WorldEditor)
        UpdateGraphics2();
    else
        UpdateGraphics();

    updateScreenFaders();

    // TODO: if there's a second screen, draw editor screen there too

    UpdateSound();
}

void KillIt()
{
#ifdef __WIIU__
    if(!GameIsActive)
        return; // Don't call this twice
#endif

    GameIsActive = false;
    Integrator::quitIntegrations();
#ifndef RENDER_FULLSCREEN_ALWAYS
    XWindow::hide();
    if(g_config.fullscreen)
        SetOrigRes();
#else
    XRender::clearBuffer();
    XRender::repaint();
#endif
    lunaReset();
    QuitMixerX();
    UnloadGFX();
    XWindow::showCursor(1);
}


void NextLevel()
{
    int A = 0;

    for(A = 1; A <= numPlayers; A++)
        Player[A].HoldingNPC = 0;

    LevelMacro = LEVELMACRO_OFF;
    LevelMacroCounter = 0;
    LevelMacroWhich = 0;
    StopMusic();
    lunaReset();
    ResetSoundFX();
    ClearLevel();
    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    XEvents::doEvents();

    if(!TestLevel && GoToLevel.empty() && !NoMap && !g_config.unlimited_framerate)
        PGE_Delay(500);

    if(BattleMode && !LevelEditor && !TestLevel)
    {
        EndLevel = false;
        GameMenu = true;
        MenuMode = MENU_BATTLE_MODE;
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

            // if(!LevelEditor && Backup_FullFileName.empty())
            //     GameIsActive = false; // Quit game
        }
    }
}

// macros mainly used for end of level stuffs. takes over the players controls
void UpdateMacro()
{
    int A = 0;
    bool OnScreen = false;

#ifdef THEXTECH_INTERPROC_SUPPORTED
    if(LevelMacro != LEVELMACRO_OFF && LevelMacroCounter == 0 && IntProc::isEnabled())
    {
        for(int i = 0; i < numPlayers; ++i)
        {
            auto &p = Player[i + 1];
            IntProc::sendPlayerSettings(i, p.Character, p.State, p.Mount, p.MountType);
        }
    }
#endif

    if(LevelMacro == LEVELMACRO_CARD_ROULETTE_EXIT) // SMB3 Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            auto &p = Player[A];
            auto &c = p.Controls;
            if(p.Location.X < level[p.Section].Width && !p.Dead)
            {
                OnScreen = true;
                c.Down = false;
                c.Drop = false;
                c.Jump = false;
                c.Left = false;
                c.Right = true;
                c.Run = false;
                c.Up = false;
                c.Start = false;
                c.AltJump = false;
                c.AltRun = false;
                if(p.Wet > 0 && p.CanJump)
                {
                    if(p.Location.SpeedY > 1)
                        c.Jump = true;
                }
            }
            else
            {
                p.Location.SpeedY = -Physics.PlayerGravity;
                c.Down = false;
                c.Drop = false;
                c.Jump = false;
                c.Left = false;
                c.Right = true;
                c.Run = false;
                c.Up = false;
                c.Start = false;
                c.AltJump = false;
                c.AltRun = false;
            }
        }

        // !OnScreen requires Player to leave Screen during normal play.
        // is_cheat ensures the ItsVegas Cheat doesn't Softlock the game.
        bool is_cheat = (LevelMacroWhich == -1);

        if(!OnScreen || is_cheat)
        {
            LevelMacroCounter++;

            if(g_config.EnableInterLevelFade &&
                ((LevelMacroCounter == 34 && !is_cheat)
                || (LevelMacroCounter == 250 && is_cheat)))
            {
                g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);
            }

            if((!is_cheat && LevelMacroCounter >= 100) || (is_cheat && LevelMacroCounter >= 316))
            {
                LevelBeatCode = 1;
                LevelMacro = LEVELMACRO_OFF;
                LevelMacroCounter = 0;
                EndLevel = true;
            }
        }
    }
    else if(LevelMacro == LEVELMACRO_QUESTION_SPHERE_EXIT)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            auto &c = Player[A].Controls;
            c.Down = false;
            c.Drop = false;
            c.Jump = false;
            c.Left = false;
            c.Right = false;
            c.Run = false;
            c.Up = false;
            c.Start = false;
            c.AltJump = false;
            c.AltRun = false;
        }

        LevelMacroCounter++;

        if(g_config.EnableInterLevelFade && LevelMacroCounter == 395)
            g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

        if(LevelMacroCounter >= 460)
        {
            LevelBeatCode = 2;
            EndLevel = true;
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroCounter = 0;
            XRender::clearBuffer();
        }
    }
    else if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
    {
        const int keyholeMax = 192; // Was 300

        do
        {
            XEvents::doEvents();

            if(canProceedFrame())
            {
                computeFrameTime1();

                speedRun_tick();
                Controls::Update(false);
                UpdateGraphics();
                UpdateSound();
                BlockFrames();

                XEvents::doEvents();
                computeFrameTime2();

                updateScreenFaders();

                LevelMacroCounter++;

                if(g_config.EnableInterLevelFade && LevelMacroCounter == (keyholeMax - 65))
                    g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

                if(LevelMacroCounter >= keyholeMax) /*300*/
                    break;
            }

            if(!GameIsActive)
            {
                speedRun_saveStats();
                return;
            }

            if(!g_config.unlimited_framerate)
                PGE_Delay(1);
        } while(true);

        LevelBeatCode = 4;
        EndLevel = true;
        LevelMacro = LEVELMACRO_OFF;
        LevelMacroWhich = 0;
        LevelMacroCounter = 0;
        XRender::clearBuffer();
    }
    else if(LevelMacro == LEVELMACRO_CRYSTAL_BALL_EXIT)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            auto &c = Player[A].Controls;
            c.Down = false;
            c.Drop = false;
            c.Jump = false;
            c.Left = false;
            c.Right = false;
            c.Run = false;
            c.Up = false;
            c.Start = false;
            c.AltJump = false;
            c.AltRun = false;
        }

        LevelMacroCounter++;

        if(g_config.EnableInterLevelFade && LevelMacroCounter == 235)
            g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

        if(LevelMacroCounter >= 300)
        {
            LevelBeatCode = 5;
            EndLevel = true;
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroCounter = 0;
            XRender::clearBuffer();
        }
    }
    else if(LevelMacro == LEVELMACRO_GAME_COMPLETE_EXIT)
    {
        // numNPCs = 0
        for(A = 1; A <= numPlayers; A++)
        {
            auto &c = Player[A].Controls;
            c.Down = false;
            c.Drop = false;
            c.Jump = false;
            c.Left = false;
            c.Right = false;
            c.Run = false;
            c.Up = false;
            c.Start = false;
            c.AltJump = false;
            c.AltRun = false;
        }

        LevelMacroCounter++;

        if(LevelMacroCounter == 250)
            PlaySound(SFX_GameBeat);

        if(g_config.EnableInterLevelFade && LevelMacroCounter == 735)
            g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

        if(LevelMacroCounter >= 800)
        {
            EndLevel = true;
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroCounter = 0;
            if(!TestLevel)
            {
                BeatTheGame = true;
                SaveGame();
                GameOutro = true;
                MenuMode = MENU_INTRO;
                MenuCursor = 0;
            }
            XRender::clearBuffer();
        }
    }
    else if(LevelMacro == LEVELMACRO_STAR_EXIT) // Star Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            auto &c = Player[A].Controls;
            c.Down = false;
            c.Drop = false;
            c.Jump = false;
            c.Left = false;
            c.Right = false;
            c.Run = false;
            c.Up = false;
            c.Start = false;
            c.AltJump = false;
            c.AltRun = false;
        }

        LevelMacroCounter++;

        if(g_config.EnableInterLevelFade && LevelMacroCounter == 235)
            g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

        if(LevelMacroCounter >= 300)
        {
            LevelBeatCode = 7;
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
    else if(LevelMacro == LEVELMACRO_GOAL_TAPE_EXIT) // SMW Exit
    {
        for(A = 1; A <= numPlayers; A++)
        {
            auto &p = Player[A];
            auto &c = p.Controls;

            if(p.Location.X < level[p.Section].Width && !p.Dead)
            {
                c.Down = false;
                c.Drop = false;
                c.Jump = false;
                c.Left = false;
                c.Right = true;
                c.Run = false;
                c.Up = false;
                c.Start = false;
                c.AltJump = false;
                c.AltRun = false;
            }
            else
            {
                p.Location.SpeedY = -Physics.PlayerGravity;
                c.Down = false;
                c.Drop = false;
                c.Jump = false;
                c.Left = false;
                c.Right = true;
                c.Run = false;
                c.Up = false;
                c.Start = false;
                c.AltJump = false;
                c.AltRun = false;
            }
        }

        LevelMacroCounter++;

        if(g_config.EnableInterLevelFade && LevelMacroCounter == 598)
        {
            bool canTrack = (Player[1].Location.X < level[Player[1].Section].Width);
            double focusX = canTrack ?
                            Player[1].Location.X + Player[1].Location.Width / 2 :
                            level[Player[1].Section].Width;
            double focusY = Player[1].Location.Y + Player[1].Location.Height / 2;

            g_levelScreenFader.setupFader(2, 0, 65, ScreenFader::S_CIRCLE, true, focusX, focusY, 1);

            if(canTrack)
                g_levelScreenFader.setTrackedFocus(&Player[1].Location.X,
                                                   &Player[1].Location.Y,
                                                   Player[1].Location.Width / 2,
                                                   Player[1].Location.Height / 2);
        }

        if(LevelMacroCounter >= 630)
        {
            LevelBeatCode = 8;
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
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
            NPC[A].Location.Y += XnHfix;
            treeNPCUpdate(A);
        }
    }
}

void CheckActive()
{
    // It's useless on Emscripten as no way to check activity (or just differently)
    // and on Android as it has built-in application pauser
#if !defined(NO_WINDOW_FOCUS_TRACKING)
//    bool MusicPaused = false;
    bool focusLost = false;

    if(g_config.background_work)
        return;

    if(!GameIsActive)
        return;

//    If nPlay.Online = True Then Exit Sub
    // If LevelEditor = False And TestLevel = False Then Exit Sub
    // If LevelEditor = False Then Exit Sub
    while(!XWindow::hasWindowInputFocus())
    {
        XEvents::waitEvents();
//        If LevelEditor = True Or MagicHand = True Then frmLevelWindow.vScreen(1).MousePointer = 0
        if(LevelEditor || MagicHand)
            XWindow::showCursor(0);

        resetFrameTimer();

        resetTimeBuffer();
        //keyDownEnter = false;
        //keyDownAlt = false;

        if(!focusLost)
        {
            pLogDebug("Window Focus lost");
            focusLost = true;
        }

//        if(musicPlaying && !MusicPaused)
//        {
//            MusicPaused = true;
//            SoundPauseEngine(1);
//        }

        if(!GameIsActive)
        {
            speedRun_saveStats();
            break;
        }
    }

    if(focusLost)
        pLogDebug("Window Focus got back");

//    if(MusicPaused)
//        SoundPauseEngine(0);

/* // Useless condition
    if(MusicPaused)
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


Location_t newLoc(double X, double Y, double Width, double Height)
{
    Location_t ret;
    ret.X = X;
    ret.Y = Y;
    ret.Width = Width;
    ret.Height = Height;
    return ret;
}

Location_t roundLoc(const Location_t &inLoc, double grid)
{
    Location_t ret = inLoc;
    ret.X = Maths::roundTo(ret.X, grid);
    ret.Y = Maths::roundTo(ret.Y, grid);
    return ret;
}

void MoreScore(int addScore, const Location_t &Loc)
{
    vbint_t mult = 0; // dummy
    MoreScore(addScore, Loc, mult);
}

void MoreScore(int addScore, const Location_t &Loc, vbint_t &Multiplier)
{
    //int oldM = 0;
    int A = 0;

    if(GameMenu || GameOutro || BattleMode)
        return;

    A = addScore + Multiplier;

    if(A == 0)
        return;

    Multiplier++;

    if(Multiplier > 9)
        Multiplier = 8;

    if(A < addScore)
        A = addScore;

    if(A > 13)
        A = 13;

    if(Points[A] <= 5)
    {
        if(g_config.modern_lives_system)
            g_100s += Points[A];
        else
            Lives += Points[A];

        PlaySound(SFX_1up, Points[A] - 1);
    }
    else
        Score += Points[A];

    NewEffect(EFFID_SCORE, Loc);
    Effect[numEffects].Frame = A - 1;
}

void Got100Coins()
{
    if(g_config.modern_lives_system)
    {
        if(g_100s < 9999)
        {
            g_100s++;
            PlaySound(SFX_1up);
            Coins -= 100;
        }
        else
            Coins = 99;
    }
    else
    {
        if(Lives < 99)
        {
            Lives += 1;
            PlaySound(SFX_1up);
            Coins -= 100;
        }
        else
            Coins = 99;
    }
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

static void s_InitPlayersFromCharSelect()
{
    // load players from the screens
    numPlayers = 0;
    InitScreens();

    for(int s = 0; s < maxNetplayClients; s++)
    {
        Screen_t& screen = Screens[s];

        for(int i = 0; i < maxLocalPlayers; i++)
        {
            if(screen.charSelect[i] != 0)
            {
                numPlayers++;
                Screens_AssignPlayer(numPlayers, screen);

                Player_t& p = Player[numPlayers];
                p.State = 1;
                p.Mount = 0;
                p.Character = screen.charSelect[i];
                p.HeldBonus = NPCID(0);
                p.CanFly = false;
                p.CanFly2 = false;
                p.TailCount = 0;
                p.YoshiBlue = false;
                p.YoshiRed = false;
                p.YoshiYellow = false;
                p.Hearts = 0;
            }
        }
    }

    for(int i = (int)Controls::g_InputMethods.size() - 1; i >= l_screen->player_count; i--)
        Controls::DeleteInputMethodSlot(i);
}

void StartEpisode()
{
    For(A, 1, numCharacters)
    {
        SavedChar[A] = Player_t();
        SavedChar[A].Character = A;
        SavedChar[A].State = 1;
    }

    // load players from the screens
    s_InitPlayersFromCharSelect();

    ConnectScreen::SaveChars();

    numStars = 0;
    Coins = 0;
    Score = 0;
    g_100s = 3;
    Lives = 3;
    LevelSelect = true;
    GameMenu = false;
    UpdateInternalRes();
    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    StopMusic();
    XEvents::doEvents();

    // Note: this causes the rendered touchscreen controller to freeze with button pressed.
    if(!g_config.unlimited_framerate)
        PGE_Delay(500);

    ClearGame();
    FontManager::clearAllCustomFonts();

    std::string wPath = SelectWorld[selWorld].WorldPath + SelectWorld[selWorld].WorldFile;

    if(numPlayers == 1 && g_recentWorld1p != wPath)
    {
        g_recentWorld1p = wPath;
        SaveConfig();
    }
    else if(numPlayers >= 2 && g_recentWorld2p != wPath)
    {
        g_recentWorld2p = wPath;
        SaveConfig();
    }

    OpenWorld(wPath);

    if(selSave && SaveSlotInfo[selSave].Progress >= 0)
    {
        if(!NoMap)
            StartLevel.clear();
        LoadGame();
        speedRun_loadStats();
    }

    Integrator::setEpisodeName(WorldName);

    if(WorldUnlock)
    {
        For(A, 1, numWorldPaths)
        {
            TinyLocation_t tempLocation = WorldPath[A].Location;
            {
                TinyLocation_t &l = tempLocation;
                l.X += 4;
                l.Y += 4;
                l.Width -= 8;
                l.Height -= 8;
            }

            WorldPath[A].Active = true;

            For(B, 1, numScenes)
            {
                if(CheckCollision(tempLocation, Scene[B].Location))
                    Scene[B].Active = false;
            }
        }

        For(A, 1, numWorldLevels)
            WorldLevel[A].Active = true;
    }

    SetupPlayers();

    if(!StartLevel.empty() || !FileRecentSubHubLevel.empty())
    {
        // TODO: why did Wohlstand disable this?
        PlaySoundMenu(SFX_LevelSelect);
        SoundPause[26] = 200;
        LevelSelect = false;

        ResetSoundFX();
        // todo: update this!
        ClearLevel();

        std::string levelName = (FileRecentSubHubLevel.empty() ? StartLevel : FileRecentSubHubLevel);
        std::string levelPath = SelectWorld[selWorld].WorldPath + levelName;

        if(!OpenLevel(levelPath))
        {
            MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, levelName);
            PauseGame(PauseCode::Message);
            ErrorQuit = true;
        }

        GameThing(1000, 3);
    }
}

void StartBattleMode()
{
    int A = 0;
    Player_t blankPlayer;

    for(A = 1; A <= numCharacters; A++)
    {
        SavedChar[A] = blankPlayer;
        SavedChar[A].Character = A;
        SavedChar[A].State = 1;
    }

    s_InitPlayersFromCharSelect();

    for(int i = 1; i <= numPlayers; i++)
    {
        Player[i].State = 2;
        Player[i].Hearts = 2;
    }

    numStars = 0;
    Coins = 0;
    Score = 0;
    g_100s = 0;
    Lives = 99;
    for(int i = 1; i <= numPlayers; i++)
        BattleLives[i] = 3;
    LevelSelect = false;
    GameMenu = false;
    BattleMode = true;
    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    StopMusic();
    XEvents::doEvents();

    if(!g_config.unlimited_framerate)
        PGE_Delay(500);

    lunaReset();
    ResetSoundFX();
    ClearLevel();

    if(NumSelectBattle <= 1)
    {
        MessageText = g_mainMenu.errorBattleNoLevels;
        PauseGame(PauseCode::Message);
        ErrorQuit = true;
    }
    else
    {
        if(selWorld == 1)
            selWorld = (iRand(NumSelectBattle - 1)) + 2;
    }

    std::string levelPath = SelectBattle[selWorld].WorldPath + SelectBattle[selWorld].WorldFile;
    if(!OpenLevel(levelPath))
    {
        MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, SelectBattle[selWorld].WorldFile);
        PauseGame(PauseCode::Message);
        ErrorQuit = true;
    }
    SetupPlayers();

    BattleIntro = 150;
    BattleWinner = 0;
    BattleOutro = 0;
}
