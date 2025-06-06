﻿/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "config.h"
#include "frame_timer.h"
#include "core/window.h"
#include "core/render.h"
#include "core/minport/render_minport_shared.h"


namespace XRender
{

int TargetW = 800;
int TargetH = 600;

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

#ifdef __WII__
extern void video_set_rmode();
#endif

void updateViewport()
{
#ifdef __WII__
    video_set_rmode();
#endif

    resetViewport();
    offsetViewport(0, 0);

    // calculate physical render coordinates

    pLogDebug("Updating viewport. Game screen is %d x %d", TargetW, TargetH);

    int hardware_w, hardware_h;
    XWindow::getWindowSize(&hardware_w, &hardware_h);

    hardware_w /= 2;
    hardware_h /= 2;

#ifdef __3DS__
    int TargetW_Show = TargetW - MAX_3D_OFFSET * 2;
#else
    int TargetW_Show = TargetW;
#endif

    g_screen_phys_w = TargetW_Show / 2;
    g_screen_phys_h = TargetH / 2;

    if(g_screen_phys_w > hardware_w)
    {
        g_screen_phys_w = hardware_w;
        g_screen_phys_h = TargetH * g_screen_phys_w / TargetW_Show;
    }

    if(g_screen_phys_h > hardware_h)
    {
        g_screen_phys_h = hardware_h;
        g_screen_phys_w = TargetW_Show * g_screen_phys_h / TargetH;
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
    setViewport(0, 0, TargetW, TargetH);
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
#ifdef __WII__
    *w = 1280;
    *h = (CONF_GetAspectRatio()) ? 720 : 960;
#else
    return XWindow::getWindowSize(w, h);
#endif
}

static inline int FLOORDIV2(int x)
{
    return (x < 0) ? (x - 1) / 2 : x / 2;
}

static inline float FLOORDIV2(float x)
{
    return std::floor((x + 0.5f) / 2.0f);
}

static inline double FLOORDIV2(double x)
{
    return std::floor((x + 0.5) / 2.0);
}

#ifndef __WII__
static void minport_RenderBoxUnfilled(int x1, int y1, int x2, int y2, XTColor color)
{
    minport_RenderBoxFilled(x1, y1, x1 + 1, y2, color);
    minport_RenderBoxFilled(x2 - 1, y1, x2, y2, color);
    minport_RenderBoxFilled(x1, y1, x2, y1 + 1, color);
    minport_RenderBoxFilled(x1, y2 - 1, x2, y2, color);
}
#endif

void renderRect(int x, int y, int w, int h, XTColor color, bool filled)
{
    int x_div = FLOORDIV2(x);
    int w_div = FLOORDIV2(x + w) - x_div;

    int y_div = FLOORDIV2(y);
    int h_div = FLOORDIV2(y + h) - y_div;

    if(w_div <= 0 || h_div <= 0)
        return;

    if(filled)
        minport_RenderBoxFilled(x_div, y_div, x_div + w_div, y_div + h_div, color);
    else
        minport_RenderBoxUnfilled(x_div, y_div, x_div + w_div, y_div + h_div, color);
}

void renderRectBR(int _left, int _top, int _right, int _bottom, XTColor color)
{
    renderRect(_left, _top, _right-_left, _bottom-_top, color, true);
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
                      XTColor color)
{
    if(radius <= 0)
        return; // Nothing to draw

    int line_size = 4;
    int dy = line_size;

    do
    {
        int dx = num_t::floor(num_t::sqrt((2 * radius * dy) - (dy * dy)));

        renderRectBR(cx - radius, cy + dy - radius - line_size, cx - dx, cy + dy - radius + line_size,
            color);

        renderRectBR(cx + dx, cy + dy - radius - line_size, cx + radius, cy + dy - radius + line_size,
            color);

        if(dy < radius) // Don't cross lines
        {
            renderRectBR(cx - radius, cy - dy + radius - line_size, cx - dx, cy - dy + radius + line_size,
                color);

            renderRectBR(cx + dx, cy - dy + radius - line_size, cx + radius, cy - dy + radius + line_size,
                color);
        }

        dy += line_size * 2;
    } while(dy + line_size <= radius);
}

// texture chain methods

// increment the frame counter, and unload all textures not rendered since g_always_unload_after
static void minport_initFrame()
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
static void minport_unlinkTexture(StdPicture* tx)
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
static void minport_freeTextureMemory()
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

#ifdef __16M__
    pLogDebug("Unloaded %d stale textures at free texture memory request", num_unloaded);
#else
    if(g_config.log_level != PGE_LogLevel::Debug)
        return;

    int num_loaded = 0;

    for(StdPicture* p = g_render_chain_tail; p != nullptr; p = p->d.next_texture)
        num_loaded++;

    pLogDebug("Unloaded %d stale textures at free texture memory request (%d still loaded)", num_unloaded, num_loaded);
#endif
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

static inline void minport_RenderTexturePrivate_2(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                             StdPicture &tx,
                             int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                             int16_t rotateAngle, Point_t *center, unsigned int flip,
                             XTColor color)
{
    if(wDst <= 0 || hDst <= 0)
        return;

    // don't go past texture bounds!
    if(xSrc + wSrc > tx.w / 2)
    {
        if(xSrc >= tx.w / 2)
            return;

        wDst = int32_t(wDst) * (tx.w / 2 - xSrc) / wSrc;
        wSrc = tx.w / 2 - xSrc;
    }

    if(ySrc + hSrc > tx.h / 2)
    {
        if(ySrc >= tx.h / 2)
            return;

        hDst = int32_t(hDst) * (tx.h / 2 - ySrc) / hSrc;
        hSrc = tx.h / 2 - ySrc;
    }

    minport_RenderTexturePrivate(xDst, yDst, wDst, hDst,
                             tx,
                             xSrc, ySrc, wSrc, hSrc,
                             rotateAngle, center, flip,
                             color);

    minport_usedTexture(tx);
}

static inline void minport_RenderTexturePrivate_Basic_2(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                             StdPicture &tx,
                             int16_t xSrc, int16_t ySrc,
                             XTColor color)
{
    if(wDst <= 0 || hDst <= 0)
        return;

    // don't go past texture bounds!
    if(xSrc + wDst > tx.w / 2)
    {
        if(xSrc >= tx.w / 2)
            return;

        wDst = tx.w / 2 - xSrc;
    }

    if(ySrc + hDst > tx.h / 2)
    {
        if(ySrc >= tx.h / 2)
            return;

        hDst = tx.h / 2 - ySrc;
    }

    minport_RenderTexturePrivate_Basic(xDst, yDst, wDst, hDst,
                             tx,
                             xSrc, ySrc,
                             color);

    minport_usedTexture(tx);
}

static void minport_usedTexture(StdPicture &tx)
{
    if(tx.d.hasTexture() && tx.l.lazyLoaded && &tx != g_render_chain_head)
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

void renderTextureScale(int xDst, int yDst, int wDst, int hDst,
                            StdPicture &tx,
                            XTColor color)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);

    minport_RenderTexturePrivate_2(
        div_x, div_y, FLOORDIV2(xDst + wDst) - div_x, FLOORDIV2(yDst + hDst) - div_y,
        tx,
        0, 0, tx.w / 2, tx.h / 2,
        0, nullptr, X_FLIP_NONE,
        color);
}

void renderTextureBasic(int xDst, int yDst, int wDst, int hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            XTColor color)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);
    auto div_w = FLOORDIV2(xDst + wDst) - div_x;
    auto div_h = FLOORDIV2(yDst + hDst) - div_y;

    minport_RenderTexturePrivate_Basic_2(
        div_x, div_y, div_w, div_h,
        tx,
        FLOORDIV2(xSrc), FLOORDIV2(ySrc),
        color);
}

