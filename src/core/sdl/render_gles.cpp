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

#include <fstream>

#include <SDL2/SDL_version.h>
#include <SDL2/SDL_render.h>

#ifndef THEXTECH_GL_ES_ONLY
#    define GL_GLEXT_PROTOTYPES 1
#    include <SDL2/SDL_opengl.h>
#    include <SDL2/SDL_opengl_glext.h>
#endif

#include <SDL2/SDL_opengles2.h>

#include <FreeImageLite.h>
#include <Graphics/graphics_funcs.h>

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "core/sdl/render_gles.h"
#include "core/sdl/gl_program_object.h"

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

static std::string s_read_file(const char* filename)
{
    std::ifstream is(filename, std::ios::binary);

    if(!is)
        return "";

    int length;
    is.seekg(0, is.end);
    length = is.tellg();
    is.seekg(0, is.beg);

    std::string out;
    out.resize(length);

    // read data as a block:
    is.read(&out[0], length);

    if(!is)
        return "";

    return out;
}

RenderGLES::RenderGLES() :
    AbstractRender_t()
{}

RenderGLES::~RenderGLES()
{
    if(m_window)
        RenderGLES::close();
}

unsigned int RenderGLES::SDL_InitFlags()
{
    return 0;
}

bool RenderGLES::isWorking()
{
    return m_gContext;
}

// GL values (migrate to RenderGLES class members soon)
#ifndef THEXTECH_GL_ES_ONLY
static bool s_gles_mode = false;
static bool s_emulate_logic_ops = false;

static GLuint s_glcore_vao = 0;
#else
static constexpr bool s_emulate_logic_ops = true;
#endif

static std::vector<GLProgramObject*> s_program_bank;
static GLProgramObject s_program;
static GLProgramObject s_special_program;
static GLProgramObject s_output_program;
static GLProgramObject s_program_rect_filled;
static GLProgramObject s_program_rect_unfilled;
static GLProgramObject s_program_circle;
static GLProgramObject s_program_circle_hole;

static GLuint s_fb_read_texture = 0;

static GLuint s_game_texture = 0;
static GLuint s_game_texture_fb = 0;

static constexpr int s_num_buffers = 16;
static GLuint s_vertex_buffer[s_num_buffers] = {0};
static GLsizeiptr s_vertex_buffer_size[s_num_buffers] = {0};
static int s_cur_buffer_index = 0;

static std::array<GLfloat, 16> s_transform_matrix;

static void s_update_fb_read_texture(int x, int y, int w, int h)
{
    if(x >= ScreenW || y >= ScreenH)
        return;

    if(x < 0)
    {
        w += x;
        x = 0;
    }

    if(y < 0)
    {
        h += y;
        y = 0;
    }

    if(x + w >= ScreenW)
        w = ScreenW - x;

    if(y + h >= ScreenH)
        h = ScreenH - y;

    glActiveTexture(GL_TEXTURE1);
    glCopyTexSubImage2D(GL_TEXTURE_2D,
        0,
        x,
        ScreenH - (y + h),
        x,
        ScreenH - (y + h),
        w,
        h);
}

static void s_fill_buffer(const RenderGLES::Vertex_t* vertex_attribs, int count)
{
    s_cur_buffer_index++;
    if(s_cur_buffer_index >= s_num_buffers)
        s_cur_buffer_index = 0;

    GLsizeiptr buffer_size = sizeof(RenderGLES::Vertex_t) * count;

    glBindBuffer(GL_ARRAY_BUFFER, s_vertex_buffer[s_cur_buffer_index]);

    if(s_vertex_buffer_size[s_cur_buffer_index] < buffer_size)
    {
        glBufferData(GL_ARRAY_BUFFER, buffer_size, vertex_attribs, GL_STREAM_DRAW);
        s_vertex_buffer_size[s_cur_buffer_index] = buffer_size;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, vertex_attribs);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderGLES::Vertex_t), (void*)offsetof(RenderGLES::Vertex_t, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderGLES::Vertex_t), (void*)offsetof(RenderGLES::Vertex_t, texcoord));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE,  sizeof(RenderGLES::Vertex_t), (void*)offsetof(RenderGLES::Vertex_t, tint));
}

