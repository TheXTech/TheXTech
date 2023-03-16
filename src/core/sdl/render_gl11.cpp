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

#ifndef THEXTECH_GL_ES_ONLY
#include <SDL2/SDL_opengl.h>
#endif

#include <SDL2/SDL_opengles.h>

#include <FreeImageLite.h>
#include <Graphics/graphics_funcs.h>

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "render_gl11.h"
#include "video.h"
#include "core/window.h"
#include "graphics.h"


#include "sdl_proxy/sdl_stdinc.h"
#include <fmt_format_ne.h>

#include "controls.h"
#include "sound.h"

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#ifdef THEXTECH_GL_ES_ONLY
static constexpr bool s_gles_mode = true;
#else
static bool s_gles_mode = false;
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
    return m_gContext;
}

bool RenderGL11::initRender(const CmdLineSetup_t &setup, SDL_Window *window)
{
    pLogDebug("Init renderer settings...");

    if(!AbstractRender_t::init())
        return false;

    m_window = window;

    Uint32 renderFlags = 0;

#ifndef THEXTECH_GL_ES_ONLY
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    s_gles_mode = false;
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    m_gContext = SDL_GL_CreateContext(m_window);

#ifndef THEXTECH_GL_ES_ONLY
    if(!m_gContext)
    {
        pLogWarning("Unable to create GL compatibility 1.1 profile, attempting GLES 1.1.");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        m_gContext = SDL_GL_CreateContext(m_window);
        s_gles_mode = true;
    }
#endif

    if(!m_gContext)
    {
        pLogCritical("Unable to create renderer!");
        return false;
    }

    int mask, maj_ver, min_ver;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &mask);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj_ver);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min_ver);

    pLogDebug("Initialized OpenGL %d.%d with profile %d", maj_ver, min_ver, mask);
    pLogDebug("OpenGL driver version: %s", glGetString(GL_VERSION));
    pLogDebug("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    pLogDebug("OpenGL renderer: %s", glGetString(GL_RENDERER));

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

    // Clean-up from a possible start-up junk
    clearBuffer();

    updateViewport();

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

void RenderGL11::togglehud()
{
    if(m_draw_mask_mode == 0)
    {
        m_draw_mask_mode = 1;
        PlaySoundMenu(SFX_PlayerShrink);
    }
    // else if(m_draw_mask_mode == 4)
    // {
    //     m_draw_mask_mode++;
    //     PlaySoundMenu(SFX_PlayerDied2);
    // }
    else if(m_draw_mask_mode != 4)
    {
        if(m_draw_mask_mode == 1)
            m_draw_mask_mode++;

        m_draw_mask_mode++;
        PlaySoundMenu(SFX_Raccoon);
    }
    else
    {
        m_draw_mask_mode = 0;
        PlaySoundMenu(SFX_PlayerGrow);
    }
}

void RenderGL11::repaint()
{
#ifdef USE_RENDER_BLOCKING
    if(m_blockRender)
        return;
#endif

    if(m_draw_mask_mode == 0)
        SuperPrintScreenCenter("Logic Op Render (X64)", 3, 0);
    else if(m_draw_mask_mode == 1)
        SuperPrintScreenCenter("Min/Max Render", 3, 0);
    else if(m_draw_mask_mode == 2)
        SuperPrintScreenCenter("Mul/Max Render (X2)", 3, 0);
    else if(m_draw_mask_mode == 3)
        SuperPrintScreenCenter("Min/Add Render (new)", 3, 0);
    else if(m_draw_mask_mode == 4)
        SuperPrintScreenCenter("Min/SoftAdd Render (new)", 3, 0);
    else
        SuperPrintScreenCenter("Mul/Add Render", 3, 0);

#ifdef USE_SCREENSHOTS_AND_RECS
    if(TakeScreen)
    {
        makeShot();
        PlaySoundMenu(SFX_GotItem);
        TakeScreen = false;
    }
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
    clearBuffer();
}

void RenderGL11::applyViewport()
{
    if(m_recentTargetScreen)
        return;

    int off_x = m_viewport_offset_ignore ? 0 : m_viewport_offset_x;
    int off_y = m_viewport_offset_ignore ? 0 : m_viewport_offset_y;

    // fix offscreen coordinates
    int viewport_x = m_viewport_x;
    int viewport_y = m_viewport_y;
    int viewport_w = m_viewport_w;
    int viewport_h = m_viewport_h;

    if(viewport_x < 0)
    {
        off_x += viewport_x;
        viewport_w += viewport_x;
        viewport_x = 0;
    }

    if(viewport_y < 0)
    {
        off_y += viewport_y;
        viewport_h += viewport_y;
        viewport_y = 0;
    }

    if(viewport_y + viewport_h > ScreenH)
        viewport_h = ScreenH - viewport_y;

    if(viewport_x + viewport_w > ScreenW)
        viewport_w = ScreenW - viewport_x;

    int phys_offset_x = viewport_x * m_phys_w / ScreenW;
    int phys_width = viewport_w * m_phys_w / ScreenW;

    int phys_offset_y = viewport_y * m_phys_h / ScreenH;
    int phys_height = viewport_h * m_phys_h / ScreenH;

    glViewport(m_phys_x + phys_offset_x,
            m_phys_y + m_phys_h - phys_height - phys_offset_y, // relies on fact that m_phys_y is a symmetric border
            phys_width,
            phys_height);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    // pLogDebug("Setting projection to %d %d %d %d", off_x, m_viewport_w + off_x, m_viewport_h + off_y, off_y);
#ifndef THEXTECH_GL_ES_ONLY
    if(s_gles_mode)
        glOrthof( off_x, viewport_w + off_x, viewport_h + off_y, off_y, -1, 1);
    else
        glOrtho( off_x, viewport_w + off_x, viewport_h + off_y, off_y, -1, 1);
#else
    glOrthof( off_x, viewport_w + off_x, viewport_h + off_y, off_y, -1, 1);
#endif
}

void RenderGL11::updateViewport()
{
    int hardware_w, hardware_h;
    getRenderSize(&hardware_w, &hardware_h);

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
}

void RenderGL11::resetViewport()
{
    updateViewport();
}

void RenderGL11::setViewport(int x, int y, int w, int h)
{
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

void RenderGL11::getRenderSize(int *w, int *h)
{
    SDL_GL_GetDrawableSize(m_window, w, h);
}

void RenderGL11::setTargetTexture()
{
    if(!m_recentTargetScreen)
        return;

    m_recentTargetScreen = false;
    applyViewport();
}

void RenderGL11::setTargetScreen()
{
    if(m_recentTargetScreen)
        return;

    m_recentTargetScreen = true;

    int hardware_w, hardware_h;
    getRenderSize(&hardware_w, &hardware_h);

    glViewport(0,
            0,
            hardware_w,
            hardware_h);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

#ifndef THEXTECH_GL_ES_ONLY
    if(s_gles_mode)
        glOrthof(0, hardware_w, hardware_h, 0, -1, 1);
    else
        glOrtho(0, hardware_w, hardware_h, 0, -1, 1);
#else
    glOrthof(0, hardware_w, hardware_h, 0, -1, 1);
#endif
}

void RenderGL11::prepareDrawMask()
{
    if(m_draw_mask_mode == 0)
    {
        // bitwise and
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_AND);
    }
    else if(m_draw_mask_mode == 1 || m_draw_mask_mode == 3 || m_draw_mask_mode == 4)
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
        glDisable(GL_COLOR_LOGIC_OP);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_OR);
    }
    else if(m_draw_mask_mode == 1)
    {
        // max
        glBlendEquation(GL_MAX);
    }
    else if(m_draw_mask_mode == 2)
    {
        // normal
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // max
        glBlendEquation(GL_MAX);
    }
    else if(m_draw_mask_mode == 3)
    {
        // unset min
        glBlendEquation(GL_FUNC_ADD);
        // add
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    else if(m_draw_mask_mode == 4)
    {
        // unset min
        glBlendEquation(GL_FUNC_ADD);
        // softadd
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
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
    else if(m_draw_mask_mode == 2)
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
    // clear pre-existing errors
    GLuint err;
    while((err = glGetError()) != 0)
        pLogWarning("Render GL: got GL error code %d prior to texture load", (int)err);

    // SDL_Surface *surface;
    // SDL_Texture *texture = nullptr;

    target.d.nOfColors = GL_RGBA;
    target.d.format = GL_RGBA;

    GLuint tex_id;

    int pad_w = s_nextPowerOfTwo(SDL_max(least_width, width));
    int pad_h = s_nextPowerOfTwo(SDL_max(least_height, height));

    uint8_t* padded_pixels = nullptr;
    uint8_t* use_pixels = nullptr;

    pitch /= 4;

    // can't do because of pixel substitution
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

    // check for errors as a result of texture load
    err = glGetError();
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

    if(is_mask)
        target.d.mask_texture_id = tex_id;
    else
    {
        target.d.texture_id = tex_id;
        target.inited = true;
    }
    m_textureBank.insert(&target);

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

    auto corpseIt = m_textureBank.find(&tx);
    if(corpseIt != m_textureBank.end())
    {
        m_textureBank.erase(corpseIt);
        return;
    }

    if(tx.d.texture_id)
        glDeleteTextures(1, &tx.d.texture_id);

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
    for(StdPicture *tx : m_textureBank)
        deleteTexture(*tx, false);

    m_textureBank.clear();
}

void RenderGL11::clearBuffer()
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    glBindTexture(GL_TEXTURE_2D, 0);
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

    float x1 = x;
    float x2 = x + w;
    float y1 = y;
    float y2 = y + h;

    glColor4f(red, green, blue, alpha);

    glBindTexture(GL_TEXTURE_2D, 0);

    if(filled)
    {
        const float world_coords[] = {x1, y1,
                                      x1, y2,
                                      x2, y1,
                                      x2, y2};
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else
    {
        // enforce 1px wide
        const float world_coords[] =
            {x1, y1,  x1, y2,  x1 + 1, y2,  x1 + 1, y1,
             x2 - 1, y1,  x2 - 1, y2,  x2, y2,  x2, y1,
             x1, y2 - 1,  x1, y2,  x2, y2,  x2, y2 - 1,
             x1, y1,  x1, y1 + 1,  x2, y1 + 1,  x2, y1};
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 16);
    }
}

