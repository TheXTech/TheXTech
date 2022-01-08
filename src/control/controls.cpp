/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../config.h"
#include "../controls.h"
#include "../main/record.h"
#include "../main/speedrunner.h"

#include "core/render.h"
#include "core/window.h"

// for hotkeys screen
#include "main/menu_controls.h"
#include "../game_main.h"
#include "main/screen_textentry.h"
#include "main/screen_quickreconnect.h"
#include "main/cheat_code.h"
#include "../graphics.h"
#include "../frame_timer.h"

// Control methods

#include "keyboard.h"
#include "joystick.h"
#include "duplicate.h"
#include "touchscreen.h"

#include <Logger/logger.h>

namespace Controls
{

std::vector<InputMethod*> g_InputMethods;
std::vector<InputMethodType*> g_InputMethodTypes;
bool g_renderTouchscreen = false;
static bool s_enterCheatScreen = false;
bool g_disallowHotkeys = false;

void Hotkeys::Activate(size_t i)
{
    if(g_disallowHotkeys || g_pollingInput)
        return;
    g_disallowHotkeys = true;
    switch(i)
    {
#ifndef RENDER_FULLSCREEN_ALWAYS
        case Buttons::Fullscreen:
            ChangeScreen();
            return;
#endif
#ifdef USE_SCREENSHOTS_AND_RECS
        case Buttons::Screenshot:
            TakeScreen = true;
            return;
        case Buttons::RecordGif:
            XRender::toggleGifRecorder();
            return;
#endif
        case Buttons::DebugInfo:
            g_stats.enabled = !g_stats.enabled;
            return;
        case Buttons::EnterCheats:
            s_enterCheatScreen = true;
            return;
        case Buttons::ToggleHUD:
            ShowOnScreenMeta = !ShowOnScreenMeta;
            return;
        default:
            return;
    }
}

/*====================================================*\
|| implementation for InputMethod                     ||
\*====================================================*/

InputMethod::~InputMethod() {}

// Used for battery, latency, etc
// If the char* member is dynamically generated, you MUST use an instance-owned buffer
// This will NOT be freed
// returning a null pointer is allowed
StatusInfo InputMethod::GetStatus()
{
    return StatusInfo();
}

// Optional function allowing developer to consume an SDL event (on SDL clients)
//     usually used for hotkeys or for connect/disconnect events.
// Called (1) in order of InputMethodTypes, then (2) in order of InputMethods
// Returns true if event is consumed, false if other InputMethodTypes and InputMethods
//     should receive it.
bool InputMethod::ConsumeEvent(const SDL_Event* ev)
{
    (void)ev;
    return false;
}


/*====================================================*\
|| implementation for InputMethodProfile              ||
\*====================================================*/

InputMethodProfile::~InputMethodProfile() {}

// How many per-type special options are there?
size_t InputMethodProfile::GetSpecialOptionCount()
{
    return 0;
}

// get a nullable char* describing the option
const char* InputMethodProfile::GetOptionName(size_t i)
{
    (void)i;
    return nullptr;
}
// get a nullable char* describing the current option value
// must be allocated in static or instance memory
const char* InputMethodProfile::GetOptionValue(size_t i)
{
    (void)i;
    return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodProfile::OptionChange(size_t i)
{
    (void)i;
    return false;
}
// called when left is pressed
bool InputMethodProfile::OptionRotateLeft(size_t i)
{
    (void)i;
    return false;
}
// called when right is pressed
bool InputMethodProfile::OptionRotateRight(size_t i)
{
    (void)i;
    return false;
}

/*====================================================*\
|| implementation for InputMethodType                 ||
\*====================================================*/

InputMethodType::~InputMethodType()
{
    for(InputMethodProfile* p : this->m_profiles)
    {
        delete p;
    }
    this->m_profiles.clear();
}

std::vector<InputMethodProfile*> InputMethodType::GetProfiles()
{
    return this->m_profiles;
}

InputMethodProfile* InputMethodType::AddProfile()
{
    InputMethodProfile* profile = this->AllocateProfile();
    if(!profile)
        return nullptr;
    profile->Type = this;
    this->m_profiles.push_back((InputMethodProfile*) profile);
    profile->Name = this->Name + " " + std::to_string(this->m_profiles.size());
    return profile;
}

// be extremely careful never to delete a profile that is in use
bool InputMethodType::DeleteProfile(InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods)
{
    if(!profile)
        return false;

    // delete from m_profiles
    std::vector<InputMethodProfile*>::iterator loc = std::find(this->m_profiles.begin(), this->m_profiles.end(), profile);
    if(loc == this->m_profiles.end())
        return false;

    int player_no = 0;
    for(InputMethod* method : active_methods)
    {
        if(!method)
            continue;
        if(method->Profile == profile)
        {
            // try to assign an acceptable backup profile to all relevant methods
            for(InputMethodProfile* backup : this->m_profiles)
            {
                if(backup != profile && this->SetProfile(method, player_no, backup, active_methods))
                    break;
            }
            // if we couldn't find one, deleting the profile would leave the game inconsistent
            if(method->Profile == profile)
            {
                return false;
            }
        }
        player_no ++;
    }

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(this->m_defaultProfiles[i] == profile)
        {
            // m_defaultProfiles is nullable, so this is okay
            this->m_defaultProfiles[i] = nullptr;
        }
    }

    if(!this->DeleteProfile_Custom(profile, active_methods))
        return false;

    this->m_profiles.erase(loc);

    // deallocate
    delete profile;

    return true;
}

bool InputMethodType::ClearProfiles(const std::vector<InputMethod*>& active_methods)
{
    // clear existing profiles safely
    size_t i = 0;
    while(i < this->m_profiles.size())
    {
        // only increment the index when deletion fails
        if(!this->DeleteProfile(this->m_profiles[i], active_methods))
        {
            i++;
        }
    }
    // succeeded if all were cleared successfully
    return (i == 0);
}

bool InputMethodType::SetProfile(InputMethod* method, int player_no, InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods)
{
    if(player_no < 0 || player_no >= maxLocalPlayers || !profile)
    {
        return false;
    }

    if(!this->TestProfileType(profile))
        return false;

    if(!this->SetProfile_Custom(method, player_no, profile, active_methods))
    {
        return false;
    }

    this->SetDefaultProfile(player_no, profile);
    method->Profile = profile;

    return true;
}

void InputMethodType::SetDefaultProfile(int player_no, InputMethodProfile* profile)
{
    if(player_no < 0 || player_no >= maxLocalPlayers || !profile)
        return;
    this->m_defaultProfiles[player_no] = profile;
}

InputMethodProfile* InputMethodType::GetDefaultProfile(int player_no)
{
    if(player_no < 0 || player_no >= maxLocalPlayers)
        return nullptr;
    // still possibly a null pointer
    return this->m_defaultProfiles[player_no];
}

void InputMethodType::SaveConfig(IniProcessing* ctl)
{
    ctl->beginGroup(this->Name);
    this->SaveConfig_Custom(ctl);
    ctl->setValue("n-profiles", this->m_profiles.size());
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        InputMethodProfile* default_profile = this->m_defaultProfiles[i];
        std::string default_profile_key = "default-profile-" + std::to_string(i+1);
        if(default_profile == nullptr)
        {
            ctl->setValue(default_profile_key.c_str(), -1);
            continue;
        }
        std::vector<InputMethodProfile*>::iterator loc = std::find(this->m_profiles.begin(), this->m_profiles.end(), default_profile);
        size_t index = loc - this->m_profiles.begin();
        if(index == this->m_profiles.size())
            ctl->setValue(default_profile_key.c_str(), -1);
        else
            ctl->setValue(default_profile_key.c_str(), index);
    }
    ctl->endGroup();
    for(size_t i = 0; i < this->m_profiles.size(); i++)
    {
        ctl->beginGroup(this->Name + "-" + std::to_string(i+1));
        ctl->setValue("name", this->m_profiles[i]->Name);
        this->m_profiles[i]->SaveConfig(ctl);
        ctl->endGroup();
    }
}

