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

#include <SDL2/SDL_version.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_haptic.h>

#include <fmt_format_ne.h>

#include <unordered_map>
#include <Logger/logger.h>

#include "../globals.h"
#include "../sound.h"
#include "joystick.h"
#include "../pseudo_vb.h"
#include "../main/speedrunner.h"
#include "../main/menu_main.h"
#include "../main/record.h"

#ifdef USE_TOUCHSCREEN_CONTROLLER
#include "touchscreen.h"
#endif


// this module handles the players controls, both keyboard and joystick

struct Joystick_t
{
    SDL_Joystick        *joystick = nullptr;
    SDL_GameController  *control = nullptr;
    SDL_Haptic          *haptic = nullptr;
    SDL_JoystickID      id = -1;
    std::string         name;
};

static std::vector<Joystick_t> s_joysticks;

static std::unordered_map<std::string, ConJoystick_t> s_joystickControls[maxLocalPlayers];
static std::unordered_map<SDL_JoystickID, int> s_joystickMap;

#ifdef USE_TOUCHSCREEN_CONTROLLER
static TouchScreenController      s_touch;
const Controls_t &CurrentTouchControls()
{
    return s_touch.m_current_keys;
}
#endif

void joyFillDefaults(ConKeyboard_t &k)
{
    k.Down = vbKeyDown;
    k.Left = vbKeyLeft;
    k.Up = vbKeyUp;
    k.Right = vbKeyRight;
    k.Jump = vbKeyZ;
    k.Run = vbKeyX;
    k.Drop = vbKeyShift;
    k.Start = vbKeyEscape;
    k.AltJump = vbKeyA;
    k.AltRun = vbKeyS;
}

void joyFillDefaults(ConJoystick_t &j)
{
    j.Up.val = SDL_HAT_UP;
    j.Up.type = ConJoystick_t::JoyHat;
    j.Up.id = 0;
    j.Up.ctrl_val = 1;
    j.Up.ctrl_type = ConJoystick_t::CtrlButton;
    j.Up.ctrl_id = SDL_CONTROLLER_BUTTON_DPAD_UP;

    j.Down.val = SDL_HAT_DOWN;
    j.Down.type = ConJoystick_t::JoyHat;
    j.Down.id = 0;
    j.Down.ctrl_val = 1;
    j.Down.ctrl_type = ConJoystick_t::CtrlButton;
    j.Down.ctrl_id = SDL_CONTROLLER_BUTTON_DPAD_DOWN;

    j.Left.val = SDL_HAT_LEFT;
    j.Left.id = 0;
    j.Left.type = ConJoystick_t::JoyHat;
    j.Left.ctrl_val = 1;
    j.Left.ctrl_type = ConJoystick_t::CtrlButton;
    j.Left.ctrl_id = SDL_CONTROLLER_BUTTON_DPAD_LEFT;

    j.Right.val = SDL_HAT_RIGHT;
    j.Right.type = ConJoystick_t::JoyHat;
    j.Right.id = 0;
    j.Right.ctrl_val = 1;
    j.Right.ctrl_type = ConJoystick_t::CtrlButton;
    j.Right.ctrl_id = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

    j.Run.id = 2;
    j.Run.val = 1;
    j.Run.type = ConJoystick_t::JoyButton;
    j.Run.ctrl_val = 1;
    j.Run.ctrl_type = ConJoystick_t::CtrlButton;
    j.Run.ctrl_id = SDL_CONTROLLER_BUTTON_X;

    j.AltRun.id = 3;
    j.AltRun.val = 1;
    j.AltRun.type = ConJoystick_t::JoyButton;
    j.AltRun.ctrl_val = 1;
    j.AltRun.ctrl_type = ConJoystick_t::CtrlButton;
    j.AltRun.ctrl_id = SDL_CONTROLLER_BUTTON_Y;

    j.Jump.id = 0;
    j.Jump.val = 1;
    j.Jump.type = ConJoystick_t::JoyButton;
    j.Jump.ctrl_val = 1;
    j.Jump.ctrl_type = ConJoystick_t::CtrlButton;
    j.Jump.ctrl_id = SDL_CONTROLLER_BUTTON_A;

    j.AltJump.id = 1;
    j.AltJump.val = 1;
    j.AltJump.type = ConJoystick_t::JoyButton;
    j.AltJump.ctrl_val = 1;
    j.AltJump.ctrl_type = ConJoystick_t::CtrlButton;
    j.AltJump.ctrl_id = SDL_CONTROLLER_BUTTON_B;

    j.Drop.id = 6;
    j.Drop.val = 1;
    j.Drop.type = ConJoystick_t::JoyButton;
    j.Drop.ctrl_val = 1;
    j.Drop.ctrl_type = ConJoystick_t::CtrlButton;
    j.Drop.ctrl_id = SDL_CONTROLLER_BUTTON_BACK;

    j.Start.id = 7;
    j.Start.val = 1;
    j.Start.type = ConJoystick_t::JoyButton;
    j.Start.ctrl_val = 1;
    j.Start.ctrl_type = ConJoystick_t::CtrlButton;
    j.Start.ctrl_id = SDL_CONTROLLER_BUTTON_START;
}

