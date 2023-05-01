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


#include "core/opengl/gl_inc.h"

#include "core/opengl/render_gl.h"
#include "core/opengl/gl_program_object.h"

#include <FreeImageLite.h>
#include <Graphics/graphics_funcs.h>

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_assert.h"

#include "core/window.h"

#include "globals.h"
#include "sound.h"

#include "graphics.h"
#include "controls.h"


#ifdef MUTABLE_PARTICLES_DEMO
#include "core/opengl/gl_program_bank.h"
StdPicture* s_sparkle;
#endif


#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#define F_TO_B(r, g, b, a) {static_cast<GLubyte>((r) * 255.0f), static_cast<GLubyte>((g) * 255.0f), static_cast<GLubyte>((b) * 255.0f), static_cast<GLubyte>((a) * 255.0f)}


RenderGL::RenderGL() :
    AbstractRender_t()
{}

RenderGL::~RenderGL()
{
    if(m_window)
        RenderGL::close();
}

unsigned int RenderGL::SDL_InitFlags()
{
    return SDL_WINDOW_OPENGL;
}

bool RenderGL::isWorking()
{
    return m_gContext;
}

RenderGL::VertexList& RenderGL::getOrderedDrawVertexList(RenderGL::DrawContext_t context, int depth)
{
    if(context == m_recent_draw_context)
        return m_ordered_draw_queue[{m_recent_draw_context_depth, context}];

    // optimization for buffer-read shaders: only use a single depth per frame
    bool accel_mask = !m_use_logicop || m_gl_profile == SDL_GL_CONTEXT_PROFILE_ES;
    bool is_mask = context.program == &m_standard_program && context.texture && context.texture->d.mask_texture_id;

    if((context.program && (context.program->get_flags() & GLProgramObject::read_buffer)) || (accel_mask && is_mask))
    {
        int& saved_context_depth = m_mask_draw_context_depth[context];

        // if new, store current depth for all draws of the mask
        if(saved_context_depth == 0)
            saved_context_depth = depth;
        // if old, use stored depth
        else
            depth = saved_context_depth;
    }

    m_recent_draw_context = context;
    m_recent_draw_context_depth = depth;

    return m_ordered_draw_queue[{depth, context}];
}

// Adds vertices to a VertexList
inline void RenderGL::addVertices(VertexList& list, const RectI& loc, const RectF& tc, GLshort depth, const Vertex_t::Tint& tint)
{
    const GLshort x1 = loc.tl.x;
    const GLshort x2 = loc.br.x;
    const GLshort y1 = loc.tl.y;
    const GLshort y2 = loc.br.y;

    const GLfloat u1 = tc.tl.x;
    const GLfloat u2 = tc.br.x;
    const GLfloat v1 = tc.tl.y;
    const GLfloat v2 = tc.br.y;

    list.vertices.push_back({{x1, y1, depth}, tint, {u1, v1}});
    list.vertices.push_back({{x1, y2, depth}, tint, {u1, v2}});
    list.vertices.push_back({{x2, y1, depth}, tint, {u2, v1}});
    list.vertices.push_back({{x1, y2, depth}, tint, {u1, v2}});
    list.vertices.push_back({{x2, y1, depth}, tint, {u2, v1}});
    list.vertices.push_back({{x2, y2, depth}, tint, {u2, v2}});
}

inline void RenderGL::addVertices(VertexList& list, const QuadI& loc, const RectF& tc, GLshort depth, const Vertex_t::Tint& tint)
{
    const GLfloat u1 = tc.tl.x;
    const GLfloat u2 = tc.br.x;
    const GLfloat v1 = tc.tl.y;
    const GLfloat v2 = tc.br.y;

    list.vertices.push_back({{loc.tl.x, loc.tl.y, depth}, tint, {u1, v1}});
    list.vertices.push_back({{loc.bl.x, loc.bl.y, depth}, tint, {u1, v2}});
    list.vertices.push_back({{loc.tr.x, loc.tr.y, depth}, tint, {u2, v1}});
    list.vertices.push_back({{loc.bl.x, loc.bl.y, depth}, tint, {u1, v2}});
    list.vertices.push_back({{loc.tr.x, loc.tr.y, depth}, tint, {u2, v1}});
    list.vertices.push_back({{loc.br.x, loc.br.y, depth}, tint, {u2, v2}});
}

// simple helper function to make a triangle strip for a single-quad draw
std::array<RenderGL::Vertex_t, 4> RenderGL::genTriangleStrip(const RectI& loc, const RectF& tc, GLshort depth, const Vertex_t::Tint& tint)
{
    const GLshort x1 = loc.tl.x;
    const GLshort x2 = loc.br.x;
    const GLshort y1 = loc.tl.y;
    const GLshort y2 = loc.br.y;

    const GLfloat u1 = tc.tl.x;
    const GLfloat u2 = tc.br.x;
    const GLfloat v1 = tc.tl.y;
    const GLfloat v2 = tc.br.y;

    return {
        Vertex_t{{x1, y1, depth}, tint, {u1, v1}},
        Vertex_t{{x1, y2, depth}, tint, {u1, v2}},
        Vertex_t{{x2, y1, depth}, tint, {u2, v1}},
        Vertex_t{{x2, y2, depth}, tint, {u2, v2}},
    };
}

