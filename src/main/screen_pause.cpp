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

#include <vector>
#include <string>

#ifdef THEXTECH_INTERPROC_SUPPORTED
#include <InterProcess/intproc.h>
#endif // #ifdef THEXTECH_INTERPROC_SUPPORTED

#include <Logger/logger.h>

#include "../globals.h"

#include "../player.h"
#include "../sound.h"
#include "../gfx.h"
#include "../graphics.h"
#include "../frm_main.h"
#include "../config.h"
#include "../core/render.h"
#include "../core/events.h"

#include "../game_main.h"
#include "menu_main.h"
#include "screen_pause.h"
#include "screen_textentry.h"
#include "speedrunner.h"
#include "cheat_code.h"

#include "main/game_strings.h"
#include "main/level_medals.h"
#include "main/hints.h"

#include "controls.h"

#include "editor.h"

namespace PauseScreen
{

struct MenuItem
{
    std::string name;
    bool (*callback)();
    MenuItem(const std::string &n, bool(*cb)()) : name(n), callback(cb) {}
};

enum class PauseType
{
    Modern,
    Testing,
    Legacy,
};

static PauseType s_pause_type = PauseType::Modern;
static int s_pause_plr = 0;
static int s_longest_width = 0;
static std::vector<MenuItem> s_items;
static int s_cheat_menu_bits = 0;

static bool s_Continue()
{
    PlaySound(SFX_Pause);
    return true;
}

static bool s_RestartLevel()
{
    MenuMode = MENU_INTRO;
    MenuCursor = 0;
    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    EndLevel = true;
    LevelBeatCode = -2;
    StopMusic();
    XEvents::doEvents();
    return true;
}

static bool s_ResetCheckpoints()
{
    pLogDebug("Clear check-points from a menu");
    Checkpoint.clear();
    CheckpointsList.clear();
    g_curLevelMedals.reset_checkpoint();
    numStars = 0;
    Star.clear();
#ifdef THEXTECH_INTERPROC_SUPPORTED
    IntProc::sendStarsNumber(numStars);
#endif
    // numSavedEvents = 0;
    // BlockSwitch.fill(false);
    PlaySound(SFX_Bullet);
    return true;
}

static bool s_DropAddScreen()
{
    // don't allow Drop / Add screen in clone mode
    if(g_ClonedPlayerMode || SingleCoop)
    {
        PlaySoundMenu(SFX_BlockHit);
        return false;
    }

    PlaySound(SFX_Do);

    if(PauseGame(PauseCode::DropAdd, 0) == 1)
        return true;

    MenuCursorCanMove = false;
    return false;
}


static bool s_OptionsScreen()
{
    PlaySound(SFX_Do);

    PauseGame(PauseCode::Options, 0);
    MenuCursorCanMove = false;

    // re-initialize / re-translate pause menu
    SoundPause[SFX_Pause] = 1;
    Init(s_pause_plr, s_pause_type == PauseType::Legacy);

    // set MenuCursor correctly
    for(size_t i = 0; i < s_items.size(); i++)
    {
        if(s_items[i].callback == s_OptionsScreen)
        {
            MenuCursor = i;
            break;
        }
    }

    return false;
}

static bool s_CheatScreen()
{
    cheats_setBuffer(TextEntryScreen::Run(g_gameStrings.pauseItemEnterCode));
    // uncomment this if you want to return to the pause menu
    // MenuCursorCanMove = false;
    // return false;
    return true;
}

static bool s_QuitTesting()
{
    MenuMode = MENU_INTRO;
    MenuCursor = 0;
    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    EndLevel = true;
    LevelBeatCode = -1;
    StopMusic();
    XEvents::doEvents();

    if(Backup_FullFileName.empty())
    {
        GameIsActive = false; // Quit the game entirely
    }

    return true;
}

static bool s_SaveAndContinue()
{
    bool CanSave = (LevelSelect || IsHubLevel) && !Cheater;

    if(CanSave)
    {
        SaveGame();
        PlaySound(SFX_Checkpoint);
    }
    else
    {
        // player tried to cheat, scare them
        PlaySound(SFX_VillainKilled);
    }

    return true;
}

static bool s_Quit()
{
    bool CanSave = (LevelSelect || IsHubLevel) && !Cheater;

    if(CanSave)
        SaveGame(); // "Save & Quit"
    else
        speedRun_saveStats();

    s_cheat_menu_bits = 0;
    GameMenu = true;

    MenuMode = MENU_INTRO;
    MenuCursor = 0;

    if(!LevelSelect)
    {
        LevelSelect = true;
        EndLevel = true;
    }
    else
        LevelSelect = false;

    XRender::setTargetTexture();
    XRender::clearBuffer();
    XRender::repaint();
    StopMusic();
    XEvents::doEvents();

    return true;
}

void Init(int plr, bool LegacyPause)
{
    XHints::Select();

    PlaySound(SFX_Pause);
    MenuCursor = 0;
    MenuCursorCanMove = false;
    MenuCursorCanMove_Back = false;

    if(LegacyPause)
        s_pause_type = PauseType::Legacy;
    else if(TestLevel)
        s_pause_type = PauseType::Testing;
    else
        s_pause_type = PauseType::Modern;

    s_pause_plr = plr;
    if(s_cheat_menu_bits < 14)
        s_cheat_menu_bits = 0;

    // do a context-aware initialization of s_items
    s_items.clear();

    bool CanSave = (LevelSelect || IsHubLevel) && !Cheater && !TestLevel;

    // add pause menu items

    // level test
    if(s_pause_type == PauseType::Testing)
    {
        bool inter_screen = (LevelBeatCode <= -2);
        bool start_screen = (LevelBeatCode == -3);
        bool editor_test = !Backup_FullFileName.empty();

        if(!inter_screen)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemContinue, s_Continue});

