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
#ifndef RENDER_MINPORT_SHARED_H
#define RENDER_MINPORT_SHARED_H

struct StdPicture;

namespace XRender
{

extern int g_viewport_x;
extern int g_viewport_y;
extern int g_viewport_w;
extern int g_viewport_h;
extern int g_viewport_offset_x;
extern int g_viewport_offset_y;
extern bool g_viewport_offset_ignore;

extern int g_screen_phys_x;
extern int g_screen_phys_y;
extern int g_screen_phys_w;
extern int g_screen_phys_h;

// the most recently rendered texture
extern StdPicture* g_render_chain_head;

// the least recently rendered texture
extern StdPicture* g_render_chain_tail;

// the current render frame
extern uint32_t g_current_frame;

// never unload a texture that was rendered less than this many frames ago
constexpr uint32_t g_never_unload_before = 2;

// always unload a texture that was rendered more than this many frames ago
constexpr uint32_t g_always_unload_after = 18000;

// do not need to be implemented per-platform

#if 0
void updateViewport();
void resetViewport();
void setViewport(int x, int y, int w, int h);
void offsetViewport(int x, int y);
void offsetViewportIgnore(bool en);
void setTransparentColor(StdPicture &target, uint32_t rgb);
void renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled);
void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);
void renderCircle(int cx, int cy,
                  int radius,
                  float red , float green, float blue, float alpha,
                  bool filled);
void renderCircleHole(int cx, int cy,
                      int radius,
                      float red, float green, float blue, float alpha);
void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc, int wSrc, int hSrc,
                            float red, float green, float blue, float alpha);
void renderTexture(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            float red, float green, float blue, float alpha);
void renderTexture(float xDst, float yDst, StdPicture &tx,
                   float red, float green, float blue, float alpha);
void renderTexture(int xDst, int yDst, StdPicture &tx, float red, float green, float blue, float alpha);
void renderTextureScale(int xDst, int yDst, int wDst, int hDst, StdPicture &tx, float red, float green, float blue, float alpha);
void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha);
void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          int wSrc, int hSrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha);
size_t lazyLoadedBytes();
void lazyLoadedBytesReset();
#endif

// new functions that platforms should use when deleting textures or trying to free texture memory

// increment the frame counter, and unload all textures not rendered since g_always_unload_after
void minport_initFrame();

// removes a texture from the render chain
void minport_unlinkTexture(StdPicture* tx);

// unload all textures not rendered since g_never_unload_before
void minport_freeTextureMemory();


// new functions that should be implemented per-platform

// for widescreen stretch mode on Wii
void minport_TransformPhysCoords();
// update render viewports, etc
void minport_ApplyPhysCoords();
void minport_ApplyViewport();

void minport_RenderBoxFilled(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
void minport_RenderBoxUnfilled(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);

void minport_RenderTexturePrivate(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                             StdPicture &tx,
                             int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                             float rotateAngle, FPoint_t *center, unsigned int flip,
                             float red, float green, float blue, float alpha);

} // namespace XRender;

#endif // #ifndef RENDER_MINPORT_SHARED_H