bool RenderGL::initRender(const CmdLineSetup_t &setup, SDL_Window *window)
{
    pLogDebug("Init renderer settings...");

    m_window = window;

    if(!AbstractRender_t::init())
        return false;

    if(!initOpenGL(setup)
        || !initDebug()
        || !initShaders()
        || !initFramebuffers()
        || !initState()
        || !initVertexArrays())
    {
        close();
        return false;
    }

    GLenum err = glGetError();
    if(err)
    {
        pLogDebug("Render GL: GL error %d occurred during init process, falling back to SDL.", err);
        close();
        return false;
    }


    // Clean-up from a possible start-up junk
    clearBuffer();

    updateViewport();

#ifdef MUTABLE_PARTICLES_DEMO
    s_sparkle = ResolveGLParticleSystem("sparkle")->get();
#endif

    return true;
}

void RenderGL::close()
{
    RenderGL::clearAllTextures();
    AbstractRender_t::close();

#ifdef RENDERGL_HAS_SHADERS
    m_standard_program.reset();
    m_bitmask_program.reset();
    m_output_program.reset();
    m_program_circle.reset();
    m_program_circle_hole.reset();
    m_program_rect_filled.reset();
    m_program_rect_unfilled.reset();
    m_lighting_program.reset();
#endif

    for(int i = BUFFER_GAME; i < BUFFER_MAX; i++)
        destroyFramebuffer((BufferIndex_t)i);

#ifdef RENDERGL_HAS_VBO
    if(m_vertex_buffer[0])
    {
        glDeleteBuffers(s_num_vertex_buffers, m_vertex_buffer);
        for(int i = 0; i < s_num_vertex_buffers; i++)
            m_vertex_buffer[i] = 0;
    }

    if(m_light_ubo)
    {
        glDeleteBuffers(1, &m_light_ubo);
        m_light_ubo = 0;
    }

#endif

#ifdef RENDERGL_HAS_VAO
    if(m_glcore_vao)
    {
        glDeleteVertexArrays(1, &m_glcore_vao);
        m_glcore_vao = 0;
    }
#endif

    if(m_gContext)
        SDL_GL_DeleteContext(m_gContext);

    m_gContext = nullptr;
}

void RenderGL::repaint()
{
#ifdef USE_RENDER_BLOCKING
    if(m_blockRender)
        return;
#endif

#ifdef MUTABLE_PARTICLES_DEMO
    if(s_sparkle)
        renderParticleSystem(*s_sparkle, vScreenX[1], vScreenY[1]);
#endif

    SuperPrintScreenCenter(fmt::format_ne("DEV BUILD - OpenGL {0}.{1} {2}", m_gl_majver, m_gl_minver, get_profile_name(m_gl_profile)), 3, 0, 0.8f, 1.0f, 0.0f);

    std::string feature_string = "Draw: ";
    if(m_use_logicop)
        feature_string += "LogicOp, ";
    if(m_use_depth_buffer)
        feature_string += "depth, ";
    if(m_use_shaders)
        feature_string += "shaders, ";
    if(m_client_side_arrays)
        feature_string += "client arr, ";
    if(m_has_es3_shaders)
        feature_string += "ES3, ";

    feature_string.resize(feature_string.size() - 2);
    if(feature_string.size() == 4)
        feature_string += ": NONE";

    SuperPrintScreenCenter(feature_string, 3, 20, 0.8f, 1.0f, 0.0f);

    feature_string = "FBO: ";
    if(m_buffer_texture[BUFFER_GAME])
        feature_string += "game, ";
    if(m_buffer_texture[BUFFER_FB_READ])
        feature_string += "local fx, ";
    if(m_buffer_texture[BUFFER_INT_PASS_2])
        feature_string += "n-pass fx, ";
    if(m_buffer_texture[BUFFER_LIGHTING])
        feature_string += "light, ";
    if(m_depth_read_texture)
        feature_string += "depth, ";

    feature_string.resize(feature_string.size() - 2);
    if(feature_string.size() == 3)
        feature_string += ": NONE";

    SuperPrintScreenCenter(feature_string, 3, 40, 0.8f, 1.0f, 0.0f);

    flushDrawQueues();

    m_cur_depth = 1;
    if(m_use_depth_buffer)
        glClear(GL_DEPTH_BUFFER_BIT);

#ifdef USE_SCREENSHOTS_AND_RECS
    if(TakeScreen)
    {
        makeShot();
        PlaySoundMenu(SFX_GotItem);
        TakeScreen = false;
    }
#endif

#ifdef USE_SCREENSHOTS_AND_RECS
    processRecorder();
#endif

#ifdef USE_DRAW_BATTERY_STATUS
    drawBatteryStatus();
#endif

    setTargetScreen();

    if(m_use_depth_buffer)
        glDisable(GL_DEPTH_TEST);

    if(m_game_texture_fb && m_game_texture)
    {
        clearBuffer();

        int hardware_w, hardware_h;
        getRenderSize(&hardware_w, &hardware_h);

        // draw screen at correct physical coordinates
        RectI draw_loc = RectI(m_phys_x, m_phys_y, m_phys_x + m_phys_w, m_phys_y + m_phys_h);
        RectF draw_source = RectF(0.0, 0.0, 1.0, 1.0);
        Vertex_t::Tint tint = {255, 255, 255, 255};

        std::array<Vertex_t, 4> vertex_attribs = genTriangleStrip(draw_loc, draw_source, 0, tint);

        fillVertexBuffer(vertex_attribs.data(), 4);

        glBindTexture(GL_TEXTURE_2D, m_game_texture);

#ifdef RENDERGL_HAS_SHADERS
        if(m_use_shaders)
        {
            m_output_program.use_program();
            m_output_program.update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);
        }
#endif

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Controls::RenderTouchControls();

    flushDrawQueues();

    if(m_use_depth_buffer)
        glEnable(GL_DEPTH_TEST);

    m_cur_depth = 1;

    SDL_GL_SwapWindow(m_window);

    m_current_frame++;
    if(m_current_frame % 512 == 0)
        cleanupDrawQueues();

    m_shader_clock = (GLfloat)((m_current_frame) % (65 * 60)) / 65.0f;

#if 0
    GLuint err;
    while((err = glGetError()) != 0)
        pLogWarning("Render GL: got GL error code %d on repaint", (int)err);
#endif

    // clear buffer in legacy GL
    if(!m_game_texture_fb)
        clearBuffer();
}

