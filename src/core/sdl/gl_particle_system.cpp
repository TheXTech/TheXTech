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

#include <cstddef>

#include "core/sdl/gl_particle_system.h"

void GLParticleSystem::init(int particle_count)
{
    reset();

    m_vertices.clear();
    m_vertices.resize(particle_count * 6);

    m_particle_count = particle_count;
    m_next_particle = 0;

    for(int particle = 0; particle < particle_count; particle++)
    {
        for(int vertex = 0; vertex < 6; vertex++)
        {
            ParticleVertex_t& target = m_vertices[particle * 6 + vertex];

            target.index = (GLfloat)particle / particle_count;

            if(vertex == 0 || vertex == 1 || vertex == 3)
                target.coord[0] = 0;
            else
                target.coord[0] = 1;

            if(vertex == 0 || vertex == 3 || vertex == 5)
                target.coord[1] = 0;
            else
                target.coord[1] = 1;
        }
    }

    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertex_t) * m_vertices.size(), m_vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLParticleSystem::reset()
{
    if(m_vertex_buffer)
    {
        glDeleteBuffers(1, &m_vertex_buffer);
        m_vertex_buffer = 0;
    }

    m_vertices.clear();
    m_particle_count = 0;
    m_next_particle = 0;
}

void GLParticleSystem::fill_and_draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    glVertexAttribPointer(0, 1, GL_FLOAT,         GL_FALSE, sizeof(ParticleVertex_t), (void*)offsetof(ParticleVertex_t, index));
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(ParticleVertex_t), (void*)offsetof(ParticleVertex_t, coord));

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    // if(mod_count > m_particle_count) ...
    // if(mod_start + mod_count > m_particle_count) two updates
    // else
    // glBufferSubData(GL_ARRAY_BUFFER, mod_start * sizeof(ParticleVertex_t), mod_count * sizeof(ParticleVertex_t), m_vertices.data() + mod_start);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}
