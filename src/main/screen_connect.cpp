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
    Reconnecting,
    SelectChar,
    SelectProfile,
    ConfirmProfile,
    TestControls,
    DropAddMain,
    ReconnectMain,
    ControlsMenu,
    StartGame,
};

enum class Context
{
    MainMenu,
    Reconnect,
    DropAdd,
};

struct CharInfo
{
    int max_players;
    bool char_present[5];

    bool accept(int c)
    {
        if(SwapCharAllowed())
            return true;
        bool novel_add = numPlayers >= this->max_players;
        novel_add &= numPlayers < maxLocalPlayers;
        if(!novel_add && !this->char_present[c])
            return false;
        return true;
    }
};

static Context s_context;
static int s_minPlayers = 1;

static bool s_inputReady[maxLocalPlayers] = {false};

static PlayerState s_playerState[maxLocalPlayers] = {PlayerState::Disconnected};
static int s_menuItem[maxLocalPlayers] = {0};
static Controls::InputMethodProfile* s_savedProfile[maxLocalPlayers];

static CharInfo s_char_info;

void Player_ValidateChar(int p);

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

    MenuMouseRelease = false;
    MenuCursorCanMove = false;

    // prepare for first frame
    BlockFlash = 0;
    if(minPlayers == 1)
        Player_ValidateChar(0);
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

    MenuMouseRelease = false;
    MenuCursorCanMove = false;

    // prepare for first frame
    BlockFlash = 0;
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
        if(i < numPlayers)
            s_playerState[i] = PlayerState::DropAddMain;
        else
            s_playerState[i] = PlayerState::Disconnected;
        s_menuItem[i] = 0;
        s_inputReady[i] = false;
        g_charSelect[i] = Player[i+1].Character;
    }
    s_context = Context::DropAdd;

    MenuMouseRelease = false;
    MenuCursorCanMove = false;

    // prepare for first frame
    BlockFlash = 0;
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
        if(s_playerState[menuPlayer] != PlayerState::StartGame && s_playerState[menuPlayer] != PlayerState::DropAddMain && s_playerState[menuPlayer] != PlayerState::ReconnectMain)
            break;
    }
    return menuPlayer;
}

