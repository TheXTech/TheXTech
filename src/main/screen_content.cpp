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

#include <memory>
#include <vector>

#include "core/render.h"

#include "graphics.h"
#include "graphics/gfx_marquee.h"
#include "gfx.h"

#include "main/menu_main.h"
#include "main/game_info.h"
#include "editor/new_editor.h"

#include "globals.h"
#include "controls.h"
#include "sound.h"
#include "config.h"

void menu_draw_infobox_switch_arrows(int infobox_x, int infobox_y);

namespace ContentSelectScreen
{

struct Item
{
    int m_index = -1;
    MarqueeState m_title_marquee_state;
    MarqueeState m_author_marquee_state;

    const SelectWorld_t* operator->() const;
};

static std::vector<Item> s_items;
static int s_current_item = 0;
static int s_recent_item = -1;

static int ScrollDelay = 0;
static int minShow = 0;
static int maxShow = 0;

static int worldCurs = 0;

bool editor_target_thextech = true;


// Implementation for Item
const SelectWorld_t* Item::operator->() const
{
    const std::vector<SelectWorld_t>& SelectorList
        = (MenuMode == MENU_BATTLE_MODE) ? SelectBattle :
            SelectWorld;

    if(m_index > 0 && m_index < (int)SelectorList.size())
        return &SelectorList[m_index];
    else if(SelectorList.size() > 0)
        return &SelectorList[0];
    else
        return nullptr;
}


// Implementation for global routines

void Prepare()
{
    s_items.clear();

    const std::vector<SelectWorld_t>& SelectorList
        = (MenuMode == MENU_BATTLE_MODE) ? SelectBattle :
            SelectWorld;

    size_t editor_entries_start = SelectorList.size() - 2;
    size_t editor_battle_levels = editor_entries_start;
    // size_t editor_new_world = SelectorList.size() - 1;

    for(size_t i = 1; i < SelectorList.size(); i++)
    {
        // filtering condition can go here
        if(MenuMode == MENU_EDITOR)
        {
            if(!SelectorList[i].editable)
                continue;

            if(g_gameInfo.disableBattleMode && i == editor_battle_levels)
                continue;
        }
        // skip the special editor worlds if not in editor
        else if(MenuMode != MENU_BATTLE_MODE && i >= editor_entries_start)
            continue;

        s_items.emplace_back();
        s_items.back().m_index = i;

        // load thumbnail here
    }

    // find recent item
    s_recent_item = -1;

    for(size_t i = 0; i < s_items.size(); ++i)
    {
        auto &w = SelectorList[s_items[i].m_index];
        const std::string& wPath = w.WorldFilePath;

        if((MenuMode == MENU_1PLAYER_GAME && wPath == g_recentWorld1p) ||
           (MenuMode == MENU_2PLAYER_GAME && wPath == g_recentWorld2p) ||
           (MenuMode == MENU_EDITOR && wPath == g_recentWorldEditor))
        {
            s_recent_item = (int)i;
        }
    }

    if(s_recent_item >= 0)
    {
        s_current_item = s_recent_item;
        worldCurs = s_recent_item - 2;
    }
    else
    {
        s_current_item = 0;
        worldCurs = 0;
    }
}

int Logic()
{
    bool upPressed = l_SharedControls.MenuUp;
    bool downPressed = l_SharedControls.MenuDown;
    bool leftPressed = l_SharedControls.MenuLeft;
    bool rightPressed = l_SharedControls.MenuRight;
    bool homePressed = SharedCursor.Tertiary;

    bool menuDoPress = l_SharedControls.MenuDo || l_SharedControls.Pause;
    bool menuBackPress = l_SharedControls.MenuBack;

    bool altPressed = false;

    for(int i = 0; i < l_screen->player_count; i++)
    {
        Controls_t &c = Controls::g_RawControls[i];

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;

        homePressed |= c.Drop;
        altPressed |= c.AltJump;
    }

    menuBackPress |= SharedCursor.Secondary && MenuMouseRelease;

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

    int MenuX, MenuY;
    GetMenuPos(&MenuX, &MenuY);

    if(ScrollDelay > 0)
    {
        SharedCursor.Move = true;
        ScrollDelay -= 1;
    }

    if(SharedCursor.Move)
    {
        int B = 0;

        for(int A = minShow; A < maxShow; A++)
        {
            if(SharedCursor.Y >= MenuY + B * 30 && SharedCursor.Y <= MenuY + B * 30 + 16)
            {
                int menuLen = 19 * static_cast<int>(s_items[A]->WorldName.size());

                if(SharedCursor.X >= MenuX && SharedCursor.X <= MenuX + menuLen)
                {
                    if(MenuMouseRelease && SharedCursor.Primary)
                        MenuMouseClick = true;

                    if(s_current_item != A && ScrollDelay == 0)
                    {
                        ScrollDelay = 10;
                        PlaySoundMenu(SFX_Slide);
                        s_current_item = A;
                    }
                }
            }

            B += 1;
        }
    }

    if(MenuCursorCanMove || MenuMouseClick)
    {
        if(menuBackPress)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursorCanMove = false;
            return -1;
        }
        else if(menuDoPress || MenuMouseClick)
        {
            bool disabled = false;
            // Save menu state
            // listMenuLastScroll = worldCurs;
            // listMenuLastCursor = s_current_item;

            selWorld = s_items[s_current_item].m_index;

            MenuCursorCanMove = false;

            if(s_items[s_current_item]->disabled)
                disabled = true;

            if(!disabled)
                PlaySoundMenu(SFX_Do);

            if(disabled)
            {
                PlaySoundMenu(SFX_BlockHit);
                // Do nothing. stay at menu
            }
            // advance to next menu
            else
                return 1;
        }
    }

