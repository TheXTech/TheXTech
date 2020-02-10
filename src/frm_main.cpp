#include "globals.h"
#include "game_main.h"
#include "graphics.h"
#include <SDL2/SDL_mixer_ext.h>
#include <Logger/logger.h>

#include "frm_main.h"

static SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

FrmMain::FrmMain()
{
    ScaleWidth = ScreenW;
    ScaleHeight = ScreenH;
}

bool FrmMain::initSDL()
{
    bool res = false;

    LoadLogSettings(false);
    //Write into log the application start event
    pLogDebug("<Application started>");

    Uint32 sdlInitFlags = 0;
    int sdlMixerInitFlags = 0;
    // Prepare flags for SDL initialization
    sdlInitFlags |= SDL_INIT_TIMER;
    sdlInitFlags |= SDL_INIT_AUDIO;
    sdlInitFlags |= SDL_INIT_VIDEO;
    sdlInitFlags |= SDL_INIT_EVENTS;
    sdlInitFlags |= SDL_INIT_JOYSTICK;
    //(Cool thing, but is not needed yet)
    //sdlInitFlags |= SDL_INIT_HAPTIC;
    sdlInitFlags |= SDL_INIT_GAMECONTROLLER;

    sdlMixerInitFlags |= MIX_INIT_FLAC;
    sdlMixerInitFlags |= MIX_INIT_MOD;
    sdlMixerInitFlags |= MIX_INIT_MP3;
    sdlMixerInitFlags |= MIX_INIT_OGG;
    sdlMixerInitFlags |= MIX_INIT_MID;

    // Initialize SDL
    res = (SDL_Init(sdlInitFlags) < 0);
    m_sdlLoaded = !res;

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();

    // Initialize SDL Mixer
    res = (Mix_Init(sdlMixerInitFlags) < 0);

    SDL_ClearError();

    SDL_GL_ResetAttributes();

    window = SDL_CreateWindow(WindowTitle.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                          #ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
                              ScaleWidth / 2, ScaleHeight / 2,
                          #else
                              ScaleWidth, ScaleHeight,
                          #endif //__EMSCRIPTEN__
                              SDL_WINDOW_RESIZABLE |
                              SDL_WINDOW_HIDDEN |
                              SDL_WINDOW_ALLOW_HIGHDPI);

    if(window == nullptr)
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

#ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowMinimumSize(window, ScaleWidth / 2, ScaleHeight / 2);
#else
    SDL_SetWindowMinimumSize(window, ScaleWidth, ScaleHeight);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    m_gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!m_gRenderer)
    {
        pLogCritical("Unable to create renderer!");
        freeSDL();
        return false;
    }

    SDL_SetRenderDrawBlendMode(m_gRenderer, SDL_BLENDMODE_BLEND);

    return res;
}

void FrmMain::freeSDL()
{
    if(m_gRenderer)
        SDL_DestroyRenderer(m_gRenderer);
    if(window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    CloseLog();
}

void FrmMain::show()
{
    SDL_ShowWindow(window);
}

void FrmMain::hide()
{
    SDL_HideWindow(window);
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

int FrmMain::setFullScreen(bool fs)
{
    if(window == nullptr)
        return -1;

    if(fs != IsFullScreen(window))
    {
        if(fs)
        {
            // Swith to FULLSCREEN mode
            if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
            {
                //Hide mouse cursor in full screen mdoe
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }

            SDL_ShowCursor(SDL_DISABLE);
            return 1;
        }
        else
        {
            // Swith to WINDOWED mode
            if(SDL_SetWindowFullscreen(window, SDL_FALSE) < 0)
            {
                pLogWarning("Setting windowed failed: %s", SDL_GetError());
                return -1;
            }

            return 0;
        }
    }

    return 0;
}