static std::string getJoyUuidStr(SDL_Joystick *j)
{
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(j);
    std::string guidStr;
    guidStr.resize(32);
    SDL_JoystickGetGUIDString(guid, &guidStr[0], 33);
    return guidStr;
}

std::string joyGetUuidStr(int joystick)
{
    if(joystick < 0 || joystick >= int(s_joysticks.size()))
        return std::string();
    return getJoyUuidStr(s_joysticks[joystick].joystick);
}

std::string joyGetName(int joystick)
{
    if(joystick < 0 || joystick >= int(s_joysticks.size()))
        return fmt::format_ne("Joystick {0}", joystick);

    const char *n;
    auto &j = s_joysticks[joystick];

//    if(j.control)
//        n = SDL_GameControllerName(j.control);
//    else
    n = SDL_JoystickName(j.joystick);

    if(!n)
    {
        if(j.control)
            return fmt::format_ne("Controller {0}", joystick + 1);
        else
            return fmt::format_ne("Joystick {0}", joystick + 1);
    }

    return fmt::format("{0}: {1}", joystick + 1, n);
}

static int s_joyDeviceAdd(int i)
{
    Joystick_t joy;
    SDL_JoystickID instance = SDL_JoystickGetDeviceInstanceID(i);

    if(instance < 0)
    {
        pLogWarning("Joystick %d: Failed to get ID of the controller", i);
        return -1;
    }

    if(s_joystickMap.find(instance) != s_joystickMap.end())
    {
        pLogWarning("Joystick %d: Attempted to add the duplicated controller instance", i);
        return -1;
    }

    joy.joystick = SDL_JoystickOpen(i);

    if(joy.joystick)
    {
        joy.id = SDL_JoystickInstanceID(joy.joystick);
        std::string guidStr = getJoyUuidStr(joy.joystick);
        bool exist1 = s_joystickControls[0].find(guidStr) != s_joystickControls[0].end();
        bool exist2 = s_joystickControls[1].find(guidStr) != s_joystickControls[1].end();
        auto &j = s_joystickControls[0][guidStr];

//            j.hwGUID = guidStr;
        j.isValid = true;
        j.isGameController = SDL_IsGameController(i);
        j.isHaptic = SDL_JoystickIsHaptic(joy.joystick);
        if(!exist1)
            joyFillDefaults(j);

        joy.name = SDL_JoystickName(joy.joystick);

        pLogDebug("==========================");
        pLogDebug("Josytick available: %s", joy.name.c_str());
        pLogDebug("--------------------------");
        pLogDebug("GUID:    %s", guidStr.c_str());
        pLogDebug("Axes:    %d", SDL_JoystickNumAxes(joy.joystick));
        pLogDebug("Balls:   %d", SDL_JoystickNumBalls(joy.joystick));
        pLogDebug("Hats:    %d", SDL_JoystickNumHats(joy.joystick));
        pLogDebug("Buttons: %d", SDL_JoystickNumButtons(joy.joystick));
        if(j.isGameController)
            pLogDebug("Supported by the game controller interface!");
        if(j.isHaptic)
            pLogDebug("Is the haptic device!");

        if(j.isGameController)
        {
            joy.control = SDL_GameControllerOpen(i);
            if(!joy.control)
            {
                pLogWarning("Couldn't open the game controller %d, using as a joystick!", i);
                j.isGameController = false;
            }
            else
                pLogDebug("Controller: %s", SDL_GameControllerName(joy.control));
        }

        if(j.isHaptic)
        {
            joy.haptic = SDL_HapticOpenFromJoystick(joy.joystick);
            if(joy.haptic)
            {
                if(SDL_HapticRumbleSupported(joy.haptic) && SDL_HapticRumbleInit(joy.haptic) != 0)
                {
                    pLogWarning("Couldn't open the rumble at the haptic device %d, disabling the haptic support!", i);
                    SDL_HapticClose(joy.haptic);
                    j.isHaptic = false;
                }
            }
            else
            {
                pLogWarning("Couldn't open the haptic device %d, disabling the haptic support!", i);
                j.isHaptic = false;
            }
        }

        pLogDebug("==========================");

        if(exist2)
        {
            // Don't override controls when model entry is already loaded
            auto &j2 = s_joystickControls[1][guidStr];
            j2.isHaptic = j.isHaptic;
            j2.isGameController = j.isGameController;
            j2.isValid = j.isValid;
        }
        else
            s_joystickControls[1][guidStr] = j;

        int objIdx = (int)s_joysticks.size();
        s_joysticks.push_back(joy);
        s_joystickMap.emplace(joy.id, objIdx);
        return objIdx;
    }
    else
    {
        pLogWarning("==========================");
        pLogWarning("Can't open joystick #%d", i);
        pLogWarning("==========================");
        return -1;
    }
}

