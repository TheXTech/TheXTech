/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_version.h>

#include <Logger/logger.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "globals.h"
#include "video.h"

#include "core/render.h"
#include "core/opengl/render_gl.h"
#include "core/opengl/gl_shader_translator.h"

constexpr bool s_enable_debug_output = true;


#ifdef RENDERGL_LOAD_ES3_SYMBOLS

#include <EGL/egl.h>

GL_APICALL void (* GL_APIENTRY glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
GL_APICALL void (* GL_APIENTRY glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

void load_gles3_symbols()
{
    #define FIND_PROC(s) s = reinterpret_cast<decltype(s)>(eglGetProcAddress(#s))

    FIND_PROC(glBindBufferBase);
    FIND_PROC(glBlitFramebuffer);

    #undef FIND_PROC
}

#endif // #ifdef RENDERGL_LOAD_ES3_SYMBOLS


#ifdef RENDERGL_HAS_DEBUG

static std::map<GLuint, uint64_t> s_gl_message_counts;

static void APIENTRY s_HandleGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
    auto& count = s_gl_message_counts[id];

    if(count >= 3)
        return;

    s_gl_message_counts[id]++;

    auto log_call = pLogDebug;
    if(type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || severity == GL_DEBUG_SEVERITY_HIGH)
        log_call = pLogWarning;
    else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || type == GL_DEBUG_TYPE_PORTABILITY || type == GL_DEBUG_TYPE_PERFORMANCE || severity == GL_DEBUG_SEVERITY_MEDIUM)
        log_call = pLogInfo;

    UNUSED(source);
    UNUSED(id);
    UNUSED(length);
    UNUSED(userParam);

    log_call("Render GL: got debug message \"%s\"", message);

    if(count == 3)
        log_call("Render GL: (Ignoring future debug messages of this type.)");
}

#endif

void RenderGL::try_init_gl(SDL_GLContext& context, SDL_Window* window, GLint profile, GLint majver, GLint minver, RenderMode_t mode)
{
    // context already initialized
    if(context)
        return;

    pLogInfo("Render GL: attempting to create OpenGL %s %d.%d+ context...", get_profile_name(profile), majver, minver);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majver);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minver);
    context = SDL_GL_CreateContext(window);

    if(context)
        g_videoSettings.renderModeObtained = mode;
    else
        pLogInfo("Render GL: context creation failed.");
}

bool RenderGL::initOpenGL(const CmdLineSetup_t &setup)
{
    SDL_version compiled, linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    pLogInfo("Render GL: compiled for SDL %d.%d.%d, running with SDL %d.%d.%d",
        compiled.major, compiled.minor, compiled.patch,
        linked.major, linked.minor, linked.patch);

    m_gContext = nullptr;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // user request sequence

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
    if(setup.renderType == RENDER_ACCELERATED_OPENGL)
    {
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 3, 3, RENDER_ACCELERATED_OPENGL);
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_CORE, 3, 3, RENDER_ACCELERATED_OPENGL);
    }
#endif

#ifdef THEXTECH_BUILD_GL_ES_MODERN
    if(setup.renderType == RENDER_ACCELERATED_OPENGL_ES)
    {
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 3, 0, RENDER_ACCELERATED_OPENGL_ES);
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 2, 0, RENDER_ACCELERATED_OPENGL_ES);
    }
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
    if(setup.renderType == RENDER_ACCELERATED_OPENGL_LEGACY)
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 1, 1, RENDER_ACCELERATED_OPENGL_LEGACY);
#endif

#ifdef THEXTECH_BUILD_GL_ES_LEGACY
    if(setup.renderType == RENDER_ACCELERATED_OPENGL_ES_LEGACY)
        try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 1, 1, RENDER_ACCELERATED_OPENGL_ES_LEGACY);
#endif

    // default fallback sequence

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 3, 3, RENDER_ACCELERATED_OPENGL);
#endif

#ifdef THEXTECH_BUILD_GL_ES_MODERN
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 3, 0, RENDER_ACCELERATED_OPENGL_ES);
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 2, 0, RENDER_ACCELERATED_OPENGL_ES);
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_CORE, 3, 3, RENDER_ACCELERATED_OPENGL);
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 1, 1, RENDER_ACCELERATED_OPENGL_LEGACY);
#endif

#ifdef THEXTECH_BUILD_GL_ES_LEGACY
    try_init_gl(m_gContext, m_window, SDL_GL_CONTEXT_PROFILE_ES, 1, 1, RENDER_ACCELERATED_OPENGL_ES_LEGACY);