bool RenderGLES::initRender(const CmdLineSetup_t &setup, SDL_Window *window)
{
    pLogDebug("Init renderer settings...");

    if(!AbstractRender_t::init())
        return false;

    m_window = window;

    Uint32 renderFlags = 0;

#ifdef THEXTECH_GL_ES_ONLY
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_gContext = SDL_GL_CreateContext(m_window);

    if(!m_gContext)
    {
        pLogCritical("Unable to create renderer!");
        return false;
    }

    GLenum err;

    int mask, maj_ver, min_ver;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &mask);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj_ver);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min_ver);

    pLogDebug("Initialized OpenGL %d.%d with profile %d", maj_ver, min_ver, mask);
    pLogDebug("OpenGL version: %s", glGetString(GL_VERSION));
    pLogDebug("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    pLogDebug("OpenGL renderer: %s", glGetString(GL_RENDERER));

#ifndef THEXTECH_GL_ES_ONLY
    if(mask == SDL_GL_CONTEXT_PROFILE_CORE)
    {
        glGenVertexArrays(1, &s_glcore_vao);
        glBindVertexArray(s_glcore_vao);
    }
    else if(mask == SDL_GL_CONTEXT_PROFILE_ES)
    {
        s_gles_mode = true;
        s_emulate_logic_ops = true;
    }
#endif // #ifndef THEXTECH_GL_ES_ONLY

    SDL_GL_SetSwapInterval(0);

    // SDL_RendererInfo ri;
    // SDL_GetRendererInfo(m_gRenderer, &ri);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint maxTextureSize = 256;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    glGenFramebuffers(1, &s_game_texture_fb);

    if(s_game_texture_fb)
    {
        glGenTextures(1, &s_game_texture);
    }

    while(err = glGetError())
        pLogWarning("Render GL 138: initing got GL error code %d", (int)err);

    if(s_game_texture_fb && s_game_texture)
    {
        // try to allocate texture memory
        glBindTexture(GL_TEXTURE_2D, s_game_texture);

        while(err = glGetError())
            pLogWarning("Render GL 201: initing got GL error code %d", (int)err);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            ScreenW, ScreenH,
            0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        while(err = glGetError())
            pLogWarning("Render GL 208: initing got GL error code %d", (int)err);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        while(err = glGetError())
            pLogWarning("Render GL 214: initing got GL error code %d", (int)err);

        glBindTexture(GL_TEXTURE_2D, 0);

        while(err = glGetError())
            pLogWarning("Render GL 219: initing got GL error code %d", (int)err);

        GLenum err = glGetError();
        if(err)
        {
            pLogWarning("Render GL: could not allocate game screen texture (%d). Falling back to screen-space scaling.", (int)err);
            glDeleteTextures(1, &s_game_texture);
            s_game_texture = 0;

            glDeleteFramebuffers(1, &s_game_texture_fb);
            s_game_texture_fb = 0;
        }
    }

    if(s_game_texture_fb && s_game_texture)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, s_game_texture_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_game_texture, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status !=  GL_FRAMEBUFFER_COMPLETE)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            glDeleteTextures(1, &s_game_texture);
            s_game_texture = 0;

            glDeleteFramebuffers(1, &s_game_texture_fb);
            s_game_texture_fb = 0;

            pLogWarning("Render GL: could not bind framebuffer texture target (%d). Falling back to screen-space scaling.", (int)status);
        }
        else
        {
            clearBuffer();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    while(err = glGetError())
        pLogWarning("Render GL 187: initing got GL error code %d", (int)err);

    glGenTextures(1, &s_fb_read_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_fb_read_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        ScreenW, ScreenH,
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glActiveTexture(GL_TEXTURE0);

    m_maxTextureWidth = maxTextureSize;
    m_maxTextureHeight = maxTextureSize;

    while(err = glGetError())
        pLogWarning("Render GL 198: initing got GL error code %d", (int)err);

    const char* vertex_src = (
        "#version 100                 \n"
        "uniform   mat4 u_transform;  \n"
        "attribute vec4 a_position;   \n"
        "attribute vec2 a_texcoord;   \n"
        "attribute vec4 a_tint;   \n"
        "varying   vec2 v_texcoord;   \n"
        "varying   vec4 v_tint;       \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = u_transform * a_position;  \n"
        "   v_texcoord = a_texcoord;  \n"
        "   v_tint = a_tint;  \n"
        "}                            \n"
    );

    s_program = GLProgramObject(
        vertex_src,
        (
            "#version 100                  \n"
            "precision mediump float;\n"
            "varying   vec2      v_texcoord;     \n"
            "uniform   sampler2D u_texture;  \n"
            "varying   vec4      v_tint;     \n"
            "void main()                                  \n"
            "{                                            \n"
            "  vec4 l_color = texture2D(u_texture, v_texcoord);\n"
            "  gl_FragColor = v_tint * l_color;\n"
            "}                                            \n"
        )
    );

    std::string logic_contents = s_read_file((AppPath + "/logic.frag").c_str());

    if(logic_contents.empty())
    {
        logic_contents =
R"RAW(#version 100

precision mediump float;

varying   vec2      v_texcoord;
varying   vec2      v_fbcoord;
varying   vec4      v_tint;

uniform   sampler2D u_texture;
uniform   sampler2D u_framebuffer;
uniform   sampler2D u_mask;

vec3 bitwise_ops(vec3 x, vec3 y, vec3 z)
{
    x *= 255.0;
    x /= 2.0;
    vec3 x_1 = fract(x);
    x -= x_1;
    x /= 2.0;
    vec3 x_2 = fract(x);
    x -= x_2;
    x /= 2.0;
    vec3 x_4 = fract(x);
    x -= x_4;
    x /= 2.0;
    vec3 x_8 = fract(x);
    x -= x_8;
    x /= 2.0;
    vec3 x_16 = fract(x);
    x -= x_16;
    x /= 2.0;
    vec3 x_32 = fract(x);
    x -= x_32;
    x /= 2.0;
    vec3 x_64 = fract(x);
    x -= x_64;
    x /= 2.0;
    vec3 x_128 = x;

    y *= 255.0;
    y /= 2.0;
    vec3 y_1 = fract(y);
    y -= y_1;
    y /= 2.0;
    vec3 y_2 = fract(y);
    y -= y_2;
    y /= 2.0;
    vec3 y_4 = fract(y);
    y -= y_4;
    y /= 2.0;
    vec3 y_8 = fract(y);
    y -= y_8;
    y /= 2.0;
    vec3 y_16 = fract(y);
    y -= y_16;
    y /= 2.0;
    vec3 y_32 = fract(y);
    y -= y_32;
    y /= 2.0;
    vec3 y_64 = fract(y);
    y -= y_64;
    y /= 2.0;
    vec3 y_128 = y;

    z *= 255.0;
    z /= 2.0;
    vec3 z_1 = fract(z);
    z -= z_1;
    z /= 2.0;
    vec3 z_2 = fract(z);
    z -= z_2;
    z /= 2.0;
    vec3 z_4 = fract(z);
    z -= z_4;
    z /= 2.0;
    vec3 z_8 = fract(z);
    z -= z_8;
    z /= 2.0;
    vec3 z_16 = fract(z);
    z -= z_16;
    z /= 2.0;
    vec3 z_32 = fract(z);
    z -= z_32;
    z /= 2.0;
    vec3 z_64 = fract(z);
    z -= z_64;
    z /= 2.0;
    vec3 z_128 = z;

    vec3 o_1 = max(min(x_1, y_1), z_1);
    vec3 o_2 = max(min(x_2, y_2), z_2);
    vec3 o_4 = max(min(x_4, y_4), z_4);
    vec3 o_8 = max(min(x_8, y_8), z_8);
    vec3 o_16 = max(min(x_16, y_16), z_16);
    vec3 o_32 = max(min(x_32, y_32), z_32);
    vec3 o_64 = max(min(x_64, y_64), z_64);
    vec3 o_128 = max(min(x_128, y_128), z_128);

    return (o_1 + o_2 * 2.0 + o_4 * 4.0 + o_8 * 8.0 + o_16 * 16.0 + o_32 * 32.0 + o_64 * 64.0 + o_128 * 128.0) * 2.0 / 255.0;
}

void main()
{
  vec3 l_image = texture2D(u_texture, v_texcoord).rgb;
  vec3 l_mask = texture2D(u_mask, v_texcoord).rgb;

  l_image *= v_tint.rgb;

  vec2 src = v_fbcoord.xy;
  // src.y += sin(v_fbcoord.x * 6.0 + l_mask.r + l_mask.g + l_mask.b) * (l_image.r + l_image.g + l_image.b + 3.0 - l_mask.r - l_mask.g - l_mask.b) / 9.0;

  vec3 l_bg = texture2D(u_framebuffer, src).rgb;

  gl_FragColor.rgb = bitwise_ops(l_bg, l_mask, l_image);
  gl_FragColor.a = 1.0;
}
)RAW";
    }

    s_special_program = GLProgramObject(
        (
            "#version 100                 \n"
            "uniform   mat4 u_transform;  \n"
            "attribute vec4 a_position;   \n"
            "attribute vec2 a_texcoord;   \n"
            "attribute vec4 a_tint;       \n"
            "varying   vec2 v_texcoord;   \n"
            "varying   vec2 v_fbcoord;    \n"
            "varying   vec4 v_tint;       \n"
            "void main()                  \n"
            "{                            \n"
            "   gl_Position = u_transform * a_position;  \n"
            "   v_texcoord = a_texcoord;  \n"
            "   v_tint = a_tint;     \n"
            "   v_fbcoord = vec2(gl_Position);  \n"
            "   v_fbcoord.x += 1.0;  \n"
            "   v_fbcoord.x /= 2.0;  \n"
            "   v_fbcoord.y += 1.0;  \n"
            "   v_fbcoord.y /= 2.0;  \n"
            "}                            \n"
        ),
        logic_contents.c_str()
    );

    s_program_rect_filled = GLProgramObject(
        vertex_src,
        (
            "#version 100                  \n"
            "precision mediump float;\n"
            "varying   vec2      v_texcoord;     \n"
            "varying   vec4      v_tint;         \n"
            "void main()                                  \n"
            "{                                            \n"
            "  gl_FragColor = v_tint;\n"
            "}                                            \n"
        )
    );

    s_program_rect_unfilled = GLProgramObject(
        vertex_src,
        (
            "#version 100                                 \n"
            "precision mediump float;                     \n"
            "varying   vec2      v_texcoord;              \n"
            "varying   vec4      v_tint;                  \n"
            "void main()                                  \n"
            "{                                            \n"
            "  if(v_texcoord.x >= 0.0 && v_texcoord.x < 1.0 \n"
            "     && v_texcoord.y >= 0.0 && v_texcoord.y < 1.0) \n"
            "    discard;                                 \n"
            "  gl_FragColor = v_tint;                     \n"
            "}                                            \n"
        )
    );

    s_program_circle = GLProgramObject(
        vertex_src,
        (
            "#version 100                  \n"
            "precision mediump float;\n"
            "varying   vec2      v_texcoord;     \n"
            "varying   vec4      v_tint;     \n"
            "void main()                                  \n"
            "{                                            \n"
            "  if((v_texcoord.x - 0.5) * (v_texcoord.x - 0.5) \n"
            "    + (v_texcoord.y - 0.5) * (v_texcoord.y - 0.5) > 0.25)\n"
            "      discard;\n"
            "  gl_FragColor = v_tint;\n"
            "}                                            \n"
        )
    );

    s_program_circle_hole = GLProgramObject(
        vertex_src,
        (
            "#version 100                  \n"
            "precision mediump float;\n"
            "varying   vec2      v_texcoord;     \n"
            "varying   vec4      v_tint;     \n"
            "void main()                                  \n"
            "{                                            \n"
            "  if((v_texcoord.x - 0.5) * (v_texcoord.x - 0.5) \n"
            "    + (v_texcoord.y - 0.5) * (v_texcoord.y - 0.5) <= 0.25)\n"
            "      discard;\n"
            "  gl_FragColor = v_tint;\n"
            "}                                            \n"
        )
    );

    std::string shader_contents = s_read_file((AppPath + "/test.frag").c_str());

    if(!shader_contents.empty())
    {
        pLogDebug("Loading screen fragment shader from test.frag...");
        s_output_program = GLProgramObject(
            vertex_src,
            shader_contents.c_str()
        );
    }
    else
    {
        s_output_program = GLProgramObject(
            vertex_src,
            (
                "#version 100                  \n"
                "precision mediump float;\n"
                "varying   vec2      v_texcoord;     \n"
                "uniform   sampler2D u_texture;  \n"
                "varying   vec4      v_tint;     \n"
                "void main()                                  \n"
                "{                                            \n"
                "  vec4 l_color = texture2D(u_texture, v_texcoord);\n"
                "  gl_FragColor = v_tint * l_color;\n"
                "}                                            \n"
            )
        );
    }

    while(err = glGetError())
        pLogWarning("Render GL 225: initing got GL error code %d", (int)err);

    // initialize vertex buffers
    glGenBuffers(s_num_buffers, s_vertex_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    for(int i = 0; i < s_num_buffers; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, s_vertex_buffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_t) * 4, nullptr, GL_STREAM_DRAW);
        s_vertex_buffer_size[i] = sizeof(Vertex_t) * 4;
    }

    while(err = glGetError())
        pLogWarning("Render GL 200: initing got GL error code %d", (int)err);

    while(err = glGetError())
        pLogWarning("Render GL 238: initing got GL error code %d", (int)err);

    // Clean-up from a possible start-up junk
    clearBuffer();

    updateViewport();

    return true;
}

