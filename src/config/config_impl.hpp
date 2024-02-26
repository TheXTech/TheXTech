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

// included in config_main.cpp

#pragma once
#ifndef CONFIG_IMPL_HPP
#define CONFIG_IMPL_HPP

#include <set>

#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>

#include "config/config_base.hpp"
#include "main/screen_textentry.h"
#include "game_main.h"

#include "core/language.h"
#include "fontman/font_manager.h"

#include "main/translate.h"

// config_base.cpp
extern ConfigSetLevel g_configWriteLevel;

// config_main.cpp
extern bool g_configInTransaction;
extern std::set<BaseConfigOption_t<true>*> g_configModified;

// implementation for ConfigOption_t<true, value_t>

template<class value_t>
bool ConfigOption_t<true, value_t>::operator==(const BaseConfigOption_t<true>& o) const
{
    const ConfigOption_t<true, value_t>* other = dynamic_cast<const ConfigOption_t<true, value_t>*>(&o);

    if(!other)
        return false;

    if(!is_set() || !other->is_set())
        return false;

    return this->m_value == other->m_value;
}

template<class value_t>
bool ConfigOption_t<true, value_t>::rotate_left()
{
    return BaseConfigOption_t<true>::rotate_left();
}

template<class value_t>
bool ConfigOption_t<true, value_t>::rotate_right()
{
    return BaseConfigOption_t<true>::rotate_right();
}

template<class value_t>
bool ConfigOption_t<true, value_t>::change()
{
    return BaseConfigOption_t<true>::change();
}

template<class value_t>
const ConfigOption_t<true, value_t>& ConfigOption_t<true, value_t>::operator=(value_t value)
{
    if(g_configWriteLevel < m_set)
        return *this;

    m_set = g_configWriteLevel;

    if(m_value != value)
    {
        m_value = value;
        _on_change();
    }

    return *this;
}

// force the compiler to instantiate these
template const ConfigOption_t<true, bool>& ConfigOption_t<true, bool>::operator=(bool value);
template const ConfigOption_t<true, int>& ConfigOption_t<true, int>::operator=(int value);
template const ConfigOption_t<true, std::pair<int, int>>& ConfigOption_t<true, std::pair<int, int>>::operator=(std::pair<int, int> value);
template const ConfigOption_t<true, std::string>& ConfigOption_t<true, std::string>::operator=(std::string value);

template<class value_t>
void ConfigOption_t<true, value_t>::set_from_default(ConfigSetLevel level)
{
    if(level < m_set)
        return;

    if(!m_base)
        return;

    const ConfigOption_t<false, value_t>* base = dynamic_cast<const ConfigOption_t<false, value_t>*>(m_base);

    if(!base)
        return;

    *this = base->m_default_value;

    m_set = level;
}

template<class value_t>
void ConfigOption_t<true, value_t>::disable_bugfixes(ConfigSetLevel level)
{
    if(level < m_set)
        return;

    if(!m_base)
        return;

    const ConfigOption_t<false, value_t>* base = dynamic_cast<const ConfigOption_t<false, value_t>*>(m_base);

    if(!base)
        return;

    if(g_config.enable_bugfixes == Config_t::BUGFIXES_ALL)
        return;

    if(base->m_compat_info.mode == CompatClass::pure_preference)
        return;

    if(g_config.enable_bugfixes != Config_t::BUGFIXES_NONE && (base->m_compat_info.mode == CompatClass::critical_update || base->m_compat_info.mode == CompatClass::gameplay_enhancement))
        return;

    *this = base->m_compat_info.prev_value;

    m_set = level;
}

template<class value_t>
void ConfigOption_t<true, value_t>::update_from_ini(IniProcessing* ini, ConfigSetLevel level)
{
    if(level < m_set)
        return;

    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(!ini->hasKey(m_base->m_internal_name))
        return;

    value_t read_value{};
    ini->read(m_base->m_internal_name, read_value, read_value);

    *this = read_value;
    m_set = level;
}

template<class value_t>
void ConfigOption_t<true, value_t>::update_from_compat(CompatMode compat_mode, ConfigSetLevel level)
{
    if(level < m_set)
        return;

    if(!m_base)
        return;

    const ConfigOption_t<false, value_t>* base = dynamic_cast<const ConfigOption_t<false, value_t>*>(m_base);

    if(!base)
        return;

    if(base->m_compat_info.mode == CompatClass::pure_preference)
        return;

    bool use_modern = (base->m_compat_info.mode == CompatClass::critical_update && compat_mode < CompatMode::smbx64)
        || (base->m_compat_info.mode == CompatClass::gameplay_enhancement && compat_mode < CompatMode::smbx64)
        || (base->m_compat_info.mode == CompatClass::standard_bugfix && compat_mode < CompatMode::classic);

    if(use_modern)
        *this = base->m_default_value;
    else
        *this = base->m_compat_info.prev_value;

    m_set = level;
}

