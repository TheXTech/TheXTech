/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "core/opengl/gl_inc.h"

#include <Logger/logger.h>

#include "core/render.h"
#include "core/opengl/render_gl.h"
#include "core/opengl/gl_program_object.h"

#include "globals.h"
#include "sdl_proxy/sdl_assert.h"


#ifndef UNUSED
#define UNUSED(x) (void)x
#endif


// arguments
constexpr bool s_prefer_fb_copyTex = false;
static int s_num_pass = 2;


void RenderGL::s_normalize_coords(RectSizeI& r)
{
    if(r.x < 0)
    {
        r.w += r.x;
        r.x = 0;
    }

    if(r.y < 0)
    {
        r.h += r.y;
        r.y = 0;
    }

    if(r.x + r.w >= XRender::TargetW)
        r.w = XRender::TargetW - r.x;

    if(r.y + r.h >= XRender::TargetH)
        r.h = XRender::TargetH - r.y;
}

void RenderGL::m_Ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far)
{
    if(m_gl_profile == SDL_GL_CONTEXT_PROFILE_ES)
    {
#ifdef RENDERGL_HAS_ORTHOF
        glOrthof(left, right, bottom, top, near, far);
#endif
    }
    else
    {
#ifdef RENDERGL_HAS_ORTHO
        glOrtho(left, right, bottom, top, near, far);
#endif
    }

#if !defined(RENDERGL_HAS_ORTHO) && !defined(RENDERGL_HAS_ORTHOF)
    UNUSED(left); UNUSED(right); UNUSED(bottom); UNUSED(top); UNUSED(near); UNUSED(far);
#endif
}

void RenderGL::framebufferCopy(BufferIndex_t dest, BufferIndex_t source, RectSizeI r)
{
#ifdef RENDERGL_HAS_FBO

    s_normalize_coords(r);

    if(r.w <= 0 || r.h <= 0)
        return;

    // there are two methods: copy via glCopyTexSubImage2D and a draw call from the source buffer's texture to the dest fbo

    bool can_use_copyTex = m_buffer_texture[dest] && m_buffer_fb[source];
    bool can_use_draw    = m_buffer_texture[source] && m_buffer_fb[dest];

    bool use_copyTex = (s_prefer_fb_copyTex || !can_use_draw) && can_use_copyTex;

    if(use_copyTex)
    {
        if(source != m_cur_pass_target)
            glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[source]);

        glBindTexture(GL_TEXTURE_2D, m_buffer_texture[dest]);

        r = r * m_render_scale_factor;

        glCopyTexSubImage2D(
            GL_TEXTURE_2D, 0,
            r.x, r.y,
            r.x, r.y, r.w, r.h
        );

        if(source != m_cur_pass_target)
            glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[m_cur_pass_target]);
    }
    else if(can_use_draw)
    {
        // bind dest framebuffer and source texture

        if(dest != m_cur_pass_target)
            glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[dest]);

        glBindTexture(GL_TEXTURE_2D, m_buffer_texture[source]);

        // use default program for copy

        if(m_use_shaders)
        {
            m_standard_program.use_program();
            m_standard_program.update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);
        }

        // form vertices: dest is relative to viewport, source isn't

        RectI draw_loc = RectI(r);

        RectF draw_source = RectF(draw_loc);
        draw_source /= PointF(XRender::TargetW, XRender::TargetH);

        // dest rect is viewport-relative
        draw_loc -= m_viewport.xy;

        std::array<Vertex_t, 4> copy_triangle_strip =
            genTriangleStrip(draw_loc, draw_source, 0x7FFF, {255, 255, 255, 255});

        // fill vertex buffer to GL state and execute draw call

        fillVertexBuffer(copy_triangle_strip.data(), 4);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // restore standard framebuffer

        if(dest != m_cur_pass_target)
            glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[m_cur_pass_target]);
    }
    else
    {
        pLogWarning("Render GL: invalid buffer copy called");
    }

#else // #ifdef RENDERGL_HAS_FBO
    UNUSED(dest);
    UNUSED(source);
    UNUSED(r);

    pLogWarning("Render GL: framebufferCopy called without framebuffer subsystem -> no-op");