#endif

    if(!m_gContext)
    {
        pLogCritical("Render GL: unable to create any OpenGL context, falling back to SDL.");
        return false;
    }

#if (defined(_WIN32) || defined(_WIN64)) && defined(THEXTECH_BUILD_GL_DESKTOP_MODERN)
    glewInit();
#endif

#if defined(__SWITCH__) && defined(THEXTECH_BUILD_GL_DESKTOP_MODERN)
    if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        pLogCritical("Render GL: unable to load GLAD, falling back to SDL.");
        return false;
    }
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

    pLogInfo("Render GL: successfully initialized OpenGL %d.%d (Profile %s)", m_gl_majver, m_gl_minver, get_profile_name(m_gl_profile));
    pLogInfo("OpenGL version: %s", gl_ver_string);
    pLogInfo("OpenGL renderer: %s", glGetString(GL_RENDERER));
#ifdef RENDERGL_HAS_SHADERS
    if(m_gl_majver >= 2)
        pLogInfo("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    // pLogDebug("OpenGL extensions: %s", glGetString(GL_EXTENSIONS));

    GLint depth = 16;

#if defined(THEXTECH_BUILD_GL_ES_MODERN) || defined(THEXTECH_BUILD_GL_ES_LEGACY) || defined(THEXTECH_BUILD_GL_DESKTOP_LEGACY)
    if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_CORE)
    {
        GLint r, g, b;
        glGetIntegerv(GL_RED_BITS, &r);
        glGetIntegerv(GL_GREEN_BITS, &g);
        glGetIntegerv(GL_BLUE_BITS, &b);
        glGetIntegerv(GL_DEPTH_BITS, &depth);

        pLogInfo("OpenGL video mode: R%d G%d B%d with %d-bit depth buffer", r, g, b, depth);
    }
#endif

    bool gles1 = (m_gl_profile == SDL_GL_CONTEXT_PROFILE_ES && m_gl_majver == 1);

    // depth buffering broken on some ancient Android phones under OpenGL ES 1
    if(depth >= 16 && !gles1)
        m_use_depth_buffer = true;

    // Check capabilities
    GLint maxTextureSize = 256;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    pLogInfo("RenderGL: max texture size %d", (int)maxTextureSize);

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
    bool fbo_supported = (m_gl_majver >= 3) || (m_gl_profile == SDL_GL_CONTEXT_PROFILE_ES && m_gl_majver >= 2);

    if(fbo_supported && setup.renderType != RENDER_ACCELERATED_OPENGL_LEGACY)
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
        pLogInfo("Render GL: GL error %d occurred in early init process, falling back to SDL.", err);
        return false;
    }

#ifdef RENDERGL_LOAD_ES3_SYMBOLS
    if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_ES && m_gl_majver >= 3)
        load_gles3_symbols();
#endif

    return true;
}

