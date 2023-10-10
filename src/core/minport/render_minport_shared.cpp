/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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


#include <malloc.h>

#include <set>

#include <Logger/logger.h>

#include "globals.h"
#include "video.h"
#include "frame_timer.h"
#include "core/window.h"
#include "core/render.h"
#include "core/minport/render_minport_shared.h"


namespace XRender
{

int g_viewport_x = 0;
int g_viewport_y = 0;
int g_viewport_w = 0;
int g_viewport_h = 0;
int g_viewport_offset_x = 0;
int g_viewport_offset_y = 0;
bool g_viewport_offset_ignore = false;

int g_screen_phys_x = 0;
int g_screen_phys_y = 0;
int g_screen_phys_w = 0;
int g_screen_phys_h = 0;

StdPicture* g_render_chain_head = nullptr;
StdPicture* g_render_chain_tail = nullptr;

uint32_t g_current_frame = 0;

void updateViewport()
{
    resetViewport();
    offsetViewport(0, 0);

    // calculate physical render coordinates

    pLogDebug("Updating viewport. Game screen is %d x %d", ScreenW, ScreenH);

    int hardware_w, hardware_h;
    XWindow::getWindowSize(&hardware_w, &hardware_h);

    hardware_w /= 2;
    hardware_h /= 2;

    int ScreenW_Show = ScreenW;

    if(g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR || g_videoSettings.scaleMode == SCALE_DYNAMIC_NEAREST)
    {
        int res_h = hardware_h;
        int res_w = ScreenW_Show * hardware_h / ScreenH;

        if(res_w > hardware_w)
        {
            res_w = hardware_w;
            res_h = ScreenH * res_w / ScreenW;
        }

        g_screen_phys_w = res_w;
        g_screen_phys_h = res_h;
    }
    else if(g_videoSettings.scaleMode == SCALE_FIXED_1X)
    {
        g_screen_phys_w = ScreenW_Show / 2;
        g_screen_phys_h = ScreenH / 2;
    }
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
    {
        g_screen_phys_w = ScreenW_Show;
        g_screen_phys_h = ScreenH;
    }
    else if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
    {
        g_screen_phys_w = ScreenW_Show / 4;
        g_screen_phys_h = ScreenH / 4;
    }
    else if(g_videoSettings.scaleMode == SCALE_DYNAMIC_INTEGER)
    {
        g_screen_phys_w = ScreenW_Show / 2;
        g_screen_phys_h = ScreenH / 2;
        while(g_screen_phys_w <= hardware_w && g_screen_phys_h <= hardware_h)
        {
            g_screen_phys_w += ScreenW_Show / 2;
            g_screen_phys_h += ScreenH / 2;
        }
        if(g_screen_phys_w > ScreenW_Show / 2)
        {
            g_screen_phys_w -= ScreenW_Show / 2;
            g_screen_phys_h -= ScreenH / 2;
        }
    }

    pLogDebug("Phys screen is %d x %d", g_screen_phys_w, g_screen_phys_h);

    g_screen_phys_x = hardware_w / 2 - g_screen_phys_w / 2;
    g_screen_phys_y = hardware_h / 2 - g_screen_phys_h / 2;

    // for widescreen stretch mode on Wii
    minport_TransformPhysCoords();

    minport_ApplyPhysCoords();
}

void resetViewport()
{
    setViewport(0, 0, ScreenW, ScreenH);
}

void setViewport(int x, int y, int w, int h)
{
    if(g_viewport_x == x / 2 && g_viewport_y == y / 2 && g_viewport_w == w / 2 && g_viewport_h == h / 2)
        return;

    g_viewport_x = x / 2;
    g_viewport_y = y / 2;
    g_viewport_w = w / 2;
    g_viewport_h = h / 2;

    minport_ApplyViewport();
}

void offsetViewport(int x, int y)
{
    if(g_viewport_offset_x == x / 2 && g_viewport_offset_y == y / 2)
        return;

    g_viewport_offset_x = x / 2;
    g_viewport_offset_y = y / 2;

    minport_ApplyViewport();
}

void offsetViewportIgnore(bool en)
{
    if(g_viewport_offset_ignore == en)
        return;

    g_viewport_offset_ignore = en;

    minport_ApplyViewport();
}

void splitFrame()
{
    /* empty */
}

void setTransparentColor(StdPicture &target, uint32_t rgb)
{
#if defined(__WII__) || defined(__3DS__)
    target.l.colorKey = true;
    target.l.keyRgb[0] = (rgb >> 0) & 0xFF;
    target.l.keyRgb[1] = (rgb >> 8) & 0xFF;
    target.l.keyRgb[2] = (rgb >> 16) & 0xFF;
#else
    UNUSED(target);
    UNUSED(rgb);
#endif
}

void getRenderSize(int* w, int* h)
{
    return XWindow::getWindowSize(w, h);
}

inline int ROUNDDIV2(int x)
{
    return (x < 0) ? (x - 1) / 2 : (x + 1) / 2;
}

inline float ROUNDDIV2(float x)
{
    return std::roundf(x / 2.0f);
}

inline double ROUNDDIV2(double x)
{
    return std::round(x / 2.0);
}

inline int FLOORDIV2(int x)
{
    return (x < 0) ? (x - 1) / 2 : x / 2;
}

inline float FLOORDIV2(float x)
{
    return std::floor(std::roundf(x) / 2.0f);
}

inline double FLOORDIV2(double x)
{
    return std::floor(std::round(x) / 2.0);
}

#ifndef __WII__
void minport_RenderBoxUnfilled(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    minport_RenderBoxFilled(x1, y1, x1 + 1, y2, r, g, b, a);
    minport_RenderBoxFilled(x2 - 1, y1, x2, y2, r, g, b, a);
    minport_RenderBoxFilled(x1, y1, x2, y1 + 1, r, g, b, a);
    minport_RenderBoxFilled(x1, y2 - 1, x2, y2, r, g, b, a);
}
#endif

void renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    int x_div = FLOORDIV2(x);
    int w_div = FLOORDIV2(x + w) - x_div;