template<class value_t>
void ConfigOption_t<true, value_t>::update_from(const BaseConfigOption_t<true>& o, ConfigSetLevel level)
{
    if(level < m_set)
        return;

    const ConfigOption_t<true, value_t>* o_conv = dynamic_cast<const ConfigOption_t<true, value_t>*>(&o);

    if(!o_conv)
        return;

    if(!o_conv->is_set())
        return;

    *this = (value_t)*o_conv;
    m_set = level;
}

template<class value_t>
void ConfigOption_t<true, value_t>::save_to_ini(IniProcessing* ini)
{
    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(is_set())
        ini->setValue(m_base->m_internal_name, m_value);
    else
        ini->clearValue(m_base->m_internal_name);
}

template<class value_t>
const std::string& ConfigOption_t<true, value_t>::get_display_value(std::string& out) const
{
    out = std::to_string(m_value);
    return out;
}

// specializer for strings
template<>
bool ConfigOption_t<true, std::string>::change()
{
    if(!m_base)
        return false;

    m_value = TextEntryScreen::Run(m_base->m_display_name, m_value);

    if(!is_set())
        m_set = ConfigSetLevel::set;

    _on_change();

    return true;
}


template<>
const std::string& ConfigOption_t<true, std::string>::get_display_value(std::string& out) const
{
    if(m_value.empty())
        return (out = "<NONE>");

    out = m_value;
    return out;
}

// specializer for resolution
template<>
const std::string& ConfigOption_t<true, std::pair<int, int>>::get_display_value(std::string& out) const
{
    out = fmt::format_ne("{0}x{1} (CUSTOM)", m_value.first, m_value.second);
    return out;
}

template<>
void ConfigOption_t<true, std::pair<int, int>>::update_from_ini(IniProcessing* ini, ConfigSetLevel level)
{
    if(level < m_set)
        return;

    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(!ini->hasKey(m_base->m_internal_name))
        return;

    std::string read_value;
    ini->read(m_base->m_internal_name, read_value, read_value);

    int arg1, h;
    int argcount = sscanf(read_value.c_str(), "%dx%d", &arg1, &h);

    if(argcount < 1)
        return;

    if(argcount == 2)
        *this = {0, arg1};
    else
        *this = {arg1, h};

    m_set = level;
}

template<>
void ConfigOption_t<true, std::pair<int, int>>::save_to_ini(IniProcessing* ini)
{
    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(is_set())
    {
        std::string out = fmt::format_ne("{0}x{1}", m_value.first, m_value.second);
        ini->setValue(m_base->m_internal_name, out);
    }
    else
        ini->clearValue(m_base->m_internal_name);
}

// specializer for the bitblit triple
template<>
const std::string& ConfigOption_t<true, std::array<uint8_t, 3>>::get_display_value(std::string& out) const
{
    out = fmt::format_ne("{0},{1},{2}", m_value[0], m_value[1], m_value[2]);
    return out;
}

template<>
void ConfigOption_t<true, std::array<uint8_t, 3>>::update_from_ini(IniProcessing* ini, ConfigSetLevel level)
{
    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(!ini->hasKey(m_base->m_internal_name))
        return;

    *this = {0, 0, 0};

    m_set = level;
}

template<>
void ConfigOption_t<true, std::array<uint8_t, 3>>::save_to_ini(IniProcessing* ini)
{
    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(is_set())
    {
        ini->setValue(m_base->m_internal_name, "unimplemented");
    }
    else
        ini->clearValue(m_base->m_internal_name);
}

// specialized implementation for ConfigOption_t<true, bool>

template<>
bool ConfigOption_t<true, bool>::rotate_left()
{
    m_value = !m_value;

    if(!is_set())
        m_set = ConfigSetLevel::set;

    _on_change();

    return true;
}

template<>
bool ConfigOption_t<true, bool>::rotate_right()
{
    return rotate_left();
}

template<>
const std::string& ConfigOption_t<true, bool>::get_display_value(std::string& out) const
{
    if(FontManager::hasTtfSupport())
        out = m_value ? "✓" : "×";
    else
        out = m_value ? "+" : "-";

    return out;
}


