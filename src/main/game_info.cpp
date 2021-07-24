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

#include <fmt_format_ne.h>
#include <Utils/files.h>
#include <AppPath/app_path.h>
#include <IniProcessor/ini_processing.h>
#include "game_info.h"

#include "../version.h"


GameInfo g_gameInfo;

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
    g_gameInfo.title = "The X-Tech Engine";
    g_gameInfo.titleWindow = fmt::format_ne("X-Tech v{0}", V_LATEST_STABLE);
#   endif
#endif /* CUSTOM_GAME_NAME_TITLE */

    g_gameInfo.characterName[1] = "Mario";
    g_gameInfo.characterName[2] = "Luigi";
    g_gameInfo.characterName[3] = "Peach";
    g_gameInfo.characterName[4] = "Toad";
    g_gameInfo.characterName[5] = "Link";

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

    std::string gameInfoPath = AppPathManager::assetsRoot() + "gameinfo.ini";
    if(Files::fileExists(gameInfoPath))
    {
        IniProcessing config(gameInfoPath);

        config.beginGroup("game");
        {
            if(config.hasKey("title"))
                config.read("title", g_gameInfo.title, g_gameInfo.title);
            g_gameInfo.titleWindow = fmt::format_ne("{0} - (X-Tech v{1})", g_gameInfo.title, V_LATEST_STABLE);
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
    }
}