static void s_joyDeviceClose(int i)
{
    auto j = s_joysticks[i];
    if(j.control)
        SDL_GameControllerClose(j.control);
    if(j.haptic)
        SDL_HapticClose(j.haptic);
    if(j.joystick)
        SDL_JoystickClose(j.joystick);
}

void joyDeviceAddEvent(const SDL_JoyDeviceEvent *e)
{
    pLogDebug("Connected game device: %s (index %d)", SDL_JoystickNameForIndex(e->which), e->which);

    int idx;
    if((idx = s_joyDeviceAdd(e->which)) >= 0)
    {
        numJoysticks++; // Do count the successfully loaded devices only

        for(int i = 1; i <= maxLocalPlayers; i++)
        {
            // Return back the lost joystick to the player if they wanted it
            if(useJoystick[i] <= 0 && !wantedKeyboard[i])
            {
                useJoystick[i] = (idx + 1);
                if(useJoystick[i] >= 0)
                    joyGetByIndex(i, idx, conJoystick[i]);
                PlaySoundMenu(SFX_Yoshi);
                break;
            }
        }
    }
}

void joyDeviceRemoveEvent(const SDL_JoyDeviceEvent *e)
{
    auto i = s_joystickMap.find(e->which);
    if(i == s_joystickMap.end())
    {
        pLogWarning("Attempt to disconnect the dead device: %d", e->which);
        return;
    }

    int idx = i->second;

    pLogDebug("Disconnected game device: %s, index %d instance %d", s_joysticks[idx].name.c_str(), idx, e->which);

    PlaySoundMenu(SFX_SMGlass);

    if(GameMenu && getNewJoystick) // Cancel the key binding if device got disconnected in the middle of the key awaiting
    {
        auto &cj = conJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE];
        setKey(cj, MenuCursor, lastJoyButton);
        getNewJoystick = false;
        MenuCursorCanMove = false;
    }

    for(int p = 1; p <= maxLocalPlayers; ++p)
    {
        if(useJoystick[p] > 0)
        {
            int uj = useJoystick[p] - 1;
            if(uj == idx)
                useJoystick[p] = 0; // Drop player back into keyboard
            else if(uj > idx)
                useJoystick[p]--; // Removal of the device will cause the offset of all next devices down
        }
    }

    s_joyDeviceClose(idx);

    // Reduce all index numbers that got an offset
    for(auto &m : s_joystickMap)
    {
        if(m.second > idx)
            m.second--;
    }

    s_joystickMap.erase(e->which);
    s_joysticks.erase(s_joysticks.begin() + idx);
    numJoysticks--;
}

int joyCount()
{
    return (int)s_joysticks.size();
}

void joyGetAllUUIDs(int player, std::vector<std::string> &out)
{
    SDL_assert(player >= 1 && player <= maxLocalPlayers);
    out.clear();
    for(auto &q : s_joystickControls[player - 1])
        out.push_back(q.first);
}

ConJoystick_t &joyGetByUuid(int player, const std::string &uuid)
{
    SDL_assert(player >= 1 && player <= maxLocalPlayers);
    return s_joystickControls[player - 1][uuid];
}

