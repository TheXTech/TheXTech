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
#ifndef GL_PARTICLE_SYSTEM_H

#include <array>
#include <vector>

#include "core/sdl/gl_inc.h"
#include "core/sdl/gl_program_object.h"

struct ParticleVertexImmutable_t
{
    GLfloat index = 0.0;
    std::array<GLubyte, 2> texcoord = {0, 0};
};

struct ParticleVertexMutable_t
{
    std::array<GLfloat, 2> position = {0.0, 0.0};
    GLfloat spawn_time = -60.0;
    std::array<GLubyte, 4> attribs = {0, 0, 0, 0};
};

class GLParticleSystem
{
    GLuint m_vertex_buffer = 0;

    std::vector<ParticleVertexImmutable_t> m_vertices_immutable;
    std::vector<ParticleVertexMutable_t> m_vertices_mutable;

    int m_particle_count = 0;
    int m_next_particle = 0;

public:

    void init(int particle_count);

    inline bool inited() const
    {
        return m_vertex_buffer;
    }

    void reset();

    void fill_and_draw();

    inline ~GLParticleSystem()
    {
        reset();
    }

    // void AddParticle();
};

#endif // #ifndef GL_PARTICLE_SYSTEM_H
