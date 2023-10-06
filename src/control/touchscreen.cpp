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

#include <SDL2/SDL.h>

#include <Logger/logger.h>
#include <Utils/maths.h>
#ifdef __ANDROID__
#   include <Utils/files.h>
#endif

#include "touchscreen.h"
#include "../globals.h"
// #include "config.h" // Not used
#include "../game_main.h"
#include "../player.h"
#include "../core/render.h"
#include "../core/window.h"

#include "main/menu_main.h"
#include "control/controls_strings.h"

#include "core/power.h"

#include "editor/new_editor.h"

#include <SDL2/SDL_haptic.h>


#ifdef __ANDROID__
#   include <jni.h>
#   if 1
#       undef JNIEXPORT
#       undef JNICALL
#       define JNIEXPORT extern "C"
#       define JNICALL
#   endif

static double s_screenSize = -1;
static double s_screenWidth = -1;
static double s_screenHeight = -1;

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setScreenSize(
    JNIEnv* env,
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
}

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_thextechDebugLog(
    JNIEnv* env,
    jclass clazz,
    jstring line_j)
{
    const char *line;
    (void)clazz;
    line = env->GetStringUTFChars(line_j, nullptr);
    pLogDebug("Java-Side: %s", line);
    env->ReleaseStringUTFChars(line_j, line);
}

#endif

