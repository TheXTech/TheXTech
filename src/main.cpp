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

#include <ctime>
#include "sdl_proxy/sdl_head.h"

#include "../version.h"

#include "game_main.h"
#include "frm_main.h"
#include "gfx.h"
#include "rand.h"
#include "sound.h"
#include "main/game_info.h"
#include "main/speedrunner.h"
#include "main/game_info.h"
#include "main/asset_pack.h"
#include "main/translate.h"
#include "core/language.h"
#include "config.h"
#include "controls.h"
#include <AppPath/app_path.h>

#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include "capabilities.h"
#endif

#ifndef THEXTECH_NO_ARGV_HANDLING
#   include <tclap/CmdLine.h>
#endif

#include <Utils/strings.h>
#include <Utils/files.h>

#ifdef THEXTECH_CRASHHANDLER_SUPPORTED
#   include <CrashHandler/crash_handler.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <SDL2/SDL_main.h>
#endif

#ifdef VITA
#include "core/vita/vita_memory.h"
#endif

#ifdef __16M__
#include <nds.h>
#endif

#ifdef __WIIU__
#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <coreinit/filesystem.h>
#include <sysapp/launch.h>
#endif

#ifdef __WII__
#include <gccore.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef ENABLE_XTECH_LUA
#include "xtech_lua_main.h"
#endif

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <Utils/files.h>
#include <Logger/logger.h>

static std::string g_fileToOpen;
/**
 * @brief Receive an opened file from the Finder (Must be created at least one window!)
 */
static void macosReceiveOpenFile()
{
    if(g_fileToOpen.empty())
    {
        pLogDebug("Attempt to take Finder args...");
        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_DROPFILE)
            {
                std::string file(event.drop.file);
                if(Files::fileExists(file))
                {
                    g_fileToOpen = file;
                    pLogDebug("Got file path: [%s]", file.c_str());
                }
                else
                    pLogWarning("Invalid file path, sent by Mac OS X Finder event: [%s]", file.c_str());
            }
        }
        SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    }
}
#endif

#ifdef __3DS__
#include <3ds.h>
int n3ds_clocked = 0; // eventually move elsewhere

void InitClockSpeed()
{
    bool isN3DS;

    APT_CheckNew3DS(&isN3DS);

    if(!isN3DS)
        n3ds_clocked = -1;

    // I've made this configurable.
    // if(isN3DS) // make this configurable...
    // {
    //     SwapClockSpeed();
    //     printf("N3DS clock enabled\n");
    // }
}

void SwapClockSpeed()
{
    if(n3ds_clocked == -1)
        return;

    n3ds_clocked = !n3ds_clocked;
    osSetSpeedupEnable(n3ds_clocked);
}

#endif

#ifndef THEXTECH_NO_ARGV_HANDLING
static void strToPlayerSetup(int player, const std::string &setupString)
{
    if(setupString.empty())
        return; // Do nothing

    std::vector<std::string> keys;

    auto &p = testPlayer[player];

    Strings::split(keys, setupString, ";");
    for(auto &k : keys)
    {
        if(k.empty())
            continue;
        if(k[0] == 'c') // Character
            p.Character = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
        else if(k[0] == 's') // State
            p.State = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
        else if(k[0] == 'm') // Mounts
            p.Mount = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
        else if(k[0] == 't') // Mount types
            p.MountType = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
    }

    if(p.Character < 1)
        p.Character = 1;
    if(p.Character > 5)
        p.Character = 5;

    if(p.State < 1)
        p.State = 1;
    if(p.State > 7)
        p.State = 7;


    switch(p.Mount)
    {
    default:
    case 0:
        p.Mount = 0;
        p.MountType = 0;
        break; // Rejected aliens
    case 1: case 2: case 3:
        break; // Allowed
    }

    switch(p.Mount)
    {
    case 1:
        if((p.MountType < 1) || (p.MountType > 3)) // Socks
            p.MountType = 1;
        break;
    default:
        break;
    case 3:
        if((p.MountType < 1) || (p.MountType > 8)) // Cat Llamas
            p.MountType = 1;
        break;
    }
}
#endif

