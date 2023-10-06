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

#pragma once
#ifndef CONTROLS_H
#define CONTROLS_H

// forward declaration since some clients do not have SDL
typedef union SDL_Event SDL_Event;

#include <string>
#include <array>
#include <vector>

#include <IniProcessor/ini_processing.h>

#include "sdl_proxy/sdl_types.h"
#include "sdl_proxy/sdl_assert.h"

#include "globals.h"
#include "core/power.h"

extern Controls_t &operator|=(Controls_t &o1, const Controls_t &o2);

namespace Controls
{

using XPower::StatusInfo;

class InputMethod;
class InputMethodProfile;
class InputMethodType;

enum class ControlsClass
{
    None, Player, Cursor, Editor, Hotkey
};

// utility functions to access information from the Controls_t struct
namespace PlayerControls
{
// These attributes should be used by Input Methods and the game when possible
// to allow them to function even if the controls structures change.

// enumerate of the Player key indices (which are almost never used)
enum Buttons : size_t
{
    Up = 0, Down, Left, Right, Jump, AltJump, Run, AltRun, Drop, Start, MAX
};

static constexpr size_t n_buttons = Buttons::MAX;

inline const char *GetButtonName_INI(size_t i)
{
    switch(i)
    {
    case Buttons::Up:
        return "up";
    case Buttons::Down:
        return "down";
    case Buttons::Left:
        return "left";
    case Buttons::Right:
        return "right";
    case Buttons::Jump:
        return "jump";
    case Buttons::AltJump:
        return "altjump";
    case Buttons::Run:
        return "run";
    case Buttons::AltRun:
        return "altrun";
    case Buttons::Drop:
        return "drop";
    case Buttons::Start:
        return "start";
    default:
        return "NULL";
    }
}

inline const char *GetButtonName_UI_Init(size_t i)
{
    switch(i)
    {
    case Buttons::Up:
        return "Up";
    case Buttons::Down:
        return "Down";
    case Buttons::Left:
        return "Left";
    case Buttons::Right:
        return "Right";
    case Buttons::Jump:
        return "Jump";
    case Buttons::AltJump:
        return "Alt Jump";
    case Buttons::Run:
        return "Run";
    case Buttons::AltRun:
        return "Alt Run";
    case Buttons::Drop:
        return "Drop Item";
    case Buttons::Start:
        return "Start";
    default:
        return "NULL";
    }
}

extern std::array<std::string, Buttons::MAX> g_button_name_UI;

inline const char *GetButtonName_UI(size_t i)
{
    if(i < Buttons::MAX)
        return g_button_name_UI[i].c_str();

    return "NULL";
}

// convenience function for accessing a button index
inline bool &GetButton(Controls_t &c, size_t i)
{
    switch(i)
    {
    case Buttons::Up:
        return c.Up;
    case Buttons::Down:
        return c.Down;
    case Buttons::Left:
        return c.Left;
    case Buttons::Right:
        return c.Right;
    case Buttons::Jump:
        return c.Jump;
    case Buttons::AltJump:
        return c.AltJump;
    case Buttons::Run:
        return c.Run;
    case Buttons::AltRun:
        return c.AltRun;
    case Buttons::Drop:
        return c.Drop;
    case Buttons::Start:
        return c.Start;
    default:
        SDL_assert(false); // -V654 // Made especially to fail on abnormal value
        return c.Start;
    }
}
} // namespace (Controls::)PlayerControls

// utility functions to access information from the CursorControls_t struct
// note that CursorX and CursorY are analogue and do not directly correspond with
// the virtual buttons CursorUp, CursorDown, CursorLeft, and CursorRight
namespace CursorControls
{
// These attributes should be used by Input Methods and the game when possible
// to allow them to function even if the controls structures change.

// enumerate of the Cursor key indices (which are almost never used)
enum Buttons : size_t
{
    CursorUp = 0, CursorDown, CursorLeft, CursorRight, Primary,
    Secondary, Tertiary, MAX
};

static constexpr size_t n_buttons = Buttons::MAX;


inline const char *GetButtonName_INI(size_t i)
{
    switch(i)
    {
    case Buttons::CursorUp:
        return "cursor-up";
    case Buttons::CursorDown:
        return "cursor-down";
    case Buttons::CursorLeft:
        return "cursor-left";
    case Buttons::CursorRight:
        return "cursor-right";
    case Buttons::Primary:
        return "primary";
    case Buttons::Secondary:
        return "secondary";
    case Buttons::Tertiary:
        return "tertiary";
    default:
        return "NULL";
    }
}

inline const char *GetButtonName_UI_Init(size_t i)
{
    switch(i)
    {
    case Buttons::CursorUp:
        return "Mouse Up";
    case Buttons::CursorDown:
        return "Mouse Down";
    case Buttons::CursorLeft:
        return "Mouse Left";
    case Buttons::CursorRight:
        return "Mouse Right";
    case Buttons::Primary:
        return "Primary";
    case Buttons::Secondary:
        return "Secondary";
    case Buttons::Tertiary:
        return "Tertiary";
    default:
        return "NULL";
    }
}

extern std::array<std::string, Buttons::MAX> g_button_name_UI;

inline const char *GetButtonName_UI(size_t i)
{
    if(i < Buttons::MAX)
        return g_button_name_UI[i].c_str();

    return "NULL";
}

// convenience function for accessing a button index
inline bool &GetButton(CursorControls_t &c, size_t i)
{
    switch(i)
    {
    case Buttons::Primary:
        return c.Primary;
    case Buttons::Secondary:
        return c.Secondary;
    case Buttons::Tertiary:
        return c.Tertiary;
    case Buttons::CursorUp:
    case Buttons::CursorDown:
    case Buttons::CursorLeft:
    case Buttons::CursorRight:
    default:
        SDL_assert(false); // -V654 // Made especially to fail on abnormal value
        return c.Primary;
    }
}
} // namespace (Controls::)CursorControls

// utility functions to access information from the EditorControls_t struct
// note that ScrollUp, ScrollDown, ScrollLeft, and ScrollRight are analogue and are set separately
namespace EditorControls
{
// These attributes should be used by Input Methods and the game when possible
// to allow them to function even if the controls structures change.

// enumerate of the Editor key indices (which are almost never used)
enum Buttons : size_t
{
    ScrollUp = 0, ScrollDown, ScrollLeft, ScrollRight, FastScroll,
    ModeSelect, ModeErase, PrevSection, NextSection, SwitchScreens, TestPlay, MAX
};

static constexpr size_t n_buttons = Buttons::MAX;

inline const char *GetButtonName_INI(size_t i)
{
    switch(i)
    {
    case Buttons::ScrollUp:
        return "scroll-up";
    case Buttons::ScrollDown:
        return "scroll-down";
    case Buttons::ScrollLeft:
        return "scroll-left";
    case Buttons::ScrollRight:
        return "scroll-right";
    case Buttons::FastScroll:
        return "fast-scroll";
    case Buttons::ModeSelect:
        return "mode-select";
    case Buttons::ModeErase:
        return "mode-erase";
    case Buttons::PrevSection:
        return "prev-section";
    case Buttons::NextSection:
        return "next-section";
    case Buttons::SwitchScreens:
        return "switch-screens";
    case Buttons::TestPlay:
        return "test-play";
    default:
        return "NULL";
    }
}

inline const char *GetButtonName_UI_Init(size_t i)
{
    switch(i)
    {
    case Buttons::ScrollUp:
        return "Scrl Up";
    case Buttons::ScrollDown:
        return "Scrl Down";
    case Buttons::ScrollLeft:
        return "Scrl Left";
    case Buttons::ScrollRight:
        return "Scrl Right";
    case Buttons::FastScroll:
        return "Fast Scrl";
    case Buttons::ModeSelect:
        return "Mode Select";
    case Buttons::ModeErase:
        return "Mode Erase";
    case Buttons::PrevSection:
        return "Prev Sect";
    case Buttons::NextSection:
        return "Next Sect";
    case Buttons::SwitchScreens:
        return "Show Pane";
    case Buttons::TestPlay:
        return "Test Play";
    default:
        return "NULL";
    }
}

extern std::array<std::string, Buttons::MAX> g_button_name_UI;

inline const char *GetButtonName_UI(size_t i)
{
    if(i < Buttons::MAX)
        return g_button_name_UI[i].c_str();

    return "NULL";
}

// convenience function for accessing a button index
inline bool &GetButton(EditorControls_t &c, size_t i)
{
    switch(i)
    {
    case Buttons::FastScroll:
        return c.FastScroll;
    case Buttons::ModeSelect:
        return c.ModeSelect;
    case Buttons::ModeErase:
        return c.ModeErase;
    case Buttons::PrevSection:
        return c.PrevSection;
    case Buttons::NextSection:
        return c.NextSection;
    case Buttons::SwitchScreens:
        return c.SwitchScreens;
    case Buttons::TestPlay:
        return c.TestPlay;
    case Buttons::ScrollUp:
    case Buttons::ScrollDown:
    case Buttons::ScrollLeft:
    case Buttons::ScrollRight:
    default:
        SDL_assert(false); // -V654 // Made especially to fail on abnormal value
        return c.FastScroll;
    }
}
} // namespace (Controls::)EditorControls

// utility functions to handle hotkeys. can accommodate new hotkeys.
namespace Hotkeys
{
// These attributes should be used by Input Methods and the game when possible
// to allow them to function even when hotkeys change / are added.

// enumerate of the Hotkey indices (which are almost never used)
enum Buttons : size_t
{
    Fullscreen = 0,
    Screenshot, RecordGif,
    DebugInfo, EnterCheats,
    ToggleHUD, LegacyPause,
#ifdef DEBUG_BUILD
    ReloadLanguage,
#endif
    MAX
};

constexpr size_t n_buttons = Buttons::MAX;

inline const char *GetButtonName_INI(size_t i)
{
    switch(i)
    {
    case Buttons::Fullscreen:
        return "fullscreen";
    case Buttons::Screenshot:
        return "screenshot";
    case Buttons::RecordGif:
        return "record-gif";
    case Buttons::DebugInfo:
        return "debug-info";
    case Buttons::EnterCheats:
        return "enter-cheats";
    case Buttons::ToggleHUD:
        return "toggle-hud";
    case Buttons::LegacyPause:
        return "legacy-pause";
#ifdef DEBUG_BUILD
    case Buttons::ReloadLanguage:
        return "reload-language";
#endif
    default:
        return "NULL";
    }
}

inline const char *GetButtonName_UI_Init(size_t i)
{
    switch(i)
    {
    case Buttons::Fullscreen:
        return "Fullscreen";
    case Buttons::Screenshot:
        return "Screenshot";
    case Buttons::RecordGif:
        return "Record GIF";
    case Buttons::DebugInfo:
        return "Debug Info";
    case Buttons::EnterCheats:
        return "Enter Cheat";
    case Buttons::ToggleHUD:
        return "Toggle HUD";
    case Buttons::LegacyPause:
        return "Old Pause";
#ifdef DEBUG_BUILD
    case Buttons::ReloadLanguage:
        return "Reload lang";
#endif
    default:
        return "NULL";
    }
}

extern std::array<std::string, Buttons::MAX> g_button_name_UI;

inline const char *GetButtonName_UI(size_t i)
{
    if(i < Buttons::MAX)
        return g_button_name_UI[i].c_str();

    return "NULL";
}

// function for activating a hotkey
void Activate(size_t i, int player = 0);
} // namespace (Controls::)Hotkeys

using HotkeysPressed_t = std::array<int, Hotkeys::n_buttons>;

// represents a particular bound input method
class InputMethod
{
public:
    // not required to be unique
    std::string Name;

