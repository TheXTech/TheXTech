#include <fmt_format_ne.h>

#include "../core/render.h"

#include "../global_constants.h"
#include "../controls.h"
#include "../sound.h"
#include "../globals.h"
#include "../gfx.h"
#include "../graphics.h"
#include "../player.h"
#include "../compat.h"
#include "../config.h"

#include "screen_connect.h"
#include "menu_main.h"
#include "speedrunner.h"

int g_charSelect[maxLocalPlayers] = {0};

namespace ConnectScreen
{

enum class PlayerState
{
    Disconnected,
    SelectChar,
    SelectProfile,
    ConfirmProfile,
    TestControls,
    ReconnectMain,
    StartGame,
};

enum class Context
{
    MainMenu,
    Reconnect,
    DropAdd,
};

static Context s_context;
static int s_minPlayers = 1;

static bool s_inputReady[maxLocalPlayers] = {false};

static PlayerState s_playerState[maxLocalPlayers] = {PlayerState::Disconnected};
static int s_menuItem[maxLocalPlayers] = {0};
// used for special submenus
static PlayerState s_savedState[maxLocalPlayers];
static Controls::InputMethodProfile* s_savedProfile[maxLocalPlayers];

void MainMenu_Start(int minPlayers)
{
    Controls::ClearInputMethods();
    s_minPlayers = minPlayers;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        g_charSelect[i] = 0;
        s_playerState[i] = PlayerState::Disconnected;
        s_menuItem[i] = 0;
        s_inputReady[i] = false;
    }
    s_context = Context::MainMenu;
    Logic();
}

void Reconnect_Start()
{
    if(BattleMode)
        s_minPlayers = 2;
    else
        s_minPlayers = 1;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        s_playerState[i] = PlayerState::Disconnected;
        s_menuItem[i] = 0;
        s_inputReady[i] = false;
    }
    s_context = Context::Reconnect;
    Logic();
}

void DropAdd_Start()
{
    if(BattleMode)
        s_minPlayers = 2;
    else
        s_minPlayers = 1;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        s_playerState[i] = PlayerState::Disconnected;
        s_menuItem[i] = 0;
        s_inputReady[i] = false;
        g_charSelect[i] = Player[i+1].Character;
    }
    s_context = Context::DropAdd;
    Logic();
}

// find the first player that isn't done (receives MenuControls)
int GetMenuPlayer()
{
    int n = Controls::g_InputMethods.size();
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer;
    for(menuPlayer = 0; menuPlayer < n; menuPlayer++)
    {
        if(menuPlayer == (int)Controls::g_InputMethods.size())
            break;
        if(s_playerState[menuPlayer] != PlayerState::StartGame)
            break;
    }
    return menuPlayer;
}
{
    if(c < 1 || c > 5)
        return false;
    if(blockCharacter[c])
        return false;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(i == (int)Controls::g_InputMethods.size())
            break;
        if(g_charSelect[i] == c)
            return false;
    }
    return true;
}


bool CheckDone()
{
    // What is the first player that is not done?
    int menuPlayer = GetMenuPlayer();

    if(menuPlayer >= s_minPlayers && (int)Controls::g_InputMethods.size() >= s_minPlayers
        && (menuPlayer == (int)Controls::g_InputMethods.size() || menuPlayer == maxLocalPlayers))
    {
        // everyone is done!
        return true;
    }

    return false;
}

void Player_ValidateChar(int p)
{
    // ensure that each character's selection is still valid
    for(int i = 0; i < 5; i++)
    {
        if(s_menuItem[p] < 0 || CharAvailable(s_menuItem[p]+1))
            break;
        s_menuItem[p] ++;
        if(s_menuItem[p] == 5)
            s_menuItem[p] = 0;
    }
}

bool Player_Remove(int p)
{
    Controls::DeleteInputMethodSlot(p);
    for(int p2 = p; p2+1 < maxLocalPlayers; p2++)
    {
        g_charSelect[p2] = g_charSelect[p2+1];
        s_playerState[p2] = s_playerState[p2+1];
        s_menuItem[p2] = s_menuItem[p2+1];
        s_inputReady[p2] = s_inputReady[p2+1];
    }
    // in drop-add, remove the Player officially
    if(p+1 <= numPlayers && s_context == Context::DropAdd)
    {
        DropPlayer(p+1);
    }

    if(Controls::g_InputMethods.empty())
        return true;
    return false;
}