void RenderGLES::close()
{
    RenderGLES::clearAllTextures();
    AbstractRender_t::close();

    if(s_game_texture_fb)
    {
        glDeleteFramebuffers(1, &s_game_texture_fb);
        s_game_texture_fb = 0;
    }

    if(s_game_texture)
    {
        glDeleteTextures(1, &s_game_texture);
        s_game_texture = 0;
    }

    if(s_fb_read_texture)
    {
        glDeleteTextures(1, &s_fb_read_texture);
        s_fb_read_texture = 0;
    }

    if(s_vertex_buffer[0])
    {
        glDeleteBuffers(s_num_buffers, s_vertex_buffer);
        for(int i = 0; i < s_num_buffers; i++)
            s_vertex_buffer[i] = 0;
    }

#ifndef THEXTECH_GL_ES_ONLY
    if(s_glcore_vao)
    {
        glDeleteVertexArrays(1, &s_glcore_vao);
        s_glcore_vao = 0;
    }
#endif

    if(m_gContext)
        SDL_GL_DeleteContext(m_gContext);

    m_gContext = nullptr;
}

void RenderGLES::togglehud()
{
#ifndef THEXTECH_GL_ES_ONLY
    if(!s_gles_mode)
    {
        s_emulate_logic_ops = !s_emulate_logic_ops;
        PlaySoundMenu(SFX_Raccoon);
    }
    else
#endif
        PlaySoundMenu(SFX_BlockHit);
}

