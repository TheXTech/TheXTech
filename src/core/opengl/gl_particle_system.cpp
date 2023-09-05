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

#include "core/opengl/gl_particle_system.h"
#include "core/opengl/gl_inc.h"

void GLParticleSystem::init(int particle_count)
{
    reset();

    m_vertices_immutable.clear();
    m_vertices_immutable.resize(particle_count * 6);

    m_vertices_mutable.clear();
    m_vertices_mutable.resize(particle_count * 6);

    m_particle_count = particle_count;
    m_next_particle = 0;

    for(int particle = 0; particle < particle_count; particle++)
    {
        for(int vertex = 0; vertex < 6; vertex++)
        {
            ParticleVertexImmutable_t& target = m_vertices_immutable[particle * 6 + vertex];

            target.index = (GLfloat)particle / particle_count;

            if(vertex == 0 || vertex == 1 || vertex == 3)
                target.texcoord[0] = 0;
            else
                target.texcoord[0] = 1;

            if(vertex == 0 || vertex == 3 || vertex == 5)
                target.texcoord[1] = 0;
            else
                target.texcoord[1] = 1;
        }
    }

    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertexImmutable_t) * m_vertices_immutable.size() + sizeof(ParticleVertexMutable_t) * m_vertices_mutable.size(), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleVertexImmutable_t) * m_vertices_immutable.size(), m_vertices_immutable.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(ParticleVertexImmutable_t) * m_vertices_immutable.size(), sizeof(ParticleVertexMutable_t) * m_vertices_mutable.size(), m_vertices_mutable.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLParticleSystem::reset()
{
    if(m_vertex_buffer)
    {
        glDeleteBuffers(1, &m_vertex_buffer);
        m_vertex_buffer = 0;
    }

    m_vertices_immutable.clear();
    m_vertices_mutable.clear();

    m_particle_count = 0;

    m_next_particle = 0;
    m_modified_count = 0;

    m_oldest_active_particle = -1;
}

inline void GLParticleSystem::m_update_buffer_section(int begin, int count)
{
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(ParticleVertexImmutable_t) * m_vertices_immutable.size() + sizeof(ParticleVertexMutable_t) * (begin * 6),
        sizeof(ParticleVertexMutable_t) * (count * 6), m_vertices_mutable.data() + (begin * 6));
}

void GLParticleSystem::fill_and_draw(GLfloat clock)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    // allow any old particles to expire

    // to properly delete expired particles (prevent wraparound loop), do something like:
    //   - track "oldest active"
    //   - if oldest active overwritten, just shift up on
    //   - each draw, check if oldest has expired (is about to wraparound)
    //     - compare with clock, erase if 90% of loop_period has elapsed
    //     - erase by setting spawn_time to -loop_period
    //     - iteratively check next-oldest if so

    int particle_wipe_start = m_oldest_active_particle;
    int particle_wipe_end = m_oldest_active_particle;
    while(m_oldest_active_particle != -1)
    {
        GLfloat spawn_time = m_vertices_mutable[m_oldest_active_particle * 6].spawn_time;

        // if spawn_time was already negative, then no particles are currently active
        if(spawn_time < 0.0f)
        {
            m_oldest_active_particle = -1;
            break;
        }

        GLfloat time_since_spawn = clock - spawn_time;

        // oldest particle should be inactivated
        if((time_since_spawn >= 0.0f) ? (time_since_spawn > 50.0f) : (time_since_spawn > 50.0f + -60.0f))
        {
            // clear the particle
            for(int i = 0; i < 6; i++)
                m_vertices_mutable[m_oldest_active_particle * 6 + i].spawn_time = -60.0f;

            m_oldest_active_particle++;
            particle_wipe_end++;

            // push the wipe to GL now if wrapping around
            if(m_oldest_active_particle == m_particle_count)
            {
                // particles from (particle_wipe_start) up to (particle_wipe_end), delta = particle_wipe_end - particle_wipe_start
                m_update_buffer_section(particle_wipe_start, particle_wipe_end - particle_wipe_start);

                m_oldest_active_particle = 0;
                particle_wipe_start = 0;
                particle_wipe_end = 0;
            }
        }
        // all particles still active
        else
        {
            break;
        }
    }

    if(particle_wipe_end != particle_wipe_start)
        m_update_buffer_section(particle_wipe_start, particle_wipe_end - particle_wipe_start);


    // update all needed particles

    // whole buffer
    if(m_modified_count >= m_particle_count)
    {
        m_update_buffer_section(0, (int)m_vertices_mutable.size());
    }
    // wraparound (start and finish of buffer)
    else if(m_modified_count > m_next_particle)
    {
        // particles up to (not including) m_next_particle
        m_update_buffer_section(0, m_next_particle);

        // particles from (m_next_particle + m_particle_count - m_modified_count) up to (m_particle_count), delta = m_modified_count - m_next_particle
        int mod_wrap_start = m_next_particle + m_particle_count - m_modified_count;
        int mod_wrap_count = m_modified_count - m_next_particle;
        m_update_buffer_section(mod_wrap_start, mod_wrap_count);
    }
    // internal section of buffer
    else if(m_modified_count > 0)
    {
        // particles from (m_next_particle - m_modified_count) up to (m_next_particle), delta = m_modified_count
        int mod_start = m_next_particle - m_modified_count;
        m_update_buffer_section(mod_start, m_modified_count);
    }

    m_modified_count = 0;


    // load all attributes
    glVertexAttribPointer(0, 1, GL_FLOAT,         GL_FALSE, sizeof(ParticleVertexImmutable_t), (void*)offsetof(ParticleVertexImmutable_t, index));
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(ParticleVertexImmutable_t), (void*)offsetof(ParticleVertexImmutable_t, texcoord));

    uint8_t* mutable_array_offset = (uint8_t*)(sizeof(ParticleVertexImmutable_t) * m_vertices_immutable.size());
    glVertexAttribPointer(2, 2, GL_FLOAT,         GL_FALSE, sizeof(ParticleVertexMutable_t), mutable_array_offset + offsetof(ParticleVertexMutable_t, position));
    glVertexAttribPointer(3, 1, GL_FLOAT,         GL_FALSE, sizeof(ParticleVertexMutable_t), mutable_array_offset + offsetof(ParticleVertexMutable_t, spawn_time));
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(ParticleVertexMutable_t), mutable_array_offset + offsetof(ParticleVertexMutable_t, attribs));

    // enable the arrays
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    // draw!
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices_immutable.size());

    // disable the arrays (necessary for Emscripten and possibly also GL core)
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void GLParticleSystem::add_particle(const ParticleVertexMutable_t& state)
{
    if(m_next_particle < 0 || m_next_particle * 6 >= (int)m_vertices_mutable.size())
        m_next_particle = 0;

    for(int i = 0; i < 6; i++)
    {
        m_vertices_mutable[(m_next_particle * 6) + i] = state;
    }

    // keep track of true oldest particle
    if(m_oldest_active_particle == -1)
        m_oldest_active_particle = m_next_particle;
    else if(m_oldest_active_particle == m_next_particle)
    {
        m_oldest_active_particle += 1;

        if(m_oldest_active_particle == m_particle_count)
            m_oldest_active_particle = 0;
    }

    m_next_particle++;
    m_modified_count++;
}
