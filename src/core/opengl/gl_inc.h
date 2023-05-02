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
#ifndef RENDERGL_INC_H
#define RENDERGL_INC_H

// this auxiliary header includes the correct OpenGL headers, and sets defines based on which functions are available at compile time

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN

#    if defined(_WIN32) || defined(_WIN64)
#        define NO_SDL_GLEXT
#        include <GL/glew.h>
#    elif defined(__SWITCH__)
#        define NO_SDL_GLEXT
#        include <glad/glad.h>
#    else
#        define GL_GLEXT_PROTOTYPES 1
#    endif

#    include <SDL2/SDL_opengl.h>
#    include <SDL2/SDL_opengl_glext.h>

#    if !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64)
#        define RENDERGL_HAS_DEBUG
#    endif

#    define RENDERGL_HAS_LOGICOP
#    define RENDERGL_HAS_SHADERS
#    define RENDERGL_HAS_VAO
#    define RENDERGL_HAS_VBO
#    define RENDERGL_HAS_FBO

#    define RENDERGL_SUPPORTED
#endif

#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
#    include <SDL2/SDL_opengl.h>

#    define RENDERGL_HAS_LOGICOP
#    define RENDERGL_HAS_ORTHO
#    define RENDERGL_HAS_FIXED_FUNCTION

#    define RENDERGL_SUPPORTED
#endif

#ifdef THEXTECH_BUILD_GL_ES_MODERN
#    include <SDL2/SDL_opengles2.h>

#    ifdef __ANDROID__

#        define GL_UNIFORM_BUFFER       0x8A11
#        define GL_DRAW_FRAMEBUFFER     0x8CA9

// defined and loaded in render_gl_init.cpp

extern GL_APICALL void (* GL_APIENTRY glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
extern GL_APICALL void (* GL_APIENTRY glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

#    else
#        include <GLES3/gl3.h>
#    endif

#    define RENDERGL_HAS_SHADERS
#    define RENDERGL_HAS_VBO
#    define RENDERGL_HAS_FBO

#    define RENDERGL_SUPPORTED
#endif

#ifdef THEXTECH_BUILD_GL_ES_LEGACY
#    include <SDL2/SDL_opengles.h>

#    define RENDERGL_HAS_LOGICOP
#    define RENDERGL_HAS_ORTHOF
#    define RENDERGL_HAS_FIXED_FUNCTION
#    define RENDERGL_HAS_VBO

#    define RENDERGL_SUPPORTED
#endif

#endif // #ifndef RENDERGL_INC_H