#endif
}

void RenderGL::depthbufferCopy()
{
#ifdef RENDERGL_HAS_FBO

    RectSizeI r = m_viewport;

    s_normalize_coords(r);

    if(r.w <= 0 || r.h <= 0)
        return;

    // there are two methods: copy via glCopyTexSubImage2D (faster, but doesn't work on GL ES) or copy via glBlitFramebuffer

    bool can_use_copyTex = m_depth_read_texture && (m_gl_profile != SDL_GL_CONTEXT_PROFILE_ES);
    bool can_use_blitFramebuffer = m_buffer_fb[BUFFER_DEPTH_READ];

    // in every case, even failure, should bind m_depth_read_texture to TEXTURE_UNIT_DEPTH_READ

    glActiveTexture(TEXTURE_UNIT_DEPTH_READ);
    glBindTexture(GL_TEXTURE_2D, m_depth_read_texture);

    if(can_use_copyTex)
    {
        r = r * m_render_scale_factor;

        // easy copy call
        glCopyTexSubImage2D(
            GL_TEXTURE_2D, 0,
            r.x, r.y,
            r.x, r.y, r.w, r.h
        );
    }
    else if(can_use_blitFramebuffer)
    {
        // slightly more complex, glBlitFramebuffer blits from the READ framebuffer to the DRAW framebuffer

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_buffer_fb[BUFFER_DEPTH_READ]);

        glBlitFramebuffer(
            r.x, r.y, r.x + r.w, r.y + r.h,
            r.x, r.y, r.x + r.w, r.y + r.h,
            GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );

        // restore original framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[m_cur_pass_target]);
    }
    else
    {
        pLogWarning("Render GL: invalid depth buffer copy called");
    }

    // restore standard active texture before returning
    glActiveTexture(TEXTURE_UNIT_IMAGE);

#else // #ifdef RENDERGL_HAS_FBO

    pLogWarning("Render GL: depthbufferCopy called without framebuffer subsystem -> no-op");

#endif
}

void RenderGL::fillVertexBuffer(const RenderGL::Vertex_t* vertex_attribs, int count)
{
#ifndef RENDERGL_HAS_VBO
    UNUSED(count);
#endif

    const uint8_t* array_start = 0;

    if(m_client_side_arrays)
    {
        array_start = reinterpret_cast<const uint8_t*>(vertex_attribs);
    }
    else
    {
#ifdef RENDERGL_HAS_VBO
        m_cur_vertex_buffer_index++;
        if(m_cur_vertex_buffer_index >= s_num_vertex_buffers)
            m_cur_vertex_buffer_index = 0;

        GLsizeiptr buffer_size = sizeof(RenderGL::Vertex_t) * count;

        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer[m_cur_vertex_buffer_index]);

        if(m_vertex_buffer_size[m_cur_vertex_buffer_index] < buffer_size)
        {
            const auto draw_mode = (m_gl_majver == 1 ? GL_DYNAMIC_DRAW : GL_STREAM_DRAW);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, vertex_attribs, draw_mode);
            m_vertex_buffer_size[m_cur_vertex_buffer_index] = buffer_size;
        }
        else
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, vertex_attribs);
        }
#endif // #ifdef RENDERGL_HAS_VBO
    }

    if(m_use_shaders)
    {
#ifdef RENDERGL_HAS_SHADERS
        glVertexAttribPointer(0, 3, GL_SHORT,         GL_FALSE, sizeof(RenderGL::Vertex_t), array_start + offsetof(RenderGL::Vertex_t, position));
        glVertexAttribPointer(1, 2, GL_FLOAT,         GL_FALSE, sizeof(RenderGL::Vertex_t), array_start + offsetof(RenderGL::Vertex_t, texcoord));
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(RenderGL::Vertex_t), array_start + offsetof(RenderGL::Vertex_t, tint));
#endif
    }
    else
    {
#ifdef RENDERGL_HAS_FIXED_FUNCTION
        glVertexPointer(   3, GL_SHORT,         sizeof(RenderGL::Vertex_t), array_start + offsetof(RenderGL::Vertex_t, position));
        glTexCoordPointer( 2, GL_FLOAT,         sizeof(RenderGL::Vertex_t), array_start + offsetof(RenderGL::Vertex_t, texcoord));
        glColorPointer(    4, GL_UNSIGNED_BYTE, sizeof(RenderGL::Vertex_t), array_start + offsetof(RenderGL::Vertex_t, tint));
#endif
    }
}