    // Once an InputMethod has been returned by PollInputMethod,
    //     the developer may trust that neither of these are null.
    // The InputMethodType is responsible for setting Type to itself.
    InputMethodType *Type = nullptr;
    // PollInputMethod is responsible for setting Profile;
    //     but the game may later change it depending on user interaction.
    InputMethodProfile *Profile = nullptr;

    virtual ~InputMethod();

    // Update function that sets player controls, cursor controls, and editor controls,
    // as well as optionally processing hotkeys, based on current device input.
    // Remember that hotkeys should only process once per hotkey press, so it may be easier
    // to put them in ConsumeEvent.
    // Return false if device lost.
    virtual bool Update(int player, Controls_t &c, CursorControls_t &m, EditorControls_t &e, HotkeysPressed_t &h) = 0;

    virtual void Rumble(int ms, float strength) = 0;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/

    // Used for battery, latency, etc
    // If the char* member is dynamically generated, you MUST use an instance-owned buffer
    // This will NOT be freed
    // returning a null pointer is allowed
    virtual StatusInfo GetStatus();

    // Optional function allowing developer to consume an SDL event (on SDL clients)
    //     usually used for hotkeys or for connect/disconnect events.
    // Called (1) in order of InputMethodTypes, then (2) in order of InputMethods
    // Returns true if event is consumed, false if other InputMethodTypes and InputMethods
    //     should receive it.
    virtual bool ConsumeEvent(const SDL_Event *ev);
};

// represents an input method profile for a particular input class
// some input method classes (keyboard) might disallow multiple
//   players using the same input profile, others (gamepad)
//   might disallow them using the same hardware device
class InputMethodProfile
{
public:
    // strongly encouraged to be unique, but nothing will break if it isn't
    std::string Name;