void DropNotDone(bool strict)
{
    int n = Controls::g_InputMethods.size()-1;
    if(n >= maxLocalPlayers)
        n = maxLocalPlayers-1;
    // remove all not fully added players
    for(; n >= numPlayers; n--)
        Player_Remove(n);
    // if strict, remove all unfinished players
    for(; n >= 0 && numPlayers > s_minPlayers; n--)
    {
        if(strict && s_playerState[n] != PlayerState::StartGame)
            Player_Remove(n);
    }
}

void Player_Swap(int p1, int p2)
{
    if(p1 == p2)
        return;
    if(p1 >= (int)Controls::g_InputMethods.size() || p2 >= (int)Controls::g_InputMethods.size())
        return;
    std::swap(Controls::g_InputMethods[p1], Controls::g_InputMethods[p2]);
    std::swap(g_charSelect[p1], g_charSelect[p2]);
    std::swap(s_playerState[p1], s_playerState[p2]);
    std::swap(s_menuItem[p1], s_menuItem[p2]);
    s_inputReady[p1] = false;
    s_inputReady[p2] = false;
}

struct PlayDoSentinel
{
    bool active = true;
    ~PlayDoSentinel()
    {
        if(active)
            PlaySoundMenu(SFX_Do);
    }
};

// returns true to signal exit from menu
bool Player_Back(int p)
{
    s_inputReady[p] = false;
    if(s_playerState[p] == PlayerState::SelectChar && s_context == Context::MainMenu)
    {
        DeleteInputMethod(Controls::g_InputMethods[p]);
        s_playerState[p] = PlayerState::Disconnected;
        g_charSelect[p] = 0;
        return false;
    }
    else if(s_playerState[p] == PlayerState::SelectChar && p >= numPlayers)
    {
        Player_Remove(p);
        return false;
    }
    else if(s_playerState[p] == PlayerState::SelectChar)
    {
        s_menuItem[p] = Player[p+1].Character-1;
        Player_ValidateChar(p);
        g_charSelect[p] = s_menuItem[p]+1;
        if(g_charSelect[p] != Player[p+1].Character)
            SwapCharacter(p+1, g_charSelect[p], g_config.StrictDropAdd);
        s_playerState[p] = PlayerState::ReconnectMain;
        s_menuItem[p] = 1;
    }

    if(s_playerState[p] == PlayerState::ReconnectMain)
    {
        if(s_context == Context::DropAdd)
            return true;
        else
            return false;
    }

    PlaySoundMenu(SFX_Slide);
    if(s_playerState[p] == PlayerState::StartGame && s_context == Context::MainMenu)
    {
        s_playerState[p] = PlayerState::SelectChar;
        s_menuItem[p] = g_charSelect[p] - 1;
        g_charSelect[p] = 0;
    }
    else if(s_playerState[p] == PlayerState::StartGame)
    {
        s_playerState[p] = PlayerState::ReconnectMain;
        s_menuItem[p] = 0;
    }
    else if(s_playerState[p] == PlayerState::SelectProfile)
    {
        s_playerState[p] = s_savedState[p];
        s_savedState[p] = PlayerState::Disconnected;
        s_menuItem[p] = -3;
    }

    return  false;
}