void RenderGL::applyViewport()
{
    if(m_recentTargetScreen)
        return;

    PointI off = m_viewport_offset_ignore ? PointI(0, 0) : m_viewport_offset;

    // fix offscreen coordinates
    RectSizeI viewport = m_viewport;

    s_normalize_coords(viewport);

    off += viewport.xy;
    off -= m_viewport.xy;

    int y_sign = 1;

    if(m_has_fbo && m_game_texture_fb)
    {
        RectSizeI viewport_scaled = viewport * m_render_scale_factor;

        glViewport(viewport_scaled.x, viewport_scaled.y, viewport_scaled.w, viewport_scaled.h);
    }
    else
    {
        int phys_offset_x = viewport.x * m_phys_w / ScreenW;
        int phys_width = viewport.w * m_phys_w / ScreenW;

        int phys_offset_y = viewport.y * m_phys_h / ScreenH;
        int phys_height = viewport.h * m_phys_h / ScreenH;

        y_sign = -1;

        glViewport(m_phys_x + phys_offset_x,
                m_phys_y + m_phys_h - phys_height - phys_offset_y, // relies on fact that m_phys_y is a symmetric border
                phys_width,
                phys_height);
    }

    if(m_use_shaders)
    {
        m_transform_matrix = {
            2.0f / (float)viewport.w, 0.0f, 0.0f, 0.0f,
            0.0f, y_sign * 2.0f / (float)viewport.h, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / (float)(1 << 15), 0.0f,
            -(float)(viewport.w + off.x + off.x) / (viewport.w), -y_sign * (float)(viewport.h + off.y + off.y) / (viewport.h), -1.0f, 1.0f,
        };

        m_shader_read_viewport = {
            // multiply
            0.5f * (float)viewport.w / (float)ScreenW,
            0.5f * (float)viewport.h / (float)ScreenH,

            // add
            ((float)(viewport.x + off.x) + 0.5f * (float)viewport.w) / (float)ScreenW,
            ((float)(viewport.y + off.y) + 0.5f * (float)viewport.h) / (float)ScreenH,
        };

        m_transform_tick++;
    }
    else
    {
#ifdef RENDERGL_HAS_FIXED_FUNCTION
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        m_Ortho(off.x, viewport.w + off.x, (y_sign == -1) * viewport.h + off.y, (y_sign == 1) * viewport.h + off.y, (1 << 15), -(1 << 15));
#endif
    }
}

void RenderGL::updateViewport()
{
    flushDrawQueues();

    int hardware_w, hardware_h;
    getRenderSize(&hardware_w, &hardware_h);

    // quickly update the HiDPI scaling factor
    int window_w, window_h;
    XWindow::getWindowSize(&window_w, &window_h);
    m_hidpi_x = (float)hardware_w / (float)window_w;
    m_hidpi_y = (float)hardware_h / (float)window_h;

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

    m_viewport = RectSizeI(0, 0, ScreenW, ScreenH);

    m_viewport_offset = PointI(0, 0);
    m_viewport_offset_ignore = false;

    pLogDebug("Phys screen is %d x %d", m_phys_w, m_phys_h);

    m_phys_x = hardware_w / 2 - m_phys_w / 2;
    m_phys_y = hardware_h / 2 - m_phys_h / 2;

    applyViewport();
}

void RenderGL::resetViewport()
{
    updateViewport();
}

void RenderGL::setViewport(int x, int y, int w, int h)
{
    if(x == m_viewport.x && y == m_viewport.y && w == m_viewport.w && h == m_viewport.h)
        return;

    flushDrawQueues();

    m_viewport = RectSizeI(x, y, w, h);

    applyViewport();
}

