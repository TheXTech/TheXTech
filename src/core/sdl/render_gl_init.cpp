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

#include "core/sdl/gl_inc.h"

#include <fstream>

#include <SDL2/SDL_version.h>

#include <Logger/logger.h>

#include "globals.h"
#include "video.h"

#include "core/sdl/render_gl.h"

constexpr bool s_enable_debug_output = true;

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

#ifdef RENDERGL_HAS_DEBUG
static void APIENTRY s_HandleGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
    pLogWarning("Got GL error %s", message);
}
#endif

void RenderGL::try_init_gl(SDL_GLContext& context, SDL_Window* window, GLint profile, GLint majver, GLint minver)
{
    // context already initialized
    if(context)
        return;

    pLogDebug("Render GL: attempting to create OpenGL %s %d.%d+ context...", get_profile_name(profile), majver, minver);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majver);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minver);
    context = SDL_GL_CreateContext(window);

    if(!context)
        pLogDebug("Render GL: context creation failed.");

}

bool RenderGL::initOpenGL(const CmdLineSetup_t &setup)
{
    SDL_version compiled, linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    pLogDebug("Render GL: compiled for SDL %d.%d.%d, running with SDL %d.%d.%d",
        compiled.major, compiled.minor, compiled.patch,
        linked.major, linked.minor, linked.patch);

    m_gContext = nullptr;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    // user request sequence

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
    if(setup.renderType == RENDER_ACCELERATED_OPENGL)
    {
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 3, 3);
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_CORE, 3, 3);
    }
#endif

#ifdef THEXTECH_BUILD_GL_ES_MODERN
    if(setup.renderType == RENDER_ACCELERATED_OPENGL_ES)
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 2, 0);
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
    if(setup.renderType == RENDER_ACCELERATED_OPENGL_LEGACY)
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 1, 1);
#endif

#ifdef THEXTECH_BUILD_GL_ES_LEGACY
    if(setup.renderType == RENDER_ACCELERATED_OPENGL_ES_LEGACY)
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 1, 1);
#endif

    // default fallback sequence

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 3, 3);
#endif

#ifdef THEXTECH_BUILD_GL_ES_MODERN
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 2, 0);
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_CORE, 3, 3);
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 1, 1);
#endif

#ifdef THEXTECH_BUILD_GL_ES_LEGACY
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 1, 1);
#endif

    if(!m_gContext)
    {
        pLogCritical("Render GL: unable to create any OpenGL context, falling back to SDL.");
        return false;
    }

#if (defined(_WIN32) || defined(_WIN64)) && defined(THEXTECH_BUILD_GL_DESKTOP_MODERN)
    glewInit();
#endif

    // Check version
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &m_gl_profile);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &m_gl_majver);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &m_gl_minver);

    // can't trust SDL2
    const GLubyte* gl_ver_string = glGetString(GL_VERSION);
    if(gl_ver_string)
    {
        for(int ver_end = 0; gl_ver_string[ver_end] != '\0'; ver_end++)
        {
            int ver_start = ver_end - 2;

            if(ver_start >= 0
                && gl_ver_string[ver_start] >= '0' && gl_ver_string[ver_start] <= '9'
                && gl_ver_string[ver_end]   >= '0' && gl_ver_string[ver_end]   <= '9')
            {
                m_gl_majver = gl_ver_string[ver_start] - '0';
                m_gl_minver = gl_ver_string[ver_end] - '0';
                break;
            }
        }
    }

    pLogDebug("Render GL: successfully initialized OpenGL %d.%d (Profile %s)", m_gl_majver, m_gl_minver, get_profile_name(m_gl_profile));
    pLogDebug("OpenGL version: %s", gl_ver_string);
    pLogDebug("OpenGL renderer: %s", glGetString(GL_RENDERER));