bool Player_Select(int p)
{
    s_inputReady[p] = false;

    // plays SFX_Do at end of scope unless inactivated
    PlayDoSentinel do_sentinel;

    // global cases
    if(s_menuItem[p] == -1)
    {
        DeleteInputMethod(Controls::g_InputMethods[p]);
        s_playerState[p] = PlayerState::Disconnected;
        if(s_context == Context::MainMenu)
            g_charSelect[p] = 0;
        do_sentinel.active = false;
        return false;
    }
    else if(s_menuItem[p] == -2)
    {
        s_savedState[p] = s_playerState[p];
        s_playerState[p] = PlayerState::TestControls;
        return false;
    }
    else if(s_menuItem[p] == -3)
    {
        // only enter the profile select screen if safe to do so.
        if(Controls::g_InputMethods[p])
        {
            s_savedState[p] = s_playerState[p];
            s_playerState[p] = PlayerState::SelectProfile;

            // set the current menu item to the current profile.
            std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            std::vector<Controls::InputMethodProfile*>::iterator p_profile
                = std::find(profiles.begin(), profiles.end(), Controls::g_InputMethods[p]->Profile);
            if(profiles.size() > 1 && p_profile != profiles.end())
                s_menuItem[p] = p_profile - profiles.begin();
            else
                s_menuItem[p] = 0;
        }
        return false;
    }

    if(s_playerState[p] == PlayerState::SelectChar)
    {
        g_charSelect[p] = s_menuItem[p]+1;
        if(s_context == Context::MainMenu)
        {
            s_playerState[p] = PlayerState::StartGame;
            s_menuItem[p] = -4;
            if(CheckDone())
                return true;
        }
        else
        {
            s_playerState[p] = PlayerState::ReconnectMain;
            s_menuItem[p] = 1;
            if(p+1 <= numPlayers)
            {
                if(g_charSelect[p] != Player[p+1].Character)
                    SwapCharacter(p+1, g_charSelect[p], g_config.StrictDropAdd);
            }
            else
            {
                // swap p with the first non-existent player slot
                s_menuItem[p] = 0;
                Player_Swap(numPlayers, p);
                // AddPlayer increments numPlayers by 1
                AddPlayer(g_charSelect[numPlayers]);
                // lose a life if StrictDropAdd enabled
                if(g_config.StrictDropAdd && !LevelSelect)
                {
                    if(Lives <= 0)
                        Player[numPlayers].Dead = true;
                    else
                        Lives -= 1;
                }
            }
        }
    }
    else if(s_playerState[p] == PlayerState::ReconnectMain && s_context == Context::Reconnect)
    {
        if(s_menuItem[p] == 0)
        {
            s_playerState[p] = PlayerState::StartGame;
            if(CheckDone())
                return true;
            s_menuItem[p] = -4;
        }
    }
    else if(s_playerState[p] == PlayerState::ReconnectMain && s_context == Context::DropAdd)
    {
        if(s_menuItem[p] == 0)
        {
            s_playerState[p] = PlayerState::StartGame;
            if(CheckDone())
                return true;
            s_menuItem[p] = -4;
        }
        else if(s_menuItem[p] == 1)
        {
            s_playerState[p] = PlayerState::SelectChar;
            s_menuItem[p] = g_charSelect[p] - 1;
            g_charSelect[p] = 0;
        }
        else if(s_menuItem[p] == 2 && numPlayers > s_minPlayers)
        {
            Player_Remove(p);
        }
        else
        {
            do_sentinel.active = false;
        }
    }
    else if(s_playerState[p] == PlayerState::StartGame)
    {
        if(CheckDone())
            return true;
        if(s_menuItem[p] == -4 && s_context != Context::MainMenu)
        {
            do_sentinel.active = false;
            s_menuItem[p] = 0;
        }
        else if(s_context == Context::DropAdd || (s_context == Context::Reconnect && g_compatibility.allow_drop_add))
        {
            if(s_menuItem[p] == 0) // FORCE RESUME
            {
                DropNotDone(false);
                return true;
            }
            else if(s_menuItem[p] == 1 && numPlayers > s_minPlayers) // DROP OTHERS
            {
                Context c = s_context;
                s_context = Context::DropAdd;
                DropNotDone(true);
                s_context = c;
                return true;
            }
            else if(s_menuItem[p] == 2) // BACK
            {
                do_sentinel.active = false;
                Player_Back(p);
            }
        }
        else if(s_context == Context::Reconnect)
        {
            if(s_menuItem[p] == 0) // FORCE RESUME
                return true;
            else if(s_menuItem[p] == 1) // BACK
            {
                do_sentinel.active = false;
                Player_Back(p);
            }
        }
    }
    else if(s_playerState[p] == PlayerState::SelectProfile)
    {
        if(Controls::g_InputMethods[p])
        {
            std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            s_savedProfile[p] = Controls::g_InputMethods[p]->Profile;

            int back_index = profiles.size();
            if(s_menuItem[p] < 0 || s_menuItem[p] >= back_index)
            {
                Player_Back(p);
                do_sentinel.active = false;
            }
            else if(Controls::SetInputMethodProfile(p, profiles[s_menuItem[p]]))
            {
                s_playerState[p] = PlayerState::ConfirmProfile;
                s_menuItem[p] = 66*3;
            }
            else
            {
                PlaySoundMenu(SFX_BlockHit);
                do_sentinel.active = false;
            }

        }
    }

    return false;
}

