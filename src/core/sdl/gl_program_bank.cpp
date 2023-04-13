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

#include "gl_program_bank.h"

#include "core/render.h"

#include "globals.h"
#include "global_dirs.h"

std::vector<StdPicture> LoadedGLProgram;

static std::map<std::string, LoadedGLProgramRef_t> s_ProgramCache;
static DirListCI s_dirShaders;

void ClearAllGLPrograms()
{
    // s_ProgramCache.clear();

    // totally unsafe until StdPicture cleanup, then totally safe!
    // LoadedProgram.clear();
}

LoadedGLProgramRef_t ResolveGLProgram(const std::string& frag_name)
{
    s_dirShaders.setCurDir(AppPath + "graphics/shaders");

    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);

    std::string resolved = g_dirCustom.resolveFileCaseExistsAbs(frag_name);

    if(resolved.empty())
        resolved = g_dirEpisode.resolveFileCaseExistsAbs(frag_name);

    if(resolved.empty())
        resolved = s_dirShaders.resolveFileCaseExistsAbs(frag_name);

    if(resolved.empty())
        return static_cast<LoadedGLProgramRef_t>(-1);

    auto it = s_ProgramCache.find(resolved);
    if(it != s_ProgramCache.end())
        return it->second;

    LoadedGLProgram.emplace_back();
    StdPicture& dst = LoadedGLProgram.back();

    dst = XRender::LoadPictureShader(resolved);

    if(!dst.inited)
    {
        LoadedGLProgram.pop_back();
        return static_cast<LoadedGLProgramRef_t>(-1);
    }

    s_ProgramCache[resolved] = dst;

    return dst;
}