void RenderGLES::repaint()
{
    while(GLint err = glGetError())
        pLogWarning("Render GL rp1: initing got GL error code %d", (int)err);

#ifdef USE_RENDER_BLOCKING
    if(m_blockRender)
        return;
#endif

    if(s_emulate_logic_ops)
        SuperPrintScreenCenter("Logic Op Shader (ES)", 3, 0);
    else
        SuperPrintScreenCenter("Logic Op Render (X64)", 3, 0);

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

    glFlush();

    setTargetScreen();
    clearBuffer();

    if(s_game_texture_fb && s_game_texture)
    {
        int hardware_w, hardware_h;
        XWindow::getWindowSize(&hardware_w, &hardware_h);

        // draw screen at correct physical coordinates
        float x1 = m_phys_x;
        float x2 = m_phys_x + m_phys_w;
        float y1 = m_phys_y;
        float y2 = m_phys_y + m_phys_h;

        const Vertex_t vertex_attribs[] =
        {
            {{x1, y1, 0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 1.0}},
            {{x1, y2, 0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0}},
            {{x2, y1, 0}, {1.0, 1.0, 1.0, 1.0}, {1.0, 1.0}},
            {{x2, y2, 0}, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.0}},
        };

        s_fill_buffer(vertex_attribs, 4);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, s_game_texture);

        s_output_program.use_program();
        s_output_program.update_transform(s_transform_matrix.data());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    while(GLint err = glGetError())
        pLogWarning("Render GL rp2: initing got GL error code %d", (int)err);

    Controls::RenderTouchControls();

    glFlush();
    SDL_GL_SwapWindow(m_window);
}

