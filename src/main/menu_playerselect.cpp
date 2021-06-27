#include <fmt_format_ne.h>

#include "../global_constants.h"
#include "../controls.h"
#include "../sound.h"
#include "../globals.h"
#include "../graphics.h"

#include "menu_main.h"

static bool s_charSelectDone[maxLocalPlayers] = {false};
static bool s_inputReady[maxLocalPlayers] = {false};
static int s_lastNumInputs = 0;

int g_charSelect[maxLocalPlayers] = {0};

void menuPlayerSelect_Start()
{
    Controls::ClearInputMethods();
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        g_charSelect[i] = 0;
        s_charSelectDone[i] = false;
        s_inputReady[i] = false;
    }
    s_lastNumInputs = Controls::g_InputMethods.size();
}

void menuPlayerSelect_Resume()
{
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        s_inputReady[i] = false;
    }
    s_lastNumInputs = Controls::g_InputMethods.size();
}

bool charAvailable(int c)
{
    if(c < 1 || c > 5)
        return false;
    if(blockCharacter[c])
        return false;
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(i == (int)Controls::g_InputMethods.size())
            break;
        if(s_charSelectDone[i] && g_charSelect[i] == c)
            return false;
    }
    return true;
}


bool menuPlayerSelect_CheckDone(int minPlayers)
{
    int n = Controls::g_InputMethods.size();
    if(n < minPlayers)
        n = minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer;
    for(menuPlayer = 0; menuPlayer < n; menuPlayer++)
    {
        if(!s_charSelectDone[menuPlayer])
            break;
    }

    if(menuPlayer >= minPlayers && (int)Controls::g_InputMethods.size() >= minPlayers
        && (menuPlayer == (int)Controls::g_InputMethods.size() || menuPlayer == maxLocalPlayers))
    {
        // everyone is done!
        return true;
    }

    return false;
}

int menuPlayerSelect_Mouse_Render(bool mouse, bool render, int minPlayers)
{
    if(mouse && !MenuMouseMove && !render && !MenuMouseDown)
        return 0;

    int n = Controls::g_InputMethods.size();
    if(n < minPlayers)
        n = minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int menuPlayer;
    for(menuPlayer = 0; menuPlayer < n; menuPlayer++)
    {
        if(!s_charSelectDone[menuPlayer])
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
        SuperPrintScreenCenter(g_mainMenu.playerSelTitle, 3, sY);
    }

    for(int p = 0; p < n; p++)
    {
        // ensure that each character's selection is still valid
        if(!s_charSelectDone[p])
        {
            for(int i = 0; i < 5; i++)
            {
                if(charAvailable(g_charSelect[p]))
                    break;
                g_charSelect[p] ++;
                if(g_charSelect[p] == 6)
                    g_charSelect[p] = 1;
            }
        }

        int lX = sX + p * p_width;
        int cX = lX + p_width / 2;
        // lX is the position of aligned text
        // cX is the position of centered (possibly long) text
        if(p_width > min_width)
        {
            lX += (p_width - min_width) / 2;
        }
        lX -= lX & 1;
        cX -= cX & 1;

        if(render)
        {
            SuperPrintCenter(fmt::format_ne("{0} {1}", g_mainMenu.wordPlayer, p+1), 3, cX, sY+2*line);
        }

        if(p != 0 && p >= (int)Controls::g_InputMethods.size())
        {
            if(render)
            {
                SuperPrintCenter(g_mainMenu.playerSelAttachController, 3, cX, sY+7*line);
            }
            continue;
        }

        for(int c = 1; c < 6; c++)
        {
            // don't render/mouse unavailable chars, or ones the player didn't pick
            if((!charAvailable(c) || s_charSelectDone[p]) && g_charSelect[p] != c)
                continue;
            if(mouse && !s_charSelectDone[p])
            {
                int menuLen = g_mainMenu.selectPlayer[c].size() * 18;
                if(MenuMouseX >= lX && MenuMouseX <= lX + menuLen
                    && MenuMouseY >= sY+(4+c-1)*line && MenuMouseY <= sY+(4+c-1)*line + 16)
                {
                    if(g_charSelect[p] != c)
                    {
                        PlaySoundMenu(SFX_Slide);
                        g_charSelect[p] = c;
                    }

                    if(MenuMouseRelease && MenuMouseDown)
                    {
                        PlaySoundMenu(SFX_Do);
                        s_charSelectDone[p] = true;
                        MenuMouseRelease = false;
                    }
                }
            }
            if(render)
            {
                SuperPrint(g_mainMenu.selectPlayer[c], 3, lX, sY+(4+c-1)*line);
                if(g_charSelect[p] == c)
                    frmMain.renderTexture(lX - 20, sY+(4+c-1)*line, GFX.MCursor[0]);
            }
        }

        if(render)
        {
            // only happens when first player is connecting
            if(p >= (int)Controls::g_InputMethods.size() || Controls::g_InputMethods[p] == nullptr)
            {
                SuperPrintCenter(g_mainMenu.playerSelAttachController, 3, cX, sY+11*line);
                continue;
            }

            SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, sY+10*line);
            SuperPrintCenter(g_mainMenu.wordProfile + ":", 3, cX, sY+11*line);
            // should never be null
            if(Controls::g_InputMethods[p]->Profile != nullptr)
                SuperPrintCenter(Controls::g_InputMethods[p]->Profile->Name, 3, cX, sY+12*line);
            if(!s_charSelectDone[p])
                SuperPrintCenter(g_mainMenu.rotateProfileMessage, 3, cX, sY+13*line);
        }
    }

    if(menuPlayerSelect_CheckDone(minPlayers))
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

