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

#include <SDL2/SDL_version.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_opengl.h>
// #include <SDL2/SDL_opengles2.h>

#include <FreeImageLite.h>
#include <Graphics/graphics_funcs.h>

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "render_gl11.h"
#include "video.h"
#include "core/window.h"

#include "sdl_proxy/sdl_stdinc.h"
#include <fmt_format_ne.h>

#include "controls.h"

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif




RenderGL11::RenderGL11() :
    AbstractRender_t()
{}

RenderGL11::~RenderGL11()
{
    if(m_window)
        RenderGL11::close();
}

unsigned int RenderGL11::SDL_InitFlags()
{
    return 0;
}

bool RenderGL11::isWorking()
{
    return m_gContext && (m_tBufferDisabled);
}

bool RenderGL11::initRender(const CmdLineSetup_t &setup, SDL_Window *window)
{
    pLogDebug("Init renderer settings...");

    if(!AbstractRender_t::init())
        return false;

    m_window = window;

    Uint32 renderFlags = 0;

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_gContext = SDL_GL_CreateContext(m_window);

    if(!m_gContext)
    {
        pLogCritical("Unable to create renderer!");
        return false;
    }

    SDL_GL_SetSwapInterval(0);

    // SDL_RendererInfo ri;
    // SDL_GetRendererInfo(m_gRenderer, &ri);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint maxTextureSize = 256;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    m_maxTextureWidth = maxTextureSize;
    m_maxTextureHeight = maxTextureSize;

    // m_tBuffer = nullptr; //SDL_CreateTexture(m_gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ScaleWidth, ScaleHeight);
    // if(!m_tBuffer)
    {
        m_tBufferDisabled = true;
    }

    // Clean-up from a possible start-up junk
    clearBuffer();

    setTargetTexture();
    // SDL_SetRenderDrawBlendMode(m_gRenderer, SDL_BLENDMODE_BLEND);

    updateViewport();

    // Clean-up the texture buffer from the same start-up junk
    clearBuffer();

    setTargetScreen();

    repaint();

    return true;
}

void RenderGL11::close()
{
    RenderGL11::clearAllTextures();
    AbstractRender_t::close();

    // if(m_tBuffer)
    //     SDL_DestroyTexture(m_tBuffer);
    // m_tBuffer = nullptr;

    if(m_gContext)
        SDL_GL_DeleteContext(m_gContext);
    m_gContext = nullptr;
}

void RenderGL11::repaint()
{
#ifdef USE_RENDER_BLOCKING
    if(m_blockRender)
        return;
#endif

    setTargetScreen();

#ifdef USE_SCREENSHOTS_AND_RECS
    processRecorder();
#endif

#ifdef USE_DRAW_BATTERY_STATUS
    drawBatteryStatus();
#endif

    Controls::RenderTouchControls();

    glFlush();
    SDL_GL_SwapWindow(m_window);
}

void RenderGL11::applyViewport()
{
    int phys_offset_x = m_viewport_x * m_phys_w / ScreenW;
    int phys_width = m_viewport_w * m_phys_w / ScreenW;

    int phys_offset_y = m_viewport_y * m_phys_h / ScreenH;
    int phys_height = m_viewport_h * m_phys_h / ScreenH;

    if(m_phys_x + phys_offset_x < 0)
        phys_offset_x = -m_phys_x;
    if(m_phys_y + phys_offset_y < 0)
        phys_offset_y = -m_phys_y;

    if(m_phys_x + phys_offset_x + phys_width >= m_phys_w)
        phys_width = m_phys_w - (m_phys_x + phys_offset_x);
    if(m_phys_y + phys_offset_y + phys_height >= m_phys_h)
        phys_height = m_phys_h - (m_phys_y + phys_offset_y);

    // pLogDebug("Setting viewport to %d %d %d %d", m_phys_x + phys_offset_x,
    //         m_phys_h - (m_phys_y + phys_offset_y + phys_height),
    //         m_phys_x + phys_offset_x + phys_width - 1,
    //         m_phys_h - (m_phys_y + phys_offset_y) - 1);
    glViewport(m_phys_x + phys_offset_x,
            m_phys_h - (m_phys_y + phys_offset_y + phys_height),
            m_phys_x + phys_offset_x + phys_width - 1,
            m_phys_h - (m_phys_y + phys_offset_y) - 1);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    int off_x = m_viewport_offset_ignore ? 0 : m_viewport_offset_x;
    int off_y = m_viewport_offset_ignore ? 0 : m_viewport_offset_y;

    // pLogDebug("Setting projection to %d %d %d %d", off_x, m_viewport_w + off_x, m_viewport_h + off_y, off_y);
    glOrtho( off_x, m_viewport_w + off_x, m_viewport_h + off_y, off_y, -1, 1);
}

