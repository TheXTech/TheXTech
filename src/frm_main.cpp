/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include "globals.h"
#include "game_main.h"
#include "graphics.h"
#include "joystick.h"
#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <Utils/files.h>
#include <Utils/elapsed_timer.h>
#include <DirManager/dirman.h>
#include <Utils/maths.h>
#include <Graphics/graphics_funcs.h>
#include <FreeImageLite.h>
#include <chrono>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>

#include <SDL2/SDL_thread.h>

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

SDL_Window *FrmMain::getWindow()
{
    return m_window;
}

Uint8 FrmMain::getKeyState(SDL_Scancode key)
{
    if(m_keyboardState)
        return m_keyboardState[key];
    return 0;
}

bool FrmMain::initSDL()
{
    bool res = false;

    LoadLogSettings(false);
    //Write into log the application start event
    pLogDebug("<Application started>");

    g_ScreenshotPath = AppPath + "screenshots/";

    Uint32 sdlInitFlags = 0;
    // Prepare flags for SDL initialization
    sdlInitFlags |= SDL_INIT_TIMER;
    sdlInitFlags |= SDL_INIT_AUDIO;
    sdlInitFlags |= SDL_INIT_VIDEO;
    sdlInitFlags |= SDL_INIT_EVENTS;
    sdlInitFlags |= SDL_INIT_JOYSTICK;
    //(Cool thing, but is not needed yet)
    //sdlInitFlags |= SDL_INIT_HAPTIC;
    sdlInitFlags |= SDL_INIT_GAMECONTROLLER;

    // Initialize SDL
    res = (SDL_Init(sdlInitFlags) < 0);
    m_sdlLoaded = !res;

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();

    SDL_GL_ResetAttributes();

    m_window = SDL_CreateWindow(m_windowTitle.c_str(),
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

    if(m_window == nullptr)
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

    if(isSdlError())
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

#ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowMinimumSize(window, ScaleWidth / 2, ScaleHeight / 2);
#else
    SDL_SetWindowMinimumSize(m_window, ScaleWidth, ScaleHeight);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");


#ifdef _WIN32
    FIBITMAP *img[2];
    img[0] = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_16.png");
    img[1] = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_32.png");

    if(!GraphicsHelps::setWindowIcon(window, img[0], 16))
    {
        pLogWarning("Unable to setup window icon!");
        SDL_ClearError();
    }

    if(!GraphicsHelps::setWindowIcon(window, img[1], 32))
    {
        pLogWarning("Unable to setup window icon!");
        SDL_ClearError();
    }

    GraphicsHelps::closeImage(img[0]);
    GraphicsHelps::closeImage(img[1]);
#else//IF _WIN32

    FIBITMAP *img;
#   ifdef __APPLE__
    img = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_256.png");
#   else
    img = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_32.png");
#   endif //__APPLE__

    if(img)
    {
        SDL_Surface *sIcon = GraphicsHelps::fi2sdl(img);
        SDL_SetWindowIcon(m_window, sIcon);
        GraphicsHelps::closeImage(img);
        SDL_FreeSurface(sIcon);

        if(isSdlError())
        {
            pLogWarning("Unable to setup window icon!");
            SDL_ClearError();
        }
    }
#endif//IF _WIN32 #else

    pLogDebug("Init renderer settings...");

    m_gRenderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if(!m_gRenderer)
    {
        pLogCritical("Unable to create renderer!");
        freeSDL();
        return false;
    }

    SDL_SetRenderDrawBlendMode(m_gRenderer, SDL_BLENDMODE_BLEND);

    m_keyboardState = SDL_GetKeyboardState(nullptr);

    updateViewport();

    repaint();
    doEvents();

    return res;
}

void FrmMain::freeSDL()
{
    GFX.unLoad();
    clearAllTextures();
    CloseJoysticks();
    if(m_gRenderer)
        SDL_DestroyRenderer(m_gRenderer);
    if(m_window)
        SDL_DestroyWindow(m_window);
    SDL_Quit();
    CloseLog();
}

void FrmMain::show()
{
    SDL_ShowWindow(m_window);
}

void FrmMain::hide()
{
    SDL_HideWindow(m_window);
    ShowCursor(1);
}

void FrmMain::doEvents()
{
    while(SDL_PollEvent(&m_event))
    {
        processEvent();
    }
}

void FrmMain::processEvent()
{
    switch(m_event.type)
    {
    case SDL_QUIT:
        ShowCursor(1);
        KillIt();
        break;
    case SDL_WINDOWEVENT:
        if((m_event.window.event == SDL_WINDOWEVENT_RESIZED) || (m_event.window.event == SDL_WINDOWEVENT_MOVED))
            eventResize();
        break;
    case SDL_KEYDOWN:
        eventKeyDown(m_event.key);
        eventKeyPress(m_event.key.keysym.sym);
        break;
    case SDL_KEYUP:
        eventKeyUp(m_event.key);
        break;
    case SDL_MOUSEBUTTONDOWN:
        eventMouseDown(m_event.button);
        break;
    case SDL_MOUSEBUTTONUP:
        eventMouseUp(m_event.button);
        break;
    case SDL_MOUSEMOTION:
        eventMouseMove(m_event.motion);
        break;
    }
}

void FrmMain::waitEvents()
{
    if(SDL_WaitEventTimeout(&m_event, 1000))
        processEvent();
    doEvents();
}

bool FrmMain::isWindowActive()
{
    if(!m_window)
        return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
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
    case SDLK_a: CheatCode('a'); break;
    case SDLK_b: CheatCode('b'); break;
    case SDLK_c: CheatCode('c'); break;
    case SDLK_d: CheatCode('d'); break;
    case SDLK_e: CheatCode('e'); break;
    case SDLK_f: CheatCode('f'); break;
    case SDLK_g: CheatCode('g'); break;
    case SDLK_h: CheatCode('h'); break;
    case SDLK_i: CheatCode('i'); break;
    case SDLK_j: CheatCode('j'); break;
    case SDLK_k: CheatCode('k'); break;
    case SDLK_l: CheatCode('l'); break;
    case SDLK_m: CheatCode('m'); break;
    case SDLK_n: CheatCode('n'); break;
    case SDLK_o: CheatCode('o'); break;
    case SDLK_p: CheatCode('p'); break;
    case SDLK_q: CheatCode('q'); break;
    case SDLK_r: CheatCode('r'); break;
    case SDLK_s: CheatCode('s'); break;
    case SDLK_t: CheatCode('t'); break;
    case SDLK_u: CheatCode('u'); break;
    case SDLK_v: CheatCode('v'); break;
    case SDLK_w: CheatCode('w'); break;
    case SDLK_x: CheatCode('x'); break;
    case SDLK_y: CheatCode('y'); break;
    case SDLK_z: CheatCode('z'); break;
    default: CheatCode(' '); break;
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
    SDL_Point p = MapToScr(event.x, event.y);
    MenuMouseX = p.x;// int(event.x * ScreenW / ScaleWidth);
    MenuMouseY = p.y;//int(event.y * ScreenH / ScaleHeight);
    MenuMouseMove = true;
}

void FrmMain::eventMouseUp(SDL_MouseButtonEvent &)
{
    MenuMouseDown = false;
    MenuMouseRelease = true;
}

void FrmMain::eventResize()
{
    updateViewport();
    SetupScreens();
}

int FrmMain::setFullScreen(bool fs)
{
    if(m_window == nullptr)
        return -1;

    if(fs != IsFullScreen(m_window))
    {
        if(fs)
        {
            // Swith to FULLSCREEN mode
            if(SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
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
            if(SDL_SetWindowFullscreen(m_window, SDL_FALSE) < 0)
            {
                pLogWarning("Setting windowed failed: %s", SDL_GetError());
                return -1;
            }

            return 0;
        }
    }

    return 0;
}

bool FrmMain::isSdlError()
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}

void FrmMain::repaint()
{
    SDL_RenderPresent(m_gRenderer);
}

void FrmMain::updateViewport()
{
    float w, w1, h, h1;
    int   wi, hi;
    SDL_GetWindowSize(m_window, &wi, &hi);
    w = wi;
    h = hi;
    w1 = w;
    h1 = h;
    scale_x = w / ScaleWidth;
    scale_y = h / ScaleHeight;
    viewport_scale_x = scale_x;
    viewport_scale_y = scale_y;

    viewport_offset_x = 0;
    viewport_offset_y = 0;

    if(scale_x > scale_y)
    {
        w1 = scale_y * ScaleWidth;
        viewport_scale_x = w1 / ScaleWidth;
    }
    else if(scale_x < scale_y)
    {
        h1 = scale_x * ScaleHeight;
        viewport_scale_y = h1 / ScaleHeight;
    }

    offset_x = (w - w1) / 2;
    offset_y = (h - h1) / 2;

    viewport_x = 0;
    viewport_y = 0;
    viewport_w = static_cast<int>(w1);
    viewport_h = static_cast<int>(h1);

    SDL_Rect topLeftViewport = {0, 0, wi, hi};
    SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);
    clearBuffer();

    topLeftViewport.x = static_cast<int>(offset_x);
    topLeftViewport.y = static_cast<int>(offset_y);
    topLeftViewport.w = viewport_w;
    topLeftViewport.h = viewport_h;
    SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);
}

