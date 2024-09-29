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

#include "core/opengl/render_gl.h"

#ifdef RENDERGL_HAS_SHADERS

const char* const RenderGL::s_es3_lighting_calc_frag_src =
R"RAW(#version 300 es

precision mediump float;

const float shadowResolution = 0.5;
const float c_hifi_box_res = 0.0; // 1.0 / 8.0; // extremely expensive

const uint SYSTEM_TYPE_DISABLED    = 0U;
const uint SYSTEM_TYPE_SHADOW_NONE = 1U;
const uint SYSTEM_TYPE_SHADOW_RAYS = 2U;
const uint SYSTEM_TYPE_SHADOW_DROP = 3U;

const uint LIGHT_TYPE_NONE  = 0U;
const uint LIGHT_TYPE_POINT = 1U;
const uint LIGHT_TYPE_ARC   = 2U;
const uint LIGHT_TYPE_BAR   = 3U;
const uint LIGHT_TYPE_BOX   = 4U;
const uint LIGHT_TYPE_DUP   = 5U;

const int MAX_LIGHTS = 64;


// should be 32 bytes wide
struct Light
{
    uint type;
    highp uint color;
    float radius;
    float depth;
    vec4 pos;
};

layout(std140) uniform Lighting
{
    uint       u_system_type;
    float      u_shadow_strength;
    highp uint u_ambient_color;

    uint  u_padding2;
    Light u_lights[MAX_LIGHTS];
};


uniform highp mat4 u_transform;
uniform highp vec4 u_read_viewport;

uniform sampler2D u_depth_buffer;

in   vec2  v_fbcoord;

out vec4 o_FragColor;


void calculate_light_location(const in Light light, const in uint light_type, const in vec2 target_pos, out vec2 source_pos, out float source_depth)
{
    // calculate source position and draw plane
    if(light_type == LIGHT_TYPE_POINT || light_type == LIGHT_TYPE_ARC)
    {
        vec4 source_pos_raw = vec4(light.pos.xy, light.depth, 1.0);
        vec4 source_pos_transform = u_transform * source_pos_raw;
        source_pos = source_pos_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

        source_depth = (source_pos_transform.z / 2.0 + 0.5);
    }
    else if(light_type == LIGHT_TYPE_BOX || light_type == LIGHT_TYPE_BAR)
    {
        vec4 source_pos_a_raw = vec4(light.pos.xy, light.depth, 1.0);
        vec4 source_pos_a_transform = u_transform * source_pos_a_raw;
        vec2 source_pos_a = source_pos_a_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

        vec4 source_pos_b_raw = vec4(light.pos.zw, light.depth, 1.0);
        vec4 source_pos_b_transform = u_transform * source_pos_b_raw;
        vec2 source_pos_b = source_pos_b_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

        if(light_type == LIGHT_TYPE_BOX)
            source_pos = clamp(target_pos, source_pos_a, source_pos_b);
        else
        {
            vec2 start_to_target = target_pos - source_pos_a;
            vec2 start_to_end = source_pos_b - source_pos_a;

            float proj_coord = dot(start_to_end, start_to_target) / dot(start_to_end, start_to_end);
            proj_coord = clamp(proj_coord, 0.0, 1.0);
            vec2 start_to_proj = start_to_end * proj_coord;
            source_pos = start_to_proj + source_pos_a;
        }

        source_depth = (source_pos_a_transform.z / 2.0 + 0.5);
    }
}

vec3 light_color(const in highp uint color)
{
    return vec3((color >> 16) & 255U, (color >> 8) & 255U, (color >> 0) & 255U) / 255.0;
}