    // The InputMethodType that creates an InputMethodProfile
    // is responsible for setting Type to itself.
    InputMethodType *Type = nullptr;

    virtual ~InputMethodProfile();

    /*----------------------*\
    || SHARED CONFIGURATION ||
    \*----------------------*/

    struct CommonOptions
    {
        enum co
        {
            rumble = 0,
            ground_pound_by_alt_run,
            show_power_status,
            COUNT
        };
    };

    bool m_rumbleEnabled = false;
    bool m_groundPoundByAltRun = false;
    bool m_showPowerStatus = false;

    // assume that the IniProcessing* is already in the correct group
    // saves/loads the shared options and calls the device-specific Save/LoadConfig
    void SaveConfig_All(IniProcessing *ctl);
    void LoadConfig_All(IniProcessing *ctl);

    // each of these delegate to the _Custom methods.
    // How many per-profile special options are there?
    size_t GetOptionCount();

    // Methods to manage per-profile options
    // get a nullable char* describing the option
    const char *GetOptionName(size_t i);
    // get a nullable char* describing the current option value
    // must be allocated in static or instance memory
    const char *GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    bool OptionChange(size_t i);
    // called when left is pressed
    bool OptionRotateLeft(size_t i);
    // called when right is pressed
    bool OptionRotateRight(size_t i);