void RenderGL11::updateViewport()
{
    int hardware_w, hardware_h;
    XWindow::getWindowSize(&hardware_w, &hardware_h);

    // if(g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR || g_videoSettings.scaleMode == SCALE_DYNAMIC_NEAREST)
    {
        int res_h = hardware_h;
        int res_w = ScreenW * hardware_h / ScreenH;

        if(res_w > hardware_w)
        {
            res_w = hardware_w;
            res_h = ScreenH * res_w / ScreenW;
        }

        m_phys_w = res_w;
        m_phys_h = res_h;
    }

    m_viewport_x = 0;
    m_viewport_y = 0;
    m_viewport_w = ScreenW;
    m_viewport_h = ScreenH;

    m_viewport_offset_x = 0;
    m_viewport_offset_y = 0;
    m_viewport_offset_ignore = false;

    pLogDebug("Phys screen is %d x %d", m_phys_w, m_phys_h);

    m_phys_x = hardware_w / 2 - m_phys_w / 2;
    m_phys_y = hardware_h / 2 - m_phys_h / 2;

    applyViewport();

    // clearBuffer();
}

void RenderGL11::resetViewport()
{
    // FIXME: Clarify the version of SDL2 with the buggy viewport
//#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 22)
    // set to an alt viewport as a workaround for SDL bug (doesn't allow resizing viewport without changing position)
    // SDL_Rect altViewport = {m_viewport_x + 1, m_viewport_y + 1, 1, 1};
    // SDL_RenderSetViewport(m_gRenderer, &altViewport);
//#endif

    updateViewport();
    // SDL_RenderSetViewport(m_gRenderer, nullptr);

}

void RenderGL11::setViewport(int x, int y, int w, int h)
{
    // SDL_Rect topLeftViewport;

    // FIXME: Clarify the version of SDL2 with the buggy viewport
//#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 22)
    // set to an alt viewport as a workaround for SDL bug (doesn't allow resizing viewport without changing position)
    // topLeftViewport = {m_viewport_x + 1, m_viewport_y + 1, 1, 1};
    // SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);
//#endif

    // topLeftViewport = {x, y, w, h};
    // SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);

    m_viewport_x = x;
    m_viewport_y = y;
    m_viewport_w = w;
    m_viewport_h = h;

    applyViewport();
}

void RenderGL11::offsetViewport(int x, int y)
{
    m_viewport_offset_x = x;
    m_viewport_offset_y = y;

    applyViewport();
}

void RenderGL11::offsetViewportIgnore(bool en)
{
    m_viewport_offset_ignore = en;

    applyViewport();
}

void RenderGL11::mapToScreen(int x, int y, int *dx, int *dy)
{
    *dx = static_cast<int>((static_cast<float>(x) - m_phys_x) * ScreenW / m_phys_w);
    *dy = static_cast<int>((static_cast<float>(y) - m_phys_y) * ScreenH / m_phys_h);
}

void RenderGL11::mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    *window_x = (float)scr_x * m_phys_w / ScreenW + m_phys_x;
    *window_y = (float)scr_y * m_phys_h / ScreenH + m_phys_y;
}

void RenderGL11::setTargetTexture()
{
    // if(m_tBufferDisabled || m_recentTarget == m_tBuffer)
        return;
    // SDL_SetRenderTarget(m_gRenderer, m_tBuffer);
    // m_recentTarget = m_tBuffer;
}