void InputMethodType::LoadConfig(IniProcessing* ctl)
{
    int n_profiles;
    int n_existing = this->m_profiles.size(); // should usually be zero

    ctl->beginGroup(this->Name);
    ctl->read("n-profiles", n_profiles, 0);
    ctl->endGroup();

    for(int i = 0; i < n_profiles; i++)
    {
        InputMethodProfile* new_profile = this->AddProfile();
        if(new_profile)
        {
            // load legacy profile
            if(!this->LegacyName.empty())
            {
                ctl->beginGroup("player-" + std::to_string(i+1) + "-" + this->LegacyName);
                new_profile->LoadConfig(ctl);
                ctl->endGroup();
            }
            // load modern profile
            ctl->beginGroup(this->Name + "-" + std::to_string(i+1));
            new_profile->LoadConfig(ctl);
            ctl->read("name", new_profile->Name, this->Name + " " + std::to_string(i+n_existing+1));
            ctl->endGroup();
        }
    }

    ctl->beginGroup(this->Name);
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        std::string default_profile_key = "default-profile-" + std::to_string(i+1);
        int index;
        ctl->read(default_profile_key.c_str(), index, -1);
        if(index == -1)
            this->m_defaultProfiles[i] = nullptr;
        else
        {
            index += n_existing;
            this->m_defaultProfiles[i] = this->m_profiles[index];
        }
    }
    this->LoadConfig_Custom(ctl);
    ctl->endGroup();
}

