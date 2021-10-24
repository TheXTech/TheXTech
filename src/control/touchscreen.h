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

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <map>
#include <string>
#include <vector>

#include <SDL2/SDL_touch.h>

#include "../controls.h"
#include "../std_picture.h"

typedef struct _SDL_Haptic SDL_Haptic;

namespace Controls
{


/*!
 * \brief A basic class including graphics for touchscreen controller
 */

class TouchScreenGFX_t
{
    std::vector<StdPicture*> m_loadedImages;
    void loadImage(StdPicture &img, std::string path);
    int m_loadErrors = 0;
public:
    bool m_success = false;

    TouchScreenGFX_t();
    ~TouchScreenGFX_t();

    enum
    {
        BUTTON_START = 0,
        BUTTON_LEFT,
        BUTTON_LEFT_CHAR,
        BUTTON_RIGHT,
        BUTTON_RIGHT_CHAR,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_UPLEFT,
        BUTTON_UPRIGHT,
        BUTTON_DOWNLEFT,
        BUTTON_DOWNRIGHT,
        BUTTON_A,
        BUTTON_A_PS,
        BUTTON_A_BLANK,
        BUTTON_A_DO,
        BUTTON_A_ENTER,
        BUTTON_A_JUMP,
        BUTTON_B,
        BUTTON_B_PS,
        BUTTON_B_BLANK,
        BUTTON_B_JUMP,
        BUTTON_B_SPINJUMP,
        BUTTON_X,
        BUTTON_X_PS,
        BUTTON_X_BACK,
        BUTTON_X_BLANK,
        BUTTON_X_BOMB,
        BUTTON_X_BUMERANG,
        BUTTON_X_FIRE,
        BUTTON_X_HAMMER,
        BUTTON_X_RUN,
        BUTTON_X_SWORD,
        BUTTON_Y,
        BUTTON_Y_PS,
        BUTTON_Y_BLANK,
        BUTTON_Y_BOMB,
        BUTTON_Y_BUMERANG,
        BUTTON_Y_FIRE,
        BUTTON_Y_HAMMER,
        BUTTON_Y_RUN,
        BUTTON_Y_STATUE,
        BUTTON_Y_SWORD,
        BUTTON_DROP,
        BUTTON_HOLD_RUN_OFF,
        BUTTON_HOLD_RUN_ON,
        BUTTON_VIEW_TOGGLE_OFF,
        BUTTON_VIEW_TOGGLE_ON,
        BUTTON_ANALOG_BORDER,
        BUTTON_ANALOG_STICK,
        BUTTONS_END
    };
    StdPicture touch[BUTTONS_END];
};

/*!
 * \brief A mobile touch-screen controller which reads state of the keyboard device
 */
class TouchScreenController
{
    //! Count of touch devices
    int m_touchDevicesCount = 0;
    //! Physical screen width
    int m_screenWidth = 0;
    //! Physical screen height
    int m_screenHeight = 0;
    //! Actual touch device to use
    int m_actualDevice = -1;
    //! Graphics for controller
    TouchScreenGFX_t m_GFX;

public:

    SDL_Haptic *m_vibrator = nullptr;

    /*!
     * \brief Is touch-screen supported?
     */
    bool touchSupported();

    /*!
     * \brief Is touch-screen being touched?
     */
    bool touchOn();

    enum commands
    {
        key_BEGIN = 0,
        key_start = 0,
        key_left,
        key_right,
        key_up,
        key_down,
        key_upleft,
        key_upright,
        key_downleft,
        key_downright,
        key_run,
        key_jump,
        key_altrun,
        key_altjump,
        key_drop,
        key_holdRun,
        key_toggleKeysView,
        key_END
    };

    enum sizes
    {
        size_small = 0,
        size_medium,
        size_large,
        size_END
    };
    int m_preferredSize = size_medium;

