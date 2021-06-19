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

static bool s_canDelete = false;

static bool s_deleteProfileSel = false;

int menuControls_Mouse_Render(bool mouse, bool render)
{
    if(mouse && !MenuMouseMove && !render && !MenuMouseDown)
        return 0;

    const int n_types = Controls::g_InputMethodTypes.size();

    // want 680px width. if not possible, use double-line mode on settable option screens
    int width = 680;
    if(ScreenW < 720)
    {
        width = ScreenW - 40;
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

    if(render)
    {
        frmMain.renderRect(sX, sY - (line-18), width, line*max_line + line-18, 0, 0, 0, .5);
    }

    // rendering of profile deletion screen
    if(s_deleteProfileSel)
    {
        SuperPrintScreenCenter(g_mainMenu.controlsReallyDeleteProfile, 3, sY);

        if(s_curType < 0 || s_curType >= n_types)
        {
            return 0;
        }
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        const int n_profiles = type->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            return 0;
        }

        Controls::InputMethodProfile* profile = type->GetProfiles()[s_curProfile];

        if(profile)
            SuperPrintScreenCenter(profile->Name, 3, sY+line);

        SuperPrint(g_mainMenu.wordNo, 3, sX+width/4, sY+line*3);
        if(MenuCursor == 0)
            frmMain.renderTexture(sX+width/4-24, sY+line*3, GFX.MCursor[0]);
        SuperPrint(g_mainMenu.wordYes, 3, sX+width/4, sY+line*4);
        if(MenuCursor == 1)
            frmMain.renderTexture(sX+width/4-24, sY+line*4, GFX.MCursor[0]);

        return 0;
    }

    // rendering / mouse for the main controls screen
    if(s_curType == -1)
    {
        if(render)
        {
            SuperPrintScreenCenter(g_mainMenu.controlsTitle, 3, sY);
        }

        // render the types at the top of the screen and the players at the bottom
        if(render)
        {
            SuperPrint(g_mainMenu.controlsDeviceTypes, 3, sX+16, sY+2*line);
        }

        // TODO: scrolling (shouldn't be needed except on crazy systems w/ little screen and lots of controllers)
        for(int i = 0; i < n_types; i++)
        {
            if(render)
            {
                SuperPrint(Controls::g_InputMethodTypes[i]->Name, 3, sX+48, sY+(3+i)*line);
                if(MenuCursor == i)
                    frmMain.renderTexture(sX + 24, sY+(3+i)*line, GFX.MCursor[0]);
            }
        }



        // render the players
        if(render)
        {
            SuperPrintScreenCenter(g_mainMenu.controlsConnected, 3, sY+(max_line-5)*line);
            for(size_t p = 0; p < Controls::g_InputMethods.size(); p++)
            {
                if(!Controls::g_InputMethods[p])
                    continue;
                int cX = sX + 100 + 200*p;
                SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, sY+(max_line-4)*line);
                SuperPrintCenter(g_mainMenu.wordProfile + ":", 3, cX, sY+(max_line-3)*line);
                // should never be null
                if(Controls::g_InputMethods[p]->Profile != nullptr)
                    SuperPrintCenter(Controls::g_InputMethods[p]->Profile->Name, 3, cX, sY+(max_line-2)*line);
                SuperPrintCenter(g_mainMenu.rotateProfileMessage, 3, cX, sY+(max_line-1)*line);
            }
        }
    }
    // rendering / mouse for the input method type screens
    else if(s_curProfile == -1)
    {
        bool double_line = false;
        if(width < 500)
            double_line = true;

        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curType = -1;
            MenuCursorCanMove = false;
            return 0;
        }
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        const int n_options = type->GetSpecialOptionCount();
        std::vector<Controls::InputMethodProfile*> profiles = type->GetProfiles();
        const int n_profiles = profiles.size();

        if(render)
        {
            SuperPrintScreenCenter(type->Name, 3, sY);

            bool in_use = false;

            for(Controls::InputMethod* method : Controls::g_InputMethods)
            {
                if(!method)
                    continue;
                if(method->Type == type)
                    in_use = true;
            }

            if(in_use)
                SuperPrintScreenCenter(g_mainMenu.controlsInUse, 3, sY+16);
            else
                SuperPrintScreenCenter(g_mainMenu.controlsNotInUse, 3, sY+16);
        }

        // first come the profiles, and then the type options.

        // overall title, possibly "OPTIONS" and the options, and "PROFILES" come before the profiles
        int pY = sY + (2+1)*line;

        if(render)
        {
            SuperPrint(g_mainMenu.wordProfiles, 3, sX+16, pY-line);
            SuperPrintRightAlign(g_mainMenu.controlsDeleteKey, 3, sX+width-16, pY-line);
        }

        for(int i = 0; i <= n_profiles; i++)
        {
            if(render)
            {
                if(i == n_profiles)
                    SuperPrint(g_mainMenu.controlsNewProfile, 3, sX+48, pY+i*line);
                else
                    SuperPrint(profiles[i]->Name, 3, sX+48, pY+i*line);
                if(MenuCursor == i)
                    frmMain.renderTexture(sX + 24, pY+i*line, GFX.MCursor[0]);
            }
        }

        // overall title, the profiles, "NEW PROFILE", and "OPTIONS" come before the options
        int oY = sY+(2+1+n_profiles+1+2)*line;

        if(n_options && render)
        {
            SuperPrint(g_mainMenu.mainOptions, 3, sX+16, oY-line);
        }

        for(int i = 0; i < n_options; i++)
        {
            if(double_line)
            {
                if(render)
                {
                    SuperPrint(type->GetOptionName(i), 3, sX+24, oY+(2*i)*line);
                    SuperPrint(type->GetOptionValue(i), 3, sX+48, oY+(2*i+1)*line);
                    if(MenuCursor - n_profiles - 1 == i)
                        frmMain.renderTexture(sX + 24, oY+(2*i+1)*line, GFX.MCursor[0]);
                }
            }
            else
            {
                if(render)
                {
                    SuperPrint(type->GetOptionName(i), 3, sX+48, oY+i*line);
                    SuperPrintRightAlign(type->GetOptionValue(i), 3, sX+width-32, oY+i*line);
                    if(MenuCursor - n_profiles - 1 == i)
                        frmMain.renderTexture(sX + 24, oY+i*line, GFX.MCursor[0]);
                }
            }
        }

    }
    // rendering / mouse for the input method profile screens
    else
    {
        bool double_line = false;
        if(width < 680)
        {
            double_line = true;
            if(line > 20)
                line = 20;
        }
        // should never happen
        if(s_curType < 0 || s_curType >= n_types)
        {
            return 0;
        }
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        const int n_profiles = type->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            return 0;
        }

        Controls::InputMethodProfile* profile = type->GetProfiles()[s_curProfile];
        const int n_options = profile->GetSpecialOptionCount();
        const int n_playerButtons = Controls::PlayerControls::n_buttons;


        if(render)
        {
            SuperPrintScreenCenter(profile->Name, 3, sY);

            bool in_use = false;

            for(Controls::InputMethod* method : Controls::g_InputMethods)
            {
                if(!method)
                    continue;
                if(method->Profile == profile)
                    in_use = true;
            }

            if(in_use)
                SuperPrintScreenCenter(g_mainMenu.controlsInUse, 3, sY+16);
            else
                SuperPrintScreenCenter(g_mainMenu.controlsNotInUse, 3, sY+16);
        }

        // first come the player buttons, then the profile options.

        // overall title and "BUTTONS" come before the buttons
        int bY = sY+(2+1)*line;

        if(render)
        {
            SuperPrint(g_mainMenu.wordButtons, 3, sX+16, bY-line);
            SuperPrintRightAlign(g_mainMenu.controlsDeleteKey, 3, sX+width-16, bY-line);
        }

        for(int i = 0; i < n_playerButtons; i++)
        {
            if(double_line)
            {
                if(render)
                {
                    SuperPrint(Controls::PlayerControls::GetButtonName_UI(i), 3, sX+32+(width-32)/4, bY+(2*i)*line);
                    if(MenuCursor == i && !s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+32, bY+(2*i+1)*line);
                    else
                        SuperPrint(profile->NamePrimaryButton(i), 3, sX+32, bY+(2*i+1)*line);
                    if(MenuCursor == i && s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+32+(width-32)/2, bY+(2*i+1)*line);
                    else
                        SuperPrint(profile->NameSecondaryButton(i), 3, sX+32+(width-32)/2, bY+(2*i+1)*line);
                    if(MenuCursor == i)
                    {
                        if(!s_secondaryInput)
                            frmMain.renderTexture(sX+8, bY+(2*i+1)*line, GFX.MCursor[0]);
                        else
                            frmMain.renderTexture(sX+8+(width-32)/2, bY+(2*i+1)*line, GFX.MCursor[0]);
                    }
                }
            }
            else
            {
                if(render)
                {
                    SuperPrint(Controls::PlayerControls::GetButtonName_UI(i), 3, sX+48, bY+i*line);
                    if(MenuCursor == i && !s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+width-420, bY+i*line);
                    else
                        SuperPrint(profile->NamePrimaryButton(i), 3, sX+width-420, bY+i*line);
                    if(MenuCursor == i && s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+width-210, bY+i*line);
                    else
                        SuperPrint(profile->NameSecondaryButton(i), 3, sX+width-210, bY+i*line);
                    if(MenuCursor == i)
                    {
                        if(!s_secondaryInput)
                            frmMain.renderTexture(sX+width-420-24, bY+i*line, GFX.MCursor[0]);
                        else
                            frmMain.renderTexture(sX+width-210-24, bY+i*line, GFX.MCursor[0]);
                    }
                }
            }
        }

        if(n_options && render)
        {
            SuperPrint(g_mainMenu.mainOptions, 3, sX+16, sY+2*line);
        }


        // overall title, player buttons, and "OPTIONS" come before the options
        int oY;
        if(double_line)
        {
            oY = sY+(2+1+2*n_playerButtons+2)*line;
        }
        else if(n_options)
        {
            oY = sY+(2+1+n_playerButtons+2)*line;
        }

        for(int i = 0; i < n_options; i++)
        {
            if(double_line)
            {
                if(render)
                {
                    SuperPrint(profile->GetOptionName(i), 3, sX+24, oY+(2*i)*line);
                    SuperPrint(profile->GetOptionValue(i), 3, sX+48, oY+(2*i+1)*line);
                    if(MenuCursor - n_playerButtons == i)
                        frmMain.renderTexture(sX + 24, oY+(2*i+1)*line, GFX.MCursor[0]);
                }
            }
            else
            {
                if(render)
                {
                    SuperPrint(profile->GetOptionName(i), 3, sX+48, oY+i*line);
                    SuperPrintRightAlign(profile->GetOptionValue(i), 3, sX+width-32, oY+i*line);
                    if(MenuCursor - n_playerButtons == i)
                        frmMain.renderTexture(sX + 24, oY+i*line, GFX.MCursor[0]);
                }
            }
        }
    }

    return 0;
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
        const int n_playerButtons = Controls::PlayerControls::n_buttons;

        if(MenuCursor < 0 || MenuCursor >= n_playerButtons)
        {
            g_pollingInput = false;
            return 0;
        }

        if((!s_secondaryInput && profile->PollPrimaryButton(MenuCursor))
            || (s_secondaryInput && profile->PollSecondaryButton(MenuCursor)))
        {
            g_pollingInput = false;
            MenuCursorCanMove = false;
            PlaySoundMenu(SFX_PSwitch);
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

    if(delPressed && s_canDelete)
    {
        s_canDelete = false;
    }
    else if(delPressed)
    {
        delPressed = false;
    }
    else
    {
        s_canDelete = true;
    }

    // logic for the profile deletion screen
    if(s_deleteProfileSel)
    {
        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += 2;
        while(MenuCursor >= 2)
            MenuCursor -= 2;

        if(!MenuCursorCanMove)
            return 0;

        if(s_curType < 0 || s_curType >= n_types)
        {
            s_deleteProfileSel = false;
            return 0;
        }
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        const int n_profiles = type->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            s_deleteProfileSel = false;
            return 0;
        }

        Controls::InputMethodProfile* profile = type->GetProfiles()[s_curProfile];

        if(menuBackPress || (menuDoPress && MenuCursor == 0))
        {
            PlaySoundMenu(SFX_Slide);
            s_deleteProfileSel = false;
            MenuCursor = s_curProfile;
            s_curProfile = -1;
            MenuCursorCanMove = false;
        }
        else if(menuDoPress && MenuCursor == 1)
        {
            if(type->DeleteProfile(profile, Controls::g_InputMethods))
            {
                PlaySoundMenu(SFX_BowserKilled);
                s_deleteProfileSel = false;
                MenuCursor = s_curProfile;
                s_curProfile = -1;
            }
            else
                PlaySoundMenu(SFX_BlockHit);
            MenuCursorCanMove = false;
        }
    }
    // logic for the main controls screen
    else if(s_curType == -1)
    {
        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_types;
        while(MenuCursor >= n_types)
            MenuCursor -= n_types;

        if(!MenuCursorCanMove)
            return 0;

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
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        const int n_options = type->GetSpecialOptionCount();
        const int n_profiles = type->GetProfiles().size();

        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_profiles + 1 + n_options;
        while(MenuCursor >= n_profiles + 1 + n_options)
            MenuCursor -= n_profiles + 1 + n_options;

        if(!MenuCursorCanMove)
            return 0;

        // backward navigation
        if(menuBackPress)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursor = s_curType;
            s_curType = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        // first come the profiles, then the type options.

        // forward navigation
        if(menuDoPress && MenuCursor >= 0 && MenuCursor < n_profiles)
        {
            PlaySoundMenu(SFX_Do);
            s_curProfile = MenuCursor;
            s_secondaryInput = false;
            MenuCursor = 0;
            MenuCursorCanMove = false;
            return 0;
        }

        // deletion
        if(delPressed && MenuCursor >= 0 && MenuCursor < n_profiles)
        {
            PlaySoundMenu(SFX_PlayerDied2);
            s_deleteProfileSel = true;
            s_curProfile = MenuCursor;
            MenuCursor = 0;
            MenuCursorCanMove = false;
            return 0;
        }

        // creation
        if(menuDoPress && MenuCursor == n_profiles)
        {
            PlaySoundMenu(SFX_DropItem);
            type->AddProfile();
            MenuCursorCanMove = false;
            return 0;
        }

        // options logic
        if(MenuCursor >= n_profiles + 1 && MenuCursor < n_profiles + 1 + n_options)
        {
            if(menuDoPress)
            {
                PlaySoundMenu(SFX_Slide);
                type->OptionChange(MenuCursor - n_profiles - 1);
                MenuCursorCanMove = false;
            }
            else if(leftPressed)
            {
                PlaySoundMenu(SFX_Slide);
                type->OptionRotateLeft(MenuCursor - n_profiles - 1);
                MenuCursorCanMove = false;
            }
            else if(rightPressed)
            {
                PlaySoundMenu(SFX_Slide);
                type->OptionRotateRight(MenuCursor - n_profiles - 1);
                MenuCursorCanMove = false;
            }
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
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        const int n_profiles = type->GetProfiles().size();

        if(s_curProfile < 0 || s_curProfile >= n_profiles)
        {
            SDL_assert_release(false); // invalid state in controls settings
            PlaySoundMenu(SFX_BlockHit);
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        Controls::InputMethodProfile* profile = type->GetProfiles()[s_curProfile];
        const int n_options = profile->GetSpecialOptionCount();
        const int n_playerButtons = Controls::PlayerControls::n_buttons;

        // first come the player buttons, then the profile options.

        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_options + n_playerButtons;
        while(MenuCursor >= n_options + n_playerButtons)
            MenuCursor -= n_options + n_playerButtons;

        if(!MenuCursorCanMove)
            return 0;

        // backward navigation
        if(menuBackPress)
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursor = s_curProfile;
            s_curProfile = -1;
            MenuCursorCanMove = false;
            return 0;
        }

        // key logic
        if(MenuCursor >= 0 && MenuCursor < n_playerButtons)
        {
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
                profile->DeleteSecondaryButton(MenuCursor);
                MenuCursorCanMove = false;
            }
        }

        // options logic
        if(MenuCursor >= n_playerButtons && MenuCursor < n_playerButtons + n_options)
        {
            if(menuDoPress)
            {
                PlaySoundMenu(SFX_Slide);
                profile->OptionChange(MenuCursor - n_playerButtons);
                MenuCursorCanMove = false;
            }
            else if(leftPressed)
            {
                PlaySoundMenu(SFX_Slide);
                profile->OptionRotateLeft(MenuCursor - n_playerButtons);
                MenuCursorCanMove = false;
            }
            else if(rightPressed)
            {
                PlaySoundMenu(SFX_Slide);
                profile->OptionRotateRight(MenuCursor - n_playerButtons);
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