// optionally overriden methods

bool InputMethodType::ConsumeEvent(const SDL_Event* ev)
{
    (void)ev;
    return false;
}

bool InputMethodType::SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods)
{
    (void)active_methods;
    if(!method || !profile || player_no < 0 || player_no >= maxLocalPlayers)
    {
        return false;
    }
    return true;
}

bool InputMethodType::DeleteProfile_Custom(InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods)
{
    (void)profile;
    (void)active_methods;
    return true;
}

// How many per-type special options are there?
size_t InputMethodType::GetSpecialOptionCount()
{
    return 0;
}

// get a nullable char* describing the option
const char* InputMethodType::GetOptionName(size_t i)
{
    (void)i;
    return nullptr;
}
// get a nullable char* describing the current option value
// must be allocated in static or instance memory
const char* InputMethodType::GetOptionValue(size_t i)
{
    (void)i;
    return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType::OptionChange(size_t i)
{
    (void)i;
    return false;
}
// called when left is pressed
bool InputMethodType::OptionRotateLeft(size_t i)
{
    (void)i;
    return false;
}
// called when right is pressed
bool InputMethodType::OptionRotateRight(size_t i)
{
    (void)i;
    return false;
}

void InputMethodType::SaveConfig_Custom(IniProcessing* ctl)
{
    (void)ctl;
    // must be implemented if user has created special options
    SDL_assert_release(this->GetSpecialOptionCount() == 0);
}
void InputMethodType::LoadConfig_Custom(IniProcessing* ctl)
{
    (void)ctl;
    // must be implemented if user has created special options
    SDL_assert_release(this->GetSpecialOptionCount() == 0);
}

/*====================================================*\
|| implementation for global functions                ||
\*====================================================*/

/*====================================================*\
||                                                    ||
||         ADD EVERY NEW INPUT METHOD HERE,           ||
||           IF SUPPORTED AT COMPILE TIME             ||
||                                                    ||
\*====================================================*/

// allocate InputMethodTypes according to system configuration
void Init()
{
    g_InputMethodTypes.push_back(new InputMethodType_Keyboard);
    g_InputMethodTypes.push_back(new InputMethodType_Joystick);
#ifdef TOUCHSCREEN_H
    g_InputMethodTypes.push_back(new InputMethodType_TouchScreen);
#endif
    // not yet ready for prime time
    // g_InputMethodTypes.push_back(new InputMethodType_Duplicate);
}

void Quit()
{
    ClearInputMethods();
    for(InputMethodType* type : g_InputMethodTypes)
    {
        delete type;
    }
    g_InputMethodTypes.clear();
}

// (for SDL clients) process SDL_Event using active InputMethodTypes
// return true if successfully processed, false if unrecognized
bool ProcessEvent(const SDL_Event* ev)
{
    for(InputMethodType* type : g_InputMethodTypes)
    {
        if(type && type->ConsumeEvent(ev))
            return true;
    }
    for(InputMethod* method : g_InputMethods)
    {
        if(method && method->ConsumeEvent(ev))
            return true;
    }
    return false;
}

// 1. Calls the UpdateControlsPre hooks of loaded InputMethodTypes
//    a. Syncs hardware state as needed
// 2. Updates Player and Editor controls by calling currently bound InputMethods
//    a. May call or set changeScreen, frmMain.toggleGifRecorder, TakeScreen, g_stats.enabled, etc
// 3. Calls the UpdateControlsPost hooks of loaded InputMethodTypes
//    a. May call or set changeScreen, frmMain.toggleGifRecorder, TakeScreen, g_stats.enabled, etc
//    b. May update SharedControls or SharedCursor using hardcoded keys
// 4. Updates speedrun and recording modules
// 5. Resolves inconsistent control states (Left+Right, etc)
bool Update()
{
    bool okay = true;

    // reset per-frame state for SharedCursor
    SharedCursor.Move = false;
    SharedCursor.Primary = false;
    SharedCursor.Secondary = false;
    SharedCursor.Tertiary = false;

    for(InputMethodType* type : g_InputMethodTypes)
    {
        type->UpdateControlsPre();
    }

    Controls_t blankControls = Controls_t();

    for(size_t i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t& controls = Player[i+1].Controls;
        CursorControls_t& cursor = SharedCursor;
        EditorControls_t& editor = ::EditorControls;

        controls = blankControls;

        if(i >= g_InputMethods.size())
            continue;

        InputMethod* method = g_InputMethods[i];
        if(!method)
        {
            // okay = false;
            continue;
        }
        if(!method->Update(controls, cursor, editor))
        {
            okay = false;
            DeleteInputMethod(method);
            // the method pointer is no longer valid
            continue;
        }
    }

    SharedControls = SharedControls_t();
    for(InputMethodType* type : g_InputMethodTypes)
    {
        type->UpdateControlsPost();
    }

    if(SharedCursor.Move)
    {
        if(SharedCursor.X >= 0. && SharedCursor.Y >= 0.)
        {
            int window_x, window_y;
            XRender::mapFromScreen(SharedCursor.X, SharedCursor.Y, &window_x, &window_y);
            XWindow::placeCursor(window_x, window_y);
        }
    }

    // sync controls
    Record::Sync();
    for(int i = 0; i < numPlayers && i < maxLocalPlayers; i++)
    {
        speedRun_syncControlKeys(i, Player[i+1].Controls);
    }

    // resolve invalid states
    For(B, 1, numPlayers)
    {
        int A;
        // if there is an input method bound to the player,
        //   let them control themselves.
        //   (same in spirit as old B == 2 && numPlayers == 2 case)
        if((B-1 < (int)g_InputMethods.size() && g_InputMethods[B-1]) || QuickReconnectScreen::g_active)
            A = B;
        // otherwise, let Player 1 control them (blank controls later for SingleCoop)
        else
            A = 1;

        // With Player(A).Controls
        {
            auto &p = Player[A];
            Controls_t &c = p.Controls;

            if(!c.Start && !c.Jump)
                p.UnStart = true;

            if(c.Up && c.Down)
            {
                c.Up = false;
                c.Down = false;
            }

            if(c.Left && c.Right)
            {
                c.Left = false;
                c.Right = false;
            }

            if(!(p.State == 5 && p.Mount == 0) && c.AltRun)
                c.Run = true;

            if(ForcedControls && GamePaused == PauseCode::None)
            {
                c = ForcedControl;
            }

            // new location for multi-mario (SingleCoop, "supermario128") code
            if(A != B)
                Player[B].Controls = c;
        } // End With
    }

    // single coop code -- may want to revise
    if(SingleCoop > 0)
    {
        if(numPlayers == 1 || numPlayers > 2)
            SingleCoop = 0;

        if(SingleCoop == 1)
            Player[2].Controls = blankControls;
        else
            Player[1].Controls = blankControls;
    }

    For(A, 1, numPlayers)
    {
        {
            Player_t &p = Player[A];
            if(p.SpinJump)
                p.Direction = (p.SpinFrame < 4 || p.SpinFrame > 9) ? -1 : 1;
        }
    }

    if(((int)g_InputMethods.size() < numPlayers) && (numPlayers <= maxLocalPlayers)
        && !SingleCoop && !GameMenu && !Record::replay_file)
    {
        // fill with nullptrs
        while((int)g_InputMethods.size() < numPlayers)
            g_InputMethods.push_back(nullptr);
        okay = false;
    }

    if(s_enterCheatScreen && GamePaused != PauseCode::TextEntry)
    {
        cheats_setBuffer(TextEntryScreen::Run("Enter cheat:"));
        s_enterCheatScreen = false;
        MenuCursorCanMove = false;
        MenuMouseRelease = false;
        MouseRelease = false;
    }

    g_disallowHotkeys = false;

    return okay;
}

void SaveConfig(IniProcessing* ctl)
{
    for(InputMethodType* type : g_InputMethodTypes)
    {
        type->SaveConfig(ctl);
    }
}

void LoadConfig(IniProcessing* ctl)
{
    for(InputMethodType* type : g_InputMethodTypes)
    {
        type->ClearProfiles(g_InputMethods);
        type->LoadConfig(ctl);
    }
}

InputMethod* PollInputMethod() noexcept
{
    // don't poll unless there is a free slot
    size_t player_no = 0;
    while(player_no < g_InputMethods.size() && g_InputMethods[player_no] != nullptr)
    {
        player_no ++;
    }

    if(player_no >= maxLocalPlayers)
        return nullptr;
 
    // try all input method types
    InputMethod* new_method = nullptr;
    for(InputMethodType* type : g_InputMethodTypes)
    {
        new_method = type->Poll(g_InputMethods);
        if(new_method)
            break;
    }
    if(!new_method)
        return nullptr;

    // check that the InputMethodType properly assigned itself as the new InputMethod's Type
    SDL_assert_release(new_method->Type != nullptr); // InputMethodType did not assign itself as Type for new InputMethod
    if(!new_method->Type)
        return nullptr;

    // tentatively add to the input method list so we can call SetInputMethodProfile
    if(player_no < g_InputMethods.size())
    {
        g_InputMethods[player_no] = new_method;
    }
    else
    {
        g_InputMethods.push_back(new_method);
    }

    // try a number of ways of assigning a profile if one has not already been assigned
    std::vector<InputMethodProfile*> profiles = new_method->Type->GetProfiles();

    // fallback 1: last profile used by this player index
    if(!new_method->Profile)
    {
        InputMethodProfile* default_profile = new_method->Type->GetDefaultProfile(player_no);

        if(default_profile)
        {
            SetInputMethodProfile(player_no, default_profile);
        }
    }
    // fallback 2: find first unused profile
    if(!new_method->Profile)
    {
        size_t i;

        for(i = 0; i < profiles.size(); i++)
        {
            bool unused = true;
            for(InputMethod* other_method : g_InputMethods)
            {
                if(other_method && other_method->Profile == profiles[i])
                {
                    unused = false;
                    break;
                }
            }

            if(unused)
                break;
        }

        if(i != profiles.size())
            SetInputMethodProfile(player_no, profiles[i]);
    }
    // fallback 3: use first profile
    if(!new_method->Profile && !profiles.empty())
    {
        SetInputMethodProfile(player_no, profiles[0]);
    }
    // fallback 4: new default profile
    if(!new_method->Profile)
    {
        SetInputMethodProfile(player_no, new_method->Type->AddProfile());
    }
    // should only STILL be null if something is very wrong (alloc failed, etc)
    if(!new_method->Profile)
    {
        pLogWarning("Failed to find/alloc/set profile for new %s.",
            new_method->Type->Name.c_str());
        delete new_method;
        return nullptr;
    }

    pLogDebug("Just activated new %s '%s' with profile '%s'.",
        new_method->Type->Name.c_str(),
        new_method->Name.c_str(),
        new_method->Profile->Name.c_str());

    return new_method;
}

void DeleteInputMethod(InputMethod* method)
{
    if(!method)
        return;
    std::vector<InputMethod*>::iterator loc
        = std::find(g_InputMethods.begin(), g_InputMethods.end(), method);
    while(loc != g_InputMethods.end())
    {
        *loc = nullptr;
        loc = std::find(g_InputMethods.begin(), g_InputMethods.end(), method);
    }
    pLogDebug("Just disconnected %s '%s' with profile '%s'.",
        method->Type->Name.c_str(),  method->Name.c_str(), method->Profile->Name.c_str());
    delete method;
}

void DeleteInputMethodSlot(int slot)
{
    if(slot < 0 || (size_t)slot > g_InputMethods.size())
        return;
    if(g_InputMethods[slot] != nullptr)
    {
        DeleteInputMethod(g_InputMethods[slot]);
    }
    g_InputMethods.erase(g_InputMethods.begin() + slot);
}

bool SetInputMethodProfile(int player_no, InputMethodProfile* profile)
{
    if(player_no >= (int)g_InputMethods.size() || !g_InputMethods[player_no] || !profile)
        return false;
    InputMethod* method = g_InputMethods[player_no];
    if(!method->Type)
        return false;
    return method->Type->SetProfile(method, player_no, profile, g_InputMethods);
}

bool SetInputMethodProfile(InputMethod* method, InputMethodProfile* profile)
{
    if(!method || !profile || !method->Type)
        return false;

    std::vector<InputMethod*>::iterator loc = std::find(g_InputMethods.begin(), g_InputMethods.end(), method);
    size_t player_no = loc - g_InputMethods.begin();
    if(player_no == g_InputMethods.size())
        return false;

    return method->Type->SetProfile(method, player_no, profile, g_InputMethods);
}

void ClearInputMethods()
{
    for(size_t i = 0; i < g_InputMethods.size(); i++)
    {
        DeleteInputMethod(g_InputMethods[i]);
    }
    g_InputMethods.clear();
}


// player is 1-indexed :(
void Rumble(int player, int ms, float strength)
{
    if(!g_config.JoystickEnableRumble || GameMenu || GameOutro)
        return;

    if(player < 1 || player > (int)g_InputMethods.size())
        return;
    if(!g_InputMethods[player-1])
        return;
    g_InputMethods[player-1]->Rumble(ms, strength);
}

void RumbleAllPlayers(int ms, float strength)
{
    if(!g_config.JoystickEnableRumble || GameMenu || GameOutro)
        return;

    for(InputMethod* method : g_InputMethods)
    {
        if(!method)
            continue;
        method->Rumble(ms, strength);
    }
}


StatusInfo GetStatus(int player)
{
    if(player < 1 || player >= (int)g_InputMethods.size() + 1 || g_InputMethods[player - 1] == nullptr)
        return StatusInfo();
    return g_InputMethods[player - 1]->GetStatus();
}


void RenderTouchControls()
{
#ifdef TOUCHSCREEN_H
    // only want to render when the touchscreen is in use
    InputMethod_TouchScreen* active_touchscreen = nullptr;
    int player_no = 1;
    for(size_t i = 0; i < g_InputMethods.size(); i++)
    {
        InputMethod* method = g_InputMethods[i];
        if(!method)
            continue;
        InputMethod_TouchScreen* m = dynamic_cast<InputMethod_TouchScreen*>(method);
        if(m)
        {
            active_touchscreen = m;
            player_no = i + 1;
            break;
        }
    }
    if(!g_renderTouchscreen)
        return;

    InputMethodType_TouchScreen* touchscreen = nullptr;
    if(active_touchscreen)
        touchscreen = dynamic_cast<InputMethodType_TouchScreen*>(active_touchscreen->Type);
    else
    {
        for(InputMethodType* type : g_InputMethodTypes)
        {
            if(!type)
                continue;
            InputMethodType_TouchScreen* t = dynamic_cast<InputMethodType_TouchScreen*>(type);
            if(t)
            {
                touchscreen = t;
                break;
            }
        }
    }
    if(!touchscreen)
        return;

    // This is also called within Controls::Update.
    //   We also need to call here, because of cases
    //   such as GameThing where Controls::Update is
    //   not called.
    //   Will result in duplicated calls many frames,
    //   but this is not a problem.
    touchscreen->m_controller.update();
    touchscreen->m_controller.render(player_no);
#endif // #ifdef TOUCHSCREEN_H
}

void UpdateTouchScreenSize()
{
#ifdef TOUCHSCREEN_H
    InputMethodType_TouchScreen* touchscreen = nullptr;
    for(InputMethodType* type : g_InputMethodTypes)
    {
        if(!type)
            continue;
        InputMethodType_TouchScreen* t = dynamic_cast<InputMethodType_TouchScreen*>(type);
        if(t)
        {
            touchscreen = t;
            break;
        }
    }

    if(!touchscreen)
        return;

    touchscreen->m_controller.updateScreenSize();
#endif // #ifdef TOUCHSCREEN_H
}

} // namespace Controls