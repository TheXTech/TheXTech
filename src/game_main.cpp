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

#ifndef PGE_NO_THREADING
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_thread.h>
#endif

#ifdef __WIIU__
#include <sysapp/launch.h>
#endif

#if defined(THEXTECH_ASSERTS_INGAME_MESSAGE) && !defined(THEXTECH_NO_SDL_BUILD)
#   include <CrashHandler/crash_handler.h>
#   include <signal.h>
#endif

#include <Logger/logger.h>
#include <Utils/files.h>
#include <Utils/strings.h>
#include <Archives/archives.h>
#include <AppPath/app_path.h>
#include <Integrator/integrator.h>
#include <PGE_File_Formats/file_formats.h>
#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif
#include <pge_delay.h>
#include <fmt_format_ne.h>
#include <sorting/tinysort.h>

#ifdef ENABLE_XTECH_LUA
#include "xtech_lua_main.h"
#endif

#ifdef THEXTECH_ENABLE_SDL_NET
#include "main/client_methods.h"
#endif

#include "sdl_proxy/sdl_timer.h"

#include "globals.h"
#include "game_main.h"
#include "gfx.h"

#include "config.h"
#include "message.h"
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
#include "main/game_loop_interrupt.h"

#include "main/level_medals.h"

#include "main/trees.h"

bool g_ShortDelay = false;

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

    LoaderUpdateDebugString(g_gameStrings.loaderStatusGameInfo);
    initGameInfo();
    cheats_reset();

    LoaderUpdateDebugString(g_gameStrings.loaderStatusTranslations);
    XLanguage::findLanguages(); // find present translations
    ReloadTranslations(); // load translations

    LoaderUpdateDebugString(g_gameStrings.loaderStatusAssetPacks);
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

    LoaderUpdateDebugString(g_gameStrings.loaderStatusFinishing, true);
    UpdateLoad();

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

/**
 * @brief Choice randomly one of level from the directory
 * @param out The selection path (absolute path)
 * @param dir Directory with levels, must end with /
 * @param rootIntro The fallback absolute path to level if introset not exists or empty
 * @return true if choice was done, otherwise, nothing
 */
static bool choiceFromIntroset(std::string &out, const std::string &dir, const std::string &rootIntro = std::string())
{
    DirMan introSet(dir);
    std::vector<std::string> intros;

    if(!introSet.exists() || !introSet.getListOfFiles(intros, {".lvl", "lvlx"}) || intros.empty())
    {
        if(rootIntro.empty())
            return false;
        else
        {
            out = rootIntro;
            return true;
        }
    }

    tinysort(intros.begin(), intros.end());

    for(auto &i : intros)
    {
        i.insert(0, dir);
        pLogDebug("Found introset intro level: %s", i.c_str());
    }

    // The final choice
    out = intros[iRand2T(intros.size())];

    return true;
}

static std::string findIntroLevel()
{
    std::string choice;

    // Try to find custom intros
    if(!g_recentWorldIntro.empty())
    {
        if(DirMan::exists(g_recentWorldIntro)) // If path is a directory
        {
            if(g_recentWorldIntro.back() != '/')
                g_recentWorldIntro.push_back('/');

            if(choiceFromIntroset(choice, g_recentWorldIntro))
                return choice;
        }
        else if(Files::fileExists(g_recentWorldIntro))
            return g_recentWorldIntro;

        // When found nothing
        Strings::dealloc(g_recentWorldIntro);
    }

    // Find default set
    choiceFromIntroset(choice, AppPath + "introset/", getIntrFile());

    return choice;
}

