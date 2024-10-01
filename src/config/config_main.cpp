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

#include <PGE_File_Formats/file_formats.h>
#include <IniProcessor/ini_processing.h>
#include <Utils/files_ini.h>
#include <fmt_format_ne.h>

#include "config.h"
#include "config/config_impl.hpp"
#include "main/game_info.h"
#include "globals.h"
#include "global_dirs.h"
#include "main/menu_main.h"
#include "sound.h"

static int s_backup_bugfixes = Config_t::MODE_MODERN;
static int s_backup_creator_compat = Config_t::CREATORCOMPAT_ENABLE;

template<>
void Config_t::Clear()
{
    // only update backup variables for main config class
    if(this == &g_config)
    {
        if(playstyle.m_set == ConfigSetLevel::ep_config)
            s_backup_bugfixes = playstyle;

        if(GameMenu)
            s_backup_creator_compat = Config_t::CREATORCOMPAT_ENABLE;
        else
        {
            if(creator_compat.m_set == ConfigSetLevel::ep_config)
                s_backup_creator_compat = creator_compat;
        }
    }

    for(BaseConfigOption_t<true>* option : m_options)
    {
        // allow speedrun mode to be kept throughout a menu quit
        if(option == &speedrun_mode)
            continue;

        switch(option->m_set)
        {
        case ConfigSetLevel::cmdline:
        case ConfigSetLevel::debug:
            // allow these levels to always be kept
            break;
        case ConfigSetLevel::ep_config:
        case ConfigSetLevel::cheat:
            // allow these levels to be kept for the duration of a run
            if(!GameMenu)
                break;
            // fallthrough
        default:
            option->unset();
        }
    }

    // restore episode config
    playstyle.m_value = s_backup_bugfixes;
    playstyle.m_set = ConfigSetLevel::ep_config;

    creator_compat.m_value = s_backup_creator_compat;
    creator_compat.m_set = ConfigSetLevel::ep_config;
}

template<>
void Config_t::UpdateFromIni(IniProcessing* ini, ConfigSetLevel level)
{
    for(BaseConfigOption_t<true>* option : m_options)
        option->update_from_ini(ini, level);
}

template<>
void Config_t::SaveToIni(IniProcessing* ini)
{
    for(BaseConfigOption_t<true>* option : m_options)
        option->save_to_ini(ini);
}

template<>
void Config_t::SaveEpisodeConfig(saveUserData& userdata)
{
    saveUserData::DataSection ep_config;
    ep_config.name = "_xt_ep_conf";
    ep_config.location = saveUserData::DATA_GLOBAL;

    if(speedrun_mode.m_set >= ConfigSetLevel::ep_config)
        ep_config.data.emplace_back(saveUserData::DataEntry{"speedrun-mode", std::to_string(speedrun_mode.m_value)});

    int playstyle_value = (playstyle.m_set == ConfigSetLevel::ep_config) ? playstyle : s_backup_bugfixes;
    ep_config.data.emplace_back(saveUserData::DataEntry{"playstyle", (playstyle_value == MODE_VANILLA) ? "vanilla" : ((playstyle_value == MODE_CLASSIC) ? "classic" : "modern")});

    int creator_compat_value = (creator_compat.m_set == ConfigSetLevel::ep_config) ? creator_compat : s_backup_creator_compat;
    ep_config.data.emplace_back(saveUserData::DataEntry{"creator-compat", (creator_compat_value == CREATORCOMPAT_DISABLE) ? "disable" : ((creator_compat_value == CREATORCOMPAT_FILEONLY) ? "file-only" : "enable")});

    userdata.store.push_back(ep_config);
}