void Player_Up(int p)
{
    s_inputReady[p] = false;
    if(s_playerState[p] == PlayerState::SelectChar)
    {
        PlaySoundMenu(SFX_Slide);
        for(int i = 0; i < 6; i++)
        {
            if(s_menuItem[p] == -3)
                s_menuItem[p] = 5;
            s_menuItem[p] --;
            if(s_menuItem[p] < 0 || CharAvailable(s_menuItem[p]+1))
                break;
        }
    }
    if(s_playerState[p] == PlayerState::SelectProfile)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 0)
        {
            if(Controls::g_InputMethods[p])
            {
                std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
                int back_index = profiles.size();
                s_menuItem[p] = back_index;
            }
        }
        else
            s_menuItem[p] --;
    }
    if(s_playerState[p] == PlayerState::ReconnectMain)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == -3)
        {
            if(s_context == Context::Reconnect)
                s_menuItem[p] = 0;
            else if(numPlayers > s_minPlayers)
                s_menuItem[p] = 2;
            else
                s_menuItem[p] = 1;
        }
        else
            s_menuItem[p] --;
    }
    if(s_playerState[p] == PlayerState::StartGame && s_context != Context::MainMenu)
    {
        if(s_menuItem[p] == -4)
        {
            s_menuItem[p] = 0;
            return;
        }
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 0)
            s_menuItem[p] = (s_context == Context::Reconnect ? 1 : 2);
        else
            s_menuItem[p] --;
    }
}

void Player_Down(int p)
{
    s_inputReady[p] = false;
    if(s_playerState[p] == PlayerState::SelectChar)
    {
        PlaySoundMenu(SFX_Slide);
        for(int i = 0; i < 6; i++)
        {
            s_menuItem[p] ++;
            if(s_menuItem[p] == 5)
                s_menuItem[p] = -3;
            if(s_menuItem[p] < 0 || CharAvailable(s_menuItem[p]+1))
                break;
        }
    }
    if(s_playerState[p] == PlayerState::SelectProfile)
    {
        PlaySoundMenu(SFX_Slide);
        if(Controls::g_InputMethods[p])
        {
            std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            int back_index = profiles.size();
            if(s_menuItem[p] == back_index)
                s_menuItem[p] = -1;
        }
        s_menuItem[p] ++;
    }
    if(s_playerState[p] == PlayerState::ReconnectMain)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 0 && s_context == Context::Reconnect)
            s_menuItem[p] = -3;
        else if(s_menuItem[p] == 1 && s_context == Context::DropAdd && numPlayers <= s_minPlayers)
            s_menuItem[p] = -3;
        else if(s_menuItem[p] == 2 && s_context == Context::DropAdd)
            s_menuItem[p] = -3;
        else
            s_menuItem[p] ++;
    }
    if(s_playerState[p] == PlayerState::StartGame && s_context != Context::MainMenu)
    {
        if(s_menuItem[p] == -4)
        {
            s_menuItem[p] = 0;
            return;
        }
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 1 && s_context == Context::Reconnect)
            s_menuItem[p] = 0;
        else if(s_menuItem[p] == 2 && s_context == Context::DropAdd)
            s_menuItem[p] = 0;
        else
            s_menuItem[p] ++;
    }
}

bool Player_MouseItem(int p, int i)
{
    if(s_menuItem[p] != i)
    {
        PlaySoundMenu(SFX_Slide);
        s_menuItem[p] = i;
    }

    if(MenuMouseRelease && SharedCursor.Primary)
    {
        MenuMouseRelease = false;
        return Player_Select(p);
    }

    return false;
}

bool Player_MenuItem_Mouse_Render(int p, int i, const std::string& label, int X, int Y, bool mouse, bool render)
{
    if(mouse)
    {
        int menuLen = label.size() * 18;
        if(SharedCursor.X >= X && SharedCursor.X <= X + menuLen
            && SharedCursor.Y >= Y && SharedCursor.Y <= Y + 16)
        {
            return Player_MouseItem(p, i);
        }
    }
    if(render)
    {
        SuperPrint(label, 3, X, Y);
    }
    return false;
}

