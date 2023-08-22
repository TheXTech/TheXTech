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

#include "core/opengl/render_gl.h"

#ifdef RENDERGL_HAS_SHADERS

const char* const RenderGL::s_es2_standard_vert_src =
R"RAW(#version 100
uniform   mat4 u_transform;

attribute vec4 a_position;
attribute vec2 a_texcoord;
attribute vec4 a_tint;

varying   vec2 v_texcoord;
varying   vec4 v_tint;

void main()
{
    gl_Position = u_transform * a_position;
    v_texcoord = a_texcoord;
    v_tint = a_tint;
}
)RAW";


const char* const RenderGL::s_es2_advanced_vert_src =
R"RAW(#version 100
uniform   mat4 u_transform;
uniform   vec4 u_read_viewport;

attribute vec4 a_position;
attribute vec2 a_texcoord;
attribute vec4 a_tint;

varying   vec2 v_texcoord;
varying   vec2 v_fbcoord;
varying   vec4 v_tint;

void main()
{
    gl_Position = u_transform * a_position;
    v_texcoord = a_texcoord;
    v_tint = a_tint;
    v_fbcoord = vec2(gl_Position);
    v_fbcoord *= u_read_viewport.xy;
    v_fbcoord += u_read_viewport.zw;
}
)RAW";


const char* const RenderGL::s_es3_advanced_vert_src =
R"RAW(#version 300 es

uniform   mat4 u_transform;
uniform   vec4 u_read_viewport;

in vec4 a_position;
in vec2 a_texcoord;
in vec4 a_tint;

out   vec2 v_texcoord;
out   vec2 v_fbcoord;
out   vec4 v_tint;

void main()
{
    gl_Position = u_transform * a_position;
    v_texcoord = a_texcoord;
    v_tint = a_tint;
    v_fbcoord = vec2(gl_Position);
    v_fbcoord *= u_read_viewport.xy;
    v_fbcoord += u_read_viewport.zw;
}
)RAW";


const char* const RenderGL::s_es2_standard_frag_src =
R"RAW(#version 100
precision mediump float;
varying   vec2      v_texcoord;
uniform   sampler2D u_texture;
varying   vec4      v_tint;
const     float     c_alpha_test_thresh = (8.0 / 255.0);
void main()
{
  vec4 l_color = texture2D(u_texture, v_texcoord);
  if(l_color.a < c_alpha_test_thresh) discard;
  gl_FragColor = v_tint * l_color;
}
)RAW";

const char* const RenderGL::s_es2_bitmask_frag_src =
R"RAW(#version 100

precision mediump float;

varying   vec2      v_texcoord;
varying   vec2      v_fbcoord;
varying   vec4      v_tint;

uniform   sampler2D u_texture;
uniform   sampler2D u_framebuffer;
uniform   sampler2D u_mask;

vec3 bitwise_ops(vec3 x, vec3 y, vec3 z)
{
    x *= 255.0;
    x /= 2.0;
    vec3 x_1 = fract(x);
    x -= x_1;
    x /= 2.0;
    vec3 x_2 = fract(x);
    x -= x_2;
    x /= 2.0;
    vec3 x_4 = fract(x);
    x -= x_4;
    x /= 2.0;
    vec3 x_8 = fract(x);
    x -= x_8;
    x /= 2.0;
    vec3 x_16 = fract(x);
    x -= x_16;
    x /= 2.0;
    vec3 x_32 = fract(x);
    x -= x_32;
    x /= 2.0;
    vec3 x_64 = fract(x);
    x -= x_64;
    x /= 2.0;
    vec3 x_128 = x;

    y *= 255.0;
    y /= 2.0;
    vec3 y_1 = fract(y);
    y -= y_1;
    y /= 2.0;
    vec3 y_2 = fract(y);
    y -= y_2;
    y /= 2.0;
    vec3 y_4 = fract(y);
    y -= y_4;
    y /= 2.0;
    vec3 y_8 = fract(y);
    y -= y_8;
    y /= 2.0;
    vec3 y_16 = fract(y);
    y -= y_16;
    y /= 2.0;
    vec3 y_32 = fract(y);
    y -= y_32;
    y /= 2.0;
    vec3 y_64 = fract(y);
    y -= y_64;
    y /= 2.0;
    vec3 y_128 = y;

    z *= 255.0;
    z /= 2.0;
    vec3 z_1 = fract(z);
    z -= z_1;
    z /= 2.0;
    vec3 z_2 = fract(z);
    z -= z_2;
    z /= 2.0;
    vec3 z_4 = fract(z);
    z -= z_4;
    z /= 2.0;
    vec3 z_8 = fract(z);
    z -= z_8;
    z /= 2.0;
    vec3 z_16 = fract(z);
    z -= z_16;
    z /= 2.0;
    vec3 z_32 = fract(z);
    z -= z_32;
    z /= 2.0;
    vec3 z_64 = fract(z);
    z -= z_64;
    z /= 2.0;
    vec3 z_128 = z;

    vec3 o_1 = max(min(x_1, y_1), z_1);
    vec3 o_2 = max(min(x_2, y_2), z_2);
    vec3 o_4 = max(min(x_4, y_4), z_4);
    vec3 o_8 = max(min(x_8, y_8), z_8);
    vec3 o_16 = max(min(x_16, y_16), z_16);
    vec3 o_32 = max(min(x_32, y_32), z_32);
    vec3 o_64 = max(min(x_64, y_64), z_64);
    vec3 o_128 = max(min(x_128, y_128), z_128);

    return (o_1 + o_2 * 2.0 + o_4 * 4.0 + o_8 * 8.0 + o_16 * 16.0 + o_32 * 32.0 + o_64 * 64.0 + o_128 * 128.0) * 2.0 / 255.0;
}

