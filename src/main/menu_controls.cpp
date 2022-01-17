#include <fmt_format_ne.h>

#include "../global_constants.h"
#include "../controls.h"
#include "../sound.h"
#include "../globals.h"
#include "../graphics.h"

#include "../gfx.h"
#include "../core/render.h"

#include "screen_textentry.h"
#include "../game_main.h"

#include "speedrunner.h"
#include "menu_main.h"

bool g_pollingInput = false;

static int s_curType = -1;
static int s_curProfile = -1;

static int s_changingProfilePlayer = -1;
static Controls::InputMethodProfile* s_lastProfile = nullptr;
static int s_profileChangeTimer = 0;

static bool s_secondaryInput = false;

static bool s_canDelete = false;

static bool s_deleteProfileSel = false;

static Controls::ControlsClass s_profileTab = Controls::ControlsClass::None;

// only partially refactored from the mouse and standard menu logic functions
//   (which currently duplicate part of their logic)
int menuControls_Do()
{
    if(s_curType == -1)
    {
    }
    else if(s_curProfile == -1)
    {
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        (void)type;
    }
    else if(s_profileTab == Controls::ControlsClass::None)
    {
        Controls::InputMethodType* type = Controls::g_InputMethodTypes[s_curType];
        Controls::InputMethodProfile* profile = type->GetProfiles()[s_curProfile];

        if(MenuCursor == 0) // Activate Profile
        {
            MenuCursorCanMove = false;
            // Figure out who pressed this
            int i;
            for(i = 0; i < maxLocalPlayers; i++)
            {
                if(Player[i+1].Controls.Start || Player[i+1].Controls.Jump)
                    break;
            }
            if(i == maxLocalPlayers || !Controls::g_InputMethods[i])
            {
                PlaySoundMenu(SFX_BlockHit);
                return 0;
            }

            // save the profile before attempting to set it
            s_lastProfile = Controls::g_InputMethods[i]->Profile;

            // attempt to set the profile, if it fails, remove record
            if(s_lastProfile == profile || !Controls::SetInputMethodProfile(i, profile))
            {
                s_lastProfile = nullptr;
                PlaySoundMenu(SFX_BlockHit);
                return 0;
            }

            PlaySoundMenu(SFX_PSwitch);
            s_changingProfilePlayer = i;
            s_profileChangeTimer = 66*3;
            return 0;
        }
        else if(MenuCursor == 1) // Rename Profile
        {
            PlaySoundMenu(SFX_Do);
            MenuCursorCanMove = false;
            profile->Name = TextEntryScreen::Run("Rename profile:", profile->Name);
            MenuCursorCanMove = false;
            return 0;
        }
        else if(MenuCursor == 2) // Delete Profile
        {
            PlaySoundMenu(SFX_PlayerDied2);
            s_deleteProfileSel = true;
            MenuCursor = 0;
            MenuCursorCanMove = false;
            return 0;
        }
        else // submenu
        {
            PlaySoundMenu(SFX_Do);
            if(MenuCursor == 3)
                s_profileTab = Controls::ControlsClass::Player;
            else if(MenuCursor == 4)
                s_profileTab = Controls::ControlsClass::Cursor;
            else if(MenuCursor == 5)
                s_profileTab = Controls::ControlsClass::Editor;
            else if(MenuCursor == 6)
                s_profileTab = Controls::ControlsClass::Hotkey;
            MenuCursor = 0;
            MenuCursorCanMove = false;
            return 0;
        }
    }
    return 0;
}