    // New world select scroll options!
    // Based on Wohlstand's but somewhat simpler and less keyboard-specific.
    // Left and right are -/+ 3 (repeatable, so they also provide a quick-first/quick-last function).
    // DropItem / Tertiary cursor button is return to last episode.
    bool dontWrap = false;

    if(SharedCursor.ScrollUp || SharedCursor.ScrollDown)
    {
        PlaySoundMenu(SFX_Saw);
        dontWrap = true;

        if(SharedCursor.ScrollUp)
            s_current_item -= 1;
        else
            s_current_item += 1;
    }
    else if(leftPressed || rightPressed || upPressed || downPressed)
    {
        if(MenuCursorCanMove || ScrollDelay == 0)
        {
            if(leftPressed || rightPressed)
            {
                int first_new_content = (g_gameInfo.disableBattleMode) ? s_items.size() - 1 : s_items.size() - 2;

                if(MenuMode == MENU_EDITOR && s_current_item >= first_new_content)
                {
                    if(MenuCursorCanMove)
                    {
                        editor_target_thextech = !editor_target_thextech;
                        PlaySoundMenu(SFX_Climbing);
                        ScrollDelay = -1;
                    }
                }
                else
                {
                    PlaySoundMenu(SFX_Saw);
                    ScrollDelay = 15;
                    dontWrap = true;

                    if(leftPressed)
                        s_current_item -= 3;
                    else
                        s_current_item += 3;
                }
            }
            else
            {
                PlaySoundMenu(SFX_Slide);
                ScrollDelay = -1;

                if(upPressed)
                    s_current_item -= 1;
                else
                    s_current_item += 1;
            }

            MenuCursorCanMove = false;
        }
    }

    if(homePressed && MenuCursorCanMove && s_recent_item >= 0)
    {
        PlaySoundMenu(SFX_Camera);
        s_current_item = s_recent_item;
        MenuCursorCanMove = false;
        dontWrap = true;
    }