void main()
{
  vec3 l_image = texture2D(u_texture, v_texcoord).rgb;
  vec3 l_mask = texture2D(u_mask, v_texcoord).rgb;

  if(l_image.r == 0.0 && l_image.g == 0.0 && l_image.b == 0.0 && l_mask.r == 1.0 && l_mask.g == 1.0 && l_mask.b == 1.0)
    discard;

  // l_image *= v_tint.rgb;

  vec2 src = v_fbcoord.xy;
  // src.y += sin(v_fbcoord.x * 6.0 + l_mask.r + l_mask.g + l_mask.b) * (l_image.r + l_image.g + l_image.b + 3.0 - l_mask.r - l_mask.g - l_mask.b) / 9.0;

  vec3 l_bg = texture2D(u_framebuffer, src).rgb;

  gl_FragColor.rgb = bitwise_ops(l_bg, l_mask, l_image);
  gl_FragColor.a = 1.0;
}
)RAW";

const char* const RenderGL::s_es3_bitmask_frag_src =
R"RAW(#version 300 es

precision mediump float;

in   vec2      v_texcoord;
in   vec2      v_fbcoord;
in   vec4      v_tint;

uniform   sampler2D u_texture;
uniform   sampler2D u_framebuffer;
uniform   sampler2D u_mask;

out vec4 FragColor;

vec3 bitwise_ops(vec3 x, vec3 y, vec3 z)
{
    uvec3 x_u = uvec3(x * 255.9);
    uvec3 y_u = uvec3(y * 255.9);
    uvec3 z_u = uvec3(z * 255.9);

    return vec3((x_u & y_u) | z_u) / 255.0;
}

void main()
{
  vec3 l_image = texture(u_texture, v_texcoord).rgb;
  vec3 l_mask = texture(u_mask, v_texcoord).rgb;

  if(l_image.r == 0.0 && l_image.g == 0.0 && l_image.b == 0.0 && l_mask.r == 1.0 && l_mask.g == 1.0 && l_mask.b == 1.0)
    discard;

  // l_image *= v_tint.rgb;

  vec2 src = v_fbcoord.xy;

  vec3 l_bg = texture(u_framebuffer, src).rgb;

  FragColor.rgb = bitwise_ops(l_bg, l_mask, l_image);
  FragColor.a = 1.0;
}
)RAW";

const char* const RenderGL::s_es2_rect_filled_frag_src =
R"RAW(#version 100
precision mediump float;
varying   vec2      v_texcoord;
varying   vec4      v_tint;
void main()
{
  gl_FragColor = v_tint;
}
)RAW";

const char* const RenderGL::s_es2_rect_unfilled_frag_src =
R"RAW(#version 100
precision mediump float;
varying   vec2      v_texcoord;
varying   vec4      v_tint;
void main()
{
  if(v_texcoord.x >= 0.0 && v_texcoord.x < 1.0
     && v_texcoord.y >= 0.0 && v_texcoord.y < 1.0)
    discard;
  gl_FragColor = v_tint;
}
)RAW";

const char* const RenderGL::s_es2_circle_frag_src =
R"RAW(#version 100
precision mediump float;
varying   vec2      v_texcoord;
varying   vec4      v_tint;
void main()
{
  if((v_texcoord.x - 0.5) * (v_texcoord.x - 0.5)
    + (v_texcoord.y - 0.5) * (v_texcoord.y - 0.5) > 0.25)
      discard;
  gl_FragColor = v_tint;
}
)RAW";

