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

// again, want all Config objects to share:
//   internal names, display names, tooltips, default values, compat values, value internal names, display names, tooltips

// want the specific Config objects to have:
//   current values, whether they are set

#pragma once
#ifndef CONFIG_BASE_HPP
#define CONFIG_BASE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

class IniProcessing;
class XTechTranslate;

// There are two versions of each class, one to represent the read-only information about an option,
//   and another to represent a particular setting of the option.

template<bool writable>
class BaseConfigOption_t;

template<bool writable>
class ConfigSection_t;

template<bool writable>
class ConfigSubSection_t;

template<bool writable, class value_t>
class ConfigOption_t;

template<bool writable, class value_t>
class ConfigEnumOption_t;

template<bool writable>
class ConfigLanguage_t;

template<bool writable>
class ConfigRenderMode_t;

template<bool writable>
class _Config_t;

// helper classes

enum class ConfigSetLevel : uint8_t
{
    unset = 0,
    set, // used by sub-configs
    game_defaults,
    game_info,
    user_config,
    bugfix_defaults,
    ep_compat,
    file_compat,
    ep_config, // per-save configuration of an episode (very limited)
    cmdline,
    cheat,
    speedrun,
    compat,
    script,
    debug,
};

class ConfigChangeSentinel
{
    ConfigSetLevel m_prev_level;

public:
    ConfigChangeSentinel(ConfigSetLevel level);
    ~ConfigChangeSentinel();
};


// the compat modes are identical to the NetPlay and speedrun presets
enum class CompatMode
{
    off,       // use the user's config and the creator compat
    modern,    // force modern defaults for all user settings, do use creator compats
    classic,   // disable all standard bugfixes, force critical updates and gameplay enhancements on, do use creator compats
    smbx64,    // disable all bugfixes and gameplay enhancements, do not use creator compats
};

enum class CompatClass
{
    pure_preference,      // user-side preference (or gameinfo value) that does not impact game logic at all
    critical_update,      // performance- and flexibility-critical enhancements and bugfixes that prevent predictable unfair deaths, disabled in SMBX64 compat mode only
    gameplay_enhancement, // these are normally preferences: cosmetic enhancements that interact in some way with the game's logic (such as world map movement, medals display, or screen shake)
    standard_bugfix,      // ordinary fixes of vanilla bugs that get disabled during non-modern gameplay; these are determined by a user's Modern / Classic setting for a world even when compat mode is off
};


template<class value_t>
struct ConfigCompatInfo_t
{
    const CompatClass mode = CompatClass::pure_preference;
    const value_t prev_value = value_t();

    ConfigCompatInfo_t() {}

    ConfigCompatInfo_t(CompatClass mode, value_t prev_value)
        : mode(mode), prev_value(prev_value) {}
};

template<class value_t>
struct ConfigEnumValueInfo_t
{
    const value_t m_value;
    const char* m_internal_name;
    std::string m_display_name;
    std::string m_display_tooltip;

    ConfigEnumValueInfo_t(const value_t value, const char* internal_name, const char* display_name = "", const char* display_tooltip = "")
        : m_value(value), m_internal_name(internal_name), m_display_name(display_name ? display_name : ""), m_display_tooltip(display_tooltip ? display_tooltip : "") {}
};

// read-only classes

template<>
class BaseConfigOption_t<false>
{
public:
    const int16_t m_index;
    const uint8_t m_scope;

    const char* m_internal_name;
    std::string m_display_name;
    std::string m_display_tooltip;

    void (*m_onupdate)();
    bool (*m_active)();
    bool (*m_validate)(void*);

    // adds self to the parent's list
    BaseConfigOption_t(_Config_t<false>* parent, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)(), bool (*active)(), bool (*validate)(void*));

    virtual ~BaseConfigOption_t() = default;

    virtual void make_translation(XTechTranslate& translate, const char* cur_section_id);
};

template<class value_t>
class ConfigOption_t<false, value_t> : public BaseConfigOption_t<false>
{
public:
    value_t m_default_value;
    ConfigCompatInfo_t<value_t> m_compat_info;

    ConfigOption_t(_Config_t<false>* parent, const value_t default_value,
        const ConfigCompatInfo_t<value_t>& compat_info, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)() = nullptr, bool (*active)() = nullptr, bool (*validate)(void*) = nullptr) :
            BaseConfigOption_t(parent, scope,
                internal_name, display_name, display_tooltip,
                onupdate, active, validate),
            m_default_value(default_value),
            m_compat_info(compat_info) {}
};

