/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

// #include <Logger/logger.h>

#include "globals.h"
#include "../control/joystick.h"

#include "pseudo_vb.h"

#include <3ds.h>

/*
static void updateJoyKey(uint32_t keys, bool &key, const KM_Key &mkey)
{
    key = mkey.id & keys;
}*/

void joyFillDefaults(ConKeyboard_t &k) {}
void editorJoyFillDefaults(EditorConKeyboard_t &k) {}

void joyFillDefaults(ConJoystick_t &j)
{
    j.Up.id = KEYCODE_PAD_UP;
    j.Up.val = KEYID_PAD_UP;
    j.Up.type = ConJoystick_t::JoyButton;

    j.Down.id = KEYCODE_PAD_DOWN;
    j.Down.val = KEYID_PAD_DOWN;
    j.Down.type = ConJoystick_t::JoyButton;

    j.Left.id = KEYCODE_PAD_LEFT;
    j.Left.val = KEYID_PAD_LEFT;
    j.Left.type = ConJoystick_t::JoyButton;

    j.Right.id = KEYCODE_PAD_RIGHT;
    j.Right.val = KEYID_PAD_RIGHT;
    j.Right.type = ConJoystick_t::JoyButton;

    j.Run.id = KEYCODE_B;
    j.Run.val = KEYID_B;
    j.Run.type = ConJoystick_t::JoyButton;

    j.AltRun.id = KEYCODE_Y;
    j.AltRun.val = KEYID_Y;
    j.AltRun.type = ConJoystick_t::JoyButton;

    j.Jump.id = KEYCODE_A;
    j.Jump.val = KEYID_A;
    j.Jump.type = ConJoystick_t::JoyButton;

    j.AltJump.id = KEYCODE_X;
    j.AltJump.val = KEYID_X;
    j.AltJump.type = ConJoystick_t::JoyButton;

    j.Drop.id = KEYCODE_SELECT;
    j.Drop.val = KEYID_SELECT;
    j.Drop.type = ConJoystick_t::JoyButton;

    j.Start.id = KEYCODE_START;
    j.Start.val = KEYID_START;
    j.Start.type = ConJoystick_t::JoyButton;
}

void editorJoyFillDefaults(EditorConJoystick_t &j)
{
    j.ScrollUp.id = KEYCODE_PAD_UP;
    j.ScrollUp.val = KEYID_PAD_UP;
    j.ScrollUp.type = EditorConJoystick_t::JoyButton;

    j.ScrollDown.id = KEYCODE_PAD_DOWN;
    j.ScrollDown.val = KEYID_PAD_DOWN;
    j.ScrollDown.type = EditorConJoystick_t::JoyButton;

    j.ScrollLeft.id = KEYCODE_PAD_LEFT;
    j.ScrollLeft.val = KEYID_PAD_LEFT;
    j.ScrollLeft.type = EditorConJoystick_t::JoyButton;

    j.ScrollRight.id = KEYCODE_PAD_RIGHT;
    j.ScrollRight.val = KEYID_PAD_RIGHT;
    j.ScrollRight.type = EditorConJoystick_t::JoyButton;

    j.PrevSection.id = KEYCODE_L;
    j.PrevSection.val = KEYID_L;
    j.PrevSection.type = ConJoystick_t::JoyButton;

    j.NextSection.id = KEYCODE_R;
    j.NextSection.val = KEYID_R;
    j.NextSection.type = ConJoystick_t::JoyButton;

    j.SwitchScreens.id = KEYCODE_SELECT;
    j.SwitchScreens.val = KEYID_SELECT;
    j.SwitchScreens.type = ConJoystick_t::JoyButton;

    j.TestPlay.id = KEYCODE_START;
    j.TestPlay.val = KEYID_START;
    j.TestPlay.type = ConJoystick_t::JoyButton;

    j.Select.id = KEYCODE_DUP;
    j.Select.val = KEYID_DUP;
    j.Select.type = ConJoystick_t::JoyButton;

    j.Erase.id = KEYCODE_DDOWN;
    j.Erase.val = KEYID_DDOWN;
    j.Erase.type = ConJoystick_t::JoyButton;
}

