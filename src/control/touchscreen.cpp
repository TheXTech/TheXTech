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

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "touchscreen.h"
#include "../globals.h"

#include <SDL2/SDL_haptic.h>

#ifdef __ANDROID__
#   include <jni.h>
#   if 1
#       undef JNIEXPORT
#       undef JNICALL
#       define JNIEXPORT extern "C"
#       define JNICALL
#   endif
#endif

namespace Controls
{

/*-----------------*\
|| definitions     ||
\*-----------------*/

enum
{
    KEYBOARD_NOKEYS = 0x00000001,
    KEYBOARD_QWERTY = 0x00000002,
    KEYBOARD_12KEY = 0x00000003
};

enum
{
    TOUCHSCREEN_DISABLE = 0,
    TOUCHSCREEN_DISABLE_ON_KEYBOARD = 1,
    TOUCHSCREEN_ENABLE = 2,
};

enum
{
    TOUCHPAD_STYLE_ACTIONS = 0,
    TOUCHPAD_STYLE_ABXY = 1,
    TOUCHPAD_STYLE_XODA = 2
};

//! Is hardware keyboard presented?
static bool s_showTouchscreenOnStart = true;
static int  s_touchPadStyle = 0;

#ifdef __ANDROID__
static double s_screenSize = 0;
static double s_screenWidth = 0;
static double s_screenHeight = 0;
#endif

static bool     s_vibrationEnable = false;
static float    s_vibrationStrength = 1.0;
static int      s_vibrationLength = 12;

/*-----------------*\
|| Java interface  ||
\*-----------------*/

#ifdef __ANDROID__

static int s_TouchScreenPresence = KEYBOARD_NOKEYS;
static int s_touchscreenMode = TOUCHSCREEN_DISABLE_ON_TouchScreen;

static void initTouchMap();

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setHardwareKeyboardPresence(
    JNIEnv *env,
    jclass type,
    jint keyboard
)
{
    (void)env;
    (void)type;
    s_TouchScreenPresence = keyboard;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setTouchScreenMode(
    JNIEnv *env,
    jclass type,
    jint mode
)
{
    (void)env;
    (void)type;
    s_touchscreenMode = mode;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setTouchScreenShowOnStart(
        JNIEnv *env,
        jclass type,
        jboolean showOnStart
)
{
    (void)env;
    (void)type;
    s_showTouchscreenOnStart = showOnStart;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setScreenSize(
        JNIEnv *env,
        jclass type,
        jdouble screenSize,
        jdouble screenWidth,
        jdouble screenHeight
)
{
    (void)env;
    (void)type;
    s_screenSize = screenSize;
    s_screenWidth = screenWidth;
    s_screenHeight = screenHeight;
    initTouchMap();
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setTouchPadStyle(
        JNIEnv *env,
        jclass type,
        jint style
)
{
    (void)env;
    (void)type;
    s_touchPadStyle = style;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setVibrationEnabled(
    JNIEnv *env,
    jclass type,
    jboolean enableVibration
)
{
    (void)env;
    (void)type;
    s_vibrationEnable = enableVibration;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setVibrationStrength(
    JNIEnv *env,
    jclass type,
    jfloat strength
)
{
    (void)env;
    (void)type;
    s_vibrationStrength = strength;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setVibrationLength(
    JNIEnv *env,
    jclass type,
    jint length
)
{
    (void)env;
    (void)type;
    s_vibrationLength = length;
}
#endif

/*------------------------------------------*\
|| implementation for TouchScreenGFX_t      ||
\*------------------------------------------*/

void TouchScreenGFX_t::loadImage(StdPicture &img, std::string path)
{
    pLogDebug("Loading texture %s...", path.c_str());
    img = frmMain.LoadPicture(path);
    if(!img.texture)
    {
        pLogWarning("Failed to load texture: %s...", path.c_str());
        m_loadErrors++;
    }
    m_loadedImages.push_back(&img);
}

TouchScreenGFX_t::TouchScreenGFX_t()
{
    std::string uiPath = AppPath + "graphics/touchscreen/";

    // Loading a touch-screen buttons from assets
    loadImage(touch[BUTTON_START], uiPath + "Start.png");
    loadImage(touch[BUTTON_LEFT], uiPath + "Left.png");
    loadImage(touch[BUTTON_LEFT_CHAR], uiPath + "Left_char.png");
    loadImage(touch[BUTTON_RIGHT], uiPath + "Right.png");
    loadImage(touch[BUTTON_RIGHT_CHAR], uiPath + "Right_char.png");
    loadImage(touch[BUTTON_UP], uiPath + "Up.png");
    loadImage(touch[BUTTON_DOWN], uiPath + "Down.png");
    loadImage(touch[BUTTON_UPLEFT], uiPath + "UpLeft.png");
    loadImage(touch[BUTTON_UPRIGHT], uiPath + "UpRight.png");
    loadImage(touch[BUTTON_DOWNLEFT], uiPath + "DownLeft.png");
    loadImage(touch[BUTTON_DOWNRIGHT], uiPath + "DownRight.png");
    loadImage(touch[BUTTON_A], uiPath + "A.png");
    loadImage(touch[BUTTON_A_PS], uiPath + "A_ps.png");
    loadImage(touch[BUTTON_A_BLANK], uiPath + "A_blank.png");
    loadImage(touch[BUTTON_A_DO], uiPath + "A_do.png");
    loadImage(touch[BUTTON_A_ENTER], uiPath + "A_enter.png");
    loadImage(touch[BUTTON_A_JUMP], uiPath + "A_jump.png");
    loadImage(touch[BUTTON_B], uiPath + "V.png");
    loadImage(touch[BUTTON_B_PS], uiPath + "V_ps.png");
    loadImage(touch[BUTTON_B_BLANK], uiPath + "V_blank.png");
    loadImage(touch[BUTTON_B_JUMP], uiPath + "V_jump.png");
    loadImage(touch[BUTTON_B_SPINJUMP], uiPath + "V_spinjump.png");
    loadImage(touch[BUTTON_X], uiPath + "X.png");
    loadImage(touch[BUTTON_X_PS], uiPath + "X_ps.png");
    loadImage(touch[BUTTON_X_BACK], uiPath + "X_back.png");
    loadImage(touch[BUTTON_X_BLANK], uiPath + "X_blank.png");
    loadImage(touch[BUTTON_X_BOMB], uiPath + "X_bomb.png");
    loadImage(touch[BUTTON_X_BUMERANG], uiPath + "X_bumerang.png");
    loadImage(touch[BUTTON_X_FIRE], uiPath + "X_fire.png");
    loadImage(touch[BUTTON_X_HAMMER], uiPath + "X_hammer.png");
    loadImage(touch[BUTTON_X_RUN], uiPath + "X_run.png");
    loadImage(touch[BUTTON_X_SWORD], uiPath + "X_sword.png");
    loadImage(touch[BUTTON_Y], uiPath + "Y.png");
    loadImage(touch[BUTTON_Y_PS], uiPath + "Y_ps.png");
    loadImage(touch[BUTTON_Y_BLANK], uiPath + "Y_blank.png");
    loadImage(touch[BUTTON_Y_BOMB], uiPath + "Y_bomb.png");
    loadImage(touch[BUTTON_Y_BUMERANG], uiPath + "Y_bumerang.png");
    loadImage(touch[BUTTON_Y_FIRE], uiPath + "Y_fire.png");
    loadImage(touch[BUTTON_Y_HAMMER], uiPath + "Y_hammer.png");
    loadImage(touch[BUTTON_Y_RUN], uiPath + "Y_run.png");
    loadImage(touch[BUTTON_Y_STATUE], uiPath + "Y_statue.png");
    loadImage(touch[BUTTON_Y_SWORD], uiPath + "Y_sword.png");
    loadImage(touch[BUTTON_DROP], uiPath + "Select.png");
    loadImage(touch[BUTTON_HOLD_RUN_OFF], uiPath + "RunOff.png");
    loadImage(touch[BUTTON_HOLD_RUN_ON], uiPath + "RunOn.png");
    loadImage(touch[BUTTON_VIEW_TOGGLE_OFF], uiPath + "KeysShowOff.png");
    loadImage(touch[BUTTON_VIEW_TOGGLE_ON], uiPath + "KeysShow.png");
    loadImage(touch[BUTTON_ANALOG_BORDER], uiPath + "SBorder.png");
    loadImage(touch[BUTTON_ANALOG_STICK], uiPath + "AStick.png");

    if(m_loadErrors > 0)
    {
        m_success = false;
        return;
    }

    m_success = true;
}

TouchScreenGFX_t::~TouchScreenGFX_t()
{
    for(StdPicture *p : m_loadedImages)
        frmMain.deleteTexture(*p);
    m_loadedImages.clear();
}

/*------------------------------------------*\
|| implementation for TouchScreenController ||
\*------------------------------------------*/

bool TouchScreenController::touchSupported()
{
    if(m_touchDevicesCount <= 0)
        return false;

    if(!m_GFX.m_success)
        return false;

    return true;
}

bool TouchScreenController::touchOn()
{
    for(int dev_i = 0; dev_i < m_touchDevicesCount; dev_i++)
    {
        const SDL_TouchID dev = SDL_GetTouchDevice(dev_i);
        int fingers = SDL_GetNumTouchFingers(dev);
        if(fingers > 0)
            return true;
    }

    return false;
}

TouchScreenController::FingerState::FingerState()
{
    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = false;
        heldKeyPrev[i] = false;
    }
}

TouchScreenController::FingerState::FingerState(const FingerState &fs)
{
    alive = fs.alive;
    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = fs.heldKey[i];
        heldKeyPrev[i] = fs.heldKeyPrev[i];
    }
}

TouchScreenController::FingerState &TouchScreenController::FingerState::operator=(const FingerState &fs)
{
    alive = fs.alive;
    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = fs.heldKey[i];
        heldKeyPrev[i] = fs.heldKeyPrev[i];
    }
    return *this;
}

static int buttonLeft(int player_no)
{
    (void)player_no;
    if(LevelSelect && GamePaused == PauseCode::PauseGame)
        return TouchScreenGFX_t::BUTTON_LEFT_CHAR;
    else
        return TouchScreenGFX_t::BUTTON_LEFT;
}

static int buttonRight(int player_no)
{
    (void)player_no;
    if(LevelSelect && GamePaused == PauseCode::PauseGame)
        return TouchScreenGFX_t::BUTTON_RIGHT_CHAR;
    else
        return TouchScreenGFX_t::BUTTON_RIGHT;
}

static int buttonA(int player_no)
{
    (void)player_no;
    switch(s_touchPadStyle)
    {
    case TOUCHPAD_STYLE_ABXY:
        return TouchScreenGFX_t::BUTTON_A;
    case TOUCHPAD_STYLE_XODA:
        return TouchScreenGFX_t::BUTTON_A_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(GamePaused != PauseCode::None || GameMenu)
            return TouchScreenGFX_t::BUTTON_A_DO;
        else if(GameOutro)
            return TouchScreenGFX_t::BUTTON_A_BLANK;
        else if(LevelSelect)
            return TouchScreenGFX_t::BUTTON_A_ENTER;
        else
            return TouchScreenGFX_t::BUTTON_A_JUMP;
    }
}

static int buttonX(int player_no)
{
    switch(s_touchPadStyle)
    {
    case TOUCHPAD_STYLE_ABXY:
        return TouchScreenGFX_t::BUTTON_X;
    case TOUCHPAD_STYLE_XODA:
        return TouchScreenGFX_t::BUTTON_X_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(GamePaused != PauseCode::None || GameMenu)
            return TouchScreenGFX_t::BUTTON_X_BACK;
        else if(LevelSelect || GameOutro)
            return TouchScreenGFX_t::BUTTON_X_BLANK;
        else
        {
            if(numPlayers >= player_no)
            {
                auto &p = Player[player_no];
                if(p.Character == 5 || p.State == 4 || p.State == 5)
                    return TouchScreenGFX_t::BUTTON_X_SWORD;
                else if(p.State < 3)
                    return TouchScreenGFX_t::BUTTON_X_RUN;
                else if(p.State == 3 || p.State == 7)
                    return TouchScreenGFX_t::BUTTON_X_FIRE;
                else if(p.State == 6)
                {
                    switch(p.Character)
                    {
                    default:
                    case 1:
                    case 2:
                        return TouchScreenGFX_t::BUTTON_X_HAMMER;
                    case 3:
                        return TouchScreenGFX_t::BUTTON_X_BOMB;
                    case 4:
                        return TouchScreenGFX_t::BUTTON_X_BUMERANG;
                    case 5:
                        return TouchScreenGFX_t::BUTTON_X_SWORD;
                    }
                }
            }
            return TouchScreenGFX_t::BUTTON_X_BLANK;
        }
    }
}

static int buttonB(int player_no)
{
    switch(s_touchPadStyle)
    {
    case TOUCHPAD_STYLE_ABXY:
        return TouchScreenGFX_t::BUTTON_B;
    case TOUCHPAD_STYLE_XODA:
        return TouchScreenGFX_t::BUTTON_B_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(LevelSelect || GamePaused != PauseCode::None || GameMenu || GameOutro)
            return TouchScreenGFX_t::BUTTON_B_BLANK;
        else
        {
            if(numPlayers >= player_no)
            {
                auto &p = Player[player_no];
                if(p.Character <= 2 || p.Character == 4)
                    return TouchScreenGFX_t::BUTTON_B_SPINJUMP;
                else
                    return TouchScreenGFX_t::BUTTON_B_JUMP;
            }
            return TouchScreenGFX_t::BUTTON_B_BLANK;
        }
    }
}

static int buttonY(int player_no)
{
    switch(s_touchPadStyle)
    {
    case TOUCHPAD_STYLE_ABXY:
        return TouchScreenGFX_t::BUTTON_Y;
    case TOUCHPAD_STYLE_XODA:
        return TouchScreenGFX_t::BUTTON_Y_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(LevelSelect || GamePaused != PauseCode::None || GameMenu || GameOutro)
            return TouchScreenGFX_t::BUTTON_Y_BLANK;
        else
        {
            if(numPlayers >= player_no)
            {
                auto &p = Player[player_no];
                if(p.State == 5)
                    return TouchScreenGFX_t::BUTTON_Y_STATUE;
                if(p.Character == 5 || p.State == 4)
                    return TouchScreenGFX_t::BUTTON_Y_SWORD;
                else if(p.State < 3)
                    return TouchScreenGFX_t::BUTTON_Y_RUN;
                else if(p.State == 3 || p.State == 7)
                    return TouchScreenGFX_t::BUTTON_Y_FIRE;
                else if(p.State == 6)
                {
                    switch(p.Character)
                    {
                        default:
                        case 1:
                        case 2:
                            return TouchScreenGFX_t::BUTTON_Y_HAMMER;
                        case 3:
                            return TouchScreenGFX_t::BUTTON_Y_BOMB;
                        case 4:
                            return TouchScreenGFX_t::BUTTON_Y_BUMERANG;
                        case 5:
                            return TouchScreenGFX_t::BUTTON_Y_SWORD;
                    }
                }
            }
            return TouchScreenGFX_t::BUTTON_Y_BLANK;
        }
    }
}


static struct TouchKeyMap
{
    struct KeyPos
    {
        float x1;
        float y1;
        float x2;
        float y2;
        TouchScreenController::commands cmd;
    };

    //! Width of canvas area
    float touchCanvasWidth = 1024.0f;
    //! Height of canvas area
    float touchCanvasHeight = 600.0f;

    //! List of key hit boxes
    KeyPos touchKeysMap[TouchScreenController::key_END];

    TouchKeyMap()
    {
        SDL_assert(sizeof(touchKeysMap) == TouchScreenController::key_END * sizeof(KeyPos));
        for(int it = TouchScreenController::key_BEGIN; it < TouchScreenController::key_END; it++)
        {
            auto &p = touchKeysMap[it];
            p.cmd = static_cast<TouchScreenController::commands>(it);
            SDL_assert(p.cmd >= TouchScreenController::key_BEGIN && p.cmd < TouchScreenController::key_END);
        }
    }

    /**
     * \brief Change size of virtual canvas
     * @param width Width of touchable canvas
     * @param height Height of touchable canvas
     */
    void setCanvasSize(float width, float height)
    {
        touchCanvasWidth = width;
        touchCanvasHeight = height;
    }

    /**
     * \brief Change hitbox of key
     * @param cmd Command
     * @param left Left side position
     * @param top Top side position
     * @param right Right side position
     * @param bottom Bottom side position
     */
    void setKeyPos(TouchScreenController::commands cmd, float left, float top, float right, float bottom)
    {
        if(cmd < TouchScreenController::key_BEGIN || cmd >= TouchScreenController::key_END)
            return;

        SDL_assert_release(touchKeysMap[cmd].cmd == cmd);

        auto &key = touchKeysMap[cmd];
        key.x1 = left;
        key.y1 = top;
        key.x2 = right;
        key.y2 = bottom;
    }

    /**
     * \brief Find keys are touched by one finger
     * @param x X position of finger
     * @param y Y position of finger
     * @param keys Destination array to write captured keys
     * @return Count of keys are got touched
     */
    int findTouchKeys(float x, float y, TouchScreenController::FingerState &fs)
    {
//        const size_t touchKeyMapSize = sizeof(touchKeysMap) / sizeof(KeyPos);
        int count = 0;
        x *= touchCanvasWidth;
        y *= touchCanvasHeight;

        for(const auto &p : touchKeysMap)
        {
            SDL_assert(p.cmd >= TouchScreenController::key_BEGIN && p.cmd < TouchScreenController::key_END);
            fs.heldKey[p.cmd] = false;
            if(x >= p.x1 && x <= p.x2 && y >= p.y1 && y <= p.y2)
            {
                fs.heldKey[p.cmd] = true;
                count++;
            }
        }

        return count;
    }

} g_touchKeyMap;

/*-----------------------Screen size depending layouts ----------------------------------*/
/************************************************
 * Please use this tool to edit these layouts:  *
 * https://github.com/Wohlstand/TouchpadTuner   *
 ************************************************/

static const TouchKeyMap::KeyPos c_4_tinyPhoneMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {335.0f, 12.0f, 451.0f, 52.0f, TouchScreenController::key_start},
    {11.0f, 262.0f, 91.0f, 342.0f, TouchScreenController::key_left},
    {171.0f, 262.0f, 251.0f, 342.0f, TouchScreenController::key_right},
    {91.0f, 182.0f, 171.0f, 262.0f, TouchScreenController::key_up},
    {91.0f, 342.0f, 171.0f, 422.0f, TouchScreenController::key_down},
    {11.0f, 182.0f, 91.0f, 262.0f, TouchScreenController::key_upleft},
    {171.0f, 182.0f, 251.0f, 262.0f, TouchScreenController::key_upright},
    {11.0f, 342.0f, 91.0f, 422.0f, TouchScreenController::key_downleft},
    {171.0f, 342.0f, 251.0f, 422.0f, TouchScreenController::key_downright},
    {396.0f, 276.0f, 493.0f, 373.0f, TouchScreenController::key_run},
    {512.0f, 307.0f, 609.0f, 404.0f, TouchScreenController::key_jump},
    {416.0f, 163.0f, 513.0f, 260.0f, TouchScreenController::key_altrun},
    {531.0f, 191.0f, 628.0f, 288.0f, TouchScreenController::key_altjump},
    {196.0f, 12.0f, 312.0f, 52.0f, TouchScreenController::key_drop},
    {494.0f, 50.0f, 618.0f, 97.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 64.0f, 64.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_averagePhoneMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {542.0f, 537.0f, 693.0f, 587.0f, TouchScreenController::key_start},
    {1.0f, 410.0f, 83.0f, 492.0f, TouchScreenController::key_left},
    {165.0f, 410.0f, 247.0f, 492.0f, TouchScreenController::key_right},
    {83.0f, 328.0f, 165.0f, 410.0f, TouchScreenController::key_up},
    {83.0f, 492.0f, 165.0f, 574.0f, TouchScreenController::key_down},
    {1.0f, 328.0f, 83.0f, 410.0f, TouchScreenController::key_upleft},
    {165.0f, 328.0f, 247.0f, 410.0f, TouchScreenController::key_upright},
    {1.0f, 492.0f, 83.0f, 574.0f, TouchScreenController::key_downleft},
    {165.0f, 492.0f, 247.0f, 574.0f, TouchScreenController::key_downright},
    {764.0f, 403.0f, 868.0f, 507.0f, TouchScreenController::key_run},
    {885.0f, 436.0f, 989.0f, 540.0f, TouchScreenController::key_jump},
    {786.0f, 287.0f, 890.0f, 391.0f, TouchScreenController::key_altrun},
    {904.0f, 317.0f, 1008.0f, 421.0f, TouchScreenController::key_altjump},
    {331.0f, 537.0f, 482.0f, 587.0f, TouchScreenController::key_drop},
    {827.0f, 129.0f, 943.0f, 169.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 70.0f, 70.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_averagePhoneLongMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {727.0f, 632.0f, 911.0f, 691.0f, TouchScreenController::key_start},
    {5.0f, 444.0f, 106.0f, 545.0f, TouchScreenController::key_left},
    {207.0f, 444.0f, 308.0f, 545.0f, TouchScreenController::key_right},
    {106.0f, 343.0f, 207.0f, 444.0f, TouchScreenController::key_up},
    {106.0f, 545.0f, 207.0f, 646.0f, TouchScreenController::key_down},
    {5.0f, 343.0f, 106.0f, 444.0f, TouchScreenController::key_upleft},
    {207.0f, 343.0f, 308.0f, 444.0f, TouchScreenController::key_upright},
    {5.0f, 545.0f, 106.0f, 646.0f, TouchScreenController::key_downleft},
    {207.0f, 545.0f, 308.0f, 646.0f, TouchScreenController::key_downright},
    {1055.0f, 500.0f, 1183.0f, 628.0f, TouchScreenController::key_run},
    {1218.0f, 537.0f, 1346.0f, 665.0f, TouchScreenController::key_jump},
    {1085.0f, 351.0f, 1213.0f, 479.0f, TouchScreenController::key_altrun},
    {1242.0f, 389.0f, 1370.0f, 517.0f, TouchScreenController::key_altjump},
    {474.0f, 632.0f, 658.0f, 691.0f, TouchScreenController::key_drop},
    {1178.0f, 214.0f, 1315.0f, 275.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 85.0f, 85.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_7_tablet[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {636.0f, 544.0f, 775.0f, 582.0f, TouchScreenController::key_start},
    {3.0f, 430.0f, 76.0f, 503.0f, TouchScreenController::key_left},
    {149.0f, 430.0f, 222.0f, 503.0f, TouchScreenController::key_right},
    {76.0f, 357.0f, 149.0f, 430.0f, TouchScreenController::key_up},
    {76.0f, 503.0f, 149.0f, 576.0f, TouchScreenController::key_down},
    {3.0f, 357.0f, 76.0f, 430.0f, TouchScreenController::key_upleft},
    {149.0f, 357.0f, 222.0f, 430.0f, TouchScreenController::key_upright},
    {3.0f, 503.0f, 76.0f, 576.0f, TouchScreenController::key_downleft},
    {149.0f, 503.0f, 222.0f, 576.0f, TouchScreenController::key_downright},
    {797.0f, 439.0f, 887.0f, 529.0f, TouchScreenController::key_run},
    {897.0f, 463.0f, 987.0f, 553.0f, TouchScreenController::key_jump},
    {819.0f, 341.0f, 909.0f, 431.0f, TouchScreenController::key_altrun},
    {919.0f, 363.0f, 1009.0f, 453.0f, TouchScreenController::key_altjump},
    {257.0f, 544.0f, 396.0f, 582.0f, TouchScreenController::key_drop},
    {873.0f, 226.0f, 968.0f, 258.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 58.0f, 58.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_10_6_tablet[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {869.0f, 753.0f, 1002.0f, 785.0f, TouchScreenController::key_start},
    {19.0f, 591.0f, 89.0f, 661.0f, TouchScreenController::key_left},
    {159.0f, 591.0f, 229.0f, 661.0f, TouchScreenController::key_right},
    {89.0f, 521.0f, 159.0f, 591.0f, TouchScreenController::key_up},
    {89.0f, 661.0f, 159.0f, 731.0f, TouchScreenController::key_down},
    {19.0f, 521.0f, 89.0f, 591.0f, TouchScreenController::key_upleft},
    {159.0f, 521.0f, 229.0f, 591.0f, TouchScreenController::key_upright},
    {19.0f, 661.0f, 89.0f, 731.0f, TouchScreenController::key_downleft},
    {159.0f, 661.0f, 229.0f, 731.0f, TouchScreenController::key_downright},
    {1047.0f, 588.0f, 1137.0f, 678.0f, TouchScreenController::key_run},
    {1159.0f, 624.0f, 1249.0f, 714.0f, TouchScreenController::key_jump},
    {1067.0f, 477.0f, 1157.0f, 567.0f, TouchScreenController::key_altrun},
    {1179.0f, 505.0f, 1269.0f, 595.0f, TouchScreenController::key_altjump},
    {256.0f, 753.0f, 389.0f, 785.0f, TouchScreenController::key_drop},
    {1131.0f, 387.0f, 1225.0f, 426.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 57.0f, 57.0f, TouchScreenController::key_toggleKeysView},
};

// based on c_10_6_tablet
static const TouchKeyMap::KeyPos c_smallAutoMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {-0.5307881773399015, -0.07266009852216748, -0.3669950738916256, -0.0332512315270936, TouchScreenController::key_start},
    {0.023399014778325122, -0.27216748768472904, 0.10960591133004927, -0.18596059113300492, TouchScreenController::key_left},
    {0.1958128078817734, -0.27216748768472904, 0.28201970443349755, -0.18596059113300492, TouchScreenController::key_right},
    {0.10960591133004927, -0.3583743842364532, 0.1958128078817734, -0.27216748768472904, TouchScreenController::key_up},
    {0.10960591133004927, -0.18596059113300492, 0.1958128078817734, -0.09975369458128079, TouchScreenController::key_down},
    {0.023399014778325122, -0.3583743842364532, 0.10960591133004927, -0.27216748768472904, TouchScreenController::key_upleft},
    {0.1958128078817734, -0.3583743842364532, 0.28201970443349755, -0.27216748768472904, TouchScreenController::key_upright},
    {0.023399014778325122, -0.18596059113300492, 0.10960591133004927, -0.09975369458128079, TouchScreenController::key_downleft},
    {0.1958128078817734, -0.18596059113300492, 0.28201970443349755, -0.09975369458128079, TouchScreenController::key_downright},
    {-0.31157635467980294, -0.27586206896551724, -0.20073891625615764, -0.16502463054187191, TouchScreenController::key_run},
    {-0.17364532019704434, -0.2315270935960591, -0.06280788177339902, -0.1206896551724138, TouchScreenController::key_jump},
    {-0.2869458128078818, -0.4125615763546798, -0.17610837438423646, -0.3017241379310345, TouchScreenController::key_altrun},
    {-0.14901477832512317, -0.37807881773399016, -0.038177339901477834, -0.2672413793103448, TouchScreenController::key_altjump},
    {0.31527093596059114, -0.07266009852216748, 0.479064039408867, -0.0332512315270936, TouchScreenController::key_drop},
    {-0.20812807881773399, -0.5233990147783252, -0.09236453201970443, -0.4753694581280788, TouchScreenController::key_holdRun},
    {0.012315270935960592, 0.012315270935960592, 0.07019704433497537, 0.07019704433497537, TouchScreenController::key_toggleKeysView},
};

// based on c_averagePhoneMap
static const TouchKeyMap::KeyPos c_mediumAutoMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {0.75, -0.105, 1.0, -0.021666666666666667, TouchScreenController::key_start},
    {0.0016666666666666668, -0.31666666666666665, 0.13833333333333334, -0.18, TouchScreenController::key_left},
    {0.275, -0.31666666666666665, 0.4116666666666667, -0.18, TouchScreenController::key_right},
    {0.13833333333333334, -0.4533333333333333, 0.275, -0.31666666666666665, TouchScreenController::key_up},
    {0.13833333333333334, -0.18, 0.275, -0.043333333333333335, TouchScreenController::key_down},
    {0.0016666666666666668, -0.4533333333333333, 0.13833333333333334, -0.31666666666666665, TouchScreenController::key_upleft},
    {0.275, -0.4533333333333333, 0.4116666666666667, -0.31666666666666665, TouchScreenController::key_upright},
    {0.0016666666666666668, -0.18, 0.13833333333333334, -0.043333333333333335, TouchScreenController::key_downleft},
    {0.275, -0.18, 0.4116666666666667, -0.043333333333333335, TouchScreenController::key_downright},
    {-0.43333333333333335, -0.3283333333333333, -0.26, -0.155, TouchScreenController::key_run},
    {-0.23166666666666666, -0.2733333333333333, -0.058333333333333334, -0.1, TouchScreenController::key_jump},
    {-0.39666666666666667, -0.5216666666666666, -0.22333333333333333, -0.34833333333333333, TouchScreenController::key_altrun},
    {-0.2, -0.4716666666666667, -0.02666666666666667, -0.29833333333333334, TouchScreenController::key_altjump},
    {0.45, -0.105, 0.7, -0.021666666666666667, TouchScreenController::key_drop},
    {-0.3283333333333333, -0.785, -0.135, -0.7183333333333333, TouchScreenController::key_holdRun},
    {0.016666666666666666, 0.016666666666666666, 0.11666666666666667, 0.11666666666666667, TouchScreenController::key_toggleKeysView},
};

// based on c_4_tinyPhoneMap
static const TouchKeyMap::KeyPos c_largeAutoMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {-0.5, 0.025, -0.25, 0.10, TouchScreenController::key_start},
    {0.022916666666666665, -0.45416666666666666, 0.18958333333333333, -0.2875, TouchScreenController::key_left},
    {0.35625, -0.45416666666666666, 0.5229166666666667, -0.2875, TouchScreenController::key_right},
    {0.18958333333333333, -0.6208333333333333, 0.35625, -0.45416666666666666, TouchScreenController::key_up},
    {0.18958333333333333, -0.2875, 0.35625, -0.12083333333333333, TouchScreenController::key_down},
    {0.022916666666666665, -0.6208333333333333, 0.18958333333333333, -0.45416666666666666, TouchScreenController::key_upleft},
    {0.35625, -0.6208333333333333, 0.5229166666666667, -0.45416666666666666, TouchScreenController::key_upright},
    {0.022916666666666665, -0.2875, 0.18958333333333333, -0.12083333333333333, TouchScreenController::key_downleft},
    {0.35625, -0.2875, 0.5229166666666667, -0.12083333333333333, TouchScreenController::key_downright},
    {-0.5083333333333333, -0.425, -0.30625, -0.22291666666666668, TouchScreenController::key_run},
    {-0.26666666666666666, -0.36041666666666666, -0.06458333333333334, -0.15833333333333333, TouchScreenController::key_jump},
    {-0.4666666666666667, -0.6604166666666667, -0.26458333333333334, -0.4583333333333333, TouchScreenController::key_altrun},
    {-0.22708333333333333, -0.6020833333333333, -0.025, -0.4, TouchScreenController::key_altjump},
    {0.25, 0.025, 0.5, 0.10, TouchScreenController::key_drop},
    {-0.20, 0.125, -0.02608695652173914, 0.20, TouchScreenController::key_holdRun},
    {0.020833333333333332, 0.020833333333333332, 0.13333333333333333, 0.13333333333333333, TouchScreenController::key_toggleKeysView},
};
/*---------------------------------------------------------------------------------------*/

#if 0
static void oldTouchMap()
{
    if(s_screenSize >= 9.0) // Big tablets
    {
        g_touchKeyMap.touchCanvasWidth = 1300.0f;
        g_touchKeyMap.touchCanvasHeight = 812.0f;
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_10_6_tablet, sizeof(g_touchKeyMap.touchKeysMap));
    }
    else if(s_screenSize >= 7.0) // Middle tablets
    {
        g_touchKeyMap.touchCanvasWidth = 1024.0f;
        g_touchKeyMap.touchCanvasHeight = 600.0f;
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_7_tablet, sizeof(g_touchKeyMap.touchKeysMap));
    }
    else if(s_screenSize < 4.0) // Very small phones
    {
        g_touchKeyMap.touchCanvasWidth = 640.0f;
        g_touchKeyMap.touchCanvasHeight = 480.0f;
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_4_tinyPhoneMap, sizeof(g_touchKeyMap.touchKeysMap));
    }
    else // All other devices
    {
        // Longer screens (big ratio between sides, more like a stick)
        if((s_screenWidth / s_screenHeight) > 1.6f)
        {
            g_touchKeyMap.touchCanvasWidth = 1396.0f;
            g_touchKeyMap.touchCanvasHeight = 720.0f;
            SDL_memcpy(g_touchKeyMap.touchKeysMap, c_averagePhoneLongMap, sizeof(g_touchKeyMap.touchKeysMap));
        }
        else // Shorter screens (smaller ratio between sides, more like a square)
        {
            g_touchKeyMap.touchCanvasWidth = 1024.0f;
            g_touchKeyMap.touchCanvasHeight = 600.0f;
            SDL_memcpy(g_touchKeyMap.touchKeysMap, c_averagePhoneMap, sizeof(g_touchKeyMap.touchKeysMap));
        }
    }
}
#endif

static void updateTouchMap(int preferredSize, float screenWidth, float screenHeight)
{
    if(screenWidth > screenHeight)
    {
        g_touchKeyMap.touchCanvasWidth = screenWidth / screenHeight;
        g_touchKeyMap.touchCanvasHeight = 1.f;
    }
    else
    {
        g_touchKeyMap.touchCanvasWidth = 1.f;
        g_touchKeyMap.touchCanvasHeight = screenHeight / screenWidth;
    }
    if(preferredSize == TouchScreenController::size_small)
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_smallAutoMap, sizeof(g_touchKeyMap.touchKeysMap));
    else if(preferredSize == TouchScreenController::size_medium)
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_mediumAutoMap, sizeof(g_touchKeyMap.touchKeysMap));
    else
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_largeAutoMap, sizeof(g_touchKeyMap.touchKeysMap));
    for(int i = 0; i < TouchScreenController::key_END; i++)
    {
        if(g_touchKeyMap.touchKeysMap[i].x1 < 0)
        {
            g_touchKeyMap.touchKeysMap[i].x1 += g_touchKeyMap.touchCanvasWidth;
        }
        if(g_touchKeyMap.touchKeysMap[i].x2 < 0)
        {
            g_touchKeyMap.touchKeysMap[i].x2 += g_touchKeyMap.touchCanvasWidth;
        }
        if(g_touchKeyMap.touchKeysMap[i].y1 < 0)
        {
            g_touchKeyMap.touchKeysMap[i].y1 += g_touchKeyMap.touchCanvasHeight;
        }
        if(g_touchKeyMap.touchKeysMap[i].y2 < 0)
        {
            g_touchKeyMap.touchKeysMap[i].y2 += g_touchKeyMap.touchCanvasHeight;
        }
    }
}


void TouchScreenController::doVibration()
{
    if(!s_vibrationEnable || !m_vibrator)
        return;

    SDL_HapticRumblePlay(m_vibrator, s_vibrationStrength, s_vibrationLength);
    D_pLogDebug("TouchScreen: Vibration %g, %d ms", s_vibrationStrength, s_vibrationLength);
}

TouchScreenController::~TouchScreenController()
{
    if(m_vibrator)
        SDL_HapticClose(m_vibrator);
    m_vibrator = nullptr;
}

TouchScreenController::TouchScreenController()
{
    updateScreenSize();

    for(int key = key_BEGIN; key < key_END; ++key)
        m_keysHeld[key] = false;
    m_touchHidden = !s_showTouchscreenOnStart;
    pLogDebug("Initialization of touch-screen controller...");
    m_touchDevicesCount = SDL_GetNumTouchDevices();
    pLogDebug("Found %d touch devices, screen size: %d x %d",
                m_touchDevicesCount,
                m_screenWidth, m_screenHeight);
#ifdef __ANDROID__
    pLogDebug("The Android reported screen size: %g inches (%g x %g)", s_screenSize, s_screenWidth, s_screenHeight);
#endif

    m_vibrator = nullptr;
    int numHaptics = SDL_NumHaptics();

    for(int i = 0; i < numHaptics; ++i)
    {
        // not sure what or why...
        if(SDL_strcmp(SDL_HapticName(i), "VIBRATOR_SERVICE") == 0)
        {
            m_vibrator = SDL_HapticOpen(i);
            if(m_vibrator)
            {
                pLogDebug("TouchScreen: Opened the vibrator service");
                SDL_HapticRumbleInit(m_vibrator);
            }
            else
                pLogWarning("TouchScreen: Can't open the vibrator service");
            break;
        }
    }
}

void TouchScreenController::updateScreenSize()
{
    SDL_GetWindowSize(frmMain.getWindow(), &m_screenWidth, &m_screenHeight);
    updateTouchMap(m_preferredSize, m_screenWidth, m_screenHeight);
}

static void updateKeyValue(bool &key, bool state)
{
    key = state;
    D_pLogDebug("= Touch key: State=%d", (int)key);
}

static void updateFingerKeyState(TouchScreenController::FingerState &st,
        Controls_t &keys, int keyCommand, bool setState, TouchScreenController::ExtraKeys_t &extraSt)
{
    st.alive = (setState != 0);
    if(keyCommand >= static_cast<int>(TouchScreenController::key_BEGIN) && keyCommand < static_cast<int>(TouchScreenController::key_END))
    {
        switch(keyCommand)
        {
            case TouchScreenController::key_left:
                updateKeyValue(keys.Left, setState);
                break;
            case TouchScreenController::key_right:
                updateKeyValue(keys.Right, setState);
                break;
            case TouchScreenController::key_up:
                updateKeyValue(keys.Up, setState);
                break;
            case TouchScreenController::key_down:
                updateKeyValue(keys.Down, setState);
                break;
            case TouchScreenController::key_upleft:
                updateKeyValue(keys.Up, setState);
                updateKeyValue(keys.Left, setState);
                break;
            case TouchScreenController::key_upright:
                updateKeyValue(keys.Up, setState);
                updateKeyValue(keys.Right, setState);
                break;
            case TouchScreenController::key_downleft:
                updateKeyValue(keys.Down, setState);
                updateKeyValue(keys.Left, setState);
                break;
            case TouchScreenController::key_downright:
                updateKeyValue(keys.Down, setState);
                updateKeyValue(keys.Right, setState);
                break;
            case TouchScreenController::key_jump:
                updateKeyValue(keys.Jump, setState);
                break;
            case TouchScreenController::key_altjump:
                updateKeyValue(keys.AltJump, setState);
                break;
            case TouchScreenController::key_run:
                extraSt.keyRunOnce = (setState & !keys.Run);
                updateKeyValue(keys.Run, setState);
                break;
            case TouchScreenController::key_altrun:
                extraSt.keyAltRunOnce = (setState & !keys.AltRun);
                updateKeyValue(keys.AltRun, setState);
                break;
            case TouchScreenController::key_drop:
                updateKeyValue(keys.Drop, setState);
                break;
            case TouchScreenController::key_start:
                updateKeyValue(keys.Start, setState);
                break;
            case TouchScreenController::key_toggleKeysView:
                extraSt.keyToggleViewOnce = (setState & !extraSt.keyToggleView);
                extraSt.keyToggleView = setState;
                break;
            case TouchScreenController::key_holdRun:
                extraSt.keyHoldRunOnce = (setState & !extraSt.keyHoldRun);
                extraSt.keyHoldRun = setState;
                break;
            default:
                break;
        }
    }
    else
    {
        st.alive = false;
    }
}

void TouchScreenController::processTouchDevice(int dev_i)
{
    const SDL_TouchID dev = SDL_GetTouchDevice(dev_i);

    int fingers = SDL_GetNumTouchFingers(dev);

    for(auto &m_finger : m_fingers)
    {
        // Mark as "dead"
        m_finger.second.alive = false;
    }

    for(int i = 0; i < fingers; i++)
    {
        SDL_Finger *f = SDL_GetTouchFinger(dev, i);
        if (!f || (f->id < 0)) //Skip a wrong finger
            continue;

        SDL_FingerID finger_id = f->id;
        float finger_x = f->x, finger_y = f->y, finger_pressure = f->pressure;
        (void)finger_pressure;

        if(!m_cursorHeld)
        {
            m_cursorHeld = true;
            SDL_Point p = frmMain.MapToScr(finger_x*m_screenWidth, finger_y*m_screenHeight);
            m_cursorX = p.x;
            m_cursorY = p.y;
        }

        auto found = m_fingers.find(finger_id);
        if(found != m_fingers.end())
        {
            FingerState &fs = found->second;
            int keysCount = g_touchKeyMap.findTouchKeys(finger_x, finger_y, fs);
            for(int key = key_BEGIN; key < key_END; ++key)
            {
                if(fs.ignore)
                    break;
                if(m_touchHidden && key != key_toggleKeysView)
                    key = key_toggleKeysView;
                if(fs.heldKeyPrev[key] && !fs.heldKey[key]) // set key off
                {
                    updateFingerKeyState(fs, m_current_keys, key, false, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d released (move)", static_cast<int>(key));
                    m_keysHeld[key] = false;
                    fs.heldKeyPrev[key] = fs.heldKey[key];
                }
                else if(fs.heldKey[key]) // set key on and keep alive
                {
                    if(!fs.heldKeyPrev[key] && fs.heldKey[key])
                        doVibration(); // Vibrate when key gets on
                    updateFingerKeyState(fs, m_current_keys, key, true, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d pressed (move)", static_cast<int>(key));
                    m_keysHeld[key] = true;
                    fs.heldKeyPrev[key] = fs.heldKey[key];
                }
            }
            fs.alive = (keysCount > 0);
        }
        else
        {
            // Detect which key is pressed, and press it
            FingerState st;
            int keysCount = g_touchKeyMap.findTouchKeys(finger_x, finger_y, st);
            for(int key = key_BEGIN; key < key_END; key++)
            {
                if(m_touchHidden && key != key_toggleKeysView)
                    key = key_toggleKeysView;
                if(st.heldKey[key]) // set key on
                {
                    updateFingerKeyState(st, m_current_keys, key, true, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d pressed (put)", static_cast<int>(key));
                    doVibration();
                    m_keysHeld[key] = true;
                    st.heldKeyPrev[key] = st.heldKey[key];
                    // Also: when more than one touch devices found, choose one which is actual
                    // Otherwise, the spam of on/off events will happen
                    if(m_actualDevice < 0)
                        m_actualDevice = dev_i;
                }
            }

            st.alive = (keysCount > 0);
            if(st.alive)
            {
                D_pLogDebug("= Finger ID=%d came", static_cast<int>(finger_id));
                m_fingers.insert({finger_id, st});
            }
        }

        D_pLogDebug("= Finger press: ID=%d, X=%.04f, Y=%.04f, P=%.04f",
                    static_cast<int>(finger_id), finger_x, finger_y, finger_pressure);
    }

    for(auto it = m_fingers.begin(); it != m_fingers.end();)
    {
        if(!it->second.alive)
        {
            for(int key = key_BEGIN; key < key_END; key++)
            {
                if(it->second.heldKey[key]) // Key was previously held
                {
                    updateFingerKeyState(it->second, m_current_keys, key, false, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d released (take)", static_cast<int>(key));
                    m_keysHeld[key] = false;
                }
            }
            D_pLogDebug("= Finger ID=%d has gone", static_cast<int>(it->first));

            it = m_fingers.erase(it);
            continue;
        }
        it++;
    }

    if(m_current_extra_keys.keyToggleViewOnce)
        m_touchHidden = !m_touchHidden;

    if(m_current_extra_keys.keyHoldRunOnce)
        m_runHeld = !m_runHeld;
}

void TouchScreenController::update()
{
    if(!touchSupported())
        return;

    m_cursorHeld = false;

    // If actually used in the game touch was found, use it
    if(m_actualDevice >= 0)
    {
        processTouchDevice(m_actualDevice);
        return;
    }
    // Otherwise, find it
    for(int dev_i = 0; dev_i < m_touchDevicesCount; dev_i++)
    {
        processTouchDevice(dev_i);
        if(m_actualDevice >= 0)
            break;
    }
}

void TouchScreenController::render(int player_no)
{
    if(!touchSupported())
        return;

    for(int key = key_BEGIN; key < key_END; key++)
    {
        if((m_touchHidden && key != TouchScreenController::key_toggleKeysView) || LoadingInProcess || GamePaused == PauseCode::TextEntry)
            continue;
        const auto &k = g_touchKeyMap.touchKeysMap[key];
        int x1 = Maths::iRound((k.x1 / g_touchKeyMap.touchCanvasWidth) * float(m_screenWidth));
        int y1 = Maths::iRound((k.y1 / g_touchKeyMap.touchCanvasHeight) * float(m_screenHeight));
        int x2 = Maths::iRound((k.x2 / g_touchKeyMap.touchCanvasWidth) * float(m_screenWidth));
        int y2 = Maths::iRound((k.y2 / g_touchKeyMap.touchCanvasHeight) * float(m_screenHeight));
        int w = x2 - x1;
        int h = y2 - y1;
        float r = 1.0f;
        float g = 0.0f;

//        if(key == key_holdRun && m_runHeld)
//        {
//            r = 0.f;
//            g = 1.f;
//        }

        float a = m_keysHeld[key] ? 0.9f : 0.3f;

        switch(key)
        {
        case TouchScreenController::key_toggleKeysView:
            frmMain.renderTextureScale(x1, y1, w, h,
                                       m_GFX.touch[m_touchHidden ? TouchScreenGFX_t::BUTTON_VIEW_TOGGLE_OFF : TouchScreenGFX_t::BUTTON_VIEW_TOGGLE_ON],
                                       1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_start:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_START], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_drop:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DROP], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_up:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_UP], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_left:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonLeft(player_no)], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_right:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonRight(player_no)], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_down:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DOWN], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_upleft:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_UPLEFT], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_upright:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_UPRIGHT], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_downleft:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DOWNLEFT], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_downright:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DOWNRIGHT], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_holdRun:
            frmMain.renderTextureScale(x1, y1, w, h,
                                       m_GFX.touch[m_runHeld ? TouchScreenGFX_t::BUTTON_HOLD_RUN_ON : TouchScreenGFX_t::BUTTON_HOLD_RUN_OFF],
                                       1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_jump:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonA(player_no)], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_run:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonX(player_no)], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_altjump:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonB(player_no)], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_altrun:
            frmMain.renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonY(player_no)], 1.f, 1.f, 1.f, a);
            break;
        default:
            frmMain.renderRect(x1, y1, w, h, r, g, 0.f, 0.3f);
            break;
        }
    }
}