bool Player_Mouse_Render(int p, int pX, int cX, int pW, int sY, int line, bool mouse, bool render)
{
    bool ret = false;

    // render the player's header

    if(render)
    {
        SuperPrintCenter(fmt::format_ne("{0} {1}", g_mainMenu.wordPlayer, p+1), 3, cX, sY+2*line);
        if(g_charSelect[p] != 0)
            SuperPrintCenter(g_mainMenu.selectPlayer[g_charSelect[p]], 3, cX, sY+3*line);
    }

    // now render / process the player's menu as appropriate to its case

    // pretend that player 1 is connected in the main menu
    bool pretend_connected;
    if(p == 0 && s_context == Context::MainMenu && s_playerState[p] == PlayerState::Disconnected)
        pretend_connected = true;
    else
        pretend_connected = false;
    // if player is disconnected, show the attach controller message and return
    if(!pretend_connected && s_playerState[p] == PlayerState::Disconnected)
    {
        if(render)
        {
            BlockFlash += 1;

            if(BlockFlash >= 90)
                BlockFlash = 0;

            if(BlockFlash < 45)
                SuperPrintCenter(g_mainMenu.phrasePressAButton, 3, cX, sY+7*line);
    }
        return ret;
    }

    // render the character select screen
    if(pretend_connected || s_playerState[p] == PlayerState::SelectChar)
    {
        Player_ValidateChar(p);
        // show the menu cursor for the player
        if(s_menuItem[p] >= 0)
            XRender::renderTexture(pX - 20, sY+(4+s_menuItem[p])*line, GFX.MCursor[0]);
        for(int c = 0; c < 5; c++)
        {
            // don't render/mouse unavailable chars
            if(!CharAvailable(c+1))
                continue;
            Player_MenuItem_Mouse_Render(p, c, g_mainMenu.selectPlayer[c+1],
                pX, sY+(4+c)*line, mouse, render);
        }
    }

    // render the profile select screen
    if(s_playerState[p] == PlayerState::SelectProfile)
    {
        if(!Controls::g_InputMethods[p])
        {
            Player_Back(p);
            return ret;
        }
        // set the current menu item to the current profile.
        std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
        if(profiles.empty())
        {
            Player_Back(p);
            return ret;
        }

        int start_y = sY + (4)*line;

        // calculate scroll (top rendered index) here
        int total_lines = (int)profiles.size() + 1;
        int avail_lines = 6;

        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
            // for scroll indicator
            avail_lines --;

            if(s_menuItem[p] >= 0)
                scroll_start = s_menuItem[p] - avail_lines/2;
            if(scroll_start < 0)
                scroll_start = 0;
            scroll_end = scroll_start + avail_lines;
            if(scroll_end > total_lines)
            {
                scroll_end = total_lines;
                scroll_start = scroll_end - avail_lines;
            }
        }

        // render the scroll indicators
        if(render)
        {
            if(scroll_start > 0)
                XRender::renderTexture(cX - GFX.MCursor[1].w / 2, start_y - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < total_lines)
                XRender::renderTexture(cX - GFX.MCursor[2].w / 2, start_y + (avail_lines)*line - line + 18, GFX.MCursor[2]);
        }

        // show the menu cursor for the player
        if(s_menuItem[p] >= 0)
            XRender::renderTexture(pX - 20, start_y + (s_menuItem[p]-scroll_start)*line, GFX.MCursor[0]);

        for(int i = scroll_start; i < scroll_end; i++)
        {
            std::string* name = &g_mainMenu.wordBack;
            if(i != (int)profiles.size())
                name = &(profiles[i]->Name);
            Player_MenuItem_Mouse_Render(p, i, *name,
                pX, start_y + (i-scroll_start)*line, mouse, render);
        }
    }

    // render the profile confirmation and test controls screens
    if(s_playerState[p] == PlayerState::ConfirmProfile || s_playerState[p] == PlayerState::TestControls)
    {
        if(render)
        {
            RenderControls(p+1, cX-38, sY+4*line, 76, 30);
            SuperPrintCenter(g_mainMenu.phraseHoldStartToReturn, 3, cX, sY+7*line);
            int n_stars;
            int n_empty;
            if(s_playerState[p] == PlayerState::ConfirmProfile && s_menuItem[p] < 66*3)
            {
                n_stars = (s_menuItem[p] + 65) / 66;
                n_empty = 6 - n_stars;
            }
            else if(s_playerState[p] == PlayerState::ConfirmProfile)
            {
                n_stars = 3 + (s_menuItem[p] - 66*3)/22;
                n_empty = 6 - n_stars;
            }
            else
            {
                n_stars = s_menuItem[p]/11;
                n_empty = 6 - n_stars;
            }
            const std::string squeeze = std::string(n_empty, '-') + std::string(n_stars*2, '*') + std::string(n_empty, '-');
            SuperPrintCenter(squeeze, 3, cX, sY+8*line);
        }
    }

    // render the main Reconnect/DropAdd screen
    if(s_playerState[p] == PlayerState::ReconnectMain)
    {
        if(s_menuItem[p] >= 0)
            XRender::renderTexture(pX - 20, sY+(4+s_menuItem[p])*line, GFX.MCursor[0]);

        Player_MenuItem_Mouse_Render(p, 0, "CONTINUE",
            pX, sY+(4+0)*line, mouse, render);
        if(s_context == Context::DropAdd)
        {
            Player_MenuItem_Mouse_Render(p, 1, "CHANGE CHAR",
                pX, sY+(4+1)*line, mouse, render);
            if(numPlayers > s_minPlayers)
            {
                Player_MenuItem_Mouse_Render(p, 2, "DROP ME",
                    pX, sY+(4+2)*line, mouse, render);
            }
        }
    }

    // render the (waiting for other players / game start screen)
    if(s_playerState[p] == PlayerState::StartGame)
    {
        if(CheckDone() && render)
        {
            SuperPrint(g_mainMenu.playerSelStartGame, 3, pX, sY+7*line);
            XRender::renderTexture(pX - 20, sY+7*line, GFX.MCursor[0]);
        }
        else
        {
            // make the "Waiting" text flash
            if(render)
            {
                BlockFlash += 1;

                if(BlockFlash >= 90)
                    BlockFlash = 0;

                if(BlockFlash < 45)
                    SuperPrintCenter(g_mainMenu.wordWaiting, 3, cX, sY+7*line);
            }

            if((s_context == Context::DropAdd || (s_context == Context::Reconnect && g_compatibility.allow_drop_add))
                && s_menuItem[p] != -4)
            {
                XRender::renderTexture(pX - 20, sY+(9+s_menuItem[p])*line, GFX.MCursor[0]);
                Player_MenuItem_Mouse_Render(p, 0, "FORCE RESUME",
                    pX, sY+(9+0)*line, mouse, render);
                Player_MenuItem_Mouse_Render(p, 1, "DROP OTHERS",
                    pX, sY+(9+1)*line, mouse, render);
                Player_MenuItem_Mouse_Render(p, 2, "BACK",
                    pX, sY+(9+2)*line, mouse, render);
            }
            else if(s_context == Context::Reconnect && s_menuItem[p] != -4)
            {
                XRender::renderTexture(pX - 20, sY+(9+s_menuItem[p])*line, GFX.MCursor[0]);
                Player_MenuItem_Mouse_Render(p, 0, "FORCE RESUME",
                    pX, sY+(9+0)*line, mouse, render);
                Player_MenuItem_Mouse_Render(p, 1, "BACK",
                    pX, sY+(9+1)*line, mouse, render);
            }
        }
    }

    // only happens when first player is connecting
    if(s_playerState[p] == PlayerState::Disconnected
        || p >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[p])
    {
        if(render)
            SuperPrintCenter(g_mainMenu.phrasePressAButton, 3, cX, sY+11*line);
        return ret;
    }

    // don't process any of the controls stuff if already in a controls submenu
    if(s_playerState[p] == PlayerState::SelectProfile || s_playerState[p] == PlayerState::ConfirmProfile
            || s_playerState[p] == PlayerState::TestControls || s_playerState[p] == PlayerState::StartGame)
        return ret;

    if(render)
    {
        // global information about controller
        SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, sY+10*line);

        // show the menu cursor for the player
        if(s_menuItem[p] < 0)
            XRender::renderTexture(pX - 20, sY+(14+s_menuItem[p])*line, GFX.MCursor[0]);

        // should never be null
        if(Controls::g_InputMethods[p]->Profile != nullptr)
            SuperPrint(Controls::g_InputMethods[p]->Profile->Name, 3, pX, sY+11*line);
        else
            SuperPrint(g_mainMenu.wordProfile, 3, pX, sY+11*line);
        SuperPrint(g_mainMenu.phraseTestControls, 3, pX, sY+12*line);
        SuperPrint(g_mainMenu.wordDisconnect, 3, pX, sY+13*line);
    }
    if(mouse)
    {
        if(SharedCursor.X >= pX && SharedCursor.X <= pX+pW)
        {
            if(SharedCursor.Y >= sY+11*line && SharedCursor.Y <= sY+11*line + 16)
                ret |= Player_MouseItem(p, -3);
            if(SharedCursor.Y >= sY+12*line && SharedCursor.Y <= sY+12*line + 16)
                ret |= Player_MouseItem(p, -2);
            if(SharedCursor.Y >= sY+13*line && SharedCursor.Y <= sY+13*line + 16)
                ret |= Player_MouseItem(p, -1);
        }
    }
    return ret;
}