void RenderGL11::setTargetScreen()
{
    // if(m_tBufferDisabled || m_recentTarget == nullptr)
        return;
    // SDL_SetRenderTarget(m_gRenderer, nullptr);
    // m_recentTarget = nullptr;
}

void RenderGL11::prepareDrawMask()
{
    if(m_draw_mask_mode == 0)
    {
        // bitwise and
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_AND);
    }
    else if(m_draw_mask_mode == 1)
    {
        // min
        glBlendEquation(GL_MIN);
    }
    else
    {
        // multiply
        glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
        // glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    }
}

void RenderGL11::prepareDrawImage()
{
    if(m_draw_mask_mode == 0)
    {
        // bitwise or
        glLogicOp(GL_OR);
    }
    else if(m_draw_mask_mode == 1)
    {
        // max
        glBlendEquation(GL_MAX);
    }
    else
    {
        // add
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
}

void RenderGL11::leaveMaskContext()
{
    if(m_draw_mask_mode == 0)
    {
        // no bitwise op
        glDisable(GL_COLOR_LOGIC_OP);
    }
    else if(m_draw_mask_mode == 1)
    {
        // normal
        glBlendEquation(GL_FUNC_ADD);
    }
    else
    {
        // normal
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

static int s_nextPowerOfTwo(int val)
{
    int power = 1;
    while (power < val)
        power *= 2;
    return power;
}

void RenderGL11::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch, bool is_mask, uint32_t least_width, uint32_t least_height)
{
    // SDL_Surface *surface;
    // SDL_Texture *texture = nullptr;

    target.d.nOfColors = GL_RGBA;
    target.d.format = GL_BGRA;

    GLuint tex_id;

    int pad_w = s_nextPowerOfTwo(SDL_max(least_width, width));
    int pad_h = s_nextPowerOfTwo(SDL_max(least_height, height));

    uint8_t* padded_pixels = nullptr;
    uint8_t* use_pixels = nullptr;

    pitch /= 4;

    // if(pad_w == pitch && height == pad_h)
    // {
        // use_pixels = RGBApixels;
    // }
    // else
    {
        padded_pixels = (uint8_t*) malloc(pad_w * pad_h * 4);
        if(!padded_pixels)
        {
            pLogWarning("Render GL: Failed to allocate padded texture memory");
            if(!is_mask)
            {
                target.d.clear();
                target.inited = false;
            }
            return;
        }

        if(is_mask)
            SDL_memset(padded_pixels, 255, pad_w * pad_h * 4);
        else
            SDL_memset(padded_pixels, 0, pad_w * pad_h * 4);

        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                padded_pixels[(y * pad_w + x) * 4 + 0] = RGBApixels[(y * pitch + x) * 4 + 2];
                padded_pixels[(y * pad_w + x) * 4 + 1] = RGBApixels[(y * pitch + x) * 4 + 1];
                padded_pixels[(y * pad_w + x) * 4 + 2] = RGBApixels[(y * pitch + x) * 4 + 0];
                padded_pixels[(y * pad_w + x) * 4 + 3] = RGBApixels[(y * pitch + x) * 4 + 3];
            }
        }

        use_pixels = padded_pixels;
    }

    if(!use_pixels && is_mask)
    {
        pLogWarning("Render GL: Couldn't find mask data");
        return;
    }
    else if(!use_pixels)
    {
        pLogWarning("Render GL: Couldn't find texture data");
        target.d.clear();
        target.inited = false;
        return;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGBA,
        pad_w,
        pad_h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        use_pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    if(padded_pixels)
    {
        free(padded_pixels);
        padded_pixels = nullptr;
    }

    use_pixels = nullptr;

    if(!is_mask)
    {
        if(target.l.w_scale == 0)
            target.l.w_scale = 1;
        if(target.l.h_scale == 0)
            target.l.h_scale = 1;

        target.l.w_scale /= pad_w;
        target.l.h_scale /= pad_h;
    }

    GLenum err = glGetError();
    if(err != GL_NO_ERROR && is_mask)
    {
        glDeleteTextures(1, &tex_id);
        pLogWarning("Render GL: Couldn't load mask data (GL error code %d)", (int)err);
        return;
    }
    else if(err != GL_NO_ERROR)
    {
        glDeleteTextures(1, &tex_id);

        pLogWarning("Render GL: Couldn't load texture data (GL error code %d)", (int)err);
        target.d.clear();
        target.inited = false;
        return;
    }

    // if(!texture)
    // {
    //     pLogWarning("Render SDL: Failed to load texture! (%s)", SDL_GetError());
    //     target.d.clear();
    //     target.inited = false;
    //     return;
    // }

    if(is_mask)
        target.d.mask_texture_id = tex_id;
    else
    {
        target.d.texture_id = tex_id;
        target.inited = true;
    }
    // m_textureBank.insert(texture);

#if defined(__APPLE__) && defined(USE_APPLE_X11)
    // SDL_GL_UnbindTexture(texture); // Unbind texture after it got been loaded (otherwise a white screen will happen)
#endif
}

void RenderGL11::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch)
{
    loadTexture(target, width, height, RGBApixels, pitch, false, width, height);
}