// mount an archive used in an episode (or level)'s path
static std::string s_prepare_episode_path(const std::string& path)
{
    std::string target_path = path;

    // parse and mount assets if possible
    if(target_path[0] == '@')
    {
        // mount target, then replace path
        auto archive_end = target_path.begin();
        // don't check for path end until after the first slash
        while(archive_end != target_path.end() && *archive_end != '/' && *archive_end != '\\')
            ++archive_end;
        while(archive_end != target_path.end() && *archive_end != ':')
            ++archive_end;

        if(archive_end != target_path.end())
        {
            *archive_end = '\0';

            if(Archives::mount_episode(target_path.c_str() + 1))
            {
                target_path.erase(target_path.begin() + 2, archive_end + 1);
                target_path[0] = ':';
                target_path[1] = 'e';
            }
            else
                *archive_end = ':';
        }
    }

    return target_path;
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

void ReportLoadFailure(const std::string& filename, bool isIPC)
{
#ifdef THEXTECH_ENABLE_SDL_NET
    XMessage::Disconnect();
#endif
#if !defined(THEXTECH_INTERPROC_SUPPORTED)
    UNUSED(isIPC);
#endif

    g_MessageType = MESSAGE_TYPE_SYS_WARNING;

    // temporarily store error code from load process in MessageTitle string
    std::swap(MessageText, MessageTitle);
#if defined(THEXTECH_INTERPROC_SUPPORTED)
    MessageText = isIPC ? g_gameStrings.errorOpenIPCDataFailed : fmt::format_ne(g_gameStrings.errorOpenFileFailed, filename);
#else
    MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, filename);
#endif

    // add error code from load process
    if(!MessageTitle.empty())
    {
        MessageText += '\n';
        MessageText += '\n';
        MessageText += MessageTitle;
        MessageTitle.clear();
    }

    PauseGame(PauseCode::Message);
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

            // Draw last frame
            UpdateLoadREAL();
        }
    }
#else
    loadingThread(nullptr);
    UpdateLoad();
#endif

    Integrator::setGameName(g_gameInfo.title, g_gameInfo.statusIconName);
    XWindow::setTitle(g_gameInfo.titleWindow().c_str());
    XWindow::updateWindowIcon();

    LoaderFinish();
    LoadingInProcess = false;

    g_AssetsLoaded = true;
}


#if defined(THEXTECH_ASSERTS_INGAME_MESSAGE) && !defined(THEXTECH_NO_SDL_BUILD)
static SDL_AssertState ingame_assert_sdl_handler(const SDL_AssertData *data, void *)
{
    CrashHandler::logAssertInfo(data);
    g_MessageType = MESSAGE_TYPE_SYS_FATAL_ASSERT;
    MessageTitle = "Fatal error!";
    MessageText =  fmt::sprintf_ne("Assertion condition has failed:\n"
                                   "\n"
                                   "File: %s(%d)\n"
                                   "Function: %s\n"
                                   "Condition: %s\n"
                                   "\n"
                                   "Game will be closed.\n\n"
                                   "See log for details:\n"
                                   "%s",
                                   data->filename, data->linenum,
                                   data->function,
                                   data->condition,
                                   getLogFilePath().c_str());
    PauseGame(PauseCode::Message);

    return SDL_ASSERTION_ABORT;
}

static void ingame_crash_msg_handler(const std::string &title, const std::string &message)
{
    CloseLog(); // Before this, last log message was written, so, no more logs will be printed
    g_MessageType = MESSAGE_TYPE_SYS_FATAL_ASSERT;
    MessageTitle = "Fatal error!";
    signal(SIGABRT, SIG_DFL);
    MessageText =  fmt::sprintf_ne("%s\n"
                                  "\n"
                                  "%s\n\n"
                                  "Game will be closed.\n\n"
                                  "See log for details:\n"
                                  "%s",
                                  title.c_str(),
                                  message.c_str(),
                                  getLogFilePath().c_str());
    PauseGame(PauseCode::Message);
}
#endif


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

    // StartMenu = true;
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

    UpdateInternalRes();

//    Unload frmLoader
    gfxLoaderTestMode = setup.testLevelMode;

    // find asset pack and load required UI graphics
    bool init_failure = !InitUIAssetsFrom(setup.assetPack);

    if(init_failure)
    {
        if(!setup.assetPack.empty())
            return 1;

        FontManager::initFallback();
    }

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
    if(!init_failure)
        FontManager::initFull();

    XWindow::show(); // Don't show window until playing an initial sound

    while(!SharedCursor.Primary)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();
        SuperPrintScreenCenter("Click to start a game", 5, XRender::TargetH / 2 - 10);
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