float fade_light(const in Light light, const in uint light_type, const in vec2 source_pos)
{
    // find out what the size of a pixel offset is
    vec2 pixel_size = (u_transform * vec4(1.0, 1.0, 0.0, 0.0)).xy * u_read_viewport.xy;

    // top-left and bottom-right 16-px margins
    vec2 margin_size = 64.0 * pixel_size;
    vec2 margin_extension = 16.0 * pixel_size;

    if(light_type == LIGHT_TYPE_BOX)
        margin_extension = vec2(0.0);

    vec2 margin_tl = u_read_viewport.zw - u_read_viewport.xy + margin_size - margin_extension;
    vec2 margin_br = u_read_viewport.zw + u_read_viewport.xy - margin_size + margin_extension;

    float decay = 1.0;

    if(light_type == LIGHT_TYPE_POINT || light_type == LIGHT_TYPE_ARC)
    {
        if(source_pos.x < margin_tl.x)
            decay *= 1.0 - (margin_tl.x - source_pos.x) / margin_size.x;

        if(source_pos.x > margin_br.x)
            decay *= 1.0 - (source_pos.x - margin_br.x) / margin_size.x;

        if(source_pos.y < margin_tl.y)
            decay *= 1.0 - (margin_tl.y - source_pos.y) / margin_size.y;

        if(source_pos.y > margin_br.y)
            decay *= 1.0 - (source_pos.y - margin_br.y) / margin_size.y;
    }
    else if(light_type == LIGHT_TYPE_BOX || light_type == LIGHT_TYPE_BAR)
    {
        vec4 source_pos_a_raw = vec4(light.pos.xy, light.depth, 1.0);
        vec4 source_pos_a_transform = u_transform * source_pos_a_raw;
        vec2 source_pos_a = source_pos_a_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

        vec4 source_pos_b_raw = vec4(light.pos.zw, light.depth, 1.0);
        vec4 source_pos_b_transform = u_transform * source_pos_b_raw;
        vec2 source_pos_b = source_pos_b_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

        if(light_type == LIGHT_TYPE_BAR)
        {
            vec2 source_pos_a2 = source_pos_a;
            source_pos_a = min(source_pos_a2, source_pos_b);
            source_pos_b = max(source_pos_a2, source_pos_b);
        }

        if(source_pos_b.x < margin_tl.x)
            decay *= 1.0 - (margin_tl.x - source_pos_b.x) / margin_size.x;

        if(source_pos_a.x > margin_br.x)
            decay *= 1.0 - (source_pos_a.x - margin_br.x) / margin_size.x;

        if(source_pos_b.y < margin_tl.y)
            decay *= 1.0 - (margin_tl.y - source_pos_b.y) / margin_size.y;

        if(source_pos_a.y > margin_br.y)
            decay *= 1.0 - (source_pos_a.y - margin_br.y) / margin_size.y;
    }

    return decay;
}