void FrmMain::resetViewport()
{
    float w, w1, h, h1;
    int   wi, hi;
    SDL_GetWindowSize(m_window, &wi, &hi);
    w = wi;
    h = hi;
    w1 = w;
    h1 = h;
    scale_x = w / ScaleWidth;
    scale_y = h / ScaleHeight;
    viewport_scale_x = scale_x;
    viewport_scale_y = scale_y;

    viewport_offset_x = 0;
    viewport_offset_y = 0;

    if(scale_x > scale_y)
    {
        w1 = scale_y * ScaleWidth;
        viewport_scale_x = w1 / ScaleWidth;
    }
    else if(scale_x < scale_y)
    {
        h1 = scale_x * ScaleHeight;
        viewport_scale_y = h1 / ScaleHeight;
    }

    offset_x = (w - w1) / 2;
    offset_y = (h - h1) / 2;

    viewport_x = 0;
    viewport_y = 0;
    viewport_w = static_cast<int>(w1);
    viewport_h = static_cast<int>(h1);

    SDL_Rect topLeftViewport = {0, 0, wi, hi};
    topLeftViewport.x = static_cast<int>(offset_x);
    topLeftViewport.y = static_cast<int>(offset_y);
    topLeftViewport.w = viewport_w;
    topLeftViewport.h = viewport_h;
    SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);
}

