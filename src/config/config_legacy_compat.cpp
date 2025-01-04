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

#include <IniProcessor/ini_processing.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "globals.h"
#include "config.h"

#include "main/menu_main.h"
#include "main/screen_prompt.h"

template<>
void Config_t::LoadLegacyCompat(IniProcessing* ini, ConfigSetLevel level)
{
    ini->beginGroup("compatibility");
    {
        std::string buffer;
        if(ini->hasKey("enable-last-warp-hub-resume"))
        {
            ini->read("enable-last-warp-hub-resume", enable_last_warp_hub_resume.m_value, true);
            enable_last_warp_hub_resume.m_set = level;
        }
    }
    ini->endGroup();

    if(!ini->beginGroup("fails-counter"))
    {
        ini->beginGroup("death-counter"); // Backup fallback
    }
    {
        std::string buffer;
        if(ini->hasKey("enabled"))
        {
            ini->read("enabled", enable_fails_tracking.m_value, true);
            enable_fails_tracking.m_set = level;
        }

        // note: title attribute has been removed, now belongs to gameinfo.ini only.
    }
    ini->endGroup();

    ini->beginGroup("luna-script");
    {
        std::string enable_luna_value;
        ini->read("enable-engine", enable_luna_value, "unspecified");
        if(enable_luna_value != "unspecified")
        {
            luna_enable_engine = (enable_luna_value == "enable" || enable_luna_value == "true");
            luna_enable_engine.m_set = level;
        }

        if(ini->hasKey("allow-level-codes"))
        {
            ini->read("allow-level-codes", luna_allow_level_codes.m_value, false);
            luna_allow_level_codes.m_set = level;
        }
    }
    ini->endGroup();
}