namespace Controls
{

/*------------------------------------------*\
|| implementation for TouchScreenGFX_t      ||
\*------------------------------------------*/

void TouchScreenGFX_t::loadImage(StdPicture& img, const std::string& fileName)
{
    std::string imgPath = m_gfxPath + fileName;

#ifdef __ANDROID__
    if(!Files::fileExists(imgPath)) // If not exists at assets, do load bundled
        imgPath = "buttons/" + fileName;
#endif

    pLogDebug("Loading texture %s...", imgPath.c_str());
    XRender::LoadPicture(img, imgPath);

    if(!img.inited)
    {
        pLogWarning("Failed to load texture: %s...", imgPath.c_str());
        m_loadErrors++;
    }
}

TouchScreenGFX_t::TouchScreenGFX_t()
{
    m_gfxPath = AppPath + "graphics/touchscreen/";

    // Loading a touch-screen buttons from assets
    loadImage(touch[BUTTON_START], "Start.png");
    loadImage(touch[BUTTON_LEFT], "Left.png");
    loadImage(touch[BUTTON_LEFT_CHAR], "Left_char.png");
    loadImage(touch[BUTTON_RIGHT], "Right.png");
    loadImage(touch[BUTTON_RIGHT_CHAR], "Right_char.png");
    loadImage(touch[BUTTON_UP], "Up.png");
    loadImage(touch[BUTTON_DOWN], "Down.png");
    loadImage(touch[BUTTON_UPLEFT], "UpLeft.png");
    loadImage(touch[BUTTON_UPRIGHT], "UpRight.png");
    loadImage(touch[BUTTON_DOWNLEFT], "DownLeft.png");
    loadImage(touch[BUTTON_DOWNRIGHT], "DownRight.png");
    loadImage(touch[BUTTON_A], "A.png");
    loadImage(touch[BUTTON_A_PS], "A_ps.png");
    loadImage(touch[BUTTON_A_BLANK], "A_blank.png");
    loadImage(touch[BUTTON_A_DO], "A_do.png");
    loadImage(touch[BUTTON_A_ENTER], "A_enter.png");
    loadImage(touch[BUTTON_A_JUMP], "A_jump.png");
    loadImage(touch[BUTTON_B], "V.png");
    loadImage(touch[BUTTON_B_PS], "V_ps.png");
    loadImage(touch[BUTTON_B_BLANK], "V_blank.png");
    loadImage(touch[BUTTON_B_JUMP], "V_jump.png");
    loadImage(touch[BUTTON_B_SPINJUMP], "V_spinjump.png");
    loadImage(touch[BUTTON_X], "X.png");
    loadImage(touch[BUTTON_X_PS], "X_ps.png");
    loadImage(touch[BUTTON_X_BACK], "X_back.png");
    loadImage(touch[BUTTON_X_BLANK], "X_blank.png");
    loadImage(touch[BUTTON_X_BOMB], "X_bomb.png");
    loadImage(touch[BUTTON_X_BUMERANG], "X_bumerang.png");
    loadImage(touch[BUTTON_X_FIRE], "X_fire.png");
    loadImage(touch[BUTTON_X_HAMMER], "X_hammer.png");
    loadImage(touch[BUTTON_X_RUN], "X_run.png");
    loadImage(touch[BUTTON_X_SWORD], "X_sword.png");
    loadImage(touch[BUTTON_Y], "Y.png");
    loadImage(touch[BUTTON_Y_PS], "Y_ps.png");
    loadImage(touch[BUTTON_Y_BLANK], "Y_blank.png");
    loadImage(touch[BUTTON_Y_BOMB], "Y_bomb.png");
    loadImage(touch[BUTTON_Y_BUMERANG], "Y_bumerang.png");
    loadImage(touch[BUTTON_Y_FIRE], "Y_fire.png");
    loadImage(touch[BUTTON_Y_HAMMER], "Y_hammer.png");
    loadImage(touch[BUTTON_Y_RUN], "Y_run.png");
    loadImage(touch[BUTTON_Y_STATUE], "Y_statue.png");
    loadImage(touch[BUTTON_Y_SWORD], "Y_sword.png");
    loadImage(touch[BUTTON_DROP], "Select.png");
    loadImage(touch[BUTTON_HOLD_RUN_OFF], "RunOff.png");
    loadImage(touch[BUTTON_HOLD_RUN_ON], "RunOn.png");
    loadImage(touch[BUTTON_VIEW_TOGGLE_OFF], "KeysShowOff.png");
    loadImage(touch[BUTTON_VIEW_TOGGLE_ON], "KeysShow.png");
    loadImage(touch[BUTTON_ANALOG_BORDER], "SBorder.png");
    loadImage(touch[BUTTON_ANALOG_STICK], "AStick.png");
    loadImage(touch[BUTTON_ENTER_CHEATS], "EnterCheats.png");

    if(m_loadErrors > 0)
    {
        m_success = false;
        return;
    }

    m_success = true;
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

int TouchScreenController::numDevices() const
{
    return m_touchDevicesCount;
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

TouchScreenController::FingerState::FingerState(const FingerState& fs)
{
    alive = fs.alive;

    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = fs.heldKey[i];
        heldKeyPrev[i] = fs.heldKeyPrev[i];
    }
}

TouchScreenController::FingerState& TouchScreenController::FingerState::operator=(const FingerState& fs)
{
    alive = fs.alive;

    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = fs.heldKey[i];
        heldKeyPrev[i] = fs.heldKeyPrev[i];
    }

    return *this;
}

static int buttonLeft(int player_no, int style)
{
    (void)player_no;
    (void)style;

    if(SwapCharAllowed() && GamePaused == PauseCode::PauseScreen)
        return TouchScreenGFX_t::BUTTON_LEFT_CHAR;
    else
        return TouchScreenGFX_t::BUTTON_LEFT;
}

static int buttonRight(int player_no, int style)
{
    (void)player_no;
    (void)style;

    if(SwapCharAllowed() && GamePaused == PauseCode::PauseScreen)
        return TouchScreenGFX_t::BUTTON_RIGHT_CHAR;
    else
        return TouchScreenGFX_t::BUTTON_RIGHT;
}

static int buttonA(int player_no, int style)
{
    (void)player_no;

    switch(style)
    {
    case TouchScreenController::style_abxy:
        return TouchScreenGFX_t::BUTTON_A;

    case TouchScreenController::style_xoda:
        return TouchScreenGFX_t::BUTTON_A_PS;

    default:
    case TouchScreenController::style_actions:
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

static int buttonX(int player_no, int style)
{
    switch(style)
    {
    case TouchScreenController::style_abxy:
        return TouchScreenGFX_t::BUTTON_X;

    case TouchScreenController::style_xoda:
        return TouchScreenGFX_t::BUTTON_X_PS;

    default:
    case TouchScreenController::style_actions:
        if(GamePaused != PauseCode::None || GameMenu)
            return TouchScreenGFX_t::BUTTON_X_BACK;
        else if(LevelSelect || GameOutro)
            return TouchScreenGFX_t::BUTTON_X_BLANK;
        else
        {
            if(numPlayers >= player_no)
            {
                auto& p = Player[player_no];

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

static int buttonB(int player_no, int style)
{
    switch(style)
    {
    case TouchScreenController::style_abxy:
        return TouchScreenGFX_t::BUTTON_B;

    case TouchScreenController::style_xoda:
        return TouchScreenGFX_t::BUTTON_B_PS;

    default:
    case TouchScreenController::style_actions:
        if(LevelSelect || GamePaused != PauseCode::None || GameMenu || GameOutro)
            return TouchScreenGFX_t::BUTTON_B_BLANK;
        else
        {
            if(numPlayers >= player_no)
            {
                auto& p = Player[player_no];

                if(p.Character <= 2 || p.Character == 4)
                    return TouchScreenGFX_t::BUTTON_B_SPINJUMP;
                else
                    return TouchScreenGFX_t::BUTTON_B_JUMP;
            }

            return TouchScreenGFX_t::BUTTON_B_BLANK;
        }
    }
}

static int buttonY(int player_no, int style)
{
    switch(style)
    {
    case TouchScreenController::style_abxy:
        return TouchScreenGFX_t::BUTTON_Y;

    case TouchScreenController::style_xoda:
        return TouchScreenGFX_t::BUTTON_Y_PS;

    default:
    case TouchScreenController::style_actions:
        if(LevelSelect || GamePaused != PauseCode::None || GameMenu || GameOutro)
            return TouchScreenGFX_t::BUTTON_Y_BLANK;
        else
        {
            if(numPlayers >= player_no)
            {
                auto& p = Player[player_no];

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
    enum anchor
    {
        T = 0,
        B,
        L = 0,
        R,
        C
    };

    struct KeyPos
    {
        anchor xa;
        anchor ya;
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
        static_assert(sizeof(touchKeysMap) == TouchScreenController::key_END * sizeof(KeyPos),
                      "The size of touchscreen keys pam must have the size of key_END multiplied to size of KeyPos");

        for(int it = TouchScreenController::key_BEGIN; it < TouchScreenController::key_END; it++)
        {
            auto& p = touchKeysMap[it];
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

        auto& key = touchKeysMap[cmd];
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
    int findTouchKeys(float x, float y, TouchScreenController::FingerState& fs)
    {
        // const size_t touchKeyMapSize = sizeof(touchKeysMap) / sizeof(KeyPos);
        int count = 0;
        x *= touchCanvasWidth;
        y *= touchCanvasHeight;

        for(const auto& p : touchKeysMap)
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
    {TouchKeyMap::L, TouchKeyMap::T, 335.0f, 12.0f, 451.0f, 52.0f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::T, 11.0f, 262.0f, 91.0f, 342.0f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::T, 171.0f, 262.0f, 251.0f, 342.0f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::T, 91.0f, 182.0f, 171.0f, 262.0f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::T, 91.0f, 342.0f, 171.0f, 422.0f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::T, 11.0f, 182.0f, 91.0f, 262.0f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::T, 171.0f, 182.0f, 251.0f, 262.0f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::T, 11.0f, 342.0f, 91.0f, 422.0f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::T, 171.0f, 342.0f, 251.0f, 422.0f, TouchScreenController::key_downright},
    {TouchKeyMap::L, TouchKeyMap::T, 396.0f, 276.0f, 493.0f, 373.0f, TouchScreenController::key_run},
    {TouchKeyMap::L, TouchKeyMap::T, 512.0f, 307.0f, 609.0f, 404.0f, TouchScreenController::key_jump},
    {TouchKeyMap::L, TouchKeyMap::T, 416.0f, 163.0f, 513.0f, 260.0f, TouchScreenController::key_altrun},
    {TouchKeyMap::L, TouchKeyMap::T, 531.0f, 191.0f, 628.0f, 288.0f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::T, 196.0f, 12.0f, 312.0f, 52.0f, TouchScreenController::key_drop},
    {TouchKeyMap::L, TouchKeyMap::T, 494.0f, 50.0f, 618.0f, 97.0f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 10.0f, 64.0f, 64.0f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 66.0f, 64.0f, 110.0f, TouchScreenController::key_enterCheats},
};

static const TouchKeyMap::KeyPos c_averagePhoneMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::L, TouchKeyMap::T, 542.0f, 537.0f, 693.0f, 587.0f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::T, 1.0f, 410.0f, 83.0f, 492.0f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::T, 165.0f, 410.0f, 247.0f, 492.0f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::T, 83.0f, 328.0f, 165.0f, 410.0f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::T, 83.0f, 492.0f, 165.0f, 574.0f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::T, 1.0f, 328.0f, 83.0f, 410.0f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::T, 165.0f, 328.0f, 247.0f, 410.0f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::T, 1.0f, 492.0f, 83.0f, 574.0f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::T, 165.0f, 492.0f, 247.0f, 574.0f, TouchScreenController::key_downright},
    {TouchKeyMap::L, TouchKeyMap::T, 764.0f, 403.0f, 868.0f, 507.0f, TouchScreenController::key_run},
    {TouchKeyMap::L, TouchKeyMap::T, 885.0f, 436.0f, 989.0f, 540.0f, TouchScreenController::key_jump},
    {TouchKeyMap::L, TouchKeyMap::T, 786.0f, 287.0f, 890.0f, 391.0f, TouchScreenController::key_altrun},
    {TouchKeyMap::L, TouchKeyMap::T, 904.0f, 317.0f, 1008.0f, 421.0f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::T, 331.0f, 537.0f, 482.0f, 587.0f, TouchScreenController::key_drop},
    {TouchKeyMap::L, TouchKeyMap::T, 827.0f, 129.0f, 943.0f, 169.0f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 10.0f, 70.0f, 70.0f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 74.0f, 70.0f, 134.0f, TouchScreenController::key_enterCheats},
};

static const TouchKeyMap::KeyPos c_averagePhoneLongMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::L, TouchKeyMap::T, 727.0f, 632.0f, 911.0f, 691.0f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::T, 5.0f, 444.0f, 106.0f, 545.0f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::T, 207.0f, 444.0f, 308.0f, 545.0f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::T, 106.0f, 343.0f, 207.0f, 444.0f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::T, 106.0f, 545.0f, 207.0f, 646.0f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::T, 5.0f, 343.0f, 106.0f, 444.0f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::T, 207.0f, 343.0f, 308.0f, 444.0f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::T, 5.0f, 545.0f, 106.0f, 646.0f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::T, 207.0f, 545.0f, 308.0f, 646.0f, TouchScreenController::key_downright},
    {TouchKeyMap::L, TouchKeyMap::T, 1055.0f, 500.0f, 1183.0f, 628.0f, TouchScreenController::key_run},
    {TouchKeyMap::L, TouchKeyMap::T, 1218.0f, 537.0f, 1346.0f, 665.0f, TouchScreenController::key_jump},
    {TouchKeyMap::L, TouchKeyMap::T, 1085.0f, 351.0f, 1213.0f, 479.0f, TouchScreenController::key_altrun},
    {TouchKeyMap::L, TouchKeyMap::T, 1242.0f, 389.0f, 1370.0f, 517.0f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::T, 474.0f, 632.0f, 658.0f, 691.0f, TouchScreenController::key_drop},
    {TouchKeyMap::L, TouchKeyMap::T, 1178.0f, 214.0f, 1315.0f, 275.0f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 10.0f, 85.0f, 85.0f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 90.0f, 85.0f, 165.0f, TouchScreenController::key_enterCheats},
};

static const TouchKeyMap::KeyPos c_7_tablet[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::L, TouchKeyMap::T, 636.0f, 544.0f, 775.0f, 582.0f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::T, 3.0f, 430.0f, 76.0f, 503.0f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::T, 149.0f, 430.0f, 222.0f, 503.0f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::T, 76.0f, 357.0f, 149.0f, 430.0f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::T, 76.0f, 503.0f, 149.0f, 576.0f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::T, 3.0f, 357.0f, 76.0f, 430.0f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::T, 149.0f, 357.0f, 222.0f, 430.0f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::T, 3.0f, 503.0f, 76.0f, 576.0f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::T, 149.0f, 503.0f, 222.0f, 576.0f, TouchScreenController::key_downright},
    {TouchKeyMap::L, TouchKeyMap::T, 797.0f, 439.0f, 887.0f, 529.0f, TouchScreenController::key_run},
    {TouchKeyMap::L, TouchKeyMap::T, 897.0f, 463.0f, 987.0f, 553.0f, TouchScreenController::key_jump},
    {TouchKeyMap::L, TouchKeyMap::T, 819.0f, 341.0f, 909.0f, 431.0f, TouchScreenController::key_altrun},
    {TouchKeyMap::L, TouchKeyMap::T, 919.0f, 363.0f, 1009.0f, 453.0f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::T, 257.0f, 544.0f, 396.0f, 582.0f, TouchScreenController::key_drop},
    {TouchKeyMap::L, TouchKeyMap::T, 873.0f, 226.0f, 968.0f, 258.0f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 10.0f, 58.0f, 58.0f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 64.0f, 58.0f, 112.0f, TouchScreenController::key_enterCheats},
};

static const TouchKeyMap::KeyPos c_10_6_tablet[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::L, TouchKeyMap::T, 869.0f, 753.0f, 1002.0f, 785.0f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::T, 19.0f, 591.0f, 89.0f, 661.0f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::T, 159.0f, 591.0f, 229.0f, 661.0f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::T, 89.0f, 521.0f, 159.0f, 591.0f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::T, 89.0f, 661.0f, 159.0f, 731.0f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::T, 19.0f, 521.0f, 89.0f, 591.0f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::T, 159.0f, 521.0f, 229.0f, 591.0f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::T, 19.0f, 661.0f, 89.0f, 731.0f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::T, 159.0f, 661.0f, 229.0f, 731.0f, TouchScreenController::key_downright},
    {TouchKeyMap::L, TouchKeyMap::T, 1047.0f, 588.0f, 1137.0f, 678.0f, TouchScreenController::key_run},
    {TouchKeyMap::L, TouchKeyMap::T, 1159.0f, 624.0f, 1249.0f, 714.0f, TouchScreenController::key_jump},
    {TouchKeyMap::L, TouchKeyMap::T, 1067.0f, 477.0f, 1157.0f, 567.0f, TouchScreenController::key_altrun},
    {TouchKeyMap::L, TouchKeyMap::T, 1179.0f, 505.0f, 1269.0f, 595.0f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::T, 256.0f, 753.0f, 389.0f, 785.0f, TouchScreenController::key_drop},
    {TouchKeyMap::L, TouchKeyMap::T, 1131.0f, 387.0f, 1225.0f, 426.0f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 10.0f, 57.0f, 57.0f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 10.0f, 60.0f, 57.0f, 107.0f, TouchScreenController::key_enterCheats},
};

// watch your aspect ratios. they are preserved now!

// based on c_10_6_tablet; no longer used.
#if 0
static const TouchKeyMap::KeyPos c_smallAutoMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::C, TouchKeyMap::B, 0.04, -0.08, 0.15, -0.04, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::B, 0.015, -0.245, 0.105, -0.155, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::B, 0.195, -0.245, 0.285, -0.155, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::B, 0.105, -0.335, 0.195, -0.245, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::B, 0.105, -0.155, 0.195, -0.065, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::B, 0.015, -0.335, 0.105, -0.245, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::B, 0.195, -0.335, 0.285, -0.245, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::B, 0.015, -0.155, 0.105, -0.065, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::B, 0.195, -0.155, 0.285, -0.065, TouchScreenController::key_downright},
    {TouchKeyMap::R, TouchKeyMap::B, -0.265, -0.215, -0.165, -0.115, TouchScreenController::key_run},
    {TouchKeyMap::R, TouchKeyMap::B, -0.15, -0.20, -0.05, -0.10, TouchScreenController::key_jump},
    {TouchKeyMap::R, TouchKeyMap::B, -0.25, -0.33, -0.15, -0.23, TouchScreenController::key_altrun},
    {TouchKeyMap::R, TouchKeyMap::B, -0.135, -0.315, -0.035, -0.215, TouchScreenController::key_altjump},
    {TouchKeyMap::C, TouchKeyMap::B, -0.1525, -0.08, -0.0375, -0.04, TouchScreenController::key_drop},
    {TouchKeyMap::R, TouchKeyMap::B, -0.12, -0.40, -0.045, -0.36, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 0.01, 0.01, 0.07, 0.07, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 0.01, 0.08, 0.07, 0.14, TouchScreenController::key_enterCheats},
};
#endif

// based on c_averagePhoneMap
static const TouchKeyMap::KeyPos c_standardAutoMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::R, TouchKeyMap::B, -0.555f, -0.07f, -0.39f, -0.01f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::B, 0.01f, -0.34f, 0.15f, -0.20f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::B, 0.29f, -0.34f, 0.43f, -0.20f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::B, 0.15f, -0.48f, 0.29f, -0.34f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::B, 0.15f, -0.20f, 0.29f, -0.06f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::B, 0.01f, -0.48f, 0.15f, -0.34f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::B, 0.29f, -0.48f, 0.43f, -0.34f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::B, 0.01f, -0.20f, 0.15f, -0.06f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::B, 0.29f, -0.20f, 0.43f, -0.06f, TouchScreenController::key_downright},
    {TouchKeyMap::R, TouchKeyMap::B, -0.375f, -0.275f, -0.225f, -0.125f, TouchScreenController::key_run},
    {TouchKeyMap::R, TouchKeyMap::B, -0.20f, -0.25f, -0.05f, -0.10f, TouchScreenController::key_jump},
    {TouchKeyMap::R, TouchKeyMap::B, -0.35f, -0.45f, -0.20f, -0.30f, TouchScreenController::key_altrun},
    {TouchKeyMap::R, TouchKeyMap::B, -0.175f, -0.425f, -0.025f, -0.275f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::B, 0.44f, -0.07f, 0.6125f, -0.01f, TouchScreenController::key_drop},
    {TouchKeyMap::R, TouchKeyMap::B, -0.20f, -0.56f, -0.05f, -0.48f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 0.02f, 0.02f, 0.12f, 0.12f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 0.02f, 0.14f, 0.12f, 0.24f, TouchScreenController::key_enterCheats},
};

// based on c_4_tinyPhoneMap
static const TouchKeyMap::KeyPos c_tightAutoMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {TouchKeyMap::R, TouchKeyMap::T, -0.50f, 0.02f, -0.17f, 0.14f, TouchScreenController::key_start},
    {TouchKeyMap::L, TouchKeyMap::B, 0.01f, -0.37f, 0.18f, -0.20f, TouchScreenController::key_left},
    {TouchKeyMap::L, TouchKeyMap::B, 0.35f, -0.37f, 0.52f, -0.20f, TouchScreenController::key_right},
    {TouchKeyMap::L, TouchKeyMap::B, 0.18f, -0.54f, 0.35f, -0.37f, TouchScreenController::key_up},
    {TouchKeyMap::L, TouchKeyMap::B, 0.18f, -0.20f, 0.35f, -0.03f, TouchScreenController::key_down},
    {TouchKeyMap::L, TouchKeyMap::B, 0.01f, -0.54f, 0.18f, -0.37f, TouchScreenController::key_upleft},
    {TouchKeyMap::L, TouchKeyMap::B, 0.35f, -0.54f, 0.52f, -0.37f, TouchScreenController::key_upright},
    {TouchKeyMap::L, TouchKeyMap::B, 0.01f, -0.20f, 0.18f, -0.03f, TouchScreenController::key_downleft},
    {TouchKeyMap::L, TouchKeyMap::B, 0.35f, -0.20f, 0.52f, -0.03f, TouchScreenController::key_downright},
    {TouchKeyMap::R, TouchKeyMap::B, -0.48f, -0.25f, -0.28f, -0.05f, TouchScreenController::key_run},
    {TouchKeyMap::R, TouchKeyMap::B, -0.25f, -0.22f, -0.05f, -0.02f, TouchScreenController::key_jump},
    {TouchKeyMap::R, TouchKeyMap::B, -0.45f, -0.48f, -0.25f, -0.28f, TouchScreenController::key_altrun},
    {TouchKeyMap::R, TouchKeyMap::B, -0.22f, -0.45f, -0.02f, -0.25f, TouchScreenController::key_altjump},
    {TouchKeyMap::L, TouchKeyMap::T, 0.165f, 0.02f, 0.50f, 0.14f, TouchScreenController::key_drop},
    {TouchKeyMap::R, TouchKeyMap::B, -0.245f, -0.62f, -0.02f, -0.50f, TouchScreenController::key_holdRun},
    {TouchKeyMap::L, TouchKeyMap::T, 0.02f, 0.02f, 0.14f, 0.14f, TouchScreenController::key_toggleKeysView},
    {TouchKeyMap::L, TouchKeyMap::T, 0.02f, 0.16f, 0.14f, 0.28f, TouchScreenController::key_enterCheats},
};
/*---------------------------------------------------------------------------------------*/

// figures out the ratio between sides (such that smaller, usually y, axis size is 1)
//   then handles all of the bottom/right anchored items
static void updateTouchMap(int preferredLayout,
                           float screenWidth, float screenHeight,
                           int scaleFactor,
                           int scaleFactorDPad,
                           int scaleFactorButtons,
                           int ssSpacing)
{
    switch(preferredLayout)
    {
    case(TouchScreenController::layout_old_tiny):
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_4_tinyPhoneMap, sizeof(g_touchKeyMap.touchKeysMap));
        g_touchKeyMap.touchCanvasWidth = 640.f;
        g_touchKeyMap.touchCanvasHeight = 480.f;
        break;

    case(TouchScreenController::layout_old_average):
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_averagePhoneMap, sizeof(g_touchKeyMap.touchKeysMap));
        g_touchKeyMap.touchCanvasWidth = 1024.f;
        g_touchKeyMap.touchCanvasHeight = 600.f;
        break;

    case(TouchScreenController::layout_old_long):
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_averagePhoneLongMap, sizeof(g_touchKeyMap.touchKeysMap));
        g_touchKeyMap.touchCanvasWidth = 1396.0f;
        g_touchKeyMap.touchCanvasHeight = 720.0f;
        break;