void joyGetByUuid(ConJoystick_t &dst, int player, const std::string &uuid)
{
    auto &j = joyGetByUuid(player, uuid);
    SDL_memcpy(&dst, &j, sizeof(ConJoystick_t));
}

ConJoystick_t &joyGetByIndex(int player, int joyNum)
{
    SDL_assert(joyNum >= 0 && (size_t)joyNum < s_joysticks.size());
    SDL_assert(player >= 1 && player <= maxLocalPlayers);

    std::string guidStr = getJoyUuidStr(s_joysticks[joyNum].joystick);
    auto &ret = s_joystickControls[player - 1][guidStr];
//    if(ret.hwGUID.empty() && ret.hwGUID != guidStr)
//        ret.hwGUID = guidStr;
    return ret;
}

void joyGetByIndex(int player, int joyNum, ConJoystick_t &dst)
{
    SDL_assert(joyNum >= 0 && (size_t)joyNum < s_joysticks.size());
    SDL_assert(player >= 1 && player <= maxLocalPlayers);

    std::string guidStr = getJoyUuidStr(s_joysticks[joyNum].joystick);
    auto &ret = s_joystickControls[player - 1][guidStr];
    SDL_memcpy(&dst, &ret, sizeof(ConJoystick_t));
}


void joySetByUuid(int player, const std::string &uuid, const ConJoystick_t &cj)
{
    SDL_assert(player >= 1 && player <= maxLocalPlayers);
    SDL_memcpy(&s_joystickControls[player - 1][uuid], &cj, sizeof(ConJoystick_t));
}

void joySetByIndex(int player, int index, const ConJoystick_t &cj)
{
    auto u = joyGetUuidStr(index);
    joySetByUuid(player, u, cj);
}

int joyGetPowerLevel(int joyNum)
{
    SDL_assert(joyNum >= 0 && (size_t)joyNum < s_joysticks.size());
    auto *j = s_joysticks[joyNum].joystick;
    if(j)
        return SDL_JoystickCurrentPowerLevel(j);
    return SDL_JOYSTICK_POWER_UNKNOWN;
}

void joyRumble(int joyNum, int ms, float strength)
{
    if(!JoystickEnableRumble || GameMenu || GameOutro)
        return;

    SDL_assert(joyNum >= 0 && (size_t)joyNum < s_joysticks.size());
    auto &j = s_joysticks[joyNum];

    if(j.haptic)
    {
        if(SDL_HapticRumblePlay(j.haptic, strength, ms) == 0)
            return;
    }

    int intStrength = (int)(0xFFFF * strength + 0.5f);

    if(intStrength > 0xFFFF)
        intStrength = 0xFFFF;

    if(intStrength < 0)
        intStrength = 0;

#if SDL_VERSION_ATLEAST(2, 0, 12)
    if(j.control)
    {
        if(SDL_GameControllerRumble(j.control, intStrength, intStrength, ms) == 0)
            return;
    }
    if(j.joystick)
        SDL_JoystickRumble(j.joystick, intStrength, intStrength, ms);
#endif
}

void joyRumbleAllPlayers(int ms, float strength)
{
    if(!JoystickEnableRumble)
        return;

    for(int plr = 1; plr <= numPlayers && plr <= maxLocalPlayers; ++plr)
    {
        if(useJoystick[plr] > 0)
            joyRumble(useJoystick[plr] - 1, ms, strength);
    }
}


static void updateJoyKey(SDL_Joystick *j, bool &key, const KM_Key &mkey)
{
    Sint32 val = 0, dx = 0, dy = 0;
    Sint16 val_initial = 0;
    bool key_new = false;

    switch(mkey.type)
    {
    case ConJoystick_t::JoyAxis:
        //Note: SDL_JoystickGetAxisInitialState is a new API function added into dev version
        //      and doesn't available in already released assemblies
        if(SDL_JoystickGetAxisInitialState(j, mkey.id, &val_initial) == SDL_FALSE)
        {
            key_new = false;
            break;
        }
        val = SDL_JoystickGetAxis(j, mkey.id);

        if(SDL_abs(val) <= 15000)
            key_new = false;
        else if(mkey.val > val_initial)
            key_new = (val > val_initial);
        else if(mkey.val < val_initial)
            key_new = (val < val_initial);
        else
            key_new = false;

        break;

    case ConJoystick_t::JoyBallX:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key_new = (dx > 0);
        else if(mkey.id < 0)
            key_new = (dx < 0);
        else key_new = false;

        break;

    case ConJoystick_t::JoyBallY:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key_new = (dy > 0);
        else if(mkey.id < 0)
            key_new = (dy < 0);
        else key_new = false;

        break;

    case ConJoystick_t::JoyHat:
        val = (Sint32)SDL_JoystickGetHat(j, mkey.id);
        key_new = ((val & mkey.val)) != 0;
        break;

    case ConJoystick_t::JoyButton:
        key_new = (0 != (Sint32)SDL_JoystickGetButton(j, mkey.id));
        break;

    default:
        key_new = false;
        break;
    }

//    key_pressed = (key_new && !key);
    key = key_new;
}

