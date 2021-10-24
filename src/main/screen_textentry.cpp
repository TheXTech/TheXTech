#include "../frm_main.h"
#include "../globals.h"
#include "../graphics.h"

namespace TextEntryScreen
{

// KEYMAP!
// we are assumed to be operating on a Nx5x12 (levels, rows, columns) grid.
// special characters: \b is backspace, \x0e and \x0f are shift up and shift down (a grid level)
// you will likely produce a segfault if you allow the user to shift into a grid level that you did not define.
// \n is a newline (actually, accept), and \x1d and \x1c are left and right.
// special control character meanings: \x11 is "last button wider" and \x12 is "empty"

// the UTF-8 character length encoding is not supported.

// first bit unset: single char
// first two bits set: 2 chars / one character
// first three bits set: 3 chars / one character
// first four bits set: 4 chars / one character
static const char* s_keymap_US = "1234567890-\b"
    "qwertyuiop[]"
    "asdfghjkl;'="
    "`zxcvbnm,./\\"
    "\x0e\x11 \x11\x11\x11\x12\x1d\x1c\x12\n\x11"
    "!@#$%^&*()_\b"
    "QWERTYUIOP{}"
    "ASDFGHJKL:\"+"
    "~ZXCVBNM<>?|"
    "\x0f\x11\x12 \x11\x11\x11\x12\x1d\x1c\n\x11";

// please help add more useful keymaps as soon as we have more printable characters.

static const char* s_current_keymap = s_keymap_US;
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

bool UpdateButton(int x, int y, const char* c, bool sel, bool mouse, bool render)
{
    if(*c == '\x12' || *c == '\x11') // empty space or someone else's continuation
        return false;
    const char* next_char = c+1;
    char print_char[6];
    if(*c == '\b')
    {
        print_char[0] = 'b'; print_char[1] = 's'; print_char[2] = '\0';
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

    int width = 40;

    while(*next_char == '\x11') // wider
    {
        width += 40;
        next_char ++;
    }

    // the button is 36x36 and outlined by a 2 pixel box
    bool coll = false;
    if(SharedCursor.X >= x + 2 && SharedCursor.X < x + width - 2
        && SharedCursor.Y >= y + 2 && SharedCursor.Y < y + 38)
        coll = true;
    // outline:
    if(sel)
    {
        if(coll && SharedCursor.Primary)
            frmMain.renderRect(x, y, width, 40, 0.f, 0.5f, 0.f, 1.0f, true);
        else
            frmMain.renderRect(x, y, width, 40, 0.f, 1.0f, 0.f, 1.0f, true);
    }
    else if(coll && SharedCursor.Primary)
        frmMain.renderRect(x, y, width, 40, 0.f, 0.f, 0.f, 1.0f, true);
    // background:
    if(SharedCursor.Primary && coll)
        frmMain.renderRect(x+2, y+2, width-4, 36, 0.2f, 0.2f, 0.2f, true);
    else
        frmMain.renderRect(x+2, y+2, width-4, 36, 0.5f, 0.5f, 0.5f, true);

    SuperPrintCenter(print_char, 4, x+width/2, y+10);

    return (MenuMouseRelease && coll);
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

void RenderKeyboard()
{
    for(int row = 0; row < 5; row ++)
    {
        for(int col = 0; col < 12; col ++)
        {
            bool sel = false;
            if(s_render_sel && s_cur_row == row && s_cur_col == col)
                sel = true;
            UpdateButton(40*col, 40*row, get_char(s_cur_level, row, col), sel, false, true);
        }
    }
}

};