    case(TouchScreenController::layout_old_phablet):
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_7_tablet, sizeof(g_touchKeyMap.touchKeysMap));
        g_touchKeyMap.touchCanvasWidth = 1024.f;
        g_touchKeyMap.touchCanvasHeight = 600.f;
        break;

    case(TouchScreenController::layout_old_tablet):
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_10_6_tablet, sizeof(g_touchKeyMap.touchKeysMap));
        g_touchKeyMap.touchCanvasWidth = 1300.f;
        g_touchKeyMap.touchCanvasHeight = 812.f;
        break;

    case(TouchScreenController::layout_tight):
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_tightAutoMap, sizeof(g_touchKeyMap.touchKeysMap));
        break;

    case(TouchScreenController::layout_standard):
    default:
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_standardAutoMap, sizeof(g_touchKeyMap.touchKeysMap));
        break;
    }

    // legacy layout
    if(g_touchKeyMap.touchKeysMap[TouchScreenController::key_start].x1 > 2.f)
        return;

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

    for(int i = 0; i < TouchScreenController::key_END; i++)
    {
        g_touchKeyMap.touchKeysMap[i].x1 *= scaleFactor / 100.f;
        g_touchKeyMap.touchKeysMap[i].x2 *= scaleFactor / 100.f;
        g_touchKeyMap.touchKeysMap[i].y1 *= scaleFactor / 100.f;
        g_touchKeyMap.touchKeysMap[i].y2 *= scaleFactor / 100.f;

        if(i >= TouchScreenController::key_left && i <= TouchScreenController::key_downright)
        {
            g_touchKeyMap.touchKeysMap[i].x1 *= scaleFactorDPad / 100.f;
            g_touchKeyMap.touchKeysMap[i].x2 *= scaleFactorDPad / 100.f;
            g_touchKeyMap.touchKeysMap[i].y1 *= scaleFactorDPad / 100.f;
            g_touchKeyMap.touchKeysMap[i].y2 *= scaleFactorDPad / 100.f;
        }
        else if(i >= TouchScreenController::key_run && i <= TouchScreenController::key_altjump)
        {
            g_touchKeyMap.touchKeysMap[i].x1 *= scaleFactorButtons / 100.f;
            g_touchKeyMap.touchKeysMap[i].x2 *= scaleFactorButtons / 100.f;
            g_touchKeyMap.touchKeysMap[i].y1 *= scaleFactorButtons / 100.f;
            g_touchKeyMap.touchKeysMap[i].y2 *= scaleFactorButtons / 100.f;
        }

        if(g_touchKeyMap.touchKeysMap[i].xa == TouchKeyMap::R)
        {
            g_touchKeyMap.touchKeysMap[i].x1 += g_touchKeyMap.touchCanvasWidth;
            g_touchKeyMap.touchKeysMap[i].x2 += g_touchKeyMap.touchCanvasWidth;
        }
        else if(g_touchKeyMap.touchKeysMap[i].xa == TouchKeyMap::C)
        {
            g_touchKeyMap.touchKeysMap[i].x1 += g_touchKeyMap.touchCanvasWidth / 2;
            g_touchKeyMap.touchKeysMap[i].x2 += g_touchKeyMap.touchCanvasWidth / 2;
        }

        if(g_touchKeyMap.touchKeysMap[i].ya == TouchKeyMap::B)
        {
            g_touchKeyMap.touchKeysMap[i].y1 += g_touchKeyMap.touchCanvasHeight;
            g_touchKeyMap.touchKeysMap[i].y2 += g_touchKeyMap.touchCanvasHeight;
        }
        else if(g_touchKeyMap.touchKeysMap[i].ya == TouchKeyMap::C)
        {
            g_touchKeyMap.touchKeysMap[i].y1 += g_touchKeyMap.touchCanvasHeight / 2;
            g_touchKeyMap.touchKeysMap[i].y2 += g_touchKeyMap.touchCanvasHeight / 2;
        }

        // Don't let hold-run button go lower than buttons
        if(i == TouchScreenController::key_holdRun)
        {
            if(g_touchKeyMap.touchKeysMap[i].ya == TouchKeyMap::B)
            {
                float yb1 = g_touchKeyMap.touchKeysMap[TouchScreenController::key_altrun].y1;
                float yb2 = g_touchKeyMap.touchKeysMap[TouchScreenController::key_altjump].y1;
                float yb = SDL_min(yb1, yb2) - 0.05f;

                // is aligning needed?
                if(yb < g_touchKeyMap.touchKeysMap[i].y2)
                {
                    float o = g_touchKeyMap.touchKeysMap[i].y2;
                    g_touchKeyMap.touchKeysMap[i].y2 = yb;
                    g_touchKeyMap.touchKeysMap[i].y1 += (g_touchKeyMap.touchKeysMap[i].y2 - o);
                }
            }
        }
        // Compute spacing between start and select
        else if(ssSpacing != 100 && (i == TouchScreenController::key_start || i == TouchScreenController::key_drop))
        {
            if(g_touchKeyMap.touchKeysMap[i].xa == TouchKeyMap::L)
            {
                float o = g_touchKeyMap.touchKeysMap[i].x1;
                g_touchKeyMap.touchKeysMap[i].x1 *= 1.0f / (ssSpacing / 100.f);
                g_touchKeyMap.touchKeysMap[i].x2 += (g_touchKeyMap.touchKeysMap[i].x1 - o);
            }

            if(g_touchKeyMap.touchKeysMap[i].xa == TouchKeyMap::R)
            {
                float o = g_touchKeyMap.touchKeysMap[i].x2;
                float off = g_touchKeyMap.touchCanvasWidth - g_touchKeyMap.touchKeysMap[i].x2;
                off *= 1.0f / (ssSpacing / 100.f);
                g_touchKeyMap.touchKeysMap[i].x2 = g_touchKeyMap.touchCanvasWidth - off;
                g_touchKeyMap.touchKeysMap[i].x1 += (g_touchKeyMap.touchKeysMap[i].x2 - o);
            }
        }
    }
}


