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


#include <cstdint>
#include <string>

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "fontman/font_manager.h"

#include "core/render.h"

#include "main/hints.h"
#include "main/speedrunner.h"
#include "main/translate.h"

#include "std_picture.h"
#include "screen.h"
#include "globals.h"
#include "npc_id.h"
#include "compat.h"
#include "controls.h"
#include "graphics.h"


namespace XHints
{

struct Hint
{
    const char* text_en;
    const char* tr_id;
    uint8_t (&check_priority)();
    void (&draw_icon)(int, int);

    Hint(const char* text_en, const char* tr_id, uint8_t (&check_priority)(), void (&draw_icon)(int, int))
        : text_en(text_en), tr_id(tr_id), check_priority(check_priority), draw_icon(draw_icon) {}
};


// default hints defined here

static void s_draw_purple_pet(const Controls_t& c, int x, int y)
{
    RenderControls(c, x + 96 / 2 - 76 / 2, y + 96 - 34, 76, 30, false, 255);

    StdPicture& tex = GFXNPC[NPCID_PET_PURPLE];
    XRender::renderTexture(x + 96 / 3 - tex.w / 3, y + 96 - 34 - 4 - tex.h / 2, tex.w, tex.h / 2, tex, 0, tex.h / 2);
}

static void s_draw_purple_pet_down(int x, int y)
{
    Controls_t c;
    c.Down = true;

    s_draw_purple_pet(c, x, y);
}

static void s_draw_purple_pet_altrun(int x, int y)
{
    Controls_t c;
    c.AltRun = true;

    s_draw_purple_pet(c, x, y);
}

static void s_draw_no_lives(int x, int y)
{
    DrawLives(x + 40, y + 48 - 8, Lives, g_100s);
}

static bool s_purple_pet_present()
{
    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 3 && Player[A].MountType == 6)
            return true;
    }

    if(LevelSelect)
        return false;

    for(int A = 1; A <= numNPCs; A++)
    {
        int type = NPC[A].Type;

        if(type == NPCID_ITEM_BURIED || type == NPCID_ITEM_POD ||
           type == NPCID_ITEM_BUBBLE || type == NPCID_ITEM_THROWER)
        {
            type = int(NPC[A].Special);
        }

        if(type == NPCID_PET_PURPLE)
            return true;
    }

    for(int A = 1; A <= numBlock; A++)
    {
        if(Block[A].Special != 0 && Block[A].Special == 1000 + NPCID_PET_PURPLE)
            return true;
    }

    return false;
}

static uint8_t s_altrun_pound_applies()
{
    if(!g_compatibility.pound_by_alt_run)
        return 0;

    if(!s_purple_pet_present())
        return 0;

    return 108;
}

static uint8_t s_down_pound_applies()
{
    if(g_compatibility.pound_by_alt_run)
        return 0;

    if(!s_purple_pet_present())
        return 0;

    return 108;
}

static uint8_t s_no_lives_new()
{
    if(!g_compatibility.modern_lives_system)
        return 0;

    if(g_100s == 0)
        return 100;
    else if(g_100s < 0)
        return 8;

    return 0;
}

static uint8_t s_no_lives_old()
{
    if(g_compatibility.modern_lives_system)
        return 0;

    if(Lives == 0)
        return 100;

    return 0;
}

static const Hint s_hints[] = {
    {"Press to pound downwards!", "pound-key", s_altrun_pound_applies, s_draw_purple_pet_altrun},
    {"Press to pound downwards!", "pound-key", s_down_pound_applies,   s_draw_purple_pet_down},
    {"If you fail, your score will reset.",   "no-lives-new", s_no_lives_new, s_draw_no_lives},
    {"If you fail, the game will end.", "no-lives-old", s_no_lives_old, s_draw_no_lives},
};

static constexpr size_t s_hint_count = sizeof(s_hints) / sizeof(Hint);


// library state defined here

//! translatable strings for hints
static std::vector<std::string> s_translated_hint_text;

