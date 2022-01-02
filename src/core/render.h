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

#pragma once
#ifndef RENDER_H
#define RENDER_H

#ifndef __EMSCRIPTEN__
#include <deque>
#endif
#include <string>
#include <gif_writer.h>

#include "../std_picture.h"

#ifndef __EMSCRIPTEN__
#   define USE_SCREENSHOTS_AND_RECS
#endif


typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_mutex SDL_mutex;


enum RendererFlip_t
{
    X_FLIP_NONE       = 0x00000000,    /**< Do not flip */
    X_FLIP_HORIZONTAL = 0x00000001,    /**< flip horizontally */
    X_FLIP_VERTICAL   = 0x00000002     /**< flip vertically */
};

typedef struct FPoint_t
{
    float x;
    float y;
} FPoint_t;


class AbstractRender_t
{
    friend class FrmMain;

    size_t m_lazyLoadedBytes = 0;

protected:
    //! Maximum texture width
    int    m_maxTextureWidth = 0;
    //! Maximum texture height
    int    m_maxTextureHeight = 0;

    int    ScaleWidth = 0;
    int    ScaleHeight = 0;

#ifdef __ANDROID__
    bool m_blockRender = false;
#endif

public:
    AbstractRender_t() = default;
    virtual ~AbstractRender_t() = default;


    /*!
     * \brief Flags needed to initialize SDL-based window
     * \return Bitwise flags of SDL Window or 0 if no special flags set
     */
    virtual unsigned int SDL_InitFlags() = 0;


    /*!
     * \brief Identify does render engine works or not
     * \return true if render initialized and works
     */
    virtual bool isWorking() = 0;

    /*!
     * \brief Initialize defaults of the renderer
     * \return false on error, true on success
     */
    virtual bool init();

    /*!
     * \brief Close the renderer
     */
    virtual void close() = 0;

    /*!
     * \brief Call the repaint
     */
    virtual void repaint() = 0;

    /*!
     * \brief Update viewport (mainly after screen resize)
     */
    virtual void updateViewport() = 0;

    /*!
     * \brief Reset viewport into default state
     */
    virtual void resetViewport() = 0;

    /*!
     * \brief Set the viewport area
     * \param x X position
     * \param y Y position
     * \param w Viewport Width
     * \param h Viewport Height
     */
    virtual void setViewport(int x, int y, int w, int h) = 0;

    /*!
     * \brief Set the render offset
     * \param x X offset
     * \param y Y offset
     *
     * All drawing objects will be drawn with a small offset
     */
    virtual void offsetViewport(int x, int y) = 0; // for screen-shaking

    /*!
     * \brief Set temporary ignore of render offset
     * \param en Enable viewport offset ignore
     *
     * Use this to draw certain objects with ignorign of the GFX offset
     */
    virtual void offsetViewportIgnore(bool en) = 0;

    /*!
     * \brief Set render target into the virtual in-game screen (use to render in-game world)
     */
    virtual void setTargetTexture() = 0;

    /*!
     * \brief Set render target into the real window or screen (use to render on-screen buttons and other meta-info)
     */
    virtual void setTargetScreen() = 0;




    // Load and unload textures

    StdPicture LoadPicture(const std::string &path,
                           const std::string &maskPath = std::string(),
                           const std::string &maskFallbackPath = std::string());

    StdPicture lazyLoadPicture(const std::string &path,
                               const std::string &maskPath = std::string(),
                               const std::string &maskFallbackPath = std::string());

    virtual void loadTexture(StdPicture &target,
                             uint32_t width,
                             uint32_t height,
                             uint8_t *RGBApixels,
                             uint32_t pitch) = 0;

    void lazyLoad(StdPicture &target);
    void lazyUnLoad(StdPicture &target);

    virtual void deleteTexture(StdPicture &tx, bool lazyUnload = false) = 0;
    virtual void clearAllTextures() = 0;

    virtual void clearBuffer() = 0;




    // Draw primitives

    virtual void renderRect(int x, int y, int w, int h,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                            bool filled = true) = 0;

    virtual void renderRectBR(int _left, int _top, int _right,
                              int _bottom, float red, float green, float blue, float alpha) = 0;

    virtual void renderCircle(int cx, int cy,
                              int radius,
                              float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                              bool filled = true) = 0;

    virtual void renderCircleHole(int cx, int cy,
                                  int radius,
                                  float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) =0;




    // Draw texture

    virtual void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                              StdPicture &tx,
                              int xSrc, int ySrc,
                              int wSrc, int hSrc,
                              double rotateAngle = 0.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                              float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) = 0;

    virtual void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) = 0;

    virtual void renderTexture(double xDst, double yDst, double wDst, double hDst,
                               StdPicture &tx,
                               int xSrc, int ySrc,
                               float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) = 0;

    virtual void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                                 StdPicture &tx,
                                 int xSrc, int ySrc,
                                 double rotateAngle = 0.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                                 float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) = 0;

    virtual void renderTexture(float xDst, float yDst, StdPicture &tx,
                               float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) = 0;



    // Retrieve raw pixel data

    virtual void getScreenPixels(int x, int y, int w, int h, unsigned char *pixels) = 0;

    virtual void getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels) = 0;

    virtual int  getPixelDataSize(const StdPicture &tx) = 0;

    virtual void getPixelData(const StdPicture &tx, unsigned char *pixelData) = 0;



    // Screenshots, GIF recordings, etc., etc.
#ifdef __ANDROID__
    bool renderBlocked();
#endif

#ifdef USE_SCREENSHOTS_AND_RECS
    void makeShot();

    static int makeShot_action(void *_pixels);
    SDL_Thread *m_screenshot_thread = nullptr;


    void drawBatteryStatus();

    struct PGE_GL_shoot
    {
        AbstractRender_t *me = nullptr;
        uint8_t *pixels = nullptr;
        int pitch = 0;
        int w = 0, h = 0;
    };

    struct GifRecorder
    {
        AbstractRender_t *m_self = nullptr;
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

        void init(AbstractRender_t *self);
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
    static int processRecorder_action(void *_recorder);

    std::string m_screenshotPath;
    std::string m_gifRecordPath;
#endif // USE_SCREENSHOTS_AND_RECS

};


#endif // RENDER_H