int menuControls_Mouse_Render(bool mouse, bool render)
{
    if(mouse && !SharedCursor.Move && !render && !SharedCursor.Primary)
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

    // render the background
    if(render)
    {
        XRender::renderRect(sX, sY - (line-18), width, line*max_line + (line-18), 0, 0, 0, .5);
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
                XRender::renderTexture(sX+width/4-24, sY+line*3, GFX.MCursor[0]);
            SuperPrint(g_mainMenu.wordYes, 3, sX+width/4, sY+line*4);
            if(MenuCursor == 1)
                XRender::renderTexture(sX+width/4-24, sY+line*4, GFX.MCursor[0]);
        }

        if(mouse)
        {
            // collision and click handling for "NO"
            int menuLen = g_mainMenu.wordNo.size() * 18;
            if(SharedCursor.X >= sX+width/4 && SharedCursor.X <= sX+width/4 + menuLen
                && SharedCursor.Y >= sY+line*3 && SharedCursor.Y <= sY+line*3 + 16)
            {
                if(MenuCursor != 0)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = 0;
                }

                if(MenuMouseRelease && SharedCursor.Primary)
                {
                    PlaySoundMenu(SFX_Slide);
                    s_deleteProfileSel = false;
                    MenuCursor = 2; // Delete Profile
                    MenuMouseRelease = false;
                }
            }
            // collision and click handling for "YES"
            menuLen = g_mainMenu.wordYes.size() * 18;
            if(SharedCursor.X >= sX+width/4 && SharedCursor.X <= sX+width/4 + menuLen
                && SharedCursor.Y >= sY+line*4 && SharedCursor.Y <= sY+line*4 + 16)
            {
                if(MenuCursor != 1)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = 1;
                }

                if(MenuMouseRelease && SharedCursor.Primary)
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

    // render the players
    if(render && s_changingProfilePlayer != -1)
    {
        // just render the players, and center onscreen, if changing profile
        int top_line = max_line / 2 - 3;
        if(top_line < 0)
            top_line = 0;

        SuperPrintScreenCenter(g_mainMenu.phraseTestControls, 3, sY+(top_line)*line);

        size_t p = s_changingProfilePlayer;
        if(!Controls::g_InputMethods[p])
            return 0;
        SuperPrintScreenCenter(Controls::g_InputMethods[p]->Name, 3, sY+(top_line+1)*line);

        // display the current profile
        SuperPrintScreenCenter(g_mainMenu.wordProfile + ":", 3, sY+(top_line+2)*line);
        // should never be null
        if(Controls::g_InputMethods[p]->Profile != nullptr)
            SuperPrintScreenCenter(Controls::g_InputMethods[p]->Profile->Name, 3, sY+(top_line+3)*line);

        // display the test controls and profile reversion countdown (add more details)
        RenderControls(p+1, ScreenW/2-38, sY+(top_line+4)*line, 76, 30);
        SuperPrintScreenCenter(g_mainMenu.phraseHoldStartToReturn, 3, sY+(top_line+6)*line);
        int n_stars;
        int n_empty;
        if(s_profileChangeTimer < 66*3)
        {
            n_stars = (s_profileChangeTimer + 65) / 66;
            n_empty = 6 - n_stars;
        }
        else
        {
            n_stars = 3 + (s_profileChangeTimer - 66*3)/22;
            n_empty = 6 - n_stars;
        }
        const std::string squeeze = std::string(n_empty, '-') + std::string(n_stars*2, '*') + std::string(n_empty, '-');
        SuperPrintScreenCenter(squeeze, 3, sY+(top_line+7)*line);
    }

    // for both mouse and render
    if(s_changingProfilePlayer != -1)
        return 0;

    // rendering / mouse for the main controls screen
    if(s_curType == -1)
    {
        if(render)
        {
            SuperPrintScreenCenter(g_mainMenu.controlsTitle, 3, sY);
        }

        // render the types at the top of the screen and the currently connected devices at the bottom
        if(render)
        {
            SuperPrint(g_mainMenu.controlsDeviceTypes, 3, sX+16, sY+2*line);
        }

        int scroll_start = 0;
        int scroll_end = n_types;

        if(max_line - 9 < n_types)
        {
            int scroll_n = max_line - 10;
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

        // render the scroll indicators
        if(render)
        {
            if(scroll_start > 0)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[1].w / 2, sY + 3*line - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < n_types)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[2].w / 2, sY + (3+scroll_end-scroll_start)*line - line + 18, GFX.MCursor[2]);
        }

        // render the menu items
        for(int i = 0; i < scroll_end - scroll_start; i++)
        {
            if(render)
            {
                bool in_use = false;

                for(Controls::InputMethod* method : Controls::g_InputMethods)
                {
                    if(!method)
                        continue;
                    if(method->Type == Controls::g_InputMethodTypes[scroll_start + i])
                    {
                        in_use = true;
                        break;
                    }
                }

                if(in_use)
                    SuperPrint(Controls::g_InputMethodTypes[scroll_start + i]->Name + " " + g_mainMenu.controlsInUse, 3, sX+48, sY+(3+i)*line);
                else
                    SuperPrint(Controls::g_InputMethodTypes[scroll_start + i]->Name, 3, sX+48, sY+(3+i)*line);
                if(MenuCursor == scroll_start + i)
                    XRender::renderTexture(sX + 24, sY+(3+i)*line, GFX.MCursor[0]);
            }
            int item_width = Controls::g_InputMethodTypes[scroll_start + i]->Name.size()*18;
            if(mouse && SharedCursor.X >= sX+48 && SharedCursor.X <= sX+48 + item_width
                && SharedCursor.Y >= sY+(3+i)*line && SharedCursor.Y <= sY+(3+i)*line + 16)
            {
                if(MenuCursor != scroll_start + i)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = scroll_start + i;
                }

                if(MenuMouseRelease && SharedCursor.Primary)
                {
                    PlaySoundMenu(SFX_Do);
                    s_curType = MenuCursor;
                    MenuCursor = 0;
                    MenuMouseRelease = false;
                }
            }
        }

        // render the players
        if(render && s_curType == -1)
        {
            SuperPrint(g_mainMenu.controlsConnected, 3, sX+16, sY+(max_line-5)*line);
            for(size_t p = 0; p < Controls::g_InputMethods.size(); p++)
            {
                if(!Controls::g_InputMethods[p])
                    continue;
                int cX = sX + 100 + 200*p;
                SuperPrintCenter(Controls::g_InputMethods[p]->Name, 3, cX, sY+(max_line-4)*line);

                // display the current profile
                SuperPrintCenter(g_mainMenu.wordProfile + ":", 3, cX, sY+(max_line-3)*line);
                // should never be null
                if(Controls::g_InputMethods[p]->Profile != nullptr)
                    SuperPrintCenter(Controls::g_InputMethods[p]->Profile->Name, 3, cX, sY+(max_line-2)*line);
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
        const int n_options = type->GetOptionCount();
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
        // the scrolling is determined here

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
            avail_lines --; // for scroll indicator

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

        // overall title and "PROFILES" come before the profiles
        int start_y = sY + 2*line;

        // render the scroll indicators
        if(render)
        {
            if(scroll_start > 0)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[1].w / 2, start_y - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < total_lines)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[2].w / 2, start_y + (avail_lines)*line - line + 18, GFX.MCursor[2]);
        }

        if(render && 0 >= scroll_start && 0 < scroll_end)
            SuperPrint(g_mainMenu.wordProfiles, 3, sX+16, start_y + (0-scroll_start)*line);

        for(int i = 0; i <= n_profiles; i++)
        {
            if(i+1 >= scroll_start && i+1 < scroll_end)
            {
                if(render)
                {
                    bool in_use = false;

                    for(Controls::InputMethod* method : Controls::g_InputMethods)
                    {
                        if(!method || i == n_profiles)
                            continue;
                        if(method->Profile == profiles[i])
                        {
                            in_use = true;
                            break;
                        }
                    }

                    if(i == n_profiles)
                        SuperPrint(g_mainMenu.controlsNewProfile, 3, sX+48, start_y + (i+1-scroll_start)*line);
                    else if(in_use)
                        SuperPrint(profiles[i]->Name + " " + g_mainMenu.controlsInUse, 3, sX+48, start_y + (i+1-scroll_start)*line);
                    else
                        SuperPrint(profiles[i]->Name, 3, sX+48, start_y + (i+1-scroll_start)*line);
                    if(MenuCursor == i)
                        XRender::renderTexture(sX + 24, start_y + (i+1-scroll_start)*line, GFX.MCursor[0]);
                }
                if(mouse)
                {
                    int item_width;
                    if(i == n_profiles)
                        item_width = g_mainMenu.controlsNewProfile.size()*18;
                    else
                        item_width = profiles[i]->Name.size()*18;
                    if(SharedCursor.X >= sX+48 && SharedCursor.X <= sX+48 + item_width
                        && SharedCursor.Y >= start_y + (i+1-scroll_start)*line && SharedCursor.Y <= start_y + (i+1-scroll_start)*line + 16)
                    {
                        if(MenuCursor != i)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i;
                        }

                        if(MenuMouseRelease && SharedCursor.Primary)
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
            const char* name = type->GetOptionName(i);
            const char* value = type->GetOptionValue(i);
            if(double_line)
            {
                if(render)
                {
                    if(o_base + 2*i >= scroll_start && o_base + 2*i + 1 < scroll_end)
                    {
                        if(name)
                            SuperPrint(name, 3, sX+24, start_y + (o_base + 2*i - scroll_start)*line);
                        if(value)
                            SuperPrint(value, 3, sX+48, start_y + (o_base + 2*i + 1 - scroll_start)*line);
                        if(MenuCursor - n_profiles - 1 == i)
                            XRender::renderTexture(sX + 24, start_y + (o_base + 2*i + 1 - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse && value)
                {
                    int item_width = strlen(value)*18;
                    if(SharedCursor.X >= sX+48 && SharedCursor.X <= sX+48 + item_width
                        && SharedCursor.Y >= start_y + (o_base + 2*i + 1 - scroll_start)*line && SharedCursor.Y <= start_y + (o_base + 2*i + 1 - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_profiles + 1)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_profiles + 1;
                        }

                        if(MenuMouseRelease && SharedCursor.Primary)
                        {
                            if(type->OptionChange(i))
                                PlaySoundMenu(SFX_Do);
                            else
                                PlaySoundMenu(SFX_BlockHit);
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
                        if(name)
                            SuperPrint(name, 3, sX+48, start_y + (o_base + i - scroll_start)*line);
                        if(value)
                            SuperPrintRightAlign(value, 3, sX+width-32, start_y + (o_base + i - scroll_start)*line);
                        if(MenuCursor - n_profiles - 1 == i)
                            XRender::renderTexture(sX + 24, start_y + (o_base + i - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse)
                {
                    if(SharedCursor.X >= sX+48 && SharedCursor.X <= sX+width-32
                        && SharedCursor.Y >= start_y + (o_base + i - scroll_start)*line && SharedCursor.Y <= start_y + (o_base + i - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_profiles + 1)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_profiles + 1;
                        }

                        if(MenuMouseRelease && SharedCursor.Primary)
                        {
                            if(type->OptionChange(i))
                                PlaySoundMenu(SFX_Do);
                            else
                                PlaySoundMenu(SFX_BlockHit);
                            MenuMouseRelease = false;
                        }
                    }
                }
            }
        }

    }
    // rendering / mouse for the input method profile main screen
    else if(s_profileTab == Controls::ControlsClass::None)
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
        const int n_options = profile->GetOptionCount();
        const int n_stock = 7;

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

        // first come the stock options, then the profile options.
        // the scrolling is determined here

        int avail_lines = max_line - 2;

        int total_lines;
        if(double_line)
            total_lines = n_stock + 1 + n_options*2;
        else
            total_lines = n_stock + 1 + n_options;
        if(n_options)
            total_lines += 2;

        int cur_line;
        if(double_line && MenuCursor > n_stock)
            cur_line = n_stock + (MenuCursor-n_stock)*2;
        else
            cur_line = MenuCursor;
        if(MenuCursor >= 3)
            cur_line += 1;
        if(MenuCursor >= n_stock)
            cur_line += 1;

        // handle scrolling
        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
            avail_lines --; // for scroll indicator

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

        // overall title comes before the stock options
        int start_y = sY + 1*line;

        // render the scroll indicators
        if(render)
        {
            if(scroll_start > 0)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[1].w / 2, start_y - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < total_lines)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[2].w / 2, start_y + (avail_lines)*line - line + 18, GFX.MCursor[2]);
        }

        for(int i = 0; i < n_stock; i++)
        {
            const char* name;
            if(i == 0) // Activate profile
                name = "Activate profile";
            else if(i == 1) // Rename profile
                name = "Rename profile";
            else if(i == 2) // Delete profile
                name = "Delete profile";
            else if(i == 3) // Player
                name = "Player controls";
            else if(i == 4) // Cursor
                name = "Cursor controls";
            else if(i == 5) // Editor
                name = "Editor controls";
            else if(i == 6) // Hotkeys
                name = "Hotkeys";
            else
                name = "";

            int s; // shift
            if(i >= 3)
                s = 2;
            else
                s = 1;

            if(render && i == 3 && i+s-2 >= scroll_start && i+s-2 < scroll_end)
                SuperPrint("Controls", 3, sX+16, start_y+(i+s - 1 - scroll_start)*line);

            if(i+s-1 < scroll_start || i+s-1 >= scroll_end)
                continue;

            if(render)
            {
                SuperPrint(name, 3, sX+48, start_y+(i+s - scroll_start)*line);
                if(MenuCursor == i)
                    XRender::renderTexture(sX + 24, start_y + (i+s - scroll_start)*line, GFX.MCursor[0]);
            }
            if(mouse)
            {
                int item_width = SDL_strlen(name) * 18;
                if(SharedCursor.X >= sX+48 && SharedCursor.X <= sX+48 + item_width
                    && SharedCursor.Y >= start_y + (i+s-scroll_start)*line && SharedCursor.Y <= start_y + (i+s-scroll_start)*line + 16)
                {
                    if(MenuCursor != i)
                    {
                        PlaySoundMenu(SFX_Slide);
                        MenuCursor = i;
                    }

                    if(MenuMouseRelease && SharedCursor.Primary)
                    {
                        MenuMouseRelease = false;

                        menuControls_Do();
                    }
                }
            }
        }

        // overall title, stock options, and "OPTIONS" come before the options
        int o_base = n_stock+3;

        if(n_options && render && o_base - 1 >= scroll_start && o_base - 1 < scroll_end)
        {
            SuperPrint(g_mainMenu.mainOptions, 3, sX+16, start_y + (o_base - 1 - scroll_start)*line);
        }

        for(int i = 0; i < n_options; i++)
        {
            const char* name = profile->GetOptionName(i);
            const char* value = profile->GetOptionValue(i);
            if(double_line)
            {
                if(render)
                {
                    if(o_base + 2*i >= scroll_start && o_base + 2*i < scroll_end && name)
                        SuperPrint(name, 3, sX+24, start_y + (o_base + 2*i - scroll_start)*line);
                    if(o_base + 2*i + 1 >= scroll_start && o_base + 2*i + 1 < scroll_end)
                    {
                        if(value)
                            SuperPrint(value, 3, sX+48, start_y + (o_base + 2*i + 1 - scroll_start)*line);
                        if(MenuCursor - n_stock == i)
                            XRender::renderTexture(sX + 24, start_y + (o_base + 2*i + 1 - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse && value)
                {
                    int item_width = strlen(value)*18;
                    if(SharedCursor.X >= sX+48 && SharedCursor.X <= sX+48 + item_width
                        && SharedCursor.Y >= start_y + (o_base + 2*i + 1 - scroll_start)*line && SharedCursor.Y <= start_y + (o_base + 2*i + 1 - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_stock)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_stock;
                        }

                        if(MenuMouseRelease && SharedCursor.Primary)
                        {
                            if(type->OptionChange(i))
                                PlaySoundMenu(SFX_Do);
                            else
                                PlaySoundMenu(SFX_BlockHit);
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
                        if(name)
                            SuperPrint(name, 3, sX+48, start_y + (o_base + i - scroll_start)*line);
                        if(value)
                            SuperPrintRightAlign(value, 3, sX+width-32, start_y + (o_base + i - scroll_start)*line);
                        if(MenuCursor - n_stock == i)
                            XRender::renderTexture(sX + 24, start_y + (o_base + i - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
                if(mouse)
                {
                    if(SharedCursor.X >= sX+48 && SharedCursor.X <= sX+width-32
                        && SharedCursor.Y >= start_y + (o_base + i - scroll_start)*line && SharedCursor.Y <= start_y + (o_base + i - scroll_start)*line + 16)
                    {
                        if(MenuCursor != i + n_stock)
                        {
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i + n_stock;
                        }

                        if(MenuMouseRelease && SharedCursor.Primary)
                        {
                            if(profile->OptionChange(i))
                                PlaySoundMenu(SFX_Do);
                            else
                                PlaySoundMenu(SFX_BlockHit);
                            MenuMouseRelease = false;
                        }
                    }
                }
            }
        }
    }
    // rendering / mouse for the input method profile sub screens
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

        int n_buttons;
        if(s_profileTab == Controls::ControlsClass::Player)
            n_buttons = Controls::PlayerControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Cursor)
            n_buttons = Controls::CursorControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Editor)
            n_buttons = Controls::EditorControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Hotkey)
            n_buttons = Controls::Hotkeys::n_buttons;
        else
        {
            // shouldn't happen
            s_profileTab = Controls::ControlsClass::None;
            return 0;
        }

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

        // the buttons are the only thing now.
        // work out scrolling here

        int avail_lines = max_line - 3;

        int total_lines;
        if(double_line)
            total_lines = 1 + n_buttons*2;
        else
            total_lines = 1 + n_buttons;

        int cur_line;
        if(double_line)
            cur_line = MenuCursor*2;
        else
            cur_line = MenuCursor;

        // handle scrolling
        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
            avail_lines --; // for scroll indicator

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
        int start_y = sY + 2*line;
        int b_base = 1;

        // render the scroll indicators
        if(render)
        {
            if(scroll_start > 0)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[1].w / 2, start_y - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < total_lines)
                XRender::renderTexture(sX + width / 2 - GFX.MCursor[2].w / 2, start_y + (avail_lines)*line - line + 18, GFX.MCursor[2]);
        }

        // render the word "BUTTONS"
        if(render && scroll_start == 0)
        {
            SuperPrint(g_mainMenu.wordButtons, 3, sX+16, start_y);
            SuperPrintRightAlign(g_mainMenu.controlsDeleteKey, 3, sX+width-16, start_y);
        }

        for(int i = 0; i < n_buttons; i++)
        {
            if(double_line)
            {
                if(render && b_base+2*i >= scroll_start && b_base+2*i+1 < scroll_end)
                {
                    const char* name = nullptr;
                    if(s_profileTab == Controls::ControlsClass::Player)
                        name = Controls::PlayerControls::GetButtonName_UI(i);
                    else if(s_profileTab == Controls::ControlsClass::Cursor)
                        name = Controls::CursorControls::GetButtonName_UI(i);
                    else if(s_profileTab == Controls::ControlsClass::Editor)
                        name = Controls::EditorControls::GetButtonName_UI(i);
                    else if(s_profileTab == Controls::ControlsClass::Hotkey)
                        name = Controls::Hotkeys::GetButtonName_UI(i);

                    SuperPrint(name, 3, sX+32+(width-32)/4, start_y+(b_base+2*i - scroll_start)*line);

                    if(MenuCursor == i && !s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+32, start_y+(b_base+2*i+1 - scroll_start)*line);
                    else
                        SuperPrint(profile->NamePrimaryButton(s_profileTab, i), 3, sX+32, start_y+(b_base+2*i+1 - scroll_start)*line);
                    if(MenuCursor == i && s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+32+(width-32)/2, start_y+(b_base+2*i+1 - scroll_start)*line);
                    else
                        SuperPrint(profile->NameSecondaryButton(s_profileTab, i), 3, sX+32+(width-32)/2, start_y+(b_base+2*i+1 - scroll_start)*line);
                    if(MenuCursor == i)
                    {
                        if(!s_secondaryInput)
                            XRender::renderTexture(sX+8, start_y+(b_base+2*i+1 - scroll_start)*line, GFX.MCursor[0]);
                        else
                            XRender::renderTexture(sX+8+(width-32)/2, start_y+(b_base+2*i+1 - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
            }
            else
            {
                if(render && b_base+i >= scroll_start && b_base+i < scroll_end)
                {
                    const char* name = nullptr;
                    if(s_profileTab == Controls::ControlsClass::Player)
                        name = Controls::PlayerControls::GetButtonName_UI(i);
                    else if(s_profileTab == Controls::ControlsClass::Cursor)
                        name = Controls::CursorControls::GetButtonName_UI(i);
                    else if(s_profileTab == Controls::ControlsClass::Editor)
                        name = Controls::EditorControls::GetButtonName_UI(i);
                    else if(s_profileTab == Controls::ControlsClass::Hotkey)
                        name = Controls::Hotkeys::GetButtonName_UI(i);

                    SuperPrint(name, 3, sX+48, start_y+(b_base+i - scroll_start)*line);
                    if(MenuCursor == i && !s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+width-420, start_y+(b_base+i - scroll_start)*line);
                    else
                        SuperPrint(profile->NamePrimaryButton(s_profileTab, i), 3, sX+width-420, start_y+(b_base+i - scroll_start)*line);
                    if(MenuCursor == i && s_secondaryInput && g_pollingInput)
                        SuperPrint("...", 3, sX+width-210, start_y+(b_base+i - scroll_start)*line);
                    else
                        SuperPrint(profile->NameSecondaryButton(s_profileTab, i), 3, sX+width-210, start_y+(b_base+i - scroll_start)*line);
                    if(MenuCursor == i)
                    {
                        if(!s_secondaryInput)
                            XRender::renderTexture(sX+width-420-24, start_y+(b_base+i - scroll_start)*line, GFX.MCursor[0]);
                        else
                            XRender::renderTexture(sX+width-210-24, start_y+(b_base+i - scroll_start)*line, GFX.MCursor[0]);
                    }
                }
            }
            if(mouse && !g_pollingInput)
            {
                int primary_width = strlen(profile->NamePrimaryButton(s_profileTab, i))*18;
                if(primary_width < 72)
                    primary_width = 72;
                if((double_line && SharedCursor.X >= sX+32 && SharedCursor.X <= sX+32+primary_width
                    && SharedCursor.Y >= start_y + (b_base + 2*i+1 - scroll_start)*line && SharedCursor.Y <= start_y + (b_base + 2*i+1 - scroll_start)*line + 16)
                    || (!double_line && SharedCursor.X >= sX+width-420 && SharedCursor.X <= sX+width-420+primary_width
                    && SharedCursor.Y >= start_y + (b_base + i - scroll_start)*line && SharedCursor.Y <= start_y + (b_base + i - scroll_start)*line + 16))
                {
                    if(MenuCursor != i || s_secondaryInput)
                    {
                        PlaySoundMenu(SFX_Slide);
                        MenuCursor = i;
                        s_secondaryInput = false;
                    }

                    if(MenuMouseRelease && SharedCursor.Primary)
                    {
                        PlaySoundMenu(SFX_PSwitch);
                        g_pollingInput = true;
                        MenuCursorCanMove = false;
                        MenuMouseRelease = false;
                        return 0;
                    }
                }
                int secondary_width = strlen(profile->NameSecondaryButton(s_profileTab, i))*18;
                if(secondary_width < 72)
                    secondary_width = 72;
                if((double_line && SharedCursor.X >= sX+32+(width-32)/2 && SharedCursor.X <= sX+32+(width-32)/2+secondary_width
                    && SharedCursor.Y >= start_y + (b_base + 2*i+1 - scroll_start)*line && SharedCursor.Y <= start_y + (b_base + 2*i+1 - scroll_start)*line + 16)
                    || (!double_line && SharedCursor.X >= sX+width-210 && SharedCursor.X <= sX+width-210+secondary_width
                    && SharedCursor.Y >= start_y + (b_base + i - scroll_start)*line && SharedCursor.Y <= start_y + (b_base + i - scroll_start)*line + 16))
                {
                    if(MenuCursor != i || !s_secondaryInput)
                    {
                        PlaySoundMenu(SFX_Slide);
                        MenuCursor = i;
                        s_secondaryInput = true;
                    }

                    if(MenuMouseRelease && SharedCursor.Primary)
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
    }

    return 0;
}

int menuControls_MouseLogic()
{
    return menuControls_Mouse_Render(true, false);
}

int menuControls_Logic()
{
    // allow input methods to join so long as not currently switching a method
    if(s_changingProfilePlayer == -1)
        Controls::PollInputMethod();

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

        int n_buttons;
        if(s_profileTab == Controls::ControlsClass::Player)
            n_buttons = Controls::PlayerControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Cursor)
            n_buttons = Controls::CursorControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Editor)
            n_buttons = Controls::EditorControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Hotkey)
            n_buttons = Controls::Hotkeys::n_buttons;
        else
            n_buttons = 0;

        if(MenuCursor < 0 || MenuCursor >= n_buttons)
        {
            g_pollingInput = false;
            return 0;
        }

        if((!s_secondaryInput && profile->PollPrimaryButton(s_profileTab, MenuCursor))
            || (s_secondaryInput && profile->PollSecondaryButton(s_profileTab, MenuCursor)))
        {
            g_pollingInput = false;
            MenuCursorCanMove = false;
            s_canDelete = false;
            Controls::g_disallowHotkeys = true;
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
            s_canDelete = false;
            PlaySoundMenu(SFX_Slide);
            return 0;
        }

        return 0;
    }

    // stop the profile change process if the player is no longer there
    if(s_changingProfilePlayer != -1 &&
        (s_changingProfilePlayer >= (int)Controls::g_InputMethods.size()
            || !Controls::g_InputMethods[s_changingProfilePlayer]))
    {
        s_changingProfilePlayer = -1;
        s_lastProfile = nullptr;
    }

    // do the countdown of the profile change process
    if(s_changingProfilePlayer != -1)
    {
        const Controls_t& c = Player[s_changingProfilePlayer+1].Controls;

        // count down if no buttons pressed, 3 seconds if any buttons pressed
        if(!c.Run && !c.Jump && !c.AltRun && !c.AltJump && !c.Start && !c.Drop && !c.Down && !c.Up && !c.Left && !c.Right)
        {
            s_profileChangeTimer --;
        }
        else if(s_profileChangeTimer < 66*3)
            s_profileChangeTimer = 66*3;
        // if start button pressed, count up!
        if(c.Start)
            s_profileChangeTimer ++;

        if(s_profileChangeTimer < 0)
        {
            PlaySoundMenu(SFX_PSwitch);
            Controls::SetInputMethodProfile(Controls::g_InputMethods[s_changingProfilePlayer], s_lastProfile);
            s_lastProfile = nullptr;
            s_changingProfilePlayer = -1;
        }
        if(s_profileChangeTimer > 66*4)
        {
            PlaySoundMenu(SFX_Yoshi);
            s_lastProfile = nullptr;
            s_changingProfilePlayer = -1;
        }

        MenuCursorCanMove = false;
        return 0;
    }

    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;
    bool leftPressed = SharedControls.MenuLeft;
    bool rightPressed = SharedControls.MenuRight;

    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack || (SharedCursor.Secondary && MenuMouseRelease);

    bool delPressed = false;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(s_changingProfilePlayer == i)
            continue;

        Controls_t &c = Player[i+1].Controls;

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;

        delPressed |= c.AltJump;
    }

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

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
            MenuCursor = 2; // Delete Profile
            MenuCursorCanMove = false;
        }
        else if(menuDoPress && MenuCursor == 1)
        {
            if(profile != s_lastProfile && type->DeleteProfile(profile, Controls::g_InputMethods))
            {
                PlaySoundMenu(SFX_BowserKilled);
                s_deleteProfileSel = false;
                MenuCursor = s_curProfile - 1;
                if(MenuCursor < 0)
                    MenuCursor = 0;
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
        const int n_options = type->GetOptionCount();
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
            s_profileTab = Controls::ControlsClass::None;
            s_secondaryInput = false;
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
                if(type->OptionChange(MenuCursor - n_profiles - 1))
                    PlaySoundMenu(SFX_Slide);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
            else if(leftPressed)
            {
                if(type->OptionRotateLeft(MenuCursor - n_profiles - 1))
                    PlaySoundMenu(SFX_Slide);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
            else if(rightPressed)
            {
                if(type->OptionRotateRight(MenuCursor - n_profiles - 1))
                    PlaySoundMenu(SFX_Slide);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
        }
    }
    // logic for the input method profile screens
    else if(s_profileTab == Controls::ControlsClass::None)
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
        const int n_stock = 7;
        const int n_options = profile->GetOptionCount();

        // first come the stock options, then the profile options.

        // keep things in range
        while(MenuCursor < 0)
            MenuCursor += n_options + n_stock;
        while(MenuCursor >= n_options + n_stock)
            MenuCursor -= n_options + n_stock;

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

        // stock options
        if(MenuCursor >= 0 && MenuCursor < n_stock && menuDoPress)
        {
            return menuControls_Do();
        }

        // options logic
        if(MenuCursor >= n_stock && MenuCursor < n_stock + n_options)
        {
            if(menuDoPress)
            {
                if(profile->OptionChange(MenuCursor - n_stock))
                    PlaySoundMenu(SFX_Slide);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
            else if(leftPressed)
            {
                if(profile->OptionRotateLeft(MenuCursor - n_stock))
                    PlaySoundMenu(SFX_Slide);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
            else if(rightPressed)
            {
                if(profile->OptionRotateRight(MenuCursor - n_stock))
                    PlaySoundMenu(SFX_Slide);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
        }
    }
    // logic for the controls submenus of the input method profile screens
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

        int n_buttons;
        if(s_profileTab == Controls::ControlsClass::Player)
            n_buttons = Controls::PlayerControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Cursor)
            n_buttons = Controls::CursorControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Editor)
            n_buttons = Controls::EditorControls::n_buttons;
        else if(s_profileTab == Controls::ControlsClass::Hotkey)
            n_buttons = Controls::Hotkeys::n_buttons;
        else
        {
            // shouldn't happen
            SDL_assert_release(false);
            s_profileTab = Controls::ControlsClass::None;
            return 0;
        }

        // keep things in range
        // why didn't I write MenuCursor %= n_buttons?
        while(MenuCursor < 0)
            MenuCursor += n_buttons;
        while(MenuCursor >= n_buttons)
            MenuCursor -= n_buttons;

        if(!MenuCursorCanMove)
            return 0;

        // backward navigation
        if(menuBackPress)
        {
            PlaySoundMenu(SFX_Slide);
            if(s_profileTab == Controls::ControlsClass::Player)
                MenuCursor = 3;
            else if(s_profileTab == Controls::ControlsClass::Cursor)
                MenuCursor = 4;
            else if(s_profileTab == Controls::ControlsClass::Editor)
                MenuCursor = 5;
            else if(s_profileTab == Controls::ControlsClass::Hotkey)
                MenuCursor = 6;
            s_profileTab = Controls::ControlsClass::None;
            s_secondaryInput = false;
            MenuCursorCanMove = false;
            return 0;
        }

        // key logic
        if(MenuCursor >= 0 && MenuCursor < n_buttons)
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
                if(profile->DeleteSecondaryButton(s_profileTab, MenuCursor))
                    PlaySoundMenu(SFX_PlayerDied2);
                else
                    PlaySoundMenu(SFX_BlockHit);
                MenuCursorCanMove = false;
            }
            else if(delPressed)
            {
                if(profile->DeletePrimaryButton(s_profileTab, MenuCursor))
                    PlaySoundMenu(SFX_PlayerDied2);
                else
                    PlaySoundMenu(SFX_BlockHit);
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