template<class value_t>
class ConfigEnumOption_t<false, value_t> : public ConfigOption_t<false, value_t>
{
public:
    using BaseConfigOption_t<false>::m_internal_name;
    using BaseConfigOption_t<false>::m_display_name;
    using BaseConfigOption_t<false>::m_display_tooltip;

    std::vector<ConfigEnumValueInfo_t<value_t>> m_enum_values;

    ConfigEnumOption_t(_Config_t<false>* parent,
        const std::vector<ConfigEnumValueInfo_t<value_t>>& enum_values,
        const value_t default_value,
        const ConfigCompatInfo_t<value_t>& compat_info, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)() = nullptr, bool (*active)() = nullptr, bool (*validate)(void*) = nullptr) :
            ConfigOption_t<false, value_t>(parent, default_value, compat_info, scope,
                internal_name, display_name, display_tooltip,
                onupdate, active, validate),
            m_enum_values(enum_values) {}

    virtual void make_translation(XTechTranslate& translate, const char* cur_section_id) override;
};

template<>
class ConfigLanguage_t<false> : public ConfigOption_t<false, std::string>
{
    using value_t = std::string;

public:
    ConfigLanguage_t(_Config_t<false>* parent,
        const value_t default_value,
        const ConfigCompatInfo_t<value_t>& compat_info, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)() = nullptr, bool (*active)() = nullptr, bool (*validate)(void*) = nullptr) :
            ConfigOption_t<false, value_t>(parent, default_value, compat_info, scope,
                internal_name, display_name, display_tooltip,
                onupdate, active, validate)  {}
};

template<>
class ConfigRenderMode_t<false> : public ConfigEnumOption_t<false, int>
{
    using ConfigEnumOption_t<false, int>::ConfigEnumOption_t;
};

template<>
class ConfigSection_t<false> : public BaseConfigOption_t<false>
{
public:
    ConfigSection_t(_Config_t<false>* parent, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip = nullptr) :
        BaseConfigOption_t<false>(parent, scope,
            internal_name, display_name, display_tooltip,
            nullptr, nullptr, nullptr) {}

    virtual void make_translation(XTechTranslate& translate, const char* cur_section_id) override;
};

template<>
class ConfigSubSection_t<false> : public BaseConfigOption_t<false>
{
public:
    ConfigSubSection_t(_Config_t<false>* parent,
        const char* internal_name, const char* display_name, const char* display_tooltip = nullptr) :
        BaseConfigOption_t<false>(parent, 0xff,
            internal_name, display_name, display_tooltip,
            nullptr, nullptr, nullptr) {}
};

// writable classes

template<>
class BaseConfigOption_t<true>
{
public:
    const BaseConfigOption_t<false>* m_base = nullptr;
    const int16_t m_index;
    ConfigSetLevel m_set = ConfigSetLevel::unset;

    // adds self to the parent's list
    BaseConfigOption_t(_Config_t<true>* parent);

    BaseConfigOption_t(_Config_t<true>* parent, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)(), bool (*active)(), bool (*validate)(void*)) :
            BaseConfigOption_t(parent)
    { (void)scope, (void)internal_name; (void)display_name; (void)display_tooltip; (void)onupdate; (void)active; (void)validate; }

    // returns whether this is a member of g_config or not
    bool is_main() const;

    // executes hooks if appropriate
    void _on_change();

    inline bool is_set() const
    {
        return m_set != ConfigSetLevel::unset;
    }

    virtual bool operator==(const BaseConfigOption_t<true>& o) const { (void)o; return false; }

    inline void unset() { m_set = ConfigSetLevel::unset; }

    virtual bool rotate_left() { return false; }
    virtual bool rotate_right() { return false; }
    virtual bool change() { return rotate_right(); }

    virtual void set_from_default(ConfigSetLevel level) { (void)level; }
    virtual void update_from_ini(IniProcessing* ini, ConfigSetLevel level) { (void)ini; (void)level; }

    virtual void disable_bugfixes(ConfigSetLevel level) { (void)level; }

    virtual void update_from_compat(CompatMode compat_mode, ConfigSetLevel level) { (void)compat_mode; (void)level; }
    virtual void update_from(const BaseConfigOption_t<true>& o, ConfigSetLevel level) { (void)o; (void)level; }

    virtual void save_to_ini(IniProcessing* ini) { (void)ini; }

    const std::string& get_display_name(std::string& out) const;
    virtual const std::string& get_display_value(std::string& out) const { out.clear(); return out; }

    const std::string& get_tooltip(std::string& out) const;
    virtual const std::string& get_value_tooltip(std::string& out) const { out.clear(); return out; }
};

template<class value_t>
class ConfigOption_t<true, value_t> : public BaseConfigOption_t<true>
{
public:
    value_t m_value;