void RenderGL11::loadTextureMask(StdPicture &target, uint32_t mask_width, uint32_t mask_height, uint8_t *RGBApixels, uint32_t pitch, uint32_t image_width, uint32_t image_height)
{
    loadTexture(target, mask_width, mask_height, RGBApixels, pitch, true, image_width, image_height);
}

bool RenderGL11::textureMaskSupported()
{
    return true;
}

void RenderGL11::deleteTexture(StdPicture &tx, bool lazyUnload)
{
    if(!tx.inited || !tx.d.texture_id)
    {
        if(!lazyUnload)
            tx.inited = false;
        return;
    }

    // auto corpseIt = m_textureBank.find(tx.d.texture_id);
    // if(corpseIt == m_textureBank.end())
    // {
    //     SDL_DestroyTexture(tx.d.texture_id);
    //     tx.d.texture_id = nullptr;
    //     if(!lazyUnload)
    //         tx.inited = false;
    //     return;
    // }

    // SDL_Texture *corpse = *corpseIt;
    // if(corpse)
    //     SDL_DestroyTexture(corpse);
    // m_textureBank.erase(corpse);

    if(tx.inited)
    {
        GLuint tex_id = tx.d.texture_id;
        glDeleteTextures(1, &tex_id);
    }

    if(tx.d.mask_texture_id)
        glDeleteTextures(1, &tx.d.mask_texture_id);

    tx.d.texture_id = 0;
    tx.d.mask_texture_id = 0;

    if(!lazyUnload)
        tx.resetAll();

    tx.d.format = 0;
    tx.d.nOfColors = 0;

    tx.resetColors();
}

void RenderGL11::clearAllTextures()
{
    // for(SDL_Texture *tx : m_textureBank)
    //     SDL_DestroyTexture(tx);
    m_textureBank.clear();
}

void RenderGL11::clearBuffer()
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // SDL_SetRenderDrawColor(m_gRenderer, 0, 0, 0, 255);
    // SDL_RenderClear(m_gRenderer);
}

void RenderGL11::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    SDL_Rect aRect = {x + m_viewport_offset_x,
                      y + m_viewport_offset_y,
                      w, h};
    // SDL_SetRenderDrawColor(m_gRenderer,
    //                        static_cast<unsigned char>(255.f * red),
    //                        static_cast<unsigned char>(255.f * green),
    //                        static_cast<unsigned char>(255.f * blue),
    //                        static_cast<unsigned char>(255.f * alpha)
    //                       );

    // if(filled)
    //     SDL_RenderFillRect(m_gRenderer, &aRect);
    // else
    //     SDL_RenderDrawRect(m_gRenderer, &aRect);
}

void RenderGL11::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    SDL_Rect aRect = {_left + m_viewport_offset_x,
                      _top + m_viewport_offset_y,
                      _right - _left, _bottom - _top};
//     SDL_SetRenderDrawColor(m_gRenderer,
//                            static_cast<unsigned char>(255.f * red),
//                            static_cast<unsigned char>(255.f * green),
//                            static_cast<unsigned char>(255.f * blue),
//                            static_cast<unsigned char>(255.f * alpha)
//                           );
//     SDL_RenderFillRect(m_gRenderer, &aRect);
}