        s_items.push_back(MenuItem{start_screen ? g_gameStrings.pauseItemContinue : g_gameStrings.pauseItemRestartLevel, s_RestartLevel});

        if(!start_screen && !BattleMode)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemResetCheckpoints, s_ResetCheckpoints});

        if(g_config.allow_drop_add)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemPlayerSetup, s_DropAddScreen});

        if(!inter_screen && s_cheat_menu_bits == 14 && !BattleMode)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemEnterCode, s_CheatScreen});

        s_items.push_back(MenuItem{g_mainMenu.mainOptions, s_OptionsScreen});

        s_items.push_back(MenuItem{editor_test ? g_gameStrings.pauseItemReturnToEditor : g_gameStrings.pauseItemQuitTesting, s_QuitTesting});
    }
    // main game pause
    else
    {
        s_items.push_back(MenuItem{g_gameStrings.pauseItemContinue, s_Continue});

        if(g_config.allow_drop_add && s_pause_type != PauseType::Legacy)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemPlayerSetup, s_DropAddScreen});

        if(s_cheat_menu_bits == 14 && s_pause_type != PauseType::Legacy && !BattleMode)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemEnterCode, s_CheatScreen});

        if(s_pause_type != PauseType::Legacy)
            s_items.push_back(MenuItem{g_mainMenu.mainOptions, s_OptionsScreen});

        if(CanSave)
        {
            s_items.push_back(MenuItem{g_gameStrings.pauseItemSaveAndContinue, s_SaveAndContinue});
            s_items.push_back(MenuItem{g_gameStrings.pauseItemSaveAndQuit, s_Quit});
        }
        else
        {
            s_items.push_back(MenuItem{g_gameStrings.pauseItemQuit, TestLevel ? s_QuitTesting : s_Quit});
        }
    }

    const int font = (s_pause_type == PauseType::Legacy) ? 3 : 4;

    // set the longest width
    s_longest_width = 0;

    for(size_t i = 0; i < s_items.size(); i++)
    {
        int item_width = SuperTextPixLen(s_items[i].name, font);
        if(item_width > s_longest_width)
            s_longest_width = item_width;
    }

    int total_menu_height = (int)s_items.size() * 36 - 18;
    int total_menu_width = s_longest_width + 40;

    // GBA bounds
    if(total_menu_height > 320 || total_menu_width > 480)
        pLogWarning("Menu doesn't fit within bounds (actual size %dx%d, bounds 480x320)", total_menu_width, total_menu_height);
}