template<>
void Config_t::LoadEpisodeConfig(const saveUserData& userdata)
{
    for(const auto& section : userdata.store)
    {
        if(section.name != "_xt_ep_conf" || section.location != saveUserData::DATA_GLOBAL)
            continue;

        for(const auto& entry : section.data)
        {
            if(entry.key == "speedrun-mode")
            {
                if(speedrun_mode.m_set <= ConfigSetLevel::ep_config)
                {
                    speedrun_mode.m_value = std::atoi(entry.value.c_str());
                    if(speedrun_mode.m_value < 0 || speedrun_mode.m_value > 4)
                        speedrun_mode.m_value = 0;

                    speedrun_mode.m_set = ConfigSetLevel::ep_config;
                }
            }
            // TODO: remove this legacy clause
            else if(entry.key == "enable-bugfixes")
            {
                if(entry.value == "none")
                    playstyle.m_value = MODE_VANILLA;
                else if(entry.value == "critical")
                    playstyle.m_value = MODE_CLASSIC;
                else
                    playstyle.m_value = MODE_MODERN;

                playstyle.m_set = ConfigSetLevel::ep_config;
            }
            else if(entry.key == "playstyle")
            {
                if(entry.value == "vanilla")
                    playstyle.m_value = MODE_VANILLA;
                else if(entry.value == "classic")
                    playstyle.m_value = MODE_CLASSIC;
                else
                    playstyle.m_value = MODE_MODERN;

                playstyle.m_set = ConfigSetLevel::ep_config;
            }
            else if(entry.key == "creator-compat")
            {
                if(entry.value == "disable")
                    creator_compat.m_value = CREATORCOMPAT_DISABLE;
                else if(entry.value == "file-only")
                    creator_compat.m_value = CREATORCOMPAT_FILEONLY;
                else
                    creator_compat.m_value = CREATORCOMPAT_ENABLE;

                creator_compat.m_set = ConfigSetLevel::ep_config;
            }
        }

        break;
    }
}

template<>
void Config_t::SetFromDefaults(ConfigSetLevel level)
{
    for(BaseConfigOption_t<true>* option : m_options)
        option->set_from_default(level);
}

template<>
void Config_t::DisableBugfixes(ConfigSetLevel level)
{
    for(BaseConfigOption_t<true>* option : m_options)
        option->disable_bugfixes(level);
}

template<>
void Config_t::UpdateFrom(const Config_t& o, ConfigSetLevel level)
{
    size_t i = 0;
    for(BaseConfigOption_t<true>* option : m_options)
    {
        if(i >= o.m_options.size())
            break;

        if(m_base->m_options[i] && (m_base->m_options[i]->m_scope & o.m_scope) && o.m_options[i])
            option->update_from(*o.m_options[i], level);

        i++;
    }
}

template<>
void Config_t::UpdateFromCompat(CompatMode compat_mode, ConfigSetLevel level)
{
    for(BaseConfigOption_t<true>* option : m_options)
        option->update_from_compat(compat_mode, level);
}

template<>
void Options_t::reset_options()
{
    // invoke destructor
    this->~Options_t();

    // invoke constructor
    new(this) Options_t();
}

template<>
void Options_t::make_translation(XTechTranslate& translate)
{
    const char* cur_section_id = "";

    for(BaseConfigOption_t<false>* option : m_options)
    {
        bool is_section = dynamic_cast<ConfigSection_t<false>*>(option);

        // switch to the section's header
        if(is_section)
            cur_section_id = option->m_internal_name;

        option->make_translation(translate, cur_section_id);
    }
}

void LoadCustomConfig()
{
    std::string episodeCompat, customCompat, episodeConfig, customConfig, userEpConfig;

    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);

    g_config_episode_creator.Clear();
    g_config_file_creator.Clear();

    // Episode-wide custom creator setup
    episodeCompat = g_dirEpisode.resolveFileCaseExistsAbs("compat.ini");
    // Level-wide custom creator setup
    customCompat = g_dirCustom.resolveFileCaseExistsAbs("compat.ini");

    if(!episodeCompat.empty())
    {
        IniProcessing ini = Files::load_ini(episodeCompat);
        g_config_episode_creator.LoadLegacyCompat(&ini, ConfigSetLevel::ep_compat);
        g_config_episode_creator.UpdateFromIni(&ini, ConfigSetLevel::ep_compat);
    }

    if(!customCompat.empty())
    {
        IniProcessing ini = Files::load_ini(customCompat);
        g_config_file_creator.LoadLegacyCompat(&ini, ConfigSetLevel::file_compat);
        g_config_file_creator.UpdateFromIni(&ini, ConfigSetLevel::file_compat);
    }

    UpdateConfig();
}

void ResetCustomConfig()
{
    g_config_episode_creator.Clear();
    g_config_file_creator.Clear();

    UpdateConfig();
}