int Mouse_Render(bool mouse, bool render)
{
    if(mouse && !SharedCursor.Move && !render && !SharedCursor.Primary)
        return 0;

    int n = Controls::g_InputMethods.size();
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer = GetMenuPlayer();

    /*--------------------*\
    || Get screen pos     ||
    \*--------------------*/

    // there are 15 rows of text; 30px each by default.
    // line height is scaled so that these fit onscreen
    // with at least 30px of padding on either side
    int line = (ScreenH - 60) / 15;
    line -= line & 1;
    if(line > 30)
        line = 30;

    // vertical start of the menu
    int sY = ScreenH/2 - (line*15)/2;
    sY -= sY & 1;

    // Assume that the controller profile name is up to 15 characters.
    // This means that each player should have up to 270px horizontal space
    // to themselves. Each player absolutely needs 144px to themselves
    // (enough to spell "player 1").

    // Automatically set up the screen to enable this, with minimum width 144px
    // and maximum width 270px per player.
    int p_width = (ScreenW-20)/n;
    const int min_width = 200;
    const int max_width = 320;
    bool scroll_mode = false;

    if(p_width < min_width)
    {
        p_width = min_width;
        scroll_mode = true;
    }
    else if(p_width > max_width)
    {
        p_width = max_width;
    }

    // horizontal start of the menu
    int sX;
    // if scroll mode, must adjust screen start to first unfinished player
    if(scroll_mode)
    {
        sX = -min_width*menuPlayer + 10;
    }
    // otherwise, center players on screen
    else
    {
        sX = ScreenW/2 - (p_width*n)/2;
        sX -= sX & 1;
    }

    if(render)
    {
        XRender::renderRect(sX, sY - (line-16), p_width*n, line*15 + line-16, 0, 0, 0, .5);
    }

    /*--------------------*\
    || Render menu        ||
    \*--------------------*/

    if(render)
    {
        if(s_context == Context::MainMenu)
            SuperPrintScreenCenter(g_mainMenu.charSelTitle, 3, sY);
        else if(s_context == Context::DropAdd)
            SuperPrintScreenCenter(g_mainMenu.dropAddTitle, 3, sY);
        else if(s_context == Context::Reconnect)
            SuperPrintScreenCenter(g_mainMenu.reconnectTitle, 3, sY);
    }

    for(int p = 0; p < n; p++)
    {
        // locate player's screen
        // lX is the position of aligned text
        // cX is the position of centered (possibly long) text
        int lX = sX + p * p_width;
        int cX = lX + p_width / 2;
        if(p_width > min_width)
        {
            lX += (p_width - min_width) / 2;
        }
        lX -= lX & 1;
        cX -= cX & 1;

        Player_Mouse_Render(p, lX, cX, p_width, sY, line, mouse, render);
    }

    if(CheckDone())
    {
        if(render)
            SuperPrintScreenCenter(g_mainMenu.playerSelStartGame, 3, sY+14*line);
        if(mouse && MenuMouseRelease && SharedCursor.Primary)
        {
            PlaySoundMenu(SFX_Do);
            MenuMouseRelease = false;
            return 1;
        }
    }

    return 0;
}

