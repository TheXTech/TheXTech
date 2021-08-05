#ifdef __cplusplus
extern "C" {
#endif


#ifndef __VGL_RENDERER_H__
#define __VGL_RENDERER_H__


#if defined(__APPLE__) || defined(PC_BUILD) 
#include <GL/glew.h>

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/glu.h>
#endif

#include <GLFW/glfw3.h>
#else
#include <vitasdk.h>
#include <vitaGL.h>
#include <psp2/gxm.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>

#define VERTICES_PER_QUAD 4
#define VERTICES_PER_TRI 3
#define VERTICES_PER_PRIM VERTICES_PER_QUAD

#define VERTEX_POS_SIZE 2 // Number of elements in our position (2: (x, y))
#define VERTEX_TEXCOORD_SIZE 2 // Number of elements in our tex coords. (2: (s, v))
#define VERTEX_COLOR_SIZE 4 // The number of elements in our vertex color attribute. (4: (r, g, b, a))

#define VERTEX_POS_OFFSET 0 // How many elements into one block will we find this property?
#define VERTEX_TEXCOORD_OFFSET 2 // 2 elements in (after x, y)
#define VERTEX_COLOR_OFFSET 4 // 4 elements in (after x, y and s, v)

// How many components are in the _vert.
// for just x & y, it's 2. this is used for stride.
// In this case, we have (x, y) (s, v) and (r,g,b,a) available to us.
#define VERTEX_ATTR_ELEM_COUNT 8
#define MAX_VERTICES 8096 // TODO: This should be renamed to MAX_DRAWCALLS. We allocate our VBO with memory to fill MAX_VERTICES * sizeof(DrawCall)

#define VERTEX_ATTRIB_TOTAL_SIZE_1 (VERTEX_ATTR_ELEM_COUNT * sizeof(float)) + (sizeof(void *))

#include "vgl_renderer_types.h"

typedef unsigned int GLuint;

static clock_t start_time_s = 0;
static clock_t last_frame_time_s = 0;
static clock_t last_frame_time_consumed_s = 0; // cur_time - last_frame_time
static clock_t last_printf_time = 0;

static const char* GLINVALIDENUM = "GL_INVALID_ENUM";
static const char* GLINVALIDVALUE = "GL_INVALID_VALUE";
static const char* GLINVALIDOP = "GL_INVALID_OPERATION";
static const char* GLOUTOFMEM = "GL_OUT_OF_MEMORY";
static const char* GLSTACKUNDER = "GL_STACK_UNDERFLOW";
static const char* GLSTACKOVER = "GL_STACK_OVERFLOW";
static const char* GLUNKNOWN = "GL_UNKNOWN. Sorry.";

static inline void DEBUG_PRINT_OBJ_EX_DATA(struct _obj_extra_data *ex_data)
{
    if(ex_data == NULL) return;

    printf("\tEX_DATA\n\ttex ID: %d\n\tpiv: %.2f, %.2f\n\trot: %.2f, %.2f, %.2f\n\tscale: %.2f\n\n",
        ex_data->textureID,
        ex_data->piv_x,
        ex_data->piv_y,
        ex_data->rot_x,
        ex_data->rot_y,
        ex_data->rot_z,
        ex_data->scale
    );
}

static inline void GET_GL_ERROR(GLenum error, char* output)
{
    switch(error)
    {
    case GL_INVALID_ENUM:
        memcpy(output, GLINVALIDENUM, strlen(GLINVALIDENUM));
        break;
    case GL_INVALID_VALUE:
        memcpy(output, GLINVALIDVALUE, strlen(GLINVALIDVALUE));
        break;
    case GL_INVALID_OPERATION:
        memcpy(output, GLINVALIDVALUE, strlen(GLINVALIDOP));
        break;
    case GL_OUT_OF_MEMORY:
        memcpy(output, GLOUTOFMEM, strlen(GLOUTOFMEM));
        break;
    case GL_STACK_UNDERFLOW:
        memcpy(output, GLSTACKUNDER, strlen(GLSTACKUNDER));
        break;
    case GL_STACK_OVERFLOW:
        memcpy(output, GLSTACKOVER, strlen(GLSTACKOVER));
        break;
    }

    memcpy(output, GLUNKNOWN, strlen(GLUNKNOWN));
}

static inline void CHECK_GL_ERROR(char* prefix) 
{
    GLenum gl_error = 0;
    char error_buffer[128];
    if((gl_error = glGetError()) != GL_NO_ERROR)
    {
        GET_GL_ERROR(gl_error, error_buffer);
        printf("[%s] OPENGL ERROR: %s\n", prefix, error_buffer);
    }

#ifndef VITA
    memset(error_buffer, 0, sizeof(error_buffer));
    const char* _eb = (char*)error_buffer;
    if(glfwGetError(&_eb))
    {
        printf("\t\tGLFW: %s", error_buffer);
    }
#endif
}

static inline void _printGLVersion()
{
    GLenum enums[] = { GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_VENDOR, GL_RENDERER };

    const unsigned char* temp_gl_string;
    for(int i = 0; i < 4; i++)
    {
        temp_gl_string = glGetString(enums[i]);

        switch(enums[i])
        {
        case GL_VERSION:
            printf("\n\nOpenGL Version: %s\n", temp_gl_string);
            break;
        case GL_SHADING_LANGUAGE_VERSION:
#ifdef VITA
            printf("GLSL (CG Shaders for Vita) Ver: %s\n", temp_gl_string);
#else
            printf("GLSL Language Version: %s\n", temp_gl_string);
#endif
            break;
        case GL_VENDOR:
            printf("GL Vendor: %s\n", temp_gl_string);
            break;
        case GL_RENDERER:
            printf("GL Renderer: %s\n", temp_gl_string);
            break;
        }
    }

    free(enums);
}

// ------------------------------------- Functions for the public

// TODO: Combine the 3 inits into one init function?
// TODO: Function prefixes.

int initGL(void (*dbgPrintFn)(const char*, ...));
int initGLAdv();
int initGLShading();
int initGLShading2(char* vertex_shader, char* fragment_shader);
int deInitGL();

// TODO: Function prefixes for these.
void Vita_SetClearColor(float r, float g, float b, float a);
void Vita_Clear();
void Vita_Repaint();

int Vita_AddShaderPass(char* vert_shader, char* frag_shader, int order);

/// The most basic of draw functions. Draws a white square at a given point.
void Vita_Draw(float x, float y, float wDst, float hDst);

/**
 * Vita_DrawRectColor():
 *  Draws a colored rect of a given wDst and hDst.
 */
void Vita_DrawRectColor(float x, float y,
                        float wDst, float hDst,
                        float _r,
                        float _g,
                        float _b,
                        float _a
);

/**
 * Vita_DrawRect4xColor():
 *  Draws a colored rect of a given wDst and hDst
 *  with a unique rgba color for each vertex.
 */
void Vita_DrawRect4xColor(float x, float y,
                          float wDst, float hDst,
                          float rgba0[4],
                          float rgba1[4],
                          float rgba2[4],
                          float rgba3[4]
);

/**
 * Vita_DrawRectColorExData():
 *  Draws a colored rect with the option of passing in 
 *  pointer to `ex_data` for scale, rotation, and pivot data.
 */
void Vita_DrawRectColorExData(float x, float y,
                           float wDst, float hDst,
                           float _r,
                           float _g,
                           float _b,
                           float _a,
                           struct _obj_extra_data *ex_data
);

/**
 * Vita_DrawTextureAnimColorExData():
 * 
 *  Draws a sub-sprite from a given texId.
 *  The sample area is defined by passing total tex_w & tex_h,
 *  along with src_x/src_y & src_w/src_h.
 * 
 *  The tex_w & tex_h is used to normalize the given src coordinates
 *  into graphics texture space.
 * 
 *  You can also specify a pointer to `ex_data` for setting
 *  scaling, rotation, and pivot data. 
 */
void Vita_DrawTextureAnimColorExData(
    float x,
    float y,
    float wDst,
    float hDst,
    GLuint texId,
    float tex_w,
    float tex_h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    float _r,
    float _g,
    float _b,
    float _a,
    struct _obj_extra_data *ex_data
);

/**
 * Vita_DrawTextureAnimColor():
 *  Draws a sub sprite from a given texId.
 *  
 *  Sample area is defined by passing total tex_w/tex_h
 *  along with src_x/src_y & src_w/src_h.
 *  
 *  tex_w and tex_h are used to normalize the src coordinates
 *  into texture coord space.
 *  
 *  A tint may be specified by passing color. 
 *  However, for scale & rotation, see @ Vita_DrawTextureAnimColorExData.
 */
void Vita_DrawTextureAnimColor(
    float x,
    float y,
    float wDst,
    float hDst,
    GLuint texId,
    float tex_w,
    float tex_h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    float _r,
    float _g,
    float _b,
    float _a
);

// ------------------------------------- End Functions for the public

#endif //__VGL_RENDERER_H__

#ifdef __cplusplus
}
#endif