void TouchScreenController::resetState()
{
    this->update();
    for(std::pair<const long int, Controls::TouchScreenController::FingerState>& state : m_fingers)
    {
        state.second.ignore = true;
    }
    m_current_keys = Controls_t();
    m_current_extra_keys = ExtraKeys_t();
    for(int i = 0; i < key_END; i++)
    {
        m_keysHeld[i] = false;
    }
}

/*====================================================*\
|| implementation for InputMethod_TouchScreen         ||
\*====================================================*/

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_TouchScreen::Update(Controls_t& c, CursorControls_t& m, EditorControls_t& e)
{
    InputMethodType_TouchScreen* t = dynamic_cast<InputMethodType_TouchScreen*>(this->Type);
    if(!t)
        return false;

    if(!t->m_controller.touchSupported())
        return false;

    c = t->m_controller.m_current_keys;

    TouchScreenController::ExtraKeys_t& te = t->m_controller.m_current_extra_keys;
    // This run lock logic has been modified to work as run invert code instead.
    // The original code always delayed running by one frame,
    //   even when run lock was not enabled.
    if(GamePaused == PauseCode::None && !GameMenu && !GameOutro && !LevelSelect)
    {
        // Alt Run functions as normal
        if(t->m_controller.m_runHeld && c.AltRun)
        {
            // make sure Alt Run activates properly
            if(te.keyAltRunOnce)
                c.AltRun = false;
            c.Run = false;
        }
        // Run is inverted
        else if(t->m_controller.m_runHeld)
            c.Run = !c.Run;
    }

    // use the touchscreen as a mouse if the buttons are currently hidden or we are in TextEntry mode
    bool allowed = t->m_controller.m_touchHidden || GamePaused == PauseCode::TextEntry;
    if(allowed && t->m_controller.m_cursorHeld)
    {
        m.Primary = true;
        if(t->m_controller.m_cursorX - m.X <= 1 || t->m_controller.m_cursorX - m.X >= 1
            || t->m_controller.m_cursorY - m.Y <= 1 || t->m_controller.m_cursorY - m.Y >= 1)
        {
            m.Move = true;
            m.X = t->m_controller.m_cursorX;
            m.Y = t->m_controller.m_cursorY;
        }
    }

    // TODO: beautiful editor controls :)

    return true;
}

