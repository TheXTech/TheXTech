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


#include <cstdint>
#include <string>

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "fontman/font_manager.h"

#include "core/render.h"

#include "main/hints.h"
#include "main/speedrunner.h"
#include "main/translate.h"
#include "main/trees.h"
#include "game_main.h"

#include "std_picture.h"
#include "screen.h"
#include "globals.h"
#include "player.h"
#include "npc_id.h"
#include "npc_traits.h"
#include "npc/npc_queues.h"
#include "config.h"
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
    XRender::renderTextureBasic(x + 96 / 3 - tex.w / 3, y + 96 - 34 - 4 - tex.h / 2, tex.w, tex.h / 2, tex, 0, tex.h / 2);
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

static void s_draw_no_lives_new(int x, int y)
{
    DrawLives(x + 40, y + 48 - 8, Lives, g_100s);
}

static void s_draw_no_lives_old(int x, int y)
{
    StdPicture &tex = GFXNPC[NPCID_POISON];

    int frame_h = NPCTraits[NPCID_POISON].HeightGFX;
    if(frame_h == 0)
        frame_h = NPCTraits[NPCID_POISON].THeight;

    XRender::renderTextureBasic(x + 48 - tex.w / 2, y + 48 - frame_h / 2,
        tex.w, frame_h, tex, 0, 0);
}

static void s_draw_rainbow_surf(int x, int y)
{
    Controls_t c;
    c.Down = true;

    if(Player[1].Direction < 0)
        c.Left = true;
    else
        c.Right = true;

    if(GamePaused == PauseCode::None || CommonFrame % 128 < 64)
        c.Run = true;

    RenderControls(c, x + 96 / 2 - 76 / 2, y + 96 - 34, 76, 30, false, 255);

    StdPicture& tex = GFXNPC[NPCID_FLIPPED_RAINBOW_SHELL];

    int frame_h = NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].HeightGFX;
    if(frame_h == 0)
        frame_h = NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].THeight;
    int frame_idx = ((CommonFrame % 64) / 4);

    XRender::renderTextureBasic(x + 96 / 2 - tex.w / 2, y + 96 - 34 - 4 - frame_h, tex.w, frame_h, tex, 0, frame_h * frame_idx);
}

static void s_draw_char5_bombs(int x, int y)
{
    DrawPlayerRaw(x + 10, y + 96 - 56, 5, 2, 8, 1);

    StdPicture& tex = GFXNPC[NPCID_BOMB];
    XRender::renderTextureBasic(x + 96 + 4 - NPCTraits[NPCID_BOMB].WidthGFX, y + 96 - 8 - NPCTraits[NPCID_BOMB].HeightGFX, NPCTraits[NPCID_BOMB].WidthGFX, NPCTraits[NPCID_BOMB].HeightGFX, tex, 0, 0);
}

static void s_draw_shoe_block(int x, int y)
{
    StdPicture& plant = GFXNPC[NPCID_FIRE_PLANT];
    XRender::renderTextureBasic(x + 96 - 4 - 32, y + 96 - 8 - NPCTraits[NPCID_FIRE_PLANT].THeight, 32, NPCTraits[NPCID_FIRE_PLANT].THeight, plant, 0, 0);

    StdPicture& fire = GFXNPC[NPCID_PLANT_FIRE];
    XRender::renderTextureBasic(x + 96 - 56, y + 96 - 56, 32, NPCTraits[NPCID_PLANT_FIRE].THeight, fire, 0, 0);

    DrawPlayerRaw(x + 8 + 4, y + 96 - 8 - 32 - 22, 3, 1, 1, 1);

    StdPicture& boot = GFXNPC[NPCID_RED_BOOT];
    XRender::renderTextureBasic(x + 8, y + 96 - 8 - 32, 32, 32, boot, 0, 64);
}

static void s_draw_heavy_duck(int x, int y)
{
    StdPicture& plant = GFXNPC[NPCID_FIRE_PLANT];
    XRender::renderTextureBasic(x + 96 - 4 - 32, y + 96 - 8 - NPCTraits[NPCID_FIRE_PLANT].THeight, 32, NPCTraits[NPCID_FIRE_PLANT].THeight, plant, 0, 0);

    StdPicture& fire = GFXNPC[NPCID_PLANT_FIRE];
    XRender::renderTextureBasic(x + 96 - 56, y + 96 - 56, 32, NPCTraits[NPCID_PLANT_FIRE].THeight, fire, 0, 0);

    DrawPlayerRaw(x + 8, y + 96 - 56, 1, 6, 7, 1);
}

