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

#include "../globals.h"
#include "../game_main.h"
#include "../graphics.h"
#include "../sound.h"
#include "../config.h"
#include "../controls.h"

#include "core/render.h"

#include "speedrunner.h"
#include "record.h"
#include "main/asset_pack.h"

#include <Utils/files.h>
#include <Utils/strings.h>
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>


void ConfigReloadRecentEpisodes()
{
    // reload recently used episodes for the new asset pack
    std::string configPath = AppPathManager::settingsFileSTD();

    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);

        std::string asset_pack_prefix = g_AssetPackID;
        if(!asset_pack_prefix.empty())
            asset_pack_prefix += '-';

        config.beginGroup("recent");
        config.read((asset_pack_prefix + "episode-1p").c_str(), g_recentWorld1p, std::string());
        config.read((asset_pack_prefix + "episode-2p").c_str(), g_recentWorld2p, std::string());
        config.read((asset_pack_prefix + "episode-editor").c_str(), g_recentWorldEditor, std::string());
        config.endGroup();

        pLogDebug("Loaded recent episodes for asset pack [id: %s] from [%s]", g_AssetPackID.c_str(), configPath.c_str());
    }
}


void OpenConfig()
{
    g_pLogGlobalSetup.logPathDefault = AppPathManager::logsDir();
    g_pLogGlobalSetup.logPathFallBack = AppPathManager::userAppDirSTD();

    int FileRelease = 0;

    std::string configPath = AppPathManager::settingsFileSTD();

    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);

        config.beginGroup("main");
        config.read("release", FileRelease, curRelease);
        bool default_set = config.hasKey("default-set");
        config.endGroup();

        std::string asset_pack_prefix = g_AssetPackID;
        if(!asset_pack_prefix.empty())
            asset_pack_prefix += '-';

        config.beginGroup("recent");
        config.read("asset-pack", g_recentAssetPack, std::string());
        config.read((asset_pack_prefix + "episode-1p").c_str(), g_recentWorld1p, std::string());
        config.read((asset_pack_prefix + "episode-2p").c_str(), g_recentWorld2p, std::string());
        config.read((asset_pack_prefix + "episode-editor").c_str(), g_recentWorldEditor, std::string());
        config.endGroup();

        config.beginGroup("logging");
        config.read("log-path", g_pLogGlobalSetup.logPathCustom, std::string());
        config.read("max-log-count", g_pLogGlobalSetup.maxFilesCount, 10);
        config.endGroup();

        g_config_game_user.Clear();
        if(!default_set)
            g_config_game_user.LoadLegacySettings(&config);
        g_config_game_user.UpdateFromIni(&config);

        g_config.log_level.set_from_default(ConfigSetLevel::set);

        g_pLogGlobalSetup.level = (PGE_LogLevel::Level)(int)(g_config_game_user.log_level.is_set() ? g_config_game_user.log_level : g_config.log_level);

        pLogDebug("Loaded config: %s", configPath.c_str());
    }
    else
    {
        pLogDebug("Writing new config on first run.");
        SaveConfig(); // Create the config file on first run
    }

}

void SaveConfig()
{
    std::string configPath = AppPathManager::settingsFileSTD();

    IniProcessing config(configPath);
    config.beginGroup("main");
    config.setValue("release", curRelease);
    config.endGroup();

    config.beginGroup("logging");
    {
        if(!g_pLogGlobalSetup.logPathCustom.empty())
            config.setValue("log-path", g_pLogGlobalSetup.logPathCustom);
        config.setValue("max-log-count", g_pLogGlobalSetup.maxFilesCount);
    }
    config.endGroup();

    std::string asset_pack_prefix = g_AssetPackID;
    if(!asset_pack_prefix.empty())
        asset_pack_prefix += '-';

    config.beginGroup("recent");
    config.setValue("asset-pack", g_AssetPackID);
    config.setValue((asset_pack_prefix + "episode-1p").c_str(), g_recentWorld1p);
    config.setValue((asset_pack_prefix + "episode-2p").c_str(), g_recentWorld2p);
    config.setValue((asset_pack_prefix + "episode-editor").c_str(), g_recentWorldEditor);
    config.endGroup();

    g_config_game_user.SaveToIni(&config);

    config.writeIniFile();
    AppPathManager::syncFs();

    pLogDebug("Saved config: %s", configPath.c_str());
}