void FrmMain::setViewport(int x, int y, int w, int h)
{
    auto xF = static_cast<float>(x);
    auto yF = static_cast<float>(y);
    auto wF = static_cast<float>(w);
    auto hF = static_cast<float>(h);
    SDL_Rect topLeftViewport;
    topLeftViewport.x = Maths::iRound(offset_x + std::ceil(xF * viewport_scale_x));
    topLeftViewport.y = Maths::iRound(offset_y + std::ceil(yF * viewport_scale_y));
    topLeftViewport.w = Maths::iRound(wF * viewport_scale_x);
    topLeftViewport.h = Maths::iRound(hF * viewport_scale_y);
    SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);
    viewport_x = xF;
    viewport_y = yF;
    viewport_w = wF;
    viewport_h = hF;
}

void FrmMain::offsetViewport(int x, int y)
{
    if(viewport_offset_x != x || viewport_offset_y != y)
    {
        viewport_offset_x = x;
        viewport_offset_y = y;
    }
}

StdPicture FrmMain::LoadPicture(std::string path, std::string maskPath, std::string maskFallbackPath)
{
    StdPicture target;
    FIBITMAP *sourceImage;

    if(path.empty())
        return target;

    sourceImage = GraphicsHelps::loadImage(path);

    // Don't load mask if PNG image is used
    if(Files::hasSuffix(path, ".png"))
    {
        maskPath.clear();
        maskFallbackPath.clear();
    }

    if(!sourceImage)
    {
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    (Files::fileExists(path) ? "wrong image format" : "file not exist"));
        // target = g_renderer->getDummyTexture();
        return target;
    }