int MouseLogic()
{
    return Mouse_Render(true, false);
}

int Logic()
{
    bool block_poll = false;
    // do not allow more players than previously existed to reconnect
    if(s_context == Context::Reconnect)
    {
        if((int)Controls::g_InputMethods.size() < s_minPlayers)
            block_poll = false;
        else
        {
            block_poll = true;
            for(Controls::InputMethod* method : Controls::g_InputMethods)
            {
                if(!method)
                    block_poll = false;
            }
        }
    }
    for(int p = 0; p < maxLocalPlayers; p++)
    {
        if(s_playerState[p] == PlayerState::TestControls || s_playerState[p] == PlayerState::ConfirmProfile)
            block_poll = true;
    }
    if(!block_poll && Controls::PollInputMethod())
    {
        if((int)Controls::g_InputMethods.size() > 1)
        {
            PlaySoundMenu(SFX_DropItem);
        }
    }

    // What is the first player that is not done?
    int menuPlayer = GetMenuPlayer();

    for(int p = 0; p < maxLocalPlayers; p++)
    {
        // ensure that the selected menu item is valid for select char and drop me
        if(s_playerState[p] == PlayerState::SelectChar)
            Player_ValidateChar(p);

        if(s_context == Context::DropAdd && s_playerState[p] == PlayerState::ReconnectMain
            && s_menuItem[p] == 2 && numPlayers <= s_minPlayers)
        {
            s_menuItem[p] = 1;
        }

        // if player doesn't exist, or was lost *not on menu screen*
        if(p >= (int)Controls::g_InputMethods.size()
            || (!Controls::g_InputMethods[p] && s_context != Context::MainMenu))
        {
            s_playerState[p] = PlayerState::Disconnected;
            if(s_context == Context::MainMenu)
                g_charSelect[p] = 0;
            // because P1's UI is always shown,
            // allow the first controller to immediately act upon connection
            if(p == 0)
                s_inputReady[p] = true;
            else
                s_inputReady[p] = false;
            continue;
        }

        // if player was lost (necessarily on menu screen) remove them
        if(!Controls::g_InputMethods[p])
        {
            // if Player is last player, return to previous menu screen
            if(Player_Remove(p))
            {
                PlaySoundMenu(SFX_Slide);
                return -1;
            }
            else
            {
                PlaySoundMenu(SFX_PlayerDied2);
            }
            // either way, other players' controls invalidated
            break;
        }

        // if the player has a controller and was disconnected, mark them as connected
        if(s_playerState[p] == PlayerState::Disconnected)
        {
            s_inputReady[p] = false;
            s_menuItem[p] = 0;
            if(s_context == Context::MainMenu)
            {
                if(p == 0)
                    s_inputReady[p] = true;
                s_playerState[p] = PlayerState::SelectChar;
            }
            else if(s_context == Context::Reconnect || s_context == Context::DropAdd)
            {
                s_playerState[p] = PlayerState::ReconnectMain;
                // this is an Add situation (wouldn't happen if s_context == Context::Reconnect)
                if(p >= numPlayers)
                {
                    g_charSelect[p] = 0;
                    s_playerState[p] = PlayerState::SelectChar;
                }
            }
            // wait one frame to process them because Controls are not updated yet
            continue;
        }

        const Controls_t& c = Player[p+1].Controls;

        if(!s_inputReady[p])
        {
            if(!SharedControls.MenuDo && !SharedControls.MenuBack && !SharedControls.MenuDown
                && !SharedControls.MenuUp && !SharedControls.MenuLeft && !SharedControls.MenuRight
                && !c.Jump && !c.Start && !c.Run && !c.Down && !c.Up && !c.Left && !c.Right && !c.Drop)
            {
                s_inputReady[p] = true;
            }
            continue;
        }

        if(s_playerState[p] == PlayerState::ConfirmProfile || s_playerState[p] == PlayerState::TestControls)
        {
            if(s_playerState[p] == PlayerState::ConfirmProfile)
            {
                if(!c.Run && !c.Jump && !c.AltRun && !c.AltJump && !c.Start && !c.Drop && !c.Down && !c.Up && !c.Left && !c.Right)
                {
                    s_menuItem[p] --;
                }
                else if(s_menuItem[p] < 66*3)
                    s_menuItem[p] = 66*3;
            }
            if(s_playerState[p] == PlayerState::TestControls || s_menuItem[p] > 66*3)
            {
                if(!c.Start)
                {
                    s_menuItem[p] --;
                }
            }
            if(c.Start)
            {
                s_menuItem[p] ++;
            }
            if(s_menuItem[p] < 0)
            {
                if(s_playerState[p] == PlayerState::TestControls)
                    s_menuItem[p] = 0;
                if(s_playerState[p] == PlayerState::ConfirmProfile)
                {
                    Controls::SetInputMethodProfile(Controls::g_InputMethods[p], s_savedProfile[p]);
                    s_playerState[p] = s_savedState[p];
                    s_savedState[p] = PlayerState::Disconnected;
                    s_savedProfile[p] = nullptr;
                    s_menuItem[p] = -3;
                    s_inputReady[p] = false;
                    PlaySoundMenu(SFX_Slide);
                }
            }
            else if((s_playerState[p] == PlayerState::TestControls && s_menuItem[p] > 66*1)
                || (s_playerState[p] == PlayerState::ConfirmProfile && s_menuItem[p] > 66*4))
            {
                PlaySoundMenu(SFX_Do);
                if(s_playerState[p] == PlayerState::TestControls)
                    s_menuItem[p] = -2;
                else
                    s_menuItem[p] = -3;
                s_playerState[p] = s_savedState[p];
                s_savedState[p] = PlayerState::Disconnected;
                s_savedProfile[p] = nullptr;
                s_inputReady[p] = false;
            }
            continue;
        }

        if(SharedControls.MenuDo || SharedControls.MenuBack || SharedControls.MenuDown
            || SharedControls.MenuUp || SharedControls.MenuLeft || SharedControls.MenuRight)
        {
            s_inputReady[p] = false;
        }

        if(c.Run || (p == menuPlayer-1 && SharedControls.MenuBack)
            || (p == 0 && p == menuPlayer && SharedControls.MenuBack))
        {
            if(Player_Back(p))
                return -1;
        }
        else if(c.Jump || c.Start || (p == menuPlayer && SharedControls.MenuDo))
        {
            if(Player_Select(p))
                return 1;
        }
        else if(c.Down || (p == menuPlayer && SharedControls.MenuDown))
        {
            Player_Down(p);
        }
        else if(c.Up || (p == menuPlayer && SharedControls.MenuUp))
        {
            Player_Up(p);
        }
    }

    return MouseLogic();
}

void Render()
{
    Mouse_Render(false, true);
}

} // namespace ConnectScreen