#ifdef RENDERGL_HAS_SHADERS
    if(m_gl_majver >= 2)
        pLogDebug("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    D_pLogDebug("OpenGL extensions: %s", glGetString(GL_EXTENSIONS));

    // Check capabilities
    GLint maxTextureSize = 256;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    pLogDebug("RenderGL: max texture size %d", (int)maxTextureSize);

    m_maxTextureWidth = maxTextureSize;
    m_maxTextureHeight = maxTextureSize;

#ifdef RENDERGL_HAS_LOGICOP
    if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES || m_gl_majver == 1)
        m_use_logicop = true;
    else
        m_use_logicop = false;
#else
    m_use_logicop = false;
#endif

#ifdef RENDERGL_HAS_SHADERS
    if(m_gl_majver >= 2 && m_gl_profile != SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
        m_use_shaders = true;
    else if(m_gl_majver >= 2 && setup.renderType != RENDER_ACCELERATED_OPENGL_LEGACY)
        m_use_shaders = true;
    else
        m_use_shaders = false;
#else
    m_use_shaders = false;
#endif

#ifdef __EMSCRIPTEN__
    m_client_side_arrays = false;
#else
    if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_CORE)
        m_client_side_arrays = false;
    else
        m_client_side_arrays = true;
#endif // #ifdef __EMSCRIPTEN__

#ifdef RENDERGL_HAS_FBO
    if(m_gl_majver >= 2 && setup.renderType != RENDER_ACCELERATED_OPENGL_LEGACY)
        m_has_fbo = true;
    else
        m_has_fbo = false;
#else
    m_has_fbo = false;
#endif

    // should check for NPOT and BGRA textures

    // setup vSync
    SDL_GL_SetSwapInterval(setup.vSync);

    GLenum err = glGetError();

    if(err)
    {
        pLogDebug("Render GL: GL error %d occurred in early init process, falling back to SDL.", err);
        return false;
    }

    return true;
}

bool RenderGL::initDebug()
{
    if(s_enable_debug_output)
    {
#ifdef RENDERGL_HAS_DEBUG
        if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES)
        {
            pLogDebug("Enabling GL debug output...");

            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(s_HandleGLDebugMessage, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
#endif
    }

    return true;
}

bool RenderGL::initShaders()
{
#ifdef RENDERGL_HAS_SHADERS
    if(!m_use_shaders)
        return true;

    // do something to assess es3 compatibility here

    m_standard_program = GLProgramObject(
        s_es2_standard_vert_src,
        s_es2_standard_frag_src
    );

    if(!m_standard_program.inited())
    {
        m_standard_program.reset();

        m_use_shaders = false;

        pLogWarning("Render GL: Failed to compile standard GLSL ES 1.00 shader.");

        if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY || m_gl_majver < 2)
        {
            pLogDebug("Render GL: Falling back to fixed-function rendering.");
            return true;
        }

        pLogDebug("Render GL: Falling back to SDL.");
        return false;
    }

    std::string logic_contents = s_read_file((AppPath + "/logic.frag").c_str());

    m_bitmask_program = GLProgramObject(
        s_es2_advanced_vert_src,
        logic_contents.empty() ? s_es2_bitmask_frag_src : logic_contents.c_str()
    );

    m_program_rect_filled = GLProgramObject(
        s_es2_standard_vert_src,
        s_es2_rect_filled_frag_src
    );

    m_program_rect_unfilled = GLProgramObject(
        s_es2_standard_vert_src,
        s_es2_rect_unfilled_frag_src
    );

    m_program_circle = GLProgramObject(
        s_es2_standard_vert_src,
        s_es2_circle_frag_src
    );

    m_program_circle_hole = GLProgramObject(
        s_es2_standard_vert_src,
        s_es2_circle_hole_frag_src
    );

    std::string output_contents = s_read_file((AppPath + "/output.frag").c_str());

    if(!output_contents.empty())
        pLogDebug("Loading screen fragment shader from output.frag...");

    m_output_program = GLProgramObject(
        s_es2_standard_vert_src,
        output_contents.empty() ? s_es2_standard_frag_src : output_contents.c_str()
    );

    return true;
#else // #ifdef RENDERGL_HAS_SHADERS
    if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY || m_gl_majver < 2)
        return true;

    pLogDebug("Render GL: Built without shader support but OpenGL version requires shaders.");
    return false;
