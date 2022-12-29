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

#pragma once
#ifndef ABTRACTRENDER_T_H
#define ABTRACTRENDER_T_H

#include <string>

#include "std_picture.h"
#include "render_types.h"

#ifdef USE_SCREENSHOTS_AND_RECS
struct GifRecorder;
#endif

typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_mutex SDL_mutex;


class AbstractRender_t
{
    friend class FrmMain;

    static size_t m_lazyLoadedBytes;

protected:
    //! Maximum texture width
    static int    m_maxTextureWidth;
    //! Maximum texture height
    static int    m_maxTextureHeight;

    static int    ScaleWidth;
    static int    ScaleHeight;

#ifdef USE_RENDER_BLOCKING
    static bool m_blockRender;
#endif

public:
    AbstractRender_t();
    virtual ~AbstractRender_t();


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
    virtual void close();

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
     * \brief Map absolute point coordinate into screen relative
     * \param x Window X position
     * \param y Window Y position
     * \param dx Destinition on-screen X position
     * \param dy Destinition on-screen Y position
     */
    virtual void mapToScreen(int x, int y, int *dx, int *dy) = 0;

    /*!
     * \brief Map screen relative coordinate into physical canvas
     * \param x On-screen X position
     * \param y On-screen Y position
     * \param dx Destinition window X position
     * \param dy Destinition window Y position
     */
    virtual void mapFromScreen(int x, int y, int *dx, int *dy) = 0;

    /*!
     * \brief Set render target into the virtual in-game screen (use to render in-game world)
     */
    virtual void setTargetTexture() = 0;

    /*!
     * \brief Set render target into the real window or screen (use to render on-screen buttons and other meta-info)
     */
    virtual void setTargetScreen() = 0;




    // Load and unload textures

    static StdPicture LoadPicture(const std::string &path,
                                  const std::string &maskPath = std::string(),
                                  const std::string &maskFallbackPath = std::string());

    static StdPicture lazyLoadPicture(const std::string &path,
                                      const std::string &maskPath = std::string(),
                                      const std::string &maskFallbackPath = std::string());

    static void setTransparentColor(StdPicture &target, uint32_t rgb);

    virtual void loadTexture(StdPicture &target,
                             uint32_t width,
                             uint32_t height,
                             uint8_t *RGBApixels,
                             uint32_t pitch) = 0;

    static void lazyLoad(StdPicture &target);
    static void lazyUnLoad(StdPicture &target);
    static void lazyPreLoad(StdPicture &target);

    static size_t lazyLoadedBytes();
    static void lazyLoadedBytesReset();

    virtual void deleteTexture(StdPicture &tx, bool lazyUnload = false) = 0;
    virtual void clearAllTextures() = 0;

    virtual void clearBuffer() = 0;




    // Draw primitives

    virtual void renderRect(int x, int y, int w, int h,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                            bool filled = true) = 0;

    virtual void renderRectBR(int _left, int _top, int _right, int _bottom,
                              float red, float green, float blue, float alpha) = 0;

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



#ifdef USE_RENDER_BLOCKING
    static bool renderBlocked();
    static void setBlockRender(bool b);
#endif

    // Screenshots, GIF recordings, etc., etc.
#ifdef USE_DRAW_BATTERY_STATUS
    static void drawBatteryStatus();
#endif

#ifdef USE_SCREENSHOTS_AND_RECS
    static void makeShot();

    static void toggleGifRecorder();
    static void processRecorder();

private:
    static GifRecorder *m_gif;
    static bool recordInProcess();
#endif // USE_SCREENSHOTS_AND_RECS

};


//! Globally available renderer instance
extern AbstractRender_t* g_render;


#endif // ABTRACTRENDER_T_H
