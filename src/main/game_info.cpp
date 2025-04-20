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

#include <fmt_format_ne.h>
#include <Utils/files.h>
#include <Utils/files_ini.h>
#include <Utils/strings.h>
#include <IniProcessor/ini_processing.h>
#include <PGE_File_Formats/pge_file_lib_globs.h>

#include "game_info.h"
#include "script/luna/luna.h"

#include "globals.h"

#include "../version.h"


GameInfo g_gameInfo;

static void readCheats(IniProcessing &conf, std::vector<GameInfo::CheatAlias> &dst, const std::string &group)
{
    dst.clear();
    conf.beginGroup(group);
    {
        auto ak = conf.allKeys();
        for(const auto &k : ak)
        {
            GameInfo::CheatAlias ca;
            ca.first = k;
            conf.read(k.c_str(), ca.second, std::string());
            if(ca.second.empty())
                continue; // Skip empty aliases
            dst.push_back(ca);
        }
    }
    conf.endGroup();
}

const std::string GameInfo::titleWindow() const
{
#if defined(ENABLE_OLD_CREDITS) && !defined(CUSTOM_GAME_NAME_TITLE)
    return "Super Mario Bros. X - Version 1.3 - www.SuperMarioBrothers.org";
#else
    const char* ver_string = "TheXTech v" V_LATEST_STABLE ", #" V_BUILD_VER;
    return fmt::sprintf_ne("%s - (%s)", this->title.c_str(), ver_string);
#endif
}

void GameInfo::InitGameInfo()
{
#ifdef CUSTOM_GAME_NAME_TITLE
    title = CUSTOM_GAME_NAME_TITLE;
#else
#   ifdef ENABLE_OLD_CREDITS
    title = "Super Mario Bros. X";
#   else
    title = "TheXTech Engine";
#   endif
#endif /* CUSTOM_GAME_NAME_TITLE */

    statusIconName.clear();

    characterName[1] = "Mario";
    characterName[2] = "Luigi";
    characterName[3] = "Peach";
    characterName[4] = "Toad";
    characterName[5] = "Link";

    fails_counter_title = "FAILS";

    wordStarAccusativeSingular = "star";
    wordStarAccusativeDual_Cnt = "";
    wordStarAccusativePlural = "stars";

    creditsFont = 5;

#ifdef CUSTOM_CREDITS_URL
    creditsHomePage = CUSTOM_CREDITS_URL;
#else
#   ifdef ENABLE_OLD_CREDITS
    creditsHomePage = "www.SuperMarioBrothers.org";
#   else
    creditsHomePage = "wohlsoft.ru";
#   endif
#endif

    disableTwoPlayer = false;
    disableBattleMode = false;

    activity_settings_in_compat = false;
    contentFeatureLevel = 0;

    ResetIntroActivitySettings();
    ResetOutroActivitySettings();
}

void GameInfo::ResetIntroActivitySettings()
{
    introEnableActivity = true;
    introMaxPlayersCount = 6;
    introCharacters = {1, 2, 3, 4, 5};
    introCharacterCurrent = 0;
    introDeadMode = false;
}

void GameInfo::ResetOutroActivitySettings()
{
    outroEnableActivity = true;
    outroMaxPlayersCount = 5;
    outroCharacters = {1, 2, 3, 4, 5};
    outroStates = {4, 7, 5, 3, 6};
    outroMounts = {0, 3, 0, 1, 0};
    outroCharacterCurrent = 0;
    outroInitialDirections = {0, 0, 0, 0, 0};
    outroWalkDirection = -1;
    outroAutoJump = true;
    outroDeadMode = false;
}