    int y_div = FLOORDIV2(y);
    int h_div = FLOORDIV2(y + h) - y_div;

    uint8_t r = red * 255.0f + 0.5f;
    uint8_t g = green * 255.0f + 0.5f;
    uint8_t b = blue * 255.0f + 0.5f;
    uint8_t a = alpha * 255.0f + 0.5f;

    if(filled)
        minport_RenderBoxFilled(x_div, y_div, x_div + w_div, y_div + h_div, r, g, b, a);
    else
        minport_RenderBoxUnfilled(x_div, y_div, x_div + w_div, y_div + h_div, r, g, b, a);
}

void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
    renderRect(_left, _top, _right-_left, _bottom-_top, red, green, blue, alpha, true);
}

void renderCircle(int cx, int cy,
                  int radius,
                  float red , float green, float blue, float alpha,
                  bool filled)
{
    UNUSED(cx);
    UNUSED(cy);
    UNUSED(radius);
    UNUSED(red);
    UNUSED(green);
    UNUSED(blue);
    UNUSED(alpha);
    UNUSED(filled);
    // TODO: Implement this: it's needed for GIF recording status
}

void renderCircleHole(int cx, int cy,
                      int radius,
                      float red, float green, float blue, float alpha)
{
    if(radius <= 0)
        return; // Nothing to draw

    double line_size = 4;
    double dy = line_size;

    do
    {
        double dx = std::floor(std::sqrt((2.0 * radius * dy) - (dy * dy)));

        renderRectBR(cx - radius, cy + dy - radius - line_size, cx - dx, cy + dy - radius + line_size,
            red, green, blue, alpha);

        renderRectBR(cx + dx, cy + dy - radius - line_size, cx + radius, cy + dy - radius + line_size,
            red, green, blue, alpha);

        if(dy < radius) // Don't cross lines
        {
            renderRectBR(cx - radius, cy - dy + radius - line_size, cx - dx, cy - dy + radius + line_size,
                red, green, blue, alpha);

            renderRectBR(cx + dx, cy - dy + radius - line_size, cx + radius, cy - dy + radius + line_size,
                red, green, blue, alpha);
        }

        dy += line_size * 2;
    } while(dy + line_size <= radius);
}

// texture chain methods

// increment the frame counter, and unload all textures not rendered since g_always_unload_after
void minport_initFrame()
{
    g_current_frame++;

    int num_unloaded = 0;

    while(g_render_chain_tail && g_current_frame - g_render_chain_tail->d.last_draw_frame > g_always_unload_after)
    {
        StdPicture* last_tail = g_render_chain_tail;

        // will internally invoke minport_unlinkTexture if written properly
        unloadTexture(*last_tail);
        num_unloaded++;

        if(g_render_chain_tail == last_tail)
        {
            pLogCritical("Failed to unlink texture during unloadTexture! Manually unlinking texture. VRAM may be leaked.");
            minport_unlinkTexture(g_render_chain_tail);
        }
    }

    if(num_unloaded > 0)
    {
        pLogDebug("Unloaded %d stale textures at frame start", num_unloaded);
    }
}

// removes a texture from the render chain
void minport_unlinkTexture(StdPicture* tx)
{
    // redirect the tail and head
    if(tx == g_render_chain_tail)
        g_render_chain_tail = tx->d.next_texture;

    if(tx == g_render_chain_head)
        g_render_chain_head = tx->d.last_texture;

    // unlink from its context
    if(tx->d.last_texture)
        tx->d.last_texture->d.next_texture = tx->d.next_texture;

    if(tx->d.next_texture)
        tx->d.next_texture->d.last_texture = tx->d.last_texture;

    tx->d.last_texture = nullptr;
    tx->d.next_texture = nullptr;
}

// unload all textures not rendered since g_never_unload_before
void minport_freeTextureMemory()
{
    int num_unloaded = 0;

    while(g_render_chain_tail && g_current_frame - g_render_chain_tail->d.last_draw_frame > g_never_unload_before)
    {
        StdPicture* last_tail = g_render_chain_tail;

        // will internally invoke minport_unlinkTexture if written properly
        unloadTexture(*last_tail);
        num_unloaded++;

        if(g_render_chain_tail == last_tail)
        {
            pLogCritical("Failed to unlink texture during unloadTexture! Manually unlinking texture. VRAM may be leaked.");
            minport_unlinkTexture(g_render_chain_tail);
        }
    }

    pLogDebug("Unloaded %d stale textures at free texture memory request", num_unloaded);
}