static void updateCtrlKey(SDL_GameController *j, bool &key, const KM_Key &mkey)
{
    key = (0 != (Sint32)SDL_GameControllerGetButton(j, static_cast<SDL_GameControllerButton>(mkey.ctrl_id)));
}


static void updateCtrlAxisOr(SDL_GameController *j, bool &key, SDL_GameControllerAxis axis, Sint16 dst_val)
{
    bool key_new = false;
    Sint16 val = SDL_GameControllerGetAxis(j, axis);

    dst_val *= 25000;

    if(SDL_abs(val) <= 15000)
        key_new = false;
    else if(dst_val > 0)
        key_new = (val > 0);
    else if(dst_val < 0)
        key_new = (val < 0);
    else
        key_new = false;
    key |= key_new;
}


static bool bindControllerKey(SDL_GameController *ctrl, KM_Key &k)
{
    Uint8 val;

    for(int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        val = SDL_GameControllerGetButton(ctrl, static_cast<SDL_GameControllerButton>(i));
        if(val != 0)
        {
            k.ctrl_val = val;
            k.ctrl_id = i;
            k.ctrl_type = (int)ConJoystick_t::CtrlButton;
            k.type = k.ctrl_type;
            return true;
        }
    }

    k.ctrl_val = 0;
    k.ctrl_id = -1;
    k.ctrl_type = (int)ConJoystick_t::NoControl;
    k.type = k.ctrl_type;
    return false;
}


static bool bindJoystickKey(SDL_Joystick *joy, KM_Key &k)
{
    Sint32 val = 0;
    Sint16 val_initial = 0;
    int dx = 0, dy = 0;
    //SDL_PumpEvents();
    //SDL_JoystickUpdate();
    int balls = SDL_JoystickNumBalls(joy);
    int hats = SDL_JoystickNumHats(joy);
    int buttons = SDL_JoystickNumButtons(joy);
    int axes = SDL_JoystickNumAxes(joy);

    for(int i = 0; i < buttons; i++)
    {
        val = SDL_JoystickGetButton(joy, i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyButton;
            return true;
        }
    }

    for(int i = 0; i < hats; i++)
    {
        val = SDL_JoystickGetHat(joy, i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyHat;
            return true;
        }
    }

    for(int i = 0; i < axes; i++)
    {
        //Note: The SDL 2.0.6 and higher is requires to support this function
        if(SDL_JoystickGetAxisInitialState(joy, i, &val_initial) == SDL_FALSE)
            break;

        val = SDL_JoystickGetAxis(joy, i);

        if(SDL_abs(val) > 15000 && val != (Sint32)val_initial)
        {
            k.val = val;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyAxis;
            return true;
        }
    }

    for(int i = 0; i < balls; i++)
    {
        dx = 0;
        dy = 0;
        SDL_JoystickGetBall(joy, i, &dx, &dy);

        if(dx != 0)
        {
            k.val = dx;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyBallX;
            return true;
        }
        else if(dy != 0)
        {
            k.val = dy;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyBallY;
            return true;
        }
    }

    k.val = 0;
    k.id = 0;
    k.type = (int)ConJoystick_t::NoControl;
    return false;
}