extern "C"
int main(int argc, char**argv)
{
#ifdef __16M__
    // install the default exception handler
    defaultExceptionHandler();

    videoSetModeSub(MODE_0_2D);
    vramSetBankH(VRAM_H_SUB_BG);
    vramSetBankI(VRAM_I_SUB_BG_0x06208000);

    PrintConsole* defaultConsole = consoleGetDefault();

    consoleInit(nullptr, defaultConsole->bgLayer, BgType_Text4bpp, BgSize_T_256x256, defaultConsole->mapBase, defaultConsole->gfxBase, false, true);

    printf("Hello, 16MB world!\n");
#endif

#ifdef __WIIU__
    WHBProcInit();
#endif

#ifdef __WII__
    VIDEO_Init();
    VIDEO_SetBlack(TRUE);
#endif

    CmdLineSetup_t setup;

#if defined(__APPLE__) && defined(USE_APPLE_X11)
    char *x11_display_env = getenv("DISPLAY");
    if(!x11_display_env || x11_display_env[0] == '\0')
        setenv("DISPLAY", ":0", 1); // Automatically set the display to :0 if not defined
#endif

#ifdef THEXTECH_CRASHHANDLER_SUPPORTED
    CrashHandler::initSigs();
#endif

#ifdef __3DS__
    InitClockSpeed();
    SwapClockSpeed();
#endif

    seedRandom(std::time(NULL));

    testPlayer.fill(Player_t());
    for(int i = 1; i <= maxLocalPlayers; i++)
    {
        testPlayer[i].Character = i;
    }

#ifndef THEXTECH_NO_ARGV_HANDLING
    try
    {
        // Define the command line object.
        TCLAP::CmdLine  cmd("TheXTech Engine\n"
                            "Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>\n\n"
                            "This program is distributed under the GPLv3 license\n\n", ' ', V_LATEST_STABLE " [" V_BUILD_BRANCH ", #" V_BUILD_VER "]");

        TCLAP::ValueArg<std::string> customAssetsPath("c", "asset-pack", "Specify the different assets pack name or directory to play",
                                                      false, "",
                                                      "string or directory path",
                                                      cmd);

        TCLAP::ValueArg<std::string> customUserDirectory("u", "user-directory", "Specify the different writable user directory to store settings, gamesaves, logs, screenshots, etc.",
                                                         false, "",
                                                         "directory path",
                                                         cmd);

        TCLAP::ValueArg<std::string> customGameDirName(std::string(), "game-dirname",
                                                       "Specify the game directory name for default locations (ignored if user-directory is specified)",
                                                       false, "",
                                                       "directory name",
                                                       cmd);


        TCLAP::SwitchArg switchFrameSkip("f", "frameskip", "Enable frame skipping mode", false);
        TCLAP::SwitchArg switchDisableFrameSkip(std::string(), "no-frameskip", "Disable frame skipping mode", false);
        TCLAP::SwitchArg switchNoSound("s", "no-sound", "Disable sound", false);
        TCLAP::SwitchArg switchNoPause("p", "never-pause", "Never pause game when window losts a focus", false);
        TCLAP::SwitchArg switchBgInput(std::string(), "bg-input", "Allow background input for joysticks", false);
        TCLAP::SwitchArg switchVSync(std::string(), "vsync", "Limit the framerate to the screen refresh rate", false);
        TCLAP::ValueArg<std::string> renderType("r", "render", "Sets the graphics mode:\n"
                                                "  sw - software SDL2 render (fallback)\n"
                                                "  hw - generic hardware accelerated render (currently SDL2) [Default]\n"
                                                "  vsync - generic hardware accelerated render with vSync [deprecated]\n"
                                                "  sdl - hardware accelerated SDL2 render\n"
#   ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
                                                "  opengl - hardware accelerated OpenGL 2.1+ render\n"
#   endif
#   ifdef THEXTECH_BUILD_GL_ES_MODERN
                                                "  opengles - hardware accelerated OpenGL ES (mobile) 2.0+ render\n"
#   endif
#   ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
                                                "  opengl11 - hardware accelerated OpenGL 1.1-2.0 render (legacy)\n"
#   endif
#   ifdef THEXTECH_BUILD_GL_ES_LEGACY
                                                "  opengles11 - hardware accelerated OpenGL ES 1.1 render (legacy)"
#   endif
                                                ,
                                                false, "",
                                                "render type",
                                                cmd);

        TCLAP::ValueArg<std::string> testLevel("l", "leveltest", "Start a level test of a given level file.\n"
                                                "[OBSOLETE OPTION]: now you able to specify the file path without -l or --leveltest argument.",
                                                false, "",
                                                "file path",
                                                cmd);

        TCLAP::ValueArg<unsigned int> numPlayers("n", "num-players", "Count of players",
                                                    false, 1u,
                                                   "number 1 or 2",
                                                   cmd);

        TCLAP::ValueArg<unsigned int> startWarp("w", "start-warp", "Warp index to start level test at",
                                                    false, 0u,
                                                   "start warp index",
                                                   cmd);

        TCLAP::SwitchArg switchBattleMode("b", "battle", "Test level in battle mode", false);

        TCLAP::ValueArg<std::string> playerCharacter1("1",
                                                      "player1",
                                                      "Setup of playable character for player 1:\n"
                                                      "  Semicolon separated key-argument values:\n"
                                                      "  c - character, s - state, m - mount, t - mount type.\n\n"
                                                      "Example:\n"
                                                      "  c1;s2;m3;t0 - Character as 1, State as 2, Mount as 3, M.Type as 0",
                                                      false, "",
                                                      "c1;s2;m0;t0",
                                                      cmd);

        TCLAP::ValueArg<std::string> playerCharacter2("2",
                                                      "player2",
                                                      "Setup of playable character for player 2:\n"
                                                      "  Semicolon separated key-argument values:\n"
                                                      "  c - character, s - state, m - mount, t - mount type.\n\n"
                                                      "Example:\n"
                                                      "  c1;s2;m3;t0 - Character as 1, State as 2, Mount as 3, M.Type as 0",
                                                      false, "",
                                                      "c1;s2;m0;t0",
                                                      cmd);

        TCLAP::SwitchArg switchTestGodMode("g", "god-mode", "Enable god mode in level testing", false);
        TCLAP::SwitchArg switchTestGrabAll("a", "grab-all", "Enable ability to grab everything while level testing", false);
        TCLAP::SwitchArg switchTestShowFPS("m", "show-fps", "Show FPS counter on the screen", false);
        TCLAP::SwitchArg switchTestMaxFPS("x", "max-fps", "Run FPS as fast as possible", false);
        TCLAP::SwitchArg switchTestMagicHand("k", "magic-hand", "Enable magic hand functionality while level test running", false);
        TCLAP::SwitchArg switchTestEditor("e", "editor", "Open level in the editor", false);
#ifdef THEXTECH_INTERPROC_SUPPORTED
        TCLAP::SwitchArg switchTestInterprocess("i", "interprocessing", "Enable an interprocessing mode with Editor", false);
        TCLAP::SwitchArg switchPrintCapabilities(std::string(), "capabilities", "Print the JSON string of this build's capabilities", false);
#endif

        TCLAP::ValueArg<std::string> compatLevel(std::string(), "compat-level",
                                                   "Enforce the specific gameplay compatibility level. Supported values:\n"
                                                   "  modern - TheXTech native, all features and fixes enabled [Default]\n"
                                                   "  smbx2  - Disables all features and bugfixes except fixed at SMBX2\n"
                                                   "  smbx13 - Enforces the full compatibility with the SMBX 1.3 behaviour\n"
                                                   "\n"
                                                   "  Deprecated: acts as an alias for speed-run mode. Will be overridden if speed-run mode is set.",
                                                    false, "modern",
                                                   "modern, smbx2, smbx3",
                                                   cmd);
        TCLAP::ValueArg<unsigned int> speedRunMode(std::string(), "speed-run-mode",
                                                   "Enable the speed-runer mode: the playthrough timer will be shown, "
                                                   "and some gameplay limitations will be enabled. Supported values:\n"
                                                   "  0 - Disabled [Default]\n"
                                                   "  1 - TheXTech native\n"
                                                   "  2 - Disable time-winning updates (SMBX2 mode)\n"
                                                   "  3 - Strict vanilla SMBX 1.3, enable all bugs",
                                                    false, 0u,
                                                   "0, 1, 2, or 3",
                                                   cmd);
        TCLAP::SwitchArg switchSpeedRunSemiTransparent(std::string(), "speed-run-semitransparent",
                                                       "Make the speed-runner mode timer be drawn transparently", false);
        TCLAP::ValueArg<std::string> speedRunBlinkMode(std::string(), "speed-run-blink-mode",
                                                   "Choose the speed-run timer blinking effect for a level/episode completion\n"
                                                   "Supported values:\n"
                                                   "  opaque - Blink effect works when semi-transparent mode is not enabled\n"
                                                   "  always - Blink effect will work always\n"
                                                   "  never - Disable blink effect completely",
                                                    false, "undefined",
                                                   "opaque, always, never",
                                                    cmd);
        TCLAP::SwitchArg switchDisplayControls(std::string(), "show-controls", "Display current controller state while the game process", false);
        TCLAP::ValueArg<unsigned int> showBatteryStatus(std::string(), "show-battery-status",
                                                   "Display the battery status indicator (if available):\n"
                                                   "  0 - Never show [Default]\n"
                                                   "  1 - Show on fullscreen only when battery low\n"
                                                   "  2 - Show when battery low\n"
                                                   "  3 - Show always on fullscreen only\n"
                                                   "  4 - Always show",
                                                    false, 0u,
                                                   "0, 1, 2, 3, or 4",
                                                   cmd);
        TCLAP::ValueArg<int> saveSlot(std::string(), "save-slot", "Save slot to use for world play", false, 0, std::string("number from 1 to ") + std::to_string(maxSaveSlots), cmd);

#ifndef THEXTECH_DISABLE_LANG_TOOLS
        TCLAP::SwitchArg switchMakeLangTemplate(std::string(), "export-lang", "Exports the default language template", false);
        TCLAP::SwitchArg switchLangUpdate(std::string(), "lang-update", "Updated all language of assets package: missing lines will be added", false);
        TCLAP::SwitchArg switchLangNoBlank(std::string(), "lang-no-blank", "Don't put blank lines into translation files", false);
        TCLAP::ValueArg<std::string> langOutputPath(std::string(), "lang-output",
                                                    "Path to the languages directory that needs to be updated (by default, the in-assets directory is used)",
                                                    false, std::string(),
                                                    "path to directory");
#endif
        TCLAP::ValueArg<std::string> lang(std::string(), "lang", "Set the engine's language by code", false, "", "en, ru, zh-cn, etc.");

        TCLAP::SwitchArg switchVerboseLog(std::string(), "verbose", "Enable log output into the terminal", false);

        TCLAP::UnlabeledMultiArg<std::string> inputFileNames("levelpath", "Path to level file or replay data to run the test", false, std::string(), "path to file");

        cmd.add(&switchFrameSkip);
        cmd.add(&switchDisableFrameSkip);
        cmd.add(&switchNoSound);
        cmd.add(&switchNoPause);
        cmd.add(&switchBgInput);
        cmd.add(&switchVSync);
        cmd.add(&switchBattleMode);

        cmd.add(&switchTestGodMode);
        cmd.add(&switchTestGrabAll);
        cmd.add(&switchTestShowFPS);
        cmd.add(&switchTestMaxFPS);
        cmd.add(&switchTestMagicHand);
        cmd.add(&switchTestEditor);
#ifdef THEXTECH_INTERPROC_SUPPORTED
        cmd.add(&switchTestInterprocess);
        cmd.add(&switchPrintCapabilities);
#endif
        cmd.add(&switchVerboseLog);
        cmd.add(&switchSpeedRunSemiTransparent);
        cmd.add(&switchDisplayControls);
#ifndef THEXTECH_DISABLE_LANG_TOOLS
        cmd.add(&switchMakeLangTemplate);
        cmd.add(&switchLangUpdate);
        cmd.add(&switchLangNoBlank);
        cmd.add(&langOutputPath);
#endif
        cmd.add(&lang);
        cmd.add(&inputFileNames);

        cmd.parse(argc, argv);

        // Initialize the assets and user paths
        {
            setup.assetPack = customAssetsPath.getValue();
            std::string customUserDir = customUserDirectory.getValue();
            std::string customGameDir = customGameDirName.getValue();

            if(!setup.assetPack.empty())
                AppPathManager::addAssetsRoot(setup.assetPack);

            if(!customUserDir.empty())
                AppPathManager::setUserDirectory(customUserDir);

            if(!customGameDir.empty())
                AppPathManager::setGameDirName(customGameDir);

            AppPathManager::initAppPath();
        }

#ifdef THEXTECH_INTERPROC_SUPPORTED
        if(switchPrintCapabilities.isSet() && switchPrintCapabilities.getValue())
        {
            std::fprintf(stdout, "%s\n", g_capabilities);
            std::fflush(stdout);
            return 0;
        }
#endif

        OpenConfig();


#ifndef THEXTECH_DISABLE_LANG_TOOLS
        // Print the language template to the screen
        if(switchMakeLangTemplate.isSet() && switchMakeLangTemplate.getValue())
        {
            // Locate asset pack. The `true` here indicates to skip actually loading UI GFX.
            if(!InitUIAssetsFrom(setup.assetPack, true))
                return 1;

            printf("== Language template for [%s] ==\n", AppPath.c_str());

            initGameInfo();

            XTechTranslate translate;
            translate.reset();
            translate.exportTemplate();
            return 0;
        }

        // Update all translation files at current assets pack
        if(switchLangUpdate.isSet() && switchLangUpdate.getValue())
        {
            // Locate asset pack. The `true` here indicates to skip actually loading UI GFX.
            if(!InitUIAssetsFrom(setup.assetPack, true))
                return 1;

            printf("== Updating language files at [%s] ==\n", AppPath.c_str());

            initGameInfo();

            XTechTranslate translate;
            translate.reset();
            translate.updateLanguages(langOutputPath.getValue(), switchLangNoBlank.isSet());
            return 0;
        }
#endif

        // Store all of the command-line config options
        ConfigChangeSentinel sent(ConfigSetLevel::cmdline);

        if(compatLevel.isSet() && !speedRunMode.isSet())
        {
            std::string compatModeVal = compatLevel.getValue();
            if(compatModeVal == "classic" || compatModeVal == "smbx2")
                g_config.speedrun_mode = 2;
            else if(compatModeVal == "smbx13")
                g_config.speedrun_mode = 3;
            else if(compatModeVal == "modern")
                g_config.speedrun_mode = 1;
            else
            {
                std::cerr << "Error: Invalid value for the --compat-level argument: " << compatModeVal << std::endl;
                std::cerr.flush();
                return 2;
            }
        }

        if(speedRunMode.isSet())
            g_config.speedrun_mode = speedRunMode.getValue();

        if(speedRunBlinkMode.isSet())
        {
            bool is_transparent = false;
            if(switchSpeedRunSemiTransparent.isSet())
                is_transparent = switchSpeedRunSemiTransparent.getValue();

            std::string mode = speedRunBlinkMode.getValue();
            if(mode == "always")
                g_config.show_playtime_counter = Config_t::PLAYTIME_COUNTER_ANIMATED;
            else if(is_transparent)
                g_config.show_playtime_counter = Config_t::PLAYTIME_COUNTER_SUBTLE;
            else if(mode == "opaque")
                g_config.show_playtime_counter = Config_t::PLAYTIME_COUNTER_ANIMATED;
            else if(mode == "never")
                g_config.show_playtime_counter = Config_t::PLAYTIME_COUNTER_OPAQUE;
            else
            {
                std::cerr << "Error: Invalid value for the --speed-run-blink argument: " << mode << std::endl;
                std::cerr.flush();
                return 2;
            }
        }
        else if(switchSpeedRunSemiTransparent.isSet())
        {
            if(switchSpeedRunSemiTransparent.getValue())
                g_config.show_playtime_counter = Config_t::PLAYTIME_COUNTER_SUBTLE;
            else
                g_config.show_playtime_counter = Config_t::PLAYTIME_COUNTER_ANIMATED;
        }

        if(switchTestShowFPS.isSet())
            g_config.show_fps = switchTestShowFPS.getValue();

        if(switchTestMaxFPS.isSet())
            g_config.unlimited_framerate = switchTestMaxFPS.getValue();

        if(switchDisplayControls.isSet())
            g_config.show_controllers = switchDisplayControls.getValue();

        if(showBatteryStatus.isSet() && IF_INRANGE(showBatteryStatus.getValue(), 1, 4))
            g_config.show_battery_status = showBatteryStatus.getValue();

        if(switchDisableFrameSkip.isSet())
            g_config.enable_frameskip = false;
        else if(switchFrameSkip.isSet())
            g_config.enable_frameskip = true;

        if(switchNoSound.isSet())
            g_config.audio_enable    = !switchNoSound.getValue();

        if(switchVSync.isSet())
            g_config.render_vsync    = switchVSync.getValue();

#ifndef NO_WINDOW_FOCUS_TRACKING
        if(switchNoPause.isSet())
            g_config.background_work = switchNoPause.getValue();

        if(switchBgInput.isSet())
            g_config.background_work = switchBgInput.getValue();
#endif // NO_WINDOW_FOCUS_TRACKING

#ifndef RENDER_CUSTOM
        if(renderType.isSet())
        {
            std::string rt = renderType.getValue();
            if(rt == "sw")
                g_config.render_mode = Config_t::RENDER_SOFTWARE;
            else if(rt == "vsync")
            {
                g_config.render_mode = Config_t::RENDER_ACCELERATED_AUTO;
                g_config.render_vsync = true;
            }
            else if(rt == "hw")
                g_config.render_mode = Config_t::RENDER_ACCELERATED_AUTO;
            else if(rt == "sdl")
                g_config.render_mode = Config_t::RENDER_ACCELERATED_SDL;
            else if(rt == "opengl")
                g_config.render_mode = Config_t::RENDER_ACCELERATED_OPENGL;
            else if(rt == "opengl11")
                g_config.render_mode = Config_t::RENDER_ACCELERATED_OPENGL_LEGACY;
            else if(rt == "opengles")
                g_config.render_mode = Config_t::RENDER_ACCELERATED_OPENGL_ES;
            else if(rt == "opengles11")
                g_config.render_mode = Config_t::RENDER_ACCELERATED_OPENGL_ES_LEGACY;
            else
            {
                std::cerr << "Error: Invalid value for the --render argument: " << rt << std::endl;
                std::cerr.flush();
                return 2;
            }
#   ifdef DEBUG_BUILD
            std::cerr << "Manually selected renderer: " << rt << " - " << g_config.render_mode << std::endl;
            std::cerr.flush();
#   endif
        }
#endif

        // store the game setup options

        setup.testLevel = testLevel.getValue();

        if(inputFileNames.isSet())
        {
            for(const auto &fpath : inputFileNames.getValue())
            {
                if(Files::hasSuffix(fpath, ".lvl") || Files::hasSuffix(fpath, ".lvlx"))
                    setup.testLevel = fpath;
                else if(Files::hasSuffix(fpath, ".wld") || Files::hasSuffix(fpath, ".wldx"))
                    setup.testLevel = fpath;
                else if(Files::hasSuffix(fpath, ".rec"))
                    setup.testReplay = fpath;

//                //TODO: Implement a world map running and testing
//                else if(Files::hasSuffix(fpath, ".wld") || Files::hasSuffix(fpath, ".wldx"))
//                {

//                }
            }
        }

        setup.verboseLogging = switchVerboseLog.getValue();
#ifdef THEXTECH_INTERPROC_SUPPORTED
        setup.interprocess = switchTestInterprocess.getValue();
#endif
        setup.testLevelMode = !setup.testLevel.empty() || setup.interprocess;
        setup.testNumPlayers = int(numPlayers.getValue());
        if(setup.testNumPlayers > 2)
            setup.testNumPlayers = 2;
        setup.testBattleMode = switchBattleMode.getValue();
        if(setup.testLevelMode)
        {
            strToPlayerSetup(1, playerCharacter1.getValue());
            strToPlayerSetup(2, playerCharacter2.getValue());
        }

        setup.testGodMode = switchTestGodMode.getValue();
        setup.testGrabAll = switchTestGrabAll.getValue();
        setup.testMagicHand = switchTestMagicHand.getValue();
        setup.testEditor = switchTestEditor.getValue();
        setup.testSave = saveSlot.getValue();

        if(startWarp.isSet() && startWarp.getValue() > 0 && startWarp.getValue() < maxWarps)
            testStartWarp = startWarp.getValue();

        if(lang.isSet())
            g_config.language = lang;
    }
    catch(TCLAP::ArgException &e)   // catch any exceptions
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cerr.flush();
        return 2;
    }