//! index into s_translated_hint_text for each hint (for case of duplicated keys)
static std::array<int, s_hint_count> s_hint_text_index;

//! currently loaded hint
static int s_select_hint = -1;
static uint8_t s_select_hint_priority = 0;


// main library functions here

/**
 * \brief idempotent; prepares hint strings for translation and use
 **/
static void Init()
{
    if(!s_translated_hint_text.empty())
        return;

    for(size_t i = 0; i < s_hint_count; i++)
    {
        // check if a duplicate
        size_t use_idx = 0;
        for(; use_idx < i; use_idx++)
        {
            if(SDL_strcmp(s_hints[i].tr_id, s_hints[use_idx].tr_id) == 0)
                break;
        }

        // if not duplicate, add unique string
        if(use_idx == i)
        {
            s_hint_text_index[i] = s_translated_hint_text.size();
            s_translated_hint_text.push_back(s_hints[i].text_en);
        }
        // otherwise, refer to existing string
        else
            s_hint_text_index[i] = s_hint_text_index[use_idx];
    }
}

void ResetStrings()
{
    Init();

    for(size_t i = s_hint_count; i != 0;)
    {
        --i;

        if(s_hint_text_index[i] >= (int)s_translated_hint_text.size())
            continue;

        s_translated_hint_text[s_hint_text_index[i]] = s_hints[i].text_en;
    }
}

void InitTranslations(XTechTranslate& translate)
{
    Init();

    int top_index = -1;

    for(size_t i = 0; i < s_hint_count; i++)
    {
        if(s_hint_text_index[i] <= top_index)
            continue;

        SDL_assert(s_hint_text_index[i] == top_index + 1); // no hints should be skipped

        top_index = s_hint_text_index[i];

        translate.m_engineMap.insert({fmt::format_ne("game.hint.{0}", s_hints[i].tr_id), &(s_translated_hint_text[top_index])});
    }
}

void Select()
{
    Init();

    s_select_hint = -1;
    s_select_hint_priority = 0;

    for(size_t hint_i = 0; hint_i < s_hint_count; hint_i++)
    {
        const Hint& hint = s_hints[hint_i];

        uint8_t hint_priority = hint.check_priority();

        if(hint_priority <= s_select_hint_priority)
            continue;

        s_select_hint = (int)hint_i;
        s_select_hint_priority = hint_priority;
    }
}

void Draw(int top, int min_priority, int width)
{
    if(s_select_hint == -1 || s_hint_text_index[s_select_hint] >= (int)s_translated_hint_text.size())
        return;

    const Hint& hint = s_hints[s_select_hint];
    const std::string& tr_hint = s_translated_hint_text[s_hint_text_index[s_select_hint]];

    if(s_select_hint_priority < min_priority)
        return;

    constexpr int hint_box_height = 96;
    const int hint_box_width = width;
    constexpr int text_start = 110;

    int hint_box_left = XRender::TargetW / 2 - hint_box_width / 2;

    XRender::renderRect(hint_box_left -  4, top    , hint_box_width + 8, hint_box_height + 8, {  0,   0,   0});
    XRender::renderRect(hint_box_left -  2, top + 2, hint_box_width + 4, hint_box_height + 4, {255, 255, 255});
    XRender::renderRect(hint_box_left     , top + 4, hint_box_width    , hint_box_height    , {  0,   0,   0});
    XRender::renderRect(hint_box_left + 96, top + 4,                  2, hint_box_height    , {255, 255, 255});

    // draw icon
    hint.draw_icon(hint_box_left, top + 4);

    // draw text
    std::string text = tr_hint;
    PGE_Size text_size = FontManager::optimizeTextPx(text, hint_box_width - 2 - text_start, FontManager::fontIdFromSmbxFont(4));

    FontManager::printText(text.c_str(), text.size(),
                           hint_box_left + text_start,
                           top + hint_box_height / 2 - text_size.h() / 2,
                           FontManager::fontIdFromSmbxFont(4));
}

} // namespace XHints