void GameInfo::LoadGameInfo()
{
    InitGameInfo();

    std::string gameInfoPath = AppPath + "gameinfo.ini";
    if(Files::fileExists(gameInfoPath))
    {
        IniProcessing config = Files::load_ini(gameInfoPath);

        config.beginGroup("game");
        {
            if(config.hasKey("title"))
                config.read("title", title, title);
            config.read("disable-two-player", disableTwoPlayer, false);
            config.read("disable-battle-mode", disableBattleMode, false);
            config.read("status-icon-name", statusIconName, std::string());
            config.read("feature-level-supported", contentFeatureLevel, 0);

            constexpr unsigned int engineFeatureLevel = V_FEATURE_LEVEL;
            if(contentFeatureLevel > engineFeatureLevel)
            {
                pLogDebug("Limiting asset pack to feature level %u (supports %u)", engineFeatureLevel, contentFeatureLevel);
                contentFeatureLevel = engineFeatureLevel;
            }
        }
        config.endGroup();

        // legacy options
        if(!config.beginGroup("fails-counter"))
        {
            config.beginGroup("death-counter"); // Backup fallback
        }
        {
            config.read("title", fails_counter_title, fails_counter_title);
        }
        config.endGroup();

        config.beginGroup("objects");
        {
            config.read("star", wordStarAccusativeSingular, "star");
            config.read("star-singular", wordStarAccusativeSingular, g_gameInfo.wordStarAccusativeSingular);
            config.read("star-plural", wordStarAccusativePlural, g_gameInfo.wordStarAccusativeSingular + "s");
        }
        config.endGroup();

        config.beginGroup("characters");
        {
            for(int i = 1; i <= numCharacters; ++i)
            {
                std::string name = fmt::sprintf_ne("name%d", i);
                config.read(name.c_str(),
                            characterName[i],
                            characterName[i]);
            }
        }
        config.endGroup();

        config.beginGroup("intro");
        LoadIntroActivitySettings(config);
        config.endGroup();

        config.beginGroup("outro");
        LoadOutroActivitySettings(config);
        config.endGroup();

        config.beginGroup("credits");
        {
            config.read("font", creditsFont, 5);
            config.read("homepage", creditsHomePage, creditsHomePage);

            std::string value;
            Strings::dealloc(creditsGame);

            std::string credits_file;
            config.read("game-credits-file", credits_file, "");

            if(!credits_file.empty())
            {
                PGE_FileFormats_misc::RWopsTextInput input(Files::open_file(AppPath + credits_file, "r"));
                while(!input.eof())
                {
                    input.readLine(value);
                    if(!value.empty() || !input.eof())
                        creditsGame.push_back(value);
                }
            }

            if(creditsGame.empty())
            {
                // Old format
                for(int cr = 1; ; cr++)
                {
                    std::string key = fmt::sprintf_ne("game-credit-%d", cr);
                    if(!config.hasKey(key))
                        break;
                    config.read(key.c_str(), value, value);
                    creditsGame.push_back(value);
                }
            }
        }
        config.endGroup();

        readCheats(config, cheatsGlobalAliases, "cheats-global-aliases");
        readCheats(config, cheatsGlobalRenames, "cheats-global-renames");
        readCheats(config, cheatsWorldAliases, "cheats-world-aliases");
        readCheats(config, cheatsWorldRenames, "cheats-world-renames");
        readCheats(config, cheatsLevelAliases, "cheats-level-aliases");
        readCheats(config, cheatsLevelRenames, "cheats-level-renames");
    }
}

void GameInfo::LoadIntroActivitySettings(IniProcessing& config)
{
    config.read("enable-activity", introEnableActivity, introEnableActivity);
    config.read("max-players-count", introMaxPlayersCount, introMaxPlayersCount);
    config.read("characters", introCharacters, introCharacters);
    introDeadMode = !introEnableActivity || introMaxPlayersCount < 1;
}

void GameInfo::LoadOutroActivitySettings(IniProcessing& config)
{
    config.read("enable-activity", outroEnableActivity, outroEnableActivity);
    config.read("max-players-count", outroMaxPlayersCount, outroMaxPlayersCount);
    config.read("characters", outroCharacters, outroCharacters);
    config.read("states", outroStates, outroStates);
    config.read("mounts", outroMounts, outroMounts);
    config.read("auto-jump", outroAutoJump, outroAutoJump);
    IniProcessing::StrEnumMap dirs
    {
        {"left", -1},
        {"idle", 0},
        {"right", +1}
    };
    config.readEnum("walk-direction", outroWalkDirection, outroWalkDirection, dirs);
    config.read("initial-directions", outroInitialDirections, outroInitialDirections);
    outroDeadMode = !outroEnableActivity || outroMaxPlayersCount < 1;
}

int GameInfo::introCharacterNext()
{
    if(introCharacters.empty())
        return 1;
    if(g_gameInfo.introCharacterCurrent >= introCharacters.size())
        g_gameInfo.introCharacterCurrent = 0;
    int ret = g_gameInfo.introCharacters[g_gameInfo.introCharacterCurrent++];

    if(ret > 5) // anti-idiot protection
        ret = 5;
    else if(ret < 1)
        ret = 1;

    return ret;
}

int GameInfo::outroCharacterNext()
{
    if(outroCharacters.empty())
        return 1;
    if(g_gameInfo.outroCharacterCurrent >= outroCharacters.size())
        g_gameInfo.outroCharacterCurrent = 0;
    int ret = g_gameInfo.outroCharacters[g_gameInfo.outroCharacterCurrent++];

    if(ret > 5) // anti-idiot protection
        ret = 5;
    else if(ret < 1)
        ret = 1;

    return ret;
}

void initGameInfo()
{
    g_gameInfo.InitGameInfo();
    g_gameInfo.LoadGameInfo();
}
