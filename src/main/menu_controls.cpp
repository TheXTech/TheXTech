#include <fmt_format_ne.h>

#include "../global_constants.h"
#include "../controls.h"
#include "../sound.h"
#include "../globals.h"
#include "../graphics.h"

#include "menu_main.h"

bool g_pollingInput = false;

static int s_curType = -1;
static int s_curProfile = -1;
static bool s_canRotate[maxLocalPlayers] = {false};

static bool s_secondaryInput = false;

int menuControls_Mouse_Render(bool mouse, bool render)
{
    const int n_types = Controls::g_InputMethodTypes.size();

    // want 600px width. if not possible, use double-line mode on settable option screens
    int width = 600;
    bool double_line = false;
    if(ScreenW < 640)
    {
        width = ScreenW - 40;
        double_line = true;
    }

    // want up to 15 lines of text
    int line = (ScreenH - 60) / 15;
    line -= line & 1;
    if(line > 30)
        line = 30;
    // (okay if we don't get 15 lines)
    int max_line = 15;
    if(line < 18)
    {
        line = 18;
        max_line = (int)ScreenH / line;
    }

    // horizontal start of the menu
    int sX = ScreenW/2 - width/2;
    sX -= sX & 1;
    // vertical start of the menu
    int sY = ScreenH/2 - (line*max_line)/2;
    sY -= sY & 1;

    {
        frmMain.renderRect(sX, sY - (line-18), width, line*max_line + line-18, 0, 0, 0, .5);
    }

    // rendering / mouse for the main controls screen
    if(s_curType == -1)
    {
        SuperPrintScreenCenter(g_mainMenu.controlsTitle, 3, sY);

        // render the types at the top of the screen and the players at the bottom
        // TODO: scrolling (shouldn't be needed except on crazy systems w/ little screen and lots of controllers)
        for(int i = 0; i < n_types; i++)
        {
            SuperPrint(Controls::g_InputMethodTypes[i]->Name, 3, sX+32, sY+(2+i)*line);
        }

        // render the players
        SuperPrintScreenCenter(g_mainMenu.controlsConnected, 3, sY+(max_line-5)*line);
        for(int p = 0; p < Controls::g_InputMethods.size(); p++)
        {
            int cX = sX + 100 + 200*p;
            SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, sY+(max_line-4)*line);
            SuperPrintCenter(g_mainMenu.wordProfile + ":", 3, cX, sY+(max_line-3)*line);
            // should never be null
            if(Controls::g_InputMethods[p]->Profile != nullptr)
                SuperPrintCenter(Controls::g_InputMethods[p]->Profile->Name, 3, cX, sY+(max_line-2)*line);
            SuperPrintCenter(g_mainMenu.rotateProfileMessage, 3, cX, sY+(max_line-1)*line);
        }
    }
    // rendering / mouse for the input method type screens
    else if(s_curProfile == -1)
    {
        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curType = -1;
            MenuCursorCanMove = false;
            return 0;
        }
        const int n_options = Controls::g_InputMethodTypes[s_curType]->GetSpecialOptionCount();
        const int n_profiles = Controls::g_InputMethodTypes[s_curType]->GetProfiles().size();


    }
    // rendering / mouse for the input method profile screens
    else
    {
        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curType = -1;
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }
        const int n_typeOptions = Controls::g_InputMethodTypes[s_curType]->GetSpecialOptionCount();
        const int n_profiles = Controls::g_InputMethodTypes[s_curType]->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        Controls::InputMethodProfile* profile = Controls::g_InputMethodTypes[s_curType]->GetProfiles()[s_curProfile];
        const int n_options = profile->GetSpecialOptionCount();
        const int n_playerKeys = Controls::PlayerControls::n_buttons;

        // first come the profile options, then the player keys.

    }
}

