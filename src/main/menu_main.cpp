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

#include "sdl_proxy/sdl_timer.h"
#include "sdl_proxy/sdl_atomic.h"

#ifndef PGE_NO_THREADING
#include <SDL2/SDL_thread.h>
#endif

#ifdef THEXTECH_ENABLE_SDL_NET
#include <md5/md5tools.hpp>
#include "main/client_methods.h"
#endif

#include <fmt_format_ne.h>
#include <array>
#include <sorting/tinysort.h>

#include <AppPath/app_path.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Utils/files_ini.h>
#include <Archives/archives.h>
#include <PGE_File_Formats/file_formats.h>
#include <Integrator/integrator.h>

#include "menu_main.h"
#include "game_info.h"
#include "../version.h"
#include "../gfx.h"
#include "main/screen_connect.h"
#include "main/screen_options.h"
#include "main/menu_controls.h"
#include "main/translate_episode.h"

#include "speedrunner.h"
#include "main/gameplay_timer.h"
#include "../game_main.h"
#include "../sound.h"
#include "../player.h"
#include "../collision.h"
#include "../graphics.h"
#include "../core/render.h"
#include "../core/window.h"
#include "../core/events.h"
#include "../controls.h"
#include "../config.h"
#include "level_file.h"
#include "world_file.h"
#include "pge_delay.h"
#include "change_res.h"
#include "game_globals.h"
#include "core/language.h"
#include "main/translate.h"
#include "change_res.h"
#include "game_globals.h"
#include "npc_id.h"
#include "fontman/font_manager.h"
#include "custom.h"

#include "editor.h"
#include "frm_main.h"
#include "load_gfx.h"

#include "screen_textentry.h"
#include "main/asset_pack.h"
#include "main/screen_asset_pack.h"
#include "main/screen_content.h"
#include "editor/new_editor.h"
#include "editor/write_level.h"
#include "editor/write_world.h"
#include "editor/editor_custom.h"
#include "script/luna/luna.h"

MainMenuContent g_mainMenu;

#ifndef PGE_NO_THREADING
static bool                 s_atomicsInited = false;
static SDL_atomic_t         loading = {};
static SDL_atomic_t         loadingProgrss = {};
static SDL_atomic_t         loadingProgrssMax = {};

static SDL_Thread*          loadingThread = nullptr;
#endif

#ifdef THEXTECH_ENABLE_SDL_NET
static XMessage::RoomInfo   s_room_info;
#endif

static constexpr int c_menuSavesLength = maxSaveSlots + 2;
static constexpr int c_menuSavesFooterHint = (c_menuSavesLength * 30) - 30;
static constexpr int c_menuItemSavesEndList = maxSaveSlots - 1;
static constexpr int c_menuItemSavesCopy = maxSaveSlots;
static constexpr int c_menuItemSavesDelete = maxSaveSlots + 1;
static constexpr int c_menuSavesOffsetY = (maxSaveSlots - 3) * 30;

static uint8_t s_episode_playstyle = 0;
static uint8_t s_episode_speedrun_mode = 0;

int NumSelectWorld = 0;
int NumSelectBattle = 0;
std::vector<SelectWorld_t> SelectWorld;
std::vector<SelectWorld_t> SelectBattle;


void initMainMenu()
{
#ifndef PGE_NO_THREADING
    if(!s_atomicsInited)
    {
        SDL_AtomicSet(&loading, 0);
        SDL_AtomicSet(&loadingProgrss, 0);
        SDL_AtomicSet(&loadingProgrssMax, 0);
        s_atomicsInited = true;
    }
#endif

    g_mainMenu.introPressStart = "Press Start";

    g_mainMenu.mainPlayEpisode = "Play Episode";
    g_mainMenu.main1PlayerGame = "1 Player Game";
    g_mainMenu.mainMultiplayerGame = "2 Player Game";
    g_mainMenu.mainBattleGame = "Battle Game";
    g_mainMenu.mainEditor = "Editor";
    g_mainMenu.mainOptions = "Options";
    g_mainMenu.mainExit = "Exit";

    g_mainMenu.loading = "Loading...";

    g_mainMenu.languageName = "English";
    g_mainMenu.pluralRules = "one-is-singular";

    g_mainMenu.selectCharacter = "{0} game";

    g_mainMenu.editorBattles = "<Battle Levels>";
    g_mainMenu.editorNewWorld = "<New World>";
    g_mainMenu.editorMakeFor = "Make for:";
    g_mainMenu.editorErrorMissingResources = "Sorry! You are missing {0}, required for the in-game editor.";
    g_mainMenu.editorPromptNewWorldName = "New world name";

    g_mainMenu.gameNoEpisodesToPlay = "<No episodes to play>";
    g_mainMenu.gameNoBattleLevels = "<No battle levels>";
    g_mainMenu.gameBattleRandom = "Random Level";

    g_mainMenu.warnEpCompat = "This episode was made for a different branch of SMBX and may not work properly.";

    g_mainMenu.gameSlotContinue = "SLOT {0} ... {1}%";
    g_mainMenu.gameSlotNew = "SLOT {0} ... NEW GAME";
    g_mainMenu.gameCopySave = "Copy save";
    g_mainMenu.gameEraseSave = "Erase save";
    g_mainMenu.gameSourceSlot = "Select the source slot";
    g_mainMenu.gameTargetSlot = "Now select the target";
    g_mainMenu.gameEraseSlot = "Select the slot to erase";

    g_mainMenu.phraseScore = "Score: {0}";
    g_mainMenu.phraseTime = "Time: {0}";

    g_mainMenu.errorBattleNoLevels = "Can't start battle because of no levels available";

    g_mainMenu.optionsRestartEngine = "Restart engine for changes to take effect.";

    g_mainMenu.wordPlayer = "Player";
    g_mainMenu.wordProfile = "Profile";
    g_mainMenu.wordBack = "Back";
    g_mainMenu.wordResume = "Resume";
    g_mainMenu.wordWaiting = "Waiting";

    g_mainMenu.controlsTitle = "Controls";
    g_mainMenu.controlsConnected = "Connected:";
    g_mainMenu.controlsDeleteKey = "(Alt Jump to Delete)";
    g_mainMenu.controlsDeviceTypes = "Device Types";
    g_mainMenu.controlsInUse = "(In Use)";
    g_mainMenu.controlsNotInUse = "(Not In Use)";

    g_mainMenu.controlsActivateProfile = "Activate profile";
    g_mainMenu.controlsRenameProfile = "Rename profile";
    g_mainMenu.controlsDeleteProfile = "Delete profile";
    g_mainMenu.controlsPlayerControls = "Player controls";
    g_mainMenu.controlsCursorControls = "Cursor controls";
    g_mainMenu.controlsEditorControls = "Editor controls";
    g_mainMenu.controlsHotkeys = "Hotkeys";

    g_mainMenu.controlsOptionRumble = "Rumble";
    // g_mainMenu.controlsOptionGroundPoundButton = "Ground Pound Button";
    g_mainMenu.controlsOptionBatteryStatus = "Battery Status";

    g_mainMenu.wordProfiles = "Profiles";
    g_mainMenu.wordButtons  = "Buttons";

    g_mainMenu.controlsReallyDeleteProfile = "Really delete profile?";
    g_mainMenu.controlsNewProfile = "<New Profile>";

    g_mainMenu.wordNo   = "No";
    g_mainMenu.wordYes  = "Yes";
    g_mainMenu.wordOkay = "Okay";
    g_mainMenu.caseNone = "<None>";
    g_mainMenu.wordOn   = "On";
    g_mainMenu.wordOff  = "Off";
    g_mainMenu.wordShow = "Show";
    g_mainMenu.wordHide = "Hide";
    g_mainMenu.abbrevMilliseconds = "MS";

#ifdef THEXTECH_ENABLE_SDL_NET
    // NetPlay
    g_mainMenu.mainNetplay = "NetPlay";
    g_mainMenu.netplayRoomKey = "Room key:";
    g_mainMenu.netplayJoinRoom = "Join Room";
    g_mainMenu.netplayCreateRoom = "Create Room";
    g_mainMenu.netplayLeaveRoom = "Leave Room";
    g_mainMenu.netplayServer = "Server:";
    g_mainMenu.netplayNickname = "Nickname:";
#endif

    // g_mainMenu.promptDeprecatedSetting = "This file uses a deprecated compatibility flag that will be removed in version 1.3.7.\n\nOld flag: \"{0}\"\nNew flag: \"{1}\"\n\n\nReplace it with the updated flag for version 1.3.6 and newer?";
    // g_mainMenu.promptDeprecatedSettingUnwritable = "An unwritable file ({0}) uses a deprecated compatibility flag that will be removed in version 1.3.7.\n\nSection: [{1}]\nOld flag: \"{2}\"\nNew flag: \"{3}\"\n\n\nPlease update it manually and copy to your device.";
}


static int menuPlayersNum = 0;
static int menuBattleMode = false;

static int menuCopySaveSrc = 0;
static int menuCopySaveDst = 0;

static int s_startAssetPackTimer = 0;

static const int TinyScreenH = 400;
static const int SmallScreenH = 500;
static const int TinyScreenW = 600;

static bool s_prefer_modern_char_sel()
{
    return (g_config.compatibility_mode != Config_t::COMPAT_SMBX13) && (s_episode_speedrun_mode != 3);
}

static bool s_show_separate_2P()
{
    return g_config.compatibility_mode == Config_t::COMPAT_SMBX13 && !g_gameInfo.disableTwoPlayer;
}

#ifdef THEXTECH_ENABLE_SDL_NET
static bool s_show_online()
{
    return !g_gameInfo.disableTwoPlayer;
}

static void s_StartEpisodeOnline()
{
    selSave = 0;

    if((int)Controls::g_InputMethods.size() > 1)
        Controls::ClearInputMethods();

    for(int A = 1; A <= numCharacters; A++)
        blockCharacter[A] = (g_forceCharacter) ? false : SelectWorld[selWorld].blockChar[A];

    for(int p = 0; p < maxLocalPlayers; p++)
        l_screen->charSelect[p] = 0;

    // find character for P1 (player on episode start)
    l_screen->charSelect[0] = 1;

    for(int A = 1; A <= numCharacters; A++)
    {
        if(blockCharacter[A])
            continue;

        l_screen->charSelect[0] = A;
        break;
    }

    StartEpisode();
}

uint32_t s_assetPackHash()
{
    return 0;
}

uint32_t s_engineHash()
{
    return md5::string_to_u32(V_BUILD_VER);
}
#endif