void TouchScreenController::doVibration()
{
    if(!m_vibrator)
        return;

    if(m_feedback_strength == 0.f)
        return;

    SDL_HapticRumblePlay(m_vibrator, m_feedback_strength, m_feedback_length);
    D_pLogDebug("TouchScreen: Vibration %g, %d ms", m_feedback_strength, m_feedback_length);
}

TouchScreenController::~TouchScreenController()
{
    if(m_vibrator)
        SDL_HapticClose(m_vibrator);

    m_vibrator = nullptr;
}

TouchScreenController::TouchScreenController() noexcept
{
    updateScreenSize();

    pLogDebug("Initialization of touch-screen controller...");

    if(!m_GFX.m_success)
        pLogDebug("Touch-screen controller cannot be used due to missing assets.");

    scanTouchDevices();

    for(auto &d : m_devices)
    {
        for(int key = key_BEGIN; key < key_END; ++key)
            d.keysHeld[key] = false;
    }

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

void TouchScreenController::scanTouchDevices()
{
    m_touchDevicesCount = SDL_GetNumTouchDevices();

    if(touchSupported())
    {
        pLogDebug("Found %d touch devices, screen size: %d x %d",
                  m_touchDevicesCount,
                  m_screenWidth, m_screenHeight);

        m_devices.clear();
        m_devices.resize(m_touchDevicesCount);

        for(int i = 0; i < m_touchDevicesCount; ++i)
        {
            auto &d = m_devices[i];
            d.id = SDL_GetTouchDevice(i);
            if(!d.id) // Invalid touch device, will be dropped from the list
            {
                pLogDebug("Touch device %d: <Invalid device>", i);
                continue;
            }

#if SDL_VERSION_ATLEAST(2, 0, 10)
            const char *typeText = "Invalid type";
            SDL_TouchDeviceType ty = SDL_GetTouchDeviceType(d.id);

            switch(ty)
            {
            default:
            case SDL_TOUCH_DEVICE_INVALID: // invalid
                break;
            case SDL_TOUCH_DEVICE_DIRECT:
                typeText = "Direct touch type";
                break;
            case SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE:
                typeText = "Indiriect absolute type";
                break;
            case SDL_TOUCH_DEVICE_INDIRECT_RELATIVE:
                typeText = "Indiriect relative type";
                break;
            }

            pLogDebug("Touch device %d (id=%d): %s", i, (int)d.id, typeText);

            if(ty != SDL_TOUCH_DEVICE_DIRECT)
            {
                d.id = 0; // Drop any indirect devices
                continue;
            }
#endif

            SDL_memset(d.keysHeld, 0, sizeof(d.keysHeld));
        }

        // Remove unnecessary devices from the list
        for(auto it = m_devices.begin(); it != m_devices.end(); )
        {
            if(!it->id)
                it = m_devices.erase(it);
            else
                ++it;
        }
    }

    pLogDebug("Totally loaded valid touch devices: %d", (int)m_devices.size());
}

void TouchScreenController::updateScreenSize()
{
    XWindow::getWindowSize(&m_screenWidth, &m_screenHeight);
    XRender::getRenderSize(&m_renderWidth, &m_renderHeight);

    updateTouchMap(m_layout,
                   m_screenWidth, m_screenHeight,
                   m_scale_factor,
                   m_scale_factor_dpad,
                   m_scale_factor_buttons,
                   m_scale_factor_ss_spacing);
}

static void updateKeyValue(bool& key, bool state)
{
    key = state;
    D_pLogDebug("= Touch key: State=%d", (int)key);
}

static void updateFingerKeyState(TouchScreenController::FingerState& st,
                                 Controls_t& keys, int keyCommand, bool setState, TouchScreenController::ExtraKeys_t& extraSt)
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

        case TouchScreenController::key_enterCheats:
            extraSt.keyCheats = setState;
            break;

        default:
            break;
        }
    }
    else
        st.alive = false;
}

