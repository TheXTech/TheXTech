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

#include <vector>
#include <string>

#ifdef THEXTECH_INTERPROC_SUPPORTED
#include <InterProcess/intproc.h>
#endif // #ifdef THEXTECH_INTERPROC_SUPPORTED

#ifdef THEXTECH_ENABLE_SDL_NET
#include "main/client_methods.h"
#endif

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
#include "main/screen_pause.h"
#include "main/screen_textentry.h"
#include "main/screen_connect.h"
#include "speedrunner.h"
#include "cheat_code.h"
#include "message.h"

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
    bool is_private;
    MenuItem(const std::string &n, bool(*cb)(), bool p = false) : name(n), callback(cb), is_private(p) {}
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
static uint32_t s_cheat_menu_frame = 0;
static std::array<bool, maxLocalPlayers> s_leftright_release;

uint8_t g_pending_action = 255;
static bool s_force_exit = false;

static void s_push_unpause()
{
    XMessage::Message unpause;
    unpause.type = XMessage::Type::menu_action;
    unpause.message = 0;
    XMessage::PushMessage(unpause);
}

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

    PauseInit(PauseCode::DropAdd, 0);

    return false;
}


static void s_OptionsScreen_Resume();

static bool s_OptionsScreen()
{
    PlaySound(SFX_Do);

    PauseInit(PauseCode::Options, 0, s_OptionsScreen_Resume);

    return false;
}

static void s_OptionsScreen_Resume()
{
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
}

static void s_CheatScreen_callback()
{
    cheats_setBuffer(TextEntryScreen::Text, false);

    // comment this if you want to return to the pause menu
    s_push_unpause();
}

static bool s_CheatScreen()
{
    TextEntryScreen::Init(g_gameStrings.pauseItemEnterCode, s_CheatScreen_callback);

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
#ifdef THEXTECH_ENABLE_SDL_NET
    XMessage::Disconnect();
    s_force_exit = true;
#endif

    bool CanSave = (LevelSelect || IsHubLevel) && !Cheater;

    if(CanSave)
        SaveGame(); // "Save & Quit"
    else
        speedRun_saveStats();

    ConnectScreen::SaveChars();

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

void UnlockCheats()
{
    s_cheat_menu_bits = 15;
    s_cheat_menu_frame = CommonFrame;

    if(GamePaused == PauseCode::PauseScreen)
        TextEntryScreen::Init(g_gameStrings.pauseItemEnterCode, s_CheatScreen_callback);
}

void Init(int plr, bool LegacyPause)
{
    XHints::Select();

    PlaySound(SFX_Pause);
    MenuCursor = 0;
    MenuCursorCanMove = false;
    MenuCursorCanMove_Back = false;
    g_pending_action = 255;
    s_force_exit = false;

    if(LegacyPause)
        s_pause_type = PauseType::Legacy;
    else if(TestLevel)
        s_pause_type = PauseType::Testing;
    else
        s_pause_type = PauseType::Modern;

    s_pause_plr = plr;
    if(s_cheat_menu_bits < 14)
        s_cheat_menu_bits = 0;

    s_leftright_release.fill(false);

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
            s_items.push_back(MenuItem{g_gameStrings.pauseItemPlayerSetup, s_DropAddScreen, true});

        if(!inter_screen && s_cheat_menu_bits >= 14 && !BattleMode)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemEnterCode, s_CheatScreen, true});

        s_items.push_back(MenuItem{g_mainMenu.mainOptions, s_OptionsScreen, true});

        s_items.push_back(MenuItem{editor_test ? g_gameStrings.pauseItemReturnToEditor : g_gameStrings.pauseItemQuitTesting, s_QuitTesting});
    }
#ifdef THEXTECH_ENABLE_SDL_NET
    // NetPlay pause
    else if(XMessage::GetStatus() != XMessage::Status::local)
    {
        s_items.push_back(MenuItem{g_gameStrings.pauseItemContinue, s_Continue});

        if(g_config.allow_drop_add && s_pause_type != PauseType::Legacy)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemPlayerSetup, s_DropAddScreen, true});

        if(s_cheat_menu_bits >= 14 && s_pause_type != PauseType::Legacy && !BattleMode)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemEnterCode, s_CheatScreen, true});

        if(s_pause_type != PauseType::Legacy)
            s_items.push_back(MenuItem{g_mainMenu.mainOptions, s_OptionsScreen, true});

        s_items.push_back(MenuItem{g_mainMenu.netplayLeaveRoom, s_Quit, true});
    }
#endif
    // main game pause
    else
    {
        s_items.push_back(MenuItem{g_gameStrings.pauseItemContinue, s_Continue});

        if(g_config.allow_drop_add && s_pause_type != PauseType::Legacy)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemPlayerSetup, s_DropAddScreen, true});

        if(s_cheat_menu_bits >= 14 && s_pause_type != PauseType::Legacy && !BattleMode)
            s_items.push_back(MenuItem{g_gameStrings.pauseItemEnterCode, s_CheatScreen, true});

        if(s_pause_type != PauseType::Legacy)
            s_items.push_back(MenuItem{g_mainMenu.mainOptions, s_OptionsScreen, true});

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

    // force cheat entry if needed
    if(s_cheat_menu_bits == 15)
    {
        s_cheat_menu_bits = 14;
        if(CommonFrame - s_cheat_menu_frame < 60)
            TextEntryScreen::Init(g_gameStrings.pauseItemEnterCode, s_CheatScreen_callback);
    }
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

    // display room info above the pause menu