static bool bindJoystickKey(KM_Key &k)
{
    for(int i = 0; i < 32; i++)
    {
        if (frmMain.getKeyPressed(1<<i)) {
            k.val = i;
            k.id = 1<<i;
            k.type = (int)ConJoystick_t::JoyButton;
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

    For(B, 1, numPlayers)
    {
        if(B == 2 && numPlayers == 2) {
            A = 2;
        } else {
            A = 1;
        }

        {
            Controls_t &c = Player[A].Controls;
            c.Down = frmMain.getKeyHeld(conJoystick[A].Down.id);
            c.Drop = frmMain.getKeyHeld(conJoystick[A].Drop.id);
            c.Jump = frmMain.getKeyHeld(conJoystick[A].Jump.id);
            c.Left = frmMain.getKeyHeld(conJoystick[A].Left.id);
            c.Right = frmMain.getKeyHeld(conJoystick[A].Right.id);
            c.Run = frmMain.getKeyHeld(conJoystick[A].Run.id);
            c.Start = frmMain.getKeyHeld(conJoystick[A].Start.id);
            c.Up = frmMain.getKeyHeld(conJoystick[A].Up.id);
            c.AltJump = frmMain.getKeyHeld(conJoystick[A].AltJump.id);
            c.AltRun = frmMain.getKeyHeld(conJoystick[A].AltRun.id);

            if(!Player[A].Controls.Start && !Player[A].Controls.Jump) {
                Player[A].UnStart = true;
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

            if(!(Player[A].State == 5 && Player[A].Mount == 0) && c.AltRun)
                c.Run = true;
            if(ForcedControls && !GamePaused )
            {
                Player[A].Controls = ForcedControl;
            }
        }
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

void UpdateEditorControls()
{
    EditorControls_t &c = EditorControls;
    EditorConJoystick_t &j = editorConJoystick;
    c.FastScroll = frmMain.getKeyHeld(j.FastScroll.id);
    c.ScrollUp = frmMain.getKeyHeld(j.ScrollUp.id);
    c.ScrollDown = frmMain.getKeyHeld(j.ScrollDown.id);
    c.ScrollLeft = frmMain.getKeyHeld(j.ScrollLeft.id);
    c.ScrollRight = frmMain.getKeyHeld(j.ScrollRight.id);

    c.PrevSection = frmMain.getKeyHeld(j.PrevSection.id);
    c.NextSection = frmMain.getKeyHeld(j.NextSection.id);

    c.SwitchScreens = frmMain.getKeyHeld(j.SwitchScreens.id);
    c.TestPlay = frmMain.getKeyHeld(j.TestPlay.id);
    c.Select = frmMain.getKeyHeld(j.Select.id);
    c.Erase = frmMain.getKeyHeld(j.Erase.id);
}

bool PollJoystick(KM_Key &key)
{
    return bindJoystickKey(key);
}

bool joyPollJoystick(int, KM_Key &key)
{
    return bindJoystickKey(key);
}

bool joyIsKeyDown(int JoystickNumber, const KM_Key &key)
{
    return frmMain.getKeyHeld(key.id);
}

int joyInitJoysticks() {return 0;}
void joyGetAllUUIDs(int player, std::vector<std::string> &out)
{
    out.clear();
}

bool joyStartJoystick(int) { return true;}
int joyCount() {return 1;}
ConJoystick_t &joyGetByUuid(int player, const std::string &uuid) {return conJoystick[player];}
void joyGetByUuid(ConJoystick_t &dst, int player, const std::string &uuid) {}
ConJoystick_t &joyGetByIndex(int player, int joyNum) {return conJoystick[player];}
void joyGetByIndex(int player, int joyNum, ConJoystick_t &dst) {}
void joySetByUuid(int player, const std::string &uuid, const ConJoystick_t &cj) {}
void joySetByIndex(int player, int index, const ConJoystick_t &cj) {}

int joyGetPowerLevel(int joyNum) {return 0;}
void joyRumble(int joyNum, int ms, float strength) {}
void joyRumbleAllPlayers(int ms, float strength) {}

std::string joyGetUuidStr(int joystick) {return "";}
std::string joyGetName(int joystick) {return "";}

void joyDeviceAddEvent(const SDL_JoyDeviceEvent *e) {}
void joyDeviceRemoveEvent(const SDL_JoyDeviceEvent *e) {}

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