void renderTextureBasic(int xDst, int yDst, StdPicture &tx,
                   XTColor color)
{
    int w = tx.w / 2;
    int h = tx.h / 2;

    minport_RenderTexturePrivate_Basic_2(
        FLOORDIV2(xDst), FLOORDIV2(yDst), w, h,
        tx,
        0, 0,
        color);
}

void renderTextureFL(int xDst, int yDst, int wDst, int hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          int16_t rotateAngle, Point_t *center, unsigned int flip,
                          XTColor color)
{
    auto div_x = FLOORDIV2(xDst), div_y = FLOORDIV2(yDst);
    auto div_w = FLOORDIV2(xDst + wDst) - div_x;
    auto div_h = FLOORDIV2(yDst + hDst) - div_y;

    minport_RenderTexturePrivate_2(
        div_x, div_y, div_w, div_h,
        tx,
        FLOORDIV2(xSrc), FLOORDIV2(ySrc), div_w, div_h,
        rotateAngle, center, flip,
        color);
}

void renderTextureScaleEx(int xDst, int yDst, int wDst, int hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          int wSrc, int hSrc,
                          int16_t rotateAngle, Point_t *center, unsigned int flip,
                          XTColor color)
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
        color);
}

#ifndef __16M__
void renderSizableBlock(int bLeftOnscreen, int bTopOnscreen, int wDst, int hDst, StdPicture &tx)
{
    int bRightOnscreen = bLeftOnscreen + wDst;
    if(bRightOnscreen < 0)
        return;

    int bBottomOnscreen = bTopOnscreen + hDst;
    if(bBottomOnscreen < 0)
        return;

    int left_sx = 0;
    if(bLeftOnscreen <= -32)
    {
        left_sx = 32;
        bLeftOnscreen = bLeftOnscreen % 32;

        // go straight to right if less than 33 pixels in total
        if(bRightOnscreen - bLeftOnscreen < 33)
            left_sx = 64;
    }

    int top_sy = 0;
    if(bTopOnscreen <= -32)
    {
        top_sy = 32;
        bTopOnscreen = bTopOnscreen % 32;

        // go straight to bottom if less than 33 pixels in total
        if(bBottomOnscreen - bTopOnscreen < 33)
            top_sy = 64;
    }

    // location of second-to-last row/column in screen coordinates
    int colSemiLast = bRightOnscreen - 64;
    int rowSemiLast = bBottomOnscreen - 64;

    if(bRightOnscreen > g_viewport_w * 2)
        bRightOnscreen = g_viewport_w * 2;

    if(bBottomOnscreen > g_viewport_h * 2)
        bBottomOnscreen = g_viewport_h * 2;

    // first row source
    int src_y = top_sy;

    for(int dst_y = bTopOnscreen; dst_y < bBottomOnscreen; dst_y += 32)
    {
        // first col source
        int src_x = left_sx;

        for(int dst_x = bLeftOnscreen; dst_x < bRightOnscreen; dst_x += 32)
        {
            renderTextureBasic(dst_x, dst_y, 32, 32, tx, src_x, src_y);

            // next col source
            if(dst_x >= colSemiLast)
                src_x = 64;
            else
                src_x = 32;
        }

        // next row source
        if(dst_y >= rowSemiLast)
            src_y = 64;
        else
            src_y = 32;
    }
}
#endif


size_t lazyLoadedBytes()
{
    return 0;
}

void lazyLoadedBytesReset()
{}

} // namespace XRender