void UpdateConfig()
{
    // do not allow UpdateConfig to call itself recursively
    static Config_t g_config_backup(g_options, Config_t::Scope::All);
    static bool extra_request = false;

    if(g_configInTransaction)
    {
        extra_request = true;
        return;
    }

    // initialize transaction
    g_configInTransaction = true;

    g_config_backup.Clear();
    g_config_backup.UpdateFrom(g_config);

    // start filling in the config
    g_config.Clear();
    g_config.SetFromDefaults(ConfigSetLevel::game_defaults);

    // scoping for a sentinel for audio defaults
    {
        ConfigChangeSentinel sent(ConfigSetLevel::game_defaults);

#ifndef THEXTECH_NO_SDL_BUILD
        g_config.audio_channels = g_audioDefaults.channels;
        g_config.audio_sample_rate = g_audioDefaults.sampleRate;
        g_config.audio_buffer_size = g_audioDefaults.bufferSize;
        g_config.audio_format = g_audioDefaults.format;
#endif
    }

    // load from all levels of config
    g_config.UpdateFrom(g_config_game_user, ConfigSetLevel::user_config);

    g_config.DisableBugfixes(ConfigSetLevel::bugfix_defaults);

    if(g_config.creator_compat == Config_t::CREATORCOMPAT_ENABLE)
        g_config.UpdateFrom(g_config_episode_creator, ConfigSetLevel::ep_compat);

    if(g_config.creator_compat != Config_t::CREATORCOMPAT_DISABLE)
        g_config.UpdateFrom(g_config_file_creator, ConfigSetLevel::file_compat);

    // also do the speedrun, including compat...
    if(g_config.speedrun_mode >= 1) // Always show FPS and don't pause the game work when focusing other windows
    {
        ConfigChangeSentinel sent(ConfigSetLevel::speedrun);

#ifndef NO_WINDOW_FOCUS_TRACKING
        g_config.background_work = true;
#endif
        g_config.show_fps = true;
        g_config.enable_frameskip = false;
        if(g_config.speedrun_mode == 1)
            g_config.compatibility_mode = Config_t::COMPAT_MODERN;
        else
            g_config.compatibility_mode = g_config.speedrun_mode - 1;
        g_config.enable_playtime_tracking = true;
    }

    if(g_config.compatibility_mode)
    {
        ConfigChangeSentinel sent(ConfigSetLevel::compat);

        CompatMode compat_mode = CompatMode::off;

        if(g_config.compatibility_mode == Config_t::COMPAT_SMBX13)
        {
            compat_mode = CompatMode::smbx64;
            g_config.playstyle = Config_t::MODE_VANILLA;
        }
        else if(g_config.compatibility_mode == Config_t::COMPAT_CLASSIC)
        {
            compat_mode = CompatMode::classic;
            g_config.playstyle = Config_t::MODE_CLASSIC;
        }
        else if(g_config.compatibility_mode == Config_t::COMPAT_MODERN)
        {
            compat_mode = CompatMode::modern;
            g_config.playstyle = Config_t::MODE_MODERN;
        }

        g_config.UpdateFromCompat(compat_mode, ConfigSetLevel::compat);
    }

    for(const BaseConfigOption_t<true>* changed_opt : g_configModified)
    {
        size_t index = std::find(g_config.m_options.begin(), g_config.m_options.end(), changed_opt) - g_config.m_options.begin();
        if(index >= g_config_backup.m_options.size())
            continue;

        if(!(*changed_opt == *g_config_backup.m_options[index]))
            changed_opt->m_base->m_onupdate();
    }

    g_configModified.clear();
    g_configInTransaction = false;

    if(extra_request)
    {
        extra_request = 0;
        UpdateConfig();
    }
}

bool g_configInTransaction = false;
std::set<BaseConfigOption_t<true>*> g_configModified;

Options_t g_options;

// <defaults>                                                                     1
Config_t g_config_game_user(g_options, Config_t::Scope::Config);               // 3
Config_t g_config_episode_creator(g_options, Config_t::Scope::CreatorEpisode); // 4
Config_t g_config_file_creator(g_options, Config_t::Scope::CreatorFile);       // 6
// <compat>                                                                       9

// const std::vector<const Config_t*> g_config_levels =
// {
//     &g_gameInfo, &g_config_game_user,
//     &g_config_episode_creator, &g_config_episode_user,
//     &g_config_file_creator, &g_config_file_user,
//     &g_config_cmdline
// };

Config_t g_config(g_options, Config_t::Scope::All);