void Render()
{
    // height includes intermediate padding but no top/bottom padding
    // width includes cursor on left and 20px padding on right for symmetry
    int total_menu_height = (int)s_items.size() * 36 - 18;
    int total_menu_width = s_longest_width + 40;

    // enforce GBA bounds (480x320)
    if(total_menu_height > 320)
        total_menu_height = 320;

    if(total_menu_width > 480)
        total_menu_width = 480;

    int menu_box_height = 200;
    int menu_box_width = 380;

    if(menu_box_height - total_menu_height < 18)
        menu_box_height = total_menu_height + 18;

    if(menu_box_width - total_menu_width < 32)
        menu_box_width = total_menu_width + 32;

    int menu_left_X = XRender::TargetW / 2 - total_menu_width / 2 + 20;
    int menu_top_Y = XRender::TargetH / 2 - total_menu_height / 2;

    switch(s_pause_type)
    {
    case(PauseType::Legacy):
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2, XRender::TargetH / 2 - menu_box_height / 2, menu_box_width, menu_box_height, {0, 0, 0});
        break;
    case(PauseType::Modern):
    default:
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2 - 4, XRender::TargetH / 2 - menu_box_height / 2 - 4, menu_box_width + 8, menu_box_height + 8, {0, 0, 0});
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2 - 2, XRender::TargetH / 2 - menu_box_height / 2 - 2, menu_box_width + 4, menu_box_height + 4, {255, 255, 255});
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2, XRender::TargetH / 2 - menu_box_height / 2, menu_box_width, menu_box_height, {0, 0, 0});
        break;
    case(PauseType::Testing):
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2, XRender::TargetH / 2 - menu_box_height / 2, menu_box_width, menu_box_height, {0, 0, 0, 127});
        break;
    }

    if(s_pause_type == PauseType::Testing)
    {
        for(size_t i = 0; i < s_items.size(); i++)
        {
            XTColor color = ((int)i == MenuCursor) ? XTColor() : XTColor(127, 127, 127);

            SuperPrintScreenCenter(s_items[i].name, 5, menu_top_Y + (i * 36), color);
        }
    }
    else
    {
        const int font = (s_pause_type == PauseType::Legacy) ? 3 : 4;

        for(size_t i = 0; i < s_items.size(); i++)
            SuperPrint(s_items[i].name, font, menu_left_X, menu_top_Y + (i * 36));

        if(GFX.PCursor.inited)
        {
            if(s_pause_plr == 2 && s_pause_type != PauseType::Legacy)
                XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor, {0, 255, 0});
            else if(s_pause_plr != 1 && s_pause_type != PauseType::Legacy)
                XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor);
            else
                XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor, {255, 0, 0});
        }
        else if(s_pause_plr == 2 && s_pause_type != PauseType::Legacy)
            XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), 16, 16, GFX.MCursor[3], 0, 0);
        else
            XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), 16, 16, GFX.MCursor[0], 0, 0);
    }

    if(XRender::TargetH > XRender::TargetH / 2 + menu_box_height / 2 + 16 + 96 + 8)
        XHints::Draw(XRender::TargetH / 2 + menu_box_height / 2 + 16, 100, menu_box_width);
}