// export it, let's hope for some nice LTO here
int mainMenuPlaystyle()
{
    return s_episode_playstyle;
}

static void s_change_save_item()
{
    if(MenuCursor < 0 || MenuCursor >= maxSaveSlots)
        return;

    int save_configs = SaveSlotInfo[MenuCursor + 1].ConfigDefaults;

    if(save_configs > 0)
    {
        s_episode_playstyle = save_configs - 1;
        s_episode_speedrun_mode = 0;
    }
    else if(save_configs < 0)
    {
        s_episode_speedrun_mode = -save_configs;
        s_episode_playstyle = s_episode_speedrun_mode - 1;
    }
    else
    {
        s_episode_playstyle = SelectWorld[selWorld].bugfixes_on_by_default ? Config_t::MODE_MODERN : Config_t::MODE_CLASSIC;
        s_episode_speedrun_mode = g_config.speedrun_mode.m_value;
    }
}

void menu_draw_infobox_switch_arrows(int infobox_x, int infobox_y)
{
    // XRender::renderRect(infobox_x, infobox_y, 480, 68, {0, 0, 0, 192});

    if(CommonFrame % 90 < 45)
        return;

    if(GFX.CharSelIcons.inited)
    {
        XRender::renderTextureFL(infobox_x + 8, infobox_y + 34 - 24 / 2, 24, 24, GFX.CharSelIcons, 72, 0, 0, nullptr, X_FLIP_HORIZONTAL);
        XRender::renderTextureBasic(infobox_x + 480 - 8 - 24, infobox_y + 34 - 24 / 2, 24, 24, GFX.CharSelIcons, 72, 0);
    }
    else
    {
        XRender::renderTextureFL(infobox_x + 8, infobox_y + 34 - GFX.MCursor[1].w / 2, GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, -90);
        XRender::renderTextureFL(infobox_x + 480 - 8 - GFX.MCursor[1].h, infobox_y + 34 - GFX.MCursor[2].w / 2, GFX.MCursor[2].w, GFX.MCursor[2].h, GFX.MCursor[2], 0, 0, -90);
    }
}

void GetMenuPos(int* MenuX, int* MenuY)
{
    if(MenuX)
        *MenuX = XRender::TargetW / 2 - 100;

    if(MenuY)
        *MenuY = XRender::TargetH - 250;

    // tweaks for MenuX
    if(MenuX && XRender::TargetW < TinyScreenW)
    {
        *MenuX = XRender::TargetW / 2 - 240;
        if(*MenuX < 24 + XRender::TargetOverscanX)
            *MenuX = 24 + XRender::TargetOverscanX;
    }

    // the rest is tweaks for MenuY
    if(!MenuY)
        return;

    if(XRender::TargetH < TinyScreenH)
        *MenuY = 100;
    else if(XRender::TargetH < SmallScreenH)
        *MenuY = XRender::TargetH - 220;

    if(MenuMode >= MENU_SELECT_SLOT_BASE && MenuMode < MENU_SELECT_SLOT_END)
        *MenuY -= c_menuSavesOffsetY;
}

#if !defined(PGE_NO_THREADING)
static int FindWorldsThread(void *)
{
    FindWorlds();
    return 0;
}
#endif

#if (defined(__APPLE__) && defined(USE_BUNDLED_ASSETS)) || defined(FIXED_ASSETS_PATH) || defined(__EMSCRIPTEN__)
#   define CAN_WRITE_APPPATH_WORLDS false
#else
#   define CAN_WRITE_APPPATH_WORLDS true
#endif

struct WorldRoot_t
{
    std::string path;
    bool editable;
};

// helper functions used by FindWorlds() and LoadSingleWorld()
static void s_LoadSingleWorld(const std::string& epDir, const std::string& fName, WorldData& head, TranslateEpisode& tr, bool editable);
static void s_LoadWorldArchive(const std::string& archive);
static void s_FinishFindWorlds();

void FindWorlds()
{
    TranslateEpisode tr;
    WorldData head;
    NumSelectWorld = 0;

    std::vector<WorldRoot_t> worldRoots =
    {
        {AppPath + "worlds/", CAN_WRITE_APPPATH_WORLDS}
    };

    if(AppPathManager::userDirIsAvailable())
    {
        // assume that assets are not writable
        worldRoots.push_back({AppPathManager::userWorldsRootDir(), true});
        worldRoots[0].editable = false;
    }

#ifdef APP_PATH_HAS_EXTRA_WORLDS
    // add worlds from additional romfs packages
    for(const std::string& root : AppPathManager::worldRootDirs())
        worldRoots.push_back({root, false});
#endif

    SelectWorld.clear();
    SelectWorld.emplace_back(SelectWorld_t()); // Dummy entry

#ifndef PGE_NO_THREADING
    if(SDL_AtomicGet(&loading))
    {
        SDL_AtomicSet(&loadingProgrss, 0);
        SDL_AtomicSet(&loadingProgrssMax, 0);

        for(const auto &worldsRoot : worldRoots)
        {
            std::vector<std::string> dirs;
            DirMan episodes(worldsRoot.path);
            episodes.getListOfFolders(dirs);
            SDL_AtomicAdd(&loadingProgrssMax, (int)dirs.size());
            if(!Archives::has_prefix(worldsRoot.path))
            {
                episodes.getListOfFiles(dirs);
                SDL_AtomicAdd(&loadingProgrssMax, (int)dirs.size());
            }
        }
    }
#endif

    for(const auto &worldsRoot : worldRoots)
    {
        DirMan episodes(worldsRoot.path);

        std::vector<std::string> dirs;
        std::vector<std::string> files;
        episodes.getListOfFolders(dirs);

        for(const auto &dir : dirs)
        {
            std::string epDir = worldsRoot.path + dir + "/";
            DirMan episode(epDir);
            episode.getListOfFiles(files, {".wld", ".wldx"});

            for(std::string &fName : files)
                s_LoadSingleWorld(epDir, fName, head, tr, worldsRoot.editable);

#ifdef THEXTECH_PRELOAD_LEVELS
            if(LoadingInProcess)
                UpdateLoad();
#endif

#ifndef PGE_NO_THREADING
            SDL_AtomicAdd(&loadingProgrss, 1);
#endif
        }

        if(!Archives::has_prefix(worldsRoot.path))
        {
            episodes.getListOfFiles(dirs);

            for(const auto &dir : dirs)
            {
                s_LoadWorldArchive(worldsRoot.path + dir);

#ifdef THEXTECH_PRELOAD_LEVELS
                if(LoadingInProcess)
                    UpdateLoad();
#endif

#ifndef PGE_NO_THREADING
                SDL_AtomicAdd(&loadingProgrss, 1);
#endif
            }
        }
    }

    s_FinishFindWorlds();
}

static void s_LoadSingleWorld(const std::string& epDir, const std::string& fName, WorldData& head, TranslateEpisode& tr, bool editable)
{
    SelectWorld_t w;
    w.WorldFilePath = epDir + fName;

    PGE_FileFormats_misc::RWopsTextInput in(Files::open_file(w.WorldFilePath, "r"), w.WorldFilePath);

    if(FileFormats::OpenWorldFileHeaderT(in, head))
    {
        w.WorldName = head.EpisodeTitle;
        head.charactersToS64();
        if(w.WorldName.empty())
            w.WorldName = fName;

        bool is_wldx = (head.meta.RecentFormat == WorldData::PGEX);
        bool is_wld38a = (head.meta.RecentFormat == WorldData::SMBX38A);
        if(is_wldx && (head.meta.configPackId == "SMBX2" || head.meta.engineFeatureLevel > g_gameInfo.contentFeatureLevel))
            w.probably_incompatible = true;
        else if(is_wld38a)
            w.probably_incompatible = true;
        else
            w.bugfixes_on_by_default = is_wldx;

        w.blockChar[1] = head.nocharacter1;
        w.blockChar[2] = head.nocharacter2;
        w.blockChar[3] = head.nocharacter3;
        w.blockChar[4] = head.nocharacter4;
        w.blockChar[5] = head.nocharacter5;

        w.editable = editable;

        if(tr.tryTranslateTitle(epDir, fName, w.WorldName))
            pLogDebug("Translated world title: %s", w.WorldName.c_str());

#ifdef THEXTECH_ENABLE_SDL_NET
        if(w.WorldFilePath.size() > 2 && w.WorldFilePath[0] == ':' && w.WorldFilePath[1] == 'a')
            w.lz4_content_hash = md5::string_to_u32(w.WorldFilePath);
#endif

        SelectWorld.push_back(std::move(w));
    }
}

static void s_LoadWorldArchive(const std::string& archive)
{
    SelectWorld_t w;
    w.WorldFilePath = "@";
    w.WorldFilePath += archive;
    w.WorldFilePath += ":/";

    IniProcessing ini = Files::load_ini(w.WorldFilePath + "_meta.ini");

    ini.beginGroup("content");

    // confirm filename exists
    ini.read("filename", w.WorldName, w.WorldName);
    if(w.WorldName.empty())
    {
        pLogInfo("Episode [%s] not loaded; could not parse _meta.ini", archive.c_str());
        return;
    }
    w.WorldFilePath += w.WorldName;

    // confirm engine support
    std::string engine;
    ini.read("engine", engine, engine);
    if(engine != "TheXTech")
    {
        pLogInfo("Episode [%s] not loaded; for incompatible engine [%s]", archive.c_str(), engine.c_str());
        return;
    }

    // confirm platform support
    std::string platform;
    ini.read("platform", platform, platform);

    bool platform_okay = false;

#ifndef __16M__
    if(platform == "main")
        platform_okay = true;
#endif

#ifdef __3DS__
    if(platform == "3ds")
        platform_okay = true;
#endif

#ifdef __WII__
    if(platform == "wii")
        platform_okay = true;
#endif

#ifdef __16M__
    if(platform == "dsi")
        platform_okay = true;
#endif

    if(!platform_okay)
    {
        pLogInfo("Episode [%s] not loaded; for incompatible platform [%s]", archive.c_str(), platform.c_str());
        return;
    }

#ifdef THEXTECH_ENABLE_SDL_NET
    ini.read("source-hash", w.lz4_content_hash, 0);
#endif

    ini.endGroup();

    // load properties
    ini.beginGroup("properties");

    // playstyle classic
    bool is_classic;
    ini.read("is-classic", is_classic, false);
    w.bugfixes_on_by_default = !is_classic;

    // character block
    std::string char_block;
    ini.read("char-block", char_block, char_block);
    char_block.resize(numCharacters, '0');

    for(int i = 1; i <= numCharacters; i++)
        w.blockChar[i] = (char_block[i - 1] == '1');

    // title
    ini.read("title", w.WorldName, w.WorldName);

    std::string title_key = "title-";
    title_key += CurrentLanguage;
    ini.read(title_key.c_str(), w.WorldName, w.WorldName);

    title_key += '-';
    title_key += CurrentLangDialect;
    ini.read(title_key.c_str(), w.WorldName, w.WorldName);

    ini.endGroup();

    // last details
    w.editable = false;

#ifdef THEXTECH_ENABLE_SDL_NET
    if(!w.lz4_content_hash)
    {
        SDL_RWops* rw = Files::open_file(archive, "rb");
        if(rw)
        {
            SDL_RWseek(rw, -4, RW_SEEK_END);
            uint8_t hash[4];
            if(SDL_RWread(rw, hash, 1, 4) == 4)
            {
                w.lz4_content_hash = (hash[0])
                    | ((uint32_t)(hash[1]) <<  8)
                    | ((uint32_t)(hash[2]) << 16)
                    | ((uint32_t)(hash[3]) << 24);
            }
            SDL_RWclose(rw);
        }
    }
#endif

    // store it!
    SelectWorld.push_back(std::move(w));
}