void RenderGL::cleanupDrawQueues()
{
    for(auto it = m_unordered_draw_queue.begin(); it != m_unordered_draw_queue.end();)
    {
        if(!it->second.active)
        {
            it = m_unordered_draw_queue.erase(it);
        }
        else
        {
            it->second.active = false;
            ++it;
        }
    }

    for(auto it = m_ordered_draw_queue.begin(); it != m_ordered_draw_queue.end();)
    {
        if(!it->second.active)
        {
            it = m_ordered_draw_queue.erase(it);
        }
        else
        {
            it->second.active = false;
            ++it;
        }
    }
}

void RenderGL::clearDrawQueues()
{
    m_drawQueued = false;

    for(auto& i : m_unordered_draw_queue)
        i.second.vertices.clear();

    for(auto& i : m_ordered_draw_queue)
        i.second.vertices.clear();

    m_recent_draw_context = DrawContext_t(nullptr);
    m_mask_draw_context_depth.clear();
}

void RenderGL::flushUnorderedDrawQueue()
{
    for(auto& i : m_unordered_draw_queue)
    {
        // (0) extract draw call info

        const DrawContext_t& context = i.first;
        VertexList& vertex_list = i.second;

        std::vector<Vertex_t>& vertex_attribs = vertex_list.vertices;

        // mark VertexList as active if anything will be drawn
        if(!vertex_attribs.empty())
            vertex_list.active = true;
        else
            continue;

        // context.program is not nullable
        if(!context.program)
        {
            SDL_assert(context.program);
            continue;
        }

        GLProgramObject* const program = context.program;

        // context.texture is nullable
        StdPicture* const texture = context.texture;


        // (1) load program and update state if necessary (uses m_transform_tick to prevent unnecessary updates)
        if(m_use_shaders)
        {
            SDL_assert(program->inited());

            program->use_program();
            program->update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);
            program->activate_uniform_step(context.uniform_step);
        }


        // (2) draw!

        // load vertex attributes into current GL buffer
        fillVertexBuffer(vertex_attribs.data(), (int)vertex_attribs.size());

        // bind texture if it exists
        if(texture)
            glBindTexture(GL_TEXTURE_2D, texture->d.texture_id);
        else
            glBindTexture(GL_TEXTURE_2D, 0);

        // draw!
        glDrawArrays(GL_TRIANGLES, 0, vertex_attribs.size());


        // (3) clear list
        vertex_attribs.clear();
    }
}