#if defined(THEXTECH_ASSERTS_INGAME_MESSAGE) && !defined(THEXTECH_NO_SDL_BUILD)
    SDL_SetAssertionHandler(&ingame_assert_sdl_handler, NULL);
    CrashHandler::setCrashMsgBoxHook(&ingame_crash_msg_handler);
#endif

    if(!setup.testLevelMode && !init_failure)
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
    else if(!init_failure)
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

    if(init_failure)
    {
        GameMenu = false;
        LevelSelect = false;
        gSMBXHUDSettings.skip = true;
        numPlayers = 1;
        InitScreens();
        Screens_AssignPlayer(1, *l_screen);
        QuickReconnectScreen::g_active = true;

        g_MessageType = MESSAGE_TYPE_SYS_ERROR;
        MessageTitle = "Fatal: assets not found";
        MessageText = "Please extract a game/asset pack to:\n";

        for(auto& i : AppPathManager::assetsSearchPath())
        {
            MessageText += "\n";
            MessageText += i.first;

            if(i.second == AssetsPathType::Legacy)
                MessageText += "assets/<pack-id>/";
            else if(i.second == AssetsPathType::Multiple)
                MessageText += "<pack-id>/";
        }

        PauseGame(PauseCode::Message);
        GracefulQuit();
        return 0;
    }
    else if(cmdline_content) // Start level testing immediately!
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

            if(SelectWorld[selWorld].WorldFilePath.empty())
            {
                LevelSelect = false;
                TestLevel = true;
                EndLevel = false;

                ReportLoadFailure(setup.testLevel);
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

    while(GameIsActive)
    {
        SyncSysCursorDisplay();

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

            g_VanillaCam = false;
            UpdateInternalRes();

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

            g_VanillaCam = false;
            UpdateInternalRes();

            for(int A = 1; A <= maxPlayers; A++)
                Player[A] = blankPlayer;

            GameMenu = false;
            StopMusic();

            std::string outroPath;

            if(!g_recentWorldOutro.empty() && Files::fileExists(g_recentWorldOutro))
                outroPath = g_recentWorldOutro;
            else
            {
                outroPath = AppPath + "outro.lvlx";

                if(!Files::fileExists(outroPath))
                    outroPath = AppPath + "outro.lvl";
            }

            OpenLevel(outroPath);

            numPlayers = g_gameInfo.outroMaxPlayersCount;
#ifdef __16M__
            if(numPlayers > 3)
                numPlayers = 3;
#endif
            if(g_gameInfo.outroDeadMode)
                numPlayers = 1; // Deadman mode

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

            // Ensure everything is clear
            GraphicsClearScreen();
            XEvents::doEvents();
        }

        // quickly exit if returned to menu from world test
        else if(setup.testLevelMode && GameMenu)
        {
            GracefulQuit();
            return 0;
        }

        // The Game Menu
        else if(GameMenu)
        {
            {
                g_config.playstyle.m_value = Config_t::MODE_MODERN;
                g_config.playstyle.m_set = ConfigSetLevel::ep_config;

                if(g_config.speedrun_mode.m_set != ConfigSetLevel::cmdline)
                {
                    g_config.speedrun_mode.m_value = 0;
                    g_config.playstyle.m_set = ConfigSetLevel::ep_config;
                }
            }

            Integrator::clearEpisodeName();
            Integrator::clearLevelName();
            Integrator::clearEditorFile();
            FontManager::clearAllCustomFonts();

            BattleIntro = 0;
            BattleOutro = 0;
            // AllCharBlock = 0;
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

            // if(MenuMode != MENU_BATTLE_MODE)
            // {
            //     PlayerCharacter = 0;
            //     PlayerCharacter2 = 0;
            // }

            pLogDebug("Clear check-points at Game Menu start");
            Checkpoint.clear();
            CheckpointsList.clear();
            g_curLevelMedals.reset_checkpoint();
            WorldPlayer[1].Frame = 0;
            cheats_clearBuffer();
            LevelBeatCode = 0;
            curWorldLevel = 0;

            lunaReset();
            ResetSoundFX();
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
#ifdef __3DS__
            g_ForceBitmaskMerge = g_config.inaccurate_gifs;
#endif
            g_ClonedPlayerMode = false;
            g_CheatLogicScreen = false;
            g_CheatEditYourFriends = false;
            CanWallJump = false;
            g_VanillaCam = false;
            SharedPause = false;
            SharedPauseLegacy = false;
            XRender::unloadGifTextures();

            Controls::RemoveNullInputMethods();
            QuickReconnectScreen::Deactivate();

            UpdateInternalRes();

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

            numPlayers = g_gameInfo.introMaxPlayersCount;
#ifdef __16M__
            if(numPlayers > 3)
                numPlayers = 3;
#endif
            if(g_gameInfo.introDeadMode)
                numPlayers = 1;// one deadman should be

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
                            p.Location.Y = Block[B].Location.Y - p.Location.Height - 0.1_n;
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

            // Ensure everything is clear
            GraphicsClearScreen();
            XEvents::doEvents();
        }

        // World Map
        else if(LevelSelect)
        {
            // only set if loading the WORLD fails
            if(ErrorQuit)
            {
                ErrorQuit = false;
                GameMenu = true;
                MenuMode = MENU_INTRO;
                MenuCursor = 0;
                continue;
            }

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
                SoundPause[SFX_Slide] = 2000;

                LevelSelect = false;

                if(XMessage::GetStatus() != XMessage::Status::replay)
                {
                    XRender::setTargetTexture();
                    XRender::clearBuffer();
                    XRender::repaint();
                }

                lunaReset();
                ResetSoundFX();
                ClearLevel();

                std::string levelPath;
                if(GoToLevel.empty())
                {
                    if(FileRecentSubHubLevel.empty())
                        levelPath = FileNamePathWorld + StartLevel;
                    else
                        levelPath = FileNamePathWorld + FileRecentSubHubLevel;
                }
                else
                {
                    levelPath = FileNamePathWorld + GoToLevel;
                    GoToLevel.clear();
                }

                levelPath = s_prepare_episode_path(levelPath);

                if(!OpenLevel(levelPath))
                {
                    ReportLoadFailure(levelPath);
                    ErrorQuit = true;
                }

                if(!GoToLevelNoGameThing)
                {
                    GameThing(1000 - (g_ShortDelay * 250), 3);
                    g_ShortDelay = false;
                }
                else if(XMessage::GetStatus() != XMessage::Status::replay)
                {
                    XRender::setTargetTexture();
                    XRender::clearBuffer();
                    XRender::repaint();
                }

                GoToLevelNoGameThing = false;
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

                    if(p.Warp > maxWarps)
                        break;

                    p.WarpBackward = false;
                    auto &warp = Warp[p.Warp];

                    if(warp.Effect == 1)
                    {
                        if(warp.Direction2 == 1) // DOWN
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = warp.Exit.X + (warp.Exit.Width - p.Location.Width) / 2;
//                                .Location.Y = Warp(.Warp).Exit.Y - .Location.Height - 8
                            p.Location.Y = warp.Exit.Y - p.Location.Height - 8;
                        }
//                            ElseIf Warp(.Warp).Direction2 = 3 Then
                        if(warp.Direction2 == 3) // UP
                        {
//                                .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                            p.Location.X = warp.Exit.X + (warp.Exit.Width - p.Location.Width) / 2;
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
                        CheckSection_Init(A);
                        SoundPause[SFX_Warp] = 0;
                        p.Effect = PLREFF_WAITING;
                        p.Effect2 = 950;
                    }
                    else if(warp.Effect == 2)
                    {
//                            .Location.X = Warp(.Warp).Exit.X + Warp(.Warp).Exit.Width / 2 - .Location.Width / 2
                        p.Location.X = warp.Exit.X + (warp.Exit.Width - p.Location.Width) / 2;
//                            .Location.Y = Warp(.Warp).Exit.Y + Warp(.Warp).Exit.Height - .Location.Height
                        p.Location.Y = warp.Exit.Y + warp.Exit.Height - p.Location.Height;

                        CheckSection_Init(A);
                        p.Effect = PLREFF_WAITING;
                        p.Effect2 = 2000;
                    }
                    else if(warp.Effect == 3) // Portal warp
                    {
                        p.Location.X = warp.Exit.X + (warp.Exit.Width - p.Location.Width) / 2;
                        p.Location.Y = warp.Exit.Y + warp.Exit.Height - p.Location.Height;
                        CheckSection_Init(A);
                        p.WarpCD = 50;

                        if(warp.eventExit != EVENT_NONE)
                            TriggerEvent(warp.eventExit, A);
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
            bool hasCrashStartWarp = (Player[1].Warp > maxWarps);
            bool hasValidStartWarp = (Player[1].Warp > 0 && Player[1].Warp <= numWarps);
            bool startError = false;

            for(int i = 1; i <= numPlayers && i <= 2; ++i)
                hasPlayerPoint |= !PlayerStart[i].isNull();

            if(hasCrashStartWarp || (TestLevel && hasStartWarp && !hasValidStartWarp))
            {
                g_MessageType = MESSAGE_TYPE_SYS_ERROR;
                // this case would have crashed SMBX 1.3.
                MessageText = fmt::format_ne(g_gameStrings.errorInvalidEnterWarp,
                                             FullFileName,
                                             Player[1].Warp,
                                             numWarps);
                startError = true;
                Player[1].Warp = 0;
            }
            else if(!hasPlayerPoint && !hasValidStartWarp)
            {
                g_MessageType = MESSAGE_TYPE_SYS_ERROR;
                MessageText = fmt::format_ne(g_gameStrings.errorNoStartPoint, FullFileName);
                startError = true;
            }
            else if(hasStartWarp && !hasValidStartWarp)
            {
                pLogWarning("Level start: warp %d requested, but only %d warps present.", Player[1].Warp, numWarps);
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

                // intro events previously processed directly here
                g_gameLoopInterrupt.process_intro_events = true;

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

                // Ensure everything is clear
                GraphicsClearScreen();
                XEvents::doEvents();
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
                    GracefulQuit();
                    return 0;
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

        Archives::unmount_temp();
    }

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
    else if(LevelEditor)
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

void GracefulQuit(bool wait)
{
#ifdef __WIIU__
    if(!GameIsActive)
        return; // Don't call this twice
#endif

    StopMusic();

    if(wait)
    {
        const Uint64 waitDst = SDL_GetTicks64() + 500;

        do
        {
            XRender::setTargetTexture();
            XRender::clearBuffer();
            XRender::repaint();
            XEvents::doEvents();
            PGE_Delay(16);
        } while(SDL_GetTicks64() < waitDst);
    }

    StopAllSounds();

    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    XEvents::doEvents();

#ifdef __WIIU__
    if(GameIsActive && !g_isHBLauncher)
    {
        SYSLaunchMenu(); // Trigger the SDL_QUIT and the leading quit into Wii U main menu

        while(GameIsActive) // Wait until quit event will happen
        {
            XEvents::doEvents();
            PGE_Delay(10);
        }

        PGE_Delay(100);
    }
#endif

    KillIt();
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

    if(XMessage::GetStatus() != XMessage::Status::replay)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();
        XRender::repaint();
        XEvents::doEvents();
    }

    // do an inter-level delay here if there won't be a GameThing later
    if(!TestLevel && GoToLevel.empty() && !NoMap && FileRecentSubHubLevel.empty())
    {
        if(XMessage::GetStatus() != XMessage::Status::local)
        {
            for(int i = 0; i < ((g_ShortDelay) ? 16 : 32); i++)
                Controls::Update(false);
        }
        else if(!g_config.unlimited_framerate)
            PGE_Delay(500 - (g_ShortDelay * 250));

        g_ShortDelay = false;
    }

    if(BattleMode && !LevelEditor && !TestLevel)
    {
        g_ShortDelay = false;
        EndLevel = false;
        GameMenu = true;
        MenuMode = MENU_BATTLE_MODE;
        MenuCursor = selWorld - 1;
        // PlayerCharacter = Player[1].Character;
        // PlayerCharacter2 = Player[2].Character;
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
            IntProc::sendPlayerSettings2(i, p.Hearts, p.HeldBonus);
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

                if(p.State == PLR_STATE_AQUATIC && !p.Mount && !p.HoldingNPC)
                    c.Run = true;

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

            if(XMessage::GetStatus() != XMessage::Status::replay)
                XRender::clearBuffer();
        }
    }
    else if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
    {
        const int keyholeMax = 192; // Was 300

        // this was previously its own frameloop.
        // items done earlier in the frameloop have been commented out.

        speedRun_tick();
        // Controls::Update(false);
        UpdateGraphics();
        UpdateSound();
        BlockFrames();

        // XEvents::doEvents();
        // computeFrameTime2();

        updateScreenFaders();

        LevelMacroCounter++;

        if(g_config.EnableInterLevelFade && LevelMacroCounter == (keyholeMax - 65))
            g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

        if(LevelMacroCounter >= keyholeMax) /*300*/
        {
            LevelBeatCode = 4;
            EndLevel = true;
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroWhich = 0;
            LevelMacroCounter = 0;

            if(XMessage::GetStatus() != XMessage::Status::replay)
                XRender::clearBuffer();
        }
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

            if(XMessage::GetStatus() != XMessage::Status::replay)
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

            if(XMessage::GetStatus() != XMessage::Status::replay)
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
    else if(LevelMacro == LEVELMACRO_GOAL_TAPE_EXIT || LevelMacro == LEVELMACRO_FLAG_EXIT) // SMW Exit
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
                c.Up = (LevelMacro == LEVELMACRO_FLAG_EXIT);
                c.Start = false;
                c.AltJump = false;
                c.AltRun = false;

                if(p.State == PLR_STATE_AQUATIC && !p.Mount && !p.HoldingNPC)
                    c.Run = true;
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
            num_t focusX = (canTrack) ?
                            Player[1].Location.X + Player[1].Location.Width / 2 :
                            level[Player[1].Section].Width;
            num_t focusY = Player[1].Location.Y + Player[1].Location.Height / 2;

            g_levelScreenFader.setupFader(2, 0, 65, ScreenFader::S_CIRCLE, true, (int)focusX, (int)focusY, 1);

            if(canTrack)
                g_levelScreenFader.setTrackedFocus(&Player[1].Location.X,
                                                   &Player[1].Location.Y,
                                                   Player[1].Location.Width / 2,
                                                   Player[1].Location.Height / 2);
        }

        if(LevelMacroCounter >= 630)
        {
            // LEVELMACRO_GOAL_TAPE_EXIT = 7 -> 8
            // LEVELMACRO_FLAG_EXIT = 8 -> 9
            LevelBeatCode = (LevelMacro + 1);
            LevelMacro = LEVELMACRO_OFF;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
}

// main_config.cpp


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
        if(!focusLost)
        {
            XRender::setTargetTexture();
            XRender::renderRect(0, 0, XRender::TargetW, XRender::TargetH, {0, 0, 0, 127}, true);
            SuperPrintScreenCenter(g_gameStrings.screenPaused.empty() ? "Paused" : g_gameStrings.screenPaused, 3, XRender::TargetH / 2);
            pLogDebug("Window Focus lost");
            focusLost = true;
        }

        XRender::repaint();

        XEvents::waitEvents();
//        If LevelEditor = True Or MagicHand = True Then frmLevelWindow.vScreen(1).MousePointer = 0
        SyncSysCursorDisplay();

        resetFrameTimer();

        resetTimeBuffer();
        //keyDownEnter = false;
        //keyDownAlt = false;

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


Location_t newLoc(num_t X, num_t Y, num_t Width, num_t Height)
{
    Location_t ret;
    ret.X = X;
    ret.Y = Y;
    ret.Width = Width;
    ret.Height = Height;
    return ret;
}

// Location_t roundLoc(const Location_t &inLoc, double grid)
// {
//     Location_t ret = inLoc;
//     ret.X = Maths::roundTo(ret.X, grid);
//     ret.Y = Maths::roundTo(ret.Y, grid);
//     return ret;
// }

void MoreScore(int addScore, const Location_t &Loc)
{
    uint8_t mult = 0; // dummy
    MoreScore(addScore, Loc, mult);
}

void MoreScore(int addScore, const Location_t &Loc, uint8_t &Multiplier)
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
    g_ShortDelay = false;
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
    UnloadCustomSound();
    Archives::unmount_episode();

    std::string wPath = SelectWorld[selWorld].WorldFilePath;
    std::string recentWorldIntroPrev = g_recentWorldIntro;
    bool doSaveConfig = false;

    if((numPlayers == 1 || g_config.compatibility_mode != Config_t::COMPAT_SMBX13) && g_recentWorld1p != wPath)
    {
        g_recentWorld1p = wPath;
        doSaveConfig = true;
    }
    else if((numPlayers >= 2 && g_config.compatibility_mode == Config_t::COMPAT_SMBX13) && g_recentWorld2p != wPath)
    {
        g_recentWorld2p = wPath;
        doSaveConfig = true;
    }

    wPath = s_prepare_episode_path(wPath);

    if(!OpenWorld(wPath))
    {
        if(doSaveConfig)
            SaveConfig();
        ClearLevel();
        LevelSelect = false;
        ReportLoadFailure(wPath);
        LevelSelect = true;
        ErrorQuit = true;
        return;
    }

    if(recentWorldIntroPrev != g_recentWorldIntro)
        doSaveConfig = true;

    if(doSaveConfig)
        SaveConfig();

    if(selSave && SaveSlotInfo[selSave].Progress >= 0)
    {
        if(!NoMap)
            StartLevel.clear();
        LoadGame();
        speedRun_loadStats();
    }

    Integrator::setEpisodeName(WorldName);

#if 0 // unused cheat code from SMBX64
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
#endif

    SetupPlayers();

    if(!StartLevel.empty() || !FileRecentSubHubLevel.empty())
    {
        // TODO: why did Wohlstand disable this?
        PlaySoundMenu(SFX_LevelSelect);
        SoundPause[SFX_Slide] = 200;
        LevelSelect = false;

        ResetSoundFX();
        // todo: update this!
        ClearLevel();

        std::string levelName = (FileRecentSubHubLevel.empty() ? StartLevel : FileRecentSubHubLevel);
        std::string levelPath = FileNamePathWorld + levelName;

        levelPath = s_prepare_episode_path(levelPath);

        if(!OpenLevel(levelPath))
        {
            ReportLoadFailure(levelName);
            LevelSelect = true;

            // return to menu if the hub of a hub world is broken
            if(NoMap)
                ErrorQuit = true;
        }
        else
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
        g_MessageType = MESSAGE_TYPE_SYS_WARNING;
        MessageText = g_mainMenu.errorBattleNoLevels;
        PauseGame(PauseCode::Message);
        ErrorQuit = true;
    }
    else
    {
        if(selWorld == 1)
            selWorld = (iRand(NumSelectBattle - 1)) + 2;
    }

    const std::string& levelPath = SelectBattle[selWorld].WorldFilePath;
    if(!OpenLevel(levelPath))
    {
        ReportLoadFailure(levelPath);
        ErrorQuit = true;
    }
    SetupPlayers();

    BattleIntro = 150;
    BattleWinner = 0;
    BattleOutro = 0;
}