static void s_FinishFindWorlds()
{
    if(SelectWorld.size() <= 1) // No available worlds in the list
    {
        SelectWorld.clear();
        SelectWorld.emplace_back(SelectWorld_t()); // Dummy entry
        SelectWorld.emplace_back(SelectWorld_t()); // "no episodes to play" entry
        SelectWorld[1].WorldName = g_mainMenu.gameNoEpisodesToPlay;
        SelectWorld[1].disabled = true;
    }

    // Sort all worlds by alphabetical order
    tinysort(SelectWorld.begin(), SelectWorld.end(),
              [](const SelectWorld_t& a, const SelectWorld_t& b)
    {
        return a.WorldName < b.WorldName;
    });

    NumSelectWorld = (int)(SelectWorld.size() - 1);

    SelectWorld_t battles = SelectWorld_t();
    battles.WorldName = g_mainMenu.editorBattles;
    battles.WorldFilePath = "battle";
    battles.editable = true;
    SelectWorld.push_back(std::move(battles));

    SelectWorld_t createWorld = SelectWorld_t();
    createWorld.WorldName = g_mainMenu.editorNewWorld;
    createWorld.editable = true;
    SelectWorld.push_back(std::move(createWorld));

    ContentSelectScreen::Prepare();

#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loading, 0);
#endif
}

void LoadSingleWorld(const std::string wPath)
{
    TranslateEpisode tr;
    WorldData head;
    NumSelectWorld = 0;

    SelectWorld.clear();
    SelectWorld.emplace_back(SelectWorld_t()); // Dummy entry

    std::string fName = Files::basename(wPath);
    std::string epDir = wPath.substr(0, wPath.size() - fName.size());

    if(epDir.empty())
        epDir = "./";

    epDir = DirMan(epDir).absolutePath() + "/";

    s_LoadSingleWorld(epDir, fName, head, tr, true);

    s_FinishFindWorlds();
}

#if !defined(THEXTECH_PRELOAD_LEVELS) && !defined(PGE_NO_THREADING)
static int FindLevelsThread(void *)
{
    FindLevels();
    return 0;
}
#endif

void FindLevels()
{
    std::vector<WorldRoot_t> battleRoots =
    {
        {AppPath + "battle/", CAN_WRITE_APPPATH_WORLDS}
    };

    if(AppPathManager::userDirIsAvailable())
    {
        // assume that assets are not writable
        battleRoots.push_back({AppPathManager::userBattleRootDir(), true});
        battleRoots[0].editable = false;
    }

    SelectBattle.clear();
    SelectBattle.emplace_back(SelectWorld_t()); // Dummy entry

    NumSelectBattle = 1;
    SelectBattle.emplace_back(SelectWorld_t()); // "random level" entry
    SelectBattle[1].WorldName = g_mainMenu.gameBattleRandom;
    LevelData head;

#ifndef PGE_NO_THREADING
    if(SDL_AtomicGet(&loading))
    {
        SDL_AtomicSet(&loadingProgrss, 0);
        SDL_AtomicSet(&loadingProgrssMax, 0);

        for(const auto &battleRoot : battleRoots)
        {
            std::vector<std::string> files;
            DirMan battleLvls(battleRoot.path);
            battleLvls.getListOfFiles(files, {".lvl", ".lvlx"});
            SDL_AtomicAdd(&loadingProgrssMax, (int)files.size());
        }
    }
#endif

    for(const auto &battleRoot : battleRoots)
    {
        std::vector<std::string> files;
        DirMan battleLvls(battleRoot.path);
        battleLvls.getListOfFiles(files, {".lvl", ".lvlx"});
        for(std::string &fName : files)
        {
            std::string wPath = battleRoot.path + fName;

            PGE_FileFormats_misc::RWopsTextInput in(Files::open_file(wPath, "r"), wPath);
            if(FileFormats::OpenLevelFileHeaderT(in, head))
            {
                SelectWorld_t w;
                w.WorldFilePath = battleRoot.path;
                w.WorldFilePath += '/';
                w.WorldFilePath += fName;
                w.WorldName = head.LevelName;
                if(w.WorldName.empty())
                    w.WorldName = fName;
                w.editable = battleRoot.editable;
                SelectBattle.push_back(std::move(w));

#ifdef THEXTECH_PRELOAD_LEVELS
                if(LoadingInProcess)
                    UpdateLoad();
#endif
            }
#ifndef PGE_NO_THREADING
            SDL_AtomicAdd(&loadingProgrss, 1);
#endif
        }
    }

    NumSelectBattle = ((int)SelectBattle.size() - 1);

    if(SelectBattle.size() <= 2) // No available levels in the list
    {
        SelectBattle.clear();
        SelectBattle.emplace_back(SelectWorld_t()); // Dummy entry

        NumSelectBattle = 1;
        SelectBattle.emplace_back(SelectWorld_t()); // "no battle levels" entry
        SelectBattle[1].WorldName = g_mainMenu.gameNoBattleLevels;
        SelectBattle[1].disabled = true;
    }

    ContentSelectScreen::Prepare();

#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loading, 0);
#endif
}

static void s_PrepareContentSelect()
{
#ifdef THEXTECH_PRELOAD_LEVELS
    ContentSelectScreen::Prepare();
#elif defined(PGE_NO_THREADING)
    if(MenuMode == MENU_BATTLE_MODE)
        FindLevels();
    else
        FindWorlds();
#else
    SDL_AtomicSet(&loading, 1);
    if(MenuMode == MENU_BATTLE_MODE)
        loadingThread = SDL_CreateThread(FindLevelsThread, "FindLevels", nullptr);
    else
        loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", nullptr);
    SDL_DetachThread(loadingThread);
#endif
}


static void s_handleMouseMove(int items, int x, int y, int maxWidth, int itemHeight)
{
    For(A, 0, items)
    {
        if(SharedCursor.Y >= y + A * itemHeight && SharedCursor.Y <= y + 16 + A * itemHeight)
        {
            if(SharedCursor.X >= x && SharedCursor.X <= x + maxWidth)
            {
                if(MenuMouseRelease && SharedCursor.Primary)
                    MenuMouseClick = true;
                if(MenuCursor != A)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = A;
                    break;
                }
            }
        }
    }
}

static int s_quitKeyPos()
{
    int quitKeyPos = 2;

    if(s_show_separate_2P())
        quitKeyPos++;

    if(!g_gameInfo.disableBattleMode)
        quitKeyPos++;

#ifdef THEXTECH_ENABLE_SDL_NET
    if(s_show_online())
        quitKeyPos++;
#endif

    if(g_config.enable_editor)
        quitKeyPos++;

    return quitKeyPos;
}


static bool s_can_enter_ap_screen()
{
    return MenuMode == MENU_INTRO || (XRender::TargetH >= TinyScreenH && MenuMode == MENU_MAIN);
}



bool mainMenuUpdate()
{
    int MenuX, MenuY;
    GetMenuPos(&MenuX, &MenuY);

    // Location_t tempLocation;
    int menuLen;
    // Player_t blankPlayer;

    bool upPressed = l_SharedControls.MenuUp;
    bool downPressed = l_SharedControls.MenuDown;
    bool leftPressed = l_SharedControls.MenuLeft;
    bool rightPressed = l_SharedControls.MenuRight;
    bool homePressed = SharedCursor.Tertiary;

    bool menuDoPress = l_SharedControls.MenuDo || l_SharedControls.Pause;
    bool menuBackPress = l_SharedControls.MenuBack;

    bool altPressed = false;

    for(int i = 0; i < l_screen->player_count; i++)
    {
        Controls_t &c = Controls::g_RawControls[i];

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;

        homePressed |= c.Drop;
        altPressed |= c.AltJump;
    }

    menuBackPress |= SharedCursor.Secondary && MenuMouseRelease;

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

    bool should_enter_ap_screen = homePressed && s_can_enter_ap_screen() && MenuCursorCanMove;

    if(should_enter_ap_screen && GetAssetPacks().size() <= 1)
    {
        PlaySoundMenu(SFX_BlockHit);
        MenuCursorCanMove = false;
    }

    if(should_enter_ap_screen && GetAssetPacks().size() > 1)
    {
        s_startAssetPackTimer++;
        if(s_startAssetPackTimer == 64)
        {
            PlaySoundMenu(SFX_Do);
            FadeOutMusic(500);
            ScreenAssetPack::g_LoopActive = true;
            GameMenu = false;
            return true;
        }
    }
    else if(s_can_enter_ap_screen() && s_startAssetPackTimer > 0)
        s_startAssetPackTimer -= 2;
    else
        s_startAssetPackTimer = 0;

    {
        {
            bool k = false;
            k |= menuBackPress;
            k |= menuDoPress;
            k |= upPressed;
            k |= downPressed;
            k |= leftPressed;
            k |= rightPressed;
            k |= homePressed;
            k |= altPressed;

            if(!k)
                MenuCursorCanMove = true;
        }

        bool in_content_select = (MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME || MenuMode == MENU_BATTLE_MODE || MenuMode == MENU_EDITOR);
#ifdef THEXTECH_ENABLE_SDL_NET
        if(MenuMode == MENU_NETPLAY_WORLD_SELECT)
            in_content_select = true;
#endif

        if(!g_pollingInput && (MenuMode != MENU_CHARACTER_SELECT_NEW && MenuMode != MENU_CHARACTER_SELECT_NEW_BM && MenuMode != MENU_NEW_OPTIONS && MenuMode != MENU_INTRO && !in_content_select))
        {
            int cursorDelta = 0;

            if(upPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor -= 1;
                    cursorDelta = -1;
                }

                MenuCursorCanMove = false;
            }
            else if(downPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor += 1;
                    cursorDelta = +1;
                }

                MenuCursorCanMove = false;
            }

            if(cursorDelta != 0)
            {
                if(MenuMode >= MENU_SELECT_SLOT_BASE && MenuMode < MENU_SELECT_SLOT_END)
                    s_change_save_item();

                PlaySoundMenu(SFX_Slide);
            }

        } // No keyboard/Joystick grabbing active

        if(MenuMode == MENU_INTRO && XRender::TargetH >= TinyScreenH)
            MenuMode = MENU_MAIN;

