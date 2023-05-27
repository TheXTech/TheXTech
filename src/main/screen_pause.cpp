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
#include "../compat.h"
#include "../config.h"
#include "../core/render.h"
#include "../core/events.h"

#include "../game_main.h"
#include "menu_main.h"
#include "screen_pause.h"
#include "screen_textentry.h"
#include "speedrunner.h"
#include "cheat_code.h"

#include "editor.h"

namespace PauseScreen
{

struct MenuItem
{
    std::string name;
    bool (*callback)();
    MenuItem(std::string n, bool(*cb)()) : name(n), callback(cb) {};
};

static bool s_is_legacy = false;
static int s_pause_plr = 0;
static std::vector<MenuItem> s_items;

static bool s_Continue()
{
    PlaySound(SFX_Pause);
    return true;
}

static bool s_RestartLevel()
{
    MenuMode = MENU_MAIN;
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
    numStars = 0;
    Star.clear();
#ifdef THEXTECH_INTERPROC_SUPPORTED
    IntProc::sendStarsNumber(numStars);
#endif
    numSavedEvents = 0;
    BlockSwitch.fill(false);
    PlaySound(SFX_Bullet);
    return true;
}

static bool s_DropAddScreen()
{
    if(PauseGame(PauseCode::DropAdd, 0) == 1)
        return true;

    MenuCursorCanMove = false;
    return false;
}

static bool s_CheatScreen()
{
    cheats_setBuffer(TextEntryScreen::Run("Enter cheat:"));
    // uncomment this if you want to return to the pause menu
    // MenuCursorCanMove = false;
    // return false;
    return true;
}

static bool s_QuitTesting()
{
    MenuMode = MENU_MAIN;
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
    bool CanSave = (LevelSelect || (IsEpisodeIntro && NoMap)) && !Cheater;

    if(CanSave)
    {
        SaveGame();
        PlaySound(SFX_Checkpoint);
    }
    else
    {
        // player tried to cheat, scare them
        PlaySound(SFX_BowserKilled);
    }

    return true;
}

static bool s_Quit()
{
    bool CanSave = (LevelSelect || (IsEpisodeIntro && NoMap)) && !Cheater;

    if(CanSave)
        SaveGame(); // "Save & Quit"
    else
        speedRun_saveStats();

    GameMenu = true;

    MenuMode = MENU_MAIN;
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
    PlaySound(SFX_Pause);
    MenuCursor = 0;
    MenuCursorCanMove = false;

    s_is_legacy = LegacyPause;
    s_pause_plr = plr;

    // do a context-aware initialization of s_items
    s_items.clear();

    bool CanSave = (LevelSelect || (IsEpisodeIntro && NoMap)) && !Cheater;

    // pause
    if(TestLevel && LevelBeatCode == -2)
    {
        s_items.push_back(MenuItem{"RESTART", s_RestartLevel});
        s_items.push_back(MenuItem{"RESET CHECKPOINTS", s_ResetCheckpoints});

        if(Backup_FullFileName.empty())
            s_items.push_back(MenuItem{"QUIT TESTING", s_QuitTesting});
        else
            s_items.push_back(MenuItem{"RETURN TO EDITOR", s_QuitTesting});
    }
    else if(TestLevel)
    {
        s_items.push_back(MenuItem{"CONTINUE", s_Continue});
        s_items.push_back(MenuItem{"RESTART LEVEL", s_RestartLevel});
        s_items.push_back(MenuItem{"RESET CHECKPOINTS", s_ResetCheckpoints});

        if(g_compatibility.allow_drop_add && !LegacyPause)
            s_items.push_back(MenuItem{"DROP/ADD PLAYERS", s_DropAddScreen});

        if(g_config.enter_cheats_menu_item && !LegacyPause)
            s_items.push_back(MenuItem{"ENTER CHEAT", s_CheatScreen});

        if(Backup_FullFileName.empty())
            s_items.push_back(MenuItem{"QUIT TESTING", s_QuitTesting});
        else
            s_items.push_back(MenuItem{"RETURN TO EDITOR", s_QuitTesting});
    }
    else
    {
        s_items.push_back(MenuItem{"CONTINUE", s_Continue});

        if(g_compatibility.allow_drop_add && !LegacyPause)
            s_items.push_back(MenuItem{"DROP/ADD PLAYERS", s_DropAddScreen});

        if(g_config.enter_cheats_menu_item && !LegacyPause)
            s_items.push_back(MenuItem{"ENTER CHEAT", s_CheatScreen});

        if(CanSave)
        {
            s_items.push_back(MenuItem{"SAVE & CONTINUE", s_SaveAndContinue});
            s_items.push_back(MenuItem{"SAVE & QUIT", s_Quit});
        }
        else
        {
            s_items.push_back(MenuItem{"QUIT", s_Quit});
        }
    }
}

void Render()
{
    int total_menu_height = s_items.size() * 36 - 18;
    int menu_box_height = 200;

    if(menu_box_height - total_menu_height < 18)
        menu_box_height = total_menu_height + 18;

    bool has_long_item = false;

    for(const MenuItem& i : s_items)
    {
        if(i.name.size() > 10)
            has_long_item = true;
    }

    int menu_left_X = ScreenW / 2 - 190 + 62;

    if(!has_long_item)
        menu_left_X += 56;

    int menu_top_Y = ScreenH / 2 - total_menu_height / 2;

    XRender::renderRect(ScreenW / 2 - 190, ScreenH / 2 - menu_box_height / 2, 380, menu_box_height, 0, 0, 0);

    for(size_t i = 0; i < s_items.size(); i++)
    {
        SuperPrint(s_items[i].name, 3, menu_left_X, menu_top_Y + (i * 36));
    }

    if(GFX.PCursor.inited)
    {
        if(s_pause_plr == 2 && !s_is_legacy)
            XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor, 0.0, 1.0, 0.0);
        else if(s_pause_plr != 1 && !s_is_legacy)
            XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor);
        else
            XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), GFX.PCursor, 1.0, 0.0, 0.0);
    }
    else if(s_pause_plr == 2 && !s_is_legacy)
        XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), 16, 16, GFX.MCursor[3], 0, 0);
    else
        XRender::renderTexture(menu_left_X - 20, menu_top_Y + (MenuCursor * 36), 16, 16, GFX.MCursor[0], 0, 0);
}

