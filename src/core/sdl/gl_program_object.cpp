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

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_opengles2.h>

#include <Logger/logger.h>

#include "core/sdl/gl_program_object.h"

/*******************************
 *** Static helper functions ***
 *******************************/

GLuint GLProgramObject::s_compile_shader(GLenum type, const char* src)
{
    // Create the shader object
    GLuint shader = glCreateShader(type);

    if(!shader)
    {
        pLogDebug("Could not allocate shader");
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
        GLint len = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        if(len > 1)
        {
            char* log = (char*)malloc(len);

            if(log)
            {
                glGetShaderInfoLog(shader, len, NULL, log);
                pLogWarning("Error compiling shader: %s", log);
                free(log);
            }
        }

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}


/***********************
 *** Private methods ***
 ***********************/

void GLProgramObject::m_update_tint(const GLfloat* tint)
{
    glUniform4fv(m_u_tint_loc, 1, tint);
}

void GLProgramObject::m_clear_tint()
{
    glUniform4f(m_u_tint_loc, 1, 1, 1, 1);
}

void GLProgramObject::m_update_transform(const GLfloat* value)
{
    glUniformMatrix4fv(m_u_transform_loc, 1, GL_FALSE, value);
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

    // Link the program
    glLinkProgram(program);

    // detach the shaders
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    // Check the link status
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(!status)
    {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

        if(len > 1)
        {
            char* log = (char*)malloc(len);

            if(log)
            {
                glGetProgramInfoLog(program, len, NULL, log);
                pLogDebug("Error linking program: %s", log);
                free(log);
            }
        }

        glDeleteProgram(program);
        return;
    }

    m_program = program;

    // load all builtin uniform variable locations

    m_u_transform_loc = glGetUniformLocation(m_program, "u_transform");
    m_u_tint_loc = glGetUniformLocation(m_program, "u_tint");

    // set sampler texture index to 0 (fixed for all programs)
    GLint u_texture_loc = glGetUniformLocation(m_program, "u_texture");

    GLint a_position_loc = glGetAttribLocation(m_program, "a_position");
    GLint a_texcoord_loc = glGetAttribLocation(m_program, "a_texcoord");

    pLogDebug("Linked with these locations: %d %d %d %d %d",
        (int)a_position_loc, (int)a_texcoord_loc, (int)u_texture_loc,
        (int)m_u_transform_loc, (int)m_u_tint_loc);

    glUseProgram(m_program);
    glUniform1i(u_texture_loc, 0);
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
    if(m_program)
        glDeleteProgram(m_program);

    m_program = 0;
}

const GLProgramObject& GLProgramObject::operator=(GLProgramObject&& other)
{
    if(inited())
        GLProgramObject::~GLProgramObject();

    m_program = other.m_program;

    m_u_transform_loc = other.m_u_transform_loc;
    m_u_tint_loc = other.m_u_tint_loc;

    m_transform_dirty = other.m_transform_dirty;
    m_tint_applied = other.m_tint_applied;

    // prevent erasure
    other.m_program = 0;

    return *this;
}

/*!
 * \brief Activate program in OpenGL state
 */
void GLProgramObject::use_program()
{
    glUseProgram(m_program);
}