#endif
}

bool RenderGL::initState()
{
    // alpha test and texturing for legacy versions
#if defined(THEXTECH_BUILD_GL_DESKTOP_LEGACY) || defined(THEXTECH_BUILD_GL_ES_LEGACY)
    if(!m_use_shaders)
    {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL, 1.0f / 32.0f);
    }
#endif

    // depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_TRUE);

    // alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // clear color and depth
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#if (defined(THEXTECH_BUILD_GL_DESKTOP_MODERN) || defined(THEXTECH_BUILD_GL_DESKTOP_LEGACY)) && (defined(THEXTECH_BUILD_GL_ES_MODERN) || defined(THEXTECH_BUILD_GL_ES_LEGACY))
    if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES)
        glClearDepth(0.0f);
    else
        glClearDepthf(0.0f);
#elif defined(THEXTECH_BUILD_GL_ES_MODERN) || defined(THEXTECH_BUILD_GL_ES_LEGACY)
    glClearDepthf(0.0f);
#else
    glClearDepth(0.0f);
#endif

    return true;
}

void RenderGL::createFramebuffer(BufferIndex_t buffer)
{
#ifdef RENDERGL_HAS_FBO
    GLenum err;
    while((err = glGetError()))
        pLogDebug("Render GL: GL error %d occurred prior to framebuffer creation", err);

    // (0) cleanup existing framebuffer
    destroyFramebuffer(buffer);

    // (1) allocate texture
    glGenTextures(1, &m_buffer_texture[buffer]);

    if(!m_buffer_texture[buffer])
    {
        pLogWarning("Render GL: Failed to allocate buffer texture %d", buffer);
        return;
    }

    // allocate texture memory
    glBindTexture(GL_TEXTURE_2D, m_buffer_texture[buffer]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        ScreenW, ScreenH,
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    err = glGetError();
    if(err)
    {
        pLogWarning("Render GL: Failed to allocate buffer texture %d (GL error code %d)", (int)buffer, (int)err);
        destroyFramebuffer(buffer);
        return;
    }

    // (2) allocate render buffer (game texture only)
    if(buffer == BUFFER_GAME)
    {
        glGenRenderbuffers(1, &m_game_depth_rb);
        glBindRenderbuffer(GL_RENDERBUFFER, m_game_depth_rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ScreenW, ScreenH);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        err = glGetError();
        if(err)
        {
            pLogWarning("Render GL: Failed to allocate game's depth buffer texture (GL error code %d)", (int)err);
            destroyFramebuffer(buffer);
            return;
        }
    }

    // (3) allocate framebuffer (required for game texture, otherwise optional)
    glGenFramebuffers(1, &m_buffer_fb[buffer]);
    if(!m_buffer_fb[buffer])
    {
        if(buffer == BUFFER_GAME)
        {
            pLogWarning("Render GL: Failed to allocate game's render-to-texture framebuffer");
            destroyFramebuffer(buffer);
            return;
        }
        else
        {
            pLogDebug("Render GL: Failed to allocate framebuffer %d, falling back to texture buffer (minor)");
            return;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[buffer]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_buffer_texture[buffer], 0);

    if(buffer == BUFFER_GAME)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_game_depth_rb);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        if(buffer == BUFFER_GAME)
        {
            pLogWarning("Render GL: Failed to allocate game's render-to-texture framebuffer");
            destroyFramebuffer(buffer);
            return;
        }
        else
        {
            pLogDebug("Render GL: Failed to allocate framebuffer %d, falling back to texture buffer (minor)", (int)buffer);
            glDeleteFramebuffers(1, &m_buffer_fb[buffer]);
            m_buffer_fb[buffer] = 0;
            return;
        }
    }
#else // #ifdef RENDERGL_HAS_FBO
    UNUSED(buffer);
#endif
}

