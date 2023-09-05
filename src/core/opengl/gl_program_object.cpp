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

#include <algorithm>

#include <Logger/logger.h>

#include "sdl_proxy/sdl_assert.h"

#include "core/opengl/gl_inc.h"
#include "core/opengl/gl_program_object.h"
#include "core/opengl/render_gl.h"
#include "std_picture.h"


GLuint GLProgramObject::s_last_program = 0;

bool UniformValue_t::operator==(const UniformValue_t& o) const
{
    if(type != o.type || width != o.width)
        return false;

    if(width < 1 || width > 4)
        return false;

    if(type == XT_GL_FLOAT)
    {
        for(uint8_t i = 0; i < width; i++)
        {
            if(values.f[i] != o.values.f[i])
                return false;
        }

        return true;
    }
    else if(type == XT_GL_INT)
    {
        for(uint8_t i = 0; i < width; i++)
        {
            if(values.i[i] != o.values.i[i])
                return false;
        }

        return true;
    }

    return false;
}

#ifdef RENDERGL_HAS_SHADERS

void s_execute_assignment(GLint uniform_loc, const UniformValue_t& value)
{
    if(uniform_loc == -1)
        return;

    if(value.type == XT_GL_FLOAT)
    {
        if(value.width == 1)
            glUniform1f(uniform_loc, value.values.f[0]);
        else if(value.width == 2)
            glUniform2f(uniform_loc, value.values.f[0], value.values.f[1]);
        else if(value.width == 3)
            glUniform3f(uniform_loc, value.values.f[0], value.values.f[1], value.values.f[2]);
        else if(value.width == 4)
            glUniform4f(uniform_loc, value.values.f[0], value.values.f[1], value.values.f[2], value.values.f[3]);
    }
    else if(value.type == XT_GL_INT)
    {
        if(value.width == 1)
            glUniform1i(uniform_loc, value.values.i[0]);
        else if(value.width == 2)
            glUniform2i(uniform_loc, value.values.i[0], value.values.i[1]);
        else if(value.width == 3)
            glUniform3i(uniform_loc, value.values.i[0], value.values.i[1], value.values.i[2]);
        else if(value.width == 4)
            glUniform4i(uniform_loc, value.values.i[0], value.values.i[1], value.values.i[2], value.values.i[3]);
    }
}

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
    if(m_u_transform_loc != -1)
        glUniformMatrix4fv(m_u_transform_loc, 1, GL_FALSE, transform);
    if(m_u_read_viewport_loc != -1)
        glUniform4fv(m_u_read_viewport_loc, 1, read_viewport);
    if(m_u_clock_loc != -1)
        glUniform1f(m_u_clock_loc, clock);
}

void GLProgramObject::m_link_program(GLuint vertex_shader, GLuint fragment_shader, bool particle_system)
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
    if(!particle_system)
    {
        glBindAttribLocation(program, 0, "a_position");
        glBindAttribLocation(program, 1, "a_texcoord");
        glBindAttribLocation(program, 2, "a_tint");
    }
    else
    {
        glBindAttribLocation(program, 0, "a_index");
        glBindAttribLocation(program, 1, "a_texcoord");
        glBindAttribLocation(program, 2, "a_position");
        glBindAttribLocation(program, 3, "a_spawn_time");
        glBindAttribLocation(program, 4, "a_attribs");
    }

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
    GLint u_previous_pass_loc = glGetUniformLocation(m_program, "u_previous_pass");
    GLint u_depth_buffer_loc = glGetUniformLocation(m_program, "u_depth_buffer");
    GLint u_light_buffer_loc = glGetUniformLocation(m_program, "u_light_buffer");

    glUseProgram(m_program);
    glUniform1i(u_texture_loc,       TEXTURE_UNIT_IMAGE      - GL_TEXTURE0);
    glUniform1i(u_framebuffer_loc,   TEXTURE_UNIT_FB_READ    - GL_TEXTURE0);
    glUniform1i(u_mask_loc,          TEXTURE_UNIT_MASK       - GL_TEXTURE0);
    glUniform1i(u_previous_pass_loc, TEXTURE_UNIT_PREV_PASS  - GL_TEXTURE0);
    glUniform1i(u_depth_buffer_loc,  TEXTURE_UNIT_DEPTH_READ - GL_TEXTURE0);
    glUniform1i(u_light_buffer_loc,  TEXTURE_UNIT_LIGHT_READ - GL_TEXTURE0);

    if(u_previous_pass_loc != -1)
        m_flags |= Flags::multipass;
    if(u_framebuffer_loc != -1)
        m_flags |= Flags::read_buffer;
    if(u_depth_buffer_loc != -1)
        m_flags |= Flags::read_depth;
    if(u_light_buffer_loc != -1)
        m_flags |= Flags::read_light;
    if(particle_system)
        m_flags |= Flags::particles;

    D_pLogDebugNA("GLProgramObject: program successfully linked");
}