#ifdef THEXTECH_ENABLE_SDL_NET
    if(XMessage::GetStatus() == XMessage::Status::connected)
    {
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2 - 4, XRender::TargetH / 2 - menu_box_height / 2 - 40 - 4, menu_box_width + 8, 28 + 8, {0, 0, 0});
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2 - 2, XRender::TargetH / 2 - menu_box_height / 2 - 40 - 2, menu_box_width + 4, 28 + 4, {255, 255, 255});
        XRender::renderRect(XRender::TargetW / 2 - menu_box_width / 2, XRender::TargetH / 2 - menu_box_height / 2 - 40, menu_box_width, 28, {8, 96, 168});

        SuperPrintScreenCenter(g_mainMenu.netplayRoomKey + ' ' + XMessage::DisplayRoom(XMessage::CurrentRoom()).room_name, 5, XRender::TargetH / 2 - menu_box_height / 2 - 36);
    }
#endif // #ifdef THEXTECH_ENABLE_SDL_NET

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
                XRender::renderTextureBasic(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor, {0, 255, 0});
            else if(s_pause_plr != 1 && s_pause_type != PauseType::Legacy)
                XRender::renderTextureBasic(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor);
            else
                XRender::renderTextureBasic(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor, {255, 0, 0});
        }
        else if(s_pause_plr == 2 && s_pause_type != PauseType::Legacy)
            XRender::renderTextureBasic(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), 16, 16, GFX.MCursor[3], 0, 0);
        else
            XRender::renderTextureBasic(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), 16, 16, GFX.MCursor[0], 0, 0);
    }

    if(XRender::TargetH > XRender::TargetH / 2 + menu_box_height / 2 + 16 + 96 + 8)
        XHints::Draw(XRender::TargetH / 2 + menu_box_height / 2 + 16, 100, menu_box_width);
}

void ControlsLogic()
{
    bool upPressed = l_SharedControls.MenuUp;
    bool downPressed = l_SharedControls.MenuDown;
    bool leftPressed = l_SharedControls.MenuLeft;
    bool rightPressed = l_SharedControls.MenuRight;

    bool menuDoPress = l_SharedControls.MenuDo || l_SharedControls.Pause;
    bool menuBackPress = l_SharedControls.MenuBack;

    // there was previously code to copy all players' controls from the main player, but this is no longer necessary (and actively harmful in the SingleCoop case)

    int plr = s_pause_plr;
    if(plr > numPlayers)
        plr = 1;

    if(!g_config.multiplayer_pause_controls && plr == 0)
        plr = 1;

    for(int i = 0; i < l_screen->player_count; i++)
    {
        if(plr != 0 && l_screen->players[i] != plr)
            continue;

        const Controls_t& c = Controls::g_RawControls[i];

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

        return;
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

        // fixes TheXTech 1.3.7-beta bug where hitting escape (bound to both Shared Back and P1 Do) would immediately exit
        MenuCursorCanMove = false;
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
                return;
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
                s_CheatScreen();
                return;
            }

            // don't swap char
            if(s_cheat_menu_bits >= 5)
            {
                PlaySound(SFX_Do);
                return;
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
        for(int plr_i = 0; plr_i < l_screen->player_count; plr_i++)
        {
            int A = l_screen->players[plr_i];

            if(!s_leftright_release[plr_i])
            {
                if(!Controls::g_RawControls[plr_i].Left && !Controls::g_RawControls[plr_i].Right)
                    s_leftright_release[plr_i] = true;
            }
            else if(Controls::g_RawControls[plr_i].Left || Controls::g_RawControls[plr_i].Right)
            {
                s_leftright_release[plr_i] = false;
                auto snd = SFX_BlockHit;

                if(!g_ClonedPlayerMode && Player[A].Effect == PLREFF_NORMAL)
                {
                    // replaced old character swap code with this new code,
                    // supporting arbitrary multiplayer and in-level swap.
                    int target = Player[A].Character;

                    // do the full wrap-around to find an acceptable target
                    for(int i = 0; i < 5; i++)
                    {
                        // move the target in the direction requested by the player
                        if(Controls::g_RawControls[plr_i].Left)
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
                        bool already_used = false;

                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(B == A)
                                continue;

                            if(target == Player[B].Character)
                            {
                                already_used = true;
                                break;
                            }
                        }

                        if(already_used)
                            continue;

                        // otherwise we are good and can keep the target
                        break;
                    }

                    // if a suitable target found, swap character
                    if(target != Player[A].Character)
                    {
                        snd = SFX_Slide;
                        XMessage::PushMessage({XMessage::Type::char_swap, (uint8_t)plr_i, (uint8_t)target});
                    }
                }

                PlaySound(snd);
            }
        }
    }

    if(menuDoPress && MenuCursor >= 0 && MenuCursor < (int)s_items.size())
    {
        if(s_items[MenuCursor].is_private)
            s_items[MenuCursor].callback();
        else
        {
            XMessage::Message menu_action;
            menu_action.type = XMessage::Type::menu_action;
            menu_action.message = MenuCursor;
            XMessage::PushMessage(menu_action);
        }
    }
}

bool Logic()
{
    if(s_force_exit)
    {
        s_force_exit = false;
        return true;
    }

    bool stopPause = false;

    if(g_pending_action < s_items.size() && !s_items[g_pending_action].is_private)
        stopPause = s_items[g_pending_action].callback();

    g_pending_action = 255;

    return stopPause;
}

} // namespace PauseScreen
