#include "globals.h"
#include "game_main.h"
#include "graphics.h"

#include "frm_main.h"

static SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

FrmMain::FrmMain()
{}

void FrmMain::initSDL()
{

}

void FrmMain::freeSDL()
{

}

void FrmMain::show()
{

}

void FrmMain::hide()
{
    ShowCursor(1);
}

void FrmMain::doEvents()
{
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            ShowCursor(1);
            KillIt();
            break;
        case SDL_WINDOWEVENT:
            if((event.window.event == SDL_WINDOWEVENT_RESIZED) || (event.window.event == SDL_WINDOWEVENT_MOVED))
                eventResize();
            break;
        case SDL_KEYDOWN:
            eventKeyDown(event.key);
            eventKeyPress(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            eventKeyUp(event.key);
            break;
        case SDL_MOUSEBUTTONDOWN:
            eventMouseDown(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            eventMouseUp(event.button);
            break;
        case SDL_MOUSEMOTION:
            eventMouseMove(event.motion);
            break;
        }
    }
}

void FrmMain::eventDoubleClick()
{

}

void FrmMain::eventKeyDown(SDL_KeyboardEvent &evt)
{
    int KeyCode = evt.keysym.scancode;
    inputKey = KeyCode;
    if(KeyCode == SDL_SCANCODE_RETURN || KeyCode == SDL_SCANCODE_KP_ENTER)
        keyDownEnter = true;
    else if(KeyCode == SDL_SCANCODE_LALT || KeyCode == SDL_SCANCODE_RALT)
        keyDownAlt = true;

    if(keyDownAlt && keyDownEnter && !TestLevel)
    {
        keyDownAlt = false;
        keyDownEnter = false;
        ChangeScreen();
    }

    if(KeyCode == SDL_SCANCODE_F12)
        TakeScreen = true;
}

void FrmMain::eventKeyPress(SDL_Keycode KeyASCII)
{
    switch(KeyASCII)
    {
    case SDLK_a: CheatCode("a"); break;
    case SDLK_b: CheatCode("b"); break;
    case SDLK_c: CheatCode("c"); break;
    case SDLK_d: CheatCode("d"); break;
    case SDLK_e: CheatCode("e"); break;
    case SDLK_f: CheatCode("f"); break;
    case SDLK_g: CheatCode("g"); break;
    case SDLK_h: CheatCode("h"); break;
    case SDLK_i: CheatCode("i"); break;
    case SDLK_j: CheatCode("j"); break;
    case SDLK_k: CheatCode("k"); break;
    case SDLK_l: CheatCode("l"); break;
    case SDLK_m: CheatCode("m"); break;
    case SDLK_n: CheatCode("n"); break;
    case SDLK_o: CheatCode("o"); break;
    case SDLK_p: CheatCode("p"); break;
    case SDLK_q: CheatCode("q"); break;
    case SDLK_r: CheatCode("r"); break;
    case SDLK_s: CheatCode("s"); break;
    case SDLK_t: CheatCode("t"); break;
    case SDLK_u: CheatCode("u"); break;
    case SDLK_v: CheatCode("v"); break;
    case SDLK_w: CheatCode("w"); break;
    case SDLK_x: CheatCode("x"); break;
    case SDLK_y: CheatCode("y"); break;
    case SDLK_z: CheatCode("z"); break;
    default: CheatCode(" "); break;
    }
}

void FrmMain::eventKeyUp(SDL_KeyboardEvent &evt)
{
    int KeyCode = evt.keysym.scancode;
    if(KeyCode == SDL_SCANCODE_RETURN || KeyCode == SDL_SCANCODE_KP_ENTER)
        keyDownEnter = false;
    else if(KeyCode == SDL_SCANCODE_LALT || KeyCode == SDL_SCANCODE_RALT)
        keyDownAlt = false;
}

void FrmMain::eventMouseDown(SDL_MouseButtonEvent &event)
{
    if(event.button == SDL_BUTTON_LEFT)
    {
        MenuMouseDown = true;
        MenuMouseMove = true;
    }
    else if(event.button == SDL_BUTTON_RIGHT)
    {
        MenuMouseBack = true;
    }
}

void FrmMain::eventMouseMove(SDL_MouseMotionEvent &event)
{
    MenuMouseX = int(event.x * ScreenW / ScaleWidth);
    MenuMouseY = int(event.y * ScreenH / ScaleHeight);
    MenuMouseMove = true;
}

void FrmMain::eventMouseUp(SDL_MouseButtonEvent &)
{
    MenuMouseDown = false;
    MenuMouseRelease = true;
}

void FrmMain::eventResize()
{
    SetupScreens();
}

void FrmMain::toggleFullScreen()
{

}
