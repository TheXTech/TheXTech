/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef RENDER_HHHHHH
#define RENDER_HHHHHH

#include <string>
#include "std_picture.h"
#include "base/render_types.h"

#ifndef RENDER_CUSTOM
#   include "sdl_proxy/sdl_stdinc.h"
#   include "base/render_base.h"
#   define E_INLINE SDL_FORCE_INLINE
#   define TAIL
#else
#   define E_INLINE    extern
#   define TAIL ;
#endif


namespace XRender
{

#ifdef __3DS__

constexpr int MAX_3D_OFFSET = 20;

#endif

#ifdef RENDER_CUSTOM

extern bool init();
extern void quit();

#endif


/*!
 * \brief Identify does render engine works or not
 * \return true if render initialized and works
 */
E_INLINE bool isWorking() TAIL
#ifndef RENDER_CUSTOM
{
    return g_render->isWorking();
}
#endif

/*!
 * \brief Call the repaint
 */
E_INLINE void repaint() TAIL
#ifndef RENDER_CUSTOM
{
    g_render->repaint();
}
#endif

/*!
 * \brief Update viewport (mainly after screen resize)
 */
E_INLINE void updateViewport() TAIL
#ifndef RENDER_CUSTOM
{
    g_render->updateViewport();
}
#endif

/*!
 * \brief Reset viewport into default state
 */
E_INLINE void resetViewport() TAIL
#ifndef RENDER_CUSTOM
{
    g_render->resetViewport();
}
#endif

/*!
 * \brief Set the viewport area
 * \param x X position
 * \param y Y position
 * \param w Viewport Width
 * \param h Viewport Height
 */
E_INLINE void setViewport(int x, int y, int w, int h) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->setViewport(x, y, w, h);
}
#endif

/*!
 * \brief Set the render offset
 * \param x X offset
 * \param y Y offset
 *
 * All drawing objects will be drawn with a small offset
 */
E_INLINE void offsetViewport(int x, int y) TAIL // for screen-shaking
#ifndef RENDER_CUSTOM
{
    g_render->offsetViewport(x, y);
}
#endif

/*!
 * \brief Set temporary ignore of render offset
 * \param en Enable viewport offset ignore
 *
 * Use this to draw certain objects with ignorign of the GFX offset
 */
E_INLINE void offsetViewportIgnore(bool en) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->offsetViewportIgnore(en);
}
#endif

/*!
 * \brief Map absolute point coordinate into screen relative
 * \param x Window X position
 * \param y Window Y position
 * \param dx Destinition on-screen X position
 * \param dy Destinition on-screen Y position
 */
E_INLINE void mapToScreen(int x, int y, int *dx, int *dy) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->mapToScreen(x, y, dx, dy);
}
#endif


/*!
 * \brief Map screen relative coordinate into physical canvas
 * \param x On-screen X position
 * \param y On-screen Y position
 * \param dx Destinition window X position
 * \param dy Destinition window Y position
 */
E_INLINE void mapFromScreen(int x, int y, int *dx, int *dy) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->mapFromScreen(x, y, dx, dy);
}
#endif

/*!
 * \brief Set render target into the E_INLINE in-game screen (use to render in-game world)
 */
E_INLINE void setTargetTexture() TAIL
#ifndef RENDER_CUSTOM
{
    g_render->setTargetTexture();
}
#endif

/*!
 * \brief Set render target into the real window or screen (use to render on-screen buttons and other meta-info)
 */
E_INLINE void setTargetScreen() TAIL
#ifndef RENDER_CUSTOM
{
    g_render->setTargetScreen();
}
#endif

#ifdef __16M__
/*!
 * \brief Clear all currently loaded textures
 */
E_INLINE void clearAllTextures() TAIL
#endif

#ifdef __3DS__
/*!
 * \brief Set render target to a certain layer of the in-game screen
 */
E_INLINE void setTargetLayer(int layer) TAIL

/*!
 * \brief Set render target to directly render to the 3DS top screen
 */
E_INLINE void setTargetMainScreen() TAIL

/*!
 * \brief Set render target to directly render to the 3DS subscreen
 */
E_INLINE void setTargetSubScreen() TAIL
#endif


E_INLINE StdPicture LoadPicture(const std::string &path,
                                const std::string &maskPath = std::string(),
                                const std::string &maskFallbackPath = std::string()) TAIL
#ifndef RENDER_CUSTOM
{
    return AbstractRender_t::LoadPicture(path, maskPath, maskFallbackPath);
}
#endif

E_INLINE StdPicture lazyLoadPicture(const std::string &path,
                                    const std::string &maskPath = std::string(),
                                    const std::string &maskFallbackPath = std::string()) TAIL
#ifndef RENDER_CUSTOM
{
    return AbstractRender_t::lazyLoadPicture(path, maskPath, maskFallbackPath);
}
#endif

#ifdef PGE_MIN_PORT
E_INLINE StdPicture lazyLoadPictureFromList(FILE* f, const std::string& dir);
#endif