bool Logic(int plr)
{
    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;

    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack;

    if(SingleCoop > 0 || numPlayers > 2)
    {
        for(int A = 1; A <= numPlayers; A++)
            Player[A].Controls = Player[1].Controls;
    }

    if(!g_compatibility.multiplayer_pause_controls && plr == 0)
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
        }
    }
    else
    {
        const Controls_t& c = Player[plr].Controls;

        menuDoPress |= (c.Start || c.Jump);
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;

    }

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

    if(!MenuCursorCanMove)
    {
        if(!menuDoPress && !menuBackPress && !upPressed && !downPressed)
            MenuCursorCanMove = true;

        return false;
    }

    int max_item = s_items.size() - 1;

    if(menuBackPress)
    {
        if(MenuCursor != max_item)
            PlaySound(SFX_Slide);

        MenuCursor = max_item;
        MenuCursorCanMove = false;
    }

    if(upPressed)
    {
        PlaySound(SFX_Slide);
        MenuCursor = MenuCursor - 1;
        MenuCursorCanMove = false;
    }
    else if(downPressed)
    {
        PlaySound(SFX_Slide);
        MenuCursor = MenuCursor + 1;
        MenuCursorCanMove = false;
    }

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

                if(AllCharBlock == 0 && numPlayers <= maxLocalPlayers)
                {
                    PlaySound(SFX_Slide);
                    Player[A].RunRelease = false;

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
                        SwapCharacter(A, target);

                        if(LevelSelect)
                            SetupPlayers();
                    }
                }
            }
        }
    }

    bool stopPause = false;

    if(menuDoPress && MenuCursor >= 0 && MenuCursor < (int)s_items.size())
        stopPause = s_items[MenuCursor].callback();

    return stopPause;
}

} // namespace PauseScreen