#else
    UNUSED(argc);
    UNUSED(argv);

    printf("Launching AppPath...\n");
    AppPathManager::initAppPath();

    OpenConfig();

    setup.verboseLogging = true;
#endif

#if defined(__EMSCRIPTEN__) && defined(THEXTECH_DEBUG_INFO)
    setup.verboseLogging = true;
#endif

#ifdef __16M__
    // setup.testMaxFPS = true;
    setCpuClock(true);
#endif

    UpdateConfig();

    // set this flag before SDL initialization to allow game be quit when closing a window before a loading process will be completed
    GameIsActive = true;

    if(g_frmMain.initSystem(setup))
    {
        g_frmMain.freeSystem();
        return 1;
    }

#ifdef __APPLE__
    macosReceiveOpenFile();
    if(!g_fileToOpen.empty())
    {
        setup.testLevel = g_fileToOpen;
        setup.testLevelMode = !setup.testLevel.empty();
        setup.testNumPlayers = 1;
        setup.testGodMode = false;
        setup.testGrabAll = false;
    }
#endif

#ifdef ENABLE_XTECH_LUA
    if(!xtech_lua_init())
        return 1;
#endif

    Controls::Init();
    Controls::LoadConfig();

    int ret = GameMain(setup);

#ifdef ENABLE_XTECH_LUA
    if(!xtech_lua_quit())
        ret = 1;
#endif

    Controls::Quit();

    g_frmMain.freeSystem();

#ifdef __WIIU__
    WHBProcShutdown();
    SYSLaunchMenu();
#endif

#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
    EM_ASM(
        setTimeout(() => {
            console.log("Attempting to close window following game exit...");
            document.getElementById("canvas").style.display = 'none';
            document.getElementById("exit-msg").style.display = null;
            window.close();
        }, 250);
    );
#endif

    return ret;
}