void InputMethod_TouchScreen::Rumble(int ms, float strength)
{
    (void)ms;
    (void)strength;
    InputMethodType_TouchScreen* t = dynamic_cast<InputMethodType_TouchScreen*>(this->Type);
    if(!t)
        return;

    if(!t->m_controller.touchSupported())
        return;

    if(!t->m_controller.m_vibrator)
        return;

    pLogDebug("Trying to use SDL haptic rumble: %dms %f", ms, strength);
    if(SDL_HapticRumblePlay(t->m_controller.m_vibrator, strength, ms) == 0)
        return;
}

/*====================================================*\
|| implementation for InputMethodProfile_TouchScreen  ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_TouchScreen::InputMethodProfile_TouchScreen()
{
}

bool InputMethodProfile_TouchScreen::PollPrimaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

bool InputMethodProfile_TouchScreen::PollSecondaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

bool InputMethodProfile_TouchScreen::DeletePrimaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

bool InputMethodProfile_TouchScreen::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

const char* InputMethodProfile_TouchScreen::NamePrimaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return "(TOUCH)";
}

const char* InputMethodProfile_TouchScreen::NameSecondaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return "";
}

void InputMethodProfile_TouchScreen::SaveConfig(IniProcessing* ctl)
{
    (void)ctl;
}

void InputMethodProfile_TouchScreen::LoadConfig(IniProcessing* ctl)
{
    (void)ctl;
}

/*====================================================*\
|| implementation for InputMethodType_TouchScreen     ||
\*====================================================*/

