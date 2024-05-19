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

#include <set>

#include <fmt_format_ne.h>

#include "config/config_base.hpp"
#include "config.h"

#include "main/translate.h"

// can't include config_impl.hpp to avoid multiple declarations

// config_base.cpp
extern ConfigSetLevel g_configWriteLevel;

// config_main.cpp
extern bool g_configInTransaction;
extern std::set<BaseConfigOption_t<true>*> g_configModified;


ConfigSetLevel g_configWriteLevel = ConfigSetLevel::debug;

ConfigChangeSentinel::ConfigChangeSentinel(ConfigSetLevel level)
{
    m_prev_level = g_configWriteLevel;
    g_configWriteLevel = level;
}

ConfigChangeSentinel::~ConfigChangeSentinel()
{
    g_configWriteLevel = m_prev_level;
}


// base config
BaseConfigOption_t<false>::BaseConfigOption_t(_Config_t<false>* parent, uint8_t scope,
    const char* internal_name, const char* display_name, const char* display_tooltip,
    void (*onupdate)(), bool (*active)(), bool (*validate)(void*))
 : m_index(parent->m_options.size()),
   m_scope(scope),
   m_internal_name(internal_name),
   m_display_name(display_name ? display_name : ""),
   m_display_tooltip(display_tooltip ? display_tooltip : ""),
   m_onupdate(onupdate),
   m_active(active),
   m_validate(validate)
{
    parent->m_options.push_back(this);
}

void BaseConfigOption_t<false>::make_translation(XTechTranslate& translate, const char* cur_section_id)
{
    if((m_scope & Options_t::Scope::User) == 0)
        return;

    translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}.{1}", cur_section_id, m_internal_name), &m_display_name});

    if(!m_display_tooltip.empty())
        translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}.{1}-tip", cur_section_id, m_internal_name), &m_display_tooltip});
}

BaseConfigOption_t<true>::BaseConfigOption_t(_Config_t<true>* parent) : m_index(parent->m_options.size())
{
    parent->m_options.push_back(this);

    if(!parent->m_base)
        return;
    if((size_t)m_index >= parent->m_base->m_options.size())
        return;
    m_base = parent->m_base->m_options[m_index];
}

bool BaseConfigOption_t<true>::is_main() const
{
    return (size_t)this >= (size_t)(&g_config) && (size_t)this < (size_t)(&g_config) + sizeof(_Config_t<true>);
}

void BaseConfigOption_t<true>::_on_change()
{
    if(m_base && m_base->m_onupdate && is_main())
    {
        if(g_configInTransaction)
            g_configModified.insert(this);
        else
            m_base->m_onupdate();
    }
}


const std::string& BaseConfigOption_t<true>::get_display_name(std::string& out) const
{
    if(!m_base || m_base->m_display_name.empty())
        return out = "(INVALID)";

    return out = m_base->m_display_name;
}

const std::string& BaseConfigOption_t<true>::get_tooltip(std::string& out) const
{
    if(!m_base || m_base->m_display_tooltip.empty())
        return out = "";

    return out = m_base->m_display_tooltip;
}

void ConfigSection_t<false>::make_translation(XTechTranslate& translate, const char* cur_section_id)
{
    if((m_scope & Options_t::Scope::User) == 0)
        return;

    translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}._header", cur_section_id), &m_display_name});

    if(!m_display_tooltip.empty())
        translate.m_engineMap.insert({fmt::format_ne("menu.options.{0}._tooltip", cur_section_id), &m_display_tooltip});
}
