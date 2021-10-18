/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <SDL2/SDL_version.h>
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

// Workaround for older SDL versions that lacks the floating-point based rects and points
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 10)
#define XTECH_SDL_NO_RECTF_SUPPORT
typedef struct SDL_FPoint
{
    float x;
    float y;
} SDL_FPoint;
#endif


class FrmMain
{
    std::string m_windowTitle;
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_gRenderer = nullptr;
    SDL_Texture  *m_tBuffer = nullptr;
    SDL_Texture  *m_recentTarget = nullptr;
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

    /*!
     * \brief Set render target into the virtual in-game screen (use to render in-game world)
     */
    void setTargetTexture();

    /*!
     * \brief Set render target into the real window or screen (use to render on-screen buttons and other meta-info)
     */
    void setTargetScreen();


    StdPicture LoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    StdPicture lazyLoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    void deleteTexture(StdPicture &tx, bool lazyUnload = false);
    void clearAllTextures();

    void clearBuffer();
    void renderRect(int x, int y, int w, int h, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
    void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);

    void renderCircle(int cx, int cy, int radius, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);

    // Similar to BitBlt, but without masks, just draw a texture or it's fragment!
//    void renderTextureI(int xDst, int yDst, int wDst, int hDst,
//                        StdPicture &tx,
//                        int xSrc, int ySrc,
//                        double rotateAngle = 0.0, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
//                        float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
//    void renderTextureScaleI(int xDst, int yDst, int wDst, int hDst,
//                             StdPicture &tx,
//                             int xSrc, int ySrc,
//                             int wSrc, int hSrc,
//                             double rotateAngle = 0.0, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
//                             float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
    void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            int wSrc, int hSrc,
                            double rotateAngle = 0.0, SDL_FPoint *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTexture(double xDst, double yDst, double wDst, double hDst,
                       StdPicture &tx,
                       int xSrc, int ySrc,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                         StdPicture &tx,
                         int xSrc, int ySrc,
                         double rotateAngle = 0.0, SDL_FPoint *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                         float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTexture(float xDst, float yDst, StdPicture &tx,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
//    void renderTextureScale(int xDst, int yDst, int wDst, int hDst,
//                            StdPicture &tx,
//                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

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
    void drawBatteryStatus();

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