    if(dontWrap)
    {
        if(s_current_item >= (int)s_items.size())
            s_current_item = (int)s_items.size() - 1;
        if(s_current_item < 0)
            s_current_item = 0;
    }
    else
    {
        if(s_current_item >= (int)s_items.size())
            s_current_item = 0;
        if(s_current_item < 0)
            s_current_item = (int)s_items.size() - 1;
    }

    return 0;
}

void Render()
{
    int MenuX, MenuY;
    GetMenuPos(&MenuX, &MenuY);

    // std::string tempStr;

    minShow = 0;
    maxShow = s_items.size();

    int original_maxShow = maxShow;
    if(maxShow > 5)
    {
        minShow = worldCurs;
        maxShow = minShow + 5;

        if(s_current_item <= minShow)
            worldCurs -= 1;

        if(s_current_item >= maxShow - 1)
            worldCurs += 1;

        if(worldCurs < 0)
            worldCurs = 0;

        if(worldCurs > original_maxShow - 5)
            worldCurs = original_maxShow - 5;

        minShow = worldCurs;
        maxShow = minShow + 5;
    }

    for(auto A = minShow; A < maxShow; A++)
    {
        const Item& w = s_items[A];

        XTColor color;
        color.r = (A == s_recent_item) ? 0 : 255;

        if(w->disabled)
            color = {127, 127, 127};

        if(w->probably_incompatible)
            color = {255, 127, 127};

        int B = A - minShow + 1;

        SuperPrint(w->WorldName, 3, MenuX, MenuY - 30 + (B * 30), color);
    }

    // preview type of content being created
    int first_new_content = (g_gameInfo.disableBattleMode) ? s_items.size() - 1 : s_items.size() - 2;
    if(MenuMode == MENU_EDITOR && s_current_item >= first_new_content)
    {
        int B = s_current_item - minShow + 1;

        // draw mode icon
        int mode_icon_X = MenuX + 340;
        int mode_icon_Y = MenuY - 34 + (B * 30);

        uint16_t rot = SDL_abs((int)(CommonFrame % 64) - 32);
        rot = rot - 16;

        if(!editor_target_thextech)
            XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFXNPC[NPCID_POWER_S3], 0, 0, 32, 32, rot, nullptr, X_FLIP_NONE);
        else if(GFX.EIcons.inited)
            XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFX.EIcons, 0, 32*Icon::thextech, 32, 32, rot, nullptr, X_FLIP_NONE);
        else
            XRender::renderTextureScaleEx(mode_icon_X, mode_icon_Y, 24, 24, GFXNPC[NPCID_ICE_POWER_S3], 0, 0, 32, 32, rot, nullptr, X_FLIP_NONE);

        // draw infobox
        int infobox_x = XRender::TargetW / 2 - 240;
        int infobox_y = MenuY + 145;

        XRender::renderRect(infobox_x, infobox_y, 480, 68, {0, 0, 0, 192});

        XTColor color;

        if(editor_target_thextech)
            color = XTColorF(0.5_n, 0.8_n, 1.0_n);
        else
            color = XTColorF(1.0_n, 0.5_n, 0.5_n);

        SuperPrintScreenCenter(g_mainMenu.editorMakeFor, 3, infobox_y + 14, color);

        if(editor_target_thextech)
            SuperPrintScreenCenter("TheXTech", 3, infobox_y + 34, color);
        else
            SuperPrintScreenCenter("SMBX 1.3", 3, infobox_y + 34, color);

        menu_draw_infobox_switch_arrows(infobox_x, infobox_y);
    }

    // render the scroll indicators
    if(minShow > 0)
        XRender::renderTextureBasic(XRender::TargetW/2 - 8, MenuY - 20, GFX.MCursor[1]);

    if(maxShow < original_maxShow)
        XRender::renderTextureBasic(XRender::TargetW/2 - 8, MenuY + 140, GFX.MCursor[2]);

    int B = s_current_item - minShow;

    if(B >= 0 && B < 5)
        XRender::renderTextureBasic(MenuX - 20, MenuY + (B * 30), GFX.MCursor[0]);
}


} // namespace ContentSelectScreen