int menuControls_Logic()
{
    const int n_types = Controls::g_InputMethodTypes.size();

    if(g_pollingInput)
    {
        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            g_pollingInput = false;
            return 0;
        }
        const int n_profiles = Controls::g_InputMethodTypes[s_curType]->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            g_pollingInput = false;
            return 0;
        }

        Controls::InputMethodProfile* profile = Controls::g_InputMethodTypes[s_curType]->GetProfiles()[s_curProfile];
        const int n_options = profile->GetSpecialOptionCount();
        const int n_playerKeys = Controls::PlayerControls::n_buttons;

        if(MenuCursor < n_options || MenuCursor >= n_options + n_playerKeys)
        {
            g_pollingInput = false;
            return 0;
        }

        size_t key = MenuCursor - n_options;
        if((!s_secondaryInput && profile->PollPrimaryButton(key))
            || (s_secondaryInput && profile->PollSecondaryButton(key)))
        {
            g_pollingInput = false;
            MenuCursorCanMove = false;
            return 0;
        }

        bool cancel = SharedControls.MenuBack;

        for(int i = 0; i < maxLocalPlayers; i++)
        {
            cancel |= Player[i+1].Controls.Run;
        }

        if(MenuCursorCanMove && cancel)
        {
            g_pollingInput = false;
            MenuCursorCanMove = false;
            PlaySoundMenu(SFX_Slide);
            return 0;
        }

        return 0;
    }

    if(!MenuCursorCanMove)
    	return 0;

    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;
    bool leftPressed = SharedControls.MenuLeft;
    bool rightPressed = SharedControls.MenuRight;

    bool menuDoPress = SharedControls.MenuDo;
    bool menuBackPress = SharedControls.MenuBack;

    bool delPressed = false;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t &c = Player[i+1].Controls;

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;

        delPressed |= c.AltJump;
    }

    // logic for the main controls screen
    if(s_curType == -1)
    {
        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_types;
        while(MenuCursor >= n_types)
            MenuCursor -= n_types;

        // back and forward nav
    	if(menuBackPress)
    	{
            PlaySoundMenu(SFX_Slide);
            MenuCursorCanMove = false;
    		return -1;
    	}
        if(menuDoPress && MenuCursor >= 0 && MenuCursor < n_types)
        {
            s_curType = MenuCursor;
            MenuCursor = 0;
            PlaySoundMenu(SFX_Do);
            MenuCursorCanMove = false;
            return 0;
        }

        // rotate player control profiles
        for(int p = 0; p < maxLocalPlayers; p++)
        {
            Controls_t &c = Player[p+1].Controls;

            if(!s_canRotate[p])
            {
                if(!c.Left && !c.Right)
                    s_canRotate[p] = true;
                continue;
            }

            if(c.Left && Controls::g_InputMethods[p])
            {
                // rotate controls profile left
                std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
                std::vector<Controls::InputMethodProfile*>::iterator p_profile
                    = std::find(profiles.begin(), profiles.end(), Controls::g_InputMethods[p]->Profile);
                if(profiles.size() > 1 && p_profile != profiles.end())
                {
                    if(p_profile == profiles.begin())
                        p_profile = profiles.end() - 1;
                    else
                        p_profile --;
                    Controls::g_InputMethods[p]->Profile = *p_profile;
                    PlaySoundMenu(SFX_Slide);
                }
                else
                {
                    PlaySoundMenu(SFX_BlockHit);
                }
                s_canRotate[p] = false;
            }
            else if(c.Right && Controls::g_InputMethods[p])
            {
                // rotate controls profile right
                std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
                std::vector<Controls::InputMethodProfile*>::iterator p_profile
                    = std::find(profiles.begin(), profiles.end(), Controls::g_InputMethods[p]->Profile);
                if(profiles.size() > 1 && p_profile != profiles.end())
                {
                    p_profile ++;
                    if(p_profile == profiles.end())
                        p_profile = profiles.begin();
                    Controls::g_InputMethods[p]->Profile = *p_profile;
                    PlaySoundMenu(SFX_Slide);
                }
                else
                {
                    PlaySoundMenu(SFX_BlockHit);
                }
                s_canRotate[p] = false;
            }
        }
    }
    // logic for the input method type screens
    else if(s_curProfile == -1)
    {
        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curType = -1;
            MenuCursorCanMove = false;
            return 0;
        }
        const int n_options = Controls::g_InputMethodTypes[s_curType]->GetSpecialOptionCount();
        const int n_profiles = Controls::g_InputMethodTypes[s_curType]->GetProfiles().size();

        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_options + n_profiles;
        while(MenuCursor >= n_options + n_profiles)
            MenuCursor -= n_options + n_profiles;

        // backward navigation
        if(menuBackPress)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursor = s_curType;
            s_curType = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        // first come the type options, then the profiles.

        // options logic
        if(MenuCursor >= 0 && MenuCursor < n_options)
        {
            if(menuDoPress)
            {
                PlaySoundMenu(SFX_Slide);
                Controls::g_InputMethodTypes[s_curType]->OptionChange(MenuCursor);
                MenuCursorCanMove = false;
            }
            else if(leftPressed)
            {
                PlaySoundMenu(SFX_Slide);
                Controls::g_InputMethodTypes[s_curType]->OptionRotateLeft(MenuCursor);
                MenuCursorCanMove = false;
            }
            else if(rightPressed)
            {
                PlaySoundMenu(SFX_Slide);
                Controls::g_InputMethodTypes[s_curType]->OptionRotateRight(MenuCursor);
                MenuCursorCanMove = false;
            }
        }

        // forward navigation
        if(menuDoPress && MenuCursor >= n_options && MenuCursor < n_options + n_profiles)
        {
            PlaySoundMenu(SFX_Do);
            s_curProfile = MenuCursor - n_options;
            s_secondaryInput = false;
            MenuCursor = 0;
            MenuCursorCanMove = false;
            return 0;
        }
    }
    // logic for the input method profile screens
    else
    {
        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curType = -1;
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }
        const int n_typeOptions = Controls::g_InputMethodTypes[s_curType]->GetSpecialOptionCount();
        const int n_profiles = Controls::g_InputMethodTypes[s_curType]->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        Controls::InputMethodProfile* profile = Controls::g_InputMethodTypes[s_curType]->GetProfiles()[s_curProfile];
        const int n_options = profile->GetSpecialOptionCount();
        const int n_playerKeys = Controls::PlayerControls::n_buttons;

        // first come the profile options, then the player keys.

        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_options + n_playerKeys;
        while(MenuCursor >= n_options + n_playerKeys)
            MenuCursor -= n_options + n_playerKeys;

        // backward navigation
        if(menuBackPress)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursor = n_typeOptions + s_curProfile;
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        // options logic
        if(MenuCursor >= 0 && MenuCursor < n_options)
        {
            if(menuDoPress)
            {
                PlaySoundMenu(SFX_Slide);
                profile->OptionChange(MenuCursor);
                MenuCursorCanMove = false;
            }
            else if(leftPressed)
            {
                PlaySoundMenu(SFX_Slide);
                profile->OptionRotateLeft(MenuCursor);
                MenuCursorCanMove = false;
            }
            else if(rightPressed)
            {
                PlaySoundMenu(SFX_Slide);
                profile->OptionRotateRight(MenuCursor);
                MenuCursorCanMove = false;
            }
        }

        // key logic
        if(MenuCursor >= n_options && MenuCursor < n_options + n_playerKeys)
        {
            size_t key = MenuCursor - n_options;
            if(menuDoPress)
            {
                PlaySoundMenu(SFX_PSwitch);
                g_pollingInput = true;
                MenuCursorCanMove = false;
                return 0;
            }
            else if(leftPressed || rightPressed)
            {
                PlaySoundMenu(SFX_Slide);
                s_secondaryInput = !s_secondaryInput;
                MenuCursorCanMove = false;
            }
            else if(delPressed && s_secondaryInput)
            {
                PlaySoundMenu(SFX_PlayerDied2);
                profile->DeleteSecondaryButton(key);
                MenuCursorCanMove = false;
            }
        }
    }
    return 0;
}

void menuControls_Render()
{
    menuControls_Mouse_Render(false, true);
}