void RenderGL::offsetViewport(int x, int y)
{
    if(x == m_viewport_offset.x && y == m_viewport_offset.y)
        return;

    flushDrawQueues();

    m_viewport_offset = PointI(x, y);

    applyViewport();
}

void RenderGL::offsetViewportIgnore(bool en)
{
    if(en == m_viewport_offset_ignore)
        return;

    if(m_viewport_offset.x == 0 && m_viewport_offset.y == 0)
    {
        m_viewport_offset_ignore = en;
        return;
    }

    flushDrawQueues();

    m_viewport_offset_ignore = en;

    applyViewport();
}

void RenderGL::mapToScreen(int x, int y, int *dx, int *dy)
{
    *dx = static_cast<int>((static_cast<float>(x) * m_hidpi_x - m_phys_x) * ScreenW / m_phys_w);
    *dy = static_cast<int>((static_cast<float>(y) * m_hidpi_y - m_phys_y) * ScreenH / m_phys_h);
}

void RenderGL::mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    *window_x = ((float)scr_x * m_phys_w / ScreenW + m_phys_x) / m_hidpi_x;
    *window_y = ((float)scr_y * m_phys_h / ScreenH + m_phys_y) / m_hidpi_y;
}

void RenderGL::getRenderSize(int *w, int *h)
{
    if(!m_window)
    {
        pLogDebug("Render GL: getRenderSize called prior to GL initialization, returning 800 x 600");
        *w = 800;
        *h = 600;
    }

    SDL_GL_GetDrawableSize(m_window, w, h);

    if(*w <= 0 || *h <= 0)
    {
        pLogWarning("Render GL: SDL_GL_GetDrawableSize returned <= 0 (%d x %d)", *w, *h);
        SDL_GetWindowSize(m_window, w, h);
    }
}

void RenderGL::setTargetTexture()
{
    if(!m_recentTargetScreen)
        return;

    flushDrawQueues();

#ifdef RENDERGL_HAS_FBO
    if(m_game_texture_fb && m_game_texture)
        glBindFramebuffer(GL_FRAMEBUFFER, m_game_texture_fb);
#endif

    m_recentTargetScreen = false;
    applyViewport();
}

void RenderGL::setTargetScreen()
{
    if(m_recentTargetScreen)
        return;

    flushDrawQueues();

#ifdef RENDERGL_HAS_FBO
    if(m_has_fbo && m_game_texture_fb)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

    m_recentTargetScreen = true;

    int hardware_w, hardware_h;
    getRenderSize(&hardware_w, &hardware_h);

    glViewport(0,
            0,
            hardware_w,
            hardware_h);

    if(m_use_shaders)
    {
        m_transform_matrix = {
            2.0f / (float)hardware_w, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / (float)hardware_h, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / (float)(1 << 15), 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
        };

        m_shader_read_viewport = {
            0.0f, 0.0f, 0.0f, 0.0f,
        };

        m_transform_tick++;
    }
    else
    {
#ifdef RENDERGL_HAS_FIXED_FUNCTION
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        m_Ortho(0, hardware_w, hardware_h, 0, (1 << 15), -(1 << 15));
#endif
    }
}

void RenderGL::prepareDrawMask()
{
    if(!m_use_logicop)
        return;

#ifdef RENDERGL_HAS_LOGICOP
    // bitwise and
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_AND);
#endif // #ifdef RENDERGL_HAS_LOGICOP
}

void RenderGL::prepareDrawImage()
{
    if(!m_use_logicop)
        return;

#ifdef RENDERGL_HAS_LOGICOP
    // bitwise or
    glDisable(GL_COLOR_LOGIC_OP);
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_OR);
#endif // #ifdef RENDERGL_HAS_LOGICOP
}

void RenderGL::leaveMaskContext()
{
    if(!m_use_logicop)
        return;

#ifdef RENDERGL_HAS_LOGICOP
    // no bitwise op
    glDisable(GL_COLOR_LOGIC_OP);
#endif // #ifdef RENDERGL_HAS_LOGICOP
}

static int s_nextPowerOfTwo(int val)
{
    int power = 1;
    while(power < val)
        power *= 2;
    return power;
}

void RenderGL::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch, bool is_mask, uint32_t least_width, uint32_t least_height)
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
                target.d = StdPictureData();
            return;
        }

        if(is_mask)
            SDL_memset(padded_pixels, 255, pad_w * pad_h * 4);
        else
            SDL_memset(padded_pixels, 0, pad_w * pad_h * 4);

        for(uint32_t y = 0; y < height; y++)
        {
            for(uint32_t x = 0; x < width; x++)
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
        target.d = StdPictureData();
        return;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
    auto texborder = (m_gl_majver == 1 && m_gl_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
        ? GL_CLAMP : GL_CLAMP_TO_EDGE;
#else
    auto texborder = GL_CLAMP_TO_EDGE;
#endif

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texborder);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texborder);
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
        target.d.w_scale = static_cast<float>(width) / target.w;
        target.d.h_scale = static_cast<float>(height) / target.h;

        target.d.w_scale /= pad_w;
        target.d.h_scale /= pad_h;
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
        target.d = StdPictureData();
        return;
    }

    if(is_mask)
        target.d.mask_texture_id = tex_id;
    else
    {
        target.d.texture_id = tex_id;
        target.inited = true;
    }
    m_loadedPictures.insert(&target);