// implementation for ConfigEnumOption_t<false, value_t>

template<class value_t>
void ConfigEnumOption_t<false, value_t>::make_translation(XTechTranslate& translate, const char* cur_section_id)
{
    translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}.{1}._name", cur_section_id, m_internal_name), &m_display_name});

    // some obscure behavior of the GCC version in Ubuntu 16.04 necessitates qualifying m_display_tooltip with the base class scope
    if(!BaseConfigOption_t<false>::m_display_tooltip.empty())
        translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}.{1}._tooltip", cur_section_id, m_internal_name), &m_display_tooltip});

    for(auto& value : m_enum_values)
    {
        if(!value.m_display_name.empty())
            translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}.{1}.{2}", cur_section_id, m_internal_name, value.m_internal_name), &value.m_display_name});

        if(!value.m_display_tooltip.empty())
            translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}.{1}.{2}-tip", cur_section_id, m_internal_name, value.m_internal_name), &value.m_display_tooltip});
    }
}


// implementation for ConfigEnumOption_t<true, value_t>

template<class value_t>
void ConfigEnumOption_t<true, value_t>::update_from_ini(IniProcessing* ini, ConfigSetLevel level)
{
    if(level < m_set)
        return;

    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    const ConfigEnumOption_t<false, value_t>* base = dynamic_cast<const ConfigEnumOption_t<false, value_t>*>(m_base);

    if(!base)
        return;

    if(!ini->hasKey(base->m_internal_name))
    {
        return;
    }

    std::string key_name;
    ini->read(base->m_internal_name, key_name, key_name);

    for(const ConfigEnumValueInfo_t<value_t>& val : base->m_enum_values)
    {
        if(!strcmp(key_name.c_str(), val.m_internal_name))
        {
            ConfigOption_t<true, value_t>::operator=(val.m_value);
            m_set = level;
            return;
        }
    }

    ConfigOption_t<true, value_t>::update_from_ini(ini, level);
}

template<class value_t>
void ConfigEnumOption_t<true, value_t>::save_to_ini(IniProcessing* ini)
{
    if(!ini || !m_base || !m_base->m_internal_name)
        return;

    if(!is_set())
    {
        ini->clearValue(m_base->m_internal_name);
        return;
    }

    const ConfigEnumOption_t<false, value_t>* base = dynamic_cast<const ConfigEnumOption_t<false, value_t>*>(m_base);

    if(!base)
        return;

    for(const ConfigEnumValueInfo_t<value_t>& val : base->m_enum_values)
    {
        if((value_t)*this == val.m_value)
        {
            ini->setValue(base->m_internal_name, val.m_internal_name);
            return;
        }
    }

    ConfigOption_t<true, value_t>::save_to_ini(ini);
}

template<class value_t>
bool ConfigEnumOption_t<true, value_t>::rotate_left()
{
    const ConfigEnumOption_t<false, value_t>* base = dynamic_cast<const ConfigEnumOption_t<false, value_t>*>(m_base);

    if(!base || base->m_enum_values.size() > 255)
        return false;

    uint8_t found_i = 0;

    for(uint8_t i = 0; i < base->m_enum_values.size(); i++)
    {
        if((value_t)*this == base->m_enum_values[i].m_value)
        {
            found_i = i;
            break;
        }
    }

    uint8_t current_i = found_i;

    // loop until we find one with a display name -> not an alias
    do
    {
        if(current_i == 0)
            current_i = base->m_enum_values.size();

        current_i--;
    }
    while(base->m_enum_values[current_i].m_display_name.empty() && current_i != found_i);

    // possibly misconfigured
    if(current_i == found_i)
    {
        pLogDebug("Cycled through enum option [%s] without finding new value", base->m_display_name.c_str());
        return false;
    }

    m_value = base->m_enum_values[current_i].m_value;

    if(!is_set())
        m_set = ConfigSetLevel::set;

    _on_change();

    return true;
}

template<class value_t>
bool ConfigEnumOption_t<true, value_t>::rotate_right()
{
    const ConfigEnumOption_t<false, value_t>* base = dynamic_cast<const ConfigEnumOption_t<false, value_t>*>(m_base);

    if(!base || base->m_enum_values.size() > 255)
        return false;

    uint8_t found_i = 0;

    for(uint8_t i = 0; i < base->m_enum_values.size(); i++)
    {
        if((value_t)*this == base->m_enum_values[i].m_value)
        {
            found_i = i;
            break;
        }
    }

    uint8_t current_i = found_i;

    // loop until we find one with a display name -> not an alias
    do
    {
        current_i++;

        if(current_i == base->m_enum_values.size())
            current_i = 0;
    }
    while(base->m_enum_values[current_i].m_display_name.empty() && current_i != found_i);

    // possibly misconfigured
    if(current_i == found_i)
    {
        pLogDebug("Cycled through enum option [%s] without finding new value", base->m_display_name.c_str());
        return false;
    }

    m_value = base->m_enum_values[current_i].m_value;

    if(!is_set())
        m_set = ConfigSetLevel::set;

    _on_change();

    return true;
}