void GLProgramObject::m_activate_uniform_step(uint16_t step)
{
    // enqueue a clear next time a uniform is assigned
    m_enqueue_clear_uniform_steps = true;

    // safeties
    SDL_assert_release(m_final_uniform_step <= m_uniform_steps.size());
    SDL_assert_release(m_gl_uniform_step <= m_uniform_steps.size());

    // is step after m_final_step?
    if(step > m_final_uniform_step)
    {
        step = m_final_uniform_step;
        pLogWarning("GLProgramObject: requested to activate uniform state that has not yet been set");
    }

    // track which uniforms are now matching the requested state
    static std::vector<bool> updated;
    updated.clear();
    updated.resize(m_u_custom_loc.size());

    // rewind
    if(step < m_gl_uniform_step)
    {
        // rewind each step between the requested step and the current GL step
        // starts with step following REQUESTED STATE to avoid double-assignments
        for(uint16_t step_index = step; step_index < m_gl_uniform_step; step_index++)
        {
            const UniformAssignment_t& assignment = m_uniform_steps[step_index];

            // only undo the assignment CLOSEST to the requested state
            if(updated[assignment.index])
                continue;

            updated[assignment.index] = true;

            s_execute_assignment(get_uniform_loc(assignment.index), assignment.pre);
        }

        m_gl_uniform_step = step;
    }
    // fast-forward
    else if(step > m_gl_uniform_step)
    {
        // execute each step between the requested step and the current GL step
        // starts with step before REQUESTED STATE to avoid double-assignments
        // (includes m_gl_uniform_step because m_gl_uniform_step represents state before step, moved dec into loop to avoid unsigned int >= 0)
        for(uint16_t step_index = step; step_index > m_gl_uniform_step;)
        {
            step_index--;

            const UniformAssignment_t& assignment = m_uniform_steps[step_index];

            // only undo the assignment CLOSEST to the requested state
            if(updated[assignment.index])
                continue;

            updated[assignment.index] = true;

            s_execute_assignment(get_uniform_loc(assignment.index), assignment.post);
        }

        m_gl_uniform_step = step;
    }
}

void GLProgramObject::m_clear_uniform_steps()
{
    // activate as the current GL program object
    use_program();

    // flush GL state to the most recent step
    activate_uniform_step(m_final_uniform_step);

    // clear set of uniform steps
    m_uniform_steps.clear();

    // our current uniform step precedes the first assignment of the frame
    m_gl_uniform_step = 0;
    m_final_uniform_step = 0;
    m_enqueue_clear_uniform_steps = false;
}

/**********************
 *** Public methods ***
 **********************/

GLProgramObject::GLProgramObject()
{
    // empty
}

GLProgramObject::GLProgramObject(GLuint vertex_shader, GLuint fragment_shader, bool particle_system)
{
    m_link_program(vertex_shader, fragment_shader, particle_system);
}

