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

#include <algorithm>

#include <IniProcessor/ini_processing.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "core/render.h"

#include "global_constants.h"
#include "global_dirs.h"
#include "sound.h"
#include "globals.h"
#include "gfx.h"
#include "graphics.h"
#include "config.h"

#include "cheat_code.h"
#include "game_main.h"
#include "main/game_info.h"
#include "main/menu_main.h"
#include "main/screen_options.h"
#include "graphics/gfx_frame.h"
#include "graphics/gfx_marquee.h"
#include "main/menu_controls.h"
#include "controls.h"

namespace OptionsScreen
{

constexpr size_t SECTION_NONE = -1;
size_t section_index = SECTION_NONE;
std::vector<size_t> section_header_indexes;
size_t next_section_index = 0;

std::vector<size_t> visible_items;


size_t cur_item = 0;
size_t cur_scroll = 0;

int8_t mouse_scroll_cooldown = -1;
bool controls_ready = false;
bool cur_item_changed = false;

bool global_dirty = false;

MarqueeState name_marquee;
MarqueeState value_marquee;
MarqueeState name_tooltip_marquee;
MarqueeState value_tooltip_marquee;

static std::string s_temp_string;

// Config_t* const config_levels[] = {nullptr, nullptr, &g_gameInfo, &g_config_game_user, &g_config_episode_creator, &g_config_episode_user,
//     &g_config_file_creator, nullptr, &g_config_cmdline, nullptr};

inline void s_set_dirty()
{
    global_dirty = true;
}

inline void s_switch_item()
{
    cur_item_changed = false;
    name_marquee.reset_width();
    value_marquee.reset_width();
    name_tooltip_marquee.reset_width();
    value_tooltip_marquee.reset_width();
}

inline void s_change_item()
{
    cur_item_changed = true;
    value_marquee.reset_width();
    value_tooltip_marquee.reset_width();
}

inline size_t get_num_items()
{
    return visible_items.size();
}

inline bool is_subsection(size_t item)
{
    if(section_index == SECTION_NONE || item >= get_num_items())
        return false;

    if(g_config.m_options[visible_items[item]] == &g_config.reset_all)
        return false;

    return dynamic_cast<ConfigSubSection_t<true>*>(g_config.m_options[visible_items[item]]) != nullptr;
}

inline void s_reset_friends()
{
    bool any_compat_set = false;

    for(size_t index : visible_items)
    {
        if(index < g_config.m_options.size() && g_config.m_options[index] && g_config.m_options[index]->m_set == ConfigSetLevel::cheat)
        {
            g_config.m_options[index]->unset();
            any_compat_set = true;
        }
    }

    if(any_compat_set)
    {
        UpdateConfig();
        PlaySoundMenu(SFX_LavaMonster);
    }
    else
        PlaySoundMenu(SFX_BlockHit);
}

inline void s_check_friends_edited()
{
    bool any_compat_set = false;

    for(size_t index : visible_items)
    {
        if(index < g_config.m_options.size() && g_config.m_options[index] && g_config.m_options[index]->m_set == ConfigSetLevel::cheat)
        {
            any_compat_set = true;
            break;
        }
    }

    g_CheatEditYourFriends = any_compat_set;
}

void RefreshVisibleItems();

void ResetStrings()
{
    name_marquee.reset_width();
    value_marquee.reset_width();
    name_tooltip_marquee.reset_width();
    value_tooltip_marquee.reset_width();
}

void Init()
{
    section_header_indexes.clear();
    size_t i = 0;
    for(BaseConfigOption_t<false>* opt : g_options.m_options)
    {
        if(dynamic_cast<ConfigSection_t<false>*>(opt))
            section_header_indexes.push_back(i);
        i++;
    }

    section_index = SECTION_NONE;

    if(g_CheatEditYourFriends == 2)
    {
        // set to compat
        for(size_t i : section_header_indexes)
        {
            if(g_options.m_options[i] == &g_options.compat)
            {
                section_index = i;
                break;
            }
        }
    }

    RefreshVisibleItems();

    cur_item = 0;
    s_switch_item();
    cur_scroll = 0;

    controls_ready = false;

    global_dirty = false;
}

void RefreshVisibleItems()
{
    visible_items.clear();

    auto current_scope = Config_t::Scope::UserVisible;

    if(section_index == SECTION_NONE)
    {
        for(size_t i : section_header_indexes)
        {
            if(g_options.m_options[i] == &g_options.view_credits && !GameMenu)
                continue;

            // previously banned access to advanced menu during gameplay
            // if(g_options.m_options[i] == &g_options.advanced && !GameMenu)
            //     continue;

            if(g_options.m_options[i] == &g_options.episode_options && (GameMenu || BattleMode))
                continue;

            if(g_options.m_options[i] == &g_options.compat && !g_CheatEditYourFriends)
                continue;

            if(g_options.m_options[i]->m_scope & current_scope)
                visible_items.push_back(i);
        }
    }
    else
    {
        bool last_was_subsection = false;

        auto it = std::find(section_header_indexes.begin(), section_header_indexes.end(), section_index);
        if(it == section_header_indexes.end() || it + 1 == section_header_indexes.end())
            next_section_index = g_config.m_options.size();
        else
            next_section_index = *(it + 1);

        for(size_t i = section_index + 1; i < next_section_index; i++)
        {
            if(g_CheatEditYourFriends || g_options.m_options[i]->m_scope & current_scope)
            {
                bool this_is_subsection = g_options.m_options[i] != &g_options.reset_all && dynamic_cast<ConfigSubSection_t<false>*>(g_options.m_options[i]);

                if(last_was_subsection && this_is_subsection)
                    visible_items.resize(visible_items.size() - 1);

                visible_items.push_back(i);

                last_was_subsection = this_is_subsection;
            }
        }

        if(last_was_subsection)
            visible_items.resize(visible_items.size() - 1);
    }
}

// checks to see whether it is possible for a Left/Right/Do to be performed given the current cursor and resolve which exact option should be modified
inline BaseConfigOption_t<true>* PrepareAction(bool to_delete = false)
{
    if(cur_item >= visible_items.size())
        return nullptr;

    size_t i = visible_items[cur_item];
    if(i >= next_section_index)
        return nullptr;

    // ban changing speedrun mode
    if(g_config.m_options[i] == &g_config.speedrun_mode)
        return nullptr;

    // is it one of the other unique episode config options?
    bool is_ep_config = (g_config.m_options[i] == &g_config.creator_compat) || (g_config.m_options[i] == &g_config.playstyle);

    // check some info about the option
    ConfigSetLevel prev_level = g_config.m_options[i]->m_set;
    bool use_cheat = g_CheatEditYourFriends && ((g_options.m_options[i]->m_scope & g_config_game_user.m_scope) == 0 || prev_level > ConfigSetLevel::user_config);
    bool was_debug = (!use_cheat && prev_level == ConfigSetLevel::cheat) || (prev_level == ConfigSetLevel::debug);

    // level that will be set
    ConfigSetLevel target_level = (is_ep_config) ? ConfigSetLevel::ep_config
        : ((use_cheat) ? ConfigSetLevel::cheat
            : ConfigSetLevel::user_config);

    if(prev_level > target_level && !was_debug)
        return nullptr;

    if(!(is_ep_config || use_cheat) && (g_options.m_options[i]->m_scope & g_config_game_user.m_scope) == 0)
        return nullptr;

    BaseConfigOption_t<true>* opt = (is_ep_config || use_cheat) ? g_config.m_options[i] : g_config_game_user.m_options[i];

    // if not currently active, set from previous level
    if(!opt->is_set() || (was_debug && !to_delete))
    {
        opt->m_set = ConfigSetLevel::unset;

        if(g_config.m_options[i]->is_set())
            opt->update_from(*g_config.m_options[i], ConfigSetLevel::set);
        else
            opt->set_from_default(ConfigSetLevel::set);

        opt->m_set = ConfigSetLevel::unset;
    }

    if(was_debug)
    {
        pLogDebug("Unsetting debug / cheat option [%s]", g_options.m_options[i]->m_internal_name);

        g_config.m_options[i]->unset();
        s_change_item();
        UpdateConfig();

        // consider this the deleted option
        if(to_delete)
        {
            PlaySoundMenu(SFX_PlayerShrink);
            return nullptr;
        }
    }

    if(is_ep_config || use_cheat)
        opt->m_set = target_level;

    return opt;
}

void Do()
{
    if(cur_item >= visible_items.size())
        return;

    size_t real_item = visible_items[cur_item];

    if(real_item >= g_config.m_options.size())
        return;

    if(g_config.m_options[real_item] == &g_config.reset_all)
    {
        s_reset_friends();
    }
    else if(dynamic_cast<ConfigSection_t<true>*>(g_config.m_options[real_item]))
    {
        if(g_config.m_options[real_item] == &g_config.controls)
        {
            PlaySoundMenu(SFX_Do);

            global_dirty = true;
            MenuMode = MENU_INPUT_SETTINGS;
            MenuCursor = 0;
            MenuCursorCanMove = false;
            controls_ready = false;
            return;
        }
        else if(g_config.m_options[real_item] == &g_config.view_credits && GameMenu)
        {
            controls_ready = false;

            PlaySoundMenu(SFX_Do);
            GameMenu = false;
            GameOutro = true;
            CreditChop = XRender::TargetH / 2;
            EndCredits = 0;
            SetupCredits();
            return;
        }

        section_index = real_item;

        RefreshVisibleItems();

        if(visible_items.size() == 0)
        {
            PlaySoundMenu(SFX_BlockHit);
            section_index = SECTION_NONE;
            RefreshVisibleItems();
        }
        else
        {
            cur_item = 0;
            s_switch_item();
            while(is_subsection(cur_item) && cur_item + 1 < visible_items.size())
                cur_item++;

            cur_scroll = 0;

            PlaySoundMenu(SFX_Do);
        }
    }
    else
    {
        BaseConfigOption_t<true>* opt = PrepareAction();

        if(opt && opt->change())
        {
            s_set_dirty();
            s_change_item();
            UpdateConfig();
            PlaySoundMenu(SFX_Do);
        }
        else
        {
            PlaySoundMenu(SFX_BlockHit);
        }
    }
}

void RotateLeft()
{
    if(section_index == SECTION_NONE)
        return;

    BaseConfigOption_t<true>* opt = PrepareAction();

    if(opt && opt->rotate_left())
    {
        s_set_dirty();
        s_change_item();
        UpdateConfig();
        PlaySoundMenu(SFX_Do);
    }
    else
    {
        PlaySoundMenu(SFX_BlockHit);
    }
}

void RotateRight()
{
    if(section_index == SECTION_NONE)
        return;

    BaseConfigOption_t<true>* opt = PrepareAction();

    if(opt && opt->rotate_right())
    {
        s_set_dirty();
        s_change_item();
        UpdateConfig();
        PlaySoundMenu(SFX_Do);
    }
    else
    {
        PlaySoundMenu(SFX_BlockHit);
    }
}

void Delete()
{
    if(section_index == SECTION_NONE)
        return;

    BaseConfigOption_t<true>* opt = PrepareAction(true);

    if(opt && opt->is_set() && opt != &g_config.playstyle && opt != &g_config.creator_compat)
    {
        s_set_dirty();
        s_change_item();
        opt->unset();
        UpdateConfig();
        PlaySoundMenu(SFX_PlayerShrink);
    }
    else
    {
        PlaySoundMenu(SFX_BlockHit);
    }
}

bool Back();

void Select()
{
    // disabling for now
    return;
}

bool Back()
{
    PlaySoundMenu(SFX_Slide);

    if(g_CheatEditYourFriends == 2)
    {
        // return to game directly from first trip to edit your friends screen
        g_CheatEditYourFriends = 1;
        s_check_friends_edited();
        global_dirty = false;
        return true;
    }
    else if(section_index == SECTION_NONE)
    {
        pLogDebug("Saving config on options screen exit (config path [%s], dirty %d)",
            AppPathManager::settingsFileSTD().c_str(), (int)global_dirty);

        // save any dirty configs
        if(global_dirty)
            SaveConfig();

        return true;
    }
    else
    {
        if(section_index < g_config.m_options.size() && g_config.m_options[section_index] == &g_config.compat)
            s_check_friends_edited();

        s_switch_item();

        int last_section = section_index;

        section_index = SECTION_NONE;
        cur_scroll = 0;
        RefreshVisibleItems();

        auto it = std::find(visible_items.begin(), visible_items.end(), last_section);

        // go to last item if couldn't find
        if(it == visible_items.end() && it != visible_items.begin())
            cur_item = (int)((visible_items.end() - 1) - visible_items.begin());
        else
            cur_item = (int)(it - visible_items.begin());
    }

    return false;
}

static inline void Render_Cursor(int x, int y, int p)
{
    XRender::renderTexture(x, y, GFX.MCursor[0]);

    UNUSED(p);
}

bool Mouse_Render(bool mouse, bool render)
{
    if(mouse && !render && !SharedCursor.Move && !SharedCursor.Primary && !SharedCursor.Secondary && !SharedCursor.ScrollUp && !SharedCursor.ScrollDown)
    {
        if(!GameMenu)
            MenuMouseRelease = true;

        // subtract from the mouse scroll cooldown
        if(mouse_scroll_cooldown > 0)
            mouse_scroll_cooldown--;
    }

    // want 680px width.
    int width;
    if(XRender::TargetW < 640)
        width = XRender::TargetW - 16 - XRender::TargetOverscanX * 2;
    else if(XRender::TargetW < 720)
        width = XRender::TargetW - 40 - XRender::TargetOverscanX * 2;
    else
        width = 680;

    const bool tight_mode = width < 640 || XRender::TargetH < 480;

    // calculate line height: want 15 lines of text
    int line = (XRender::TargetH - 60) / 15;
    line -= line & 1;
    // max line height 30
    if(line > 30)
        line = 30;
    // (it's okay if we don't get 15 lines, but we need at least 18px per line.)
    int max_line = 15;
    if(line < 18)
    {
        line = 18;
        max_line = (int)XRender::TargetH / line;
    }

    // two header rows, two footer rows
    const int first_display_row = 2;
    const int display_rows = max_line - 4;

    // calculate main column and value column width
    const int total_width = tight_mode ? width - 32 : width - 64;
    const int main_width  = total_width * 3 / 4 - 16;
    const int value_width = total_width * 1 / 4;

    // horizontal start of the menu
    int sX = XRender::TargetW / 2 - width / 2;
    sX -= sX & 1;
    // vertical start of the menu
    int sY = XRender::TargetH / 2 - (line * max_line) / 2;
    sY -= sY & 1;

    int cursor_x = tight_mode ? sX + 8 : sX + 24;

    size_t num_items = get_num_items();

    // mouse logic
    if(mouse)
    {
        if(SharedCursor.ScrollUp)
        {
            if(cur_scroll > 0)
            {
                PlaySoundMenu(SFX_Saw);
                cur_scroll--;
                if(cur_item >= display_rows + cur_scroll - 1)
                {
                    cur_item--;
                    s_switch_item();
                }
            }
        }
        else if(SharedCursor.ScrollDown)
        {
            if(cur_scroll + display_rows < num_items)
            {
                PlaySoundMenu(SFX_Saw);
                cur_scroll++;

                if(cur_item < cur_scroll + 1)
                {
                    cur_item++;
                    s_switch_item();
                }
            }
        }

        if(SharedCursor.X < sX || SharedCursor.X >= sX + width / 2)
            return false;

        int cur_mouse_line = (SharedCursor.Y - sY) / line;
        int cur_mouse_item = cur_scroll + cur_mouse_line - first_display_row;

        if(SharedCursor.Primary && cur_mouse_item == (int)cur_item && MenuMouseRelease)
        {
            MenuMouseRelease = false;
            Do();
        }
        else if(SharedCursor.Secondary && MenuMouseRelease)
        {
            MenuMouseRelease = false;
            return Back();
        }
        else if(cur_mouse_item != (int)cur_item
            && cur_mouse_line >= first_display_row
            && cur_mouse_line < first_display_row + display_rows
            && cur_mouse_item >= 0
            && cur_mouse_item < (int)num_items)
        {
            if(cur_mouse_line == first_display_row || cur_mouse_line == first_display_row + display_rows - 1)
            {
                if(mouse_scroll_cooldown > 0 || (mouse_scroll_cooldown == -1 && !SharedCursor.Move))
                    return false;
                else
                    mouse_scroll_cooldown = 11;
            }
            else if(!SharedCursor.Move)
                return false;

            if(is_subsection(cur_mouse_item))
            {
                if(cur_mouse_line == first_display_row && cur_scroll > 0)
                {
                    PlaySoundMenu(SFX_Saw);
                    cur_scroll--;
                    if(cur_item >= display_rows + cur_scroll - 1)
                    {
                        cur_item--;
                        s_switch_item();
                    }
                }
                else if(cur_mouse_line == first_display_row + display_rows - 1 && cur_scroll + display_rows < num_items)
                {
                    PlaySoundMenu(SFX_Saw);
                    cur_scroll++;

                    if(cur_item < cur_scroll + 1)
                    {
                        cur_item++;
                        s_switch_item();
                    }
                }
                else
                {
                    return false;
                }
            }

            cur_item = (size_t)cur_mouse_item;
            s_switch_item();
            if(!SharedCursor.ScrollUp && !SharedCursor.ScrollDown)
                PlaySoundMenu(SFX_Slide);
        }

        if(SharedCursor.Primary)
            MenuMouseRelease = false;

        return false;
    }

    // render logic

    // render the background
    DrawSimpleFrame(sX, sY - (line - 18) - 4, width, line * max_line + (line - 18) + 8, {0, 0, 0, 127}, {255, 255, 255, 127}, {0, 0, 0, 127});

    // render the title
    if(section_index != SECTION_NONE && section_index < g_config.m_options.size())
        g_config.m_options[section_index]->get_display_name(s_temp_string);
    else
        s_temp_string = g_mainMenu.mainOptions;

    SuperPrintScreenCenter(s_temp_string, 3, sY);

    // locate the cursor
    int cur_item_row = cur_item - cur_scroll;

    // handle forced scrolling
    if(cur_item_row < 1 && cur_scroll > 0)
    {
        cur_scroll--;
        cur_item_row++;
    }
    else if(cur_item_row >= display_rows - 1 && cur_scroll + display_rows < num_items)
    {
        cur_scroll++;
        cur_item_row--;
    }

    // render the cursor
    if(cur_item_row >= 0 && cur_item_row < display_rows)
        Render_Cursor(cursor_x, sY + (cur_item_row + first_display_row) * line, 0);

    // render the scroll indicators
    if(cur_scroll > 0)
        XRender::renderTexture(sX + width / 2 - GFX.MCursor[1].w / 2, sY + first_display_row * line - GFX.MCursor[1].h, GFX.MCursor[1]);

    if(cur_scroll + display_rows < num_items)
        XRender::renderTexture(sX + width / 2 - GFX.MCursor[2].w / 2, sY + (first_display_row + display_rows) * line - line + 18, GFX.MCursor[2]);

    MarqueeSpec name_marquee_spec = MarqueeSpec(0, 10, 64, 32, -1);
    MarqueeSpec value_marquee_spec = MarqueeSpec(value_width, 10, 64, 32, 1);

    // render the options
    for(size_t i = cur_scroll; i < cur_scroll + display_rows && i < num_items; i++)
    {
        BaseConfigOption_t<true>* opt = g_config.m_options[visible_items[i]];

        bool is_header = is_subsection(i);

        // for subsection headers, indent to left and allow to fill screen
        int lX = is_header ? cursor_x : cursor_x + 24;
        XTColor name_color = is_header ? XTColorF(0.7f, 0.7f, 0.7f) : XTColor();
        int name_font = is_header ? 3 : 5;

        name_marquee_spec.marquee_width = is_header ? total_width : main_width;

        if(tight_mode && i != cur_item)
            name_color = name_color * 0.6f;

        if(opt == &g_config.compat || opt == &g_config.reset_all)
            name_color.g = 0;

        // display name
        opt->get_display_name(s_temp_string);

        MarqueeState null_marquee;

        SuperPrintMarquee(s_temp_string, name_font,
            lX, sY + (i - cur_scroll + first_display_row) * line,
            name_marquee_spec, (i == cur_item) ? name_marquee : null_marquee,
            name_color);

        if(i == cur_item)
            name_marquee.advance(name_marquee_spec);

        if(is_header)
            continue;

        // display value
        null_marquee = MarqueeState();

        opt->get_display_value(s_temp_string);

        // pick the appropriate tint based on the setting level
        XTColor vcolor;
        switch(opt->m_set)
        {
        case(ConfigSetLevel::debug):
        case(ConfigSetLevel::script):
        case(ConfigSetLevel::cheat):
            vcolor = XTColorF(1.0f, 0.0f, 1.0f, 1.0f);
            break;
        case(ConfigSetLevel::compat):
        case(ConfigSetLevel::speedrun):
            vcolor = XTColorF(0.7f, 0.5f, 0.2f, 1.0f);
            break;
        case(ConfigSetLevel::cmdline):
            vcolor = XTColorF(0.0f, 0.5f, 1.0f, 1.0f);
            break;
        case(ConfigSetLevel::file_compat):
        case(ConfigSetLevel::ep_compat):
            vcolor = XTColorF(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        default:
        case(ConfigSetLevel::user_config):
            if(s_temp_string == "✓" || s_temp_string == "+")
                vcolor = XTColorF(0.5f, 1.0f, 0.5f, 1.0f);
            else if(s_temp_string == "×" || s_temp_string == "-")
                vcolor = XTColorF(1.0f, 0.5f, 0.5f, 1.0f);
            else
                vcolor = XTColorF(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case(ConfigSetLevel::game_info):
        case(ConfigSetLevel::game_defaults):
        case(ConfigSetLevel::bugfix_defaults):
            if(s_temp_string == "✓" || s_temp_string == "+")
                vcolor = XTColorF(0.25f, 0.5f, 0.25f, 1.0f);
            else if(s_temp_string == "×" || s_temp_string == "-")
                vcolor = XTColorF(0.5f, 0.25f, 0.25f, 1.0f);
            else
                vcolor = XTColorF(0.5f, 0.5f, 0.5f, 1.0f);
            break;
        }

        if(tight_mode && i != cur_item)
            vcolor = vcolor * 0.6f;

        const int rX = tight_mode ? sX + width - 8 : sX + width - 16;
        // SuperPrintRightAlign(s_temp_string, 3, rX, sY + (i - cur_scroll + first_display_row) * line, vr, vg, vb, va * name_opacity);

        SuperPrintMarquee(s_temp_string, name_font,
            rX - value_width, sY + (i - cur_scroll + first_display_row) * line,
            value_marquee_spec, (i == cur_item) ? value_marquee : null_marquee,
            vcolor);

        if(i == cur_item)
            value_marquee.advance(value_marquee_spec);
    }

    // render the tooltips
    if(cur_item < visible_items.size())
    {
        MarqueeSpec tooltip_marquee_spec = MarqueeSpec(total_width, 10, 64, 32, 0);
        int tooltip_left = sX + (width - total_width) / 2;

        BaseConfigOption_t<true>* opt = g_config.m_options[visible_items[cur_item]];

        if(cur_item_changed && !opt->get_value_tooltip(s_temp_string).empty())
        {
            value_tooltip_marquee.advance(tooltip_marquee_spec);
            SuperPrintMarquee(s_temp_string, 5, tooltip_left, sY + (first_display_row + display_rows + 1) * line, tooltip_marquee_spec, value_tooltip_marquee);
        }
        else if(!opt->get_tooltip(s_temp_string).empty())
        {
            name_tooltip_marquee.advance(tooltip_marquee_spec);
            SuperPrintMarquee(s_temp_string, 5, tooltip_left, sY + (first_display_row + display_rows + 1) * line, tooltip_marquee_spec, name_tooltip_marquee);
        }
    }

    return false;
}

void Render()
{
    // only relevant when launched from game
    if(MenuMode == MENU_INPUT_SETTINGS)
        return menuControls_Render();

    Mouse_Render(false, true);
}

bool Logic()
{
    size_t num_items = get_num_items();

    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;
    bool leftPressed = SharedControls.MenuLeft;
    bool rightPressed = SharedControls.MenuRight;

    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack;

    bool delPressed = false;
    bool selectPressed = false;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t &c = Player[i+1].Controls;

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;

        delPressed |= c.AltJump;
        selectPressed |= c.Drop;
    }


    // IMPORTANT: delegate to MENU_INPUT_SETTINGS. Only relevant when launched from game.
    if(MenuMode == MENU_INPUT_SETTINGS)
    {
        if(!upPressed && !downPressed && !leftPressed && !rightPressed && !menuDoPress && !menuBackPress && !delPressed && !selectPressed)
            MenuCursorCanMove = true;

        if(MenuCursorCanMove && upPressed)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursor--;
            MenuCursorCanMove = false;
        }

        if(MenuCursorCanMove && downPressed)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursor++;
            MenuCursorCanMove = false;
        }

        if(!SharedCursor.Primary && !SharedCursor.Secondary)
            MenuMouseRelease = true;

        if(menuControls_Logic() == -1)
        {
            Controls::SaveConfig();
            MenuMode = MENU_NEW_OPTIONS;
            MenuMouseRelease = false;
            controls_ready = false;
        }
        return false;
    }


