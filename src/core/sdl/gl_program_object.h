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

typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;
typedef float           GLfloat;

struct UniformValue_t
{
    GLfloat value;
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
    };

private:
    static GLuint s_last_program;

    GLuint m_program = 0;
    int m_flags = translucent;

    GLint m_u_transform_loc = -1;
    GLint m_u_read_viewport_loc = -1;
    GLint m_u_clock_loc = -1;

    std::vector<GLint> m_u_custom_loc;

    uint64_t m_transform_tick = 0;

    // rewind state for uniforms
    struct UniformAssignment_t
    {
        UniformValue_t pre;
        UniformValue_t post;
        int index;
    };

    int m_uniform_epoch = 0;
    int m_epoch_uniform_step = 0;
    int m_final_uniform_step = 0;

    int m_gl_uniform_step = 0;

    std::vector<UniformAssignment_t> m_uniform_steps;
    std::vector<UniformValue_t> m_final_uniform_state;

    // internal functions
    void m_update_transform(const GLfloat* transform, const GLfloat* read_viewport, GLfloat clock);

    static GLuint s_compile_shader(GLenum type, const char* src);

    void m_link_program(GLuint vertex_shader, GLuint fragment_shader);

    void m_clear_uniform_epoch();

public:

    GLProgramObject();

    GLProgramObject(const char* vertex_src, const char* fragment_src);
    GLProgramObject(GLuint vertex_shader, GLuint fragment_shader);

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
     * \returns The internal index for the uniform
     */
    int register_uniform(const std::string& name);

    /*!
     * \brief Gets location of custom uniform variable by registered index (advanced, ignores uniform state management)
     */
    GLint get_uniform_loc(int index);

    /*!
     * \brief Updates the current rewindable set of uniform assignments; if the epoch passed is not equal to the current one, clears this set
     * \param epoch set of uniform assignments it should be possible to rewind through; generally follows the frame counter
     */
    inline void set_uniform_epoch(int epoch)
    {
        if(epoch != m_uniform_epoch)
        {
            m_clear_uniform_epoch();
            m_uniform_epoch = epoch;
        }
    }

    /*!
     * \brief Assigns a custom uniform variable to a value and stores it in the managed uniform state
     * \param index registered internal index returned by previous call to register_uniform
     * \param value to assign the uniform to
     */
    void assign_uniform(int index, const UniformValue_t& value);

    /*!
     * \brief Returns the current uniform step for rewinding during the current frame
     * \returns The current uniform step
     */
    inline int get_uniform_step()
    {
        return m_final_uniform_step;
    }

    /*!
     * \brief Returns the current uniform step for rewinding during the current frame
     * \param step a step returned from get_uniform_step during the current epoch
     */
    void activate_uniform_step(int step);
};

#endif // #ifndef GL_PROGRAM_OBJECT_H