bool RenderGL::initDebug()
{
    if(s_enable_debug_output)
    {
#ifdef RENDERGL_HAS_DEBUG
        if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES)
        {
            pLogInfo("Enabling GL debug output...");

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

    GLProgramObject::s_reset_supported_versions();

    if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES)
    {
        XTechShaderTranslator::EnsureInit();
        XTechShaderTranslator::SetOpenGLVersion(m_gl_majver, m_gl_minver);
    }

    m_standard_program = GLProgramObject(
        s_es2_standard_vert_src,
        s_es2_standard_frag_src
    );

    if(!m_standard_program.inited())
    {
        m_standard_program.reset();

        m_use_shaders = false;

        pLogInfo("Render GL: Failed to compile standard GLSL ES 1.00 shader.");

        if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY || m_gl_majver < 2)
        {
            pLogInfo("Render GL: Falling back to fixed-function rendering.");
            return true;
        }

        pLogInfo("Render GL: Falling back to SDL.");
        return false;
    }

    std::vector<char> logic_contents;
    dumpFullFile(logic_contents, (AppPath + "/logic.frag").c_str());
    if(!logic_contents.empty())
        logic_contents.push_back('\0');

    // assess es3 compatibility using the bitmask shader
    m_bitmask_program = GLProgramObject(
        s_es3_advanced_vert_src,
        s_es3_bitmask_frag_src
    );

    if(m_bitmask_program.inited())
        m_has_es3_shaders = true;
    else
    {
        m_bitmask_program = GLProgramObject(
            s_es2_advanced_vert_src,
            logic_contents.empty() ? s_es2_bitmask_frag_src : logic_contents.data()
        );
    }

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

    std::vector<char> output_contents;
    dumpFullFile(output_contents, (AppPath + "/output.frag").c_str());
    if(!output_contents.empty())
        output_contents.push_back('\0');

    if(!output_contents.empty())
        pLogDebug("Loading screen fragment shader from output.frag...");

    m_output_program = GLProgramObject(
        s_es2_standard_vert_src,
        output_contents.empty() ? s_es2_standard_frag_src : output_contents.data()
    );

    // initialize the lighting program
    if(m_has_es3_shaders)
    {
        // code to support a jump flooding algorithm to calculate a distance field
        m_distance_field_1_program = GLProgramObject(
            s_es3_advanced_vert_src,
            s_es3_distance_field_1_frag_src
        );

        m_distance_field_2_program = GLProgramObject(
            s_es3_advanced_vert_src,
            s_es3_distance_field_2_frag_src
        );

        // will assume that this is stored in the GLProgramObject's uniform slot 0 later
        StdPicture_Sub null_sub;
        m_distance_field_2_program.register_uniform("u_step_size", null_sub);

        dumpFullFile(output_contents, (AppPath + "/graphics/shaders/lighting.frag").c_str());
        if(!output_contents.empty())
            output_contents.push_back('\0');

        m_lighting_calc_program = GLProgramObject(
            s_es3_advanced_vert_src,
            output_contents.empty() ? s_es3_lighting_calc_frag_src : output_contents.data()
        );

        if(m_lighting_calc_program.inited())
        {
            // initialize uniform buffer (if supported)
            glGenBuffers(1, &m_light_ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, m_light_ubo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBuffer), &m_light_queue, GL_STREAM_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_light_ubo);

            // initialize lighting apply program
            m_lighting_apply_program = GLProgramObject(
                s_es2_advanced_vert_src,
                s_es2_lighting_apply_frag_src
            );
        }
    }

    return true;
#else // #ifdef RENDERGL_HAS_SHADERS
    if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY || m_gl_majver < 2)
        return true;

    pLogWarning("Render GL: Built without shader support but OpenGL version requires shaders.");
    return false;
#endif
}

void RenderGL::createFramebuffer(BufferIndex_t buffer)
{
#ifdef RENDERGL_HAS_FBO
    GLenum err;
    while((err = glGetError()))
        pLogInfo("Render GL: GL error %d occurred prior to framebuffer creation", err);

    // (0) cleanup existing framebuffer
    destroyFramebuffer(buffer);

    if((m_gl_majver < 3 || !m_game_depth_texture) && buffer == BUFFER_DEPTH_READ)
    {
        pLogInfo("Render GL: cannot create depth read buffer on OpenGL < 3.0 / OpenGL ES < 3.0");
        return;
    }

    // (1) allocate texture
    glGenTextures(1, &m_buffer_texture[buffer]);

    if(!m_buffer_texture[buffer])
    {
        pLogWarning("Render GL: Failed to allocate buffer texture %d", buffer);
        return;
    }

    // allocate texture memory
    glBindTexture(GL_TEXTURE_2D, m_buffer_texture[buffer]);

    float scale_factor = (buffer == BUFFER_LIGHTING) ? m_lighting_scale_factor : m_render_scale_factor;

    if(buffer == BUFFER_DEPTH_READ)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
            XRender::TargetW * scale_factor, XRender::TargetH * scale_factor,
            0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
    }
    else if(buffer == BUFFER_LIGHTING)
    {
#ifdef __EMSCRIPTEN__
        // emscripten requires an extension to render to floats
        const EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_get_current_context();
        const EM_BOOL got_float_buffer = emscripten_webgl_enable_extension(context, "EXT_color_buffer_float");
        if(got_float_buffer)
        {
            pLogInfo("Attempting to initialize lighting buffer with RGBA16F using EXT_color_buffer_float...");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                XRender::TargetW * scale_factor, XRender::TargetH * scale_factor,
                0, GL_RGBA, GL_FLOAT, nullptr);
        }
        else
        {
            pLogInfo("Initializing lighting buffer with RGB8...");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                XRender::TargetW * scale_factor, XRender::TargetH * scale_factor,
                0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        }
#else // #ifdef __EMSCRIPTEN__
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
            XRender::TargetW * scale_factor, XRender::TargetH * scale_factor,
            0, GL_RGB, GL_FLOAT, nullptr);