    ConfigOption_t(_Config_t<true>* parent, const value_t default_value,
        const ConfigCompatInfo_t<value_t>& compat_info, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)() = nullptr, bool (*active)() = nullptr, bool (*validate)(void*) = nullptr) :
            BaseConfigOption_t<true>(parent) { (void)default_value; (void)compat_info; (void)scope; (void)internal_name; (void)display_name; (void)display_tooltip; (void)onupdate; (void)active; (void)validate; }

    virtual bool operator==(const BaseConfigOption_t<true>& o) const;

    inline operator value_t() const
    {
        return m_value;
    }

    virtual bool rotate_left() override;
    virtual bool rotate_right() override;
    virtual bool change() override;

    const ConfigOption_t<true, value_t>& operator=(value_t value);

    virtual void set_from_default(ConfigSetLevel level) override;
    virtual void update_from_ini(IniProcessing* ini, ConfigSetLevel level) override;

    virtual void disable_bugfixes(ConfigSetLevel level) override;

    virtual void update_from_compat(CompatMode compat_mode, ConfigSetLevel level) override;
    virtual void update_from(const BaseConfigOption_t<true>& o, ConfigSetLevel level)  override;

    virtual void save_to_ini(IniProcessing* ini) override;

    virtual const std::string& get_display_value(std::string& out) const override;
};

template<class value_t>
class ConfigEnumOption_t<true, value_t> : public ConfigOption_t<true, value_t>
{
public:
    using BaseConfigOption_t<true>::is_set;
    using BaseConfigOption_t<true>::m_set;
    using ConfigOption_t<true, value_t>::m_value;
    using BaseConfigOption_t<true>::m_base;
    using ConfigOption_t<true, value_t>::operator=;
    using BaseConfigOption_t<true>::_on_change;

    ConfigEnumOption_t(_Config_t<true>* parent,
        const std::vector<ConfigEnumValueInfo_t<value_t>>& enum_values,
        const value_t default_value,
        const ConfigCompatInfo_t<value_t>& compat_info, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)() = nullptr, bool (*active)() = nullptr, bool (*validate)(void*) = nullptr) :
            ConfigOption_t<true, value_t>(parent, default_value, compat_info, scope,
                internal_name, display_name, display_tooltip,
                onupdate, active, validate) { (void)enum_values; }

    virtual bool rotate_left() override;
    virtual bool rotate_right() override;

    virtual void update_from_ini(IniProcessing* ini, ConfigSetLevel level) override;

    virtual void save_to_ini(IniProcessing* ini) override;

    virtual const std::string& get_display_value(std::string& out) const override;
    virtual const std::string& get_value_tooltip(std::string& out) const override;
};

template<>
class ConfigLanguage_t<true> : public ConfigOption_t<true, std::string>
{
    using value_t = std::string;

public:
    using BaseConfigOption_t<true>::m_set;
    using BaseConfigOption_t<true>::m_base;
    using ConfigOption_t<true, value_t>::operator=;

    ConfigLanguage_t(_Config_t<true>* parent,
        const value_t default_value,
        const ConfigCompatInfo_t<value_t>& compat_info, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip,
        void (*onupdate)() = nullptr, bool (*active)() = nullptr, bool (*validate)(void*) = nullptr) :
            ConfigOption_t<true, value_t>(parent, default_value, compat_info, scope,
                internal_name, display_name, display_tooltip,
                onupdate, active, validate) {}

    virtual bool rotate_left() override;
    virtual bool rotate_right() override;
};

template<>
class ConfigRenderMode_t<true> : public ConfigEnumOption_t<true, int>
{
public:
    using ConfigEnumOption_t<true, int>::ConfigEnumOption_t;
    using ConfigEnumOption_t<true, int>::operator=;

    virtual const std::string& get_display_value(std::string& out) const override;

    static int obtained;
};

template<>
class ConfigSection_t<true> : public BaseConfigOption_t<true>
{
public:
    const ConfigSection_t<false>* m_base = nullptr;

    ConfigSection_t(_Config_t<true>* parent, uint8_t scope,
        const char* internal_name, const char* display_name, const char* display_tooltip = nullptr);

    virtual void update_from_ini(IniProcessing* ini, ConfigSetLevel level) override;
    virtual void save_to_ini(IniProcessing* ini) override { update_from_ini(ini, ConfigSetLevel::unset); };
};

template<>
class ConfigSubSection_t<true> : public BaseConfigOption_t<true>
{
public:
    const ConfigSubSection_t<false>* m_base = nullptr;

    ConfigSubSection_t(_Config_t<true>* parent,
        const char* internal_name, const char* display_name, const char* display_tooltip = nullptr);
};

#endif // CONFIG_BASE_HPP