void RenderGLES::applyViewport()
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

    GLint cur_fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cur_fb);

    if(cur_fb != 0 && cur_fb == s_game_texture_fb)
    {
        glViewport(viewport_x, ScreenH - (viewport_y + viewport_h),
            viewport_w, viewport_h);

        s_transform_matrix = {
            2.0f / viewport_w, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / viewport_h, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -float(viewport_w + off_x + off_x) / (viewport_w), float(viewport_h + off_y + off_y) / (viewport_h), 0.0f, 1.0f,
        };

        s_program.set_transform_dirty();
        s_output_program.set_transform_dirty();
        s_program_rect_filled.set_transform_dirty();
        s_program_rect_unfilled.set_transform_dirty();
        s_program_circle.set_transform_dirty();
        s_program_circle_hole.set_transform_dirty();

        return;
    }

    int phys_offset_x = viewport_x * m_phys_w / ScreenW;
    int phys_width = viewport_w * m_phys_w / ScreenW;

    int phys_offset_y = viewport_y * m_phys_h / ScreenH;
    int phys_height = viewport_h * m_phys_h / ScreenH;

    glViewport(m_phys_x + phys_offset_x,
            m_phys_y + m_phys_h - phys_height - phys_offset_y, // relies on fact that m_phys_y is a symmetric border
            phys_width,
            phys_height);

    // glMatrixMode( GL_PROJECTION );
    // glLoadIdentity();

    // pLogDebug("Setting projection to %d %d %d %d", off_x, m_viewport_w + off_x, m_viewport_h + off_y, off_y);
    // glOrtho( off_x + 0.5f, viewport_w + off_x + 0.5f, viewport_h + off_y + 0.5f, off_y + 0.5f, -1, 1);
    s_transform_matrix = {
        2.0f / viewport_w, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / viewport_h, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -(viewport_w + off_x + 0.5f + off_x + 0.5f) / (viewport_w), (viewport_h + off_y + 0.5f + off_y + 0.5f) / (viewport_h), 0.0f, 1.0f,
    };

    s_program.set_transform_dirty();
    s_output_program.set_transform_dirty();
    s_program_rect_filled.set_transform_dirty();
    s_program_rect_unfilled.set_transform_dirty();
    s_program_circle.set_transform_dirty();
    s_program_circle_hole.set_transform_dirty();
}