#if defined(__APPLE__) && defined(USE_APPLE_X11)
    // SDL_GL_UnbindTexture(texture); // Unbind texture after it got been loaded (otherwise a white screen will happen)
#endif
}

void RenderGL::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch)
{
    loadTexture(target, width, height, RGBApixels, pitch, false, width, height);
}

void RenderGL::loadTextureMask(StdPicture &target, uint32_t mask_width, uint32_t mask_height, uint8_t *RGBApixels, uint32_t pitch, uint32_t image_width, uint32_t image_height)
{
    loadTexture(target, mask_width, mask_height, RGBApixels, pitch, true, image_width, image_height);
}

void RenderGL::compileShaders(StdPicture &target)
{
#ifdef RENDERGL_HAS_SHADERS
    if(!m_use_shaders || target.d.shader_program)
        return;

    pLogDebug("Render GL: compiling shader...");

    if(target.l.particleVertexShaderSource.empty())
    {
        target.d.shader_program.reset(new GLProgramObject(
            s_es2_advanced_vert_src,
            target.l.fragmentShaderSource.data()
        ));
    }
    else
    {
        target.d.shader_program.reset(new GLProgramObject(
            target.l.particleVertexShaderSource.data(),
            target.l.fragmentShaderSource.empty() ? s_es2_standard_frag_src : target.l.fragmentShaderSource.data(),
            true
        ));

        target.d.particle_system.reset(new GLParticleSystem());
        target.d.particle_system->init(256);
    }

    if(target.d.shader_program && !target.d.shader_program->inited())
    {
        target.d.shader_program.reset();
        pLogDebug("Render GL: using default program due to failed compile/link...");
    }

    if(target.d.shader_program)
        target.d.shader_program->restore_uniforms(target.l);
#else
    UNUSED(target);
#endif
}

bool RenderGL::textureMaskSupported()
{
    return (m_use_logicop) || (m_use_shaders && m_buffer_texture[BUFFER_FB_READ]);
}

bool RenderGL::depthTestSupported()
{
    return m_use_depth_buffer;
}

bool RenderGL::userShadersSupported()
{
#ifdef THEXTECH_WIP_FEATURES
    return m_use_shaders && m_has_es3_shaders && m_buffer_texture[BUFFER_FB_READ];
#else
    // FIXME: should be false before submitting to the main repo
    return m_use_shaders && m_has_es3_shaders && m_buffer_texture[BUFFER_FB_READ];
#endif
}

void RenderGL::unloadTexture(StdPicture &tx)
{
    auto corpseIt = m_loadedPictures.find(&tx);
    if(corpseIt != m_loadedPictures.end())
        m_loadedPictures.erase(corpseIt);

    D_pLogDebug("RenderSDL: unloading texture at %p, new texture count %d...", &tx, (int)m_loadedPictures.size());

    if(tx.d.texture_id)
        glDeleteTextures(1, &tx.d.texture_id);

    if(tx.d.mask_texture_id)
        glDeleteTextures(1, &tx.d.mask_texture_id);

    tx.d = StdPictureData();

    if(!tx.l.canLoad())
        static_cast<StdPicture_Sub&>(tx) = StdPicture_Sub();

    return;
}

void RenderGL::clearAllTextures()
{
    for(StdPicture *tx : m_loadedPictures)
    {
        D_pLogDebug("RenderSDL: unloading texture at %p on clearAllTextures()", tx);

        if(tx->d.texture_id)
            glDeleteTextures(1, &tx->d.texture_id);

        if(tx->d.mask_texture_id)
            glDeleteTextures(1, &tx->d.mask_texture_id);

        tx->d = StdPictureData();

        if(!tx->l.canLoad())
            static_cast<StdPicture_Sub&>(*tx) = StdPicture_Sub();
    }

    m_loadedPictures.clear();
}

void RenderGL::clearBuffer()
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    clearDrawQueues();

    glBindTexture(GL_TEXTURE_2D, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}


int RenderGL::registerUniform(StdPicture &target, const char* name)
{
#ifdef THEXTECH_BUILD_GL_MODERN

    if(userShadersSupported() && target.d.shader_program && target.d.shader_program->inited())
        return target.d.shader_program->register_uniform(name, target.l);
    else
        return AbstractRender_t::registerUniform(target, name);

#else
    return AbstractRender_t::registerUniform(target, name);
#endif
}

void RenderGL::assignUniform(StdPicture &target, int index, const UniformValue_t& value)
{
#ifdef THEXTECH_BUILD_GL_MODERN

    if(userShadersSupported() && target.d.shader_program && target.d.shader_program->inited())
        return target.d.shader_program->assign_uniform(index, value, target.l);
    else
        return AbstractRender_t::assignUniform(target, index, value);

#else
    return AbstractRender_t::assignUniform(target, index, value);
#endif
}