void UpdateControls()
{
    int A = 0;
//    int B = 0;
//    int C = 0;

//    If TestLevel = True And Not (nPlay.Online = True And nPlay.Mode = 0) Then
//    if(TestLevel)
//    {
//        if(getKeyState(vbKeyEscape) == KEY_PRESSED)
//        {
//            EndLevel = true;
//        }
//    }

//    if(numJoysticks > 0 && (useJoystick[1] || useJoystick[2]))
//        SDL_JoystickUpdate();

    For(B, 1, numPlayers)
    {
        if(B == 2 && numPlayers == 2) {
            A = 2;
        } else {
            A = 1;
        }

        auto &joyCon = conJoystick[A];
        auto &keyCon = conKeyboard[A];

        // With Player(A).Controls
        {
            auto &p = Player[A];
            Controls_t &c = p.Controls;
            c.Down = false;
            c.Drop = false;
            c.Jump = false;
            c.Left = false;
            c.Right = false;
            c.Run = false;
            c.Start = false;
            c.Up = false;
            c.AltJump = false;
            c.AltRun = false;

            if(useJoystick[A] > 0) // There is a joystick
            {
                int jNum = useJoystick[A] - 1;

                if(jNum < 0 || jNum >= int(s_joysticks.size()))
                    continue;

                auto &jj = s_joysticks[size_t(jNum)];

                auto *j = jj.joystick;
                auto *k = jj.control;

                if(joyCon.isGameController)
                {
// Experimental share-button key hook, not working yet
#if DEBUG_BUILD && SDL_VERSION_ATLEAST(2, 0, 14)
                    KM_Key share;
                    share.ctrl_type = ConJoystick_t::CtrlButton;
                    share.ctrl_id = SDL_CONTROLLER_BUTTON_MISC1;
                    share.ctrl_val = 1;
                    static bool take = false;
                    bool takePrev = take;
                    updateCtrlKey(k, take, share);

                    if(take != takePrev && take)
                        TakeScreen = true;
#endif

                    updateCtrlKey(k, c.Up, joyCon.Up);
                    updateCtrlKey(k, c.Down, joyCon.Down);
                    updateCtrlKey(k, c.Left, joyCon.Left);
                    updateCtrlKey(k, c.Right, joyCon.Right);
                    updateCtrlKey(k, c.Run, joyCon.Run);
                    updateCtrlKey(k, c.AltRun, joyCon.AltRun);
                    updateCtrlKey(k, c.Jump, joyCon.Jump);
                    updateCtrlKey(k, c.AltJump, joyCon.AltJump);
                    updateCtrlKey(k, c.Drop, joyCon.Drop);
                    updateCtrlKey(k, c.Start, joyCon.Start);
                    // Use analog stick as an additional move controller
                    updateCtrlAxisOr(k, c.Up, SDL_CONTROLLER_AXIS_LEFTY, -1);
                    updateCtrlAxisOr(k, c.Down, SDL_CONTROLLER_AXIS_LEFTY, +1);
                    updateCtrlAxisOr(k, c.Left, SDL_CONTROLLER_AXIS_LEFTX, -1);
                    updateCtrlAxisOr(k, c.Right, SDL_CONTROLLER_AXIS_LEFTX, +1);
                }
                else
                {
                    updateJoyKey(j, c.Up, joyCon.Up);
                    updateJoyKey(j, c.Down, joyCon.Down);
                    updateJoyKey(j, c.Left, joyCon.Left);
                    updateJoyKey(j, c.Right, joyCon.Right);
                    updateJoyKey(j, c.Run, joyCon.Run);
                    updateJoyKey(j, c.AltRun, joyCon.AltRun);
                    updateJoyKey(j, c.Jump, joyCon.Jump);
                    updateJoyKey(j, c.AltJump, joyCon.AltJump);
                    updateJoyKey(j, c.Drop, joyCon.Drop);
                    updateJoyKey(j, c.Start, joyCon.Start);
                }
            }

            if(useJoystick[A] == 0) // Keyboard controls
            {
                if(getKeyStateI(keyCon.Up)) {
                    c.Up = true;
                }

                if(getKeyStateI(keyCon.Down)) {
                    c.Down = true;
                }

                if(getKeyStateI(keyCon.Left)) {
                    c.Left = true;
                }

                if(getKeyStateI(keyCon.Right)) {
                    c.Right = true;
                }

                if(getKeyStateI(keyCon.Jump)) {
                    c.Jump = true;
                }

                if(getKeyStateI(keyCon.Run)) {
                    c.Run = true;
                }

                if(getKeyStateI(keyCon.Drop)) {
                    c.Drop = true;
                }

                if(getKeyStateI(keyCon.Start)) {
                    c.Start = true;
                }

                if(getKeyStateI(keyCon.AltJump)) {
                    c.AltJump = true;
                }

                if(getKeyStateI(keyCon.AltRun)) {
                    c.AltRun = true;
                }
            }

            /* // DEAD CODE
//            If .Left = True And .Right = True Then
            if(c.Left And c.Right) {
//                .Left = False
                c.Left = False;
//                .Right = False
                c.Right = False;
//            End If
            }*/
#ifdef USE_TOUCHSCREEN_CONTROLLER
            // Mix controls of a touch screen with a player 1
            if(A == 1)
            {
                s_touch.update();
                if(!s_touch.m_touchHidden)
                {
                    auto &t = s_touch.m_current_keys;
                    auto &te = s_touch.m_current_extra_keys;
                    c.Down |= t.Down;
                    c.Drop |= t.Drop;
                    c.Jump |= t.Jump;
                    c.Left |= t.Left;
                    c.Right |= t.Right;
                    c.Run |= t.Run;
                    c.Start |= t.Start;
                    c.Up |= t.Up;
                    c.AltJump |= t.AltJump;
                    c.AltRun |= t.AltRun;

                    if(!GamePaused && !GameMenu && !GameOutro && !LevelSelect)
                    {
                        if(s_touch.m_runHeld && c.AltRun)
                        {
                            if(te.keyAltRunOnce)
                                c.AltRun = false;
                            c.Run = false;
                        }
                        else if(s_touch.m_runHeld && !te.keyRunOnce)
                            c.Run |= true;
                        else if(te.keyRunOnce && c.Run)
                            c.Run = false;
                    }
                } // s_touch.m_touchHidden
            }
#endif
        }
    }

    // Push the controls state into the speed-runner to properly display
    record_sync();

    speedRun_syncControlKeys(0, Player[1].Controls);
    if(numPlayers == 2)
        speedRun_syncControlKeys(1, Player[2].Controls);

    For(B, 1, numPlayers)
    {
        if(B == 2 && numPlayers == 2) {
            A = 2;
        } else {
            A = 1;
        }

        // With Player(A).Controls
        {
            auto &p = Player[A];
            Controls_t &c = p.Controls;

            if(!c.Start && !c.Jump) {
                p.UnStart = true;
            }

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
            if(ForcedControls && !GamePaused)
            {
                c = ForcedControl;
            }
        } // End With
    }

    if(SingleCoop > 0)
    {
        if(numPlayers == 1 || numPlayers > 2)
            SingleCoop = 0;

        Controls_t tempControls;
        if(SingleCoop == 1) {
            Player[2].Controls = tempControls;
        } else {
            Player[2].Controls = Player[1].Controls;
            Player[1].Controls = tempControls;
        }
    }

    // !UNNEEDED DEAD CODE
//    If nPlay.Online = True Then
//        Player(nPlay.MySlot + 1).Controls = Player(1).Controls
//        If Not (nPlay.MyControls.AltJump = Player(1).Controls.AltJump And nPlay.MyControls.AltRun = Player(1).Controls.AltRun And nPlay.MyControls.Down = Player(1).Controls.Down And nPlay.MyControls.Drop = Player(1).Controls.Drop And nPlay.MyControls.Jump = Player(1).Controls.Jump And nPlay.MyControls.Left = Player(1).Controls.Left And nPlay.MyControls.Right = Player(1).Controls.Right And nPlay.MyControls.Run = Player(1).Controls.Run And nPlay.MyControls.Start = Player(1).Controls.Start And nPlay.MyControls.Up = Player(1).Controls.Up) Then
//            nPlay.MyControls = Player(1).Controls
//            nPlay.Player(nPlay.MySlot).Controls = Player(1).Controls
//            If Player(nPlay.MySlot + 1).Dead = False And Player(nPlay.MySlot + 1).TimeToLive = 0 Then Netplay.sendData Netplay.PutPlayerControls(nPlay.MySlot)
//        Else
//            nPlay.MyControls = Player(1).Controls
//            nPlay.Player(nPlay.MySlot).Controls = Player(1).Controls
//        End If
//        For A = 0 To numPlayers - 1
//            If nPlay.Player(A).Active = True And A <> nPlay.MySlot Then
//                Player(A + 1).Controls = nPlay.Player(A).Controls
//            End If
//        Next A
//    End If

    For(A, 1, numPlayers)
    {
        {
            Player_t &p = Player[A];
            if(p.SpinJump)
            {
                if(p.SpinFrame < 4 || p.SpinFrame > 9) {
                    p.Direction = -1;
                } else {
                    p.Direction = 1;
                }
            }
        }
    }
}