void RenderGL11::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    float x1 = _left;
    float x2 = _right;
    float y1 = _top;
    float y2 = _bottom;

    const float world_coords[] = {x1, y1,
        x1, y2,
        x2, y1,
        x2, y2};

    glColor4f(red, green, blue, alpha);

    glBindTexture(GL_TEXTURE_2D, 0);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}

#define PI 3.1415926535897932384626433832795

void RenderGL11::renderCircle(int cx, int cy, int radius, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    UNUSED(filled);

    if(radius <= 0)
        return; // Nothing to draw

    double line_size = 2;
    double dy = line_size;

    const int verts = 32;

    float world_coords[verts * 2 + 4];
    world_coords[0] = cx;
    world_coords[1] = cy;

    for(int i = 0; i < verts + 1; i++)
    {
        float theta = i * (float)PI * 2 / verts;
        float x = cx + radius * cosf(theta);
        float y = cy + radius * sinf(theta);

        world_coords[2 * i + 2] = x;
        world_coords[2 * i + 1 + 2] = y;
    }

    glColor4f(red, green, blue, alpha);

    glBindTexture(GL_TEXTURE_2D, 0);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glDrawArrays(GL_TRIANGLE_FAN, 0, verts + 2);
}

void RenderGL11::renderCircleHole(int cx, int cy, int radius, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(radius <= 0)
        return; // Nothing to draw

    double line_size = 2;
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

    float x1 = xDst;
    float x2 = xDst + wDst;
    float y1 = yDst;
    float y2 = yDst + hDst;

    float u1 = tx.l.w_scale * xSrc;
    float u2 = tx.l.w_scale * (xSrc + wSrc);
    float v1 = tx.l.h_scale * ySrc;
    float v2 = tx.l.h_scale * (ySrc + hSrc);

    if(flip & X_FLIP_HORIZONTAL)
        std::swap(u1, u2);

    if(flip & X_FLIP_VERTICAL)
        std::swap(v1, v2);

    const float world_coords[] = {x1, y1,
        x1, y2,
        x2, y1,
        x2, y2};

    const float tex_coords[] = {u1, v1,
        u1, v2,
        u2, v1,
        u2, v2};

    glColor4f(red, green, blue, alpha);

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
}

