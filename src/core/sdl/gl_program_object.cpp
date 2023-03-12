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

#include <SDL2/SDL_opengles2.h>

#include <Logger/logger.h>

#include "core/sdl/gl_program_object.h"

GLuint GLProgramObject::s_last_program = 0;

/*******************************
 *** Static helper functions ***
 *******************************/

GLuint GLProgramObject::s_compile_shader(GLenum type, const char* src)
{
    // Create the shader object
    GLuint shader = glCreateShader(type);

    if(!shader)
    {
        pLogDebug("GLProgramObject: could not allocate %s shader", type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &src, nullptr);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(!status)
    {
        pLogWarning("GLProgramObject: error compiling %s shader", type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        pLogDebug("shader source:\n%s", src);
    }

    GLint len = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    if(len > 1)
    {
        char* log = (char*)malloc(len);

        if(log)
        {
            glGetShaderInfoLog(shader, len, NULL, log);
            pLogDebug("GLProgramObject: %s shader compilation log:\n%s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);

            free(log);
        }
    }

    if(!status)
    {
        glDeleteShader(shader);

        return 0;
    }

    D_pLogDebug("GLProgramObject: %s shader successfully compiled", type == GL_VERTEX_SHADER ? "vertex" : "fragment");

    return shader;
}


/***********************
 *** Private methods ***
 ***********************/

void GLProgramObject::m_update_transform(const GLfloat* transform, const GLfloat* read_viewport, GLfloat clock)
{
    glUniformMatrix4fv(m_u_transform_loc, 1, GL_FALSE, transform);
    glUniform4fv(m_u_read_viewport_loc, 1, read_viewport);
    glUniform1f(m_u_clock_loc, clock);
}

void GLProgramObject::m_link_program(GLuint vertex_shader, GLuint fragment_shader)
{
    m_program = 0;

    if(!vertex_shader || !fragment_shader)
        return;

    // Create the program object
    GLuint program = glCreateProgram();

    if(!program)
        return;

    // attach the shaders
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    // set the builtin vertex attribute locations
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_texcoord");
    glBindAttribLocation(program, 2, "a_tint");

    // Link the program
    glLinkProgram(program);

    // detach the shaders
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    // Check the link status
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(!status)
        pLogWarning("GLProgramObject: error linking program");

    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    if(len > 1)
    {
        char* log = (char*)malloc(len);

        if(log)
        {
            glGetProgramInfoLog(program, len, NULL, log);
            pLogDebug("GLProgramObject: program link log:\n%s", log);
            free(log);
        }
    }

    if(!status)
    {
        glDeleteProgram(program);
        return;
    }

    m_program = program;

    // load all builtin uniform variable locations

    m_u_transform_loc = glGetUniformLocation(m_program, "u_transform");
    m_u_read_viewport_loc = glGetUniformLocation(m_program, "u_read_viewport");
    m_u_clock_loc = glGetUniformLocation(m_program, "u_clock");
    // m_u_fb_pixsize_loc = glGetUniformLocation(m_program, "u_fb_pixsize");
    // m_u_texture_pixsize_loc = glGetUniformLocation(m_program, "u_texture_pixsize");

    // set sampler texture index to 0 (fixed for all programs)
    GLint u_texture_loc = glGetUniformLocation(m_program, "u_texture");
    GLint u_framebuffer_loc = glGetUniformLocation(m_program, "u_framebuffer");
    GLint u_mask_loc = glGetUniformLocation(m_program, "u_mask");
    // GLint u_previous_pass_loc = glGetUniformLocation(m_program, "u_previous_pass");

    glUseProgram(m_program);
    glUniform1i(u_texture_loc, 0);
    glUniform1i(u_framebuffer_loc, 1);
    glUniform1i(u_mask_loc, 2);
    // glUniform1i(u_previous_pass_loc, 3);

    if(u_framebuffer_loc != -1)
        m_type = Type::read_buffer;
    else
        m_type = Type::translucent;

    D_pLogDebugNA("GLProgramObject: program successfully linked");
}


/**********************
 *** Public methods ***
 **********************/

GLProgramObject::GLProgramObject()
{
    // empty
}

GLProgramObject::GLProgramObject(GLuint vertex_shader, GLuint fragment_shader)
{
    m_link_program(vertex_shader, fragment_shader);
}

GLProgramObject::GLProgramObject(const char* vertex_src, const char* fragment_src)
{
    GLuint vertex_shader = s_compile_shader(GL_VERTEX_SHADER, vertex_src);

    if(!vertex_shader)
        return;

    GLuint fragment_shader = s_compile_shader(GL_FRAGMENT_SHADER, fragment_src);

    if(!fragment_shader)
        return;

    m_link_program(vertex_shader, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

GLProgramObject::~GLProgramObject()
{
    reset();
}

const GLProgramObject& GLProgramObject::operator=(GLProgramObject&& other)
{
    if(inited())
        reset();

    m_program = other.m_program;

    m_u_transform_loc = other.m_u_transform_loc;
    m_u_read_viewport_loc = other.m_u_read_viewport_loc;
    m_u_clock_loc = other.m_u_clock_loc;

    m_transform_tick = other.m_transform_tick;
    m_type = other.m_type;

    // prevent erasure
    other.m_program = 0;

    return *this;
}

void GLProgramObject::reset()
{
    if(m_program)
        glDeleteProgram(m_program);

    m_program = 0;

    m_u_custom_loc.clear();
}

/*!
 * \brief Activate program in OpenGL state
 */
void GLProgramObject::use_program()
{
    if(m_program == s_last_program)
        return;

    glUseProgram(m_program);
    s_last_program = m_program;
}

/*!
 * \brief Registers a custom uniform variable in the next available index
 */
void GLProgramObject::register_uniform(const std::string& name)
{
    m_u_custom_loc.push_back(get_uniform_loc(name));
}

/*!
 * \brief Gets location of custom uniform variable by registered index
 */
GLint GLProgramObject::get_uniform_loc(int index)
{
    return m_u_custom_loc[index];
}

/*!
 * \brief Gets location of custom uniform variable by querying GL (slow)
 */
GLint GLProgramObject::get_uniform_loc(const std::string& name)
{
    return glGetUniformLocation(m_program, name.c_str());
}