void RenderGLES::updateViewport()
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
}

void RenderGLES::resetViewport()
{
    updateViewport();
}

void RenderGLES::setViewport(int x, int y, int w, int h)
{
    m_viewport_x = x;
    m_viewport_y = y;
    m_viewport_w = w;
    m_viewport_h = h;

    applyViewport();
}

void RenderGLES::offsetViewport(int x, int y)
{
    m_viewport_offset_x = x;
    m_viewport_offset_y = y;

    applyViewport();
}

void RenderGLES::offsetViewportIgnore(bool en)
{
    m_viewport_offset_ignore = en;

    applyViewport();
}

void RenderGLES::mapToScreen(int x, int y, int *dx, int *dy)
{
    *dx = static_cast<int>((static_cast<float>(x) - m_phys_x) * ScreenW / m_phys_w);
    *dy = static_cast<int>((static_cast<float>(y) - m_phys_y) * ScreenH / m_phys_h);
}

void RenderGLES::mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    *window_x = (float)scr_x * m_phys_w / ScreenW + m_phys_x;
    *window_y = (float)scr_y * m_phys_h / ScreenH + m_phys_y;
}

void RenderGLES::setTargetTexture()
{
    if(!m_recentTargetScreen)
        return;

    if(s_game_texture_fb && s_game_texture)
        glBindFramebuffer(GL_FRAMEBUFFER, s_game_texture_fb);

    m_recentTargetScreen = false;
    applyViewport();
}

void RenderGLES::setTargetScreen()
{
    if(m_recentTargetScreen)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_recentTargetScreen = true;

    int hardware_w, hardware_h;
    XWindow::getWindowSize(&hardware_w, &hardware_h);

    glViewport(0,
            0,
            hardware_w,
            hardware_h);

    // glMatrixMode( GL_PROJECTION );
    // glLoadIdentity();

    // glOrtho(0, hardware_w, hardware_h, 0, -1, 1);
    s_transform_matrix = {
        2.0f / hardware_w, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / hardware_h, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };

    s_program.set_transform_dirty();
    s_output_program.set_transform_dirty();
    s_program_rect_filled.set_transform_dirty();
    s_program_rect_unfilled.set_transform_dirty();
    s_program_circle.set_transform_dirty();
    s_program_circle_hole.set_transform_dirty();
}

void RenderGLES::prepareDrawMask()
{
#ifndef THEXTECH_GL_ES_ONLY
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
#endif // #ifndef THEXTECH_GL_ES_ONLY
}

void RenderGLES::prepareDrawImage()
{
#ifndef THEXTECH_GL_ES_ONLY
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
    else if(m_draw_mask_mode == 2)
    {
        // normal
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // max
        glBlendEquation(GL_MAX);
    }
    else
    {
        // add
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
#endif // #ifndef THEXTECH_GL_ES_ONLY
}

void RenderGLES::leaveMaskContext()
{
#ifndef THEXTECH_GL_ES_ONLY
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
#endif // #ifndef THEXTECH_GL_ES_ONLY
}

static int s_nextPowerOfTwo(int val)
{
    int power = 1;
    while (power < val)
        power *= 2;
    return power;
}

void RenderGLES::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch, bool is_mask, uint32_t least_width, uint32_t least_height)
{
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
    // glActiveTexture(GL_TEXTURE0);
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

void RenderGLES::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch)
{
    loadTexture(target, width, height, RGBApixels, pitch, false, width, height);
}