void RenderGL::spawnParticle(StdPicture &target, double worldX, double worldY, ParticleVertexAttrs_t attrs)
{
    if(!target.inited)
        return;

    if(!target.d.texture_id && target.l.lazyLoaded)
        lazyLoad(target);

    if(!target.d.particle_system)
    {
        D_pLogWarningNA("Attempt to spawn particle in non-particle system StdPicture!");
        return;
    }

    ParticleVertexMutable_t particle;
    particle.position = {(GLfloat)worldX, (GLfloat)worldY};
    particle.spawn_time = m_shader_clock;
    particle.attribs = attrs;

    target.d.particle_system->add_particle(particle);
}

void RenderGL::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(!m_use_shaders && !filled)
    {
        renderRect(x, y, 2, h, red, green, blue, alpha, true);
        renderRect(x + w - 2, y, 2, h, red, green, blue, alpha, true);
        renderRect(x, y, w, 2, red, green, blue, alpha, true);
        renderRect(x, y + h - 2, w, 2, red, green, blue, alpha, true);
        return;
    }

    DrawContext_t context = {(filled) ? m_program_rect_filled : m_program_rect_unfilled};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    auto& vertex_list = ((m_use_depth_buffer && tint[3] == 255) ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    RectI draw_loc = RectI(x, y, x + w, y + h);

    // want interpolated value to be <= 0 for first two pixels, >= 1 for last two pixels
    float u1 = -2.0f / w;
    float u2 = (w + 2.0f) / w;

    float v1 = -2.0f / h;
    float v2 = (h + 2.0f) / h;

    RectF draw_source = RectF(u1, v1, u2, v2);

    addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    renderRect(_left, _top, _right - _left, _bottom - _top, red, green, blue, alpha);
}

