/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef CONTROLS_H
#define CONTROLS_H

#include <vector>

#include <IniProcessor/ini_processing.h>

#include "globals.h"

namespace Controls
{
class InputMethod;
class InputMethodProfile;
class InputMethodType;

// a nulled pointer in here signifies a player whose controller has disconnected
extern std::vector<InputMethod*> g_InputMethods;
extern std::vector<InputMethodType*> g_InputMethodTypes;
extern bool g_renderTouchscreen;

// utility functions to access information from the Controls_t struct
namespace PlayerControls
{
    // These attributes should be used by Input Methods and the game when possible
    // to allow them to function even if the controls structures change.
    static constexpr size_t n_buttons = 10;

    // enumerate of the Player key indices (which are almost never used)
    enum Buttons : size_t
    {
        Up = 0, Down, Left, Right, Jump, AltJump, Run, AltRun, Drop, Start
    };


    // can be adapted for locale
    inline const char* GetButtonName_INI(size_t i)
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

    // can be adapted for locale
    inline const char* GetButtonName_UI(size_t i)
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

    // convenience function for accessing a button index
    inline bool& GetButton(Controls_t& c, size_t i)
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
                SDL_assert(false);
                return c.Start;
        }
    }
} // namespace (Controls::)PlayerControls

// represents a particular bound input method
class InputMethod
{
public:
    // not required to be unique
    std::string Name;

    // Once an InputMethod has been returned by PollInputMethod,
    //     the developer may trust that neither of these are null.
    // The InputMethodType is responsible for setting Type to itself.
    InputMethodType* Type = nullptr;
    // PollInputMethod is responsible for setting Profile;
    //     but the game may later change it depending on user interaction.
    InputMethodProfile* Profile = nullptr;

    virtual ~InputMethod();

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    virtual bool Update(Controls_t& c) = 0;
    // virtual bool Update(EditorControls_t& c) = 0;

    virtual void Rumble(int ms, float strength) = 0;

    // Used for battery, latency, etc
    // If this is dynamically generated, you MUST use an instance-owned buffer
    // This will NOT be freed
    // returning a null pointer is allowed
    virtual const char* StatusInfo();
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
    InputMethodType* Type = nullptr;

    virtual ~InputMethodProfile();

    /*-------------------------*\
    || PURE VIRTUAL FUNCTIONS  ||
    \*-------------------------*/
    // Polls a new primary/secondary device button for the i'th player button
    // Returns true on success and false if no button pressed
    // Never allows two player buttons to bind to the same device button
    // Expected to use ALL of the devices available to the InputMethodType
    virtual bool PollPrimaryButton(size_t i) = 0;
    virtual bool PollSecondaryButton(size_t i) = 0;

    // Deletes a secondary device button for the i'th player button
    virtual bool DeleteSecondaryButton(size_t i) = 0;

    // Gets strings for the device buttons currently used for the i'th player button
    virtual const char* NamePrimaryButton(size_t i) = 0;
    virtual const char* NameSecondaryButton(size_t i) = 0;

    // assume that the IniProcessing* is already in the correct group
    virtual void SaveConfig(IniProcessing* ctl) = 0;
    virtual void LoadConfig(IniProcessing* ctl) = 0;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
    // How many per-profile special options are there?
    virtual size_t GetSpecialOptionCount();

    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount() returns 0.
    // get a nullable char* describing the option
    virtual const char* GetOptionName(size_t i);
    // get a nullable char* describing the current option value
    // must be allocated in static or instance memory
    virtual const char* GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    virtual bool OptionChange(size_t i);
    // called when left is pressed
    virtual bool OptionRotateLeft(size_t i);
    // called when right is pressed
    virtual bool OptionRotateRight(size_t i);
};

// represents a class of input devices, such as keyboard, SDL gamepad,
//   or 3DS input. one of each class is instantiated in Controls::Init()
class InputMethodType
{
protected:
    // OWNED by InputMethodType
    std::vector<InputMethodProfile*> m_profiles;
    // nullable
    InputMethodProfile* m_defaultProfiles[maxLocalPlayers] = {nullptr};

    /*-------------------------*\
    || PURE VIRTUAL FUNCTION   ||
    \*-------------------------*/
    // function that must be declared per-type because each
    //     InputMethodType has its own InputMethodProfile class.
    virtual InputMethodProfile* AllocateProfile() noexcept = 0;

public:
    // absolutely required to be unique, because it is used to identify configuration
    std::string Name;
    // not required to be defined, refers to original "player-X-keyboard", etc, configurations.
    std::string LegacyName;

