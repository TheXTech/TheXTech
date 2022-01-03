/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL.h>

#include "game_main.h"
#include "frm_main.h"
#include "gfx.h"
#include "sound.h"
#include "video.h"
#include "main/presetup.h"
#include "main/game_info.h"
#include "main/speedrunner.h"
#include "compat.h"
#include <AppPath/app_path.h>
#include <tclap/CmdLine.h>
#include <Utils/strings.h>
#include <Utils/files.h>
#include <CrashHandler/crash_handler.h>

#ifdef ENABLE_XTECH_LUA
#include "xtech_lua_main.h"
#endif

#ifdef __APPLE__
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
        if(p.MountType < 1 || p.MountType > 3) // Socks
            p.MountType = 1;
        break;
    default:
        break;
    case 3:
        if(p.MountType < 1 || p.MountType > 8) // Cat Llamas
            p.MountType = 1;
        break;
    }
}

extern "C"
int main(int argc, char**argv)
{
    CmdLineSetup_t setup;

    CrashHandler::initSigs();

    AppPathManager::initAppPath();
    AppPath = AppPathManager::assetsRoot();

    OpenConfig_preSetup();

    testPlayer.fill(Player_t());

    try
    {
        // Define the command line object.
        TCLAP::CmdLine  cmd("TheXTech Engine\n"
                            "Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>\n\n"
                            "This program is distributed under the GPLv3 license\n\n", ' ', "1.3");

        TCLAP::ValueArg<std::string> customAssetsPath("c", "assets-root", "Specify the different assets root directory to play",
                                                      false, "",
                                                      "directory path",
                                                      cmd);


        TCLAP::SwitchArg switchFrameSkip("f", "frameskip", "Enable frame skipping mode", false);
        TCLAP::SwitchArg switchDisableFrameSkip(std::string(), "no-frameskip", "Disable frame skipping mode", false);
        TCLAP::SwitchArg switchNoSound("s", "no-sound", "Disable sound", false);
        TCLAP::SwitchArg switchNoPause("p", "never-pause", "Never pause game when window losts a focus", false);
        TCLAP::SwitchArg switchBgInput(std::string(), "bg-input", "Allow background input for joysticks", false);
        TCLAP::ValueArg<std::string> renderType("r", "render", "Sets the graphics mode:\n"
                                                "  sw - software render (fallback)\n"
                                                "  hw - hardware accelerated render [Default]\n"
                                                "  vsync - hardware accelerated with the v-sync enabled",
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
        TCLAP::SwitchArg switchTestInterprocess("i", "interprocessing", "Enable an interprocessing mode with Editor", false);

        TCLAP::ValueArg<std::string> compatLevel(std::string(), "compat-level",
                                                   "Enforce the specific gameplay compatibiltiy level. Supported values:\n"
                                                   "  modern - TheXTech native, all features and fixes enabled [Default]\n"
                                                   "  smbx2  - Disables all features and bugfixes except fixed at SMBX2\n"
                                                   "  smbx13 - Enforces the full compatibility with the SMBX 1.3 behaviour\n"
                                                   "\n"
                                                   "  Note: If speed-run mode is set, the compatibility level will be overriden by the speed-run mode",
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

        TCLAP::SwitchArg switchVerboseLog(std::string(), "verbose", "Enable log output into the terminal", false);

        TCLAP::UnlabeledMultiArg<std::string> inputFileNames("levelpath", "Path to level file or replay data to run the test", false, std::string(), "path to file");

        cmd.add(&switchFrameSkip);
        cmd.add(&switchDisableFrameSkip);
        cmd.add(&switchNoSound);
        cmd.add(&switchNoPause);
        cmd.add(&switchBgInput);
        cmd.add(&switchBattleMode);

        cmd.add(&switchTestGodMode);
        cmd.add(&switchTestGrabAll);
        cmd.add(&switchTestShowFPS);
        cmd.add(&switchTestMaxFPS);
        cmd.add(&switchTestMagicHand);
        cmd.add(&switchTestInterprocess);
        cmd.add(&switchVerboseLog);
        cmd.add(&switchSpeedRunSemiTransparent);
        cmd.add(&switchDisplayControls);
        cmd.add(&inputFileNames);

        cmd.parse(argc, argv);

        std::string customAssets = customAssetsPath.getValue();

        if(!customAssets.empty())
        {
            AppPathManager::setAssetsRoot(customAssets);
            AppPath = AppPathManager::assetsRoot();
        }

        if(switchDisableFrameSkip.isSet())
            setup.frameSkip = !switchDisableFrameSkip.getValue();
        else if(switchFrameSkip.isSet())
            setup.frameSkip = switchFrameSkip.getValue();
        else
            setup.frameSkip = g_videoSettings.enableFrameSkip;

        setup.noSound   = switchNoSound.isSet() ? switchNoSound.getValue() : g_audioSetup.disableSound;
        setup.neverPause = switchNoPause.isSet() ? switchNoPause.getValue() : g_videoSettings.allowBgWork;
        setup.allowBgInput = switchBgInput.isSet() ? switchBgInput.getValue() : g_videoSettings.allowBgControllerInput;

        if(setup.allowBgInput) // The BG-input depends on the never-pause option
            setup.neverPause = setup.allowBgInput;

        if(renderType.isSet())
        {
            std::string rt = renderType.getValue();
            if(rt == "sw")
                setup.renderType = RENDER_SOFTWARE;
            else if(rt == "vsync")
                setup.renderType = RENDER_ACCELERATED_VSYNC;
            else if(rt == "hw")
                setup.renderType = RENDER_ACCELERATED;
            else
            {
                std::cerr << "Error: Invalid value for the --render argument: " << rt << std::endl;
                std::cerr.flush();
                return 2;
            }
#ifdef DEBUG_BUILD
            std::cerr << "Manually selected renderer:" << rt << " - " << setup.renderType << std::endl;
            std::cerr.flush();
#endif
        }
        else
        {
            setup.renderType = g_videoSettings.renderMode;
        }

        setup.testLevel = testLevel.getValue();

        if(inputFileNames.isSet())
        {
            for(const auto &fpath : inputFileNames.getValue())
            {
                if(Files::hasSuffix(fpath, ".lvl") || Files::hasSuffix(fpath, ".lvlx"))
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
        setup.interprocess = switchTestInterprocess.getValue();
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
        setup.testShowFPS = switchTestShowFPS.isSet() ?
                                switchTestShowFPS.getValue() :
                                g_videoSettings.showFrameRate;
        setup.testMaxFPS = switchTestMaxFPS.getValue();
        setup.testMagicHand = switchTestMagicHand.getValue();

        if(compatLevel.isSet())
        {
            std::string compatModeVal = compatLevel.getValue();
            if(compatModeVal == "smbx2")
                setup.compatibilityLevel = COMPAT_SMBX2;
            else if(compatModeVal == "smbx13")
                setup.compatibilityLevel = COMPAT_SMBX13;
            else if(compatModeVal == "modern")
                setup.compatibilityLevel = COMPAT_MODERN;
            else
            {
                std::cerr << "Error: Invalid value for the --compat-level argument: " << compatModeVal << std::endl;
                std::cerr.flush();
                return 2;
            }
        }
        else
        {
            setup.compatibilityLevel = g_preSetup.compatibilityMode;
        }

        setup.speedRunnerMode = speedRunMode.isSet() ?
                                    speedRunMode.getValue() :
                                    g_preSetup.speedRunMode;
        setup.speedRunnerSemiTransparent = switchSpeedRunSemiTransparent.isSet() ?
                                            switchSpeedRunSemiTransparent.getValue() :
                                            g_preSetup.speedRunSemiTransparentTimer;
        setup.showControllerState = switchDisplayControls.isSet() ?
                                        switchDisplayControls.getValue() :
                                        g_drawController;

        if(showBatteryStatus.isSet() && IF_INRANGE(showBatteryStatus.getValue(), 1, 4))
            g_videoSettings.batteryStatus = showBatteryStatus.getValue();

        if(setup.speedRunnerMode >= 1) // Always show FPS and don't pause the game work when focusing other windows
        {
            setup.testShowFPS = true;
            setup.neverPause = true;
        }
    }
    catch(TCLAP::ArgException &e)   // catch any exceptions
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cerr.flush();
        return 2;
    }

    initGameInfo();

    // set this flag before SDL initialization to allow game be quit when closing a window before a loading process will be completed
    GameIsActive = true;

    if(frmMain.initSystem(setup))
    {
        frmMain.freeSystem();
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
        setup.testShowFPS = false;
        setup.testMaxFPS = false;
    }
#endif

#ifdef ENABLE_XTECH_LUA
    if(!xtech_lua_init())
        return 1;
#endif

    int ret = GameMain(setup);

#ifdef ENABLE_XTECH_LUA
    if(!xtech_lua_quit())
        return 1;
#endif

    frmMain.freeSystem();

    return ret;
}