float calculate_light_falloff(const in Light light, const in uint light_type, const in vec2 target_pos, const in vec2 source_pos)
{
    // find out what the size of a pixel offset is
    vec2 pixel_size = (u_transform * vec4(1.0, 1.0, 0.0, 0.0)).xy * u_read_viewport.xy;

    // calculate distance from source
    vec2 dist = target_pos - source_pos;

    float pixel_dist = length(dist / pixel_size);

    if(pixel_dist >= light.radius)
        return 0.0;

    float decay = 1.0;

    if(light_type == LIGHT_TYPE_ARC)
    {
        vec2 target_angle = vec2(cos(light.pos.z), -sin(light.pos.z));
        float cosine_sim_thresh = (cos(light.pos.w) + 1.0) / 2.0;

        // allow negative cosine similarities if the angle spread is >180 degrees
        if(light.pos.w > acos(-1.0))
            cosine_sim_thresh = -cosine_sim_thresh;

        float cosine_sim = dot(normalize(dist / pixel_size), target_angle);

        if(cosine_sim < cosine_sim_thresh)
            return 0.0;
    }

    // squared decay
    return (light.radius * light.radius - (pixel_dist + 1.0) * (pixel_dist + 1.0)) / (light.radius * light.radius);
}
)RAW" // MSVC limits pre-concatenation length of strings to 16384 bytes, but C++ standard mandates support for up to 65536 bytes (after concatenation)
R"RAW(
float shadow(const in vec2 target_pos, const in float target_plane, const in vec2 source_pos, const in float source_plane)
{
    float shadow_base = u_shadow_strength < 1.0 ? 0.25 * log(1.0 - u_shadow_strength) : 0.0;

    // find out what the size of a pixel offset is
    vec2 pixel_size = (u_transform * vec4(1.0, 1.0, 0.0, 0.0)).xy * u_read_viewport.xy;

    // calculate distance from source
    vec2 dist = target_pos - source_pos;

    float pixel_dist = length(dist / pixel_size);


    // compute step count
    int step_count = int(floor(pixel_dist * shadowResolution));
    const int max_steps = 400;
    step_count = max(1, step_count);
    step_count = min(step_count, max_steps);

    vec2 step_offset = (target_pos - source_pos) / float(step_count);
    float accum_rate = 1.0 / float(step_count);


    // primary loop variables
    vec2  cur_pos = source_pos;
    float last_plane = floor(texture(u_depth_buffer, cur_pos).r * 32.0);

    float accum = 0.0;
    float mult = 0.0;

    float accum_conditional = 0.0; // accumulation for something currently at target's depth
    float mult_conditional  = 0.0; // shadow of something currently at target's depth


    // special cases:

    // if something is in front of the source, don't light it at all
    float source_occluder = (last_plane > source_plane) ? 1.0 : 0.0;
    const float source_occluder_dist = 14.0 * shadowResolution; // (will partially apply even if occluder starts up to 14px away)

    // don't cast shadows when going backwards from the source to the background
    float       long_term_plane  = source_plane;                // plane determining how shadows are cast, decreases from the source to the background
    const int   long_term_steps  = int(10.0 * shadowResolution); // number of steps required to decrease the long term plane (8px)
    int         last_switch_step = 0;                           // number of steps since last switch (used to decide when to update long_term_plane)

    // target shouldn't cast shadows on itself (fade these in)
    int         last_target_step       = 0;                            // time that last_accum_conditional and last_mult_conditional were most recently saved
    float       last_accum_conditional = 0.0;                          // accumulation for something previously at target's depth
    float       last_mult_conditional  = 0.0;                          // shadow of something previously at target's depth
    const int   self_shadow_fade       = int(12.0 * shadowResolution); // fade in target self-shadows from 2px to 12px


    for(int i = 0; i < step_count; i++)
    {
        cur_pos += step_offset;

        float plane_at = floor(texture(u_depth_buffer, cur_pos).r * 32.0);

        if(plane_at != last_plane)
        {
            if(float(i) < source_occluder_dist && plane_at > source_plane)
                source_occluder = 1.0 - float(i) / source_occluder_dist;
            else
                source_occluder = 0.0;

            if(last_plane < long_term_plane && i - last_switch_step >= long_term_steps)
                long_term_plane = last_plane;
            last_switch_step = i;

            if(plane_at == target_plane && i - last_target_step < self_shadow_fade)
            {
                // restore if it's only been a single step
                if(i - last_target_step < 6)
                {
                    accum_conditional = last_accum_conditional;
                    mult_conditional = last_mult_conditional;
                }
                else
                {
                    float self_shadow_amount = float(i - last_target_step) / float(self_shadow_fade);
                    mult += last_mult_conditional * self_shadow_amount;
                    accum_conditional = last_accum_conditional * (1.0 - self_shadow_amount);
                    mult_conditional = last_mult_conditional * (1.0 - self_shadow_amount);
                }
            }
            else
            {
                mult += last_mult_conditional;
                last_accum_conditional = accum_conditional;
                last_mult_conditional = mult_conditional;
                accum_conditional = 0.0;
                mult_conditional = 0.0;
                last_target_step = i;
            }

            last_plane = plane_at;
        }

        // no shadows from objects at background planes when the light source is a higher planes (and no shadows from effects)
        if(plane_at == long_term_plane || plane_at > 16.0 || plane_at < long_term_plane && plane_at < 7.0)
            accum += accum_rate;
        // BGOs only partially cast shadows on foreground
        else if(plane_at < 6.0 && target_plane >= 6.0)
            accum += accum_rate * 0.5;
        // target shouldn't self-shadow
        else if(plane_at == target_plane && source_occluder == 0.0)
        {
            mult_conditional += 1.0;
            accum_conditional += accum_rate;
        }
        else if(plane_at == target_plane)
        {
            mult_conditional += 1.0 - source_occluder;
            accum_conditional += accum_rate * (1.0 - source_occluder);
            mult += source_occluder;
        }
        else
            mult += 1.0;
    }

    // self-shadowed target gets most of its accumulator
    accum += accum_conditional;

    if(u_shadow_strength == 1.0 && mult != 0.0)
        return 0.0;
    else if(u_shadow_strength == 1.0)
        return accum;

    mult += mult_conditional * 0.25;

    return accum * exp(shadow_base * mult);
}