GLProgramObject::GLProgramObject(const char* vertex_src, const char* fragment_src, bool particle_system)
{
    GLuint vertex_shader = s_compile_shader(GL_VERTEX_SHADER, vertex_src);

    if(!vertex_shader)
        return;

    GLuint fragment_shader = s_compile_shader(GL_FRAGMENT_SHADER, fragment_src);

    if(!fragment_shader)
    {
        glDeleteShader(vertex_shader);
        return;
    }

    m_link_program(vertex_shader, fragment_shader, particle_system);

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
    m_flags = other.m_flags;

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
 * \param l StdPictureLoad to restore uniform registrations / assignments from
 *
 * Note: will fix any incorrect assignment types in l
 */
void GLProgramObject::restore_uniforms(StdPictureLoad& l)
{
    // clear own custom uniform state
    m_u_custom_loc.clear();

    // save l's custom uniform state, clearing l itself
    std::vector<std::string> uniform_names;
    std::vector<UniformValue_t> final_values;
    std::swap(uniform_names, l.registeredUniforms);
    std::swap(final_values, l.finalUniformState);

    // register all uniforms (saving their types)
    for(const std::string& name : uniform_names)
        register_uniform(name.c_str(), l);

    // assign all uniforms
    for(size_t i = 0; i < final_values.size(); i++)
        assign_uniform((int)i, final_values[i], l);
}

/*!
 * \brief Registers a custom uniform variable in the next available index
 */
int GLProgramObject::register_uniform(const char* name, StdPictureLoad& l)
{
    auto it = std::find(l.registeredUniforms.begin(), l.registeredUniforms.end(), name);

    if(it != l.registeredUniforms.end())
        return it - l.registeredUniforms.begin();

    GLint loc = -1;
    if(m_program)
        loc = glGetUniformLocation(m_program, name);

    if(loc < 0)
        pLogWarning("GLProgramObject: attempted to register non-existent uniform %s", name);

    GLenum full_type = 0;
    if(loc >= 0)
    {
        // size is not nullable on certain drivers, length and name are.
        GLint size;
        glGetActiveUniform(m_program, loc, 0, nullptr, &size, &full_type, nullptr);
        (void)size;
    }

    int16_t type;
    uint8_t width;

    switch(full_type)
    {
    default:
        if(loc >= 0)
            pLogWarning("GLProgramObject: attempted to register uniform %s of invalid type %x", name, (unsigned int)full_type);

        // fall through

    case(GL_FLOAT):
        type = XT_GL_FLOAT;
        width = 1;
        break;

    case(GL_FLOAT_VEC2):
        type = XT_GL_FLOAT;
        width = 2;
        break;

    case(GL_FLOAT_VEC3):
        type = XT_GL_FLOAT;
        width = 3;
        break;

    case(GL_FLOAT_VEC4):
        type = XT_GL_FLOAT;
        width = 4;
        break;

    case(GL_INT):
        type = XT_GL_INT;
        width = 1;
        break;

    case(GL_INT_VEC2):
        type = XT_GL_INT;
        width = 2;
        break;

    case(GL_INT_VEC3):
        type = XT_GL_INT;
        width = 3;
        break;

    case(GL_INT_VEC4):
        type = XT_GL_INT;
        width = 4;
        break;
    }

    l.registeredUniforms.push_back(name);
    l.finalUniformState.push_back(UniformValue_t(type, width));

    m_u_custom_loc.push_back(loc);

    return (int)m_u_custom_loc.size() - 1;
}

/*!
 * \brief Gets location of custom uniform variable by registered index
 */
GLint GLProgramObject::get_uniform_loc(int index)
{
    if(index >= 0 && index < (int)m_u_custom_loc.size())
        return m_u_custom_loc[index];
    else
        return -1;
}

void GLProgramObject::assign_uniform(int index, const UniformValue_t& value, StdPictureLoad& l)
{
    if(index < 0 || index >= (int)m_u_custom_loc.size() || index >= (int)l.finalUniformState.size())
    {
        pLogWarning("GLProgramObject: invalid assignment called for uniform registered at index %d, only %d registered.", index, (int)m_u_custom_loc.size());
        return;
    }

    if(m_u_custom_loc[index] == -1)
    {
        D_pLogDebug("GLProgramObject: attempted assignment to non-existent uniform registered at index %d.", index);
        l.finalUniformState[index] = value;
        return;
    }

    if(value.type != l.finalUniformState[index].type || value.width != l.finalUniformState[index].width)
    {
        pLogWarning("GLProgramObject: type mismatch in attempted assignment for uniform registered at index %d.", index);
        return;
    }

    if(m_enqueue_clear_uniform_steps)
        m_clear_uniform_steps();

    if(value != l.finalUniformState[index])
    {
        if(m_u_custom_loc[index] != -1)
        {
            m_uniform_steps.push_back({l.finalUniformState[index], value, index});
            m_final_uniform_step++;
        }

        l.finalUniformState[index] = value;
    }
}

#else // #ifdef RENDERGL_HAS_SHADERS

// all other functions intentionally left undefined, will cause link error if used

GLProgramObject::GLProgramObject()
{
    // empty
}

GLProgramObject::~GLProgramObject()
{
    // empty
}

void GLProgramObject::m_activate_uniform_step(uint16_t step)
{
    (void)(step);
}

void GLProgramObject::m_update_transform(const GLfloat* transform, const GLfloat* read_viewport, GLfloat clock)
{
    // empty
    (void)(transform);
    (void)(read_viewport);
    (void)(clock);
}

#endif