    if(!upPressed && !downPressed && !leftPressed && !rightPressed && !menuDoPress && !menuBackPress && !delPressed && !selectPressed)
        controls_ready = true;
    else
        mouse_scroll_cooldown = -1;

    if(controls_ready && menuBackPress)
    {
        controls_ready = false;

        if(Back())
        {
            MenuCursorCanMove = false;
            MenuMouseRelease = false;
            return true;
        }
    }

    if(controls_ready && menuDoPress)
    {
        controls_ready = false;
        Do();
    }

    if(controls_ready && leftPressed)
    {
        controls_ready = false;
        RotateLeft();
    }

    if(controls_ready && rightPressed)
    {
        controls_ready = false;
        RotateRight();
    }

    if(controls_ready && delPressed)
    {
        controls_ready = false;
        Delete();
    }

    if(controls_ready && selectPressed)
    {
        controls_ready = false;
        Select();
    }

    if(controls_ready && upPressed)
    {
        controls_ready = false;
        if(cur_item == 0)
            cur_item = num_items;
        cur_item--;

        if(is_subsection(cur_item))
        {
            if(cur_item == 0)
                cur_item = num_items;
            cur_item--;
        }

        s_switch_item();
        PlaySoundMenu(SFX_Slide);
    }

    if(controls_ready && downPressed)
    {
        controls_ready = false;
        cur_item++;
        if(cur_item == num_items)
            cur_item = 0;

        if(is_subsection(cur_item))
        {
            cur_item++;
            if(cur_item == num_items)
                cur_item = 0;
        }

        s_switch_item();
        PlaySoundMenu(SFX_Slide);
    }

    return Mouse_Render(true, false);
}

} // namespace OptionsScreen
