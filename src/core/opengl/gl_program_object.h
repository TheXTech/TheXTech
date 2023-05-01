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

#pragma once
#ifndef GL_PROGRAM_OBJECT_H

#include <vector>
#include <cstdint>

typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;
typedef float           GLfloat;

constexpr int XT_GL_FLOAT = 0x1406;
constexpr int XT_GL_INT   = 0x1404;

struct StdPictureLoad;


struct UniformValue_t
{
    union values_t
    {
        GLfloat f[4] = {0.0};
        GLint i[4];

        constexpr values_t(GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) :
            f{v1, v2, v3, v4} {}

        constexpr values_t(GLint v1, GLint v2, GLint v3, GLint v4) :
            i{v1, v2, v3, v4} {}
    };

    values_t values;
    int16_t type = XT_GL_FLOAT;
    uint8_t width = 1;

    explicit constexpr UniformValue_t(int16_t type, uint8_t width) :
        values(0.0f, 0.0f, 0.0f, 0.0f), type(type), width(width) {}

    constexpr UniformValue_t(GLfloat v1) :
        values(  v1, 0.0f, 0.0f, 0.0f), type(XT_GL_FLOAT), width(1) {}

    constexpr UniformValue_t(GLfloat v1, GLfloat v2) :
        values(  v1,   v2, 0.0f, 0.0f), type(XT_GL_FLOAT), width(2) {}

    constexpr UniformValue_t(GLfloat v1, GLfloat v2, GLfloat v3) :
        values(  v1,   v2,   v3, 0.0f), type(XT_GL_FLOAT), width(3) {}

    constexpr UniformValue_t(GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) :
        values(  v1,   v2,   v3,  v4), type(XT_GL_FLOAT), width(4) {}

    constexpr UniformValue_t(GLint   v1) :
        values(  v1,    0,    0,    0), type(XT_GL_INT  ), width(1) {}

    constexpr UniformValue_t(GLint   v1, GLint   v2) :
        values(  v1,   v2,    0,    0), type(XT_GL_INT  ), width(2) {}

    constexpr UniformValue_t(GLint   v1, GLint   v2, GLint   v3) :
        values(  v1,   v2,   v3,    0), type(XT_GL_INT  ), width(3) {}

    constexpr UniformValue_t(GLint   v1, GLint   v2, GLint   v3, GLint   v4) :
        values(  v1,   v2,   v3,   v4), type(XT_GL_INT  ), width(4) {}

    bool operator==(const UniformValue_t& o) const;

    inline bool operator!=(const UniformValue_t& o) const
    {
        return !(*this == o);
    }
};


class GLProgramObject
{
public:
    enum Flags
    {
        translucent = (1 << 0),
        read_buffer = (1 << 1),
        multipass   = (1 << 2),
        read_depth  = (1 << 3),
        read_light  = (1 << 4),
        particles   = (1 << 5),
    };

private:
    static GLuint s_last_program;

    GLuint m_program = 0;
    int m_flags = translucent;

    // OpenGL locations for common uniforms
    GLint m_u_transform_loc = -1;
    GLint m_u_read_viewport_loc = -1;
    GLint m_u_clock_loc = -1;

    // OpenGL locations for custom uniforms (indices match those returned by register_uniform)
    std::vector<GLint> m_u_custom_loc;

    uint64_t m_transform_tick = 0;

    // rewind state for uniforms
    struct UniformAssignment_t
    {
        UniformValue_t pre;
        UniformValue_t post;
        int index;
    };

    // these each denote states which PRECEDE the step's execution.
    // thus, the highest legal step is m_uniform_steps.size()

    // the uniform state following all issued assignments
    uint16_t m_final_uniform_step = 0;
    // the uniform state currently active in the GL
    uint16_t m_gl_uniform_step = 0;
    // whether the next uniform assignment should apply and clear all previous uniform steps
    bool m_enqueue_clear_uniform_steps = false;

    // a list of all uniform assignments since the previous clear
    std::vector<UniformAssignment_t> m_uniform_steps;

