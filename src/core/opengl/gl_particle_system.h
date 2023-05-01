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

#include <cstdint>

#include <array>
#include <vector>

typedef float           GLfloat;
typedef uint8_t         GLubyte;

#include "core/opengl/gl_program_object.h"

struct ParticleVertexImmutable_t
{
    GLfloat index = 0.0;
    std::array<GLubyte, 2> texcoord = {0, 0};
};

using ParticleVertexAttrs_t = std::array<GLubyte, 4>;

struct ParticleVertexMutable_t
{
    std::array<GLfloat, 2> position = {0.0, 0.0};
    GLfloat spawn_time = -60.0;
    ParticleVertexAttrs_t attribs = {0, 0, 0, 0};
};

class GLParticleSystem
{
    GLuint m_vertex_buffer = 0;

    std::vector<ParticleVertexImmutable_t> m_vertices_immutable;
    std::vector<ParticleVertexMutable_t> m_vertices_mutable;

    // this is the particle system's capacity
    int m_particle_count = 0;

    // the next particle index to use for add_particle
    int m_next_particle = 0;
    // the number of particles added during the last frame
    int m_modified_count = 0;

    // tracked to ensure particles expire before the clock wraps around
    int m_oldest_active_particle = -1;

    // update section of mutable vertex buffer
    void m_update_buffer_section(int begin, int count);

public:

    void init(int particle_count);

    inline bool inited() const
    {
        return m_vertex_buffer;
    }

    void reset();

    void fill_and_draw(GLfloat clock);

    inline ~GLParticleSystem()
    {
        reset();
    }

    void add_particle(const ParticleVertexMutable_t& state);
};

#endif // #ifndef GL_PARTICLE_SYSTEM_H
