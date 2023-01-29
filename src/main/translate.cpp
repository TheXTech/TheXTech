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

#include <Logger/logger.h>
#include <AppPath/app_path.h>
#include <Utils/files.h>
#include <json/json.hpp>
#include <fmt_format_ne.h>

#include "globals.h"
#include "translate.h"
#include "menu_main.h"
#include "game_info.h"


static void setJsonValue(nlohmann::json &j, const std::string &key, const std::string &value)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        j[key] = value;
        return;
    }

    std::string subKey = key.substr(0, dot);
    setJsonValue(j[subKey], key.substr(dot + 1), value);
}

static std::string getJsonValue(nlohmann::json &j, const std::string &key, const std::string &defVal)
{
    auto dot = key.find(".");
    if(dot == std::string::npos)
    {
        auto out = j.value(key, defVal);
        return out;
    }

    std::string subKey = key.substr(0, dot);
    return getJsonValue(j[subKey], key.substr(dot + 1), defVal);
}


XTechTranslate::XTechTranslate()
{
    // List of all translatable strings of the engine
    m_translationsMap =
    {
        {"menu.main.mainStartGame",        &g_mainMenu.mainStartGame},
        {"menu.main.main1PlayerGame",      &g_mainMenu.main1PlayerGame},
        {"menu.main.mainStartGame",        &g_mainMenu.mainStartGame},
        {"menu.main.main1PlayerGame",      &g_mainMenu.main1PlayerGame},
        {"menu.main.mainMultiplayerGame",  &g_mainMenu.mainMultiplayerGame},
        {"menu.main.mainBattleGame",       &g_mainMenu.mainBattleGame},
        {"menu.main.mainEditor",           &g_mainMenu.mainEditor},
        {"menu.main.mainOptions",          &g_mainMenu.mainOptions},
        {"menu.main.mainExit",             &g_mainMenu.mainExit},

        {"menu.loading",                   &g_mainMenu.loading},

        {"menu.character.charSelTitle",    &g_mainMenu.charSelTitle},

        {"menu.wordPlayer",                &g_mainMenu.wordPlayer},
        {"menu.wordProfile",               &g_mainMenu.wordProfile},
        {"menu.reconnectTitle",            &g_mainMenu.reconnectTitle},
        {"menu.dropAddTitle",              &g_mainMenu.dropAddTitle},
        {"menu.playerSelStartGame",        &g_mainMenu.playerSelStartGame},
        {"menu.phrasePressAButton",        &g_mainMenu.phrasePressAButton},
        {"menu.phraseTestControls",        &g_mainMenu.phraseTestControls},
        {"menu.wordDisconnect",            &g_mainMenu.wordDisconnect},
        {"menu.phraseHoldStartToReturn",   &g_mainMenu.phraseHoldStartToReturn},
        {"menu.wordBack",                  &g_mainMenu.wordBack},
        {"menu.wordResume",                &g_mainMenu.wordResume},
        {"menu.wordWaiting",               &g_mainMenu.wordWaiting},
        {"menu.phraseForceResume",         &g_mainMenu.phraseForceResume},
        {"menu.phraseDropOthers",          &g_mainMenu.phraseDropOthers},
        {"menu.phraseDropSelf",            &g_mainMenu.phraseDropSelf},
        {"menu.phraseChangeChar",          &g_mainMenu.phraseChangeChar},

        {"menu.controls.controlsTitle",     &g_mainMenu.controlsTitle},
        {"menu.controls.controlsConnected", &g_mainMenu.controlsConnected},
        {"menu.controls.controlsDeleteKey", &g_mainMenu.controlsDeleteKey},
        {"menu.controls.controlsDeviceTypes", &g_mainMenu.controlsDeviceTypes},
        {"menu.controls.controlsInUse", &g_mainMenu.controlsInUse},
        {"menu.controls.controlsNotInUse", &g_mainMenu.controlsNotInUse},
        {"menu.controls.wordProfiles", &g_mainMenu.wordProfiles},
        {"menu.controls.wordButtons", &g_mainMenu.wordButtons},

        {"menu.controls.controlsReallyDeleteProfile", &g_mainMenu.controlsReallyDeleteProfile},
        {"menu.controls.controlsNewProfile", &g_mainMenu.controlsNewProfile},

        {"menu.wordNo", &g_mainMenu.wordNo},
        {"menu.wordYes", &g_mainMenu.wordYes},
    };

    for(int i = 1; i <= numCharacters; ++i)
    {
        m_translationsMap.insert({fmt::format_ne("charcter.name{0}", i), &g_gameInfo.characterName[i]});
        m_translationsMap.insert({fmt::format_ne("menu.player.selectPlayer{0}", i), &g_mainMenu.selectPlayer[i]});
    }
}

void XTechTranslate::reset()
{
    initMainMenu();
}

void XTechTranslate::exportTemplate()
{
    reset();

    try
    {
        nlohmann::json langFile;

        for(auto &k : m_translationsMap)
        {
            std::printf("-- writing %s -> %s\n", k.first.c_str(), k.second->c_str());
            setJsonValue(langFile, k.first, *k.second);
        }

        std::printf("Lang file data: \n\n%s\n\n", langFile.dump(4, ' ', false).c_str());
        std::fflush(stdout);
    }
    catch(const std::exception &e)
    {
        std::printf("JSON: Caught an exception: %s", e.what());
        std::fflush(stdout);
    }
}

bool XTechTranslate::translate()
{
    std::string langFilePath = AppPathManager::languagesDir() + fmt::format_ne("thextech_{0}.json", CurrentLanguage.c_str());
    if(!Files::fileExists(langFilePath))
        return false; // File is not exists, do nothing

    try
    {
        off_t end;
        std::string data;
        FILE *in = Files::utf8_fopen(langFilePath.c_str(), "r");
        if(!in)
        {
            pLogWarning("Failed to load the translation file %s: can't open file", langFilePath.c_str());
            return false;
        }

        std::fseek(in, 0, SEEK_END);
        end = std::ftell(in);
        std::fseek(in, 0, SEEK_SET);
        data.resize(end);
        std::fread((void*)data.data(), 1, data.size(), in);
        std::fclose(in);

        nlohmann::json langFile = nlohmann::json::parse(data);

        for(auto &k : m_translationsMap)
        {
            std::string &res = *k.second;
            res = getJsonValue(langFile, k.first, *k.second);
        }
    }
    catch(const std::exception &e)
    {
        reset();
        pLogWarning("Failed to load the translation file %s: %s", langFilePath.c_str(), e.what());
        return false;
    }

    return true;
}