    // internal functions
    void m_update_transform(const GLfloat* transform, const GLfloat* read_viewport, GLfloat clock);

    static GLuint s_compile_shader(GLenum type, const char* src);

    void m_link_program(GLuint vertex_shader, GLuint fragment_shader, bool particle_system = false);

    void m_activate_uniform_step(uint16_t step);

    void m_clear_uniform_steps();

public:

    GLProgramObject();

    GLProgramObject(const char* vertex_src, const char* fragment_src, bool particle_system = false);
    GLProgramObject(GLuint vertex_shader, GLuint fragment_shader, bool particle_system = false);

    virtual ~GLProgramObject();

    const GLProgramObject& operator=(GLProgramObject&& other);

    /*!
     * \brief Resets program object and unloads in OpenGL
     */
    void reset();

    /*!
     * \brief Activate program in OpenGL state
     */
    void use_program();

    // inline functions for fast paths

    /*!
     * \brief If the program's transform matrix was not updated this tick, update it with the matrix provided.
     * \param transform_tick    the number of times the transform matrix has been changed since game start.
     * \param transform         pointer to GLfloat array of size 16, the projection matrix that should be used if necessary.
     * \param read_viewport     pointer to GLfloat array of size 4, an additional multiply-and-add transform for fb read shaders.
     *
     * Important note: may only be called while program has been activated by use_program()
     */
    inline void update_transform(uint64_t transform_tick, const GLfloat* transform, const GLfloat* read_viewport, GLfloat clock)
    {
        if(transform_tick != m_transform_tick)
        {
            m_transform_tick = transform_tick;
            m_update_transform(transform, read_viewport, clock);
        }
    }

    /*!
     * \brief Reports whether there is a valid program stored in the object
     */
    inline bool inited() const
    {
        return m_program != 0;
    }

    /*!
     * \brief Reports the program type for multipass rendering
     */
    inline int get_flags() const
    {
        return m_flags;
    }

    /*!
     * \brief Registers a custom uniform variable in the next available index
     * \param l StdPictureLoad to restore uniform registrations / assignments from
     *
     * Note: will fix the type of any assignments in l
     */
    void restore_uniforms(StdPictureLoad& l);

    /*!
     * \brief Registers a custom uniform variable in the next available index
     * \param name name of uniform
     * \param l StdPictureLoad to cache the registration in, in case of unload
     * \returns The internal index for the uniform, -1 on failure
     */
    int register_uniform(const char* name, StdPictureLoad& l);

    /*!
     * \brief Gets location of custom uniform variable by registered index (advanced, ignores uniform state management)
     */
    GLint get_uniform_loc(int index);

    /*!
     * \brief Assigns a custom uniform variable to a value and stores it in the managed uniform state
     * \param index registered internal index returned by previous call to register_uniform
     * \param value to assign the uniform to
     * \param l StdPictureLoad to check current state from and cache the assignment in, in case of unload
     *
     * Note: clears the previous rewind buffer if activate_uniform_step has been called.
     */
    void assign_uniform(int index, const UniformValue_t& value, StdPictureLoad& l);

    /*!
     * \brief Returns the current uniform step (following all pending assignments) for rewinding during the current frame
     * \returns The current uniform step
     *
     * FIXME: should clear the previous rewind buffer if activate_uniform_step has been called
     */
    inline uint16_t get_uniform_step()
    {
        if(m_enqueue_clear_uniform_steps)
            m_clear_uniform_steps();

        return m_final_uniform_step;
    }

    /*!
     * \brief Returns the current uniform step for rewinding during the current frame
     * \param step a step returned from get_uniform_step during the current frame
     *
     * Important note: may only be called while program has been activated by use_program()
     * Important note: this method is called while clearing the render queue;
     * thus, after calling this function at any point, any subsequent call to
     * assign_uniform will clear the rewind buffer and reset the final step to 0
     */
    void activate_uniform_step(uint16_t step)
    {
        if(step == m_gl_uniform_step)
            return;

        m_activate_uniform_step(step);
    }
};

#endif // #ifndef GL_PROGRAM_OBJECT_H