template<class value_t>
const std::string& ConfigEnumOption_t<true, value_t>::get_display_value(std::string& out) const
{
    const ConfigEnumOption_t<false, value_t>* base = dynamic_cast<const ConfigEnumOption_t<false, value_t>*>(m_base);

    if(!base)
        return out = "INVALID";

    bool found = false;

    for(const ConfigEnumValueInfo_t<value_t>& val : base->m_enum_values)
    {
        if((value_t)*this == val.m_value)
        {
            if(!val.m_display_name.empty())
                out = val.m_display_name;
            else if(val.m_internal_name)
                out = val.m_internal_name;
            else
                continue;

            found = true;
            break;
        }
    }

    if(!found)
        ConfigOption_t<true, value_t>::get_display_value(out);

    return out;
}

template<class value_t>
const std::string& ConfigEnumOption_t<true, value_t>::get_value_tooltip(std::string& out) const
{
    const ConfigEnumOption_t<false, value_t>* base = dynamic_cast<const ConfigEnumOption_t<false, value_t>*>(m_base);

    if(!base)
        return out = "";

    for(const ConfigEnumValueInfo_t<value_t>& val : base->m_enum_values)
    {
        if((value_t)*this == val.m_value)
        {
            if(!val.m_display_tooltip.empty())
                return out = val.m_display_tooltip;
            else
                return out = "";
        }
    }

    return out = "";
}


// implementation for ConfigLanguage_t<true>

bool ConfigLanguage_t<true>::rotate_left()
{
    std::string old_value = m_value;
    XLanguage::rotateLanguage(m_value, -1);

    if(m_value == old_value)
        return false;

    if(!is_set())
        m_set = ConfigSetLevel::set;

    _on_change();

    return true;
}

bool ConfigLanguage_t<true>::rotate_right()
{
    std::string old_value = m_value;
    XLanguage::rotateLanguage(m_value, 1);

    if(m_value == old_value)
        return false;

    if(!is_set())
        m_set = ConfigSetLevel::set;

    _on_change();

    return true;
}

// implementation for ConfigRenderMode_t<true>
int ConfigRenderMode_t<true>::obtained = -1;

const std::string& ConfigRenderMode_t<true>::get_display_value(std::string& out) const
{
    ConfigEnumOption_t<true, int>::get_display_value(out);

    if(m_value != obtained)
    {
        // FIXME: should update g_mainMenu.optionsRenderX to not include the "Render: ", then use that.
        out += " (X)";
    }

    return out;
}


// implementation for ConfigEnumOption_t<true, value_t>

// implementation for ConfigSection_t<true>

ConfigSection_t<true>::ConfigSection_t(_Config_t<true>* parent, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip) :
        BaseConfigOption_t<true>(parent)
{
    (void)internal_name; (void)display_name; (void)display_tooltip; (void)scope;

    if(!parent->m_base)
        return;
    if((size_t)m_index >= parent->m_base->m_options.size())
        return;
    m_base = dynamic_cast<ConfigSection_t<false>*>(parent->m_base->m_options[m_index]);
}

void ConfigSection_t<true>::update_from_ini(IniProcessing* ini, ConfigSetLevel level)
{
    (void)level;

    if(!ini || !m_base)
        return;

    ini->endGroup();
    ini->beginGroup(m_base->m_internal_name);
}

// implementation for ConfigSubSection_t<true>

ConfigSubSection_t<true>::ConfigSubSection_t(_Config_t<true>* parent,
        const char* internal_name, const char* display_name, const char* display_tooltip) :
        BaseConfigOption_t(parent)
{
    (void)internal_name; (void)display_name; (void)display_tooltip;

    if(!parent->m_base)
        return;
    if((size_t)m_index >= parent->m_base->m_options.size())
        return;
    m_base = dynamic_cast<ConfigSubSection_t<false>*>(parent->m_base->m_options[m_index]);
}

#endif // #CONFIG_IMPL_HPP