void RenderGL11::renderCircle(int cx, int cy, int radius, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    UNUSED(filled);

    if(radius <= 0)
        return; // Nothing to draw

    // SDL_SetRenderDrawColor(m_gRenderer,
    //                            static_cast<unsigned char>(255.f * red),
    //                            static_cast<unsigned char>(255.f * green),
    //                            static_cast<unsigned char>(255.f * blue),
    //                            static_cast<unsigned char>(255.f * alpha)
    //                       );

    cx += m_viewport_offset_x;
    cy += m_viewport_offset_y;

    // double dy = 1;
    // do //for(double dy = 1; dy <= radius; dy += 1.0)
    // {
    //     double dx = std::floor(std::sqrt((2.0 * radius * dy) - (dy * dy)));
    //     SDL_RenderDrawLine(m_gRenderer,
    //                        int(cx - dx),
    //                        int(cy + dy - radius),
    //                        int(cx + dx),
    //                        int(cy + dy - radius));

    //     if(dy < radius) // Don't cross lines
    //     {
    //         SDL_RenderDrawLine(m_gRenderer,
    //                            int(cx - dx),
    //                            int(cy - dy + radius),
    //                            int(cx + dx),
    //                            int(cy - dy + radius));
    //     }

    //     dy += 1.0;
    // } while(dy <= radius);
}

void RenderGL11::renderCircleHole(int cx, int cy, int radius, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(radius <= 0)
        return; // Nothing to draw

    // SDL_SetRenderDrawColor(m_gRenderer,
    //                            static_cast<unsigned char>(255.f * red),
    //                            static_cast<unsigned char>(255.f * green),
    //                            static_cast<unsigned char>(255.f * blue),
    //                            static_cast<unsigned char>(255.f * alpha)
    //                       );

    cx += m_viewport_offset_x;
    cy += m_viewport_offset_y;

    // double dy = 1;
    // do //for(double dy = 1; dy <= radius; dy += 1.0)
    // {
    //     double dx = std::floor(std::sqrt((2.0 * radius * dy) - (dy * dy)));

    //     SDL_RenderDrawLine(m_gRenderer,
    //                        int(cx - radius),
    //                        int(cy + dy - radius),
    //                        int(cx - dx),
    //                        int(cy + dy - radius));

    //     SDL_RenderDrawLine(m_gRenderer,
    //                        int(cx + dx),
    //                        int(cy + dy - radius),
    //                        int(cx + radius),
    //                        int(cy + dy - radius));


    //     if(dy < radius) // Don't cross lines
    //     {
    //         SDL_RenderDrawLine(m_gRenderer,
    //                            int(cx - radius),
    //                            int(cy - dy + radius),
    //                            int(cx - dx),
    //                            int(cy - dy + radius));

    //         SDL_RenderDrawLine(m_gRenderer,
    //                            int(cx + dx),
    //                            int(cy - dy + radius),
    //                            int(cx + radius),
    //                            int(cy - dy + radius));
    //     }

    //     dy += 1.0;
    // } while(dy <= radius);
}