float shadow2(const in uint light_type, const in vec2 target_pos, const in float target_plane, const in vec2 source_pos, const in float source_plane, const in float source_depth)
{
    // depths used to calculate shadows
    const float c_shadow_dist = 8.0;

    // find out what the size of a pixel offset is
    vec2 pixel_size = (u_transform * vec4(1.0, 1.0, 0.0, 0.0)).xy * u_read_viewport.xy;

    // calculate distance from source
    vec2 dist = target_pos - source_pos;

    float pixel_dist = length(dist / pixel_size);

    vec2 dist_norm = normalize(dist);
    // vec2 dist_norm = dist / max(abs(dist.x), abs(dist.y));
    vec2 shadow_offset = dist_norm * c_shadow_dist * pixel_size;


    if(target_plane >= 16.0)
        return 1.0;

    // TODO: it would be nice to have the minimum and maximum depth of the source to use here
    float source_depth_thresh = (light_type == LIGHT_TYPE_BOX) ? 1.0 / 256.0 : 1.0 / (1024.0 * 128.0);

    float max_plane = (target_plane < 3.0) ? 3.0 :
                      (target_plane < 6.0) ? 6.0 :
                      16.0;

    // quick path for hard shadows
    if(u_shadow_strength >= 1.0)
    {
        vec2 shadow_pos = target_pos - shadow_offset;
        float depth_at_pos = texture(u_depth_buffer, shadow_pos).r;
        float plane_at_pos = floor(depth_at_pos * 32.0);
        if(plane_at_pos >= max_plane && abs(depth_at_pos - source_depth) >= source_depth_thresh)
            return 0.0;

        return 1.0;
    }

    float num = 0.0;
    float den = 0.0;

    float shadow_strength = max(u_shadow_strength, 0.01);

    float radius = ceil(-log(shadow_strength) * 5.0);
    radius = min(radius, 10.0);

    for(vec2 off = vec2(-radius); off.x <= radius; off.x += 1.0)
    {
        float ylim = radius - abs(off.x);

        for(off.y = -ylim; off.y <= ylim; off.y += 1.0)
        {
            vec2 shadow_pos = target_pos - shadow_offset + off * pixel_size;

            float weight = 1.0 / (length(off) + 1.0);

            weight = exp(log(weight) * shadow_strength);

            float depth_at_pos = texture(u_depth_buffer, shadow_pos).r;
            float plane_at_pos = floor(depth_at_pos * 32.0);
            if(plane_at_pos < max_plane || abs(depth_at_pos - source_depth) < source_depth_thresh)
                num += weight;

            den += weight;
        }
    }

    return num / den;
}