void TouchScreenController::processTouchDevice(TouchDevice_t& dev)
{
    auto& m_fingers = dev.fingers;
    auto& keysHeld = dev.keysHeld;
    auto& current_extra_keys = dev.extra_keys;
    auto& current_keys = dev.current_keys;
    int  fingers = SDL_GetNumTouchFingers(dev.id);

    for(auto& m_finger : m_fingers)
    {
        // Mark as "dead"
        m_finger.second.alive = false;
    }

    int n_real_fingers = 0;
    double total_finger_x = 0;
    double total_finger_y = 0;

    for(int i = 0; i < fingers; i++)
    {
        SDL_Finger* f = SDL_GetTouchFinger(dev.id, i);

        if(!f) //Skip a wrong finger
            continue;

        SDL_FingerID finger_id = f->id;
        float finger_x = f->x, finger_y = f->y, finger_pressure = f->pressure;
        (void)finger_pressure;

        n_real_fingers++;
        total_finger_x += finger_x;
        total_finger_y += finger_y;

        // track which finger is controlling the cursor
        if(!m_wasScrolling && (m_cursorFinger == -1 || m_cursorFinger == finger_id))
        {
            m_cursorActive = true;
            m_cursorFinger = finger_id;
            int cX, cY;
            XRender::mapToScreen(finger_x * m_screenWidth, finger_y * m_screenHeight, &cX, &cY);
            m_cursorX = cX;
            m_cursorY = cY;
        }

        auto found = m_fingers.find(finger_id);

        if(found != m_fingers.end())
        {
            FingerState& fs = found->second;
            int keysCount = g_touchKeyMap.findTouchKeys(finger_x, finger_y, fs);

            for(int key = key_BEGIN; key < key_END; ++key)
            {
                if(fs.ignore)
                    break;

                if(m_touchHidden && key != key_toggleKeysView)
                    continue;

                if(fs.heldKeyPrev[key] && !fs.heldKey[key]) // set key off
                {
                    updateFingerKeyState(fs, current_keys, key, false, current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d released (move)", static_cast<int>(key));
                    keysHeld[key] = false;
                    fs.heldKeyPrev[key] = fs.heldKey[key];
                }
                else if(fs.heldKey[key]) // set key on and keep alive
                {
                    if(!fs.heldKeyPrev[key] /*&& fs.heldKey[key]*/) // Already true because of condition from above
                        doVibration(); // Vibrate when key gets on

                    updateFingerKeyState(fs, current_keys, key, true, current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d pressed (move)", static_cast<int>(key));
                    keysHeld[key] = true;
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
                    continue;

                if(st.heldKey[key]) // set key on
                {
                    updateFingerKeyState(st, current_keys, key, true, current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d pressed (put)", static_cast<int>(key));
                    doVibration();
                    keysHeld[key] = true;
                    st.heldKeyPrev[key] = st.heldKey[key];
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
                    updateFingerKeyState(it->second, current_keys, key, false, current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d released (take)", static_cast<int>(key));
                    keysHeld[key] = false;
                }
            }

            D_pLogDebug("= Finger ID=%d has gone", static_cast<int>(it->first));

            it = m_fingers.erase(it);
            continue;
        }

        ++it;
    }

    if(n_real_fingers > 1 && LevelEditor && !editorScreen.active)
    {
        int MeanX, MeanY;
        XRender::mapToScreen(total_finger_x / n_real_fingers * m_screenWidth, total_finger_y / n_real_fingers * m_screenHeight, &MeanX, &MeanY);

        if(m_wasScrolling)
        {
            m_scrollX += m_lastMeanX - MeanX;
            m_scrollY += m_lastMeanY - MeanY;
        }

        m_lastMeanX = MeanX;
        m_lastMeanY = MeanY;
        m_scrollActive = true;
    }

    // Merge per-device states into common states:
    for(int key = key_BEGIN; key < key_END; ++key)
        m_keysHeld[key] |= keysHeld[key];

    m_current_keys |= current_keys;
    m_current_extra_keys |= current_extra_keys;
}

void TouchScreenController::update()
{
    if(!touchSupported())
        return;

    if(this->m_active_method)
    {
        auto* p = dynamic_cast<InputMethodProfile_TouchScreen*>(this->m_active_method->Profile);

        if(p)
        {
            this->m_touchpad_style = p->m_touchpad_style;
            this->m_enable_enter_cheats = p->m_enable_enter_cheats;

            if(p->m_device_count <= 0)
                p->m_device_count = this->m_touchDevicesCount;

            if(this->m_feedback_strength != p->m_feedback_strength
               || this->m_feedback_length != p->m_feedback_length)
            {
                this->m_feedback_strength = p->m_feedback_strength;
                this->m_feedback_length = p->m_feedback_length;
                this->doVibration();
            }

            if(this->m_hold_run != p->m_hold_run)
            {
                this->m_hold_run = p->m_hold_run;
                this->m_runHeld = this->m_hold_run;
            }

            if(this->m_layout != p->m_layout)
            {
                this->m_layout = p->m_layout;
                this->updateScreenSize();
            }

            if(this->m_scale_factor != p->m_scale_factor)
            {
                this->m_scale_factor = p->m_scale_factor;
                this->updateScreenSize();
            }

            if(this->m_scale_factor_dpad != p->m_scale_factor_dpad)
            {
                this->m_scale_factor_dpad = p->m_scale_factor_dpad;
                this->updateScreenSize();
            }

            if(this->m_scale_factor_buttons != p->m_scale_factor_buttons)
            {
                this->m_scale_factor_buttons = p->m_scale_factor_buttons;
                this->updateScreenSize();
            }

            if(this->m_scale_factor_ss_spacing != p->m_scale_factor_ss_spacing)
            {
                this->m_scale_factor_ss_spacing = p->m_scale_factor_ss_spacing;
                this->updateScreenSize();
            }
        }
    }


    bool cursor_was_held = m_cursorActive;
    m_cursorActive = false;
    m_scrollActive = false;

    m_scrollX = 0;
    m_scrollY = 0;

    for(int key = key_BEGIN; key < key_END; ++key)
        m_keysHeld[key] = false;

    m_current_extra_keys = ExtraKeys_t();
    m_current_keys = Controls_t();

    // Process all working touch devices
    for(auto& d : m_devices)
        processTouchDevice(d);

    // handle cursor for double-tap logic
    if(!m_cursorActive && cursor_was_held)
    {
        m_lastCursorX = m_cursorX;
        m_lastCursorY = m_cursorY;
        m_cursorFinger = -1;
    }


    // handle scrolling and scroll momentum

    if(m_scrollActive && m_wasScrolling)
    {
        m_scrollMomentumX = m_lastScrollX;
        m_scrollMomentumY = m_lastScrollY;
    }

    if(m_scrollActive)
    {
        // need two counters because of an SDL bug where the frame
        // a finger it is lifted its finger still exists at its old location
        m_lastScrollX = m_scrollX;
        m_lastScrollY = m_scrollY;
    }

    m_wasScrolling = m_scrollActive;

    if(!m_scrollActive && m_scrollMomentumX != 0.)
    {
        m_scrollMomentumX *= 0.97;
        m_scrollX += m_scrollMomentumX;
        if(m_scrollMomentumX > -0.1 && m_scrollMomentumX < 0.1)
            m_scrollMomentumX = 0.;
    }

    if(!m_scrollActive && m_scrollMomentumY != 0.)
    {
        m_scrollMomentumY *= 0.97;
        m_scrollY += m_scrollMomentumY;
        if(m_scrollMomentumY > -0.1 && m_scrollMomentumY < 0.1)
            m_scrollMomentumY = 0.;
    }

    // handle special keys
    if(m_current_extra_keys.keyToggleViewOnce)
    {
        m_touchHidden = !m_touchHidden;

        if(!m_touchHidden)
            SharedCursor.GoOffscreen();
    }

    if(m_current_extra_keys.keyHoldRunOnce)
        m_runHeld = !m_runHeld;
}

void TouchScreenController::render(int player_no)
{
    if(!touchSupported())
        return;

    int style = m_touchpad_style;

    XRender::offsetViewportIgnore(true);

    for(int key = key_BEGIN; key < key_END; key++)
    {
        if((m_touchHidden && key != TouchScreenController::key_toggleKeysView) || LoadingInProcess || LevelEditor)
            continue;

        const auto& k = g_touchKeyMap.touchKeysMap[key];
        int x1 = Maths::iRound((k.x1 / g_touchKeyMap.touchCanvasWidth) * float(m_renderWidth));
        int y1 = Maths::iRound((k.y1 / g_touchKeyMap.touchCanvasHeight) * float(m_renderHeight));
        int x2 = Maths::iRound((k.x2 / g_touchKeyMap.touchCanvasWidth) * float(m_renderWidth));
        int y2 = Maths::iRound((k.y2 / g_touchKeyMap.touchCanvasHeight) * float(m_renderHeight));
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
            XRender::renderTextureScale(x1, y1, w, h,
                                        m_GFX.touch[m_touchHidden ? TouchScreenGFX_t::BUTTON_VIEW_TOGGLE_OFF : TouchScreenGFX_t::BUTTON_VIEW_TOGGLE_ON],
                                        1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_start:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_START], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_drop:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DROP], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_up:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_UP], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_left:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonLeft(player_no, style)], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_right:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonRight(player_no, style)], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_down:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DOWN], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_upleft:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_UPLEFT], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_upright:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_UPRIGHT], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_downleft:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DOWNLEFT], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_downright:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_DOWNRIGHT], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_holdRun:
            XRender::renderTextureScale(x1, y1, w, h,
                                        m_GFX.touch[m_runHeld ? TouchScreenGFX_t::BUTTON_HOLD_RUN_ON : TouchScreenGFX_t::BUTTON_HOLD_RUN_OFF],
                                        1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_jump:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonA(player_no, style)], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_run:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonX(player_no, style)], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_altjump:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonB(player_no, style)], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_altrun:
            XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[buttonY(player_no, style)], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_enterCheats:
            if(m_enable_enter_cheats)
                XRender::renderTextureScale(x1, y1, w, h, m_GFX.touch[TouchScreenGFX_t::BUTTON_ENTER_CHEATS], 1.f, 1.f, 1.f, a);

            break;

        default:
            XRender::renderRect(x1, y1, w, h, r, g, 0.f, 0.3f);
            break;
        }
    }

    XRender::offsetViewportIgnore(false);
}

