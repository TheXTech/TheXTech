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

#include <SDL2/SDL_opengles2.h>
#include <vector>

class GLProgramObject
{
private:
    GLuint m_program = 0;

    GLint m_u_transform_loc = -1;

    std::vector<GLint> m_u_custom_loc;

    bool m_transform_dirty = true;

    void m_update_transform(const GLfloat* value);

    static GLuint s_compile_shader(GLenum type, const char* src);

    virtual void m_link_program(GLuint vertex_shader, GLuint fragment_shader);

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
     * \brief Mark that the program's transform matrix should be updated on next call
     */
    inline void set_transform_dirty()
    {
        m_transform_dirty = true;
    }

    /*!
     * \brief If the program's transform matrix is dirty, update it with the matrix provided.
     * \param value pointer to GLfloat array of size 16, the projection matrix that should be used if necessary.
     *
     * Important note: may only be called while program has been activated by use_program()
     */
    inline void update_transform(const GLfloat* value)
    {
        if(m_transform_dirty)
        {
            m_update_transform(value);
            m_transform_dirty = false;
        }
    }

    /*!
     * \brief Reports whether there is a valid program stored in the object
     */
    inline bool inited()
    {
        return m_program != 0;
    }

    /*!
     * \brief Registers a custom uniform variable in the next available index
     */
    void register_uniform(const std::string& name);

    /*!
     * \brief Gets location of custom uniform variable by registered index
     */
    GLint get_uniform_loc(int index);

    /*!
     * \brief Gets location of custom uniform variable by querying GL (slow)
     */
    GLint get_uniform_loc(const std::string& name);
};

#endif // #ifndef GL_PROGRAM_OBJECT_H