const char* const RenderGL::s_es2_circle_hole_frag_src =
R"RAW(#version 100
precision mediump float;
varying   vec2      v_texcoord;
varying   vec4      v_tint;
void main()
{
  if((v_texcoord.x - 0.5) * (v_texcoord.x - 0.5)
    + (v_texcoord.y - 0.5) * (v_texcoord.y - 0.5) <= 0.25)
      discard;
  gl_FragColor = v_tint;
}
)RAW";

const char* const RenderGL::s_es3_lighting_frag_src =
R"RAW(#version 300 es

//precision mediump float;
precision highp float;

out vec4 o_FragColor;

in   vec2  v_texcoord;
in   vec2  v_fbcoord;

uniform highp mat4 u_transform;
uniform highp vec4 u_read_viewport;

uniform   sampler2D u_depth_buffer;

uniform   float u_clock;


const float shadowSoftness = 0.95;
//const float shadowSoftness = 0.5;
//const float shadowSoftness = 0.0;
const float shadowResolution = 0.5;


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
     Light u_lights[64];
};

vec3 shadow(const in Light light, const in vec2 pixel_pos, const in float pixel_depth)
{
    vec4 source_pos_raw = vec4(light.pos.xy, light.depth, 1.0);
    vec4 source_pos_transform = u_transform * source_pos_raw;
    vec2 source_pos = source_pos_transform.xy * u_read_viewport.xy + u_read_viewport.zw;

    float source_depth = source_pos_transform.z / 2.0 + 0.5;

    source_depth = floor(source_depth * 65535.0 + 0.5) / 65535.0;

    vec2 dist = pixel_pos - source_pos;

    float pixel_dist = length(dist * vec2(800.0, 600.0));

    if(pixel_dist > light.radius)
        return vec3(0.0);

    vec3 color = vec3((light.color >> 16) & 255U, (light.color >> 8) & 255U, (light.color >> 0) & 255U) / 255.0;

    // squared decay
    color *= (light.radius * light.radius - (pixel_dist + 1.0) * (pixel_dist + 1.0)) / (light.radius * light.radius);

    int step_count = int(floor(pixel_dist * shadowResolution));
    const int max_steps = 400;
    step_count = max(1, step_count);
    step_count = min(step_count, max_steps);

    vec2 step_offset = dist / float(step_count);

    vec2 step_pos = source_pos;

    if(shadowSoftness == 0.0)
    {
        for(int i = 0; i < step_count; i++)
        {
            step_pos += step_offset;

            float depth_at = texture(u_depth_buffer, step_pos).r;

            // gets blocked if something is between source and dest
            // special case: source does not block self

            // first, test: lights everything not in front of it

            if(pixel_depth < depth_at && depth_at != source_depth)
                return vec3(0.0);
        }

        return color;
    }

    float accum_rate = 1.0 / float(step_count);

    float accum = 0.0;
    float mult = 1.0;

    for(int i = 0; i < step_count; i++)
    {
        step_pos += step_offset;

        float depth_at = texture(u_depth_buffer, step_pos).r;

        if(pixel_depth < depth_at && depth_at != source_depth)
            mult *= shadowSoftness;
        else
            accum += accum_rate;
    }

    return color * accum * mult;
}

void main()
{
//    o_FragColor = texture(u_framebuffer, v_fbcoord);
    o_FragColor.rgb = vec3(0.5, 0.5, 0.5);

    vec2 pixel_floor = v_fbcoord - vec2(0.4999 / 800.0, 0.4999 / 600.0);
    float pixel_depth_TL = texture(u_depth_buffer, pixel_floor).r;
    float pixel_depth_TR = texture(u_depth_buffer, pixel_floor + vec2(1.0 / 800.0, 0.0)).r;
    float pixel_depth_BL = texture(u_depth_buffer, pixel_floor + vec2(0.0, 1.0 / 600.0)).r;
    float pixel_depth_BR = texture(u_depth_buffer, pixel_floor + vec2(1.0 / 800.0, 1.0 / 600.0)).r;
    float pixel_depth = min(min(min(pixel_depth_TL, pixel_depth_TR), pixel_depth_BL), pixel_depth_BR);

    for(int i = 0; i < 64; i++)
    {
        if(u_lights[i].type == 0U)
            break;

        o_FragColor.rgb += shadow(u_lights[i], v_fbcoord, pixel_depth);
    }

    o_FragColor.a = 1.0;
}
)RAW";

#endif