    /*-------------------------*\
    || PURE VIRTUAL FUNCTIONS  ||
    \*-------------------------*/
    // Polls a new primary/secondary device button for the i'th button of class c
    // Returns true on success and false if no button currently pressed
    // Never allows two player buttons to bind to the same device button
    // Expected to use ALL of the devices available to the InputMethodType
    virtual bool PollPrimaryButton(ControlsClass c, size_t i) = 0;
    virtual bool PollSecondaryButton(ControlsClass c, size_t i) = 0;

    // Deletes a primary button for the i'th button of class c (only called for non-Player buttons)
    virtual bool DeletePrimaryButton(ControlsClass c, size_t i) = 0;

    // Deletes a secondary device button for the i'th button of class c
    virtual bool DeleteSecondaryButton(ControlsClass c, size_t i) = 0;

    // Gets strings for the device buttons currently used for the i'th button of class c
    virtual const char *NamePrimaryButton(ControlsClass c, size_t i) = 0;
    virtual const char *NameSecondaryButton(ControlsClass c, size_t i) = 0;

    // assume that the IniProcessing* is already in the correct group
    // saves/loads the controls and the device-specific options
    virtual void SaveConfig(IniProcessing *ctl) = 0;
    virtual void LoadConfig(IniProcessing *ctl) = 0;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
    // How many device-specific per-profile special options are there?
    virtual size_t GetOptionCount_Custom();

