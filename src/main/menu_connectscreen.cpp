#include <fmt_format_ne.h>

#include "../global_constants.h"
#include "../controls.h"
#include "../sound.h"
#include "../globals.h"
#include "../graphics.h"

#include "menu_connectscreen.h"
#include "menu_main.h"

int g_charSelect[maxLocalPlayers] = {0};

namespace ConnectScreen
{

enum class PlayerState
{
    Disconnected,
    ChoosePlayer,
    SelectChar,
    ConfirmProfile,
    TestControls,
    StartGame
};

enum class Context
{
    MainMenu,
    Reconnect,
    DropAdd
};

static PlayerState s_playerState[maxLocalPlayers] = {PlayerState::Disconnected};
static int s_menuItem[maxLocalPlayers] = {0};
static bool s_inputReady[maxLocalPlayers] = {false};
static int s_lastNumInputs = 0;
static int s_minPlayers = 1;
static Context s_context;

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
    s_lastNumInputs = Controls::g_InputMethods.size();
}

// void menuPlayerSelect_Resume()
// {
//     for(int i = 0; i < maxLocalPlayers; i++)
//     {
//         s_inputReady[i] = false;
//     }
//     s_lastNumInputs = Controls::g_InputMethods.size();
// }

bool CharAvailable(int c)
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
    int n = Controls::g_InputMethods.size();
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer;
    for(menuPlayer = 0; menuPlayer < n; menuPlayer++)
    {
        if(s_playerState[menuPlayer] != PlayerState::StartGame)
            break;
    }

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
    if(s_playerState[p] == PlayerState::SelectChar)
    {
        for(int i = 0; i < 5; i++)
        {
            if(CharAvailable(s_menuItem[p]+1))
                break;
            s_menuItem[p] ++;
            if(s_menuItem[p] == 5)
                s_menuItem[p] = 0;
        }
    }
}

bool Player_Remove(int p)
{
    Controls::DeleteInputMethodSlot(p);
    s_lastNumInputs -= 1;
    for(int p2 = p; p2+1 < maxLocalPlayers; p2++)
    {
        g_charSelect[p2] = g_charSelect[p2+1];
        s_playerState[p2] = s_playerState[p2+1];
        s_menuItem[p2] = s_menuItem[p2+1];
        s_inputReady[p2] = s_inputReady[p2+1];
    }
    // in drop-add, will need to take even more drastic actions

    if(Controls::g_InputMethods.empty())
        return true;
    return false;
}

bool Player_Select(int p)
{
    s_inputReady[p] = false;
    if(s_playerState[p] == PlayerState::SelectChar)
    {
        g_charSelect[p] = s_menuItem[p]+1;
        s_playerState[p] = PlayerState::StartGame;
        if(CheckDone())
            return true;
    }
    else if(s_playerState[p] == PlayerState::StartGame)
    {
        if(CheckDone())
            return true;
    }

    PlaySoundMenu(SFX_Do);
    return false;
}

void Player_Back(int p)
{
    s_inputReady[p] = false;
    if(s_playerState[p] == PlayerState::SelectChar)
    {
        DeleteInputMethod(Controls::g_InputMethods[p]);
        s_playerState[p] = PlayerState::Disconnected;
        return;
    }

    PlaySoundMenu(SFX_Slide);
    if(s_playerState[p] == PlayerState::StartGame)
    {
        s_playerState[p] = PlayerState::SelectChar;
        s_menuItem[p] = g_charSelect[p] - 1;
        g_charSelect[p] = 0;
    }
}

void Player_Up(int p)
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
            if(s_menuItem[p] < 0 || CharAvailable(g_charSelect[p]+1))
                break;
        }
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
            if(s_menuItem[p] == -3)
                s_menuItem[p] = 5;
            s_menuItem[p] --;
            if(s_menuItem[p] < 0 || CharAvailable(g_charSelect[p]+1))
                break;
        }
    }
}