E_INLINE void setTransparentColor(StdPicture &target, uint32_t rgb) TAIL
#ifndef RENDER_CUSTOM
{
    AbstractRender_t::setTransparentColor(target, rgb);
}
#endif

E_INLINE void lazyLoad(StdPicture &target) TAIL
#ifndef RENDER_CUSTOM
{
    AbstractRender_t::lazyLoad(target);
}
#endif

E_INLINE void lazyUnLoad(StdPicture &target) TAIL
#ifndef RENDER_CUSTOM
{
    AbstractRender_t::lazyUnLoad(target);
}
#endif

E_INLINE void lazyPreLoad(StdPicture &target) TAIL
#ifndef RENDER_CUSTOM
{
    AbstractRender_t::lazyPreLoad(target);
}
#endif

E_INLINE size_t lazyLoadedBytes() TAIL
#ifndef RENDER_CUSTOM
{
    return AbstractRender_t::lazyLoadedBytes();
}
#endif

E_INLINE void lazyLoadedBytesReset() TAIL
#ifndef RENDER_CUSTOM
{
    AbstractRender_t::lazyLoadedBytesReset();
}
#endif



E_INLINE void deleteTexture(StdPicture &tx, bool lazyUnload = false) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->deleteTexture(tx, lazyUnload);
}
#endif

E_INLINE void clearBuffer() TAIL
#ifndef RENDER_CUSTOM
{
    g_render->clearBuffer();
}
#endif



// Draw primitives

E_INLINE void renderRect(int x, int y, int w, int h,
                        float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                        bool filled = true) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderRect(x, y, w, h,
                         red, green, blue, alpha,
                         filled);
}
#endif

E_INLINE void renderRectBR(int _left, int _top, int _right, int _bottom,
                           float red, float green, float blue, float alpha) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderRectBR(_left, _top, _right, _bottom,
                           red, green, blue, alpha);
}
#endif

E_INLINE void renderCircle(int cx, int cy,
                          int radius,
                          float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                          bool filled = true) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderCircle(cx, cy,
                           radius,
                           red, green, blue, alpha,
                           filled);
}
#endif

E_INLINE void renderCircleHole(int cx, int cy,
                              int radius,
                              float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderCircleHole(cx, cy,
                               radius,
                               red, green, blue, alpha);
}
#endif


// Draw texture

E_INLINE void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          int wSrc, int hSrc,
                          double rotateAngle = 0.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                          float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderTextureScaleEx(xDst, yDst, wDst, hDst,
                                   tx,
                                   xSrc, ySrc,
                                   wSrc, hSrc,
                                   rotateAngle, center, flip,
                                   red, green, blue, alpha);
}
#endif

E_INLINE void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                        StdPicture &tx,
                        float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderTextureScale(xDst, yDst, wDst, hDst,
                                 tx,
                                 red, green, blue, alpha);
}
#endif

E_INLINE void renderTexture(double xDst, double yDst, double wDst, double hDst,
                           StdPicture &tx,
                           int xSrc, int ySrc,
                           float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderTexture(xDst, yDst, wDst, hDst,
                            tx,
                            xSrc, ySrc,
                            red, green, blue, alpha);
}
#endif

E_INLINE void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                             StdPicture &tx,
                             int xSrc, int ySrc,
                             double rotateAngle = 0.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                             float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderTextureFL(xDst, yDst, wDst, hDst,
                              tx,
                              xSrc, ySrc,
                              rotateAngle, center, flip,
                              red, green, blue, alpha);
}
#endif

E_INLINE void renderTexture(float xDst, float yDst, StdPicture &tx,
                           float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) TAIL
#ifndef RENDER_CUSTOM
{
    g_render->renderTexture(xDst, yDst, tx,
                            red, green, blue, alpha);
}
#endif


#ifdef USE_RENDER_BLOCKING
E_INLINE bool renderBlocked() TAIL
#   ifndef RENDER_CUSTOM
{
    return AbstractRender_t::renderBlocked();
}
#   endif

E_INLINE void setBlockRender(bool b) TAIL
#   ifndef RENDER_CUSTOM
{
    AbstractRender_t::setBlockRender(b);
}
#   endif
#endif // USE_RENDER_BLOCKING


#ifdef USE_SCREENSHOTS_AND_RECS

E_INLINE void makeShot() TAIL
#   ifndef RENDER_CUSTOM
{
    AbstractRender_t::makeShot();
}
#   endif

E_INLINE void toggleGifRecorder() TAIL
#   ifndef RENDER_CUSTOM
{
    AbstractRender_t::toggleGifRecorder();
}
#   endif

E_INLINE void processRecorder() TAIL
#   ifndef RENDER_CUSTOM
{
    AbstractRender_t::processRecorder();
}
#   endif

#endif // USE_SCREENSHOTS_AND_RECS


} // XRender

#ifndef RENDER_CUSTOM
#   undef E_INLINE
#   undef TAIL
#endif


#endif // RENDER_HHHHHH