    // Methods to manage device-specific per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount_Custom() returns 0.
    // get a nullable char* describing the option
    virtual const char *GetOptionName_Custom(size_t i);
    // get a nullable char* describing the current option value
    // must be allocated in static or instance memory
    virtual const char *GetOptionValue_Custom(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    virtual bool OptionChange_Custom(size_t i);
    // called when left is pressed
    virtual bool OptionRotateLeft_Custom(size_t i);
    // called when right is pressed
    virtual bool OptionRotateRight_Custom(size_t i);
};

// represents a class of input devices, such as keyboard, SDL gamepad,
//   or 3DS input. one of each class is instantiated in Controls::Init()
class InputMethodType
{
protected:
    // OWNED by InputMethodType
    std::vector<InputMethodProfile *> m_profiles;
    // nullable
    InputMethodProfile *m_defaultProfiles[maxLocalPlayers] = {nullptr};

    /*-------------------------*\
    || PURE VIRTUAL FUNCTION   ||
    \*-------------------------*/
    // function that must be declared per-type because each
    //     InputMethodType has its own InputMethodProfile class.
    virtual InputMethodProfile *AllocateProfile() noexcept = 0;

public:
    // absolutely required to be unique, because it is used to identify configuration. DO NOT LOCALIZE.
    std::string Name;
    // not required to be defined, refers to original "player-X-keyboard", etc, configurations.
    std::string LegacyName;

    // InputMethodType frees its InputMethodProfiles in its destructor
    virtual ~InputMethodType();

    // returns localized name based on translations, possibly to name individual methods and profiles
    virtual const std::string& LocalName() const;

    std::vector<InputMethodProfile *> GetProfiles();
    // returns a pointer to the new profile on success
    InputMethodProfile *AddProfile();
    // ensures that the profile is not in use by any active methods, and that it is not a default profile.
    // returns false if this would delete the last remaining profile, and any are in use.
    bool DeleteProfile(InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);
    // succeeds and returns true if no profiles are in use.
    // otherwise, leaves one profile allocated.
    bool ClearProfiles(const std::vector<InputMethod *> &active_methods);

protected:
    void SetDefaultProfile(int player_no, InputMethodProfile *profile);
public:
    InputMethodProfile *GetDefaultProfile(int player_no);
    bool SetProfile(InputMethod *method, int player_no, InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);

    void SaveConfig(IniProcessing *ctl);
    void LoadConfig(IniProcessing *ctl);

    /*-------------------------*\
    || PURE VIRTUAL FUNCTIONS  ||
    \*-------------------------*/
    // Tests whether a profile is of the corresponding type using RTTI
    virtual bool TestProfileType(InputMethodProfile *profile) = 0;

    // Returns true if Rumble is ever supported
    virtual bool RumbleSupported() = 0;

    // Hooks that are called before and after Player controls are updated
    // Update any information that the InputMethods will reference
    virtual void UpdateControlsPre() = 0;
    // Do post-update calls to global methods,
    //     emergency sets of the menu controls, etc.
    // This is where shared controls should be modified.
    virtual void UpdateControlsPost() = 0;