void RenderGL::executeOrderedDrawQueue(bool clear)
{
    for(auto& i : m_ordered_draw_queue)
    {
        // (0) extract draw call info

        // int draw_depth = i.first.first; // unused
        const DrawContext_t& context = i.first.second;

        VertexList& vertex_list = i.second;
        std::vector<Vertex_t>& vertex_attribs = vertex_list.vertices;

        // mark VertexList as active if anything will be drawn
        if(!vertex_attribs.empty())
            vertex_list.active = true;
        else
            continue;

        // context.program is not nullable
        if(!context.program)
        {
            SDL_assert(context.program);
            continue;
        }

        GLProgramObject* program = context.program;

        // context.texture is nullable
        StdPicture* texture = context.texture;


        // (1) figure out whether this draw should use logic op rendering and prepare if so
        bool use_gl_logic_op = (texture && texture->d.mask_texture_id && program == &m_standard_program);

        // emulate the logic op if we can't use it directly
        if(use_gl_logic_op && !m_use_logicop)
        {
            program = &m_bitmask_program;
            use_gl_logic_op = false;

#ifdef RENDERGL_HAS_FBO
            // bind the bitmask texture to the secondary texture unit
            glActiveTexture(TEXTURE_UNIT_MASK);
            glBindTexture(GL_TEXTURE_2D, texture->d.mask_texture_id);
            glActiveTexture(TEXTURE_UNIT_IMAGE);
#endif
        }


        // (2) setup the framebuffer read state as needed
        if(program->get_flags() & GLProgramObject::read_buffer)
        {
            // multiple quads -> copy the whole screen
            if(program->get_flags() & GLProgramObject::particles || vertex_attribs.size() > 6)
            {
                framebufferCopy(BUFFER_FB_READ, m_cur_pass_target, m_viewport);
            }
            // copy the quad behind the draw (speedup, doesn't work for rotated draw)
            else if(vertex_attribs.size() == 6)
            {
                RectSizeI dest = RectSizeI(
                    vertex_attribs[0].position[0],
                    vertex_attribs[0].position[1],
                    vertex_attribs[5].position[0] - vertex_attribs[0].position[0],
                    vertex_attribs[5].position[1] - vertex_attribs[0].position[1]
                );

                framebufferCopy(BUFFER_FB_READ, m_cur_pass_target, dest + m_viewport.xy + m_viewport_offset);
            }
        }


        // (3) load program and update state if necessary (uses m_transform_tick to prevent unnecessary updates)
        if(m_use_shaders)
        {
            SDL_assert(program->inited());

            program->use_program();
            program->update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);
            program->activate_uniform_step(context.uniform_step);
        }


        // (4) draw! (separate logic for particle system, OpenGL LogicOp draw, and standard vertex-based draw)
        if(program->get_flags() & GLProgramObject::particles)
        {
            bool state_valid = program->inited() && texture && texture->d.particle_system;
            SDL_assert(state_valid);

            if(state_valid)
            {
                glBindTexture(GL_TEXTURE_2D, texture->d.texture_id);
                texture->d.particle_system->fill_and_draw(m_shader_clock);
            }
        }
        else if(use_gl_logic_op)
        {
            // load vertex attributes into current GL buffer
            fillVertexBuffer(vertex_attribs.data(), (int)vertex_attribs.size());

            // draw mask
            prepareDrawMask();
            glBindTexture(GL_TEXTURE_2D, texture->d.mask_texture_id);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex_attribs.size());

            // draw image at same coordinates
            prepareDrawImage();
            glBindTexture(GL_TEXTURE_2D, texture->d.texture_id);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex_attribs.size());

            // return to standard LogicOp state
            leaveMaskContext();
        }
        else
        {
            // load vertex attributes into current GL buffer
            fillVertexBuffer(vertex_attribs.data(), (int)vertex_attribs.size());

            // bind image texture if it exists
            if(texture)
                glBindTexture(GL_TEXTURE_2D, texture->d.texture_id);
            else
                glBindTexture(GL_TEXTURE_2D, 0);

            // draw!
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex_attribs.size());
        }


        // (5) clear vertex list, only if requested
        if(clear)
            vertex_attribs.clear();
    }
}