int joyInitJoysticks()
{
#ifdef USE_TOUCHSCREEN_CONTROLLER
    s_touch.init();
#endif

    SDL_JoystickEventState(SDL_ENABLE);
    int num = SDL_NumJoysticks();

    for(int i = 0; i < num; ++i)
        s_joyDeviceAdd(i);

    return int(s_joysticks.size());
}


bool joyStartJoystick(int JoystickNumber)
{
    if(JoystickNumber < 0 || JoystickNumber >= int(s_joysticks.size()))
        return false;

    auto &joy = s_joysticks[size_t(JoystickNumber)];
    if(joy.joystick)
    {
        pLogDebug("==========================");
        pLogDebug("Josytick started: %s", SDL_JoystickName(joy.joystick));
        pLogDebug("==========================");
        return true;
    }
    else
    {
        pLogWarning("==========================");
        pLogWarning("Can't open joystick #%d", JoystickNumber);
        pLogWarning("==========================");
        return false;
    }
}

void joyCloseJoysticks()
{
#ifdef USE_TOUCHSCREEN_CONTROLLER
    s_touch.quit();
#endif

    for(size_t i = 0; i < s_joysticks.size(); ++i) // scan hats first
        s_joyDeviceClose(i);

    s_joysticks.clear();
}

bool joyPollJoystick(int joystick, KM_Key &key)
{
    if(joystick < 0 || joystick >= int(s_joysticks.size()))
        return false;

    auto &j = s_joysticks[size_t(joystick)];

    if(j.control)
        return bindControllerKey(j.control, key);

    return bindJoystickKey(j.joystick, key);
}