InputMethodProfile* InputMethodType_TouchScreen::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_TouchScreen;
}

InputMethodType_TouchScreen::InputMethodType_TouchScreen()
{
    this->Name = "Touchscreen";
}

void InputMethodType_TouchScreen::UpdateControlsPre()
{
    this->m_controller.update();
}

void InputMethodType_TouchScreen::UpdateControlsPost()
{
}

InputMethod* InputMethodType_TouchScreen::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    int n_touchscreens = 0;
    for(InputMethod* method : active_methods)
    {
        if(!method)
            continue;
        InputMethod_TouchScreen* m = dynamic_cast<InputMethod_TouchScreen*>(method);
        if(m)
        {
            n_touchscreens ++;
        }
    }

    if(n_touchscreens > 0)
    {
        this->m_canPoll = false;
        return nullptr;
    }

    if(!this->m_controller.touchSupported())
    {
        this->m_canPoll = false;
        return nullptr;
    }

    // if didn't find any touch, allow poll in future but return nullptr
    if(!this->m_controller.touchOn())
    {
        this->m_canPoll = true;
        return nullptr;
    }

    // if poll not allowed, return false
    if(!this->m_canPoll)
        return nullptr;

    // we're going to create a touchscreen method!
    // reset canPoll for next time
    this->m_canPoll = false;

    InputMethod_TouchScreen* method = new(std::nothrow) InputMethod_TouchScreen;

    if(!method)
        return nullptr;

    if(!g_renderTouchscreen)
        this->m_controller.resetState();

    method->Name = "Touchscreen";
    method->Type = this;

    return (InputMethod*)method;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/

