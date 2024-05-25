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

#include <IniProcessor/ini_processing.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "globals.h"
#include "config.h"

#include "main/menu_main.h"
#include "main/screen_prompt.h"

// support variables for autoconversion to avoid repeated nags
static int s_cur_load_iter = 0;
static std::unordered_map<std::string, int> s_first_load_iter;

static void deprecatedWarning(IniProcessing &s, const char* fieldName, const char *newName)
{
    if(s.hasKey(fieldName) && !s.hasKey(newName))
    {
        pLogWarning("File %s contains the deprecated setting \"%s\" at the section [%s]. Please rename it into \"%s\".",
                    s.fileName().c_str(),
                    fieldName,
                    s.group().c_str(),
                    newName);

        bool writable = (selWorld == 0 || LevelEditor) || (BattleMode ? SelectBattle : SelectWorld)[selWorld].editable;

        if(writable)
        {
            int response;

            // only warn on first time the file is loaded
            if(s_first_load_iter[s.fileName()] != 0 && s_first_load_iter[s.fileName()] != s_cur_load_iter)
            {
                response = 1;
            }
            else
            {
                response = PromptScreen::Run(
                    fmt::format_ne(g_mainMenu.promptDeprecatedSetting, fieldName, newName),
                    {
                        g_mainMenu.wordYes,
                        g_mainMenu.wordNo,
                    }
                );

                s_first_load_iter[s.fileName()] = s_cur_load_iter;
            }

            if(response == 0)
            {
                pLogDebug("Updating file [%s]...", s.fileName().c_str());

                std::string current_value;
                s.read(fieldName, current_value, current_value);
                s.setValue(newName, current_value);

                writable = s.writeIniFile();
            }
        }

        // NOTE: if saving failed above, writable is set to false even if it was originally true

        // final condition ensures we only warn on first time the file is loaded
        if(!writable && (s_first_load_iter[s.fileName()] == 0 || s_first_load_iter[s.fileName()] == s_cur_load_iter))
        {
            std::string filename = s.fileName();

            // remove AppPath if it's included
            pLogDebug("%s %s", filename.c_str(), AppPath.c_str());
            if(filename.compare(0, AppPath.size(), AppPath) == 0)
            {
                for(size_t i = AppPath.size(); i < filename.size(); i++)
                    filename[i - AppPath.size()] = filename[i];

                filename.resize(filename.size() - AppPath.size());
            }

            PromptScreen::Run(fmt::format_ne(g_mainMenu.promptDeprecatedSettingUnwritable, filename, s.group().c_str(), fieldName, newName), {g_mainMenu.wordOkay});

            s_first_load_iter[s.fileName()] = s_cur_load_iter;
        }
    }
}

template<>
void Config_t::LoadLegacyCompat(IniProcessing* ini, ConfigSetLevel level)
{
    if(!ini->beginGroup("fails-counter"))
        ini->beginGroup("death-counter");

    if(ini->hasKey("enabled"))
    {
        bool val;
        ini->read("enabled", val, false);

        enable_fails_tracking = val;
        enable_fails_tracking.m_set = level;

        show_fails_counter = val;
        show_fails_counter.m_set = level;
    }
    ini->endGroup();

    ini->beginGroup("speedrun");
    {
        const IniProcessing::StrEnumMap stopBy
        {
            {"none", Config_t::SPEEDRUN_STOP_NONE},
            {"event", Config_t::SPEEDRUN_STOP_EVENT},
            {"leave", Config_t::SPEEDRUN_STOP_LEAVE_LEVEL},
            {"enter", Config_t::SPEEDRUN_STOP_ENTER_LEVEL},
            {"unspecified", -1},
        };

        int val;
        ini->readEnum("stop-timer-by", val, -1, stopBy);
        if(val != -1)
        {
            speedrun_stop_timer_by = val;
            speedrun_stop_timer_by.m_set = level;
        }

        bool set_timer = false;
        bool semi_transparent_timer = false;

        if(ini->hasKey("semi-transparent-timer"))
        {
            ini->read("semi-transparent-timer", semi_transparent_timer, false);
            set_timer = true;
        }

        const IniProcessing::StrEnumMap speedRunBlinkMode =
        {
            {"opaque", !semi_transparent_timer},
            {"always", 1},
            {"true", 1},
            {"never", 0},
            {"false", 0},
            {"unspecified", -1},
        };

        ini->readEnum("blink-effect", val, -1, speedRunBlinkMode);
        if(val != -1 || set_timer)
        {
            if(val == 0 && semi_transparent_timer)
                show_playtime_counter = Config_t::PLAYTIME_COUNTER_SUBTLE;
            else if(val == 0)
                show_playtime_counter = Config_t::PLAYTIME_COUNTER_OPAQUE;
            else
                show_playtime_counter = Config_t::PLAYTIME_COUNTER_ANIMATED;

            show_playtime_counter.m_set = level;
        }

        if(ini->hasKey("stop-timer-at"))
        {
            std::string buffer;
            ini->read("stop-timer-at", buffer, "<<unspecified>>");
            if(buffer != "<<unspecified>>")
            {
                speedrun_stop_timer_at = buffer;
                speedrun_stop_timer_at.m_set = level;
            }
        }
    }
    ini->endGroup();

    s_cur_load_iter++;
}