bool Logic(int plr)
{
    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;
    bool leftPressed = SharedControls.MenuLeft;
    bool rightPressed = SharedControls.MenuRight;

    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack;

    // there was previously code to copy all players' controls from the main player, but this is no longer necessary (and actively harmful in the SingleCoop case)

    if(!g_config.multiplayer_pause_controls && plr == 0)
        plr = 1;

    if(plr == 0)
    {
        for(int i = 1; i <= numPlayers; i++)
        {
            const Controls_t& c = Player[i].Controls;

            menuDoPress |= (c.Start || c.Jump);
            menuBackPress |= c.Run;

            upPressed |= c.Up;
            downPressed |= c.Down;
            leftPressed |= c.Left;
            rightPressed |= c.Right;
        }
    }
    else
    {
        const Controls_t& c = Player[plr].Controls;

        menuDoPress |= (c.Start || c.Jump);
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;
    }

    if(!MenuCursorCanMove_Back)
    {
        if(!menuBackPress && MenuCursorCanMove)
            MenuCursorCanMove_Back = true;

        menuBackPress = false;
    }
    else if(menuBackPress)
        MenuCursorCanMove_Back = false;

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

    if(!MenuCursorCanMove)
    {
        if(!menuDoPress && !menuBackPress && !upPressed && !downPressed && (s_pause_type == PauseType::Legacy || (!leftPressed && !rightPressed)))
            MenuCursorCanMove = true;

        return false;
    }

    if((s_pause_type == PauseType::Legacy || BattleMode) && s_cheat_menu_bits < 14)
        s_cheat_menu_bits = 0;

    int max_item = (int)s_items.size() - 1;

    if(menuBackPress)
    {
        if(MenuCursor != max_item)
            PlaySound(SFX_Slide);

        MenuCursor = max_item;

        if(s_cheat_menu_bits < 14)
            s_cheat_menu_bits = 0;
    }
    else if(upPressed)
    {
        PlaySound(SFX_Slide);
        MenuCursor = MenuCursor - 1;
        MenuCursorCanMove = false;

        if(s_cheat_menu_bits < 14)
            s_cheat_menu_bits = 0;
    }
    else if(downPressed)
    {
        PlaySound(SFX_Slide);
        MenuCursor = MenuCursor + 1;
        MenuCursorCanMove = false;

        if(s_cheat_menu_bits < 14)
            s_cheat_menu_bits = 0;
    }
    else if(leftPressed && s_pause_type != PauseType::Legacy)
    {
        if(s_cheat_menu_bits == 0 || s_cheat_menu_bits == 2 || s_cheat_menu_bits == 5 || s_cheat_menu_bits == 9)
        {
            s_cheat_menu_bits++;
            MenuCursorCanMove = false;

            // don't swap char
            if(s_cheat_menu_bits >= 5)
            {
                PlaySound(SFX_Do);
                return false;
            }
        }
        else if(s_cheat_menu_bits < 14)
            s_cheat_menu_bits = 1;
    }
    else if(rightPressed && s_pause_type != PauseType::Legacy)
    {
        if(s_cheat_menu_bits != 0 && s_cheat_menu_bits != 2 && s_cheat_menu_bits != 5 && s_cheat_menu_bits != 9 && s_cheat_menu_bits < 14)
        {
            s_cheat_menu_bits++;
            MenuCursorCanMove = false;

            // do cheat screen
            if(s_cheat_menu_bits == 14)
            {
                PlaySound(SFX_MedalGet);
                return s_CheatScreen();
            }

            // don't swap char
            if(s_cheat_menu_bits >= 5)
            {
                PlaySound(SFX_Do);
                return false;
            }
        }
        else if(s_cheat_menu_bits < 14)
            s_cheat_menu_bits = 0;
    }
    else if(menuDoPress && s_cheat_menu_bits < 14)
        s_cheat_menu_bits = 0;

    if(MenuCursor < 0)
        MenuCursor = max_item;
    else if(MenuCursor > max_item)
        MenuCursor = 0;

    // special char change code
    if(SwapCharAllowed())
    {
        for(int A = 1; A <= numPlayers; A++)
        {
            if(!Player[A].RunRelease)
            {
                if(!Player[A].Controls.Left && !Player[A].Controls.Right)
                    Player[A].RunRelease = true;
            }
            else if(Player[A].Controls.Left || Player[A].Controls.Right)
            {
                AllCharBlock = 0;

                for(int B = 1; B <= numCharacters; B++)
                {
                    if(!blockCharacter[B])
                    {
                        if(AllCharBlock == 0)
                            AllCharBlock = B;
                        else
                        {
                            AllCharBlock = 0;
                            break;
                        }
                    }
                }

                Player[A].RunRelease = false;
                auto snd = SFX_BlockHit;

                if(AllCharBlock == 0 && numPlayers <= maxLocalPlayers && Player[A].Effect == PLREFF_NORMAL)
                {
                    // replaced old character swap code with this new code,
                    // supporting arbitrary multiplayer and in-level swap.
                    int target = Player[A].Character;

                    // do the full wrap-around to find an acceptable target
                    for(int i = 0; i < 5; i++)
                    {
                        // move the target in the direction requested by the player
                        if(Player[A].Controls.Left)
                        {
                            target --;

                            if(target <= 0)
                                target = 5;
                        }
                        else
                        {
                            target ++;

                            if(target > 5)
                                target = 1;
                        }

                        // immediately skip the target if it's blocked
                        if(blockCharacter[target])
                            continue;

                        // also skip the target if it's another player's character
                        int B;

                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(B == A)
                                continue;

                            if(target == Player[B].Character)
                                break;
                        }

                        // B <= numPlayers only if the above break was triggered
                        if(B <= numPlayers)
                            continue;

                        // otherwise we are good and can keep the target
                        break;
                    }

                    // if a suitable target found, swap character
                    if(target != Player[A].Character)
                    {
                        snd = SFX_Slide;
                        SwapCharacter(A, target);

                        if(LevelSelect)
                            SetupPlayers();
                    }
                }

                PlaySound(snd);
            }
        }
    }

    bool stopPause = false;

    if(menuDoPress && MenuCursor >= 0 && MenuCursor < (int)s_items.size())
        stopPause = s_items[MenuCursor].callback();

    return stopPause;
}

} // namespace PauseScreen