void RenderGL::calculateDistanceField()
{
#if defined(RENDERGL_HAS_FBO) && defined(RENDERGL_HAS_SHADERS)
    if(!m_distance_field_1_program.inited() || !m_distance_field_2_program.inited())
        return;

    glDepthMask(GL_FALSE);

    // (0) activate a half-resolution viewport
    RectSizeI viewport = m_viewport;

    s_normalize_coords(viewport);

    RectSizeI viewport_scaled = viewport * 0.5f;

    glViewport(viewport_scaled.x, viewport_scaled.y,
        viewport_scaled.w, viewport_scaled.h);

    // (1) create a draw call
    RectI draw_loc = RectI(m_viewport);

    RectF draw_source = RectF(draw_loc);
    draw_source /= PointF(XRender::TargetW, XRender::TargetH);

    // draw dest is in viewport coordinates, draw source isn't
    draw_loc -= m_viewport.xy;

    std::array<Vertex_t, 4> dist_triangle_strip =
        genTriangleStrip(draw_loc, draw_source, 0x7FFF, {255, 255, 255, 255});


    // (2) bind the FB read framebuffer (since it is depthless) and the actual game depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[BUFFER_FB_READ]);
    glActiveTexture(TEXTURE_UNIT_DEPTH_READ);
    glBindTexture(GL_TEXTURE_2D, m_game_depth_texture);

    // (2) run the first pass (edge or not?)
    m_distance_field_1_program.use_program();
    m_distance_field_1_program.update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);

    fillVertexBuffer(dist_triangle_strip.data(), 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    constexpr GLfloat pass_step_size[] = {16.0, 8.0, 4.0, 2.0, 1.0};
    constexpr int num_pass = sizeof(pass_step_size) / sizeof(GLfloat);
    for(int pass = 0; pass < num_pass; pass++)
    {
        BufferIndex_t prev_buffer = ((pass % 2) == (num_pass % 2) ? BUFFER_INT_PASS_1 : BUFFER_INT_PASS_2);
        BufferIndex_t draw_buffer = ((pass % 2) == (num_pass % 2) ? BUFFER_INT_PASS_2 : BUFFER_INT_PASS_1);

        if(pass == 0)
            prev_buffer = BUFFER_FB_READ;

        // (3) bind the correct int pass and prev pass framebuffers
        glActiveTexture(TEXTURE_UNIT_PREV_PASS);
        glBindTexture(GL_TEXTURE_2D, m_buffer_texture[prev_buffer]);
        glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[draw_buffer]);

        // (4) run the nth pass (edge distance propagation)
        m_distance_field_2_program.use_program();
        m_distance_field_2_program.update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);
        glUniform1f(m_distance_field_2_program.get_uniform_loc(0), pass_step_size[pass]);
        // glUniform1f(m_distance_field_2_program.get_uniform_loc(0), 1.0);

        fillVertexBuffer(dist_triangle_strip.data(), 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // (5) restore the original framebuffer and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_game_texture_fb);
    glActiveTexture(TEXTURE_UNIT_IMAGE);

    viewport_scaled = viewport * m_render_scale_factor;

    glViewport(viewport_scaled.x, viewport_scaled.y,
        viewport_scaled.w, viewport_scaled.h);
#endif
}

