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

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>

#include <string>
#include <set>

#ifndef __EMSCRIPTEN__
#include <deque>
#endif

#include <gif_writer.h>

#include "std_picture.h"
#include "cmd_line_setup.h"

typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_mutex SDL_mutex;

class FrmMain
{
    std::string m_windowTitle;
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_gRenderer = nullptr;
    SDL_Texture  *m_tBuffer = nullptr;
    std::set<SDL_Texture *> m_textureBank;
    bool m_sdlLoaded = false;
    const Uint8 *m_keyboardState = nullptr;
    Uint32 m_lastMousePress = 0;
    SDL_Event m_event;
    SDL_RendererInfo m_ri;
#ifdef __ANDROID__
    bool m_blockRender = false;
#endif

    size_t m_lazyLoadedBytes = 0;

public:
    int ScaleWidth = 800;
    int ScaleHeight = 600;

    int MousePointer = 0;

    FrmMain();

    SDL_Window *getWindow();

    Uint8 getKeyState(SDL_Scancode key);

    bool initSDL(const CmdLineSetup_t &setup);
    void freeSDL();

    void show();
    void hide();
    void doEvents();
    void waitEvents();

    bool isWindowActive();
    bool hasWindowMouseFocus();

    void eventDoubleClick();
    void eventKeyPress(SDL_Scancode KeyASCII);
    void eventKeyDown(SDL_KeyboardEvent &evt);
    void eventKeyUp(SDL_KeyboardEvent &evt);
    void eventMouseDown(SDL_MouseButtonEvent &m_event);
    void eventMouseMove(SDL_MouseMotionEvent &m_event);
    void eventMouseUp(SDL_MouseButtonEvent &m_event);
    void eventResize();
    int setFullScreen(bool fs);
    bool isSdlError();

    void repaint();
    void updateViewport();
    void resetViewport();
    void setViewport(int x, int y, int w, int h);
    void offsetViewport(int x, int y); // for screen-shaking

    StdPicture LoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    StdPicture lazyLoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    void deleteTexture(StdPicture &tx, bool lazyUnload = false);
    void clearAllTextures();

    void clearBuffer();
    void renderRect(int x, int y, int w, int h, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
    void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);

    void renderCircle(int cx, int cy, int radius, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);

    // Similar to BitBlt, but without masks, just draw a texture or it's fragment!
    void renderTextureI(int xDst, int yDst, int wDst, int hDst,
                        StdPicture &tx,
                        int xSrc, int ySrc,
                        double rotateAngle = 0.0, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                        float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
    void renderTextureScaleI(int xDst, int yDst, int wDst, int hDst,
                             StdPicture &tx,
                             int xSrc, int ySrc,
                             int wSrc, int hSrc,
                             double rotateAngle = 0.0, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                             float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
    void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            int wSrc, int hSrc,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
    void renderTexture(double xDst, double yDst, double wDst, double hDst,
                       StdPicture &tx,
                       int xSrc, int ySrc,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                         StdPicture &tx,
                         int xSrc, int ySrc,
                         double rotateAngle = 0.0, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                         float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTexture(int xDst, int yDst, StdPicture &tx,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
    void renderTextureScale(int xDst, int yDst, int wDst, int hDst,
                            StdPicture &tx,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void getScreenPixels(int x, int y, int w, int h, unsigned char *pixels);
    void getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels);
    int  getPixelDataSize(const StdPicture &tx);
    void getPixelData(const StdPicture &tx, unsigned char *pixelData);

    void lazyPreLoad(StdPicture &target);

    size_t lazyLoadedBytes();
    void lazyLoadedBytesReset();

#ifdef __ANDROID__
    bool renderBlocked();
#endif

#ifndef __EMSCRIPTEN__
    void makeShot();
#endif

private:
#ifndef __EMSCRIPTEN__

    struct PGE_GL_shoot
    {
        FrmMain *me = nullptr;
        uint8_t *pixels = nullptr;
        int pitch = 0;
        int w = 0, h = 0;
    };

    struct GifRecorder
    {
        GIF_H::GifWriter  writer      = {nullptr, nullptr, true, false};
        SDL_Thread *worker      = nullptr;
        uint32_t    delay       = 4;
        uint32_t    delayTimer  = 0;
        bool        enabled     = false;
        unsigned char padding[7] = {0, 0, 0, 0, 0, 0, 0};
        bool        fadeForward = true;
        float       fadeValue = 0.5f;

        std::deque<PGE_GL_shoot> queue;
        SDL_mutex  *mutex = nullptr;
        bool        doFinalize = false;

        void init();
        void quit();

        void drawRecCircle();
        bool hasSome();
        void enqueue(const PGE_GL_shoot &entry);
        PGE_GL_shoot dequeue();
    };

    GifRecorder m_gif;

    bool recordInProcess();
    void toggleGifRecorder();
    void processRecorder();
    static int processRecorder_action(void *_pixels);
#endif

    void processEvent();
    void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels);

    void lazyLoad(StdPicture &target);
    void lazyUnLoad(StdPicture &target);

    std::string m_screenshotPath;
    std::string m_gifRecordPath;

#ifndef __EMSCRIPTEN__
    static int makeShot_action(void *_pixels);
    SDL_Thread *m_screenshot_thread = nullptr;
#endif

    //Scale of virtual and window resolutuins
    float scale_x = 1.f;
    float scale_y = 1.f;
    //Side offsets to keep ratio
    float offset_x = 0.f;
    float offset_y = 0.f;
    //Offset to shake screen
    int viewport_offset_x = 0.f;
    int viewport_offset_y = 0.f;
    //Need to calculate relative viewport position when screen was scaled
    float viewport_scale_x = 1.0f;
    float viewport_scale_y = 1.0f;

    int viewport_x = 0;
    int viewport_y = 0;
    int viewport_w = 0;
    int viewport_h = 0;

    SDL_Point MapToScr(int x, int y);
};

#endif // FRMMAIN_H