    // InputMethodType frees its InputMethodProfiles in its destructor
    virtual ~InputMethodType();

    std::vector<InputMethodProfile*> GetProfiles();
    // returns a pointer to the new profile on success
    InputMethodProfile* AddProfile();
    // ensures that the profile is not in use by any active methods, and that it is not a default profile.
    // returns false if this would delete the last remaining profile, and any are in use.
    bool DeleteProfile(InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods);
    // succeeds and returns true if no profiles are in use.
    // otherwise, leaves one profile allocated.
    bool ClearProfiles(const std::vector<InputMethod*>& active_methods);

protected:
    void SetDefaultProfile(int player_no, InputMethodProfile* profile);
public:
    InputMethodProfile* GetDefaultProfile(int player_no);
    bool SetProfile(InputMethod* method, int player_no, InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods);

    void SaveConfig(IniProcessing* ctl);
    void LoadConfig(IniProcessing* ctl);

    /*-------------------------*\
    || PURE VIRTUAL FUNCTIONS  ||
    \*-------------------------*/
    // Hooks that are called before and after Player controls are updated
    // Update any information that the InputMethods will reference
    virtual void UpdateControlsPre() = 0;
    // Do post-update calls to global methods,
    //     emergency sets of the menu controls, etc.
    virtual void UpdateControlsPost() = 0;

    // null if no new input method is ready
    // allocates the new InputMethod on the heap
    // do not activate any methods using controllers or keysets that are already in active_methods
    // must set the InputMethod's Name, and set its Type to the InputMethod Type
    // may optionally set the InputMethod's Profile
    virtual InputMethod* Poll(const std::vector<InputMethod*>& active_methods) noexcept = 0;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
protected:
    // optional function allowing developer to associate device information with profile, etc
    // if developer wants to forbid assignment, return false
    virtual bool SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods);

public:
    // How many per-type special options are there?
    virtual size_t GetSpecialOptionCount();

    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount() returns 0.
    // get a (nullable) char* describing the option
    virtual const char* GetOptionName(size_t i);
    // get a (nullable) char* describing the current option value
    // if not null, must be allocated in static or instance memory
    // WILL NOT be freed
    virtual const char* GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    virtual bool OptionChange(size_t i);
    // called when left is pressed
    virtual bool OptionRotateLeft(size_t i);
    // called when right is pressed
    virtual bool OptionRotateRight(size_t i);

protected:
    virtual void SaveConfig_Custom(IniProcessing* ctl);
    virtual void LoadConfig_Custom(IniProcessing* ctl);
};

// allocate InputMethodTypes according to system configuration
void Init();

// free all InputMethodTypes, InputMethodProfiles (implicitly), and InputMethods
void Quit();

// 1. Calls the UpdateControlsPre hooks of currently active InputMethodTypes
//    a. Syncs hardware state as needed
// 2. Updates Player and Editor controls by calling currently bound InputMethods
//    a. May call or set changeScreen, frmMain.toggleGifRecorder, TakeScreen, g_stats.enabled, etc
// 3. Calls the UpdateControlsPost hooks of currently active InputMethodTypes
//    a. May call or set changeScreen, frmMain.toggleGifRecorder, TakeScreen, g_stats.enabled, etc
//    b. If GameMenu or GameOutro is set, may update controls or Menu variables using hardcoded keys
// 4. Updates speedrun and recording modules
// 5. Resolves inconsistent control states (Left+Right, etc)
// Returns false if any of the devices were lost.
bool Update();

void SaveConfig(IniProcessing* ctl);
void LoadConfig(IniProcessing* ctl);

// null if no input method is ready and being pressed
// allocates the new InputMethod on the heap
InputMethod* PollInputMethod() noexcept;
void DeleteInputMethod(InputMethod* method);
void DeleteInputMethodSlot(int slot);
bool SetInputMethodProfile(int slot, InputMethodProfile* profile);
bool SetInputMethodProfile(InputMethod* method, InputMethodProfile* profile);
void ClearInputMethods();

// player is 1-indexed here :(
void Rumble(int player, int ms, float strength);
void RumbleAllPlayers(int ms, float strength);

void RenderTouchControls();
void UpdateTouchScreenSize();

} // namespace Controls

#endif // CONTROLS_H