#endif
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            XRender::TargetW * scale_factor, XRender::TargetH * scale_factor,
            0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

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

    // (2) allocate depth buffer (game texture only)
    if(buffer == BUFFER_GAME)
    {
        if(m_gl_majver >= 3)
        {
            glGenTextures(1, &m_game_depth_texture);

            // allocate texture memory
            glBindTexture(GL_TEXTURE_2D, m_game_depth_texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
                XRender::TargetW * scale_factor, XRender::TargetH * scale_factor,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            glGenRenderbuffers(1, &m_game_depth_rb);
            glBindRenderbuffer(GL_RENDERBUFFER, m_game_depth_rb);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, XRender::TargetW * scale_factor, XRender::TargetH * scale_factor);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        err = glGetError();
        if(err)
        {
            pLogWarning("Render GL: Failed to allocate game's depth buffer texture (GL error code %d)", (int)err);
            destroyFramebuffer(buffer);
            return;
        }

        // even if window doesn't have depth buffer, this counts!
        m_use_depth_buffer = true;
    }

    // (3) allocate framebuffer (required for game texture, otherwise optional with a fallback in framebufferCopy)
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
    glFramebufferTexture2D(GL_FRAMEBUFFER, (buffer == BUFFER_DEPTH_READ) ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_buffer_texture[buffer], 0);

    if(buffer == BUFFER_GAME || buffer == BUFFER_INT_PASS_1 || buffer == BUFFER_INT_PASS_2)
    {
        if(m_game_depth_texture)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_game_depth_texture, 0);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_game_depth_rb);
    }

    // (4) status checking
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

    if(buffer == BUFFER_GAME && m_game_depth_texture)
    {
        glDeleteTextures(1, &m_game_depth_texture);
        m_game_depth_texture = 0;
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
#ifdef RENDERGL_HAS_FBO
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

        // only use FB read texture with either user shaders or logic-op emulation
        const bool emulate_logic_op = !m_use_logicop && m_use_shaders;

        // FIXME: m_use_shaders should check for user shader support here
        if(i == BUFFER_GAME && !emulate_logic_op && !m_use_shaders)
            break;

        // can't use multipass rendering buffers without user shaders
        // FIXME: m_use_shaders should check for user shader support here
        if(i == BUFFER_FB_READ && !m_use_shaders)
            break;
    }

    // bind texture unit 1 to the framebuffer read texture
    if(m_buffer_texture[BUFFER_FB_READ])
    {
        glActiveTexture(TEXTURE_UNIT_FB_READ);
        glBindTexture(GL_TEXTURE_2D, m_buffer_texture[BUFFER_FB_READ]);
    }

    // initialize null lighting texture and bind to texture unit 5
    glGenTextures(1, &m_null_light_texture);
    glActiveTexture(TEXTURE_UNIT_LIGHT_READ);
    glBindTexture(GL_TEXTURE_2D, m_null_light_texture);
    const GLubyte white_texel[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        1, 1,
        0, GL_RGB, GL_UNSIGNED_BYTE, white_texel);

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

    glActiveTexture(TEXTURE_UNIT_IMAGE);

#endif // #ifdef RENDERGL_HAS_FBO

    return true;
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
    if(m_use_depth_buffer)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);
        glDepthMask(GL_TRUE);
    }

    // alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // clear color and depth
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#if (defined(THEXTECH_BUILD_GL_DESKTOP_MODERN) || defined(THEXTECH_BUILD_GL_DESKTOP_LEGACY)) && (defined(THEXTECH_BUILD_GL_ES_MODERN) || defined(THEXTECH_BUILD_GL_ES_LEGACY))
    if(m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES)
    {
        glClearDepth(-1.0f);
        glDepthRange(-1.0f, 1.0f);
    }
    else
    {
        glClearDepthf(-1.0f);
        glDepthRangef(-1.0f, 1.0f);
    }
#elif defined(THEXTECH_BUILD_GL_ES_MODERN) || defined(THEXTECH_BUILD_GL_ES_LEGACY)
    glClearDepthf(-1.0f);
    glDepthRangef(-1.0f, 1.0f);
#else
    glClearDepth(-1.0f);
    glDepthRange(-1.0f, 1.0f);
#endif

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
#    ifdef RENDERGL_HAS_STREAM_DRAW
            const auto draw_mode = (m_gl_majver == 1 ? GL_DYNAMIC_DRAW : GL_STREAM_DRAW);
#    else
            const auto draw_mode = GL_DYNAMIC_DRAW;
#    endif
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_t) * 4, nullptr, draw_mode);
            m_vertex_buffer_size[i] = sizeof(Vertex_t) * 4;
        }
    }
#endif // #ifdef RENDERGL_HAS_VBO

    // add proper error handling

    return true;
}