bool Player_MouseItem(int p, int i)
{
    if(s_menuItem[p] != i)
    {
        PlaySoundMenu(SFX_Slide);
        s_menuItem[p] = i;
    }

    if(MenuMouseRelease && MenuMouseDown)
    {
        MenuMouseRelease = false;
        return Player_Select(p);
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
    }

    // now render / process the player's menu as appropriate to its case

    // if player is disconnected, show the attach controller message and return
    if(p != 0 && s_playerState[p] == PlayerState::Disconnected)
    {
        if(render)
            SuperPrintCenter(g_mainMenu.playerSelAttachController, 3, cX, sY+7*line);
        return ret;
    }

    // show the menu cursor for the player
    if(render)
    {
        if(s_menuItem[p] >= 0)
            frmMain.renderTexture(pX - 20, sY+(4+s_menuItem[p])*line, GFX.MCursor[0]);
        else
            frmMain.renderTexture(pX - 20, sY+(14+s_menuItem[p])*line, GFX.MCursor[0]);
    }

    // render the character select screen
    if((p == 0 && s_playerState[p] == PlayerState::Disconnected)
        || s_playerState[p] == PlayerState::SelectChar)
    {
        Player_ValidateChar(p);
        for(int c = 0; c < 5; c++)
        {
            // don't render/mouse unavailable chars
            if(!CharAvailable(c+1))
                continue;
            if(mouse)
            {
                int menuLen = g_mainMenu.selectPlayer[c+1].size() * 18;
                if(MenuMouseX >= pX && MenuMouseX <= pX + menuLen
                    && MenuMouseY >= sY+(4+c)*line && MenuMouseY <= sY+(4+c)*line + 16)
                {
                    ret |= Player_MouseItem(p, c);
                }
            }
            if(render)
            {
                SuperPrint(g_mainMenu.selectPlayer[c+1], 3, pX, sY+(4+c)*line);
            }
        }
    }

    // only happens when first player is connecting
    if(s_playerState[p] == PlayerState::Disconnected
        || p >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[p])
    {
        if(render)
            SuperPrintCenter(g_mainMenu.playerSelAttachController, 3, cX, sY+11*line);
        return ret;
    }

    // global information about controller
    if(render)
        SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, sY+10*line);
    if(render)
    {
        // should never be null
        if(Controls::g_InputMethods[p]->Profile != nullptr)
            SuperPrintCenter(Controls::g_InputMethods[p]->Profile->Name, 3, cX, sY+11*line);
        else
            SuperPrintCenter(g_mainMenu.wordProfile, 3, cX, sY+11*line);
        SuperPrintCenter(g_mainMenu.phraseTestControls, 3, cX, sY+12*line);
        SuperPrintCenter(g_mainMenu.wordDisconnect, 3, cX, sY+13*line);
    }
    if(mouse)
    {
        if(MenuMouseX >= pX && MenuMouseX <= pX+pW)
        {
            if(MenuMouseY >= sY+11*line && MenuMouseY <= sY+11*line + 16)
                ret |= Player_MouseItem(p, -3);
            if(MenuMouseY >= sY+12*line && MenuMouseY <= sY+12*line + 16)
                ret |= Player_MouseItem(p, -2);
            if(MenuMouseY >= sY+13*line && MenuMouseY <= sY+13*line + 16)
                ret |= Player_MouseItem(p, -1);
        }
    }
    return ret;
}

int Mouse_Render(bool mouse, bool render)
{
    if(mouse && !MenuMouseMove && !render && !MenuMouseDown)
        return 0;

    int n = Controls::g_InputMethods.size();
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer;
    for(menuPlayer = 0; menuPlayer < n; menuPlayer++)
    {
        if(s_playerState[menuPlayer] != PlayerState::StartGame)
            break;
    }

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
        frmMain.renderRect(sX, sY - (line-16), p_width*n, line*15 + line-16, 0, 0, 0, .5);
    }

    /*--------------------*\
    || Render menu        ||
    \*--------------------*/

    if(render)
    {
        if(s_context == Context::MainMenu)
            SuperPrintScreenCenter(g_mainMenu.charSelTitle, 3, sY);
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
        if(mouse && MenuMouseRelease && MenuMouseDown)
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
    if((int)Controls::g_InputMethods.size() > s_lastNumInputs)
    {
        s_lastNumInputs = (int)Controls::g_InputMethods.size();
        if((int)Controls::g_InputMethods.size() > 1)
        {
            PlaySoundMenu(SFX_DropItem);
        }
    }

    // What is the first player that is not done?
    int menuPlayer;
    for(menuPlayer = 0; menuPlayer < maxLocalPlayers; menuPlayer++)
    {
        if(menuPlayer == (int)Controls::g_InputMethods.size())
            break;
        if(s_playerState[menuPlayer] != PlayerState::StartGame)
            break;
    }

    for(int p = 0; p < maxLocalPlayers; p++)
    {
        Player_ValidateChar(p);

        // if player doesn't exist, or was lost *not on menu screen*
        if(p >= (int)Controls::g_InputMethods.size()
            || (!Controls::g_InputMethods[p] && s_context != Context::MainMenu))
        {
            s_playerState[p] = PlayerState::Disconnected;
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

        const Controls_t& c = Player[p+1].Controls;

        if(!s_inputReady[p])
        {
            if(!SharedControls.MenuDo && !SharedControls.MenuBack && !SharedControls.MenuDown
                && !SharedControls.MenuUp && !SharedControls.MenuLeft && !SharedControls.MenuRight
                && !c.Jump && !c.Start && !c.Run && !c.Down && !c.Up && !c.Left && !c.Right)
            {
                s_inputReady[p] = true;
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
            Player_Back(p);
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
