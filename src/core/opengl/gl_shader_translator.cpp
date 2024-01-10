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

#include <GLSLANG/ShaderLang.h>
#include <Logger/logger.h>

#include "core/opengl/gl_inc.h"
#include "core/opengl/gl_shader_translator.h"

namespace XTechShaderTranslator
{

bool inited = false;
ShHandle vertex_compiler = 0;
ShHandle fragment_compiler = 0;
ShShaderOutput output_format = SH_GLSL_COMPATIBILITY_OUTPUT;

bool Inited()
{
    return inited && vertex_compiler && fragment_compiler;
}

void CloseCompilers()
{
    if(vertex_compiler)
    {
        sh::Destruct(vertex_compiler);
        vertex_compiler = 0;
    }

    if(fragment_compiler)
    {
        sh::Destruct(fragment_compiler);
        fragment_compiler = 0;
    }
}

void InitCompilers()
{
    CloseCompilers();

    if(!inited)
        return;

    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    resources.MaxDrawBuffers             = 8;
    resources.MaxVertexTextureImageUnits = 16;
    resources.MaxTextureImageUnits       = 16;

    vertex_compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES3_SPEC, output_format, &resources);
    fragment_compiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_GLES3_SPEC, output_format, &resources);

    if(!vertex_compiler || !fragment_compiler)
        pLogWarning("Failed to initialize ANGLE shader translator compiler objects. ESSL->GLSL translation will not be available.");
}

void EnsureInit()
{
    if(inited)
        return;

    inited = sh::Initialize();
    output_format = SH_GLSL_COMPATIBILITY_OUTPUT;

    if(!inited)
        pLogWarning("Failed to initialize ANGLE shader translator library. ESSL->GLSL translation will not be available.");

    InitCompilers();
}

void SetOpenGLVersion(int major_version, int minor_version)
{
    if(major_version < 3)
        output_format = SH_GLSL_COMPATIBILITY_OUTPUT;
    else if(major_version == 3)
    {
        if(minor_version == 0)
            output_format = SH_GLSL_130_OUTPUT;
        else if(minor_version == 1)
            output_format = SH_GLSL_140_OUTPUT;
        else if(minor_version == 2)
            output_format = SH_GLSL_150_CORE_OUTPUT;
        else
            output_format = SH_GLSL_330_CORE_OUTPUT;
    }
    else if(major_version == 4)
    {
        if(minor_version <= 0)
            output_format = SH_GLSL_400_CORE_OUTPUT;
        else if(minor_version >= 5)
            output_format = SH_GLSL_450_CORE_OUTPUT;
        else
            output_format = static_cast<ShShaderOutput>(SH_GLSL_400_CORE_OUTPUT + minor_version);
    }
    else // if(major_version > 4)
        output_format = SH_GLSL_450_CORE_OUTPUT;

    InitCompilers();

    if(vertex_compiler && fragment_compiler)
        pLogDebug("Initialized ANGLE shader translator as a backup to translate OpenGL ES shaders to desktop OpenGL %d.%d shaders.", major_version, minor_version);
}

void TranslateShader(std::string& out, const char* in, GLenum type)
{
    ShHandle compiler = (type == GL_VERTEX_SHADER) ? vertex_compiler : fragment_compiler;
    ShCompileOptions compileOptions = {};
    compileOptions.objectCode = true;
    compileOptions.initializeUninitializedLocals = true;

    bool success = sh::Compile(compiler, &in, 1, compileOptions);

    std::string log = sh::GetInfoLog(compiler);

    if(!success)
        pLogWarning("ShaderTranslator: failed to translate %s shader", type == GL_VERTEX_SHADER ? "vertex" : "fragment");

    if(!log.empty())
        pLogWarning("ShaderTranslator: %s shader translation log:\n%s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", log.c_str());

    if(success)
        out = sh::GetObjectCode(compiler);

    sh::ClearResults(compiler);
}

void EnsureQuit()
{
    CloseCompilers();

    if(inited)
        sh::Finalize();

    inited = false;
}

} // namespace XTechShaderTranslator
