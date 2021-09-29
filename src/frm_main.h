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

#ifndef NO_SDL
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#else
#include "SDL_supplement.h"
#endif

#ifdef __3DS__
#include <citro3d.h>
#include <citro2d.h>
#endif

#ifdef VITA
#ifdef USE_VITA2D
#include <vita2d.h>
#include <FreeImageLite.h>
#endif
#endif

#include <string>
#include <set>

#include "std_picture.h"
#include "cmd_line_setup.h"

#ifndef NO_SCREENSHOT
#include <deque>
#include <gif_writer.h>
typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_mutex SDL_mutex;
#endif

class FrmMain
{
    bool m_headless = false;
    std::string m_windowTitle;
// this is a mess now, must clean up.
#ifndef NO_SDL
    SDL_Window *m_window = nullptr;
#    if !defined(VITA) || defined(USE_SDL_VID)
    SDL_Renderer *m_gRenderer = nullptr;
    SDL_Texture  *m_tBuffer = nullptr;
    SDL_Texture  *m_recentTarget = nullptr;
    std::set<SDL_Texture *> m_textureBank;
#    else
    // VITA SPECIFIC
#        ifdef USE_VITA2D
    std::set<struct vita2d_texture*> m_textureBank;
#        else
    std::set<GLuint> m_textureBank;
    GLuint m_renderTarget;
#        endif
    uint32_t currentFrame = 0;
    
#    endif
    bool m_sdlLoaded = false;
    const Uint8 *m_keyboardState = nullptr;
    Uint32 m_lastMousePress = 0;
    SDL_Event m_event;
    SDL_RendererInfo m_ri;
#endif
#ifdef __3DS__
    std::set<C2D_SpriteSheet> m_textureBank; // SDL_Texture
    std::set<StdPicture*> m_bigPictures;
    uint32_t currentFrame = 0;
    touchPosition m_lastMousePosition = {0, 0};
    float depthSlider = 0.;

    uint32_t keys_held = 0;
    uint32_t keys_pressed = 0;
    uint32_t keys_released = 0;

    C3D_RenderTarget* top;
    C3D_RenderTarget* right;
    C3D_RenderTarget* bottom;
    Tex3DS_SubTexture layer_subtexs[4];
    C3D_Tex layer_texs[4];
    C2D_Image layer_ims[4];
    C3D_RenderTarget* layer_targets[4];
#endif
#ifdef __ANDROID__
    bool m_blockRender = false;
#endif

    size_t m_lazyLoadedBytes = 0;

public:
#ifdef VITA
    // TODO: Make this private and put in proper getters/setters.
    void (*_debugPrintf_)(const char*, ...);    
#endif
    int ScaleWidth = 800;
    int ScaleHeight = 600;

    int MousePointer = 0;

    FrmMain();

#ifndef NO_SDL
    SDL_Window *getWindow();
    Uint8 getKeyState(SDL_Scancode key);
#else
    inline bool getKeyHeld(int id)
    {
        return id & keys_held;
    }
    inline bool getKeyPressed(int id)
    {
        return id & keys_pressed;
    }
    inline bool getKeyReleased(int id)
    {
        return id & keys_released;
    }
#endif

    bool initSDL(const CmdLineSetup_t &setup);
    void freeSDL();

    void show();
    void hide();
    void doEvents();
    void waitEvents();

    bool isWindowActive();
    bool hasWindowMouseFocus();

#ifndef NO_SDL
    void eventDoubleClick();
    void eventKeyPress(SDL_Scancode KeyASCII);
    void eventKeyDown(SDL_KeyboardEvent &evt);
    void eventKeyUp(SDL_KeyboardEvent &evt);
    void eventMouseDown(SDL_MouseButtonEvent &m_event);
    void eventMouseMove(SDL_MouseMotionEvent &m_event);
    void eventMouseUp(SDL_MouseButtonEvent &m_event);
#endif

    void eventResize();
    int setFullScreen(bool fs);
    bool isSdlError();

#ifdef __3DS__
    bool inFrame = false;
    void initDraw(int screen = 0);
    void setLayer(int layer);
    void toggleDebug();
    void cancelFrame();
#endif

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
#if defined(__3DS__) || (defined(VITA) && !defined(USE_SDL_VID))
    bool freeTextureMem();
#endif

    void clearBuffer();
    void renderRect(int x, int y, int w, int h, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
    void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);

    // this is extremely difficult to implement on some platforms, including 3DS, where circle mode is distinct from polygon mode.
#ifndef __3DS__
    void renderCircle(int cx, int cy, int radius, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
#endif

    // these operate in render coordinates on 3DS and should not be called by external units
private:
    // Similar to BitBlt, but without masks, just draw a texture or it's fragment!
#if defined(__3DS__) || (defined(VITA) && !defined(USE_SDL_VID))
    void renderTexturePrivate(float xDst, float yDst, float wDst, float hDst,
                             StdPicture &tx,
                             float xSrc, float ySrc, float wSrc, float hSrc,
                             float rotateAngle = 0.f, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                             float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
#endif
    // 3DS only handles rotation and flipping correctly for non-huge images
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
public:
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

#ifndef NO_SCREENSHOT
    void makeShot();
#endif

private:

#ifndef __EMSCRIPTEN__
    void drawBatteryStatus();
#endif

#ifndef NO_SCREENSHOT
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
#ifndef __3DS__
    void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels);
#else
    void loadTexture(StdPicture &target, C2D_SpriteSheet &sheet);
    void loadTexture2(StdPicture &target, C2D_SpriteSheet &sheet);
    void loadTexture3(StdPicture &target, C2D_SpriteSheet &sheet);
#endif

    void lazyLoad(StdPicture &target);
    void lazyUnLoad(StdPicture &target);

#ifndef NO_SCREENSHOT
    std::string m_screenshotPath;
    std::string m_gifRecordPath;

    static int makeShot_action(void *_pixels);
    SDL_Thread *m_screenshot_thread = nullptr;
#endif

    //Scale of virtual and window resolutuins
    float scale = 1.f;
    //Side offsets to keep ratio
    float offset_x = 0.f;
    float offset_y = 0.f;
    //Offset to shake screen
    int viewport_offset_x = 0;
    int viewport_offset_y = 0;
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