bool joyIsKeyDown(int JoystickNumber, const KM_Key &key)
{
    bool val = false;

    if(JoystickNumber < 0 || JoystickNumber >= int(s_joysticks.size()))
        return false;

    auto &j = s_joysticks[size_t(JoystickNumber)];
    if(j.control)
        updateCtrlKey(j.control, val, key);
    else
        updateJoyKey(j.joystick, val, key);
    return val;
}

#ifdef USE_TOUCHSCREEN_CONTROLLER
void RenderTouchControls()
{
    s_touch.render();
}

void UpdateTouchScreenSize()
{
    s_touch.updateScreenSize();
}
#endif

void setKey(ConKeyboard_t &ck, int id, int val)
{
    switch(id)
    {
    case 1:
        ck.Up = val;
        break;
    case 2:
        ck.Down = val;
        break;
    case 3:
        ck.Left = val;
        break;
    case 4:
        ck.Right = val;
        break;
    case 5:
        ck.Run = val;
        break;
    case 6:
        ck.AltRun = val;
        break;
    case 7:
        ck.Jump = val;
        break;
    case 8:
        ck.AltJump = val;
        break;
    case 9:
        ck.Drop = val;
        break;
    case 10:
        ck.Start = val;
        break;
    }
}

void setKey(ConJoystick_t &cj, int id, const KM_Key &val)
{
    switch(id)
    {
    case 1:
        cj.Up = val;
        break;
    case 2:
        cj.Down = val;
        break;
    case 3:
        cj.Left = val;
        break;
    case 4:
        cj.Right = val;
        break;
    case 5:
        cj.Run = val;
        break;
    case 6:
        cj.AltRun = val;
        break;
    case 7:
        cj.Jump = val;
        break;
    case 8:
        cj.AltJump = val;
        break;
    case 9:
        cj.Drop = val;
        break;
    case 10:
        cj.Start = val;
        break;
    }
}

KM_Key &getKey(ConJoystick_t &cj, int id)
{
    switch(id)
    {
    case 1:
    default:
        return cj.Up;
        break;
    case 2:
        return cj.Down;
        break;
    case 3:
        return cj.Left;
        break;
    case 4:
        return cj.Right;
        break;
    case 5:
        return cj.Run;
        break;
    case 6:
        return cj.AltRun;
        break;
    case 7:
        return cj.Jump;
        break;
    case 8:
        return cj.AltJump;
        break;
    case 9:
        return cj.Drop;
        break;
    case 10:
        return cj.Start;
        break;
    }
}