void RenderGL::destroyFramebuffer(BufferIndex_t buffer)
{
#ifdef RENDERGL_HAS_FBO
    if(m_buffer_fb[buffer])
    {
        glDeleteFramebuffers(1, &m_buffer_fb[buffer]);
        m_buffer_fb[buffer] = 0;
    }

    if(buffer == BUFFER_GAME && m_game_depth_rb)
    {
        glDeleteRenderbuffers(1, &m_game_depth_rb);
        m_game_depth_rb = 0;
    }

    if(m_buffer_texture[buffer])
    {
        glDeleteTextures(1, &m_buffer_texture[buffer]);
        m_buffer_texture[buffer] = 0;
    }
#else // #ifdef RENDERGL_HAS_FBO
    UNUSED(buffer);
#endif
}

bool RenderGL::initFramebuffers()
{
    if(!m_has_fbo)
        return true;

    // try to allocate each texture / framebuffer that would be useful
    for(int i = BUFFER_GAME; i < BUFFER_MAX; i++)
    {
        createFramebuffer((BufferIndex_t)i);

        // break if we have no render-to-texture at all
        if(i == BUFFER_GAME && !m_buffer_fb[i])
            break;

        // break if texture allocation failed
        if(!m_buffer_texture[i])
            break;

        // can't use other buffers without shaders
        if(i == BUFFER_GAME && !m_use_shaders)
            break;

        // can't use multipass rendering buffers without user shaders
        // FIXME: check for user shader support here
        if(i == BUFFER_FB_READ && !m_use_shaders)
            break;
    }

    // deallocate BUFFER_INIT_PASS if BUFFER_PREV_PASS didn't allocate
    if(!m_buffer_texture[BUFFER_PREV_PASS] && m_buffer_texture[BUFFER_INIT_PASS])
        destroyFramebuffer(BUFFER_INIT_PASS);

    // bind texture unit 1 to the framebuffer read texture
    if(m_buffer_texture[BUFFER_FB_READ])
    {
#ifdef RENDERGL_HAS_FBO
        glActiveTexture(TEXTURE_UNIT_FB_READ);
        glBindTexture(GL_TEXTURE_2D, m_buffer_texture[BUFFER_FB_READ]);
        glActiveTexture(TEXTURE_UNIT_IMAGE);
#endif
    }

    return true;
}

bool RenderGL::initVertexArrays()
{
#ifdef RENDERGL_HAS_VAO
    if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_CORE)
    {
        glGenVertexArrays(1, &m_glcore_vao);
        if(!m_glcore_vao)
        {
            pLogWarning("Render GL: Failed to allocate vertex array object, required for OpenGL Core profile");
            return false;
        }
        glBindVertexArray(m_glcore_vao);
    }
#endif // #ifndef RENDERGL_HAS_VAO

    if(m_use_shaders)
    {
#ifdef RENDERGL_HAS_SHADERS
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
#endif
    }
    else
    {
#ifdef RENDERGL_HAS_FIXED_FUNCTION
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
#endif
    }

    // initialize vertex buffers
#ifdef RENDERGL_HAS_VBO
    if(!m_client_side_arrays)
    {
        glGenBuffers(s_num_vertex_buffers, m_vertex_buffer);

        for(int i = 0; i < s_num_vertex_buffers; i++)
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer[i]);
#ifdef RENDERGL_HAS_STREAM_DRAW
            const auto draw_mode = (m_gl_majver == 1 ? GL_DYNAMIC_DRAW : GL_STREAM_DRAW);
#else
            const auto draw_mode = GL_DYNAMIC_DRAW;
#endif
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_t) * 4, nullptr, draw_mode);
            m_vertex_buffer_size[i] = sizeof(Vertex_t) * 4;
        }
    }
#endif // #ifdef RENDERGL_HAS_VBO

    // add proper error handling

    return true;
}