void RenderGL::coalesceLights()
{
#if defined(RENDERGL_HAS_FBO) && defined(RENDERGL_HAS_SHADERS)
    constexpr bool debug_coalesce = false;
    const GLfloat tolerance = 0.1;

    auto& lights = m_light_queue.lights;

    auto lights_begin_it = lights.begin();
    auto lights_end_it   = lights_begin_it + m_light_count;

    // sort by light type
    std::sort(lights_begin_it, lights_end_it);

    if(debug_coalesce)
        pLogDebug("Lights %d:%d", int(lights_begin_it - lights_begin_it), (lights_end_it - lights_begin_it));

    // for each light type, mark non-initial lights and try to coalesce (if box lights)
    auto type_begin_it = lights_begin_it;
    while(type_begin_it != lights_end_it)
    {
        auto type_end_it = std::upper_bound(type_begin_it, lights_end_it, *type_begin_it);

        if(debug_coalesce)
            pLogDebug("Type %d:%d", int(type_begin_it - lights_begin_it), (type_end_it - lights_begin_it));

        // can perform box coalescing algorithm
        if(type_begin_it->type == GLLightType::box)
        {
            // comparison to sort by row, higher first, then sort by height, taller first
            auto y_compare = [](const GLLight& a, const GLLight& b) {
                return a.pos[1] < b.pos[1] || (a.pos[1] == b.pos[1] && a.pos[3] > b.pos[3]);
            };

            // comparison to sort by column, left first, then sort by width, wider first
            auto x_compare = [](const GLLight& a, const GLLight& b) {
                return a.pos[0] < b.pos[0] || (a.pos[0] == b.pos[0] && a.pos[2] > b.pos[2]);
            };

            // coalesce horizontally, then vertically
            std::sort(type_begin_it, type_end_it, y_compare);

            // treat each row separately
            auto row_begin_it = type_begin_it;
            while(row_begin_it != type_end_it)
            {
                auto row_end_it = std::upper_bound(row_begin_it, type_end_it, *row_begin_it, y_compare);

                if(debug_coalesce)
                    pLogDebug("Row %d:%d", int(row_begin_it - lights_begin_it), (row_end_it - lights_begin_it));

                // sort row horizontally
                std::sort(row_begin_it, row_end_it, x_compare);

                // for each item in row, try to coalesce
                auto item_it = row_begin_it;
                while(item_it != row_end_it)
                {
                    auto orig_left = item_it->pos[0];

                    // bounds on left x of possible continuation
                    auto lower_bound = item_it->pos[2] - tolerance;
                    auto upper_bound = item_it->pos[2] + tolerance;

                    // use current item to temporarily store lower bound on left x
                    item_it->pos[0] = lower_bound;
                    auto continuation_it = std::lower_bound(item_it + 1, row_end_it, *item_it, x_compare);
                    item_it->pos[0] = orig_left;

                    // can coalesce!
                    if(continuation_it != row_end_it && continuation_it->pos[0] < upper_bound)
                    {
                        if(debug_coalesce)
                            pLogDebug("Combine item %d with continuation %d", int(item_it - lights_begin_it), int(continuation_it - lights_begin_it));

                        // set current item's right bound to continuation's right bound
                        item_it->pos[2] = continuation_it->pos[2];

                        // get rid of the continuation
                        std::rotate(continuation_it, continuation_it + 1, lights_end_it);

                        // update end iterators
                        --row_end_it;
                        --type_end_it;
                        --lights_end_it;
                    }
                    // try next item
                    else
                    {
                        ++item_it;
                    }
                }

                row_begin_it = row_end_it;
            }

            // now coalesce vertically
            std::sort(type_begin_it, type_end_it, x_compare);

            // treat each column separately
            auto col_begin_it = type_begin_it;
            while(col_begin_it != type_end_it)
            {
                auto col_end_it = std::upper_bound(col_begin_it, type_end_it, *col_begin_it, x_compare);

                if(debug_coalesce)
                    pLogDebug("Col %d:%d", int(col_begin_it - lights_begin_it), (col_end_it - lights_begin_it));

                // sort col vertically
                std::sort(col_begin_it, col_end_it, y_compare);

                // for each item in col, try to coalesce
                auto item_it = col_begin_it;
                while(item_it != col_end_it)
                {
                    auto orig_top = item_it->pos[1];

                    // bounds on top y of possible continuation
                    auto lower_bound = item_it->pos[3] - tolerance;
                    auto upper_bound = item_it->pos[3] + tolerance;

                    // use current item to temporarily store lower bound on top y
                    item_it->pos[1] = lower_bound;
                    auto continuation_it = std::lower_bound(item_it + 1, col_end_it, *item_it, y_compare);
                    item_it->pos[1] = orig_top;

                    // can coalesce!
                    if(continuation_it != col_end_it && continuation_it->pos[1] < upper_bound)
                    {
                        if(debug_coalesce)
                            pLogDebug("Combine item %d with continuation %d", int(item_it - lights_begin_it), int(continuation_it - lights_begin_it));

                        // set current item's bottom bound to continuation's bottom bound
                        item_it->pos[3] = continuation_it->pos[3];

                        // get rid of the continuation
                        std::rotate(continuation_it, continuation_it + 1, lights_end_it);

                        // update end iterators
                        --col_end_it;
                        --type_end_it;
                        --lights_end_it;
                    }
                    // try next item
                    else
                    {
                        ++item_it;
                    }
                }

                col_begin_it = col_end_it;
            }
        }

        // now mark all duplicates
        for(auto mark_it = type_begin_it + 1; mark_it < type_end_it; ++mark_it)
            mark_it->type = GLLightType::duplicate;

        type_begin_it = type_end_it;
    }

    if(debug_coalesce)
        pLogDebug("Coalesced %d lights to %d", m_light_count, int(lights_end_it - lights_begin_it));

    m_light_count = lights_end_it - lights_begin_it;
#endif
}