void RenderGL11::renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                                     StdPicture &tx,
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

    float x1 = xDst;
    float x2 = xDst + wDst;
    float y1 = yDst;
    float y2 = yDst + hDst;

    float u1 = tx.l.w_scale * 0;
    float u2 = tx.l.w_scale * (tx.w);
    float v1 = tx.l.h_scale * 0;
    float v2 = tx.l.h_scale * (tx.h);

    const float world_coords[] = {x1, y1,
        x1, y2,
        x2, y1,
        x2, y2};

    const float tex_coords[] = {u1, v1,
        u1, v2,
        u2, v1,
        u2, v2};

    glColor4f(red, green, blue, alpha);

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
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
        x2, y1,
        x2, y2};

    const float tex_coords[] = {u1, v1,
        u1, v2,
        u2, v1,
        u2, v2};

    glColor4f(red, green, blue, alpha);

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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

    float cx = center ? center->x : wDst / 2.0f;
    float cy = center ? center->y : hDst / 2.0f;

    glPushMatrix();

    glTranslatef(xDst + cx, yDst + cy, 0);
    glRotatef(rotateAngle, 0, 0, 1);

    float x1 = -cx;
    float x2 = -cx + wDst;
    float y1 = -cy;
    float y2 = -cy + hDst;

    float u1 = tx.l.w_scale * xSrc;
    float u2 = tx.l.w_scale * (xSrc + wDst);
    float v1 = tx.l.h_scale * ySrc;
    float v2 = tx.l.h_scale * (ySrc + hDst);

    if(flip & X_FLIP_HORIZONTAL)
        std::swap(u1, u2);

    if(flip & X_FLIP_VERTICAL)
        std::swap(v1, v2);

    const float world_coords[] = {x1, y1,
        x1, y2,
        x2, y1,
        x2, y2};

    const float tex_coords[] = {u1, v1,
        u1, v2,
        u2, v1,
        u2, v2};

    glColor4f(red, green, blue, alpha);

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }

    glPopMatrix();
}

void RenderGL11::renderTexture(float xDst, float yDst,
                                StdPicture &tx,
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

    float x1 = xDst;
    float x2 = xDst + tx.w;
    float y1 = yDst;
    float y2 = yDst + tx.h;

    float u1 = tx.l.w_scale * 0;
    float u2 = tx.l.w_scale * (tx.w);
    float v1 = tx.l.h_scale * 0;
    float v2 = tx.l.h_scale * (tx.h);

    const float world_coords[] = {x1, y1,
        x1, y2,
        x2, y1,
        x2, y2};

    const float tex_coords[] = {u1, v1,
        u1, v2,
        u2, v1,
        u2, v2};

    glColor4f(red, green, blue, alpha);

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glVertexPointer(2, GL_FLOAT, 0, world_coords);
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glVertexPointer(2, GL_FLOAT, 0, world_coords);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
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