#ifdef DEBUG_BUILD
    ElapsedTimer totalTime;
    ElapsedTimer maskMergingTime;
    ElapsedTimer bindingTime;
    ElapsedTimer unloadTime;
    totalTime.start();
    int64_t maskElapsed = 0;
    int64_t bindElapsed = 0;
    int64_t unloadElapsed = 0;
#endif

    //Apply Alpha mask
    if(!maskPath.empty() && Files::fileExists(maskPath))
    {
#ifdef DEBUG_BUILD
        maskMergingTime.start();
#endif
        GraphicsHelps::mergeWithMask(sourceImage, maskPath);
#ifdef DEBUG_BUILD
        maskElapsed = maskMergingTime.nanoelapsed();
#endif
    }
    else if(!maskFallbackPath.empty())
    {
#ifdef DEBUG_BUILD
        maskMergingTime.start();
#endif
        GraphicsHelps::mergeWithMask(sourceImage, "", maskFallbackPath);
#ifdef DEBUG_BUILD
        maskElapsed = maskMergingTime.nanoelapsed();
#endif
    }

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(sourceImage));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(sourceImage));

    if((w == 0) || (h == 0))
    {
        FreeImage_Unload(sourceImage);
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    "Zero image size!");
        //target = g_renderer->getDummyTexture();
        return target;
    }

#ifdef DEBUG_BUILD
    bindingTime.start();
#endif
    RGBQUAD upperColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &upperColor);
    target.ColorUpper.r = float(upperColor.rgbRed) / 255.0f;
    target.ColorUpper.b = float(upperColor.rgbBlue) / 255.0f;
    target.ColorUpper.g = float(upperColor.rgbGreen) / 255.0f;
    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, static_cast<unsigned int>(h - 1), &lowerColor);
    target.ColorLower.r = float(lowerColor.rgbRed) / 255.0f;
    target.ColorLower.b = float(lowerColor.rgbBlue) / 255.0f;
    target.ColorLower.g = float(lowerColor.rgbGreen) / 255.0f;
    FreeImage_FlipVertical(sourceImage);
    target.nOfColors = GL_RGBA;
    target.format = GL_BGRA;
    target.w = static_cast<int>(w);
    target.h = static_cast<int>(h);
    target.frame_w = static_cast<int>(w);
    target.frame_h = static_cast<int>(h);
    GLubyte *textura = reinterpret_cast<GLubyte *>(FreeImage_GetBits(sourceImage));
    loadTexture(target, w, h, textura);
#ifdef DEBUG_BUILD
    bindElapsed = bindingTime.nanoelapsed();
    unloadTime.start();
#endif
    //SDL_FreeSurface(sourceImage);
    GraphicsHelps::closeImage(sourceImage);
#ifdef DEBUG_BUILD
    unloadElapsed = unloadTime.nanoelapsed();
