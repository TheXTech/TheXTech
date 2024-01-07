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
#ifndef GL_PROGRAM_BANK_H

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "range_arr.hpp"
#include "ref_type.h"
#include "std_picture.h"
#include "global_constants.h"

// a bank to store loaded "pure" GL programs -- StdPictures whose only purpose is to execute a fragment shader

extern std::vector<std::unique_ptr<StdPicture>> LoadedGLProgram;

DECLREF_T(LoadedGLProgram);

// care must be taken to avoid dangling `LoadedGLProgramRef_t`s
void ClearAllGLPrograms();

// returns LoadedGLProgramRef_t() (evals to false) on failure
LoadedGLProgramRef_t ResolveGLProgram(const std::string& name);

// returns LoadedGLProgramRef_t() (evals to false) on failure
LoadedGLProgramRef_t ResolveGLParticleSystem(const std::string& name);

extern RangeArr<LoadedGLProgramRef_t, 0, maxSections> SectionEffect;
extern RangeArr<LoadedGLProgramRef_t, 0, maxSections> SectionParticlesBG;
extern RangeArr<LoadedGLProgramRef_t, 0, maxSections> SectionParticlesFG;
extern RangeArr<       GLLightSystem, 0, maxSections> SectionLighting;

#endif // #ifndef GL_PROGRAM_BANK_H
