#ifndef _SHADERS_H_
#define _SHADERS_H_

#include <stdio.h>

static inline int _Vita_ReadShaderFromFile(const char* path, size_t* fsize, char** buffer)
{
    FILE *_file = fopen(path, "r");

    if(_file == NULL)
    {
        return -1;
    }

    fseek(_file, 0, SEEK_END);
    *fsize = ftell(_file);
    fseek(_file, 0, SEEK_SET);

    *buffer = realloc(*buffer, (*fsize) + 1);
    fread(*buffer, *fsize, 1, _file);

    size_t offset = *fsize;
    (*buffer)[offset] = '\0';

    fclose(_file);
    return 0;
}

#ifdef VITA

#if 1
static const char vShaderString[] = 
    "void main(\n"
    " float2 aPosition,\n"
    " float2 vTexCoord,\n"
    " float4 vColor,\n"
    " uniform float4x4 mvp,\n"
    " uniform float4x4 _rot,\n"
    " uniform float4x4 _scale,\n"
    " float4 out vertPos : POSITION,\n"
    " float4 out fragColor : COLOR,\n"
    " float2 out texCoord : TEXCOORD0\n"
    ")\n"
    "{\n"
    "   float4x4 temp = mul(mvp, _rot);\n"
    "   temp = mul(temp, _scale);\n"
    "   vertPos = mul(temp, float4(aPosition, 0.f, 1.f));\n"
    // "   vertPos = float4(aPosition.xy, 0.f, 1.f);\n"
    "   fragColor = vColor;\n"
    "   texCoord = vTexCoord;\n"
    "}\n";

static const char vFragmentString[] =   
    "float4 main(\n"
      "float4 fragColor : COLOR,\n"
      "float2 texCoord : TEXCOORD0\n"
    //   "uniform bool useTexture\n"
    //   "uniform sampler2D ourTexture\n"
    ") : COLOR\n"
    "{\n"
    "   "
    "   return float4(1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n";
#else

static const char vShaderString[] = 
    "void main(\n"
    " float3 aPosition,\n"
    " float4 out vPosition: POSITION)\n"
    "{\n"
    "vPosition = float4(aPosition, 1.f);\n"
    "}\n";

static const char vFragmentString[] =   
    "float4 main(uniform float4 color) : COLOR\n"
    "{return float4(1.0f, 0.0f, 0.0f, 1.0f);}\n";
#endif
#else



/*
Note to future self:

GLSL requires that any variables be used at least once. 
 If not, you won't be able to get their attrib ID.
*/

static const char vShaderString[] = 
     "attribute vec2 vPosition;    \n"
     "attribute vec2 vTexCoord;    \n"
     "attribute vec4 vColor;      \n"
     "varying vec4 fragColor;           \n"
     "varying vec2 texCoord;            \n"
     "uniform mat4 mvp;\n"
     "uniform mat4 _rot;\n"
     "uniform mat4 _scale;\n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = mvp * _rot * _scale * vec4(vPosition.xy, 0, 1);  \n"
      "   fragColor = vColor;          \n"
      "   texCoord = vTexCoord;         \n"
      "}                            \n";

static const char vFragmentString[] =   
      "varying vec4 fragColor;\n"
      "varying vec2 texCoord;\n"
      "uniform sampler2D ourTexture;"
      "uniform bool useTexture;"
      "void main()                                  \n"
      "{                                            \n"
      " if(useTexture)\n"
      "     gl_FragColor = texture2D(ourTexture, texCoord) * fragColor;\n"
      " else\n"
      "     gl_FragColor = fragColor;\n"
      "}           ";

#endif

#endif