int menuPlayerSelect_MouseLogic(int minPlayers)
{
    return menuPlayerSelect_Mouse_Render(true, false, minPlayers);
}

int menuPlayerSelect_Logic(int minPlayers)
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
        if(!s_charSelectDone[menuPlayer])
            break;
    }

    for(int p = 0; p < maxLocalPlayers; p++)
    {
        // ensure the player's selection is still valid after other player selections
        if(!s_charSelectDone[p])
        {
            for(int i = 0; i < 5; i++)
            {
                if(charAvailable(g_charSelect[p]))
                    break;
                g_charSelect[p] ++;
                if(g_charSelect[p] == 6)
                    g_charSelect[p] = 1;
            }
        }

        if(p >= (int)Controls::g_InputMethods.size())
        {
            s_charSelectDone[p] = false;
            g_charSelect[p] = 0;
            // because P1's UI is always shown,
            // allow the first controller to immediately act upon connection
            if(p == 0)
                s_inputReady[p] = true;
            else
                s_inputReady[p] = false;
            continue;
        }

        // make sure controller not lost
        if(!Controls::g_InputMethods[p])
        {
            Controls::DeleteInputMethodSlot(p);
            s_lastNumInputs -= 1;
            for(int p2 = p; p2+1 < maxLocalPlayers; p2++)
            {
                g_charSelect[p2] = g_charSelect[p2+1];
                s_charSelectDone[p2] = s_charSelectDone[p2+1];
                s_inputReady[p2] = s_inputReady[p2+1];
            }
            // if nobody's left, exit!
            if(Controls::g_InputMethods.empty())
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
            if(s_charSelectDone[p])
            {
                PlaySoundMenu(SFX_Slide);
                s_charSelectDone[p] = false;
            }
            else
            {
                Controls::DeleteInputMethodSlot(p);
                s_lastNumInputs -= 1;
                for(int p2 = p; p2+1 < maxLocalPlayers; p2++)
                {
                    g_charSelect[p2] = g_charSelect[p2+1];
                    s_charSelectDone[p2] = s_charSelectDone[p2+1];
                }
                // if nobody's left, exit!
                if(Controls::g_InputMethods.empty())
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
            s_inputReady[p] = false;
        }
        else if(c.Jump || c.Start || (p == menuPlayer && SharedControls.MenuDo))
        {
            if(!s_charSelectDone[p])
            {
                s_charSelectDone[p] = true;
                PlaySoundMenu(SFX_Do);
            }
            else
            {
                if(menuPlayerSelect_CheckDone(minPlayers))
                    return 1;
            }
            s_inputReady[p] = false;
        }
        // next options only apply if character is still being selected
        else if(s_charSelectDone[p])
        {
            continue;
        }
        else if(c.Down || (p == menuPlayer && SharedControls.MenuDown))
        {
            for(int i = 0; i < 6; i++)
            {
                PlaySoundMenu(SFX_Slide);
                g_charSelect[p] ++;
                if(g_charSelect[p] == 6)
                    g_charSelect[p] = 1;
                if(charAvailable(g_charSelect[p]))
                    break;
            }
            s_inputReady[p] = false;
        }
        else if(c.Up || (p == menuPlayer && SharedControls.MenuUp))
        {
            for(int i = 0; i < 6; i++)
            {
                PlaySoundMenu(SFX_Slide);
                g_charSelect[p] --;
                if(g_charSelect[p] == 0)
                    g_charSelect[p] = 5;
                if(charAvailable(g_charSelect[p]))
                    break;
            }
            s_inputReady[p] = false;
        }
        else if((c.Left || (p == menuPlayer && SharedControls.MenuLeft)) && Controls::g_InputMethods[p])
        {
            // rotate controls profile left
            std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            std::vector<Controls::InputMethodProfile*>::iterator cur_profile
                = std::find(profiles.begin(), profiles.end(), Controls::g_InputMethods[p]->Profile);
            if(profiles.size() > 1 && cur_profile != profiles.end())
            {
                if(cur_profile == profiles.begin())
                    cur_profile = profiles.end() - 1;
                else
                    cur_profile --;
                Controls::SetInputMethodProfile(p, *cur_profile);
                PlaySoundMenu(SFX_Slide);
            }
            else
            {
                PlaySoundMenu(SFX_BlockHit);
            }
            s_inputReady[p] = false;
        }
        else if((c.Right || (p == menuPlayer && SharedControls.MenuRight)) && Controls::g_InputMethods[p])
        {
            // rotate controls profile right
            std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            std::vector<Controls::InputMethodProfile*>::iterator cur_profile
                = std::find(profiles.begin(), profiles.end(), Controls::g_InputMethods[p]->Profile);
            if(profiles.size() > 1 && cur_profile != profiles.end())
            {
                cur_profile ++;
                if(cur_profile == profiles.end())
                    cur_profile = profiles.begin();
                Controls::SetInputMethodProfile(p, *cur_profile);
                PlaySoundMenu(SFX_Slide);
            }
            else
            {
                PlaySoundMenu(SFX_BlockHit);
            }
            s_inputReady[p] = false;
        }
    }

    return menuPlayerSelect_MouseLogic(minPlayers);
}

void menuPlayerSelect_Render(int minPlayers)
{
    menuPlayerSelect_Mouse_Render(false, true, minPlayers);
}