#endif
#ifdef DEBUG_BUILD
    pLogDebug("Mask merging of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(maskElapsed));
    pLogDebug("Binding time of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(bindElapsed));
    pLogDebug("Unload time of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(unloadElapsed));
    pLogDebug("Total Loading of texture %s passed in %d nanoseconds (%dx%d)",
              path.c_str(),
              static_cast<int>(totalTime.nanoelapsed()),
              static_cast<int>(w),
              static_cast<int>(h));
#endif

    return target;
}

void FrmMain::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    surface = SDL_CreateRGBSurfaceFrom(RGBApixels,
                                       static_cast<int>(width),
                                       static_cast<int>(height),
                                       32,
                                       static_cast<int>(width * 4),
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       FI_RGBA_ALPHA_MASK);
    texture = SDL_CreateTextureFromSurface(m_gRenderer, surface);
    SDL_FreeSurface(surface);
    if(!texture)
    {
        pLogWarning("Render SW-SDL: Failed to load texture!");
        return;
    }

    target.texture = texture;
    m_textureBank.insert(texture);

    target.inited = true;
}


void FrmMain::makeShot()
{
    if(!m_gRenderer)
        return;

    // Make the BYTE array, factor of 3 because it's RBG.
    int w, h;
    float wF, hF;
    SDL_GetWindowSize(m_window, &w, &h);

    if((w == 0) || (h == 0))
    {
        pLogWarning("Can't make screenshot: invalid width(%d) or height(%d).", w, h);
        return;
    }

    wF = static_cast<float>(w);
    hF = static_cast<float>(h);
    wF = wF - offset_x * 2.0f;
    hF = hF - offset_y * 2.0f;
    w = static_cast<int>(wF);
    h = static_cast<int>(hF);
    uint8_t *pixels = new uint8_t[size_t(4 * w * h)];
    getScreenPixels(static_cast<int>(offset_x), static_cast<int>(offset_y), w, h, pixels);
    PGE_GL_shoot *shoot = new PGE_GL_shoot();
    shoot->pixels = pixels;
    shoot->w = w;
    shoot->h = h;
    shoot->me = this;
#ifndef PGE_NO_THREADING
    m_screenshot_thread = SDL_CreateThread(makeShot_action, "scrn_maker", reinterpret_cast<void *>(shoot));
#else
    makeShot_action(reinterpret_cast<void *>(shoot));
#endif
}

static std::string shoot_getTimedString(std::string path, const char *ext = "png")
{
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm t = fmt::localtime_ne(in_time_t);
    static int prevSec = 0;
    static int prevSecCounter = 0;
    if(prevSec != t.tm_sec)
    {
        prevSec = t.tm_sec;
        prevSecCounter = 0;
    }
    else
        prevSecCounter++;

    if(!prevSecCounter)
    {
        return fmt::sprintf_ne("%sScr_%04d-%02d-%02d_%02d-%02d-%02d.%s",
                               path,
                               (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                               t.tm_hour, t.tm_min, t.tm_sec,
                               ext);
    }
    else
    {
        return fmt::sprintf_ne("%sScr_%04d-%02d-%02d_%02d-%02d-%02d_(%d).%s",
                               path,
                               (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                               t.tm_hour, t.tm_min, t.tm_sec,
                               prevSecCounter,
                               ext);
    }
}

int FrmMain::makeShot_action(void *_pixels)
{
    PGE_GL_shoot *shoot = reinterpret_cast<PGE_GL_shoot *>(_pixels);
    FrmMain *me = shoot->me;
    FIBITMAP *shotImg = FreeImage_ConvertFromRawBits(reinterpret_cast<BYTE *>(shoot->pixels), shoot->w, shoot->h,
                        3 * shoot->w + shoot->w % 4, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);

    if(!shotImg)
    {
        delete []shoot->pixels;
        shoot->pixels = nullptr;
        delete []shoot;
        me->m_screenshot_thread = nullptr;
        return 0;
    }

    FIBITMAP *temp;
    temp = FreeImage_ConvertTo32Bits(shotImg);

    if(!temp)
    {
        FreeImage_Unload(shotImg);
        delete []shoot->pixels;
        shoot->pixels = nullptr;
        delete []shoot;
        me->m_screenshot_thread = nullptr;
        return 0;
    }

    FreeImage_Unload(shotImg);
    shotImg = temp;

    if((shoot->w != ScreenW) || (shoot->h != ScreenH))
    {
        FIBITMAP *temp = FreeImage_Rescale(shotImg, ScreenW, ScreenH, FILTER_BOX);
        if(!temp)
        {
            FreeImage_Unload(shotImg);
            delete []shoot->pixels;
            shoot->pixels = nullptr;
            delete []shoot;
            me->m_screenshot_thread = nullptr;
            return 0;
        }

        FreeImage_Unload(shotImg);
        shotImg = temp;
    }

    if(!DirMan::exists(me->g_ScreenshotPath))
        DirMan::mkAbsDir(me->g_ScreenshotPath);

    std::string saveTo = shoot_getTimedString(me->g_ScreenshotPath, "png");
    pLogDebug("%s %d %d", saveTo.c_str(), shoot->w, shoot->h);

    if(FreeImage_HasPixels(shotImg) == FALSE)
        pLogWarning("Can't save screenshot: no pixel data!");
    else
        FreeImage_Save(FIF_PNG, shotImg, saveTo.data(), PNG_Z_BEST_COMPRESSION);

    FreeImage_Unload(shotImg);
    delete []shoot->pixels;
    shoot->pixels = nullptr;
    delete []shoot;

    me->m_screenshot_thread = nullptr;
    return 0;
}

SDL_Rect FrmMain::scaledRectIS(float x, float y, int w, int h)
{
    x += viewport_offset_x;
    y += viewport_offset_y;
    return
    {
        static_cast<int>(std::ceil(x * viewport_scale_x)),
        static_cast<int>(std::ceil(y * viewport_scale_y)),
        static_cast<int>(std::ceil(static_cast<float>(w) * viewport_scale_x)),
        static_cast<int>(std::ceil(static_cast<float>(h) * viewport_scale_y))
    };
}

SDL_Rect FrmMain::scaledRect(float x, float y, float w, float h)
{
    x += viewport_offset_x;
    y += viewport_offset_y;
    return
    {
        static_cast<int>(std::ceil(x * viewport_scale_x)),
        static_cast<int>(std::ceil(y * viewport_scale_y)),
        static_cast<int>(std::ceil(w * viewport_scale_x)),
        static_cast<int>(std::ceil(h * viewport_scale_y))
    };
}

SDL_Rect FrmMain::scaledRectS(float left, float top, float right, float bottom)
{
    left += viewport_offset_x;
    top += viewport_offset_y;
    right += viewport_offset_x;
    bottom += viewport_offset_y;
    return
    {
        static_cast<int>(std::ceil(left * viewport_scale_x)),
        static_cast<int>(std::ceil(top * viewport_scale_y)),
        static_cast<int>(std::ceil((right - left)*viewport_scale_x)),
        static_cast<int>(std::ceil((bottom - top)*viewport_scale_y))
    };
}

SDL_Point FrmMain::MapToScr(int x, int y)
{
    return {
        static_cast<int>((static_cast<float>(x) - offset_x) / viewport_scale_x),
                static_cast<int>((static_cast<float>(y) - offset_y) / viewport_scale_y)
    };
}

void FrmMain::deleteTexture(StdPicture &tx)
{
    if(!tx.inited || !tx.texture)
    {
        tx.inited = false;
        return;
    }

    if(!tx.texture)
    {
        tx.inited = false;
        return;
    }

    auto corpseIt = m_textureBank.find(tx.texture);
    if(corpseIt == m_textureBank.end())
    {
        if(tx.texture)
            SDL_DestroyTexture(tx.texture);
        tx.texture = nullptr;
        tx.inited = false;
        return;
    }

    SDL_Texture *corpse = *corpseIt;
    if(corpse)
        SDL_DestroyTexture(corpse);
    m_textureBank.erase(corpse);

    tx.texture = nullptr;
    tx.inited = false;

    tx.w = 0;
    tx.h = 0;
    tx.frame_w = 0;
    tx.frame_h = 0;
    tx.format = 0;
    tx.nOfColors = 0;
    tx.ColorUpper.r = 0;
    tx.ColorUpper.g = 0;
    tx.ColorUpper.b = 0;
    tx.ColorLower.r = 0;
    tx.ColorLower.g = 0;
    tx.ColorLower.b = 0;
}

void FrmMain::clearAllTextures()
{
    for(SDL_Texture *tx : m_textureBank)
        SDL_DestroyTexture(tx);
    m_textureBank.clear();
}

void FrmMain::clearBuffer()
{
    SDL_SetRenderDrawColor(m_gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_gRenderer);
}

void FrmMain::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    SDL_Rect aRect = scaledRect(x, y, w, h);
    SDL_SetRenderDrawColor(m_gRenderer,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue),
                           static_cast<unsigned char>(255.f * alpha)
                          );

    if(filled)
        SDL_RenderFillRect(m_gRenderer, &aRect);
    else
        SDL_RenderDrawRect(m_gRenderer, &aRect);
}

void FrmMain::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
    SDL_Rect aRect = scaledRectS(_left, _top, _right, _bottom);
    SDL_SetRenderDrawColor(m_gRenderer,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue),
                           static_cast<unsigned char>(255.f * alpha)
                          );
    SDL_RenderFillRect(m_gRenderer, &aRect);
}