void TouchScreenController::resetState()
{
    this->update();

    for(auto &d : m_devices)
    {
        for(auto& state : d.fingers)
            state.second.ignore = true;
        d.current_keys = Controls_t();
        d.extra_keys = ExtraKeys_t();
        SDL_memset(d.keysHeld, 0, sizeof(d.keysHeld));
    }

    m_current_keys = Controls_t();
    m_current_extra_keys = ExtraKeys_t();
    SDL_memset(m_keysHeld, 0, sizeof(m_keysHeld));
}

/*====================================================*\
|| implementation for InputMethod_TouchScreen         ||
\*====================================================*/

InputMethod_TouchScreen::~InputMethod_TouchScreen()
{
    InputMethodType_TouchScreen* t = dynamic_cast<InputMethodType_TouchScreen*>(this->Type);

    if(!t)
        return;

    if(t->m_controller.m_active_method == this)
        t->m_controller.m_active_method = nullptr;
}

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_TouchScreen::Update(int player, Controls_t& c, CursorControls_t& m, EditorControls_t& e, HotkeysPressed_t& h)
{
    auto* t = dynamic_cast<InputMethodType_TouchScreen*>(this->Type);

    if(!t)
        return false;

    if(!t->m_controller.touchSupported())
        return false;

    c = t->m_controller.m_current_keys;

    const TouchScreenController::ExtraKeys_t& te = t->m_controller.m_current_extra_keys;

    if(GamePaused == PauseCode::None && !GameMenu && !GameOutro && !LevelSelect && t->m_controller.m_runHeld)
    {
        // Alt Run functions as normal
        if(c.AltRun)
        {
            // make sure Alt Run activates properly
            if(te.keyAltRunOnce)
                c.AltRun = false;
            c.Run = false;
        }
        else if(te.keyRunOnce)
            c.Run = false;
        else
            c.Run |= true;
    }

    // use the touchscreen as a mouse if the buttons are currently hidden, or we are in LevelEditor mode
    bool allowed = t->m_controller.m_touchHidden || LevelEditor;

    if(allowed && t->m_controller.m_scrollActive)
    {
        m.GoOffscreen();
        m.Move = true;
    }
    else if(allowed && t->m_controller.m_cursorActive)
    {
        if(LevelEditor || MagicHand)
        {
            // hold to preview, double-tap to click in main editor screen
            if(editorScreen.active || t->m_controller.m_cursorY < 40)
            {
                m.Primary = true;
            }
            else if(t->m_controller.m_lastCursorX == -42
                || (t->m_controller.m_cursorX - t->m_controller.m_lastCursorX > -10
                && t->m_controller.m_cursorX - t->m_controller.m_lastCursorX < 10
                && t->m_controller.m_cursorY - t->m_controller.m_lastCursorY > -10
                && t->m_controller.m_cursorY - t->m_controller.m_lastCursorY < 10))
            {
                m.Primary = true;
                // special value to allow drags
                t->m_controller.m_lastCursorX = -42;
            }
        }
        else
        {
            m.Primary = true;
        }

        if(t->m_controller.m_cursorX - m.X <= -1 || t->m_controller.m_cursorX - m.X >= 1
           || t->m_controller.m_cursorY - m.Y <= -1 || t->m_controller.m_cursorY - m.Y >= 1)
        {
            m.Move = true;
            m.X = t->m_controller.m_cursorX;
            m.Y = t->m_controller.m_cursorY;
        }

        m.Touch = true;
    }

    // update editor scroll
    if(t->m_controller.m_scrollX < 0)
        e.ScrollLeft += -t->m_controller.m_scrollX;
    else if(t->m_controller.m_scrollX > 0)
        e.ScrollRight += t->m_controller.m_scrollX;
    if(t->m_controller.m_scrollY < 0)
        e.ScrollUp += -t->m_controller.m_scrollY;
    else if(t->m_controller.m_scrollY > 0)
        e.ScrollDown += t->m_controller.m_scrollY;

    if(t->m_controller.m_current_extra_keys.keyCheats && t->m_controller.m_enable_enter_cheats)
        h[Hotkeys::Buttons::EnterCheats] = player;

    // auto show/hide depending on context
    if(m_wasTextEntry && GamePaused != PauseCode::TextEntry)
    {
        t->m_controller.m_touchHidden = false;
        m_wasTextEntry = false;
    }
    else if(!m_wasTextEntry && GamePaused == PauseCode::TextEntry)
    {
        t->m_controller.m_touchHidden = true;
        m_wasTextEntry = true;
    }

    return true;
}