void RenderGL::calculateLighting()
{
#if defined(RENDERGL_HAS_FBO) && defined(RENDERGL_HAS_SHADERS)

    if(!m_lighting_calc_program.inited() || !m_light_ubo || !m_light_queue.header)
    {
        glActiveTexture(TEXTURE_UNIT_LIGHT_READ);
        glBindTexture(GL_TEXTURE_2D, m_null_light_texture);
        glActiveTexture(TEXTURE_UNIT_IMAGE);

        m_light_count = 0;

        return;
    }

    // (0) calculate the distance field (currently unused after benchmarking showed minimal improvements in intensive shadowing situations and large slowdowns normally)
    // calculateDistanceField();

    // (1) flush the lights
    coalesceLights();

    int lights_to_flush = m_light_count;

    if(m_light_count < (int)m_light_queue.lights.size())
    {
        m_light_queue.lights[m_light_count].type = GLLightType::none;
        lights_to_flush += 1;
    }
    else
        lights_to_flush = (int)m_light_queue.lights.size();

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLLightSystem) + sizeof(GLLight) * lights_to_flush, &m_light_queue);

    m_light_count = 0;


    // (2) bind the lighting framebuffer and choose the correct GL viewport given downscaling
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[BUFFER_LIGHTING]);

    // fix offscreen coordinates
    RectSizeI viewport = m_viewport;

    s_normalize_coords(viewport);

    RectSizeI viewport_scaled = viewport * m_lighting_scale_factor;

    glViewport(viewport_scaled.x, viewport_scaled.y,
        viewport_scaled.w, viewport_scaled.h);


    // (3) bind the texture and program
    glActiveTexture(TEXTURE_UNIT_DEPTH_READ);
    glBindTexture(GL_TEXTURE_2D, m_game_depth_texture);
    glActiveTexture(TEXTURE_UNIT_PREV_PASS);
    glBindTexture(GL_TEXTURE_2D, m_buffer_texture[BUFFER_INT_PASS_1]);

    m_lighting_calc_program.use_program();
    m_lighting_calc_program.update_transform(m_transform_tick, m_transform_matrix.data(), m_shader_read_viewport.data(), m_shader_clock);


    // (4) create and execute the draw call
    RectI draw_loc = RectI(m_viewport);

    RectF draw_source = RectF(draw_loc);
    draw_source /= PointF(XRender::TargetW, XRender::TargetH);

    // draw dest is in viewport coordinates, draw source isn't
    draw_loc -= m_viewport.xy;

    std::array<Vertex_t, 4> lighting_triangle_strip =
        genTriangleStrip(draw_loc, draw_source, 0x7FFF, {255, 255, 255, 255});

    // fill vertex buffer to GL state and execute draw call

    fillVertexBuffer(lighting_triangle_strip.data(), 4);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    // (5) restore the normal framebuffer and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[m_cur_pass_target]);

    viewport_scaled = viewport * m_render_scale_factor;

    glViewport(viewport_scaled.x, viewport_scaled.y,
        viewport_scaled.w, viewport_scaled.h);

    // (6) bind lighting texture
    glActiveTexture(TEXTURE_UNIT_LIGHT_READ);
    glBindTexture(GL_TEXTURE_2D, m_buffer_texture[BUFFER_LIGHTING]);
    glActiveTexture(TEXTURE_UNIT_IMAGE);
#endif
}

