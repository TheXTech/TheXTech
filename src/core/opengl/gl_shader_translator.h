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

#pragma once
#ifndef GL_SHADER_TRANSLATOR_H

#include <string>
#include <utility>

typedef unsigned int    GLenum;

namespace XTechShaderTranslator
{

#ifdef THEXTECH_USE_ANGLE_TRANSLATOR

bool Inited();
void EnsureInit();
void SetOpenGLVersion(int major_version, int minor_version);
void TranslateShader(std::string& out, const char* in, GLenum type);
void EnsureQuit();

#else

static constexpr bool Inited() { return false; }
static inline void EnsureInit() {}
static inline void SetOpenGLVersion(int, int) {}
static inline void TranslateShader(std::string&, const char*, GLenum) {}
static inline void EnsureQuit() {}

#endif

} // namespace XTechShaderTranslator

#endif // #ifndef GL_SHADER_TRANSLATOR_H