void InputMethod_TouchScreen::Rumble(int ms, float strength)
{
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

StatusInfo InputMethod_TouchScreen::GetStatus()
{
    return XPower::devicePowerStatus();
}

/*====================================================*\
|| implementation for InputMethodProfile_TouchScreen  ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_TouchScreen::InputMethodProfile_TouchScreen()
{
    // this->m_showPowerStatus = g_config.JoystickEnableBatteryStatus;

#ifdef __ANDROID__
    if(s_screenSize >= 9.0) // Big tablets
    {
        m_default_layout = TouchScreenController::layout_standard;
        m_default_scale_factor = 65;
        m_default_scale_factor_dpad = 80;
        m_default_scale_factor_buttons = 105;
        m_default_scale_factor_ss_spacing = 110;
    }
    else if(s_screenSize >= 7.0) // Middle tablets
    {
        m_default_layout = TouchScreenController::layout_standard;
        m_default_scale_factor = 95;
        m_default_scale_factor_dpad = 80;
        m_default_scale_factor_buttons = 105;
        m_default_scale_factor_ss_spacing = 95;
    }
    else if(s_screenSize < 4.0) // Very small phones
    {
        m_default_layout = TouchScreenController::layout_tight;
        m_default_scale_factor = 100;
        m_default_scale_factor_dpad = 110;
        m_default_scale_factor_buttons = 135;
        m_default_scale_factor_ss_spacing = 75;
    }
    else // All other devices
    {
        // Longer screens (big ration between sides, more like a stick)
        if((s_screenWidth / s_screenHeight) > 1.6f)
        {
            m_default_layout = TouchScreenController::layout_standard;
            m_default_scale_factor = 135;
            m_default_scale_factor_dpad = 80;
            m_default_scale_factor_buttons = 100;
            m_default_scale_factor_ss_spacing = 100;
        }
        else // Shorter screens (smaller ratio between sides, more like a square)
        {
            m_default_layout = TouchScreenController::layout_standard;
            m_default_scale_factor = 135;
            m_default_scale_factor_dpad = 80;
            m_default_scale_factor_buttons = 100;
            m_default_scale_factor_ss_spacing = 90;
        }
    }
#endif
}

bool InputMethodProfile_TouchScreen::PollPrimaryButton(ControlsClass c, size_t i)
{
    UNUSED(c);
    UNUSED(i);
    return true;
}

bool InputMethodProfile_TouchScreen::PollSecondaryButton(ControlsClass c, size_t i)
{
    UNUSED(c);
    UNUSED(i);
    return true;
}

bool InputMethodProfile_TouchScreen::DeletePrimaryButton(ControlsClass c, size_t i)
{
    UNUSED(c);
    UNUSED(i);
    return true;
}

bool InputMethodProfile_TouchScreen::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    UNUSED(c);
    UNUSED(i);
    return true;
}

const char* InputMethodProfile_TouchScreen::NamePrimaryButton(ControlsClass c, size_t i)
{
    UNUSED(c);
    UNUSED(i);
    return g_controlsStrings.caseTouch.c_str();
}

const char* InputMethodProfile_TouchScreen::NameSecondaryButton(ControlsClass c, size_t i)
{
    UNUSED(c);
    UNUSED(i);
    return "";
}

void InputMethodProfile_TouchScreen::SaveConfig(IniProcessing* ctl)
{
    ctl->setValue("ui-layout", this->m_layout);
    ctl->setValue("scale-factor", this->m_scale_factor);
    ctl->setValue("scale-factor-dpad", this->m_scale_factor_dpad);
    ctl->setValue("scale-factor-buttons", this->m_scale_factor_buttons);
    ctl->setValue("scale-factor-ss-spacing", this->m_scale_factor_ss_spacing);
    ctl->setValue("ui-style", this->m_touchpad_style);
    ctl->setValue("vibration-strength", this->m_feedback_strength);
    ctl->setValue("vibration-length", this->m_feedback_length);
    ctl->setValue("hold-run", this->m_hold_run);
    ctl->setValue("enable-enter-cheats", this->m_enable_enter_cheats);
}

void InputMethodProfile_TouchScreen::LoadConfig(IniProcessing* ctl)
{
    ctl->read("ui-layout", this->m_layout, this->m_default_layout);

    if(this->m_layout >= TouchScreenController::layout_END)
        this->m_layout = this->m_default_layout;

    ctl->read("scale-factor", this->m_scale_factor, this->m_default_scale_factor);
    ctl->read("scale-factor-dpad", this->m_scale_factor_dpad, this->m_default_scale_factor_dpad);
    ctl->read("scale-factor-buttons", this->m_scale_factor_buttons, this->m_default_scale_factor_buttons);
    ctl->read("scale-factor-ss-spacing", this->m_scale_factor_ss_spacing, this->m_default_scale_factor_ss_spacing);
    ctl->read("ui-style", this->m_touchpad_style, TouchScreenController::style_actions);
    ctl->read("vibration-strength", this->m_feedback_strength, 0.f);
    ctl->read("vibration-length", this->m_feedback_length, 12);
    ctl->read("hold-run", this->m_hold_run, false);
    ctl->read("enable-enter-cheats", this->m_enable_enter_cheats, false);
}

// How many per-type special options are there?
size_t InputMethodProfile_TouchScreen::GetOptionCount_Custom()
{
    return Options::COUNT;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount_Custom() returns 0.
// get a char* describing the option
const char* InputMethodProfile_TouchScreen::GetOptionName_Custom(size_t i)
{
    switch(i)
    {
    case Options::layout:
        return g_controlsStrings.touchscreenOptionLayoutStyle.c_str();

    case Options::scale_factor:
        return g_controlsStrings.touchscreenOptionScaleFactor.c_str();

    case Options::scale_factor_dpad:
        return g_controlsStrings.touchscreenOptionScaleDPad.c_str();

    case Options::scale_factor_buttons:
        return g_controlsStrings.touchscreenOptionScaleButtons.c_str();

    case Options::scale_factor_ss_spacing:
        return g_controlsStrings.touchscreenOptionSStartSpacing.c_str();

    case Options::reset_layout:
        return g_controlsStrings.touchscreenOptionResetLayout.c_str();

    case Options::style:
        return g_controlsStrings.touchscreenOptionInterfaceStyle.c_str();

    case Options::fb_strength:
        return g_controlsStrings.touchscreenOptionFeedbackStrength.c_str();

    case Options::fb_length:
        return g_controlsStrings.touchscreenOptionFeedbackLength.c_str();

    case Options::hold_run:
        return g_controlsStrings.touchscreenOptionHoldRun.c_str();

    case Options::enable_enter_cheats:
        return g_controlsStrings.touchscreenOptionShowCodeButton.c_str();
    }

    return nullptr;
}

// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodProfile_TouchScreen::GetOptionValue_Custom(size_t i)
{
    static char length_buf[8];
    static std::string delay_buf;

    switch(i)
    {
    case Options::layout:
        if(this->m_layout == TouchScreenController::layout_tight)
            return g_controlsStrings.touchscreenLayoutTight.c_str();
        else if(this->m_layout == TouchScreenController::layout_old_tiny)
            return g_controlsStrings.touchscreenLayoutTinyOld.c_str();
        else if(this->m_layout == TouchScreenController::layout_old_average)
            return g_controlsStrings.touchscreenLayoutPhoneOld.c_str();
        else if(this->m_layout == TouchScreenController::layout_old_long)
            return g_controlsStrings.touchscreenLayoutLongOld.c_str();
        else if(this->m_layout == TouchScreenController::layout_old_phablet)
            return g_controlsStrings.touchscreenLayoutPhabletOld.c_str();
        else if(this->m_layout == TouchScreenController::layout_old_tablet)
            return g_controlsStrings.touchscreenLayoutTabletOld.c_str();
        else
            return g_controlsStrings.touchscreenLayoutStandard.c_str();

    case Options::scale_factor:
        SDL_snprintf(length_buf, 8, "%d%%", this->m_scale_factor);
        return length_buf;

    case Options::scale_factor_dpad:
        SDL_snprintf(length_buf, 8, "%d%%", this->m_scale_factor_dpad);
        return length_buf;

    case Options::scale_factor_buttons:
        SDL_snprintf(length_buf, 8, "%d%%", this->m_scale_factor_buttons);
        return length_buf;

    case Options::scale_factor_ss_spacing:
        SDL_snprintf(length_buf, 8, "%d%%", this->m_scale_factor_ss_spacing);
        return length_buf;

    case Options::style:
        if(this->m_touchpad_style == TouchScreenController::style_actions)
            return g_controlsStrings.touchscreenStyleActions.c_str();
        else if(this->m_touchpad_style == TouchScreenController::style_abxy)
            return g_controlsStrings.touchscreenStyleABXY.c_str();
        else
            return g_controlsStrings.touchscreenStyleXODA.c_str();

    case Options::fb_strength:
        if(this->m_feedback_strength == 0.f)
            return g_mainMenu.wordOff.c_str();
        else if(this->m_feedback_strength == 0.25f)
            return "25%";
        else if(this->m_feedback_strength == 0.50f)
            return "50%";
        else if(this->m_feedback_strength == 0.75f)
            return "75%";
        else
            return "100%";

    case Options::fb_length:
        delay_buf = std::to_string(this->m_feedback_length);
        delay_buf += " ";
        delay_buf += g_mainMenu.abbrevMilliseconds;
        return delay_buf.c_str();

    case Options::hold_run:
        if(this->m_hold_run)
            return g_mainMenu.wordOn.c_str();
        else
            return g_mainMenu.wordOff.c_str();

    case Options::enable_enter_cheats:
        if(this->m_enable_enter_cheats)
            return g_mainMenu.wordOn.c_str();
        else
            return g_mainMenu.wordOff.c_str();
    }

    return nullptr;
}

// called when A is pressed; allowed to interrupt main game loop
bool InputMethodProfile_TouchScreen::OptionChange_Custom(size_t i)
{
    switch(i)
    {
    case Options::reset_layout:
        this->m_layout = this->m_default_layout;
        this->m_scale_factor = this->m_default_scale_factor;
        this->m_scale_factor_dpad = this->m_default_scale_factor_dpad;
        this->m_scale_factor_buttons = this->m_default_scale_factor_buttons;
        this->m_scale_factor_ss_spacing = this->m_default_scale_factor_ss_spacing;
        return true;

    default:
        return this->OptionRotateRight_Custom(i);
    }
}

// called when left is pressed
bool InputMethodProfile_TouchScreen::OptionRotateLeft_Custom(size_t i)
{
    switch(i)
    {
    case Options::layout:
        if(this->m_layout > 0)
            this->m_layout--;
        else
            this->m_layout = TouchScreenController::layout_END - 1;

        return true;

    case Options::scale_factor:
        if(this->m_scale_factor > 50)
        {
            this->m_scale_factor -= 5;
            return true;
        }
        else
            return false;

    case Options::scale_factor_dpad:
        if(this->m_scale_factor_dpad > 50)
        {
            this->m_scale_factor_dpad -= 5;
            return true;
        }
        else
            return false;

    case Options::scale_factor_buttons:
        if(this->m_scale_factor_buttons > 50)
        {
            this->m_scale_factor_buttons -= 5;
            return true;
        }
        else
            return false;

    case Options::scale_factor_ss_spacing:
        if(this->m_scale_factor_ss_spacing > 20)
        {
            this->m_scale_factor_ss_spacing -= 5;
            return true;
        }
        else
            return false;

    case Options::style:
        if(this->m_touchpad_style > 0)
            this->m_touchpad_style--;
        else
            this->m_touchpad_style = TouchScreenController::style_END - 1;

        return true;

    case Options::fb_strength:
        if(this->m_feedback_strength > 0.f)
        {
            this->m_feedback_strength -= 0.25f;
            return true;
        }

        return false;

    case Options::fb_length:
        if(this->m_feedback_length > 2)
        {
            this->m_feedback_length -= 2;
            return true;
        }

        return false;

    case Options::hold_run:
        this->m_hold_run = !this->m_hold_run;
        return true;

    case Options::enable_enter_cheats:
        this->m_enable_enter_cheats = !this->m_enable_enter_cheats;
        return true;
    }

    return false;
}

// called when right is pressed
bool InputMethodProfile_TouchScreen::OptionRotateRight_Custom(size_t i)
{
    switch(i)
    {
    case Options::layout:
        this->m_layout++;

        if(this->m_layout >= TouchScreenController::layout_END)
            this->m_layout = 0;

        return true;

    case Options::scale_factor:
        if(this->m_scale_factor < 150)
        {
            this->m_scale_factor += 5;
            return true;
        }
        else
            return false;

    case Options::scale_factor_dpad:
        if(this->m_scale_factor_dpad < 150)
        {
            this->m_scale_factor_dpad += 5;
            return true;
        }
        else
            return false;

    case Options::scale_factor_buttons:
        if(this->m_scale_factor_buttons < 150)
        {
            this->m_scale_factor_buttons += 5;
            return true;
        }
        else
            return false;

    case Options::scale_factor_ss_spacing:
        if(this->m_scale_factor_ss_spacing < 110)
        {
            this->m_scale_factor_ss_spacing += 5;
            return true;
        }
        else
            return false;

    case Options::style:
        this->m_touchpad_style ++;

        if(this->m_touchpad_style >= TouchScreenController::style_END)
            this->m_touchpad_style = 0;

        return true;

    case Options::fb_strength:
        if(this->m_feedback_strength < 1.f)
        {
            this->m_feedback_strength += 0.25f;
            return true;
        }

        return false;

    case Options::fb_length:
        this->m_feedback_length += 2;
        return true;

    case Options::hold_run:
        this->m_hold_run = !this->m_hold_run;
        return true;

    case Options::enable_enter_cheats:
        this->m_enable_enter_cheats = !this->m_enable_enter_cheats;
        return true;
    }

    return false;
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

const std::string& InputMethodType_TouchScreen::LocalName() const
{
    return g_controlsStrings.nameTouchscreen;
}

bool InputMethodType_TouchScreen::TestProfileType(InputMethodProfile* profile)
{
    return (bool)dynamic_cast<InputMethodProfile_TouchScreen*>(profile);
}

bool InputMethodType_TouchScreen::RumbleSupported()
{
    return true;
}

bool InputMethodType_TouchScreen::ConsumeEvent(const SDL_Event* ev)
{
    // update the touch devices count as needed
    if(ev->type == SDL_FINGERDOWN && !this->m_controller.touchSupported())
        this->m_controller.scanTouchDevices();

    return false;
}

void InputMethodType_TouchScreen::UpdateControlsPre()
{
    this->m_controller.update();
}

void InputMethodType_TouchScreen::UpdateControlsPost()
{
    bool method_exists = false;

    for(InputMethod* m : g_InputMethods)
    {
        if(m)
        {
            method_exists = true;
            break;
        }
    }

    // deactivate the touchscreen rendering when there is no longer an active touchscreen,
    //   but some other input method exists (ie, not the transition between menu and player add)
    if(method_exists && this->m_controller.m_active_method == nullptr && g_renderTouchscreen)
        g_renderTouchscreen = false;
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
            n_touchscreens ++;
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
    {
        this->m_controller.resetState();
        SharedCursor.GoOffscreen();
    }

    method->Name = this->LocalName();
    method->Type = this;

    this->m_controller.m_active_method = method;
    g_renderTouchscreen = true;

    return (InputMethod*)method;
}

TouchScreenController::ExtraKeys_t& TouchScreenController::ExtraKeys_t::operator|=(const ExtraKeys_t& o)
{
    keyToggleView |= o.keyToggleView;
    keyToggleViewOnce |= o.keyToggleViewOnce;

    keyHoldRun |= o.keyHoldRun;
    keyHoldRunOnce |= o.keyHoldRunOnce;

    keyRunOnce |= o.keyRunOnce;
    keyAltRunOnce |= o.keyAltRunOnce;

    keyCheats |= o.keyCheats;

    return *this;
}

} // namespace Controls