void RenderGL::prepareMultipassState(int pass, int num_pass)
{
#ifdef RENDERGL_HAS_FBO

    BufferIndex_t prev_buffer = (pass == 1        ? BUFFER_GAME :
                                    (pass % 2 ? BUFFER_INT_PASS_2 : BUFFER_INT_PASS_1 ));
    BufferIndex_t draw_buffer = (pass == num_pass ? BUFFER_GAME :
                                    (pass % 2 ? BUFFER_INT_PASS_1 : BUFFER_INT_PASS_2 ));

    // should only happen if num_pass is forced to be 1
    if(prev_buffer == draw_buffer)
    {
        // avoid undefined behavior (reading from currently bound color attachment texture)
        framebufferCopy(BUFFER_FB_READ, prev_buffer, m_viewport);
        prev_buffer = BUFFER_FB_READ;
    }

    // bind the previous pass texture
    glActiveTexture(TEXTURE_UNIT_PREV_PASS);
    glBindTexture(GL_TEXTURE_2D, m_buffer_texture[prev_buffer]);
    glActiveTexture(TEXTURE_UNIT_IMAGE);

    // set the desired framebuffer
    m_cur_pass_target = draw_buffer;
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_fb[draw_buffer]);

    // if not the last pass, copy opaque state from main framebuffer
    if(pass != num_pass)
        framebufferCopy(draw_buffer, BUFFER_GAME, m_viewport);

#else // #ifdef RENDERGL_HAS_FBO
    (void)pass;
    (void)num_pass;
#endif
}

void RenderGL::flushDrawQueues()
{
    if(!m_drawQueued)
        return;

    m_drawQueued = false;

    // pass 0: opaque textures (unordered draw queues)
    flushUnorderedDrawQueue();

    // passes 1 to num_pass: translucent / interesting textures
    bool any_translucent_draws = false;
    int active_draw_flags = 0;
    const int flags_all = GLProgramObject::multipass | GLProgramObject::read_depth | GLProgramObject::read_light;

    // first, check what is enqueued; may allow us to skip all translucent rendering or multipass logic
    for(auto& i : m_ordered_draw_queue)
    {
        // int draw_depth = i.first.first;
        const DrawContext_t& context = i.first.second;

        const VertexList& vertex_list = i.second;
        const std::vector<Vertex_t>& vertex_attribs = vertex_list.vertices;

        if(vertex_attribs.empty())
            continue;

        any_translucent_draws = true;

        const GLProgramObject* program = context.program;

        // add shader draw flags to requests
        if(program && (program->get_flags() & flags_all))
        {
            active_draw_flags |= program->get_flags();

            if((active_draw_flags & flags_all) == flags_all)
                break;
        }
    }

    bool do_lighting = ((active_draw_flags & GLProgramObject::read_light) && m_buffer_fb[BUFFER_LIGHTING]);

    // always reset lighting buffer if lighting calculation will not run
#ifdef RENDERGL_HAS_SHADERS
    if(!do_lighting)
        m_light_count = 0;
#endif

    // if no translucent objects, return without needing to call glDepthMask (speedup on Emscripten)
    if(!any_translucent_draws)
        return;

    // default to 1-pass rendering
    int num_pass = 1;

    // if shaders use multipass rendering and intermediate pass framebuffer successfully allocated, enable multipass rendering
    if((active_draw_flags & GLProgramObject::multipass) && m_buffer_fb[BUFFER_INT_PASS_1])
        num_pass = s_num_pass;

    // if second intermediate pass framebuffer missing, cap at 2 passes
    if(!m_buffer_fb[BUFFER_INT_PASS_2] && num_pass > 2)
        num_pass = 2;

    // if shaders use lighting and lighting framebuffer successfully allocated, calculate lighting
    if(do_lighting)
        calculateLighting();

    // if any shaders read the depth buffer and it is supported, copy it from the main framebuffer
    if((active_draw_flags & GLProgramObject::read_depth) && m_depth_read_texture)
        depthbufferCopy();

    // disable depth writing while rendering translucent textures (small speedup, needed for multipass rendering)
    if(m_use_depth_buffer)
        glDepthMask(GL_FALSE);

    for(int pass = 1; pass <= num_pass; pass++)
    {
        // setup state for multipass rendering
        if(active_draw_flags & GLProgramObject::multipass)
            prepareMultipassState(pass, num_pass);

        // execute and possibly flush ordered draws
        executeOrderedDrawQueue(pass == num_pass);
    }

    // re-enable depth writing following
    if(m_use_depth_buffer)
        glDepthMask(GL_TRUE);
}