#ifdef __16M__

//! stub for 16M since it doesn't support masks
void unloadGifTextures() {}

#else

void unloadGifTextures()
{
    for(StdPicture* p = g_render_chain_tail; p != nullptr;)
    {
        StdPicture* last_p = p;
        p = p->d.next_texture;

        if(!last_p->l.mask_path.empty())
        {
            D_pLogDebug("XRender: unloading texture at %p on unloadGifTextures()", last_p);
            unloadTexture(*last_p);
        }
    }
}

#endif

// intermediate draw method

inline void minport_RenderTexturePrivate_2(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                             StdPicture &tx,
                             int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                             float rotateAngle, FPoint_t *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
    if(wDst <= 0 || hDst <= 0)
        return;

    minport_RenderTexturePrivate(xDst, yDst, wDst, hDst,
                             tx,
                             xSrc, ySrc, wSrc, hSrc,
                             rotateAngle, center, flip,
                             red, green, blue, alpha);

    if(tx.inited && tx.l.lazyLoaded && &tx != g_render_chain_head)
    {
        tx.d.last_draw_frame = g_current_frame;

        // unlink
        minport_unlinkTexture(&tx);

        // insert at head
        if(g_render_chain_head)
        {
            g_render_chain_head->d.next_texture = &tx;
            tx.d.last_texture = g_render_chain_head;
        }
        else
        {
            g_render_chain_tail = &tx;
        }

        g_render_chain_head = &tx;
    }
}

// public draw methods

void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            float red, float green, float blue, float alpha)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);

    minport_RenderTexturePrivate_2(
        div_x, div_y, FLOORDIV2(xDst + wDst) - div_x, FLOORDIV2(yDst + hDst) - div_y,
        tx,
        0.0f, 0.0f, tx.w / 2, tx.h / 2,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc, int wSrc, int hSrc,
                            float red, float green, float blue, float alpha)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);

    minport_RenderTexturePrivate_2(
        div_x, div_y, FLOORDIV2(xDst + wDst) - div_x, FLOORDIV2(yDst + hDst) - div_y,
        tx,
        xSrc / 2, ySrc / 2, wSrc / 2, hSrc / 2,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTexture(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            float red, float green, float blue, float alpha)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);
    auto div_w = FLOORDIV2(xDst + wDst) - div_x;
    auto div_h = FLOORDIV2(yDst + hDst) - div_y;

    minport_RenderTexturePrivate_2(
        div_x, div_y, div_w, div_h,
        tx,
        FLOORDIV2(xSrc), FLOORDIV2(ySrc), div_w, div_h,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTexture(float xDst, float yDst, StdPicture &tx,
                   float red, float green, float blue, float alpha)
{
    int w = tx.w / 2;
    int h = tx.h / 2;

    minport_RenderTexturePrivate_2(
        FLOORDIV2(xDst), FLOORDIV2(yDst), w, h,
        tx,
        0, 0, w, h,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTexture(int xDst, int yDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
    int w = tx.w / 2;
    int h = tx.h / 2;

    minport_RenderTexturePrivate_2(
        FLOORDIV2(xDst), FLOORDIV2(yDst), w, h,
        tx,
        0.0f, 0.0f, w, h,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTextureScale(int xDst, int yDst, int wDst, int hDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);
    auto div_w = FLOORDIV2(xDst + wDst) - div_x;
    auto div_h = FLOORDIV2(yDst + hDst) - div_y;

    minport_RenderTexturePrivate_2(
        div_x, div_y, div_w, div_h,
        tx,
        0.0f, 0.0f, tx.w / 2, tx.h / 2,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);
    auto div_w = FLOORDIV2(xDst + wDst) - div_x;
    auto div_h = FLOORDIV2(yDst + hDst) - div_y;

    minport_RenderTexturePrivate_2(
        div_x, div_y, div_w, div_h,
        tx,
        FLOORDIV2(xSrc), FLOORDIV2(ySrc), div_w, div_h,
        rotateAngle, center, flip,
        red, green, blue, alpha);
}

void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          int wSrc, int hSrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);
    auto div_w = FLOORDIV2(xDst + wDst) - div_x;
    auto div_h = FLOORDIV2(yDst + hDst) - div_y;

    auto div_sx = FLOORDIV2(xSrc), div_sy = FLOORDIV2(ySrc);
    auto div_sw = FLOORDIV2(xSrc + wSrc) - div_sx;
    auto div_sh = FLOORDIV2(ySrc + hSrc) - div_sy;

    minport_RenderTexturePrivate_2(
        div_x, div_y, div_w, div_h,
        tx,
        div_sx, div_sy, div_sw, div_sh,
        rotateAngle, center, flip,
        red, green, blue, alpha);
}


size_t lazyLoadedBytes()
{
    return 0;
}

void lazyLoadedBytesReset()
{}

} // namespace XRender
