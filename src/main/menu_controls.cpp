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
        if(render)
        {
            SuperPrintScreenCenter(g_mainMenu.controlsReallyDeleteProfile, 3, sY);
        }

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

        if(profile && render)
            SuperPrintScreenCenter(profile->Name, 3, sY+line);

        if(render)
        {
            SuperPrint(g_mainMenu.wordNo, 3, sX+width/4, sY+line*3);
            if(MenuCursor == 0)
                frmMain.renderTexture(sX+width/4-24, sY+line*3, GFX.MCursor[0]);
            SuperPrint(g_mainMenu.wordYes, 3, sX+width/4, sY+line*4);
            if(MenuCursor == 1)
                frmMain.renderTexture(sX+width/4-24, sY+line*4, GFX.MCursor[0]);
        }

        if(mouse)
        {
            // collision and click handling for "NO"
            int menuLen = g_mainMenu.wordNo.size() * 18;
            if(MenuMouseX >= sX+width/4 && MenuMouseX <= sX+width/4 + menuLen
                && MenuMouseY >= sY+line*3 && MenuMouseY <= sY+line*3 + 16)
            {
                if(MenuCursor != 0)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = 0;
                }

                if(MenuMouseRelease && MenuMouseDown)
                {
                    PlaySoundMenu(SFX_Slide);
                    s_deleteProfileSel = false;
                    MenuCursor = s_curProfile;
                    s_curProfile = -1;
                    MenuMouseRelease = false;
                }
            }
            // collision and click handling for "YES"
            menuLen = g_mainMenu.wordYes.size() * 18;
            if(MenuMouseX >= sX+width/4 && MenuMouseX <= sX+width/4 + menuLen
                && MenuMouseY >= sY+line*4 && MenuMouseY <= sY+line*4 + 16)
            {
                if(MenuCursor != 1)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = 1;
                }

                if(MenuMouseRelease && MenuMouseDown)
                {
                    if(profile && type->DeleteProfile(profile, Controls::g_InputMethods))
                    {
                        PlaySoundMenu(SFX_BowserKilled);
                        s_deleteProfileSel = false;
                        MenuCursor = s_curProfile;
                        s_curProfile = -1;
                    }
                    else
                    {
                        PlaySoundMenu(SFX_BlockHit);
                    }
                    MenuMouseRelease = false;
                }
            }
        }

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

        int scroll_start = 0;
        int scroll_end = n_types;

        if(max_line - 9 < n_types)
        {
            int scroll_n = max_line - 9;
            scroll_start = MenuCursor - scroll_n/2;
            scroll_end = scroll_start + scroll_n;
            if(scroll_start < 0)
            {
                scroll_start = 0;
                scroll_end = scroll_start + scroll_n;
            }
            if(scroll_end > n_types)
            {
                scroll_end = n_types;
                scroll_start = scroll_end - scroll_n;
            }
        }
        for(int i = 0; i < scroll_end - scroll_start; i++)
        {
            if(render)
            {
                SuperPrint(Controls::g_InputMethodTypes[scroll_start + i]->Name, 3, sX+48, sY+(3+i)*line);
                if(MenuCursor == scroll_start + i)
                    frmMain.renderTexture(sX + 24, sY+(3+i)*line, GFX.MCursor[0]);
            }
            int item_width = Controls::g_InputMethodTypes[scroll_start + i]->Name.size()*18;
            if(mouse && MenuMouseX >= sX+48 && MenuMouseX <= sX+48 + item_width
                && MenuMouseY >= sY+(3+i)*line && MenuMouseY <= sY+(3+i)*line + 16)
            {
                if(MenuCursor != scroll_start + i)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = scroll_start + i;
                }

                if(MenuMouseRelease && MenuMouseDown)
                {
                    PlaySoundMenu(SFX_Do);
                    s_curType = MenuCursor;
                    MenuCursor = 0;
                    MenuMouseRelease = false;
                }
            }
        }
        if(render)
        {
            if(scroll_start > 0)
                frmMain.renderTexture(sX + width / 2 - GFX.MCursor[1].w / 2, sY+3*line - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < n_types)
                frmMain.renderTexture(sX + width / 2 - GFX.MCursor[2].w / 2, sY+(3+scroll_end-scroll_start)*line, GFX.MCursor[2]);
        }



        // render the players
        if(render)
        {
            SuperPrint(g_mainMenu.controlsConnected, 3, sX+16, sY+(max_line-5)*line);
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
                {
                    in_use = true;
                    break;
                }
            }

            if(in_use)
                SuperPrintScreenCenter(g_mainMenu.controlsInUse, 3, sY+16);
            else
                SuperPrintScreenCenter(g_mainMenu.controlsNotInUse, 3, sY+16);
        }

        // first come the profiles, and then the type options.
        // work out scrolling here

        int avail_lines = max_line - 2;

        int total_lines;
        if(double_line)
            total_lines = 1 + (n_profiles+1) + 2 + n_options*2;
        else
            total_lines = 1 + (n_profiles+1) + 2 + n_options;

        int cur_line;
        if(MenuCursor <= n_profiles)
            cur_line = 1 + MenuCursor;
        else if(double_line)
            cur_line = 1 + (n_profiles+1) + 2 + (MenuCursor-(n_profiles+1))*2;
        else
            cur_line = 1 + (n_profiles+1) + 2 + (MenuCursor-(n_profiles+1));

        // handle scrolling
        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
            int scroll_n = avail_lines;
            scroll_start = cur_line - scroll_n/2;
            scroll_end = scroll_start + scroll_n;
            if(scroll_start < 0)
            {
                scroll_start = 0;
                scroll_end = scroll_start + scroll_n;
            }
            if(scroll_end > total_lines)
            {
                scroll_end = total_lines;
                scroll_start = scroll_end - scroll_n;
            }
        }

        // overall title and "PROFILES" come before the profiles
        int start_y = sY + 2*line;

        if(render && 0 >= scroll_start && 0 < scroll_end)
        {
            SuperPrint(g_mainMenu.wordProfiles, 3, sX+16, start_y + (0-scroll_start)*line);
            SuperPrintRightAlign(g_mainMenu.controlsDeleteKey, 3, sX+width-16, start_y + (0-scroll_start)*line);
        }

        for(int i = 0; i <= n_profiles; i++)
        {
            if(i+1 >= scroll_start && i+1 < scroll_end)
            {
                if(render)
                {
                    if(i == n_profiles)
                        SuperPrint(g_mainMenu.controlsNewProfile, 3, sX+48, start_y + (i+1-scroll_start)*line);
                    else
                        SuperPrint(profiles[i]->Name, 3, sX+48, start_y + (i+1-scroll_start)*line);
                    if(MenuCursor == i)
                        frmMain.renderTexture(sX + 24, start_y + (i+1-scroll_start)*line, GFX.MCursor[0]);
                }
                if(mouse)
                {
                    int item_width;
                    if(i == n_profiles)
                        item_width = g_mainMenu.controlsNewProfile.size()*18;
                    else
                        item_width = profiles[i]->Name.size()*18;
                    if(MenuMouseX >= sX+48 && MenuMouseX <= sX+48 + item_width
                        && MenuMouseY >= start_y + (i+1-scroll_start)*line && MenuMouseY <= start_y + (i+1-scroll_start)*line + 16)
                    {
                        if(MenuCursor != i)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i;
                        }

                        if(MenuMouseRelease && MenuMouseDown)
                        {
                            if(MenuCursor != n_profiles)
                            {
                                PlaySoundMenu(SFX_Do);
                                s_curProfile = MenuCursor;
                                MenuCursor = 0;
                            }
                            else
                            {
                                PlaySoundMenu(SFX_DropItem);
                                type->AddProfile();
                            }
                            MenuMouseRelease = false;
                        }
                    }
                }
            }
        }

        // "PROFILE", the profiles, "NEW PROFILE", a blank line, and "OPTIONS" come before the options
        int o_base = (1+n_profiles+1+2);

        if(n_options && render && o_base - 1 >= scroll_start && o_base - 1 < scroll_end)
        {
            SuperPrint(g_mainMenu.mainOptions, 3, sX+16, start_y + (o_base - 1 - scroll_start)*line);
        }

        for(int i = 0; i < n_options; i++)
        {
            if(double_line)
            {
                if(render)
                {
                    if(o_base + 2*i >= scroll_start && o_base + 2*i + 1 < scroll_end)
                    {
                        SuperPrint(type->GetOptionName(i), 3, sX+24, start_y + (o_base + 2*i - scroll_start)*line);
                        SuperPrint(type->GetOptionValue(i), 3, sX+48, start_y + (o_base + 2*i + 1 - scroll_start)*line);
                        if(MenuCursor - n_profiles - 1 == i)
                            frmMain.renderTexture(sX + 24, start_y + (o_base + 2*i + 1 - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse)
                {
                    int item_width = strlen(type->GetOptionValue(i))*18;
                    if(MenuMouseX >= sX+48 && MenuMouseX <= sX+48 + item_width
                        && MenuMouseY >= start_y + (o_base + 2*i + 1 - scroll_start)*line && MenuMouseY <= start_y + (o_base + 2*i + 1 - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_profiles + 1)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_profiles + 1;
                        }

                        if(MenuMouseRelease && MenuMouseDown)
                        {
                            PlaySoundMenu(SFX_Do);
                            type->OptionChange(i);
                            MenuMouseRelease = false;
                        }
                    }
                }
            }
            else
            {
                if(render)
                {
                    if(o_base + i >= scroll_start && o_base + i < scroll_end)
                    {
                        SuperPrint(type->GetOptionName(i), 3, sX+48, start_y + (o_base + i - scroll_start)*line);
                        SuperPrintRightAlign(type->GetOptionValue(i), 3, sX+width-32, start_y + (o_base + i - scroll_start)*line);
                        if(MenuCursor - n_profiles - 1 == i)
                            frmMain.renderTexture(sX + 24, start_y + (o_base + i - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse)
                {
                    if(MenuMouseX >= sX+48 && MenuMouseX <= sX+width-32
                        && MenuMouseY >= start_y + (o_base + i - scroll_start)*line && MenuMouseY <= start_y + (o_base + i - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_profiles + 1)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_profiles + 1;
                        }

                        if(MenuMouseRelease && MenuMouseDown)
                        {
                            PlaySoundMenu(SFX_Do);
                            type->OptionChange(i);
                            MenuMouseRelease = false;
                        }
                    }
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
            {
                max_line = max_line * line / 20;
                line = 20;
            }
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
        // work out scrolling here

        int avail_lines = max_line - 3;

        int total_lines;
        if(double_line)
            total_lines = 1 + n_playerButtons*2 + n_options*2;
        else
            total_lines = 1 + n_playerButtons + n_options;
        if(n_options)
            total_lines += 2;

        int cur_line;
        if(double_line)
            cur_line = MenuCursor*2;
        else
            cur_line = MenuCursor;
        if(MenuCursor >= n_playerButtons)
            cur_line += 2;

        // handle scrolling
        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
            scroll_start = cur_line - avail_lines/2;
            scroll_end = scroll_start + avail_lines;
            if(scroll_start < 0)
            {
                scroll_start = 0;
                scroll_end = scroll_start + avail_lines;
            }
            if(scroll_end > total_lines)
            {
                scroll_end = total_lines;
                scroll_start = scroll_end - avail_lines;
            }
        }

        // overall title and "BUTTONS" come before the buttons
        int start_y = sY + 3*line;
        int b_base = 0;

        if(render)
        {
            SuperPrint(g_mainMenu.wordButtons, 3, sX+16, sY + 2*line);
            SuperPrintRightAlign(g_mainMenu.controlsDeleteKey, 3, sX+width-16, sY + 2*line);
        }

        for(int i = 0; i < n_playerButtons; i++)
        {
            if(double_line)
            {
                if(render && b_base+2*i >= scroll_start && b_base+2*i+1 < scroll_end)
                {
                    SuperPrint(Controls::PlayerControls::GetButtonName_UI(i), 3, sX+32+(width-32)/4, start_y+(b_base+2*i - scroll_start)*line);

                    if(MenuCursor == i && !s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+32, start_y+(b_base+2*i+1 - scroll_start)*line);
                    else
                        SuperPrint(profile->NamePrimaryButton(i), 3, sX+32, start_y+(b_base+2*i+1 - scroll_start)*line);
                    if(MenuCursor == i && s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+32+(width-32)/2, start_y+(b_base+2*i+1 - scroll_start)*line);
                    else
                        SuperPrint(profile->NameSecondaryButton(i), 3, sX+32+(width-32)/2, start_y+(b_base+2*i+1 - scroll_start)*line);
                    if(MenuCursor == i)
                    {
                        if(!s_secondaryInput)
                            frmMain.renderTexture(sX+8, start_y+(b_base+2*i+1 - scroll_start)*line, GFX.MCursor[0]);
                        else
                            frmMain.renderTexture(sX+8+(width-32)/2, start_y+(b_base+2*i+1 - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
            }
            else
            {
                if(render && b_base+i >= scroll_start && b_base+i < scroll_end)
                {
                    SuperPrint(Controls::PlayerControls::GetButtonName_UI(i), 3, sX+48, start_y+(b_base+i - scroll_start)*line);
                    if(MenuCursor == i && !s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+width-420, start_y+(b_base+i - scroll_start)*line);
                    else
                        SuperPrint(profile->NamePrimaryButton(i), 3, sX+width-420, start_y+(b_base+i - scroll_start)*line);
                    if(MenuCursor == i && s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+width-210, start_y+(b_base+i - scroll_start)*line);
                    else
                        SuperPrint(profile->NameSecondaryButton(i), 3, sX+width-210, start_y+(b_base+i - scroll_start)*line);
                    if(MenuCursor == i)
                    {
                        if(!s_secondaryInput)
                            frmMain.renderTexture(sX+width-420-24, start_y+(b_base+i - scroll_start)*line, GFX.MCursor[0]);
                        else
                            frmMain.renderTexture(sX+width-210-24, start_y+(b_base+i - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
            }
            if(mouse && !g_pollingInput)
            {
                int primary_width = strlen(profile->NamePrimaryButton(i))*18;
                if(primary_width < 72)
                    primary_width = 72;
                if((double_line && MenuMouseX >= sX+32 && MenuMouseX <= sX+32+primary_width
                    && MenuMouseY >= start_y + (b_base + 2*i+1 - scroll_start)*line && MenuMouseY <= start_y + (b_base + 2*i+1 - scroll_start)*line + 16)
                    || (!double_line && MenuMouseX >= sX+width-420 && MenuMouseX <= sX+width-420+primary_width
                    && MenuMouseY >= start_y + (b_base + i - scroll_start)*line && MenuMouseY <= start_y + (b_base + i - scroll_start)*line + 16))
                {
                    if(MenuCursor != i || s_secondaryInput)
                    {
                        PlaySoundMenu(SFX_Slide);
                        MenuCursor = i;
                        s_secondaryInput = false;
                    }

                    if(MenuMouseRelease && MenuMouseDown)
                    {
                        PlaySoundMenu(SFX_PSwitch);
                        g_pollingInput = true;
                        MenuCursorCanMove = false;
                        MenuMouseRelease = false;
                        return 0;
                    }
                }
                int secondary_width = strlen(profile->NameSecondaryButton(i))*18;
                if(secondary_width < 72)
                    secondary_width = 72;
                if((double_line && MenuMouseX >= sX+32+(width-32)/2 && MenuMouseX <= sX+32+(width-32)/2+secondary_width
                    && MenuMouseY >= start_y + (b_base + 2*i+1 - scroll_start)*line && MenuMouseY <= start_y + (b_base + 2*i+1 - scroll_start)*line + 16)
                    || (!double_line && MenuMouseX >= sX+width-210 && MenuMouseX <= sX+width-210+secondary_width
                    && MenuMouseY >= start_y + (b_base + i - scroll_start)*line && MenuMouseY <= start_y + (b_base + i - scroll_start)*line + 16))
                {
                    if(MenuCursor != i || !s_secondaryInput)
                    {
                        PlaySoundMenu(SFX_Slide);
                        MenuCursor = i;
                        s_secondaryInput = true;
                    }

                    if(MenuMouseRelease && MenuMouseDown)
                    {
                        PlaySoundMenu(SFX_PSwitch);
                        g_pollingInput = true;
                        MenuCursorCanMove = false;
                        MenuMouseRelease = false;
                        return 0;
                    }
                }
            }
        }

        // overall title, player buttons, and "OPTIONS" come before the options
        int o_base;
        if(double_line)
        {
            o_base = (2+1+n_playerButtons*2+2);
        }
        else
        {
            o_base = (2+1+n_playerButtons+2);
        }

        if(n_options && render && o_base - 1 >= scroll_start && o_base - 1 < scroll_end)
        {
            SuperPrint(g_mainMenu.mainOptions, 3, sX+16, start_y + (o_base - 1 - scroll_start)*line);
        }

        for(int i = 0; i < n_options; i++)
        {
            if(double_line)
            {
                if(render)
                {
                    if(o_base + 2*i >= scroll_start && o_base + 2*i < scroll_end)
                        SuperPrint(profile->GetOptionName(i), 3, sX+24, start_y + (o_base + 2*i - scroll_start)*line);
                    if(o_base + 2*i + 1 >= scroll_start && o_base + 2*i + 1 < scroll_end)
                    {
                        SuperPrint(profile->GetOptionValue(i), 3, sX+48, start_y + (o_base + 2*i + 1 - scroll_start)*line);
                        if(MenuCursor - n_playerButtons == i)
                            frmMain.renderTexture(sX + 24, start_y + (o_base + 2*i + 1 - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse)
                {
                    int item_width = strlen(type->GetOptionValue(i))*18;
                    if(MenuMouseX >= sX+48 && MenuMouseX <= sX+48 + item_width
                        && MenuMouseY >= start_y + (o_base + 2*i + 1 - scroll_start)*line && MenuMouseY <= start_y + (o_base + 2*i + 1 - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_playerButtons)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_playerButtons;
                        }

                        if(MenuMouseRelease && MenuMouseDown)
                        {
                            PlaySoundMenu(SFX_Do);
                            type->OptionChange(i);
                            MenuMouseRelease = false;
                        }
                    }
                }
            }
            else
            {
                if(render)
                {
                    if(o_base + i >= scroll_start && o_base + i < scroll_end)
                    {
                        SuperPrint(profile->GetOptionName(i), 3, sX+48, start_y + (o_base + i - scroll_start)*line);
                        SuperPrintRightAlign(profile->GetOptionValue(i), 3, sX+width-32, start_y + (o_base + i - scroll_start)*line);
                        if(MenuCursor - n_playerButtons == i)
                            frmMain.renderTexture(sX + 24, start_y + (o_base + i - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse)
                {
                    if(MenuMouseX >= sX+48 && MenuMouseX <= sX+width-32
                        && MenuMouseY >= start_y + (o_base + i - scroll_start)*line && MenuMouseY <= start_y + (o_base + i - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_playerButtons)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_playerButtons;
                        }

                        if(MenuMouseRelease && MenuMouseDown)
                        {
                            PlaySoundMenu(SFX_Do);
                            type->OptionChange(i);
                            MenuMouseRelease = false;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int menuControls_MouseLogic()
{
    return menuControls_Mouse_Render(true, false);
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
    // todo: integrate MenuMouseBack soon!
    bool menuBackPress = SharedControls.MenuBack || MenuMouseBack;

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

            if(p >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[p])
            {
                s_canRotate[p] = false;
                continue;
            }

            if(!s_canRotate[p])
            {
                if(!c.Left && !c.Right)
                    s_canRotate[p] = true;
                continue;
            }

            if(c.Left)
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
                    Controls::SetInputMethodProfile(p, *p_profile);
                    PlaySoundMenu(SFX_Slide);
                }
                else
                {
                    PlaySoundMenu(SFX_BlockHit);
                }
                s_canRotate[p] = false;
            }
            else if(c.Right)
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
                    Controls::SetInputMethodProfile(p, *p_profile);
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

    return menuControls_MouseLogic();
}

void menuControls_Render()
{
    menuControls_Mouse_Render(false, true);
}