void FrmMain::renderTextureI(int xDst, int yDst, int wDst, int hDst,
                             const StdPicture &tx,
                             int xSrc, int ySrc,
                             float red, float green, float blue, float alpha)
{
    const unsigned int flip = SDL_FLIP_NONE;

    // Don't go more than size of texture
    if(xSrc + wDst > tx.w)
    {
        wDst = tx.w - xSrc;
        if(wDst < 0)
            wDst = 0;
    }
    if(ySrc + hDst > tx.h)
    {
        hDst = tx.h - ySrc;
        if(hDst < 0)
            hDst = 0;
    }

    SDL_Rect destRect = scaledRect(xDst, yDst, wDst, hDst);
    SDL_Rect sourceRect =
    {
        xSrc,
        ySrc,
        wDst,
        hDst
    };

    SDL_SetTextureColorMod(tx.texture,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue));
    SDL_SetTextureAlphaMod(tx.texture, static_cast<unsigned char>(255.f * alpha));
    SDL_RenderCopyEx(m_gRenderer, tx.texture, &sourceRect, &destRect,
                     0.0, nullptr, static_cast<SDL_RendererFlip>(flip));
}

void FrmMain::renderTexture(double xDst, double yDst, double wDst, double hDst,
                            const StdPicture &tx,
                            int xSrc, int ySrc,
                            float red, float green, float blue, float alpha)
{
    renderTextureI(Maths::iRound(xDst),
                   Maths::iRound(yDst),
                   Maths::iRound(wDst),
                   Maths::iRound(hDst),
                   tx,
                   xSrc,
                   ySrc,
                   red, green, blue, alpha);
}

