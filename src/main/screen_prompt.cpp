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

#include "globals.h"
#include "config.h"
#include "gfx.h"
#include "sound.h"
#include "graphics.h"
#include "npc_id.h"
#include "controls.h"
#include "screen_fader.h"
#include "game_main.h"

#include "main/game_globals.h"

#include <Logger/logger.h>

#include "core/render.h"
#include "main/screen_prompt.h"

namespace PromptScreen
{

static int8_t s_cur_item = 0;
static std::vector<std::string> s_options;
static int s_longest_width = 0;

int Run(const std::string& prompt, std::vector<std::string>&& options)
{
    // initialize text prompt
    MessageText = prompt;
    BuildUTF8CharMap(MessageText, MessageTextMap);

    // initialize options
    s_options = options;

    // perform the pause loop
    PauseGame(PauseCode::Prompt, 0);
    MenuCursorCanMove = false;
    MenuMouseRelease = false;
    MouseRelease = false;
    ScrollRelease = false;

    for(int i = 1; i <= numPlayers; i++)
    {
        Player[i].UnStart = false;
        Player[i].CanJump = false;
        Player[i].CanAltJump = false;
    }

    return s_cur_item;
}

void Init()
{
    PlayMusic("wmusic12", 0);
    PlaySoundMenu(SFX_Message);
    s_cur_item = 0;
    MenuCursorCanMove = false;

    // set the longest width
    s_longest_width = 0;

    for(const std::string& label : s_options)
    {
        int item_width = SuperTextPixLen(label, 3);
        if(item_width > s_longest_width)
            s_longest_width = item_width;
    }

    int total_menu_height = s_options.size() * 36 - 18;
    int total_menu_width = s_longest_width + 40;

    // GBA bounds
    if(total_menu_height > 200 || total_menu_width > 480)
        pLogDebug("Prompt options don't fit within bounds (actual size %dx%d, bounds 480x200)", total_menu_width, total_menu_height);

    // setup fader
    if(g_config.EnableInterLevelFade)
        g_levelScreenFader.setupFader(3, 65, 0, ScreenFader::S_CIRCLE, false, ScreenW / 2, ScreenH / 2, 0);
}

void Render()
{
    XRender::setTargetTexture();

    // reset screen
    XRender::clearBuffer();

    // display background
    vScreen[1].X = 0;
    vScreen[1].Y = 0;
    vScreen[1].Left = 0; vScreen[1].Top = 0; vScreen[1].Width = ScreenW; vScreen[1].Height = ScreenH;
    level[1] = newLoc(0, 0, ScreenW, ScreenH);
    LevelREAL[1] = newLoc(0, 0, ScreenW, ScreenH);
    Background2[1] = 1;
    DrawBackground(1, 1);

    // draw message box
    DrawMessage(MessageTextMap);

    // (from screen_pause.cpp)
    // draw menu box

    // height includes intermediate padding but no top/bottom padding
    // width includes cursor on left and 20px padding on right for symmetry
    int total_menu_height = s_options.size() * 36 - 18;
    int total_menu_width = s_longest_width + 40;

    // enforce GBA bounds (480x320)
    if(total_menu_height > 320)
        total_menu_height = 320;

    if(total_menu_width > 480)
        total_menu_width = 480;

    int menu_box_height = total_menu_height + 32;

    int menu_box_width = 160;
    if(menu_box_width - total_menu_width < 32)
        menu_box_width = total_menu_width + 32;

    int menu_box_Y = ScreenH - 24 - menu_box_height;

    int menu_left_X = ScreenW / 2 - total_menu_width / 2 + 20;
    int menu_top_Y = menu_box_Y + menu_box_height / 2 - total_menu_height / 2;

    XRender::renderRect(ScreenW / 2 - menu_box_width / 2, menu_box_Y, menu_box_width, menu_box_height, 0, 0, 0, 0.8);

    for(size_t i = 0; i < s_options.size(); i++)
        SuperPrint(s_options[i], 3, menu_left_X, menu_top_Y + (i * 36));

    if(GFX.PCursor.inited)
        XRender::renderTexture(menu_left_X - 20, menu_top_Y + (s_cur_item * 36), GFX.PCursor);
    else
        XRender::renderTextureFL(menu_left_X - 20, menu_top_Y + (s_cur_item * 36), GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, 90.0, nullptr, X_FLIP_NONE);


    // draw screen fader and repaint

    g_levelScreenFader.draw();

    XRender::repaint();

    if(TakeScreen)
        ScreenShot();
}

bool Logic()
{
    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;

    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t &c = Player[i+1].Controls;

        menuDoPress |= c.Start || c.Jump;

        upPressed |= c.Up;
        downPressed |= c.Down;
    }

    if(!upPressed && !downPressed && !menuDoPress)
        MenuCursorCanMove = true;

    if(!MenuCursorCanMove)
        return false;

    if(upPressed)
    {
        PlaySoundMenu(SFX_Slide);
        s_cur_item--;
        if(s_cur_item < 0)
            s_cur_item = s_options.size() - 1;
        MenuCursorCanMove = false;
        return false;
    }

    if(downPressed)
    {
        PlaySoundMenu(SFX_Slide);
        s_cur_item++;
        if(s_cur_item >= (int)s_options.size())
            s_cur_item = 0;
        MenuCursorCanMove = false;
        return false;
    }

    if(menuDoPress && s_cur_item >= 0 && s_cur_item <= 3)
    {
        PlaySoundMenu(SFX_Do);

        if(g_config.EnableInterLevelFade)
            g_levelScreenFader.setupFader(3, 0, 65, ScreenFader::S_CIRCLE, false, ScreenW / 2, ScreenH / 2, 0);
        else
            g_levelScreenFader.setupFader(65, 0, 65, ScreenFader::S_CIRCLE, false, ScreenW / 2, ScreenH / 2, 0);

        editorWaitForFade();
        return true;
    }

    g_levelScreenFader.update();

    return false;
}

}; // namespace PromptScreen