void RenderGL11::renderTextureScaleEx(double xDstD, double yDstD, double wDstD, double hDstD,
                                       StdPicture &tx,
                                       int xSrc, int ySrc,
                                       int wSrc, int hSrc,
                                       double rotateAngle, FPoint_t *center, unsigned int flip,
                                       float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    if(!tx.inited)
        return;

    if(!tx.d.texture_id && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.texture_id)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_assert_release(tx.d.texture_id);

    int xDst = Maths::iRound(xDstD);
    int yDst = Maths::iRound(yDstD);
    int wDst = Maths::iRound(wDstD);
    int hDst = Maths::iRound(hDstD);

    // Don't go more than size of texture
    if(xSrc + wSrc > tx.w)
    {
        wSrc = tx.w - xSrc;
        if(wSrc < 0)
            wSrc = 0;
    }
    if(ySrc + hSrc > tx.h)
    {
        hSrc = tx.h - ySrc;
        if(hSrc < 0)
            hSrc = 0;
    }

#ifndef XTECH_SDL_NO_RECTF_SUPPORT
    SDL_FRect destRect = {(float)xDst + m_viewport_offset_x,
                          (float)yDst + m_viewport_offset_y,
                          (float)wDst,
                          (float)hDst};
    SDL_FPoint *centerD = (SDL_FPoint*)center;
#else
    SDL_Rect destRect = {(int)xDst + m_viewport_offset_x,
                         (int)yDst + m_viewport_offset_y,
                         (int)wDst,
                         (int)hDst};
    SDL_Point centerI = {center ? Maths::iRound(center->x) : 0,
                         center ? Maths::iRound(center->y) : 0};
    SDL_Point *centerD = center ? &centerI : nullptr;
#endif

    float u1 = tx.l.w_scale * xSrc;
    float u2 = tx.l.w_scale * (xSrc + wSrc);
    float v1 = tx.l.h_scale * ySrc;
    float v2 = tx.l.h_scale * (ySrc + hSrc);

    // txColorMod(tx.d, red, green, blue, alpha);
    // SDL_RenderCopyExF(m_gRenderer, tx.d.texture_id, &sourceRect, &destRect,
    //                   rotateAngle, centerD, static_cast<SDL_RendererFlip>(flip));

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex2f(destRect.x, destRect.y);
    glTexCoord2f(u1, v2);
    glVertex2f(destRect.x, destRect.y + destRect.h);
    glTexCoord2f(u2, v2);
    glVertex2f(destRect.x + destRect.w, destRect.y + destRect.h);
    glTexCoord2f(u2, v1);
    glVertex2f(destRect.x + destRect.w, destRect.y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderGL11::renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                                     StdPicture &tx,
                                     float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    const unsigned int flip = SDL_FLIP_NONE;

    if(!tx.inited)
        return;

    if(!tx.d.texture_id && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.texture_id)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

#ifndef XTECH_SDL_NO_RECTF_SUPPORT
    SDL_FRect destRect = {Maths::fRound(xDst) + m_viewport_offset_x,
                          Maths::fRound(yDst) + m_viewport_offset_y,
                          (float)wDst,
                          (float)hDst};
#else
    SDL_Rect destRect = {Maths::iRound(xDst) + m_viewport_offset_x,
                         Maths::iRound(yDst) + m_viewport_offset_y,
                         (int)wDst,
                         (int)hDst};
#endif

    float u1 = 0;
    float u2 = tx.l.w_scale * tx.w;
    float v1 = 0;
    float v2 = tx.l.h_scale * tx.h;

    // txColorMod(tx.d, red, green, blue, alpha);
    // SDL_RenderCopyExF(m_gRenderer, tx.d.texture_id, &sourceRect, &destRect,
    //                   0.0, nullptr, static_cast<SDL_RendererFlip>(flip));

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex2f(destRect.x, destRect.y);
    glTexCoord2f(u1, v2);
    glVertex2f(destRect.x, destRect.y + destRect.h);
    glTexCoord2f(u2, v2);
    glVertex2f(destRect.x + destRect.w, destRect.y + destRect.h);
    glTexCoord2f(u2, v1);
    glVertex2f(destRect.x + destRect.w, destRect.y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderGL11::renderTexture(double xDstD, double yDstD, double wDstD, double hDstD,
                                StdPicture &tx,
                                int xSrc, int ySrc,
                                float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(!tx.inited)
        return;

    if(!tx.d.texture_id && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.texture_id)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_assert_release(tx.d.texture_id);

    int xDst = Maths::iRound(xDstD);
    int yDst = Maths::iRound(yDstD);
    int wDst = Maths::iRound(wDstD);
    int hDst = Maths::iRound(hDstD);

    // Don't go more than size of texture
    if(xSrc + wDst > tx.w)
    {
        wDst = tx.w - xSrc;
        if(wDst < 0)
            wDst = 0;
    }

    if(ySrc + hDst > tx.h)
    {
        hDst = tx.h - ySrc;
        if(hDst < 0)
            hDst = 0;
    }

    float x1 = xDst;
    float x2 = xDst + wDst;
    float y1 = yDst;
    float y2 = yDst + hDst;

    float u1 = tx.l.w_scale * xSrc;
    float u2 = tx.l.w_scale * (xSrc + wDst);
    float v1 = tx.l.h_scale * ySrc;
    float v2 = tx.l.h_scale * (ySrc + hDst);

    const float world_coords[] = {x1, y1,
        x1, y2,
        x2, y2,
        x2, y1};

    const float tex_coords[] = {u1, v1,
        u1, v2,
        u2, v2,
        u2, v1};

    glColor4f(red, green, blue, alpha);

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

        glDrawArrays(GL_QUADS, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glDrawArrays(GL_QUADS, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
}

void RenderGL11::renderTextureFL(double xDstD, double yDstD, double wDstD, double hDstD,
                                  StdPicture &tx,
                                  int xSrc, int ySrc,
                                  double rotateAngle, FPoint_t *center, unsigned int flip,
                                  float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    if(!tx.inited)
        return;

    if(!tx.d.texture_id && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.texture_id)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_assert_release(tx.d.texture_id);

    int xDst = Maths::iRound(xDstD);
    int yDst = Maths::iRound(yDstD);
    int wDst = Maths::iRound(wDstD);
    int hDst = Maths::iRound(hDstD);

    // Don't go more than size of texture
    if(xSrc + wDst > tx.w)
    {
        wDst = tx.w - xSrc;
        if(wDst < 0)
            wDst = 0;
    }

    if(ySrc + hDst > tx.h)
    {
        hDst = tx.h - ySrc;
        if(hDst < 0)
            hDst = 0;
    }

#ifndef XTECH_SDL_NO_RECTF_SUPPORT
    SDL_FRect destRect = {(float)xDst + m_viewport_offset_x,
                          (float)yDst + m_viewport_offset_y,
                          (float)wDst,
                          (float)hDst};
    SDL_FPoint *centerD = (SDL_FPoint*)center;
#else
    SDL_Rect destRect = {(int)xDst + m_viewport_offset_x,
                         (int)yDst + m_viewport_offset_y,
                         (int)wDst,
                         (int)hDst};
    SDL_Point centerI = {center ? Maths::iRound(center->x) : 0,
                         center ? Maths::iRound(center->y) : 0};
    SDL_Point *centerD = center ? &centerI : nullptr;
#endif

    float u1 = tx.l.w_scale * xSrc;
    float u2 = tx.l.w_scale * (xSrc + wDst);
    float v1 = tx.l.h_scale * ySrc;
    float v2 = tx.l.h_scale * (ySrc + hDst);

    // txColorMod(tx.d, red, green, blue, alpha);
    // SDL_RenderCopyExF(m_gRenderer, tx.d.texture_id, &sourceRect, &destRect,
    //                   rotateAngle, centerD, static_cast<SDL_RendererFlip>(flip));

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex2f(destRect.x, destRect.y);
    glTexCoord2f(u1, v2);
    glVertex2f(destRect.x, destRect.y + destRect.h);
    glTexCoord2f(u2, v2);
    glVertex2f(destRect.x + destRect.w, destRect.y + destRect.h);
    glTexCoord2f(u2, v1);
    glVertex2f(destRect.x + destRect.w, destRect.y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderGL11::renderTexture(float xDst, float yDst,
                                StdPicture &tx,
                                float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    const unsigned int flip = SDL_FLIP_NONE;

    if(!tx.inited)
        return;

    if(!tx.d.texture_id && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.texture_id)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

#ifndef XTECH_SDL_NO_RECTF_SUPPORT
    SDL_FRect destRect = {Maths::fRound(xDst), Maths::fRound(yDst), (float)tx.w, (float)tx.h};
#else
    SDL_Rect destRect = {Maths::iRound(xDst), Maths::iRound(yDst), tx.w, tx.h};
#endif

    float u1 = 0;
    float u2 = tx.l.w_scale * tx.w;
    float v1 = 0;
    float v2 = tx.l.h_scale * tx.h;

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex2f(destRect.x, destRect.y);
    glTexCoord2f(u1, v2);
    glVertex2f(destRect.x, destRect.y + destRect.h);
    glTexCoord2f(u2, v2);
    glVertex2f(destRect.x + destRect.w, destRect.y + destRect.h);
    glTexCoord2f(u2, v1);
    glVertex2f(destRect.x + destRect.w, destRect.y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);

    // txColorMod(tx.d, red, green, blue, alpha);
    // SDL_RenderCopyExF(m_gRenderer, tx.d.texture_id, &sourceRect, &destRect,
    //                   0.0, nullptr, static_cast<SDL_RendererFlip>(flip));
}

void RenderGL11::getScreenPixels(int x, int y, int w, int h, unsigned char *pixels)
{
    glFlush();

    int phys_x, phys_y;

    mapFromScreen(x, y, &phys_x, &phys_y);

    int phys_w = w * m_phys_w / ScreenW;
    int phys_h = h * m_phys_h / ScreenH;

    // allocate buffer for screen-space pixels
    uint8_t* phys_pixels = (uint8_t*)malloc(phys_w * phys_h * 3);
    if(!phys_pixels)
        return;

    // read screen-space pixels
    glReadPixels(phys_x, phys_y, phys_w, phys_h,
        GL_RGB, GL_UNSIGNED_BYTE, phys_pixels);

    // rescale and move to target
    for(int r = 0; r < h; r++)
    {
        int phys_r_max = phys_h - 1;
        int phys_r_ind = r * m_phys_h / ScreenH;

        // vertical flip from OpenGL to image
        int phys_r = phys_r_max - phys_r_ind;

        for(int c = 0; c < w; c++)
        {
            int phys_c = c * m_phys_w / ScreenW;

            pixels[(r * w + c) * 3 + 0] = phys_pixels[(phys_r * phys_w + phys_c) * 3 + 0];
            pixels[(r * w + c) * 3 + 1] = phys_pixels[(phys_r * phys_w + phys_c) * 3 + 1];
            pixels[(r * w + c) * 3 + 2] = phys_pixels[(phys_r * phys_w + phys_c) * 3 + 2];
        }
    }

    free(phys_pixels);
}

void RenderGL11::getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels)
{
    glFlush();

    int phys_x, phys_y;

    mapFromScreen(x, y, &phys_x, &phys_y);

    int phys_w = w * m_phys_w / ScreenW;
    int phys_h = h * m_phys_h / ScreenH;

    // allocate buffer for screen-space pixels
    uint8_t* phys_pixels = (uint8_t*)malloc(phys_w * phys_h * 4);
    if(!phys_pixels)
        return;

    // read screen-space pixels
    glReadPixels(phys_x, phys_y, phys_w, phys_h,
        GL_RGBA, GL_UNSIGNED_BYTE, phys_pixels);

    // rescale and move to target
    for(int r = 0; r < h; r++)
    {
        int phys_r_max = phys_h - 1;
        int phys_r_ind = r * m_phys_h / ScreenH;

        // vertical flip from OpenGL to image
        int phys_r = phys_r_max - phys_r_ind;

        for(int c = 0; c < w; c++)
        {
            int phys_c = c * m_phys_w / ScreenW;

            ((uint32_t*) pixels)[r * w + c] = ((uint32_t*) phys_pixels)[phys_r * phys_w + phys_c];
        }
    }

    free(phys_pixels);
}

int RenderGL11::getPixelDataSize(const StdPicture &tx)
{
    if(!tx.d.texture_id)
        return 0;
    return (tx.w * tx.h * 4);
}

void RenderGL11::getPixelData(const StdPicture &tx, unsigned char *pixelData)
{
    int pitch, w, h, a;
    void *pixels;

    if(!tx.d.texture_id)
        return;

    // SDL_SetTextureBlendMode(tx.d.texture_id, SDL_BLENDMODE_BLEND);
    // SDL_QueryTexture(tx.d.texture_id, nullptr, &a, &w, &h);
    // SDL_LockTexture(tx.d.texture_id, nullptr, &pixels, &pitch);
    // std::memcpy(pixelData, pixels, static_cast<size_t>(pitch) * h);
    // SDL_UnlockTexture(tx.d.texture_id);
}
