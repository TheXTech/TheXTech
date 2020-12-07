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