void main()
{
    vec3 total_color = vec3(0.0, 0.0, 0.0);

    if(u_system_type == SYSTEM_TYPE_DISABLED)
        return;

    vec2 pixel_floor = v_fbcoord - vec2(0.4999 / 800.0, 0.4999 / 600.0);
    float pixel_depth_TL = texture(u_depth_buffer, pixel_floor).r;
    float pixel_depth_TR = texture(u_depth_buffer, pixel_floor + vec2(1.0 / 800.0, 0.0)).r;
    float pixel_depth_BL = texture(u_depth_buffer, pixel_floor + vec2(0.0, 1.0 / 600.0)).r;
    float pixel_depth_BR = texture(u_depth_buffer, pixel_floor + vec2(1.0 / 800.0, 1.0 / 600.0)).r;
    float pixel_depth = min(min(min(pixel_depth_TL, pixel_depth_TR), pixel_depth_BL), pixel_depth_BR);
    float pixel_plane = floor(pixel_depth * 32.0);

    uint light_type;
    float best_rate = 0.0;
    float total_rate = 0.0;
    float source_count = 0.0;
    vec2 source_pos;
    float source_depth;
    float source_plane;

    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        if(u_lights[i].type == LIGHT_TYPE_NONE)
            break;

        if(u_lights[i].type != LIGHT_TYPE_DUP)
            light_type = u_lights[i].type;

        calculate_light_location(u_lights[i], light_type, v_fbcoord, source_pos, source_depth);
        source_plane = floor(source_depth * 32.0);

        float rate = calculate_light_falloff(u_lights[i], light_type, v_fbcoord, source_pos);
        rate *= fade_light(u_lights[i], light_type, source_pos);

        if(rate > 0.0)
        {
            if(u_system_type == SYSTEM_TYPE_SHADOW_RAYS)
                rate *= shadow(v_fbcoord, pixel_plane, source_pos, source_plane);
            else if(u_system_type == SYSTEM_TYPE_SHADOW_DROP)
                rate *= shadow2(light_type, v_fbcoord, pixel_plane, source_pos, source_plane, source_depth);

            if(rate > best_rate)
                best_rate = rate;

            total_rate += rate;
        }

        source_count += 1.0;

        // do hi-fi box lights
        if(light_type == LIGHT_TYPE_BOX && c_hifi_box_res != 0.0)
        {
            vec2 pixel_size = (u_transform * vec4(1.0, 1.0, 0.0, 0.0)).xy * u_read_viewport.xy;
            vec2 hifi_box_stride = pixel_size / c_hifi_box_res;

            vec4 source_pos_tl_raw = vec4(u_lights[i].pos.xy, u_lights[i].depth, 1.0);
            vec4 source_pos_tl_transform = u_transform * source_pos_tl_raw;
            vec2 source_pos_tl = source_pos_tl_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

            vec4 source_pos_br_raw = vec4(u_lights[i].pos.zw, u_lights[i].depth, 1.0);
            vec4 source_pos_br_transform = u_transform * source_pos_br_raw;
            vec2 source_pos_br = source_pos_br_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

            // top side
            if(v_fbcoord.y < source_pos_tl.y)
            {
                for(vec2 test_pos = source_pos_tl; test_pos.x <= source_pos_br.x; test_pos.x += hifi_box_stride.x)
                {
                    float rate = calculate_light_falloff(u_lights[i], light_type, v_fbcoord, test_pos);

                    if(rate > best_rate)
                    {
                        rate *= shadow(v_fbcoord, pixel_plane, test_pos, source_plane);

                        if(rate > best_rate)
                            best_rate = rate;
                    }
                }
            }

            // bottom side
            if(v_fbcoord.y > source_pos_br.y)
            {
                for(vec2 test_pos = source_pos_br; test_pos.x >= source_pos_tl.x; test_pos.x -= hifi_box_stride.x)
                {
                    float rate = calculate_light_falloff(u_lights[i], light_type, v_fbcoord, test_pos);

                    if(rate > best_rate)
                    {
                        rate *= shadow(v_fbcoord, pixel_plane, test_pos, source_plane);

                        if(rate > best_rate)
                            best_rate = rate;
                    }
                }
            }

            // left side
            if(v_fbcoord.x < source_pos_tl.x)
            {
                for(vec2 test_pos = source_pos_tl; test_pos.y <= source_pos_br.y; test_pos.y += hifi_box_stride.y)
                {
                    float rate = calculate_light_falloff(u_lights[i], light_type, v_fbcoord, test_pos);

                    if(rate > best_rate)
                    {
                        rate *= shadow(v_fbcoord, pixel_plane, test_pos, source_plane);

                        if(rate > best_rate)
                            best_rate = rate;
                    }
                }
            }

            // right side
            if(v_fbcoord.x > source_pos_br.x)
            {
                for(vec2 test_pos = source_pos_br; test_pos.y >= source_pos_tl.y; test_pos.y -= hifi_box_stride.y)
                {
                    float rate = calculate_light_falloff(u_lights[i], light_type, v_fbcoord, test_pos);

                    if(rate > best_rate)
                    {
                        rate *= shadow(v_fbcoord, pixel_plane, test_pos, source_plane);

                        if(rate > best_rate)
                            best_rate = rate;
                    }
                }
            }
        }

        // only apply the light color once for each group of lights
        if(i == MAX_LIGHTS - 1 || u_lights[i + 1].type != LIGHT_TYPE_DUP)
        {
            if(source_count > 0.0)
            {
                float average_rate = total_rate / source_count;
                // float use_rate = (best_rate * 1.5 + average_rate) / 2.5;
                // float use_rate = pow(average_rate, 0.5);

                float use_rate = (light_type == LIGHT_TYPE_BOX) ? best_rate : total_rate;

                total_color += use_rate * light_color(u_lights[i].color);
                best_rate = 0.0;
                total_rate = 0.0;
                source_count = 0.0;
            }
        }
    }

    total_color += light_color(u_ambient_color);
    vec3 falloff = tanh(total_color);
    o_FragColor.rgb = min(falloff * 1.5, total_color);
    o_FragColor.a = 1.0;
}
)RAW";

#endif // #ifdef RENDERGL_HAS_SHADERS