    // null if no new input method is ready
    // allocates the new InputMethod on the heap
    // do not activate any methods using controllers or keysets that are already in active_methods
    // must set the InputMethod's Name, and set its Type to the InputMethod Type
    // may optionally set the InputMethod's Profile
    virtual InputMethod *Poll(const std::vector<InputMethod *> &active_methods) noexcept = 0;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
public:
    // Optional function allowing developer to consume an SDL event (on SDL clients)
    //     usually used for hotkeys or for connect/disconnect events.
    // Called (1) in order of InputMethodTypes, then (2) in order of InputMethods
    // Returns true if event is consumed, false if other InputMethodTypes and InputMethods
    //     should receive it.
    virtual bool ConsumeEvent(const SDL_Event *ev);

protected:
    // optional function allowing developer to associate device information with profile, etc
    // if developer wants to forbid assignment, return false
    virtual bool SetProfile_Custom(InputMethod *method, int player_no, InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);
    // unregisters any references to the profile before final deallocation
    // returns false to prevent deletion if this is impossible
    virtual bool DeleteProfile_Custom(InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);

public:
    // How many per-type special options are there?
    virtual size_t GetOptionCount();

    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount() returns 0.
    // get a (nullable) char* describing the option
    virtual const char *GetOptionName(size_t i);
    // get a (nullable) char* describing the current option value
    // if not null, must be allocated in static or instance memory
    // WILL NOT be freed
    virtual const char *GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    virtual bool OptionChange(size_t i);
    // called when left is pressed
    virtual bool OptionRotateLeft(size_t i);
    // called when right is pressed
    virtual bool OptionRotateRight(size_t i);

protected:
    virtual void SaveConfig_Custom(IniProcessing *ctl);
    virtual void LoadConfig_Custom(IniProcessing *ctl);
};

// initialize the strings for localization (normally called by Init())
void InitStrings();

// allocate InputMethodTypes according to system configuration
void Init();

// free all InputMethodTypes, InputMethodProfiles (implicitly), and InputMethods
void Quit();

// (for SDL clients) process SDL_Event using active InputMethodTypes
// return true if successfully processed, false if unrecognized
bool ProcessEvent(const SDL_Event *ev);

// 1. Calls the UpdateControlsPre hooks of currently active InputMethodTypes
//    a. Syncs hardware state as needed
// 2. Updates Player and Editor controls by calling currently bound InputMethods
//    a. May call or set changeScreen, frmMain.toggleGifRecorder, TakeScreen, g_stats.enabled, etc
// 3. Calls the UpdateControlsPost hooks of currently active InputMethodTypes
//    a. May call or set changeScreen, frmMain.toggleGifRecorder, TakeScreen, g_stats.enabled, etc
//    b. If GameMenu or GameOutro is set, may update controls or Menu variables using hardcoded keys
// 4. Updates speedrun and recording modules
// 5. Resolves inconsistent control states (Left+Right, etc)
//
// If `check_lost_devices` is set and any of the devices were lost, removes the corresponding input method and returns false.
bool Update(bool check_lost_devices = true);

void SaveConfig(IniProcessing *ctl);
void LoadConfig(IniProcessing *ctl);

// null if no input method is ready and being pressed
// allocates the new InputMethod on the heap
InputMethod *PollInputMethod() noexcept;
void DeleteInputMethod(InputMethod *method);
void DeleteInputMethodSlot(int slot);
bool SetInputMethodProfile(int slot, InputMethodProfile *profile);
bool SetInputMethodProfile(InputMethod *method, InputMethodProfile *profile);
void ClearInputMethods();

// player is 1-indexed here :(
void Rumble(int player, int ms, float strength);
void RumbleAllPlayers(int ms, float strength);

StatusInfo GetStatus(int player);

void RenderTouchControls();
void UpdateTouchScreenSize();

// global variables at bottom

// a nulled pointer in here signifies a player whose controller has disconnected
extern std::vector<InputMethod *> g_InputMethods;
// no nulled pointers here
extern std::vector<InputMethodType *> g_InputMethodTypes;
extern bool g_renderTouchscreen;
extern HotkeysPressed_t g_hotkeysPressed;
extern bool g_disallowHotkeys;

} // namespace Controls

#endif // CONTROLS_H