void RenderGLES::loadTextureMask(StdPicture &target, uint32_t mask_width, uint32_t mask_height, uint8_t *RGBApixels, uint32_t pitch, uint32_t image_width, uint32_t image_height)
{
    loadTexture(target, mask_width, mask_height, RGBApixels, pitch, true, image_width, image_height);
}

bool RenderGLES::textureMaskSupported()
{
    return true;
}

void RenderGLES::deleteTexture(StdPicture &tx, bool lazyUnload)
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

void RenderGLES::clearAllTextures()
{
    for(StdPicture *tx : m_textureBank)
        deleteTexture(*tx, false);

    m_textureBank.clear();
}

void RenderGLES::clearBuffer()
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

void RenderGLES::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    float x1 = x;
    float x2 = x + w;
    float y1 = y;
    float y2 = y + h;

    // want interpolated value to be <= 0 for first two pixels, >= 1 for last two pixels
    float u1 = -2.0f / w;
    float u2 = (w + 2.0f) / w;

    float v1 = -2.0f / h;
    float v2 = (h + 2.0f) / h;

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {u1, v1}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {u1, v2}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {u2, v1}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {u2, v2}},
    };

    s_fill_buffer(vertex_attribs, 4);

    if(filled)
    {
        s_program_rect_filled.use_program();
        s_program_rect_filled.update_transform(s_transform_matrix.data());
    }
    else
    {
        s_program_rect_unfilled.use_program();
        s_program_rect_unfilled.update_transform(s_transform_matrix.data());
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void RenderGLES::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    renderRectBR(_left, _top, _right - _left, _bottom - _top, red, green, blue, alpha);
}

void RenderGLES::renderCircle(int cx, int cy, int radius, float red, float green, float blue, float alpha, bool filled)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif
    UNUSED(filled);

    if(radius <= 0)
        return; // Nothing to draw

    float x1 = cx - radius;
    float x2 = cx + radius;
    float y1 = cy - radius;
    float y2 = cy + radius;

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {0.0, 0.0}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {0.0, 1.0}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {1.0, 0.0}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {1.0, 1.0}},
    };

    s_fill_buffer(vertex_attribs, 4);

    s_program_circle.use_program();
    s_program_circle.update_transform(s_transform_matrix.data());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void RenderGLES::renderCircleHole(int cx, int cy, int radius, float red, float green, float blue, float alpha)
{
#ifdef USE_RENDER_BLOCKING
    SDL_assert(!m_blockRender);
#endif

    if(radius <= 0)
        return; // Nothing to draw

    float x1 = cx - radius;
    float x2 = cx + radius;
    float y1 = cy - radius;
    float y2 = cy + radius;

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {0.0, 0.0}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {0.0, 1.0}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {1.0, 0.0}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {1.0, 1.0}},
    };

    s_fill_buffer(vertex_attribs, 4);

    s_program_circle_hole.use_program();
    s_program_circle_hole.update_transform(s_transform_matrix.data());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void RenderGLES::renderTextureScaleEx(double xDstD, double yDstD, double wDstD, double hDstD,
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

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {u1, v1}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {u1, v2}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {u2, v1}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {u2, v2}}
    };

    s_fill_buffer(vertex_attribs, 4);

    if(tx.d.mask_texture_id && s_emulate_logic_ops)
    {
        s_update_fb_read_texture(xDst, yDst, tx.w, tx.h);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glActiveTexture(GL_TEXTURE0);

        s_special_program.use_program();
        s_special_program.update_transform(s_transform_matrix.data());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        return;
    }

    s_program.use_program();
    s_program.update_transform(s_transform_matrix.data());

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
}

void RenderGLES::renderTextureScale(double xDst, double yDst, double wDst, double hDst,
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

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {u1, v1}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {u1, v2}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {u2, v1}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {u2, v2}}
    };

    s_fill_buffer(vertex_attribs, 4);

    if(tx.d.mask_texture_id && s_emulate_logic_ops)
    {
        s_update_fb_read_texture(xDst, yDst, tx.w, tx.h);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glActiveTexture(GL_TEXTURE0);

        s_special_program.use_program();
        s_special_program.update_transform(s_transform_matrix.data());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        return;
    }

    s_program.use_program();
    s_program.update_transform(s_transform_matrix.data());

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
}