    //! In-game controls pressed
    Controls_t m_current_keys;
    bool m_keysHeld[key_END] = {false};
    double m_cursorX, m_cursorY;
    bool m_cursorHeld = false;

    struct ExtraKeys_t
    {
        bool keyToggleView = false;
        bool keyToggleViewOnce = false;

        bool keyHoldRun = false;
        bool keyHoldRunOnce = false;

        bool keyRunOnce = false;
        bool keyAltRunOnce = false;
    } m_current_extra_keys;

    //! Touch hidden by default, re-enablable by left-top corder
    bool       m_touchHidden = true;
    bool       m_runHeld = false;

    struct FingerState
    {
        bool alive = false;
        bool ignore = false;
        bool heldKey[key_END] = {};
        bool heldKeyPrev[key_END] = {};

        FingerState();
        FingerState(const FingerState &fs);
        FingerState &operator=(const FingerState &fs);
    };

private:
    //! Held finger states
    std::map<SDL_FingerID, FingerState> m_fingers;

    void doVibration();

public:
    /*!
     * \brief Constructor
     */
    TouchScreenController();

    /*!
     * \brief Destructor
     */
    ~TouchScreenController();

    void updateScreenSize();

    /*!
     * \brief Read current state of keyboard controller
     */
    void processTouchDevice(int dev_i);

    void update();

    void render(int player_no);

    void resetState();
};

class InputMethod_TouchScreen : public InputMethod
{
public:
    using InputMethod::Type;
    using InputMethod::Profile;

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(Controls_t& c, CursorControls_t& m, EditorControls_t& e);

    void Rumble(int ms, float strength);
};

class InputMethodProfile_TouchScreen : public InputMethodProfile
{
public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    InputMethodProfile_TouchScreen();

    // Polls a new (secondary) device button for the i'th player button
    // Returns true on success and false if no button pressed
    // Never allows two player buttons to bind to the same device button
    bool PollPrimaryButton(ControlsClass c, size_t i);
    bool PollSecondaryButton(ControlsClass c, size_t i);

    // Deletes a primary button for the i'th button of class c (only called for non-Player buttons)
    bool DeletePrimaryButton(ControlsClass c, size_t i);

    // Deletes a secondary device button for the i'th button of class c
    bool DeleteSecondaryButton(ControlsClass c, size_t i);

    // Gets strings for the device buttons currently used for the i'th button of class c
    const char* NamePrimaryButton(ControlsClass c, size_t i);
    const char* NameSecondaryButton(ControlsClass c, size_t i);

    // one can assume that the IniProcessing* is already in the correct group
    void SaveConfig(IniProcessing* ctl);
    void LoadConfig(IniProcessing* ctl);
};

class InputMethodType_TouchScreen : public InputMethodType
{
private:
    bool m_canPoll = false;

    InputMethodProfile* AllocateProfile() noexcept;

public:
    using InputMethodType::Name;
    using InputMethodType::m_profiles;

    TouchScreenController m_controller;

    InputMethodType_TouchScreen();

    void UpdateControlsPre();
    void UpdateControlsPost();

    // null if no input method is ready
    // allocates the new InputMethod on the heap
    InputMethod* Poll(const std::vector<InputMethod*>& active_methods) noexcept;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
public:
    // How many per-type special options are there?
    size_t GetSpecialOptionCount();
    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount() returns 0.
    // get a char* describing the option
    const char* GetOptionName(size_t i);
    // get a char* describing the current option value
    // must be allocated in static or instance memory
    // WILL NOT be freed
    const char* GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    bool OptionChange(size_t i);
    // called when left is pressed
    bool OptionRotateLeft(size_t i);
    // called when right is pressed
    bool OptionRotateRight(size_t i);

protected:
    void SaveConfig_Custom(IniProcessing* ctl);
    void LoadConfig_Custom(IniProcessing* ctl);
};

} // namespace Controls

#endif // TOUCHSCREEN_H