#ifndef PGE_NO_THREADING
        if(SDL_AtomicGet(&loading))
        {
            if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
                PlaySoundMenu(SFX_BlockHit);

            if(MenuCursor != 0)
                MenuCursor = 0;
        }
        else
#endif

        // Menu Intro
        if(MenuMode == MENU_INTRO)
        {
            if(MenuMouseRelease && SharedCursor.Primary)
                MenuMouseClick = true;

            if(menuBackPress && MenuCursorCanMove)
            {
                int quitKeyPos = s_quitKeyPos();

                MenuMode = MENU_MAIN;
                MenuCursor = quitKeyPos;
                MenuCursorCanMove = false;
                PlaySoundMenu(SFX_Slide);
            }

            if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
            {
                MenuCursorCanMove = false;
                MenuMode = MENU_MAIN;
                MenuCursor = 0;
                PlaySoundMenu(SFX_Do);
            }
        }
        // Main Menu
        else if(MenuMode == MENU_MAIN)
        {
            if(SharedCursor.Move)
            {
                For(A, 0, 10)
                {
                    if(SharedCursor.Y >= MenuY + A * 30 && SharedCursor.Y <= MenuY + A * 30 + 16)
                    {
                        int i = 0;
                        if(A == i++)
                            menuLen = 18 * (s_show_separate_2P() ? (int)g_mainMenu.main1PlayerGame.size() : (int)g_mainMenu.mainPlayEpisode.size()) - 2;
                        else if(s_show_separate_2P() && A == i++)
                            menuLen = 18 * (int)g_mainMenu.mainMultiplayerGame.size() - 2;
                        else if(!g_gameInfo.disableBattleMode && A == i++)
                            menuLen = 18 * (int)g_mainMenu.mainBattleGame.size();
#ifdef THEXTECH_ENABLE_SDL_NET
                        else if(s_show_online() && A == i++)
                            menuLen = 18 * (int)g_mainMenu.mainNetplay.size();
#endif
                        else if(g_config.enable_editor && A == i++)
                            menuLen = 18 * (int)g_mainMenu.mainEditor.size();
                        else if(A == i++)
                            menuLen = 18 * (int)g_mainMenu.mainOptions.size();
                        else if(A == i++)
                            menuLen = 18 * (int)g_mainMenu.mainExit.size();
                        else
                            break;

                        if(SharedCursor.X >= MenuX && SharedCursor.X <= MenuX + menuLen)
                        {
                            if(MenuMouseRelease && SharedCursor.Primary)
                                MenuMouseClick = true;

                            if(MenuCursor != A)
                            {
                                PlaySoundMenu(SFX_Slide);
                                MenuCursor = A;
                            }
                        }
                    }
                }
            }

            if(menuBackPress && MenuCursorCanMove)
            {
                int quitKeyPos = s_quitKeyPos();

                if(XRender::TargetH < TinyScreenH)
                {
                    MenuCursorCanMove = false;
                    MenuMode = MENU_INTRO;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(MenuCursor != quitKeyPos)
                {
                    MenuCursor = quitKeyPos;
                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
            }
            else if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
            {
                MenuCursorCanMove = false;
                // PlayerCharacter = 0;
                // PlayerCharacter2 = 0;

                int i = 0;
                if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_1PLAYER_GAME;
                    menuPlayersNum = 1;
                    menuBattleMode = false;
                    MenuCursor = 0;
                    s_PrepareContentSelect();
                }
                else if(s_show_separate_2P() && MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_2PLAYER_GAME;
                    menuPlayersNum = 2;
                    menuBattleMode = false;
                    MenuCursor = 0;

                    s_PrepareContentSelect();
                }
                else if(!g_gameInfo.disableBattleMode && MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_BATTLE_MODE;
                    menuPlayersNum = 2;
                    menuBattleMode = true;
                    MenuCursor = 0;

                    s_PrepareContentSelect();
                }
#ifdef THEXTECH_ENABLE_SDL_NET
                else if(s_show_online() && MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_NETPLAY;
                    MenuCursor = 0;

                    s_PrepareContentSelect();

                    XMessage::Connect();
                }
#endif
                else if(g_config.enable_editor && MenuCursor == i++)
                {
                    if(!GFX.EIcons.inited)
                    {
                        PlaySoundMenu(SFX_BlockHit);
                        g_MessageType = MESSAGE_TYPE_SYS_WARNING;
                        MessageText = fmt::format_ne(g_mainMenu.editorErrorMissingResources, "EditorIcons.png");
                        PauseGame(PauseCode::Message);
                    }
                    else if(!EditorCustom::loaded)
                    {
                        PlaySoundMenu(SFX_BlockHit);
                        g_MessageType = MESSAGE_TYPE_SYS_WARNING;
                        MessageText = fmt::format_ne(g_mainMenu.editorErrorMissingResources, "editor.ini");
                        PauseGame(PauseCode::Message);
                    }
                    else
                    {
                        PlaySoundMenu(SFX_Do);
                        MenuMode = MENU_EDITOR;
                        MenuCursor = 0;

                        s_PrepareContentSelect();
                    }
                }
                else if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_NEW_OPTIONS;
                    OptionsScreen::Init();
                }
                else if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    GracefulQuit(true);
                    return true;
                }

            }


            if(MenuMode == MENU_MAIN)
            {
                int quitKeyPos = s_quitKeyPos();

                if(MenuCursor > quitKeyPos)
                    MenuCursor = 0;

                if(MenuCursor < 0)
                    MenuCursor = quitKeyPos;
            }
        } // Main Menu

        // Character Select
        else if(MenuMode == MENU_CHARACTER_SELECT_NEW ||
                MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
        {
            int ret = ConnectScreen::Logic();
            if(ret == -1)
            {
                // disconnect input methods for convenience
                Controls::ClearInputMethods();

                if(MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
                {
                    MenuCursor = selWorld - 1;
                    MenuMode = MENU_BATTLE_MODE;
                }
                else
                {
                    MenuCursor = selSave - 1;
                    if(menuPlayersNum == 1)
                        MenuMode = MENU_SELECT_SLOT_1P;
                    else
                        MenuMode = MENU_SELECT_SLOT_2P;
                }

                MenuCursorCanMove = false;
            }
            else if(ret == 1)
            {
                if(MenuMode == MENU_CHARACTER_SELECT_NEW)
                {
                    // writing to m_value to avoid extra UpdateConfig hook
                    g_config.playstyle.m_value = s_episode_playstyle;
                    g_config.playstyle.m_set = ConfigSetLevel::ep_config;

                    if(g_config.speedrun_mode.m_set != ConfigSetLevel::cmdline)
                    {
                        g_config.speedrun_mode.m_value = s_episode_speedrun_mode;
                        g_config.speedrun_mode.m_set = ConfigSetLevel::ep_config;
                    }

                    MenuCursor = 0;
                    StartEpisode();
                    return true;
                }
                else if(MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
                {
                    MenuCursor = 0;
                    StartBattleMode();
                    return true;
                }
            }
        }
#ifdef THEXTECH_ENABLE_SDL_NET
        else if(MenuMode == MENU_NETPLAY)
        {
            if(SharedCursor.Move)
            {
                For(A, 0, 10)
                {
                    if(SharedCursor.Y >= MenuY + A * 30 && SharedCursor.Y <= MenuY + A * 30 + 16)
                    {
                        int i = 0;
                        if(A == i++)
                            menuLen = 18 * (int)g_mainMenu.netplayJoinRoom.size();
                        else if(A == i++)
                            menuLen = 18 * (int)g_mainMenu.netplayCreateRoom.size();
                        else if(A == i++)
                            menuLen = 18 * (int)g_mainMenu.netplayServer.size();
                        else if(A == i++)
                            menuLen = 18 * (int)g_mainMenu.netplayNickname.size();
                        else
                            break;

                        if(SharedCursor.X >= MenuX && SharedCursor.X <= MenuX + menuLen)
                        {
                            if(MenuMouseRelease && SharedCursor.Primary)
                                MenuMouseClick = true;

                            if(MenuCursor != A)
                            {
                                PlaySoundMenu(SFX_Slide);
                                MenuCursor = A;
                            }
                        }
                    }
                }
            }

            if(menuBackPress && MenuCursorCanMove)
            {
                int netplayPos = 1;
                if(s_show_separate_2P())
                    netplayPos++;
                if(!g_gameInfo.disableBattleMode)
                    netplayPos++;

                XMessage::Disconnect();
                MenuMode = MENU_MAIN;
                MenuCursor = netplayPos;
                MenuCursorCanMove = false;
                PlaySoundMenu(SFX_Slide);
            }
            else if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
            {
                MenuCursorCanMove = false;

                if((MenuCursor == 0 || MenuCursor == 1) && !XMessage::IsConnected())
                    PlaySoundMenu(SFX_BlockHit);
                else if(MenuCursor == 0)
                {
                    PlaySoundMenu(SFX_Do);

                    s_room_info = XMessage::RoomInfo();
                    s_room_info.room_key = XMessage::RoomFromString(TextEntryScreen::Run(g_mainMenu.netplayRoomKey));

                    if(s_room_info.room_key)
                    {
                        // this shouldn't actually happen synchronously
                        XMessage::RequestFillRoomInfo(s_room_info);

                        selWorld = -1;

                        for(size_t i = 0; i < SelectWorld.size(); i++)
                        {
                            if(SelectWorld[i].lz4_content_hash == s_room_info.content_hash)
                            {
                                selWorld = i;
                                break;
                            }
                        }

                        if(s_room_info.room_key == 0 || s_room_info.engine_hash != s_engineHash() || s_room_info.asset_hash != s_assetPackHash() || selWorld == -1)
                            PlaySoundMenu(SFX_BlockHit);
                        else
                        {
                            PlaySoundMenu(SFX_Do);
                            XMessage::JoinRoom(s_room_info.room_key);
                            s_StartEpisodeOnline();
                        }
                    }
                    else
                        PlaySoundMenu(SFX_BlockHit);
                }
                else if(MenuCursor == 1)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_NETPLAY_WORLD_SELECT;
                    menuPlayersNum = 1;
                    menuBattleMode = false;
                    MenuCursor = 0;
                }
                else if(MenuCursor == 2)
                {
                    PlaySoundMenu(SFX_Do);
                    XMessage::Disconnect();
                    g_netplayServer = TextEntryScreen::Run(g_mainMenu.netplayServer);
                    XMessage::Connect();
                }
                else if(MenuCursor == 3)
                {
                    PlaySoundMenu(SFX_Do);
                    g_netplayNickname = TextEntryScreen::Run(g_mainMenu.netplayNickname);
                }
            }

            if(MenuMode == MENU_NETPLAY)
            {
                if(MenuCursor > 3)
                    MenuCursor = 0;
                else if(MenuCursor < 0)
                    MenuCursor = 3;
            }
        }
#endif

        // World Select
        else if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME
#ifdef THEXTECH_ENABLE_SDL_NET
            || MenuMode == MENU_NETPLAY_WORLD_SELECT
#endif
            || MenuMode == MENU_BATTLE_MODE || MenuMode == MENU_EDITOR)
        {
            int ret = ContentSelectScreen::Logic();

            if(ret == -1)
            {
                MenuCursor = MenuMode - 1;

                if(MenuMode == MENU_BATTLE_MODE)
                {
                    MenuCursor = !s_show_separate_2P() ? 1 : 2;
                }
                else if(MenuMode == MENU_EDITOR)
                {
                    MenuCursor = 3;
                    if(!s_show_separate_2P())
                        MenuCursor--;
                    if(g_gameInfo.disableBattleMode)
                        MenuCursor--;
#ifdef THEXTECH_ENABLE_SDL_NET
                    if(s_show_online())
                        MenuCursor++;
#endif
                }

#ifdef THEXTECH_ENABLE_SDL_NET
                if(MenuMode == MENU_NETPLAY_WORLD_SELECT)
                {
                    MenuMode = MENU_NETPLAY;
                    MenuCursor = 1;
                }
                else
#endif
                {
                    MenuMode = MENU_MAIN;
                }
//'world select back
            }
            else if(ret == 1)
            {
                if(MenuMode == MENU_EDITOR)
                {
                    if(selWorld == (int)SelectWorld.size() - 1)
                    {
                        ClearWorld(true);
                        WorldName = TextEntryScreen::Run(g_mainMenu.editorPromptNewWorldName);
                        if(!WorldName.empty())
                        {
                            std::string fn = WorldName;
                            // eliminate bad characters
                            std::replace(fn.begin(), fn.end(), '/', '_');
                            std::replace(fn.begin(), fn.end(), '\\', '_');
                            std::replace(fn.begin(), fn.end(), '.', '_');
                            std::replace(fn.begin(), fn.end(), ':', '_');
                            std::replace(fn.begin(), fn.end(), '<', '_');
                            std::replace(fn.begin(), fn.end(), '>', '_');
                            std::replace(fn.begin(), fn.end(), '"', '_');
                            std::replace(fn.begin(), fn.end(), '|', '_');
                            std::replace(fn.begin(), fn.end(), '?', '_');
                            std::replace(fn.begin(), fn.end(), '*', '_');
                            // ensure uniqueness (but still case-sensitive for now)
                            while(DirMan::exists(AppPathManager::userWorldsRootDir() + fn))
                                fn += "2";
                            DirMan::mkAbsPath(AppPathManager::userWorldsRootDir() + fn);

                            std::string wPath = AppPathManager::userWorldsRootDir() + fn + "/world.wld";
                            if(ContentSelectScreen::editor_target_thextech)
                                wPath += "x";
                            g_recentWorldEditor = wPath;

                            SaveWorld(wPath, (ContentSelectScreen::editor_target_thextech) ? FileFormats::WLD_PGEX : FileFormats::WLD_SMBX64);

#ifdef PGE_NO_THREADING
                            FindWorlds();
#else
                            SDL_AtomicSet(&loading, 1);
                            loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", NULL);
                            SDL_DetachThread(loadingThread);
#endif
                        }
                    }
                    else if(!g_gameInfo.disableBattleMode && selWorld == (int)SelectWorld.size() - 2)
                    {
                        ClearWorld(true);
                        GameMenu = false;
                        LevelSelect = false;
                        BattleMode = true;
                        LevelEditor = true;
                        WorldEditor = false;
                        ClearLevel();
                        ClearGame();
                        SetupPlayers();

                        std::string lPath = AppPathManager::userBattleRootDir();
                        {
                            // find a single battle level to open first
                            std::vector<std::string> files;
                            DirMan battleLvls(lPath);
                            battleLvls.getListOfFiles(files, {(ContentSelectScreen::editor_target_thextech) ? ".lvlx" : ".lvl"});
                            if(files.empty())
                                lPath += "newbattle.lvl";
                            else
                                lPath += files[0];
                        }

                        if(!Files::fileExists(lPath))
                        {
                            if(ContentSelectScreen::editor_target_thextech)
                                lPath += "x";

                            SaveLevel(lPath, (ContentSelectScreen::editor_target_thextech) ? FileFormats::LVL_PGEX : FileFormats::LVL_SMBX64);
                        }

                        OpenLevel(lPath);
                        EditorBackup(); // EditorRestore() gets called when not in world editor

                        // todo: de-dupe with below
                        const std::string& wPath = SelectWorld[selWorld].WorldFilePath;
                        if(g_recentWorldEditor != wPath)
                        {
                            g_recentWorldEditor = wPath;
                            SaveConfig();
                        }

                        Integrator::setEditorFile(FileName);
                        editorScreen.ResetCursor();
                        editorScreen.active = false;
                        MouseRelease = false;
                        return true;
                    }
                    else
                    {
                        GameMenu = false;
                        LevelSelect = false;
                        BattleMode = false;
                        LevelEditor = true;
                        WorldEditor = true;
                        ClearLevel();
                        ClearGame();
                        SetupPlayers();
                        const std::string& wPath = SelectWorld[selWorld].WorldFilePath;
                        OpenWorld(wPath);
                        if(g_recentWorldEditor != wPath)
                        {
                            g_recentWorldEditor = wPath;
                            SaveConfig();
                        }
                        Integrator::setEditorFile(FileName);
                        editorScreen.ResetCursor();
                        editorScreen.active = false;
                        MouseRelease = false;
                        return true;
                    }
                }
                // battle mode
                else if(MenuMode == MENU_BATTLE_MODE)
                {
                    MenuMode = MENU_CHARACTER_SELECT_NEW_BM;
                    ConnectScreen::MainMenu_Start(2);
                }
#ifdef THEXTECH_ENABLE_SDL_NET
                // new room
                else if(MenuMode == MENU_NETPLAY_WORLD_SELECT)
                {
                    s_room_info = XMessage::RoomInfo();
                    s_room_info.engine_hash = s_engineHash();
                    s_room_info.asset_hash = s_assetPackHash();
                    s_room_info.content_hash = SelectWorld[selWorld].lz4_content_hash;

                    // this shouldn't actually happen synchronously
                    XMessage::JoinNewRoom(s_room_info);

                    s_StartEpisodeOnline();
                }
#endif
                // enter save select
                else
                {
                    LoadCustomPlayerPreviews(Files::dirname(SelectWorld[selWorld].WorldFilePath).c_str());

                    FindSaves();
                    MenuMode *= MENU_SELECT_SLOT_BASE;
                    MenuCursor = 0;

                    s_change_save_item();
                }

                MenuCursorCanMove = false;
            }
        } // World select

        // Save Select
        else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
        {
            if(SharedCursor.Move)
            {
                int old_item = MenuCursor;
                s_handleMouseMove(c_menuItemSavesDelete, MenuX, MenuY, 300, 30);

                if(MenuCursor != old_item)
                    s_change_save_item();
            }

            // new mode selection logic
            if(MenuCursor >= 0 && MenuCursor < maxSaveSlots && SaveSlotInfo[MenuCursor + 1].ConfigDefaults == 0)
            {
                // switch mode
                if(MenuCursorCanMove && (leftPressed || rightPressed) && s_episode_speedrun_mode == 0)
                {
                    if(s_episode_playstyle == Config_t::MODE_MODERN)
                        s_episode_playstyle = Config_t::MODE_CLASSIC;
                    else
                        s_episode_playstyle = Config_t::MODE_MODERN;

                    PlaySoundMenu(SFX_Climbing);
                    MenuCursorCanMove = false;
                }

                // only enter the menu if allowed by the global speedrun mode
                if(MenuCursorCanMove && homePressed && g_config.speedrun_mode.m_set == ConfigSetLevel::cmdline)
                {
                    PlaySoundMenu(SFX_BlockHit);
                }
                // enter vanilla mode if an existing save slot
                else if(MenuCursorCanMove && homePressed && SaveSlotInfo[MenuCursor + 1].Progress >= 0)
                {
                    int target_bugfixes = (SelectWorld[selWorld].bugfixes_on_by_default) ? Config_t::MODE_MODERN : Config_t::MODE_CLASSIC;

                    if(s_episode_playstyle == Config_t::MODE_VANILLA)
                        s_episode_playstyle = target_bugfixes;
                    else
                        s_episode_playstyle = Config_t::MODE_VANILLA;

                    PlaySoundMenu(SFX_Climbing);
                    MenuCursorCanMove = false;
                }
                // go to speedrun menu otherwise
                else if(MenuCursorCanMove && homePressed)
                {
                    PlaySoundMenu(SFX_PlayerGrow);
                    selSave = MenuCursor + 1;

                    if(s_episode_speedrun_mode == 0)
                        MenuCursor = s_episode_playstyle;
                    else
                        MenuCursor = s_episode_speedrun_mode + 2;

                    MenuMode += MENU_SELECT_SLOT_ADVMODE_ADD;
                    MenuCursorCanMove = false;
                    MenuMouseClick = false;
                }
            }
            else if(MenuCursor >= 0 && MenuCursor < maxSaveSlots)
            {
                int save_configs = SaveSlotInfo[MenuCursor + 1].ConfigDefaults;

                if(save_configs > 0)
                {
                    s_episode_playstyle = save_configs - 1;
                    s_episode_speedrun_mode = 0;
                }
                else if(save_configs < 0)
                {
                    s_episode_speedrun_mode = -save_configs;
                    s_episode_playstyle = s_episode_speedrun_mode - 1;
                }
            }

            if(MenuCursorCanMove || MenuMouseClick)
            {
                if(menuBackPress)
                {
//'save select back
                    UnloadCustomPlayerPreviews();

                    MenuMode /= MENU_SELECT_SLOT_BASE;

                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    if(MenuCursor == c_menuItemSavesCopy) // Copy the gamesave
                    {
                        PlaySoundMenu(SFX_Do);
                        MenuCursor = 0;
                        MenuMode += MENU_SELECT_SLOT_COPY_S1_ADD;
                        MenuCursorCanMove = false;
                    }
                    else if(MenuCursor == c_menuItemSavesDelete) // Delete the gamesave
                    {
                        PlaySoundMenu(SFX_Do);
                        MenuCursor = 0;
                        MenuMode += MENU_SELECT_SLOT_DELETE_ADD;
                        MenuCursorCanMove = false;
                    }
                    // block invalid speedrun continuation
                    else if(MenuCursor >= 0 && MenuCursor <= c_menuItemSavesEndList
                        && (int)g_config.speedrun_mode != 0 && g_config.speedrun_mode != -SaveSlotInfo[MenuCursor + 1].ConfigDefaults
                        && (SaveSlotInfo[MenuCursor + 1].ConfigDefaults != 0 || SaveSlotInfo[MenuCursor + 1].Progress >= 0))
                    {
                        PlaySoundMenu(SFX_BlockHit);
                        MenuCursorCanMove = false;
                    }
                    else if(MenuCursor >= 0 && MenuCursor <= c_menuItemSavesEndList) // Select the save slot, but still need to select players
                    {
                        selSave = MenuCursor + 1;

                        // warn the user of incompatibility if present
                        if(SaveSlotInfo[selSave].Progress < 0 && SelectWorld[selWorld].probably_incompatible)
                        {
                            PlaySoundMenu(SFX_Message);

                            g_MessageType = MESSAGE_TYPE_SYS_WARNING;
                            MessageText = g_mainMenu.warnEpCompat;
                            PauseGame(PauseCode::Message);
                        }

                        PlaySoundMenu(SFX_Do);

                        if(MenuMode == MENU_SELECT_SLOT_2P)
                            ConnectScreen::MainMenu_Start(2);
                        else if(s_prefer_modern_char_sel())
                            ConnectScreen::MainMenu_Start(1);
                        else
                            ConnectScreen::LegacyMenu_Start();

                        MenuMode = MENU_CHARACTER_SELECT_NEW;
                        MenuCursorCanMove = false;
                    }
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
            {
                if(MenuCursor > c_menuItemSavesDelete) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = c_menuItemSavesDelete;
            }
        } // Save Slot Select

        // Save Select
        else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
                MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
        {
            if(SharedCursor.Move)
                s_handleMouseMove(c_menuItemSavesEndList, MenuX, MenuY, 300, 30);

            if(MenuCursorCanMove || MenuMouseClick)
            {
                if(menuBackPress)
                {
//'save select back
                    if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
                    {
                        MenuMode -= MENU_SELECT_SLOT_COPY_S1_ADD;
                    }
                    else
                    {
                        MenuMode -= MENU_SELECT_SLOT_COPY_S1_ADD;
                        MenuCursor = c_menuItemSavesCopy;
                    }

                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Do);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    SDL_assert_release(IF_INRANGE(MenuCursor, 0, maxSaveSlots - 1));
                    int slot = MenuCursor + 1;

                    if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1)
                    {
                        if(SaveSlotInfo[slot].Progress < 0)
                            PlaySoundMenu(SFX_BlockHit);
                        else
                        {
                            PlaySoundMenu(SFX_Do);
                            menuCopySaveSrc = slot;
                            MenuMode += MENU_SELECT_SLOT_COPY_S1_ADD;
                        }

                        MenuCursorCanMove = false;
                    }
                    else if(menuCopySaveSrc == slot)
                    {
                        PlaySoundMenu(SFX_BlockHit);
                        MenuCursorCanMove = false;
                    }
                    else
                    {
                        PlaySoundMenu(SFX_Transform);
                        menuCopySaveDst = slot;
                        CopySave(selWorld, menuCopySaveSrc, menuCopySaveDst);
                        FindSaves();
                        MenuMode -= MENU_SELECT_SLOT_COPY_S2_ADD;
                        MenuCursor = c_menuItemSavesCopy;
                        MenuCursorCanMove = false;
                    }
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
               MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
            {
                if(MenuCursor > c_menuItemSavesEndList) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = c_menuItemSavesEndList;
            }
        }

        // Delete gamesave
        else if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_2P_DELETE)
        {
            if(SharedCursor.Move)
                s_handleMouseMove(c_menuItemSavesEndList, MenuX, MenuY, 300, 30);

            if(MenuCursorCanMove || MenuMouseClick)
            {
                if(menuBackPress)
                {
//'save select back
                    MenuMode -= MENU_SELECT_SLOT_DELETE_ADD;
                    MenuCursor = c_menuItemSavesDelete;
                    PlaySoundMenu(SFX_Do);
                    MenuCursorCanMove = false;
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    MenuMode -= MENU_SELECT_SLOT_DELETE_ADD;
                    DeleteSave(selWorld, (MenuCursor + 1));
                    FindSaves();
                    MenuCursor = c_menuItemSavesDelete;
                    PlaySoundMenu(SFX_LavaMonster);
                    MenuCursorCanMove = false;
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_2P_DELETE)
            {
                if(MenuCursor > c_menuItemSavesEndList) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = c_menuItemSavesEndList;
            }
        }

        // Advanced mode selection
        else if(MenuMode == MENU_SELECT_SLOT_1P_ADVMODE || MenuMode == MENU_SELECT_SLOT_2P_ADVMODE)
        {
            if(SharedCursor.Move)
                s_handleMouseMove(5, MenuX, MenuY, MenuX, 30);

            if(MenuCursorCanMove || MenuMouseClick)
            {
                if(menuBackPress)
                {
//'save select back
                    MenuMode -= MENU_SELECT_SLOT_ADVMODE_ADD;
                    MenuCursor = selSave - 1;
                    selSave = 0;
                    PlaySoundMenu(SFX_Slide);
                    MenuCursorCanMove = false;
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    if(MenuCursor < 3)
                    {
                        s_episode_playstyle = MenuCursor;
                        s_episode_speedrun_mode = 0;
                    }
                    else
                    {
                        s_episode_speedrun_mode = MenuCursor - 2;
                        s_episode_playstyle = s_episode_speedrun_mode - 1;
                    }

                    PlaySoundMenu(SFX_Do);
                    MenuMode -= MENU_SELECT_SLOT_ADVMODE_ADD;
                    MenuCursor = selSave - 1;
                    selSave = 0;
                    MenuCursorCanMove = false;
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P_ADVMODE || MenuMode == MENU_SELECT_SLOT_2P_ADVMODE)
            {
                if(MenuCursor > 5) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = 5;
            }
        }

        // Input Settings
        else if(MenuMode == MENU_INPUT_SETTINGS)
        {
            int ret = menuControls_Logic();
            if(ret == -1)
            {
                Controls::SaveConfig();
                MenuMode = MENU_NEW_OPTIONS;
                MenuCursorCanMove = false;
            }
        }

        // New Settings
        else if(MenuMode == MENU_NEW_OPTIONS)
        {
            if(OptionsScreen::Logic())
            {
                int optionsIndex = 1;
                if(s_show_separate_2P())
                    optionsIndex++;
                if(!g_gameInfo.disableBattleMode)
                    optionsIndex++;
#ifdef THEXTECH_ENABLE_SDL_NET
                if(s_show_online())
                    optionsIndex++;
#endif
                if(g_config.enable_editor)
                    optionsIndex++;
                MenuMode = MENU_MAIN;
                MenuCursor = optionsIndex;
                MenuCursorCanMove = false;
                PlaySoundMenu(SFX_Slide);
            }
        }
    }

    return false;
}

static constexpr int find_in_string(const char* haystack, const char* haystack_start, char needle)
{
    return (*haystack == '\0') ? -1 :
        ((*haystack == needle) ? haystack - haystack_start : find_in_string(haystack + 1, haystack_start, needle));
}

static constexpr bool in_string(const char* haystack, char needle)
{
    return find_in_string(haystack, haystack, needle) != -1;
}

static constexpr bool str_prefix(const char* string, const char* prefix)
{
    return (*prefix == '\0' || (*string == *prefix && str_prefix(string + 1, prefix + 1)));
}

static constexpr int find_in_string(const char* haystack, char needle)
{
    return find_in_string(haystack, haystack, needle);
}

void drawGameVersion(bool disable_git, bool git_only)
{
    constexpr bool is_release = !in_string(V_LATEST_STABLE, '-');
    constexpr bool is_main = str_prefix(V_BUILD_BRANCH, "main");
    constexpr bool is_stable = str_prefix(V_BUILD_BRANCH, "stable");
    constexpr bool is_head = str_prefix(V_BUILD_BRANCH, "HEAD");
    constexpr bool is_wip = str_prefix(V_BUILD_BRANCH, "wip-");

    constexpr bool is_dirty = in_string(V_BUILD_VER, '-');

    constexpr bool show_branch = (!is_main && !(is_release && is_stable) && !is_head);
    constexpr bool show_commit = (!is_release || (!is_main && !is_stable && !is_head));

    // show version
    if(!git_only)
        SuperPrintRightAlign("v" V_LATEST_STABLE, 5, XRender::TargetW - XRender::TargetOverscanX - 2, 2);

    // show branch
    if(show_branch && !disable_git)
    {
        int y = show_commit ? XRender::TargetH - 36 : XRender::TargetH - 18;

        if(is_wip)
        {
            // strip the "wip-"
            SuperPrintRightAlign(&V_BUILD_BRANCH[find_in_string(V_BUILD_BRANCH, '-') + 1], 5, XRender::TargetW - XRender::TargetOverscanX - 2, y);
        }
        else
            SuperPrintRightAlign(V_BUILD_BRANCH, 5, XRender::TargetW - XRender::TargetOverscanX - 2, y);
    }

    // show git commit
    if(show_commit && !disable_git)
    {
        if(is_dirty)
        {
            // only show -d, not -dirty
            SuperPrintRightAlign(find_in_string(V_BUILD_VER, '-') + 2 + 1, "#" V_BUILD_VER, 5, XRender::TargetW - XRender::TargetOverscanX - 2, XRender::TargetH - 18);
        }
        else
            SuperPrintRightAlign("#" V_BUILD_VER, 5, XRender::TargetW - XRender::TargetOverscanX - 2, XRender::TargetH - 18);
    }
}

static void s_drawGameTypeTitle(int x, int y)
{
    if(MenuMode == MENU_EDITOR)
        SuperPrint(g_mainMenu.mainEditor, 3, x, y, XTColorF(0.8_n, 0.8_n, 0.3_n));
    else if(menuBattleMode)
        SuperPrint(g_mainMenu.mainBattleGame, 3, x, y, XTColorF(0.3_n, 0.3_n, 1.0_n));
    else
    {
        // show "Play Episode" when at the save slot selecting and adjusting settings (not when the game was started in compat / speedrun mode)
        if(!s_show_separate_2P())
            SuperPrint(g_mainMenu.mainPlayEpisode, 3, x, y, XTColorF(1.0_n, 0.3_n, 0.3_n));
        else if(menuPlayersNum == 1)
            SuperPrint(g_mainMenu.main1PlayerGame, 3, x, y, XTColorF(1.0_n, 0.3_n, 0.3_n));
        else
            SuperPrint(g_mainMenu.mainMultiplayerGame, 3, x, y, XTColorF(0.3_n, 1.0_n, 0.3_n));
    }
}

static void s_drawGameSaves(int MenuX, int MenuY)
{
    int A;

    for(A = 1; A <= maxSaveSlots; A++)
    {
        int posY = MenuY - 30 + (A * 30);
        const auto& info = SaveSlotInfo[A];

        if(info.Progress >= 0)
        {
            // "SLOT {0} ... {1}%"
            std::string p = fmt::format_ne(g_mainMenu.gameSlotContinue, A, SaveSlotInfo[A].Progress);
            int len = SuperTextPixLen(p, 3);

            SuperPrint(p, 3, MenuX, posY);

            if(info.Stars > 0)
            {
                len += 4;
                XRender::renderTextureBasic(MenuX + len, posY + 1, GFX.Interface[5]);

                len += GFX.Interface[5].w + 4;
                XRender::renderTextureBasic(MenuX + len, posY + 2, GFX.Interface[1]);

                len += GFX.Interface[1].w + 4;
                SuperPrint(fmt::format_ne("{0}", info.Stars), 3, MenuX + len, posY);
            }
        }
        else
        {
            // "SLOT {0} ... NEW GAME"
            SuperPrint(fmt::format_ne(g_mainMenu.gameSlotNew, A), 3, MenuX, posY);
        }

        int mode_icon_X = MenuX + 340;
        int mode_icon_Y = MenuY - 34 + (A * 30);

        int save_configs = info.ConfigDefaults;
        if(save_configs == 0 && A == MenuCursor + 1 && (MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P))
        {
            save_configs = s_episode_playstyle + 1;
            if((int)s_episode_speedrun_mode != 0)
                save_configs = -s_episode_speedrun_mode;
            if((int)g_config.speedrun_mode != 0)
                save_configs = -g_config.speedrun_mode;
        }

        if(save_configs != 0)
        {
            uint16_t rot = 0;
            uint8_t op = 255;
            if(A == MenuCursor + 1)
            {
                rot = SDL_abs((int)(CommonFrame % 64) - 32);
                rot = rot - 16;
            }

            if(info.ConfigDefaults == 0)
                op = SDL_abs((int)(CommonFrame % 32) - 16) * 3 + 128;

            if(save_configs == Config_t::MODE_CLASSIC + 1)
            {
                XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFXNPC[NPCID_POWER_S3], 0, 0, 32, 32, rot, nullptr, X_FLIP_NONE, XTAlpha(op));
            }
            else if(save_configs == Config_t::MODE_VANILLA + 1)
            {
                XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFXNPC[NPCID_FODDER_S3], 0, 0, 32, 32, rot, nullptr, X_FLIP_NONE, XTAlpha(op));
            }
            else if(save_configs < 0)
            {
                XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFXNPC[NPCID_TIMER_S2], 0, 0, 32, 32, rot, nullptr, X_FLIP_NONE, XTAlpha(op));
                if(save_configs > -10)
                    XRender::renderTextureBasic(mode_icon_X + 12, mode_icon_Y + 12, GFX.Font1[-save_configs]);
            }
            else
            {
                if(GFX.EIcons.inited)
                    XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFX.EIcons, 0, 32*Icon::thextech, 32, 32, rot, nullptr, X_FLIP_NONE, XTAlpha(op));
                else
                    XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFXNPC[NPCID_ICE_POWER_S3], 0, 0, 32, 32, rot, nullptr, X_FLIP_NONE, XTAlpha(op));
            }
        }
    }

    if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
    {
        SuperPrint(g_mainMenu.gameCopySave, 3, MenuX, MenuY - 30 + (A * 30));
        A++;
        SuperPrint(g_mainMenu.gameEraseSave, 3, MenuX, MenuY - 30 + (A * 30));
    }


    if(MenuCursor < 0 || MenuCursor >= maxSaveSlots || (MenuMode != MENU_SELECT_SLOT_1P && MenuMode != MENU_SELECT_SLOT_2P))
        return;

    const auto& info = SaveSlotInfo[MenuCursor + 1];

    int infobox_x = XRender::TargetW / 2 - 240;
    int infobox_y = MenuY + 145 + c_menuSavesOffsetY;

    // forbid incompatible speedrun
    if((int)g_config.speedrun_mode != 0 && g_config.speedrun_mode != -info.ConfigDefaults
        && (info.ConfigDefaults != 0 || info.Progress >= 0))
    {
        XRender::renderRect(XRender::TargetW / 2 - 240, infobox_y, 480, 68, XTColorF(0, 0, 0, 1.0_n));
        SuperPrintScreenCenter("SAVE INCOMPATIBLE", 3, infobox_y + 4, XTColorF(1.0_n, 0.0_n, 0.0_n, 1.0_n));
        SuperPrintScreenCenter(fmt::format_ne("WITH SPEEDRUN MODE {0}", (int)g_config.speedrun_mode), 3, infobox_y + 24, XTColorF(1.0_n, 0.0_n, 0.0_n, 1.0_n));
        SuperPrintScreenCenter("(COMMAND LINE FLAG)", 3, infobox_y + 44, XTColorF(0.8_n, 0.8_n, 0.8_n, 1.0_n));
    }
    // initialize config
    else if(info.ConfigDefaults == 0)
    {
        XRender::renderRect(infobox_x, infobox_y, 480, 68, {0, 0, 0, 192});

        XTColor color;

        if(s_episode_playstyle == Config_t::MODE_MODERN)
            color = XTColorF(0.5_n, 0.8_n, 1.0_n);
        else if(s_episode_playstyle == Config_t::MODE_CLASSIC)
            color = XTColorF(1.0_n, 0.5_n, 0.5_n);
        else
            color = XTColorF(0.8_n, 0.5_n, 0.2_n);

        if(s_episode_speedrun_mode != 0)
        {
            SuperPrintScreenCenter("SPEEDRUN MODE " + std::to_string(s_episode_speedrun_mode), 3, infobox_y + 14, color);
        }
        else
        {
            // int target_bugfixes = (SelectWorld[selWorld].bugfixes_on_by_default) ? Config_t::MODE_MODERN : Config_t::MODE_CLASSIC;

            std::string playstyle_string;
            playstyle_string += g_options.playstyle.m_display_name;
            playstyle_string += ": ";

            if(s_episode_playstyle == Config_t::MODE_MODERN)
                playstyle_string += g_options.playstyle.m_enum_values[0].m_display_name;
            else if(s_episode_playstyle == Config_t::MODE_CLASSIC)
                playstyle_string += g_options.playstyle.m_enum_values[1].m_display_name;
            else
                playstyle_string += g_options.playstyle.m_enum_values[2].m_display_name;

            // if(target_bugfixes == g_config.playstyle)
            //     playstyle_string += " (Recommended)";

            SuperPrintScreenCenter(playstyle_string, 3, infobox_y + 14, color);

            // switch arrows
            menu_draw_infobox_switch_arrows(infobox_x, infobox_y);
        }

        const std::string& playstyle_description
            = (s_episode_playstyle == Config_t::MODE_MODERN) ?
                g_options.playstyle.m_enum_values[0].m_display_tooltip
            : (s_episode_playstyle == Config_t::MODE_CLASSIC) ?
                g_options.playstyle.m_enum_values[1].m_display_tooltip
            :
                g_options.playstyle.m_enum_values[2].m_display_tooltip;

        SuperPrintScreenCenter(playstyle_description, 5, infobox_y + 34, color);
    }
    // display fun save slot info
    else if(info.Progress >= 0)
    {
        int row_1 = infobox_y + 10;
        int row_2 = infobox_y + 42;
        int row_c = infobox_y + 26;

        bool hasFails = (g_config.enable_fails_tracking || info.FailsEnabled);
        bool show_timer = info.Time > 0 && (g_config.enable_playtime_tracking || info.ConfigDefaults < 0);

        // recenter if single row
        if(!show_timer && !hasFails)
            row_1 = infobox_y + 26;

        XRender::renderRect(infobox_x, infobox_y, 480, 68, {0, 0, 0, 127});

        // Temp string
        std::string t;

        // Score
        int row_score = show_timer ? row_1 : row_c;
        SuperPrint(t = fmt::format_ne(g_mainMenu.phraseScore, info.Score), 3, infobox_x + 10, row_score);

        // Gameplay Timer
        if(show_timer)
        {
            std::string t = GameplayTimer::formatTime(info.Time);

            if(t.size() > 9)
                t = t.substr(0, t.size() - 4);

            SuperPrint(fmt::format_ne(g_mainMenu.phraseTime, t), 3, infobox_x + 10, row_2);
        }

        // If demos off, put (l)ives and (c)oins on center
        int row_lc = (hasFails) ? row_1 : row_c;

        // Print lives on the screen (from gfx_update2.cpp)
        DrawLives(infobox_x + 272 + 32, row_lc, info.Lives, info.Hundreds, s_episode_playstyle == Config_t::MODE_VANILLA);

        // Print coins on the screen (from gfx_update2.cpp)
        int coins_x = infobox_x + 480 - 10 - 36 - 62;
        XRender::renderTextureBasic(coins_x + 16, row_lc, GFX.Interface[2]);
        XRender::renderTextureBasic(coins_x + 40, row_lc + 2, GFX.Interface[1]);
        SuperPrintRightAlign(t = std::to_string(info.Coins), 1, coins_x + 62 + 36, row_lc + 2);

        // Fails Counter
        if(hasFails)
            SuperPrintRightAlign(fmt::format_ne("{0}: {1}", g_gameInfo.fails_counter_title, info.Fails), 3, infobox_x + 480 - 10, row_2);
    }
}