void RenderGLES::renderTexture(double xDstD, double yDstD, double wDstD, double hDstD,
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

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {u1, v1}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {u1, v2}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {u2, v1}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {u2, v2}}
    };

    s_fill_buffer(vertex_attribs, 4);

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    if(tx.d.mask_texture_id && s_emulate_logic_ops)
    {
        s_update_fb_read_texture(xDst, yDst, tx.w, tx.h);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glActiveTexture(GL_TEXTURE0);

        s_special_program.use_program();
        s_special_program.update_transform(s_transform_matrix.data());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        return;
    }

    s_program.use_program();
    s_program.update_transform(s_transform_matrix.data());

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
}

void RenderGLES::renderTextureFL(double xDstD, double yDstD, double wDstD, double hDstD,
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

    // glPushMatrix();

    // glTranslatef(xDst + cx, yDst + cy, 0);
    // glRotatef(rotateAngle, 0, 0, 1);

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

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {u1, v1}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {u1, v2}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {u2, v1}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {u2, v2}}
    };

    s_fill_buffer(vertex_attribs, 4);

    if(tx.d.mask_texture_id && s_emulate_logic_ops)
    {
        s_update_fb_read_texture(xDst, yDst, tx.w, tx.h);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glActiveTexture(GL_TEXTURE0);

        s_special_program.use_program();
        s_special_program.update_transform(s_transform_matrix.data());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        return;
    }

    s_program.use_program();
    s_program.update_transform(s_transform_matrix.data());

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }

    // glPopMatrix();
}

void RenderGLES::renderTexture(float xDst, float yDst,
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

    const Vertex_t vertex_attribs[] =
    {
        {{x1, y1, 0}, {red, green, blue, alpha}, {u1, v1}},
        {{x1, y2, 0}, {red, green, blue, alpha}, {u1, v2}},
        {{x2, y1, 0}, {red, green, blue, alpha}, {u2, v1}},
        {{x2, y2, 0}, {red, green, blue, alpha}, {u2, v2}}
    };

    s_fill_buffer(vertex_attribs, 4);

    if(tx.d.mask_texture_id && s_emulate_logic_ops)
    {
        s_update_fb_read_texture(xDst, yDst, tx.w, tx.h);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);
        glActiveTexture(GL_TEXTURE0);

        s_special_program.use_program();
        s_special_program.update_transform(s_transform_matrix.data());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        return;
    }

    s_program.use_program();
    s_program.update_transform(s_transform_matrix.data());

    if(tx.d.mask_texture_id)
    {
        prepareDrawMask();

        glBindTexture(GL_TEXTURE_2D, tx.d.mask_texture_id);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        prepareDrawImage();
    }

    glBindTexture(GL_TEXTURE_2D, tx.d.texture_id);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(tx.d.mask_texture_id)
    {
        leaveMaskContext();
    }
}

void RenderGLES::getScreenPixels(int x, int y, int w, int h, unsigned char *pixels)
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

void RenderGLES::getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels)
{
    glFlush();

    int phys_x, phys_y, phys_w, phys_h;

    GLint prev_fb = -1;

    if(s_game_texture_fb && s_game_texture)
    {
        phys_x = x;
        phys_y = y;
        phys_w = w;
        phys_h = h;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fb);
        glBindFramebuffer(GL_FRAMEBUFFER, s_game_texture_fb);
    }
    else
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

    if(prev_fb != -1)
        glBindFramebuffer(GL_FRAMEBUFFER, prev_fb);

    // rescale and move to target
    for(int r = 0; r < h; r++)
    {
        int phys_r_max = phys_h - 1;
        int phys_r_ind = r * phys_h / h;

        // vertical flip from OpenGL to image
        int phys_r = phys_r_max - phys_r_ind;

        for(int c = 0; c < w; c++)
        {
            int phys_c = c * phys_w / w;

            ((uint32_t*) pixels)[r * w + c] = ((uint32_t*) phys_pixels)[phys_r * phys_w + phys_c];
        }
    }

    free(phys_pixels);
}

int RenderGLES::getPixelDataSize(const StdPicture &tx)
{
    if(!tx.d.texture_id)
        return 0;
    return (tx.w * tx.h * 4);
}

void RenderGLES::getPixelData(const StdPicture &tx, unsigned char *pixelData)
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