// How many per-type special options are there?
size_t InputMethodType_TouchScreen::GetSpecialOptionCount()
{
    return 1;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_TouchScreen::GetOptionName(size_t i)
{
    if(i == 0)
    {
        return "INTERFACE SIZE";
    }
    return nullptr;
}
// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodType_TouchScreen::GetOptionValue(size_t i)
{
    if(i == 0)
    {
        if(this->m_controller.m_preferredSize == TouchScreenController::size_small)
            return "SMALL";
        else if(this->m_controller.m_preferredSize == TouchScreenController::size_medium)
            return "MEDIUM";
        else
            return "LARGE";
    }
    return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_TouchScreen::OptionChange(size_t i)
{
    if(i == 0)
    {
        this->OptionRotateRight(i);
    }
    return false;
}
// called when left is pressed
bool InputMethodType_TouchScreen::OptionRotateLeft(size_t i)
{
    if(i == 0)
    {
        if(this->m_controller.m_preferredSize > 0)
            this->m_controller.m_preferredSize --;
        else
            this->m_controller.m_preferredSize = TouchScreenController::size_END - 1;
        this->m_controller.updateScreenSize();
        return true;
    }
    return false;
}
// called when right is pressed
bool InputMethodType_TouchScreen::OptionRotateRight(size_t i)
{
    if(i == 0)
    {
        this->m_controller.m_preferredSize ++;
        if(this->m_controller.m_preferredSize >= TouchScreenController::size_END)
            this->m_controller.m_preferredSize = 0;
        this->m_controller.updateScreenSize();
        return true;
    }
    return false;
}

void InputMethodType_TouchScreen::SaveConfig_Custom(IniProcessing* ctl)
{
    ctl->setValue("ui-size", this->m_controller.m_preferredSize);
}

void InputMethodType_TouchScreen::LoadConfig_Custom(IniProcessing* ctl)
{
    ctl->read("ui-size", this->m_controller.m_preferredSize, TouchScreenController::size_medium);
    this->m_controller.updateScreenSize();
}

} // namespace Controls