void RenderGL::renderCircle(int cx, int cy, int radius, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    UNUSED(filled);

    if(radius <= 0)
        return; // Nothing to draw

    DrawContext_t context = {m_program_circle};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    auto& vertex_list = ((m_use_depth_buffer && tint[3] == 255) ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    if(m_use_shaders)
    {
        RectI draw_loc = RectI(cx - radius, cy - radius, cx + radius, cy + radius);
        RectF draw_source = RectF(0.0, 0.0, 1.0, 1.0);

        addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);
    }
    else
    {
        auto& vertex_attribs = vertex_list.vertices;

        // manually fill with  a full circle poly here
        const int verts = 20;
        const float two_pi = (float)(2 * M_PI);

        GLshort cx_s = cx;
        GLshort cy_s = cy;

        for(int i = 0; i < verts + 1; i++)
        {
            float theta = i * two_pi / verts;
            GLshort x = cx_s + (GLshort)roundf(radius * cosf(theta));
            GLshort y = cy_s + (GLshort)roundf(radius * sinf(theta));

            if(i != 0)
            {
                vertex_attribs.push_back({{x, y, m_cur_depth}, tint, {0.0, 0.0}});
                vertex_attribs.push_back({{cx_s, cy_s, m_cur_depth}, tint, {0.0, 0.0}});
            }
            if(i != verts)
                vertex_attribs.push_back({{x, y, m_cur_depth}, tint, {0.0, 0.0}});
        }
    }

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderCircleHole(int cx, int cy, int radius, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(radius <= 0)
        return; // Nothing to draw

    DrawContext_t context = {m_program_circle_hole};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    auto& vertex_list = ((m_use_depth_buffer && tint[3] == 255) ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    if(m_use_shaders)
    {
        RectI draw_loc = RectI(cx - radius, cy - radius, cx + radius, cy + radius);
        RectF draw_source = RectF(0.0, 0.0, 1.0, 1.0);

        addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);
    }
    else
    {
        auto& vertex_attribs = vertex_list.vertices;

        // manually fill with  a full circle poly here
        const int verts = 32;
        const float two_pi = (float)(2 * M_PI);

        GLshort cx_s = cx;
        GLshort cy_s = cy;

        for(int i = 0; i < verts; i++)
        {
            float theta1 = i * two_pi / verts;
            float theta2 = (i + 1) * two_pi / verts;

            GLshort x1_perim = cx_s + (GLshort)roundf(radius * cosf(theta1));
            GLshort y1_perim = cy_s + (GLshort)roundf(radius * sinf(theta1));

            GLshort x2_perim = cx_s + (GLshort)roundf(radius * cosf(theta2));
            GLshort y2_perim = cy_s + (GLshort)roundf(radius * sinf(theta2));

            // project onto box
            GLfloat box_radius1 = radius * 2 / (fabsf(cosf(theta1) - sinf(theta1)) + fabsf(cosf(theta1) + sinf(theta1)));
            GLshort x1_box = cx_s + (GLshort)roundf(box_radius1 * cosf(theta1));
            GLshort y1_box = cy_s + (GLshort)roundf(box_radius1 * sinf(theta1));

            GLfloat box_radius2 = radius * 2 / (fabsf(cosf(theta2) - sinf(theta2)) + fabsf(cosf(theta2) + sinf(theta2)));
            GLshort x2_box = cx_s + (GLshort)roundf(box_radius2 * cosf(theta2));
            GLshort y2_box = cy_s + (GLshort)roundf(box_radius2 * sinf(theta2));

            vertex_attribs.push_back({{x1_perim, y1_perim, m_cur_depth}, tint, {0.0, 0.0}});
            vertex_attribs.push_back({{x1_box, y1_box, m_cur_depth}, tint, {0.0, 0.0}});
            vertex_attribs.push_back({{x2_perim, y2_perim, m_cur_depth}, tint, {0.0, 0.0}});
            vertex_attribs.push_back({{x1_box, y1_box, m_cur_depth}, tint, {0.0, 0.0}});
            vertex_attribs.push_back({{x2_perim, y2_perim, m_cur_depth}, tint, {0.0, 0.0}});
            vertex_attribs.push_back({{x2_box, y2_box, m_cur_depth}, tint, {0.0, 0.0}});
        }
    }

    m_cur_depth++;
    m_drawQueued = true;
}


void RenderGL::renderTextureScaleEx(double xDstD, double yDstD, double wDstD, double hDstD,
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

    QuadI draw_loc;

    if(rotateAngle != 0.0)
    {
        int cx = center ? center->x : wDst / 2;
        int cy = center ? center->y : hDst / 2;

        RectI offset = RectI(-cx, -cy, wDst - cx, hDst - cy);

        draw_loc = QuadI(offset).rotate(rotateAngle);
        draw_loc += PointI(xDst + cx, yDst + cy);
    }
    else
    {
        draw_loc = QuadI(RectI(xDst, yDst, xDst + wDst, yDst + hDst));
    }

    RectF draw_source = RectF(xSrc, ySrc, xSrc + wSrc, ySrc + hSrc);
    draw_source *= PointF(tx.d.w_scale, tx.d.h_scale);

    if(flip & X_FLIP_HORIZONTAL)
        std::swap(draw_source.tl.x, draw_source.br.x);

    if(flip & X_FLIP_VERTICAL)
        std::swap(draw_source.tl.y, draw_source.br.y);

    DrawContext_t context = {tx.d.shader_program ? *tx.d.shader_program : m_standard_program, &tx};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    bool draw_opaque = (tx.d.use_depth_test && tint[3] == 255 && !tx.d.shader_program);
    auto& vertex_list = (draw_opaque ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderTextureScale(double xDst, double yDst, double wDst, double hDst,
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

    RectI draw_loc = RectI(xDst, yDst, xDst + wDst, yDst + hDst);

    RectF draw_source = RectF(0.0, 0.0, tx.w, tx.h);
    draw_source *= PointF(tx.d.w_scale, tx.d.h_scale);

    DrawContext_t context = {tx.d.shader_program ? *tx.d.shader_program : m_standard_program, &tx};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    bool draw_opaque = (tx.d.use_depth_test && tint[3] == 255 && !tx.d.shader_program);
    auto& vertex_list = (draw_opaque ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderTexture(double xDstD, double yDstD, double wDstD, double hDstD,
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

#ifdef MUTABLE_PARTICLES_DEMO
    if(&tx >= &GFXMario[1] && &tx <= &GFXMario[7])
    {
        if(s_sparkle)
            spawnParticle(*s_sparkle, xDstD - vScreenX[1], yDstD - vScreenY[1], ParticleVertexAttrs_t());
    }
#endif

#ifdef LIGHT_DEMO
    if(m_light_count < 63)
    {
        if(&tx == &GFXNPC[13])
        {
            m_light_queue.lights[m_light_count++] = Light::Point(xDstD + wDstD / 2.0, yDstD + hDstD / 2.0, m_cur_depth, LightColor(128, 0, 0), 100.0);
        }

        if(&tx == &GFXBackground[96])
        {
            m_light_queue.lights[m_light_count++] = Light::Point(xDstD + wDstD / 2.0, yDstD + hDstD / 2.0, m_cur_depth, LightColor(64, 0, 0), 250.0);
        }

        if(&tx == &GFXBackground[97])
        {
            m_light_queue.lights[m_light_count++] = Light::Point(xDstD + wDstD / 2.0, yDstD + hDstD / 2.0, m_cur_depth, LightColor(0, 64, 64), 250.0);
        }
    }
#endif

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

    RectI draw_loc = RectI(xDst, yDst, xDst + wDst, yDst + hDst);

    RectF draw_source = RectF(xSrc, ySrc, xSrc + wDst, ySrc + hDst);
    draw_source *= PointF(tx.d.w_scale, tx.d.h_scale);

    DrawContext_t context = {tx.d.shader_program ? *tx.d.shader_program : m_standard_program, &tx};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    bool draw_opaque = (tx.d.use_depth_test && tint[3] == 255 && !tx.d.shader_program);
    auto& vertex_list = (draw_opaque ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderTextureFL(double xDstD, double yDstD, double wDstD, double hDstD,
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

    QuadI draw_loc;

    if(rotateAngle != 0.0)
    {
        int cx = center ? center->x : wDst / 2;
        int cy = center ? center->y : hDst / 2;

        RectI offset = RectI(-cx, -cy, wDst - cx, hDst - cy);

        draw_loc = QuadI(offset).rotate(rotateAngle);
        draw_loc += PointI(xDst + cx, yDst + cy);
    }
    else
    {
        draw_loc = QuadI(RectI(xDst, yDst, xDst + wDst, yDst + hDst));
    }

    RectF draw_source = RectF(xSrc, ySrc, xSrc + wDst, ySrc + hDst);
    draw_source *= PointF(tx.d.w_scale, tx.d.h_scale);

    if(flip & X_FLIP_HORIZONTAL)
        std::swap(draw_source.tl.x, draw_source.br.x);

    if(flip & X_FLIP_VERTICAL)
        std::swap(draw_source.tl.y, draw_source.br.y);

    DrawContext_t context = {tx.d.shader_program ? *tx.d.shader_program : m_standard_program, &tx};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    bool draw_opaque = (tx.d.use_depth_test && tint[3] == 255 && !tx.d.shader_program);
    auto& vertex_list = (draw_opaque ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderTexture(float xDst, float yDst,
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

    RectI draw_loc = RectI(xDst, yDst, xDst + tx.w, yDst + tx.h);

    RectF draw_source = RectF(0.0, 0.0, tx.w, tx.h);
    draw_source *= PointF(tx.d.w_scale, tx.d.h_scale);

    DrawContext_t context = {tx.d.shader_program ? *tx.d.shader_program : m_standard_program, &tx};

    Vertex_t::Tint tint = F_TO_B(red, green, blue, alpha);

    bool draw_opaque = (tx.d.use_depth_test && tint[3] == 255 && !tx.d.shader_program);
    auto& vertex_list = (draw_opaque ? m_unordered_draw_queue[context] : getOrderedDrawVertexList(context, m_cur_depth));

    addVertices(vertex_list, draw_loc, draw_source, m_cur_depth, tint);

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::renderParticleSystem(StdPicture &tx,
                          double camX,
                          double camY)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(!tx.inited)
        return;

    if(!tx.d.texture_id && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.texture_id || !tx.d.shader_program)
    {
        D_pLogWarningNA("Attempt to render an uninitialized particle system!");
        return;
    }

    // assign depth
    assignUniform(tx, 0, UniformValue_t(m_cur_depth * m_transform_matrix[2 * 4 + 2] + m_transform_matrix[3 * 4 + 2]));

    // assign camera pos
    assignUniform(tx, 1, UniformValue_t((GLfloat)camX, (GLfloat)camY));

    DrawContext_t context = {*tx.d.shader_program, &tx};

    auto& vertex_attribs = getOrderedDrawVertexList(context, m_cur_depth).vertices;

    vertex_attribs.emplace_back();

    m_cur_depth++;
    m_drawQueued = true;
}

void RenderGL::getScreenPixels(int x, int y, int w, int h, unsigned char *pixels)
{
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

void RenderGL::getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels)
{
    int phys_x, phys_y, phys_w, phys_h;

#ifdef RENDERGL_HAS_FBO
    GLint prev_fb = -1;

    if(m_game_texture_fb && m_game_texture)
    {
        phys_x = x * m_render_scale_factor;
        phys_y = y * m_render_scale_factor;
        phys_w = w * m_render_scale_factor;
        phys_h = h * m_render_scale_factor;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fb);
        glBindFramebuffer(GL_FRAMEBUFFER, m_game_texture_fb);
    }
    else
#endif
    {
        mapFromScreen(x, y, &phys_x, &phys_y);

        phys_w = w * m_phys_w / ScreenW;
        phys_h = h * m_phys_h / ScreenH;
    }

    // allocate buffer for screen-space pixels
    uint8_t* phys_pixels = (uint8_t*)malloc(phys_w * phys_h * 4);
    if(!phys_pixels)
        return;

    // read screen-space pixels
    glReadPixels(phys_x, phys_y, phys_w, phys_h,
        GL_RGBA, GL_UNSIGNED_BYTE, phys_pixels);

#ifdef RENDERGL_HAS_FBO
    if(prev_fb != -1)
        glBindFramebuffer(GL_FRAMEBUFFER, prev_fb);
#endif

    // rescale and move to target
    for(int r = 0; r < h; r++)
    {
        int phys_r = r * phys_h / h;

        for(int c = 0; c < w; c++)
        {
            int phys_c = c * phys_w / w;

            ((uint32_t*) pixels)[r * w + c] = ((uint32_t*) phys_pixels)[phys_r * phys_w + phys_c];
        }
    }

    free(phys_pixels);
}

int RenderGL::getPixelDataSize(const StdPicture &tx)
{
    if(!tx.d.texture_id)
        return 0;
    return (tx.w * tx.h * 4);
}

void RenderGL::getPixelData(const StdPicture &tx, unsigned char *pixelData)
{
    UNUSED(tx);
    UNUSED(pixelData);
}