static void s_draw_gray_bricks(int x, int y)
{
    StdPicture& brick = GFXBlock[457];
    XRender::renderTextureBasic(x + 24 + 4, y + 96 - 8 - 32, 32, 32, brick, 0, 0);

    StdPicture& st = GFXNPC[NPCID_STATUE_POWER];
    XRender::renderTextureBasic(x + 4, y + 4, 32, 32, st, 0, 0);

    StdPicture& heavy = GFXNPC[NPCID_HEAVY_POWER];
    XRender::renderTextureBasic(x + 96 - 4 - 32, y + 96 - 16 - 64, 32, 32, heavy, 0, 0);
}

static bool s_purple_pet_present()
{
    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 3 && Player[A].MountType == 6)
            return true;
    }

    return false;
}

static uint8_t s_altrun_pound_applies()
{
    if(!g_config.pound_by_alt_run)
        return 0;

    if(!s_purple_pet_present())
        return 0;

    return 108;
}

static uint8_t s_down_pound_applies()
{
    if(g_config.pound_by_alt_run)
        return 0;

    if(!s_purple_pet_present())
        return 0;

    return 108;
}

static uint8_t s_no_lives_new_applies()
{
    if(!g_config.modern_lives_system)
        return 0;

    if(g_100s == 0)
        return 100;

    return 0;
}

static uint8_t s_no_lives_old_applies()
{
    if(g_config.modern_lives_system)
        return 0;

    if(Lives == 0)
        return 100;

    return 0;
}

static uint8_t s_rainbow_surf_applies()
{
    if(LevelSelect)
        return 0;

    for(NPCRef_t n : NPCQueues::Active.no_change)
    {
        if(n->Type == NPCID_FLIPPED_RAINBOW_SHELL && n->TimeLeft > 2)
            return 107;
    }

    return 0;
}

static uint8_t s_char5_bombs_applies()
{
    if(LevelSelect)
        return 0;

    bool has_char5 = false;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Character == 5)
        {
            if(Player[A].Bombs > 0)
                return 102;
            else
            {
                has_char5 = true;
                break;
            }
        }
    }

    if(!has_char5)
        return 0;

    for(NPCRef_t n : NPCQueues::Active.no_change)
    {
        if(n->Type == NPCID_BOMB && n->TimeLeft > 2)
            return 102;
    }

    return 0;
}

static uint8_t s_shoe_block_applies()
{
    if(LevelSelect)
        return 0;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 1 && Player[A].MountType == 2)
            return 104;
    }

    return 0;
}

static uint8_t s_heavy_duck_applies()
{
    if(LevelSelect)
        return 0;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].State == 6)
            return 101;
    }

    return 0;
}

static uint8_t s_gray_bricks_applies()
{
    if(LevelSelect)
        return 0;

    int found = 0;
    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].State == 5 || Player[A].State == 6)
        {
            found = A;
            break;
        }
    }

    if(!found)
        return 0;

#ifndef LOW_MEM
    const vScreen_t& vscreen = vScreenByPlayer(found);
    for(const Block_t& b : treeBlockQuery(newLoc(-vscreen.X, -vscreen.Y, vscreen.Width, vscreen.Height), SORTMODE_NONE))
    {
        if(b.Type == 457 && !b.Hidden && !b.Invis)
            return 106;
    }
#endif

    return 30;
}

static const Hint s_hints[] = {
    {"Press to pound downwards!",                               "pound-key",    s_altrun_pound_applies, s_draw_purple_pet_altrun},
    {"Press to pound downwards!",                               "pound-key",    s_down_pound_applies,   s_draw_purple_pet_down},
    {"No coins left? Take a loan! Just watch your score...",    "no-lives-new", s_no_lives_new_applies, s_draw_no_lives_new},
    {"Be careful - Game Over is imminent!",                     "no-lives-old", s_no_lives_old_applies, s_draw_no_lives_old},
    {"Grab, run, hold down, and let go to surf.",               "rainbow-surf", s_rainbow_surf_applies, s_draw_rainbow_surf},
    {"Press Run to collect and Alt Run to throw.",              "char5-bombs",  s_char5_bombs_applies,  s_draw_char5_bombs},
    {"Duck to block most - but not all - flames!",              "heavy-duck",   s_heavy_duck_applies,   s_draw_heavy_duck},
    {"Wearing this blocks most - but not all - flames!",        "shoe-block",   s_shoe_block_applies,   s_draw_shoe_block},
    {"Some blocks are vulnerable to special powers.",           "gray-bricks",  s_gray_bricks_applies,  s_draw_gray_bricks},
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
    if(BattleMode || s_select_hint == -1 || s_hint_text_index[s_select_hint] >= (int)s_translated_hint_text.size())
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