void mainMenuDraw()
{
    int MenuX, MenuY;
    GetMenuPos(&MenuX, &MenuY);

#ifdef __3DS__
    XRender::setTargetLayer(2);
#endif

    // Render the permanent menu graphics (curtain, URL, logo)

    // URL
    if(XRender::TargetH >= SmallScreenH)
        XRender::renderTextureBasic(XRender::TargetW / 2 - GFX.MenuGFX[3].w / 2, XRender::TargetH - 24, GFX.MenuGFX[3]);

    bool at_main_title = (MenuMode == MENU_MAIN || MenuMode == MENU_INTRO);
    bool draw_in_asset_pack = at_main_title && s_startAssetPackTimer >= 2;

    if(!draw_in_asset_pack)
    {
        // Curtain
        // correction to loop the original asset properly
        int curtain_draw_w = GFX.MenuGFX[1].w;
        if(curtain_draw_w == 800)
            curtain_draw_w = 768;
        int curtain_horiz_reps = XRender::TargetW / curtain_draw_w + 2;

        for(int i = 0; i < curtain_horiz_reps; i++)
            XRender::renderTextureBasic(curtain_draw_w * i, 0, curtain_draw_w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);

#ifdef __3DS__
        if(at_main_title)
            XRender::setTargetLayer(3);
#endif

        // game logo
        int LogoMode = 0;
        if(XRender::TargetH >= TinyScreenH || MenuMode == MENU_INTRO)
            LogoMode = 1;
        else if(MenuMode == MENU_MAIN)
            LogoMode = 2;

        if(LogoMode == 1)
        {
            // show at half opacity if not at main menu on a small screen
            XTColor logo_tint = (XRender::TargetH < SmallScreenH && MenuMode != MENU_INTRO && MenuMode != MENU_MAIN) ? XTAlpha(127) : XTColor();

            int logo_y = XRender::TargetH / 2 - 230;

            // place manually on small screens
            if(XRender::TargetH < TinyScreenH)
                logo_y = 16 + (XRender::TargetH - 320) / 2;
            else if(XRender::TargetH < SmallScreenH)
                logo_y = 16;
            else if(XRender::TargetH <= 600)
                logo_y = 40;

            XRender::renderTextureBasic(XRender::TargetW / 2 - GFX.MenuGFX[2].w / 2, logo_y, GFX.MenuGFX[2], logo_tint);
        }
        else if(LogoMode == 2)
        {
            SuperPrint(g_gameInfo.title, 3, XRender::TargetW / 2 - g_gameInfo.title.length()*9, 30);
        }
    }

#ifdef __3DS__
    if(MenuMode != MENU_NEW_OPTIONS)
        XRender::setTargetLayer(3);
#endif

    drawGameVersion(false, draw_in_asset_pack);


    // Menu Intro
    if(MenuMode == MENU_INTRO)
    {
        if((CommonFrame % 90) < 45)
            SuperPrintScreenCenter(g_mainMenu.introPressStart, 3, XRender::TargetH - 48 - (XRender::TargetH - 320) / 4);
    }

#ifndef PGE_NO_THREADING
    // loading (can't safely render)
    if(SDL_AtomicGet(&loading))
    {
        if(SDL_AtomicGet(&loadingProgrssMax) <= 0)
            SuperPrint(g_mainMenu.loading, 3, MenuX, MenuY);
        else
        {
            int progress = (SDL_AtomicGet(&loadingProgrss) * 100) / SDL_AtomicGet(&loadingProgrssMax);
            SuperPrint(fmt::format_ne("{0} {1}%", g_mainMenu.loading, progress), 3, MenuX, MenuY);
        }
    }
    // DO NOT DETACH THE BELOW ELSE STATEMENT FROM THE FOLLOWING SERIES OF IF CLAUSES
    else
#endif
    // Main menu
        if(MenuMode == MENU_MAIN)
    {
        int i = 0;
        SuperPrint(s_show_separate_2P() ? g_mainMenu.main1PlayerGame : g_mainMenu.mainPlayEpisode, 3, MenuX, MenuY+30*(i++));
        if(s_show_separate_2P())
            SuperPrint(g_mainMenu.mainMultiplayerGame, 3, MenuX, MenuY+30*(i++));
        if(!g_gameInfo.disableBattleMode)
            SuperPrint(g_mainMenu.mainBattleGame, 3, MenuX, MenuY+30*(i++));
#ifdef THEXTECH_ENABLE_SDL_NET
        if(s_show_online())
            SuperPrint(g_mainMenu.mainNetplay, 3, MenuX, MenuY+30*(i++));
#endif
        if(g_config.enable_editor)
            SuperPrint(g_mainMenu.mainEditor, 3, MenuX, MenuY+30*(i++));
        SuperPrint(g_mainMenu.mainOptions, 3, MenuX, MenuY+30*(i++));
        SuperPrint(g_mainMenu.mainExit, 3, MenuX, MenuY+30*(i++));
        XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }
    // Character select
    else if(MenuMode == MENU_CHARACTER_SELECT_NEW ||
            MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        const auto& world_list = (MenuMode == MENU_CHARACTER_SELECT_NEW) ? SelectWorld : SelectBattle;
        SuperPrint(world_list[selWorld].WorldName, 3, MenuX, MenuY - 40, XTColorF(0.6_n, 1.0_n, 1.0_n));

        ConnectScreen::Render();
    }
#ifdef THEXTECH_ENABLE_SDL_NET
    // NetPlay main menu
    else if(MenuMode == MENU_NETPLAY)
    {
        int i = 0;
        XTColor c;
        XTColor s;

        if(XMessage::IsConnected())
            s = {200, 255, 200};
        else
        {
            s = {255, 200, 200};
            c = {127, 127, 127};
        }

        SuperPrint(g_mainMenu.netplayJoinRoom, 3, MenuX, MenuY+30*(i++), c);
        SuperPrint(g_mainMenu.netplayCreateRoom, 3, MenuX, MenuY+30*(i++), c);
        SuperPrint(g_mainMenu.netplayServer + ' ' + g_netplayServer, 3, MenuX, MenuY+30*(i++), s);
        SuperPrint(g_mainMenu.netplayNickname + ' ' + g_netplayNickname, 3, MenuX, MenuY+30*(i++));
        XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }
#endif

    // Episode / Level selection
    else if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME
#ifdef THEXTECH_ENABLE_SDL_NET
        || MenuMode == MENU_NETPLAY_WORLD_SELECT
#endif
        || MenuMode == MENU_BATTLE_MODE || MenuMode == MENU_EDITOR)
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        ContentSelectScreen::Render();
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P) // Save Select
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, XTColorF(0.6_n, 1.0_n, 1.0_n));
        s_drawGameSaves(MenuX, MenuY);
        XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
            MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2) // Copy save
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, XTColorF(0.6_n, 1.0_n, 1.0_n));
        s_drawGameSaves(MenuX, MenuY);

        if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1)
            SuperPrint(g_mainMenu.gameSourceSlot, 3, MenuX, MenuY + c_menuSavesFooterHint, XTColorF(0.7_n, 0.7_n, 1.0_n));
        else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
            SuperPrint(g_mainMenu.gameTargetSlot, 3, MenuX, MenuY + c_menuSavesFooterHint, XTColorF(0.7_n, 1.0_n, 0.7_n));

        if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
        {
            XRender::renderTextureBasic(MenuX - 20, MenuY + ((menuCopySaveSrc - 1) * 30), GFX.MCursor[0]);
            XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[3]);
        }
        else
            XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_2P_DELETE) // Delete save
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, XTColorF(0.6_n, 1.0_n, 1.0_n));
        s_drawGameSaves(MenuX, MenuY);

        SuperPrint(g_mainMenu.gameEraseSlot, 3, MenuX, MenuY + c_menuSavesFooterHint, XTColorF(1.0_n, 0.7_n, 0.7_n));

        XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_ADVMODE || MenuMode == MENU_SELECT_SLOT_2P_ADVMODE) // Advanced mode select
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, XTColorF(0.6_n, 1.0_n, 1.0_n));

        int A = 0;

        SuperPrint("Modern Game", 3, MenuX, MenuY + (A++ * 30), XTColorF(0.5_n, 0.7_n, 1.0_n));
        SuperPrint("Classic Game", 3, MenuX, MenuY + (A++ * 30), XTColorF(1.0_n, 0.7_n, 0.7_n));
        SuperPrint("Vanilla Game", 3, MenuX, MenuY + (A++ * 30), XTColorF(0.8_n, 0.5_n, 0.2_n));
        SuperPrint("Speedrun Mode 1", 3, MenuX, MenuY + (A++ * 30), XTColorF(0.5_n, 0.7_n, 1.0_n));
        SuperPrint("Speedrun Mode 2", 3, MenuX, MenuY + (A++ * 30), XTColorF(1.0_n, 0.7_n, 0.7_n));
        SuperPrint("Speedrun Mode 3", 3, MenuX, MenuY + (A++ * 30), XTColorF(0.8_n, 0.5_n, 0.2_n));

        XRender::renderTextureBasic(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    // Player controls setup
    else if(MenuMode == MENU_INPUT_SETTINGS)
    {
        menuControls_Render();
    }

    // New options screen
    else if(MenuMode == MENU_NEW_OPTIONS)
    {
#ifdef __3DS__
        // draw options screen (only) in top draw plane
        XRender::setTargetLayer(3);
#endif

        OptionsScreen::Render();
    }

    // fade to / from asset pack screen
    if(s_can_enter_ap_screen() && s_startAssetPackTimer > 0)
    {
        ScreenAssetPack::DrawBackground(s_startAssetPackTimer);
        g_levelScreenFader.clearFader();
    }

    // Mouse cursor
    XRender::renderTextureBasic(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);
}
