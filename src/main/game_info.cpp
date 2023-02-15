/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#include <AppPath/app_path.h>
#include <IniProcessor/ini_processing.h>
#include "game_info.h"
#include "script/luna/luna.h"

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

void initGameInfo()
{
#ifdef CUSTOM_GAME_NAME_TITLE
    g_gameInfo.title = CUSTOM_GAME_NAME_TITLE;
    g_gameInfo.titleWindow = CUSTOM_GAME_NAME_TITLE;
#else
#   ifdef ENABLE_OLD_CREDITS
    g_gameInfo.title = "Super Mario Bros. X";
    g_gameInfo.titleWindow = "Super Mario Bros. X - Version 1.3 - www.SuperMarioBrothers.org";
#   else
    g_gameInfo.title = "TheXTech Engine";
    g_gameInfo.titleWindow = fmt::format_ne("TheXTech v{0}, #{1}", V_LATEST_STABLE, V_BUILD_VER);
#   endif
#endif /* CUSTOM_GAME_NAME_TITLE */

    g_gameInfo.characterName[1] = "Mario";
    g_gameInfo.characterName[2] = "Luigi";
    g_gameInfo.characterName[3] = "Peach";
    g_gameInfo.characterName[4] = "Toad";
    g_gameInfo.characterName[5] = "Link";

    g_gameInfo.wordStarAccusativeSingle = "star";
    g_gameInfo.wordStarAccusativePlural = "stars";

    g_gameInfo.creditsFont = 4;

#ifdef CUSTOM_CREDITS_URL
    g_gameInfo.creditsHomePage = CUSTOM_CREDITS_URL;
#else
#   ifdef ENABLE_OLD_CREDITS
    g_gameInfo.creditsHomePage = "www.SuperMarioBrothers.org";
#   else
    g_gameInfo.creditsHomePage = "wohlsoft.ru";
#   endif
#endif

    g_gameInfo.introEnableActivity = true;
    g_gameInfo.introMaxPlayersCount = 6;
    g_gameInfo.introCharacters = {1, 2, 3, 4, 5};
    g_gameInfo.introCharacterCurrent = 0;
    g_gameInfo.introDeadMode = false;

    g_gameInfo.outroEnableActivity = true;
    g_gameInfo.outroMaxPlayersCount = 5;
    g_gameInfo.outroCharacters = {1, 2, 3, 4, 5};
    g_gameInfo.outroStates = {4, 7, 5, 3, 6};
    g_gameInfo.outroMounts = {0, 3, 0, 1, 0};
    g_gameInfo.outroCharacterCurrent = 0;
    g_gameInfo.outroInitialDirections = {0, 0, 0, 0, 0};
    g_gameInfo.outroWalkDirection = -1;
    g_gameInfo.outroAutoJump = true;
    g_gameInfo.outroDeadMode = false;
    g_gameInfo.disableTwoPlayer = false;
    g_gameInfo.disableBattleMode = false;

    std::string gameInfoPath = AppPathManager::assetsRoot() + "gameinfo.ini";
    if(Files::fileExists(gameInfoPath))
    {
        IniProcessing config(gameInfoPath);

        config.beginGroup("game");
        {
            if(config.hasKey("title"))
                config.read("title", g_gameInfo.title, g_gameInfo.title);
            g_gameInfo.titleWindow = fmt::format_ne("{0} - (TheXTech v{1}, #{2})", g_gameInfo.title, V_LATEST_STABLE, V_BUILD_VER);
            config.read("disable-two-player", g_gameInfo.disableTwoPlayer, false);
            config.read("disable-battle-mode", g_gameInfo.disableBattleMode, false);
        }
        config.endGroup();

        if(!config.beginGroup("fails-counter"))
            config.beginGroup("death-counter"); // Backup fallback
        {
            config.read("enabled", gEnableDemoCounter, gEnableDemoCounter);
            config.read("title", gDemoCounterTitleDefault, gDemoCounterTitleDefault);
            gDemoCounterTitle = gDemoCounterTitleDefault;
        }
        config.endGroup();

        config.beginGroup("luna-script");
        {
            config.read("enable-engine", gLunaEnabledGlobally, true);
        }
        config.endGroup();

        // FIXME: make stars name readable after deciding on generic name
        config.beginGroup("objects");
        {
            // config.read("generic-star-leek-single", wordStarAccusativeSingle, "leek");
            // config.read("generic-star-leek-plural", wordStarAccusativePlural, "leeks");
        }
        config.endGroup();

        config.beginGroup("characters");
        {
            for(int i = 1; i <= numCharacters; ++i)
            {
                std::string name = fmt::format_ne("name{0}", i);
                config.read(name.c_str(),
                            g_gameInfo.characterName[i],
                            g_gameInfo.characterName[i]);
            }
        }
        config.endGroup();

        config.beginGroup("intro");
        {
            config.read("enable-activity", g_gameInfo.introEnableActivity, true);
            config.read("max-players-count", g_gameInfo.introMaxPlayersCount, 6);
            config.read("characters", g_gameInfo.introCharacters, {1, 2, 3, 4, 5});
            g_gameInfo.introDeadMode = !g_gameInfo.introEnableActivity || g_gameInfo.introMaxPlayersCount < 1;
        }
        config.endGroup();

        config.beginGroup("outro");
        {
            config.read("enable-activity", g_gameInfo.outroEnableActivity, true);
            config.read("max-players-count", g_gameInfo.outroMaxPlayersCount, 5);
            config.read("characters", g_gameInfo.outroCharacters, {1, 2, 3, 4, 5});
            config.read("states", g_gameInfo.outroStates, {4, 7, 5, 3, 6});
            config.read("mounts", g_gameInfo.outroMounts, {0, 3, 0, 1, 0});
            config.read("auto-jump", g_gameInfo.outroAutoJump, true);
            IniProcessing::StrEnumMap dirs
            {
                {"left", -1},
                {"idle", 0},
                {"right", +1}
            };
            config.readEnum("walk-direction", g_gameInfo.outroWalkDirection, -1, dirs);
            config.read("initial-directions", g_gameInfo.outroInitialDirections, {0, 0, 0, 0, 0});
            g_gameInfo.outroDeadMode = !g_gameInfo.outroEnableActivity || g_gameInfo.outroMaxPlayersCount < 1;
        }
        config.endGroup();

        config.beginGroup("credits");
        {
            config.read("font", g_gameInfo.creditsFont, 4);
            config.read("homepage", g_gameInfo.creditsHomePage, g_gameInfo.creditsHomePage);

            int cr;
            std::string value;
            g_gameInfo.creditsGame.clear();

            for(cr = 1; ; cr++)
            {
                std::string key = fmt::format_ne("game-credit-{0}", cr);
                if(!config.hasKey(key))
                    break;
                config.read(key.c_str(), value, value);
                g_gameInfo.creditsGame.push_back(value);
            }
        }
        config.endGroup();

        readCheats(config, g_gameInfo.cheatsGlobalAliases, "cheats-global-aliases");
        readCheats(config, g_gameInfo.cheatsGlobalRenames, "cheats-global-renames");
        readCheats(config, g_gameInfo.cheatsWorldAliases, "cheats-world-aliases");
        readCheats(config, g_gameInfo.cheatsWorldRenames, "cheats-world-renames");
        readCheats(config, g_gameInfo.cheatsLevelAliases, "cheats-level-aliases");
        readCheats(config, g_gameInfo.cheatsLevelRenames, "cheats-level-renames");
    }
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
