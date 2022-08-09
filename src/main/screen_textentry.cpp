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

#include "../globals.h"
#include "../graphics.h"
#include "../config.h"
#include "core/render.h"
#include "../game_main.h"
#include "controls.h"
#include "gfx.h"

#ifdef __ANDROID__

#   include <SDL2/SDL_system.h>
#   include <jni.h>
#   if 1
#       undef JNIEXPORT
#       undef JNICALL
#       define JNIEXPORT extern "C"
#       define JNICALL
#   endif

static void s_textEntry_callDialog()
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(clazz, "requestText", "()V");
    env->CallVoidMethod(activity, method);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);
}

#endif // #ifdef __ANDROID__

namespace TextEntryScreen
{

std::string Text;
static std::string s_Prompt;
static int s_cursor = 0;
static int s_mouse_up = 2;
static int s_timer = 0;
static bool s_committed = false;

// KEYMAP!
// we are assumed to be operating on a Nx5x12 (levels, rows, columns) grid.
// special characters: \b is backspace, \x0e and \x0f are shift up and shift down (a grid level)
// you will likely produce a segfault if you allow the user to shift into a grid level that you did not define.
// \n is a newline (actually, accept), and \x1d and \x1c are left and right.
// special control character meanings: \x11 is "last button wider" and \x12 is "empty"

// the UTF-8 character length encoding is not supported yet, but is being explored.
// first bit unset: single char
// first two bits set: 2 chars / one character
// first three bits set: 3 chars / one character
// first four bits set: 4 chars / one character
static const char* s_keymap_US = "1234567890-\b"
    "qwertyuiop[]"
    "asdfghjkl;'="
    "`zxcvbnm,./\\"
    "\x0e\x11\x11 \x11\x11\x11\x11\x1d\x1c\n\x11"
    "!@#$%^&*()_\b"
    "QWERTYUIOP{}"
    "ASDFGHJKL:\"+"
    "~ZXCVBNM<>?|"
    "\x0f\x11\x11 \x11\x11\x11\x11\x1d\x1c\n\x11";

// please help add more useful keymaps as soon as we have more printable characters.

static const char* s_current_keymap = s_keymap_US;
// will eventually be used to support UTF-8 characters
// static const char** s_current_keymap_locs;
static int s_cur_level = 0;
static int s_cur_row = 0;
static int s_cur_col = 0;
static bool s_render_sel = false;

inline const char* get_char(int level, int row, int col)
{
    return &s_current_keymap[level*5*12 + row*12 + col];
}

inline const char* get_char()
{
    return get_char(s_cur_level, s_cur_row, s_cur_col);
}

bool UpdateButton(int x, int y, int size, const char* c, bool sel, bool render)
{
    if(*c == '\x12' || *c == '\x11') // empty space or someone else's continuation
        return false;

    const char* next_char = c+1;
    char print_char[6];

    if(*c == '\b')
    {
        print_char[0] = 'B'; print_char[1] = 'S'; print_char[2] = '\0';
    }
    else if(*c == '\x0e' || *c == '\x0f')
    {
        print_char[0] = 's'; print_char[1] = 'h'; print_char[2] = 'i'; print_char[3] = 'f'; print_char[4] = 't'; print_char[5] = '\0';
    }
    else if(*c == '\n')
    {
        print_char[0] = 'o'; print_char[1] = 'k'; print_char[2] = 'a'; print_char[3] = 'y'; print_char[4] = '\0';
    }
    else if(*c == ' ')
    {
        print_char[0] = 's'; print_char[1] = 'p'; print_char[2] = 'a'; print_char[3] = 'c'; print_char[4] = 'e'; print_char[5] = '\0';
    }
    else if(*c == '\x1d')
    {
        print_char[0] = '<'; print_char[1] = '-'; print_char[2] = '\0';
    }
    else if(*c == '\x1c')
    {
        print_char[0] = '-'; print_char[1] = '>'; print_char[2] = '\0';
    }
    else if(c[0] & 1<<7)
    {
        print_char[0] = c[0];
        print_char[1] = c[1];
        next_char ++;
        if(c[0] & 1<<5)
        {
            print_char[2] = c[2];
            next_char ++;
            if(c[0] & 1<<4)
            {
                print_char[3] = c[3];
                print_char[4] = '\0';
                next_char ++;
            }
            else
                print_char[3] = '\0';
        }
        else
            print_char[2] = '\0';
    }
    else
    {
        print_char[0] = c[0];
        print_char[1] = '\0';
    }

    int width = size;

    while(*next_char == '\x11') // wider
    {
        width += size;
        next_char ++;
    }

    // the button is 36x36 and outlined by a 2 pixel box
    bool coll = false;
    if(SharedCursor.X >= x + 2 && SharedCursor.X < x + width - 2
        && SharedCursor.Y >= y + 2 && SharedCursor.Y < y + size - 2)
        coll = true;

    // outline:
    if(render)
    {
        if(sel)
        {
            if(coll && SharedCursor.Primary)
                XRender::renderRect(x, y, width, size, 0.f, 0.5f, 0.5f, 1.0f, true);
            else
                XRender::renderRect(x, y, width, size, 0.f, 1.0f, 1.0f, 1.0f, true);
        }
        else if(coll && SharedCursor.Primary)
            XRender::renderRect(x, y, width, size, 0.f, 0.f, 0.f, 1.0f, true);
        // background:
        if(SharedCursor.Primary && coll)
            XRender::renderRect(x+2, y+2, width-4, size-4, 0.2f, 0.2f, 0.2f, 1.0f, true);
        else
            XRender::renderRect(x+2, y+2, width-4, size-4, 0.5f, 0.5f, 0.5f, 0.8f, true);

        SuperPrintCenter(print_char, 4, x+width/2, y+size/2-10);
    }

    return (s_mouse_up == 1 && coll);
}

void GoLeft()
{
    int break_col = s_cur_col;
    if(s_cur_col == 0)
        s_cur_col = 11;
    else
        s_cur_col --;

    while(*get_char() == '\x11' || *get_char() == '\x12')
    {
        if(s_cur_col == 0)
            s_cur_col = 11;
        else
            s_cur_col --;
        if(s_cur_col == break_col)
            break;
    }
}

void GoRight()
{
    int break_col = s_cur_col;

    if(s_cur_col == 11)
        s_cur_col = 0;
    else
        s_cur_col ++;

    while(*get_char() == '\x11' || *get_char() == '\x12')
    {
        if(s_cur_col == 11)
            s_cur_col = 0;
        else
            s_cur_col ++;
        if(s_cur_col == break_col)
            break;
    }
}

void GoDown()
{
    int break_row = s_cur_row;

    if(s_cur_row == 4)
        s_cur_row = 0;
    else
        s_cur_row ++;

    while(*get_char() == '\x12')
    {
        if(s_cur_row == 4)
            s_cur_row = 0;
        else
            s_cur_row ++;
        if(s_cur_row == break_row)
            break;
    }

    while(*get_char() == '\x11')
        s_cur_col --;
}

void GoUp()
{
    int break_row = s_cur_row;

    if(s_cur_row == 0)
        s_cur_row = 4;
    else
        s_cur_row --;

    while(*get_char() == '\x12')
    {
        if(s_cur_row == 0)
            s_cur_row = 4;
        else
            s_cur_row --;
        if(s_cur_row == break_row)
            break;
    }

    while(*get_char() == '\x11')
        s_cur_col --;
}

void Insert(const char* c)
{
    Text.insert(s_cursor, c);
    s_cursor += XStd::strlen(c);
}

inline void InsertUnicodeChar(const char* c)
{
    char unicode[5];
    unicode[0] = c[0];

    if(c[0] & 1<<7)
    {
        unicode[1] = c[1];
        if(c[0] & 1<<5)
        {
            unicode[2] = c[2];
            if(c[0] & 1<<4)
            {
                unicode[3] = c[3];
                unicode[4] = '\0';
            }
            else
                unicode[3] = '\0';
        }
        else
            unicode[2] = '\0';
    }
    else
    {
        unicode[1] = '\0';
    }

    Insert(unicode);
}

void CursorLeft()
{
    while(s_cursor > 0)
    {
        s_cursor --;
        if(s_cursor == 0)
            break;
        // break if not a continuation char (NOT 10bbbbbb)
        if(!(Text[s_cursor] & 1<<7) || (Text[s_cursor] & 1<<6))
            break;
    }
}

void CursorRight()
{
    while(s_cursor < (int)Text.size())
    {
        s_cursor ++;
        if(s_cursor == (int)Text.size())
            break;
        // break if not a continuation char (NOT 10bbbbbb)
        if(!(Text[s_cursor] & 1<<7) || (Text[s_cursor] & 1<<6))
            break;
    }
}

void Backspace()
{
    while(s_cursor > 0)
    {
        s_cursor --;
        Text.erase(s_cursor, 1);
        if(s_cursor == 0)
            break;
        // break if not a continuation char (NOT 10bbbbbb)
        if(!(Text[s_cursor] & 1<<7) || (Text[s_cursor] & 1<<6))
            break;
    }
}

void Commit()
{
    s_committed = true;
}

// returns true if the string is complete
bool DoAction()
{
    // for now, no unicode processing, but wouldn't be hard at all...
    const char* c = get_char(s_cur_level, s_cur_row, s_cur_col);

    switch(*c)
    {
    // invalid chars
    case '\x11':
    case '\x12':
        break;
    // accept input
    case '\n':
        return true;
    // left
    case '\x1d':
        CursorLeft();
        break;
    // right
    case '\x1c':
        CursorRight();
        break;
    // shift up
    case '\x0e':
        s_cur_level ++;
        break;
    // shift down
    case '\x0f':
        s_cur_level --;
        break;
    // backspace
    case '\b':
        Backspace();
        break;
    // proper text entry
    default:
        InsertUnicodeChar(c);
    }
    return false;
}

bool KeyboardMouseRender(bool mouse, bool render)
{
    int key_size = 40;
    if(g_config.osk_fill_screen)
    {
        key_size = (ScreenW - 40) / 12;
        // force even
        key_size &= ~1;
    }

    int kb_height = 5*key_size;
    int kb_width = 12*key_size;

    int win_width = kb_width + 20;
    int win_height = kb_height + 20;

    int n_prompt_lines = ((s_Prompt.size() + 26) / 27);
    int n_text_lines = ((Text.size() + 24) / 25);
    if(n_text_lines == 0)
        n_text_lines = 1;

    win_height += n_prompt_lines * 20;
    win_height += n_text_lines * 20;

    int win_x = ScreenW / 2 - win_width / 2;
    int kb_x = win_x + 10;

    // bias towards bottom of screen
    int win_y = ScreenH / 1.25 - win_height / 1.25;
    // force even
    win_y &= ~1;
    int kb_y = win_y + 10 + n_prompt_lines*20+n_text_lines*20;

    if(render)
    {
        XRender::renderRect(win_x, win_y, win_width, win_height, 0.6f, 0.6f, 1.f, 0.8f);
        for(int i = 0; i < n_prompt_lines; i ++)
        {
            SuperPrint(s_Prompt.substr(27*i, 27), 4, win_x + 10, win_y + 6 + 20*i);
        }

        XRender::renderRect(win_x + 20, win_y+n_prompt_lines*20+4, win_width - 40, n_text_lines*20, 0.f, 0.f, 0.f, 0.8f);
        for(int i = 0; i < n_text_lines; i ++)
        {
            SuperPrint(Text.substr(25*i, 25), 4, win_x + 10 + 16, win_y + 6 + 20*(n_prompt_lines+i));
            // render cursor if it is on this line
            if(s_cursor >= i*25 && s_cursor < (i+1)*25)
            {
                // after merging in, use the appropriate print library here.
                // int cursor_offset = PrintLength(Text.substr(25*i, s_cursor - i*25))
                int cursor_offset = (s_cursor - i*25) * 18;
                XRender::renderRect(win_x + 10 + 16 + cursor_offset - 2, win_y + 4 + 20*(n_prompt_lines+i), 2, 20, 1.f, 1.f, 1.f, 0.5f);
            }
        }

        XRender::renderRect(kb_x, kb_y, kb_width, kb_height, 0.f, 0.f, 0.f, 0.2f);
    }

    for(int row = 0; row < 5; row ++)
    {
        for(int col = 0; col < 12; col ++)
        {
            bool sel = false;
            if(s_render_sel && s_cur_row == row && s_cur_col == col)
                sel = true;

            if(UpdateButton(key_size*col + kb_x, key_size*row + kb_y, key_size, get_char(s_cur_level, row, col), sel, render) && mouse)
            {
                s_render_sel = false;
                s_cur_row = row;
                s_cur_col = col;
                if(DoAction())
                    return true;
            }
        }
    }

    if(!Controls::g_renderTouchscreen)
        XRender::renderTexture(SharedCursor.X, SharedCursor.Y, GFX.ECursor[2]);

    return false;
}

const std::string& Run(const std::string& Prompt, const std::string Value)
{
#ifdef __ANDROID__
    if(g_config.use_native_osk)
    {
        s_textEntry_callDialog();
        return Text;
    }
#endif
    s_Prompt = Prompt;
    Text = Value;
    s_cursor = Text.size();
    s_mouse_up = 2;
    s_cur_level = 0;
    s_cur_row = 0;
    s_cur_col = 0;
    MenuCursorCanMove = false;
    s_render_sel = false;
    s_timer = -1;
    s_committed = false;
    PauseGame(PauseCode::TextEntry, 0);
    MenuCursorCanMove = false;
    MenuMouseRelease = false;
    MouseRelease = false;
    ScrollRelease = false;

    for(int i = 1; i <= numPlayers; i++)
    {
        Player[i].UnStart = false;
        Player[i].CanJump = false;
        Player[i].CanAltJump = false;
    }

    return Text;
}

void Render()
{
    KeyboardMouseRender(false, true);
}

bool Logic()
{
    if(SharedCursor.Primary)
        s_mouse_up = 0;
    else if(s_mouse_up == 0)
        s_mouse_up = 1;
    else
        s_mouse_up = 2;

    if(KeyboardMouseRender(true, false))
    {
        MenuCursorCanMove = false;
        MenuMouseRelease = false;
        MouseRelease = false;
        ScrollRelease = false;
        return true;
    }

    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;
    bool leftPressed = SharedControls.MenuLeft;
    bool rightPressed = SharedControls.MenuRight;

    bool startPressed = false;
    bool doPressed = SharedControls.MenuDo;
    bool backPressed = SharedControls.MenuBack;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t &c = Player[i+1].Controls;

        startPressed |= c.Start;
        doPressed |= c.Jump || c.AltJump;
        backPressed |= c.Run || c.AltRun;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;
    }

    if(MenuCursorCanMove)
    {
        if(upPressed)
            GoUp();

        if(downPressed)
            GoDown();

        if(leftPressed)
            GoLeft();

        if(rightPressed)
            GoRight();

        if(backPressed)
            Backspace();

        if((doPressed && DoAction()) || startPressed)
        {
            MenuCursorCanMove = false;
            MenuMouseRelease = false;
            return true;
        }
    }


    if(!upPressed && !downPressed && !leftPressed && !rightPressed && !doPressed && !backPressed && !startPressed)
    {
        MenuCursorCanMove = true;
    }
    else
    {
        s_render_sel = true;
        MenuCursorCanMove = false;
        s_timer --;
        if(s_timer == 0)
            MenuCursorCanMove = true;
    }

    if(MenuCursorCanMove)
        s_timer = 10;

    return s_committed;
}

} // namespace TextEntryScreen

#ifdef __ANDROID__

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_textentry_1setBuffer(JNIEnv *env, jclass clazz, jstring line_j)
{
    const char *line;
    (void)clazz;
    line = env->GetStringUTFChars(line_j, nullptr);
    TextEntryScreen::Text = (std::string)line;
    env->ReleaseStringUTFChars(line_j, line);
}

#endif // #ifdef __ANDROID__
