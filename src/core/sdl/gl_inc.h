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

#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
#    define GL_GLEXT_PROTOTYPES 1
#    include <SDL2/SDL_opengl.h>
#    include <SDL2/SDL_opengl_glext.h>

#    define RENDERGL_HAS_DEBUG
#    define RENDERGL_HAS_LOGICOP
#    define RENDERGL_HAS_ORTHO
#    define RENDERGL_HAS_SHADERS
#    define RENDERGL_HAS_FIXED_FUNCTION
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