bool CharAvailable(int c, int p = 0)
{
    if(c < 1 || c > 5)
        return false;
    if(blockCharacter[c])
        return false;
    if(!s_char_info.accept(c-1))
        return false;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(i == (int)Controls::g_InputMethods.size())
            break;
        if(i == p)
            continue;
        if(s_playerState[i] == PlayerState::SelectChar && s_menuItem[i]+1 == c)
            return false;
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

inline int DropAddMain_ItemCount()
{
    int i = 1; // change controls
    if(SwapCharAllowed())
        i ++; // change char
    if(numPlayers > s_minPlayers)
        i ++; // drop me
    return i;
}

void Player_ValidateChar(int p)
{
    // ensure that each character's selection is still valid
    // i is a dummy variable so that if invalid, end up where we started
    int i;
    for(i = 0; i < 5; i++)
    {
        if(s_menuItem[p] < 0 || CharAvailable(s_menuItem[p]+1, p))
            break;
        s_menuItem[p] ++;
        if(s_menuItem[p] == 5)
            s_menuItem[p] = 0;
    }
    // if we failed (total block), first try to find an unblock char
    if(i == 5)
    {
        for(i = 0; i < 5; i++)
        {
            if(!blockCharacter[i+1] && !s_char_info.accept(i))
            {
                s_menuItem[p] = i;
                break;
            }
        }
        // just do char 1
        if(i == 5)
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
        if(strict && s_playerState[n] != PlayerState::StartGame
            && s_playerState[n] != PlayerState::DropAddMain
            && s_playerState[n] != PlayerState::ReconnectMain)
        {
            Player_Remove(n);
        }
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
    // adding player at main menu
    if(s_playerState[p] == PlayerState::SelectChar && s_context == Context::MainMenu)
    {
        Controls::DeleteInputMethod(Controls::g_InputMethods[p]);
        s_playerState[p] = PlayerState::Disconnected;
        g_charSelect[p] = 0;
        return false;
    }
    // adding player at drop/add
    else if(s_playerState[p] == PlayerState::SelectChar && p >= numPlayers)
    {
        Player_Remove(p);
        return false;
    }
    // changing char at drop/add
    else if(s_playerState[p] == PlayerState::SelectChar)
    {
        // restore old character
        s_menuItem[p] = Player[p+1].Character-1;
        Player_ValidateChar(p);
        g_charSelect[p] = s_menuItem[p]+1;
        // (unless this has become impossible)
        if(g_charSelect[p] != Player[p+1].Character)
        {
            SwapCharacter(p+1, g_charSelect[p], g_config.StrictDropAdd);
            if(!g_config.StrictDropAdd)
                PlaySound(SFX_Raccoon);
        }
        s_playerState[p] = PlayerState::DropAddMain;
        s_menuItem[p] = 1;
    }

    // escape the drop/add menu
    if(s_playerState[p] == PlayerState::DropAddMain || s_playerState[p] == PlayerState::ReconnectMain)
    {
        s_playerState[p] = PlayerState::StartGame;
        s_menuItem[p] = 0;
        if(CheckDone())
            return true;
        else
            return false;
    }

    PlaySoundMenu(SFX_Slide);

    // the fake back for P1 on the Main Menu
    if(s_context == Context::MainMenu && p == 0 && s_playerState[p] == PlayerState::Disconnected)
    {
        if(g_charSelect[p] == 0)
            return true;
        else
        {
            g_charSelect[p] = 0;
            return false;
        }
    }

    if(s_playerState[p] == PlayerState::StartGame)
    {
        if(s_context == Context::MainMenu)
        {
            s_playerState[p] = PlayerState::SelectChar;
            s_menuItem[p] = g_charSelect[p] - 1;
            g_charSelect[p] = 0;
        }
        else if(s_context == Context::DropAdd)
        {
            s_playerState[p] = PlayerState::DropAddMain;
            s_menuItem[p] = 0;
        }
        else if(s_context == Context::Reconnect)
        {
            s_playerState[p] = PlayerState::ReconnectMain;
            s_menuItem[p] = 0;
        }
    }
    else if(s_playerState[p] == PlayerState::ControlsMenu)
    {
        if(s_context == Context::MainMenu || (s_context == Context::DropAdd && p >= numPlayers))
        {
            s_playerState[p] = PlayerState::SelectChar;
            s_menuItem[p] = 0;
            Player_ValidateChar(p);
        }
        else if(s_context == Context::DropAdd)
        {
            s_playerState[p] = PlayerState::DropAddMain;
            s_menuItem[p] = 0;
        }
        else if(s_context == Context::Reconnect)
        {
            s_playerState[p] = PlayerState::ReconnectMain;
            s_menuItem[p] = 0;
        }
    }
    else if(s_playerState[p] == PlayerState::SelectProfile)
    {
        s_playerState[p] = PlayerState::ControlsMenu;
        s_menuItem[p] = 0;
    }

    return false;
}

bool Player_Select(int p)
{
    s_inputReady[p] = false;

    // plays SFX_Do at end of scope unless inactivated
    PlayDoSentinel do_sentinel;

    // controls menu
    if(s_playerState[p] == PlayerState::ControlsMenu)
    {
        if(s_menuItem[p] == 2)
        {
            Controls::DeleteInputMethod(Controls::g_InputMethods[p]);
            s_playerState[p] = PlayerState::Reconnecting;
            s_menuItem[p] = 0;
            if(s_context == Context::MainMenu && s_minPlayers == 1)
            {
                Controls::DeleteInputMethodSlot(p);
                if(g_charSelect[p] == 0)
                    Player_ValidateChar(p);
            }
            else if(s_context == Context::MainMenu)
                g_charSelect[p] = 0;
            do_sentinel.active = false;
            return false;
        }
        else if(s_menuItem[p] == 1)
        {
            s_playerState[p] = PlayerState::TestControls;
            return false;
        }
        else if(s_menuItem[p] == 0)
        {
            // only enter the profile select screen if safe to do so.
            if(Controls::g_InputMethods[p])
            {
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
        }        
        return false;
    }

    // allow mouse to work for P1 in main menu
    if(s_context == Context::MainMenu && s_minPlayers == 1 && p == 0 && s_playerState[p] == PlayerState::Disconnected)
    {
        if(g_charSelect[p] == 0)
        {
            g_charSelect[p] = s_menuItem[p]+1;
            // signal to reset player state to Disconnected
            s_playerState[p] = PlayerState::SelectChar;
            s_menuItem[p] = -1;
        }
        else
        {
            do_sentinel.active = false;
            return false;
        }
    }

    if(s_playerState[p] == PlayerState::SelectChar)
    {
        if(s_menuItem[p] != -1)
            g_charSelect[p] = s_menuItem[p]+1;
        if(s_context == Context::MainMenu)
        {
            if(s_menuItem[p] == -1)
            {
                s_playerState[p] = PlayerState::Disconnected;
                s_menuItem[p] = 0;
            }
            else
            {
                s_playerState[p] = PlayerState::StartGame;
                s_menuItem[p] = -4;
            }
            if(CheckDone())
                return true;
        }
        else
        {
            // set this player as having been selected
            if(0 <= s_menuItem[p] && s_menuItem[p] < 5)
                s_char_info.char_present[s_menuItem[p]] = true;

            s_playerState[p] = PlayerState::DropAddMain;
            if(p+1 <= numPlayers)
            {
                s_menuItem[p] = 1;
                if(g_charSelect[p] != Player[p+1].Character)
                {
                    SwapCharacter(p+1, g_charSelect[p], g_config.StrictDropAdd && !SwapCharAllowed());
                    if(!g_config.StrictDropAdd || SwapCharAllowed())
                        PlaySound(SFX_Raccoon);
                }
                do_sentinel.active = false;
            }
            else
            {
                // swap p with the first non-existent player slot
                s_menuItem[p] = 0;
                Player_Swap(numPlayers, p);

                // AddPlayer increments numPlayers by 1
                AddPlayer(g_charSelect[numPlayers]);
                s_char_info.max_players = numPlayers;

                // spend a life if StrictDropAdd enabled
                if(g_config.StrictDropAdd && !LevelSelect)
                {
                    if(Lives <= 0)
                        Player[numPlayers].Dead = true;
                    else
                        Lives -= 1;
                }
                PlaySound(SFX_Mushroom);
                do_sentinel.active = false;
            }
        }
    }
    else if(s_playerState[p] == PlayerState::ReconnectMain)
    {
        s_playerState[p] = PlayerState::StartGame;
        if(CheckDone())
            return true;
        s_menuItem[p] = 0;
    }
    else if(s_playerState[p] == PlayerState::DropAddMain)
    {
        int i = 0;
        if(s_menuItem[p] == i++)
        {
            s_playerState[p] = PlayerState::ControlsMenu;
        }
        else if(SwapCharAllowed() && s_menuItem[p] == i++)
        {
            s_playerState[p] = PlayerState::SelectChar;
            s_menuItem[p] = g_charSelect[p] - 1;
            g_charSelect[p] = 0;
        }
        else if(numPlayers > s_minPlayers && s_menuItem[p] == i++)
        {
            Player_Remove(p);
            do_sentinel.active = false;
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
        else if(s_menuItem[p] == -4)
        {
            do_sentinel.active = false;
        }
        else if(s_context == Context::DropAdd || (s_context == Context::Reconnect && g_compatibility.allow_drop_add))
        {
            if(s_menuItem[p] == 0 && numPlayers > s_minPlayers) // DROP NOT DONE
            {
                Context c = s_context;
                s_context = Context::DropAdd;
                DropNotDone(true);
                s_context = c;
                return true;
            }
            else if(s_menuItem[p] == 0) // FORCE RESUME
            {
                DropNotDone(false);
                return true;
            }
        }
        else if(s_context == Context::Reconnect)
        {
            if(s_menuItem[p] == 0) // FORCE RESUME
                return true;
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
        int i = 0;
        for(i = 0; i < 5; i++)
        {
            if(s_menuItem[p] == 0)
                s_menuItem[p] = 5;
            s_menuItem[p] --;
            if(CharAvailable(s_menuItem[p]+1, p))
                break;
        }
        // if can't traverse normally, allow duplicates
        if(i == 5)
        {
            for(i = 0; i < 5; i++)
            {
                if(s_menuItem[p] == 0)
                    s_menuItem[p] = 5;
                s_menuItem[p] --;
                if(!blockCharacter[s_menuItem[p]+1] && s_char_info.accept(s_menuItem[p]))
                    break;
            }
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
    if(s_playerState[p] == PlayerState::DropAddMain)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 0)
        {
            s_menuItem[p] = DropAddMain_ItemCount() - 1;
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
    }
    if(s_playerState[p] == PlayerState::ControlsMenu)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 0)
            s_menuItem[p] = 2;
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
        int i;
        for(i = 0; i < 5; i++)
        {
            s_menuItem[p] ++;
            if(s_menuItem[p] == 5)
                s_menuItem[p] = 0;
            if(CharAvailable(s_menuItem[p]+1, p))
                break;
        }
        // if can't traverse normally, allow duplicates
        if(i == 5)
        {
            for(i = 0; i < 5; i++)
            {
                s_menuItem[p] ++;
                if(s_menuItem[p] == 5)
                    s_menuItem[p] = 0;
                if(!blockCharacter[s_menuItem[p]+1] && s_char_info.accept(s_menuItem[p]))
                    break;
            }
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
    if(s_playerState[p] == PlayerState::DropAddMain)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == DropAddMain_ItemCount() - 1)
            s_menuItem[p] = 0;
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
    }
    if(s_playerState[p] == PlayerState::ControlsMenu)
    {
        PlaySoundMenu(SFX_Slide);
        if(s_menuItem[p] == 2)
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

inline void Render_PCursor(int x, int y, float r, float g, float b)
{
    XRender::renderTextureFL(x, y, GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, 90.0, nullptr, X_FLIP_NONE, r, g, b);
}

// render the character select screen
void Chars_Mouse_Render(int x, int w, int y, int h, bool mouse, bool render)
{
    int menu_w = 200;
    if(menu_w > w)
        menu_w = w;
    int menu_x = x + w/2 - menu_w/2;

    int line = h/5;

    int menuPlayer = GetMenuPlayer();

    // render character by character
    for(int c = 0; c < 5; c++)
    {
        // enable mousing only if the menu player is on the char select screen
        bool menuPlayer_active;
        if(menuPlayer < (int)Controls::g_InputMethods.size() && s_playerState[menuPlayer] == PlayerState::SelectChar)
            menuPlayer_active = true;
        else if(menuPlayer == 0 && s_context == Context::MainMenu && s_minPlayers == 1
            && s_playerState[menuPlayer] == PlayerState::Disconnected && g_charSelect[menuPlayer] == 0)
        {
            menuPlayer_active = true;
        }
        else
            menuPlayer_active = false;

        // only mouse available chars
        if(mouse && menuPlayer_active && CharAvailable(c+1, menuPlayer))
        {
            Player_MenuItem_Mouse_Render(menuPlayer, c, g_mainMenu.selectPlayer[c+1],
                menu_x, y+c*line, mouse, false);
        }
        if(render)
        {
            // determine if it belongs to a character
            float r = 1.f;
            float g = 1.f;
            float b = 1.f;
            float a = 1.f;
            if(blockCharacter[c+1] || !s_char_info.accept(c))
                a = 0.2f;

            for(int p = 0; p < maxLocalPlayers; p++)
            {
                // player's color
                float pr = (p == 0 ? 1.f : 0.f);
                float pg = (p == 1 ? 1.f : 0.f);
                float pb = (p > 1 ? 1.f : 0.f);
                float pa = 1.f;

                // render cursor if a player is currently (or pretend) selecting it
                bool act_select_char;
                if(s_playerState[p] == PlayerState::SelectChar)
                    act_select_char = true;
                else if(p == 0 && s_context == Context::MainMenu && s_minPlayers == 1 && s_playerState[p] == PlayerState::Disconnected && g_charSelect[p] == 0)
                    act_select_char = true;
                else
                    act_select_char = false;
                if(act_select_char && s_menuItem[p] == c)
                {
                    Render_PCursor(menu_x - 20, y+c*line, pr, pg, pb);

                    // do the fun player transformation thing!
                    // This WILL switch certain entities back and forth
                    //   over the course of this function when multiple
                    //   players are selecting their character.
                    if(s_context == Context::MainMenu)
                    {
                        for(int A = 1; A <= numPlayers; A++)
                        {
                            if(A % (p + 1) == 0)
                            {
                                if(Player[A].Character != c+1)
                                {
                                    Player[A].Character = c+1;
                                    SizeCheck(A);
                                }
                            }
                        }

                        for(int A = 1; A <= numNPCs; A++)
                        {
                            if(A % (p + 1) == 0 && NPC[A].Type == 13)
                                NPC[A].Special = c+1;
                        }
                    }
                }

                // also signal if some player has already selected it
                bool player_okay = (s_playerState[p] != PlayerState::Disconnected);
                if(p == 0 && s_context == Context::MainMenu && s_minPlayers == 1 && s_playerState[p] == PlayerState::Disconnected)
                    player_okay = true;
                // render disconnected players as transparent
                if(!player_okay && p < numPlayers
                    && (s_context == Context::DropAdd || s_context == Context::Reconnect))
                {
                    player_okay = true;
                    if(BlockFlash % 45 < 25)
                        pa = 0.5f;
                }
                if(player_okay && g_charSelect[p] == c+1)
                {
                    r = pr;
                    g = pg;
                    b = pb;
                    a = pa;
                }
            }
            SuperPrint(g_mainMenu.selectPlayer[c+1], 3, menu_x, y+c*line, r, g, b, a);
        }
    }
}

// each player gets a precious 5 lines to display and handle as much unique info as possible
bool Player_Mouse_Render(int p, int pX, int cX, int pY, int line, bool mouse, bool render)
{
    bool ret = false;

    // player's color
    float r = (p == 0 ? 1.f : 0.f);
    float g = (p == 1 ? 1.f : 0.f);
    float b = (p > 1 ? 1.f : 0.f);

    // note that all cursor rendering is done using MCursor[1] (the scroll-up indicator), rotated,
    //   because the standard cursor icons are already tinted.

    // render the player's header
    if(render && s_playerState[p] != PlayerState::SelectProfile && !(s_context == Context::MainMenu && s_minPlayers == 1))
        SuperPrintCenter(fmt::format_ne("{0} {1}", g_mainMenu.wordPlayer, p+1), 3, cX, pY, r, g, b);

    // now render / process the player's menu as appropriate to its case

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

        if(render)
            SuperPrintCenter(g_mainMenu.wordProfiles, 3, cX, pY);

        int start_y = pY + 1*line;

        // calculate scroll (top rendered index) here
        int total_lines = (int)profiles.size() + 1;
        int avail_lines = 4;

        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
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
        if(render && s_menuItem[p] >= 0)
            Render_PCursor(pX - 20, start_y + (s_menuItem[p]-scroll_start)*line, r, g, b);

        for(int i = scroll_start; i < scroll_end; i++)
        {
            std::string* name = &g_mainMenu.wordBack;
            if(i != (int)profiles.size())
                name = &(profiles[i]->Name);
            ret |= Player_MenuItem_Mouse_Render(p, i, *name,
                pX, start_y + (i-scroll_start)*line, mouse, render);
        }
    }

    // render the profile confirmation and test controls screens
    if(s_playerState[p] == PlayerState::ConfirmProfile || s_playerState[p] == PlayerState::TestControls)
    {
        if(render)
        {
            RenderControls(p+1, cX-38, pY+1.25*line, 76, 30);
            SuperPrintCenter(g_mainMenu.phraseHoldStartToReturn, 3, cX, pY+3*line);
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
            SuperPrintCenter(squeeze, 4, cX, pY+4*line);
        }
    }

    // render the main DropAdd screen
    if(s_playerState[p] == PlayerState::DropAddMain)
    {
        int i = 0;
        // show the cursor for the player
        if(render && s_menuItem[p] >= 0)
            Render_PCursor(pX - 20, pY + (1+s_menuItem[p])*line, r, g, b);

        ret |= Player_MenuItem_Mouse_Render(p, 0, "SET CONTROLS",
            pX, pY+(1)*line, mouse, render);
        i++;
        if(SwapCharAllowed())
        {
            ret |= Player_MenuItem_Mouse_Render(p, i, "CHANGE CHAR",
                pX, pY+(1+i)*line, mouse, render);
            i++;
        }
        if(numPlayers > s_minPlayers)
        {
            ret |= Player_MenuItem_Mouse_Render(p, i, "DROP ME",
                pX, pY+(1+i)*line, mouse, render);
            i++;
        }
    }

    // render the (waiting for other players / game start screen)
    if(s_playerState[p] == PlayerState::StartGame)
    {
        if(CheckDone() && render)
        {
            SuperPrint(g_mainMenu.playerSelStartGame, 3, pX, pY+2*line);
            Render_PCursor(pX - 20, pY + 2*line, r, g, b);
        }
        else
        {
            // make the "Waiting" text flash
            if(render && s_menuItem[p] == -4)
            {
                if(BlockFlash < 45)
                    SuperPrintCenter(g_mainMenu.wordWaiting, 3, cX, pY+2*line);
            }

            if(s_menuItem[p] != -4)
            {
                // show the cursor for the player
                if(render && s_menuItem[p] >= 0)
                    Render_PCursor(pX - 20, pY + (2+s_menuItem[p])*line, r, g, b);
                if(g_compatibility.allow_drop_add && numPlayers > s_minPlayers)
                {
                    // figure out which player would be dropped...
                    int x = GetMenuPlayer();
                    std::string msg = "DROP PX";
                    msg[6] = '1' + x;
                    ret |= Player_MenuItem_Mouse_Render(p, 0, msg,
                        pX, pY+(2)*line, mouse, render);
                }
                else
                {
                    ret |= Player_MenuItem_Mouse_Render(p, 0, "FORCE RESUME",
                        pX, pY+(2)*line, mouse, render);
                }
            }
        }
    }

    // render the controls menu
    if(s_playerState[p] == PlayerState::ControlsMenu)
    {
        // render the controls stuff for the player
        if(render)
        {
            // global information about controller
            SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, pY + line);

            // show the menu cursor for the player
            Render_PCursor(pX - 20, pY + (2+s_menuItem[p])*line, r, g, b);
        }

        // should never be null
        if(Controls::g_InputMethods[p]->Profile != nullptr)
            Player_MenuItem_Mouse_Render(p, 0, Controls::g_InputMethods[p]->Profile->Name,
                pX, pY + 2*line, mouse, render);
        else
            Player_MenuItem_Mouse_Render(p, 0, g_mainMenu.wordProfile,
                pX, pY + 2*line, mouse, render);
        Player_MenuItem_Mouse_Render(p, 1, g_mainMenu.phraseTestControls,
            pX, pY + 3*line, mouse, render);
        if(s_context != Context::MainMenu)
            Player_MenuItem_Mouse_Render(p, 2, "RECONNECT",
                pX, pY + 4*line, mouse, render);
        else if(Controls::g_InputMethods.size() > 1)
            Player_MenuItem_Mouse_Render(p, 2, g_mainMenu.wordDisconnect,
                pX, pY + 4*line, mouse, render);
        else
            Player_MenuItem_Mouse_Render(p, 2, "RECONNECT",
                pX, pY + 4*line, mouse, render);
    }

    // don't process any of the controls stuff when the player is connecting
    if(s_playerState[p] == PlayerState::Disconnected || s_playerState[p] == PlayerState::Reconnecting
        || p >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[p])
    {
        if(render)
        {
            if(s_context == Context::MainMenu && s_minPlayers == 1 && s_playerState[p] == PlayerState::Disconnected)
            {
                XRender::renderRect(ScreenW / 2 - 320, pY + 2.5 * line, 640, 2.5 * line, 0, 0, 0, 0.5);

                SuperPrintCenter("WAITING FOR INPUT DEVICE...", 3, cX, pY + 3*line, 0.8f, 0.8f, 0.8f, 0.8f);
                SuperPrintCenter("PRESS SELECT FOR CONTROLS OPTIONS", 3, cX, pY + 4*line, 0.8f, 0.8f, 0.8f, 0.8f);
            }
            else if(BlockFlash < 45)
                SuperPrintCenter(g_mainMenu.phrasePressAButton, 3, cX, pY+2*line);
        }
        return ret;
    }

    if(s_playerState[p] == PlayerState::SelectChar && s_context != Context::MainMenu && p < numPlayers)
    {
        SuperPrint("CHANGE CHAR", 3, pX, pY+2*line, 0.8f, 0.8f, 0.8f, 0.8f);
        return ret;
    }

    // don't process any of the controls stuff if already rendering something there
    if(s_playerState[p] != PlayerState::ReconnectMain && s_playerState[p] != PlayerState::SelectChar)
        return ret;

    // show the controls info and SELECT TO CHANGE message
    if(render && !(s_context == Context::MainMenu && s_minPlayers == 1))
    {
        if(p < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[p])
        {
            // global information about controller
            SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, pY + line);

            // Profile should never be null
            if(Controls::g_InputMethods[p]->Profile != nullptr)
                SuperPrintCenter(Controls::g_InputMethods[p]->Profile->Name,
                    3, cX, pY + 2*line);
            else
                SuperPrintCenter(g_mainMenu.wordProfile, 3, cX, pY + 2*line);
        }

        SuperPrintCenter("PRESS SELECT FOR", 3, cX, pY + 3*line, 0.8f, 0.8f, 0.8f, 0.8f);
        SuperPrintCenter("CONTROLS OPTIONS", 3, cX, pY + 4*line, 0.8f, 0.8f, 0.8f, 0.8f);
    }
    // show the squashed info for 1P mode
    else if(render)
    {
        XRender::renderRect(ScreenW / 2 - 320, pY + 2.5 * line, 640, 2.5 * line, 0, 0, 0, 0.5);

        if(p < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[p])
        {
            // global information about controller
            // Profile should never be null
            if(Controls::g_InputMethods[p]->Profile != nullptr)
                SuperPrintCenter(Controls::g_InputMethods[p]->Name + " - " + Controls::g_InputMethods[p]->Profile->Name, 3, cX, pY + 3*line);
            else
                SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, pY + 3*line);
        }

        SuperPrintCenter("PRESS SELECT FOR CONTROLS OPTIONS", 3, cX, pY + 4*line, 0.8f, 0.8f, 0.8f, 0.8f);
    }

    return ret;
}

int Mouse_Render(bool mouse, bool render)
{
    if(mouse && !SharedCursor.Move && !render && !SharedCursor.Primary && !SharedCursor.Secondary)
        return 0;

    int n = Controls::g_InputMethods.size();
    if(s_context == Context::DropAdd || (s_context == Context::MainMenu && s_minPlayers != 1))
        n += 1;
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer = GetMenuPlayer();

    // call their Player_Back routine in the correct circumstances
    if(mouse && SharedCursor.Secondary && MenuMouseRelease)
    {
        // normally want to cancel the thing that the previous player did
        MenuMouseRelease = false;
        if(menuPlayer == 0)
        {
            if(Player_Back(menuPlayer))
                return -1;
        }
        else
        {
            if(Player_Back(menuPlayer-1))
                return -1;
        }
    }


    /*--------------------*\
    || Get screen pos     ||
    \*--------------------*/

    // there are 15 rows of text; 30px each by default.
    // line height is scaled so that these fit onscreen
    // with at least 30px of padding on either side
    int max_line = 15;
    int line = (ScreenH - 60) / max_line;
    if(line > 30)
        line = 30;
    if(line < 18)
    {
        line = 18;
        max_line = (ScreenH - 60) / line;
    }
    line -= line & 1;

    // vertical start of the menu
    int sY = ScreenH/2 - (line*max_line)/2;
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

    // horizontal start and width of the menu
    int sX;
    int sW;
    // if scroll mode, must adjust screen start to first unfinished player
    if(scroll_mode)
    {
        sX = -p_width*menuPlayer + 10;
        sW = ScreenW - 20;
    }
    // otherwise, center players on screen
    else
    {
        sX = ScreenW/2 - (p_width*n)/2;
        sX -= sX & 1;
        sW = p_width*n;
    }

    if(render && !(s_context == Context::MainMenu && s_minPlayers == 1))
    {
        XRender::renderRect(sX, sY - (line-16), p_width*n, line*max_line + line-16, 0, 0, 0, .5);
    }

    /*--------------------*\
    || Render menu        ||
    \*--------------------*/

    if(render)
    {
        if(s_context == Context::MainMenu && s_minPlayers == 1)
        {
            SuperPrint(g_mainMenu.main1PlayerGame, 3, 300, 280, 1.0f, 0.3f, 0.3f);
            SuperPrint(SelectWorld[selWorld].WorldName, 3, 300, 310, 0.6f, 1.f, 1.f);
        }
        else if(s_context == Context::MainMenu)
            SuperPrintScreenCenter(g_mainMenu.charSelTitle, 3, sY);
        else if(s_context == Context::DropAdd)
            SuperPrintScreenCenter(g_mainMenu.dropAddTitle, 3, sY);
        else if(s_context == Context::Reconnect)
            SuperPrintScreenCenter(g_mainMenu.reconnectTitle, 3, sY);
    }

    if(s_context == Context::MainMenu && s_minPlayers == 1)
    {
        if(s_playerState[0] == PlayerState::Disconnected || s_playerState[0] == PlayerState::SelectChar)
            Chars_Mouse_Render(300, 200, 350, 150, mouse, render);
    }
    else
        Chars_Mouse_Render(sX, sW, sY+line*2, line*5, mouse, render);

    if(s_context == Context::Reconnect || s_context == Context::DropAdd)
    {
        if(CheckDone())
        {
            if(s_context == Context::Reconnect)
                SuperPrintScreenCenter("PRESS START TO RESUME", 3, sY+line*8);
            else
                SuperPrintScreenCenter("PRESS START TO RESUME", 3, sY+line*8, 0.8f, 0.8f, 0.8f, 0.8f);
        }
        else
        {
            // if any players connected, show the force resume text
            int p;
            for(p = 0; p < n; p++)
            {
                if(s_playerState[p] == PlayerState::ReconnectMain || s_playerState[p] == PlayerState::DropAddMain)
                    break;
            }
            if(p != n)
                SuperPrintScreenCenter("PRESS START TO FORCE RESUME", 3, sY+line*8, 0.8f, 0.8f, 0.8f, 0.8f);
        }
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

        if(s_context == Context::MainMenu && s_minPlayers == 1)
        {
            // reconnecting
            if(s_playerState[p] == PlayerState::Reconnecting)
            {
                XRender::renderRect(250, 350, 300, 200, 0, 0, 0, 0.5);
                if(BlockFlash < 45)
                    SuperPrintScreenCenter(g_mainMenu.phrasePressAButton, 3, 440);
            }
            else if(s_playerState[p] == PlayerState::Disconnected || s_playerState[p] == PlayerState::SelectChar)
            {
                Player_Mouse_Render(p, 300, 400, 420, 30, mouse, render);
            }
            else if(s_playerState[p] == PlayerState::ControlsMenu || s_playerState[p] == PlayerState::TestControls)
            {
                XRender::renderRect(250, 350, 300, 200, 0, 0, 0, 0.5);
                Player_Mouse_Render(p, 300, 400, 350, 30, mouse, render);
            }
            else
            {
                XRender::renderRect(250, 350, 300, 200, 0, 0, 0, 0.5);
                Player_Mouse_Render(p, 300, 400, 380, 30, mouse, render);
            }
        }
        else
            Player_Mouse_Render(p, lX, cX, sY+line*(max_line-5), line, mouse, render);
    }

    if(CheckDone())
    {
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
    /*-----------------------*\
    || polling input methods ||
    \*-----------------------*/
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
    // in 1-player mode, only allow a single player to connect
    if(s_context == Context::MainMenu && s_minPlayers == 1)
    {
        if((int)Controls::g_InputMethods.size() >= s_minPlayers)
            block_poll = true;
    }
    // block polling if a player is hitting random buttons
    for(int p = 0; p < maxLocalPlayers; p++)
    {
        if(s_playerState[p] == PlayerState::TestControls || s_playerState[p] == PlayerState::ConfirmProfile)
            block_poll = true;
    }
    if(!block_poll && Controls::PollInputMethod())
    {
        if((int)Controls::g_InputMethods.size() > 1 || !(s_context == Context::MainMenu && s_minPlayers == 1))
        {
            PlaySoundMenu(SFX_DropItem);
        }
    }

    /*-----------------------*\
    ||    Shared back key    ||
    \*-----------------------*/
    if(s_context == Context::MainMenu && Controls::g_InputMethods.size() == 0)
    {
        if(!SharedControls.MenuBack)
            MenuCursorCanMove = true;
        if(SharedControls.MenuBack && MenuCursorCanMove)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursorCanMove = false;
            return -1;
        }
    }

    /*-----------------------*\
    ||   per-player logic    ||
    \*-----------------------*/

    // What is the first player that is not done?
    int menuPlayer = GetMenuPlayer();

    for(int p = 0; p < maxLocalPlayers; p++)
    {
        // hide "Drop Me", etc, if it is no longer allowed
        if(s_context == Context::DropAdd && s_playerState[p] == PlayerState::DropAddMain
            && s_menuItem[p] >= DropAddMain_ItemCount())
        {
            s_menuItem[p] = DropAddMain_ItemCount() - 1;
        }

        // if player doesn't exist, or was lost *not on menu screen*
        if(p >= (int)Controls::g_InputMethods.size()
            || (!Controls::g_InputMethods[p] && s_context != Context::MainMenu))
        {
            s_playerState[p] = PlayerState::Disconnected;
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
        if(s_playerState[p] == PlayerState::Disconnected || s_playerState[p] == PlayerState::Reconnecting)
        {
            s_inputReady[p] = false;
            if(s_context == Context::MainMenu)
            {
                // because P1's UI is shown in 1 player mode on the main menu,
                // allow the first controller to immediately act upon connection
                if(p == 0 && s_minPlayers == 1)
                {
                    s_inputReady[p] = true;
                    if(s_playerState[p] == PlayerState::Reconnecting) // requested reconnect
                    {
                        s_playerState[p] = PlayerState::ControlsMenu;
                        s_menuItem[p] = 2;
                        s_inputReady[p] = false;
                        PlaySoundMenu(SFX_Yoshi);
                    }
                    else if(g_charSelect[p] != 0)
                    {
                        s_playerState[p] = PlayerState::StartGame;
                        s_menuItem[p] = -4;
                    }
                    else
                    {
                        s_playerState[p] = PlayerState::SelectChar;
                        Player_ValidateChar(p);
                    }
                }
                else
                {
                    s_menuItem[p] = 0;
                    s_playerState[p] = PlayerState::SelectChar;
                    Player_ValidateChar(p);
                }
            }
            else if(s_context == Context::DropAdd)
            {
                // this is an Add situation (wouldn't happen if s_context == Context::Reconnect)
                if(p >= numPlayers)
                {
                    s_menuItem[p] = 0;
                    g_charSelect[p] = 0;
                    s_playerState[p] = PlayerState::SelectChar;
                    Player_ValidateChar(p);
                }
                else if(s_playerState[p] == PlayerState::Reconnecting) // requested Reconnect
                {
                    s_playerState[p] = PlayerState::ControlsMenu;
                    s_menuItem[p] = 2;
                }
                else
                {
                    s_playerState[p] = PlayerState::DropAddMain;
                    s_menuItem[p] = 0;
                }
            }
            else if(s_context == Context::Reconnect)
            {
                if(s_playerState[p] == PlayerState::Reconnecting) // requested Reconnect
                {
                    s_playerState[p] = PlayerState::ControlsMenu;
                    s_menuItem[p] = 2;
                }
                else
                {
                    s_playerState[p] = PlayerState::ReconnectMain;
                    s_menuItem[p] = 0;
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

        // enter the controls menu, given Select pressed and in ReconnectMain or in an Add situation
        if(s_playerState[p] == PlayerState::ReconnectMain || (s_playerState[p] == PlayerState::SelectChar && (s_context == Context::MainMenu || p >= numPlayers)))
        {
            if(c.Drop)
            {
                s_playerState[p] = PlayerState::ControlsMenu;
                s_menuItem[p] = 0;
                continue;
            }
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
                    s_playerState[p] = PlayerState::ControlsMenu;
                    s_savedProfile[p] = nullptr;
                    s_menuItem[p] = 0;
                    s_inputReady[p] = false;
                    PlaySoundMenu(SFX_Slide);
                }
            }
            else if((s_playerState[p] == PlayerState::TestControls && s_menuItem[p] > 66*1)
                || (s_playerState[p] == PlayerState::ConfirmProfile && s_menuItem[p] > 66*4))
            {
                PlaySoundMenu(SFX_Do);
                if(s_playerState[p] == PlayerState::TestControls)
                    s_menuItem[p] = 1;
                else
                    s_menuItem[p] = 0;
                s_playerState[p] = PlayerState::ControlsMenu;
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

        if(c.Start && s_context == Context::DropAdd && s_playerState[p] == PlayerState::DropAddMain)
        {
            // Press Start to Resume
            s_playerState[p] = PlayerState::ReconnectMain;
            if(Player_Select(p))
                return 1;
        }
        else if(c.Run || (p == menuPlayer-1 && SharedControls.MenuBack)
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
    BlockFlash += 1;

    if(BlockFlash >= 90)
        BlockFlash = 0;

    Mouse_Render(false, true);
}

// reset allowed char tracking
void SaveChars()
{
    for(int c = 0; c < 5; c++)
        s_char_info.char_present[c] = false;

    s_char_info.max_players = numPlayers;

    for(int A = 1; A <= numPlayers; A++)
    {
        int c = Player[A].Character - 1;
        if(0 <= c && c < 5)
            s_char_info.char_present[c] = true;
    }
}

} // namespace ConnectScreen