void FrmMain::renderTexture(int xDst, int yDst, const StdPicture &tx, float red, float green, float blue, float alpha)
{
    const unsigned int flip = SDL_FLIP_NONE;
    SDL_Rect destRect = scaledRect(xDst, yDst, tx.w, tx.h);
    SDL_Rect sourceRect = {0, 0, tx.w, tx.h};

    SDL_SetTextureColorMod(tx.texture,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue));
    SDL_SetTextureAlphaMod(tx.texture, static_cast<unsigned char>(255.f * alpha));
    SDL_RenderCopyEx(m_gRenderer, tx.texture, &sourceRect, &destRect,
                     0.0, nullptr, static_cast<SDL_RendererFlip>(flip));
}

void FrmMain::getScreenPixels(int x, int y, int w, int h, unsigned char *pixels)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderReadPixels(m_gRenderer,
                         &rect,
                         SDL_PIXELFORMAT_BGR24,
                         pixels,
                         w * 3 + (w % 4));
}

void FrmMain::getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderReadPixels(m_gRenderer,
                         &rect,
                         SDL_PIXELFORMAT_ABGR8888,
                         pixels,
                         w * 4);
}

int FrmMain::getPixelDataSize(const StdPicture &tx)
{
    if(!tx.texture)
        return 0;
    return (tx.w * tx.h * 4);
}

void FrmMain::getPixelData(const StdPicture &tx, unsigned char *pixelData)
{
    if(tx.texture)
        return;
    int pitch, w, h, a;
    void *pixels;
    SDL_SetTextureBlendMode(tx.texture, SDL_BLENDMODE_BLEND);
    SDL_QueryTexture(tx.texture, nullptr, &a, &w, &h);
    SDL_LockTexture(tx.texture, nullptr, &pixels, &pitch);
    std::memcpy(pixelData, pixels, static_cast<size_t>(pitch * h));
    SDL_UnlockTexture(tx.texture);
}
