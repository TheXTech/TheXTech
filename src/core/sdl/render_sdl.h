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
#ifndef RENDERSDL_T_H
#define RENDERSDL_T_H

#include <set>

#include "../base/render_base.h"
#include "cmd_line_setup.h"


struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

class RenderSDL final : public AbstractRender_t
{
    SDL_Window   *m_window = nullptr;

    SDL_Renderer *m_gRenderer = nullptr;
    SDL_Texture  *m_tBuffer = nullptr;
    SDL_Texture  *m_recentTarget = nullptr;
    std::set<SDL_Texture *> m_textureBank;

    // Scale of virtual and window resolutuins
    float m_scale_x = 1.f;
    float m_scale_y = 1.f;
    // Side offsets to keep ratio
    float m_offset_x = 0.f;
    float m_offset_y = 0.f;
    // Offset to shake screen
    int m_viewport_offset_x = 0;
    int m_viewport_offset_y = 0;
    // Keep zero viewport offset while this flag is on
    bool m_viewport_offset_ignore = false;
    // Carried set value for viewport offset (used to preserve values while ignore option is on)
    int m_viewport_offset_x_cur = 0;
    int m_viewport_offset_y_cur = 0;

    //Need to calculate relative viewport position when screen was scaled
    float m_viewport_scale_x = 1.0f;
    float m_viewport_scale_y = 1.0f;

    int m_viewport_x = 0;
    int m_viewport_y = 0;
    int m_viewport_w = 0;
    int m_viewport_h = 0;

public:
    RenderSDL();
    ~RenderSDL() override;


    unsigned int SDL_InitFlags() override;

    bool isWorking() override;

    bool initRender(const CmdLineSetup_t &setup, SDL_Window *window);

    /*!
     * \brief Close the renderer
     */
    void close() override;

    /*!
     * \brief Call the repaint
     */
    void repaint() override;

    /*!
     * \brief Update viewport (mainly after screen resize)
     */
    void updateViewport() override;

    /*!
     * \brief Reset viewport into default state
     */
    void resetViewport() override;

    /*!
     * \brief Set the viewport area
     * \param x X position
     * \param y Y position
     * \param w Viewport Width
     * \param h Viewport Height
     */
    void setViewport(int x, int y, int w, int h) override;

    /*!
     * \brief Set the render offset
     * \param x X offset
     * \param y Y offset
     *
     * All drawing objects will be drawn with a small offset
     */
    void offsetViewport(int x, int y) override; // for screen-shaking

    /*!
     * \brief Set temporary ignore of render offset
     * \param en Enable viewport offset ignore
     *
     * Use this to draw certain objects with ignorign of the GFX offset
     */
    void offsetViewportIgnore(bool en) override;

    void mapToScreen(int x, int y, int *dx, int *dy) override;

    void mapFromScreen(int x, int y, int *dx, int *dy) override;

    /*!
     * \brief Set render target into the virtual in-game screen (use to render in-game world)
     */
    void setTargetTexture() override;

    /*!
     * \brief Set render target into the real window or screen (use to render on-screen buttons and other meta-info)
     */
    void setTargetScreen() override;


    void loadTexture(StdPicture &target,
                     uint32_t width,
                     uint32_t height,
                     uint8_t *RGBApixels,
                     uint32_t pitch) override;

    void deleteTexture(StdPicture &tx, bool lazyUnload = false) override;
    void clearAllTextures() override;

    void clearBuffer() override;



    // Draw primitives

    void renderRect(int x, int y, int w, int h,
                    float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                    bool filled = true) override;

    void renderRectBR(int _left, int _top, int _right,
                      int _bottom, float red, float green, float blue, float alpha) override;

    void renderCircle(int cx, int cy,
                      int radius,
                      float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                      bool filled = true) override;

    void renderCircleHole(int cx, int cy,
                          int radius,
                          float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;




    // Draw texture

    void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                              StdPicture &tx,
                              int xSrc, int ySrc,
                              int wSrc, int hSrc,
                              double rotateAngle =.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                              float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTexture(double xDst, double yDst, double wDst, double hDst,
                       StdPicture &tx,
                       int xSrc, int ySrc,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                         StdPicture &tx,
                         int xSrc, int ySrc,
                         double rotateAngle =.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                         float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTexture(float xDst, float yDst, StdPicture &tx,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;




    // Retrieve raw pixel data

    void getScreenPixels(int x, int y, int w, int h, unsigned char *pixels) override;

    void getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels) override;

    int  getPixelDataSize(const StdPicture &tx) override;

    void getPixelData(const StdPicture &tx, unsigned char *pixelData) override;

};


#endif // RENDERSDL_T_H
