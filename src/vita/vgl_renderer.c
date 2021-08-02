#ifdef __cplusplus
extern "C" {
#endif

#include "vgl_renderer.h"

#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>

#include "SHADERS.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef VITA
static GLFWwindow* _game_window;
#endif

// A pointer to the debug printf function.
// This must be set so as to avoid a crash.
static void (*_debugPrintf)(const char*, ...);

// The width of the display.
static int DISPLAY_WIDTH = 960;

// The height of the display.
static int DISPLAY_HEIGHT = 544;

// The attribute location index for the vertex position in the shader.
static int VERTEX_POS_INDEX = 0;

// Attribute location index for tex coordinates in the shader.
static int VERTEX_TEXCOORD_INDEX = 0;

// Attribute location index for color in the shader.
static int VERTEX_COLOR_INDEX = 0;

// UNIFFORM location index for the MVP in the shader.
static int VERTEX_MVP_INDEX = 3;

// Uniform location index for the rotation matrix in the shader.
static int UNIFORM_ROTMAT_INDEX = 4;

// Uniform location index for the scale matrix in the shader.
static int UNIFORM_SCALE_INDEX = 5;

// Uniform location index for the `useTexture` flag in the shader.
static int UNIFORM_USE_TEXTURE_BOOL_INDEX = 0;

// Uniform location index for the sampler2D that IS the texture in the shader.
// More than likely, this is set automatically just by calling `glBindBuffer`. 
// This will probably be removed.
static int UNIFORM_SAMPLER_TEXTURE_INDEX = 0;

static const GLuint VERTEX_SIZE = VERTEX_POS_SIZE + VERTEX_TEXCOORD_SIZE + VERTEX_COLOR_SIZE;

// ------------------------------------------   SHADERS
static DrawCall *_vgl_pending_calls; // ARRAY OF PENDING DRAW CALLS (DrawCall _vgl_pending_calls[MAX_VERTICES];)
static unsigned int _vgl_pending_offset; // INDEX
static size_t _vgl_pending_total_size; // SIZE IN BYTES
static unsigned int _DrawCalls = 0; // DRAW CALL COUNT
// ------------------------------------------ END SHADERS 

// ------------------------------------------ PASSES

// An array of passes. If any passes have a program ID 
// defined as 0, they will be skipped.
//
// There is always at least 1 shading pass, the default.
// It is put directly in the middle of the buffer to allow other passes
// to be drawn below & above it.
static ShadingPass _shading_passes[6];

// The total number of real passes in the buffer (real = program object ID != 0)
static unsigned int _ShadingPasses = 1;
// ------------------------------------------

// ------------------------------------------   BUFFERS

// The default ID of the VBO we'll be writing
// draw calls to.
static GLuint _vertexBufferID;
// ------------------------------------------ END BUFFERS

// ------------------------------------------   SHADERS

// The ID of the default vertex shader.
// This either the default embedded in SHADERS.h OR
// the shader specified when calling initGLShading2
static GLint vertexShaderID;

// The ID of the default fragment shader.
// This is either the default embedded in SHADERS.h OR
// the shader specified when calling initGLShading2
static GLint fragmentShaderID;

// The ID of the default shading program.
// This either the embedded default vert & frag shaders linked together
// OR the shaders specified in initGLShading2 linked together.
static GLint programObjectID;
// ------------------------------------------ END SHADERS

// ------------------------------------------   MAT BUFFERS

// The model view projection mat4 that each draw call will use for projection.
// By default, this is a glOrtho given the Vita's screen size.
mat4 cpu_mvp;

// The rotation identity matrix, for now, being stored.
mat4 _rot;

// An arbitrary rotation matrix buffer. This will be set and reset over and over again
// in a frame for each object. For objects that don't specify extra rotation data,
// this will mostly be the identity matrix.
mat4 _rot_arb;

// The scale identity matrix, for now, being stored.
mat4 _scale;

// An arbitrary scaling matrix buffer. This will be set and reset over and over again
// in a frame for each object. For objects that don't specify extra scaling data,
// this will mostly be the identity matrix.
mat4 _scale_arb;
// ------------------------------------------   END MAT BUFFERS


// ------------------------------------------   INTERNAL FUNCTIONS

static inline int _Vita_SortDrawCalls(const void *s1, const void *s2)
{
    DrawCall *dc1 = (DrawCall *)s1;
    DrawCall *dc2 = (DrawCall *)s2;

    if(dc1->verts[0].obj_ptr != NULL 
        && dc2->verts[0].obj_ptr != NULL)
    {
        return (((obj_extra_data *)dc1->verts[0].obj_ptr)->textureID) - (((obj_extra_data *)dc2->verts[0].obj_ptr)->textureID);
    }

    return +1;
}

/**
 * Vita_AddPass():
 *  Sets the information from the `passInfo` variable
 *  to the pass at the given order. 
 * 
 * Order is relative to the default pass which is CURRENTLY 
 * at index 2. If you want to add pass before the default pass,
 * (eg: hard drop shadows in 2D), you should set order to -1.
 * 
 * This will resolve as follows:
 * 
 * idx = (2 + order)
 * idx = 2 + -1;
 * idx = 1;
 */
static inline void Vita_AddPass(ShadingPass passInfo, int order)
{
    if(passInfo.ProgramObjectID <= 0) return;

    int idx = 2 + order;

    _debugPrintf("VITA_ADDPASS PROGRAM: %d at index: %d (specified order: %d)\n", passInfo.ProgramObjectID, idx, order);

    // (count - 1) - order
    if(idx < 0 || idx > 5)
    {
        return;
    }

    _shading_passes[idx].FragmentShaderID = passInfo.FragmentShaderID;
    _shading_passes[idx].ProgramObjectID = passInfo.ProgramObjectID;
    _shading_passes[idx].VertexShaderID = passInfo.VertexShaderID;
    _shading_passes[idx].offset_x = passInfo.offset_x;
    _shading_passes[idx].offset_y = passInfo.offset_y;

}

/**
 * _Vita_GetAvailableDrawCall():
 *  Returns the first available draw call, or if you've asked
 *  for too many, returns NULL (0);
 */
static inline DrawCall *_Vita_GetAvailableDrawCall()
{
    if((_vgl_pending_offset + 1) > _vgl_pending_total_size)
    {
        _debugPrintf("Ran out of draw calls. %d / %zu\n", _vgl_pending_offset, _vgl_pending_total_size);
        return NULL;
    }

    return (_vgl_pending_calls + _vgl_pending_offset);
}

/**
 * _Vita_DoneWithDrawCall():
 *  Concludes the draw call by incrementing 
 *  _vgl_pending_offset and _DrawCalls variables.
 */
static inline void _Vita_DoneWithDrawCall()
{
    _vgl_pending_offset += 1;
    _DrawCalls++;
}

/**
 * Writes the vertices into the given `drawCall`
 * based on the input coordinates. 
 * 
 * By passing x, y, w, h, we can determine where
 * all the vertices need to be for a given quad relatively easily.
 * 
 * n_src_x, n_src_x2
 * n_src_y, n_src_y2 form the coordinates that define the 
 * normalized texture coordinate rect for sampling textures.
 * 
 * In addition, each vertex can have its own color. However,
 * most of the time the colors are set the same for all 4 vertices.
 */
static inline void
_Vita_WriteVertices4xColor(DrawCall *drawCall,
                          float x,
                          float y,
                          float wDst, float hDst,
                          float n_src_x, float n_src_x2,
                          float n_src_y, float n_src_y2,
                          float rgba0[4], 
                          float rgba1[4], 
                          float rgba2[4],
                          float rgba3[4])
{
    if(drawCall == nullptr) return;

    drawCall->verts[0].x = x;
    drawCall->verts[0].y = y;
    drawCall->verts[0].s = n_src_x; // Tex Coord X
    drawCall->verts[0].v = n_src_y; // Tex Coord Y
    
    drawCall->verts[1].x = x;
    drawCall->verts[1].y = y + hDst;
    drawCall->verts[1].s = n_src_x; // Tex Coord X
    drawCall->verts[1].v = n_src_y2; // Tex Coord Y
    
    drawCall->verts[2].x = x + wDst;
    drawCall->verts[2].y = y;
    drawCall->verts[2].s = n_src_x2; // Tex Coord X
    drawCall->verts[2].v = n_src_y; // Tex Coord Y

    if(VERTICES_PER_PRIM == 4)
    {
        drawCall->verts[3].x = x + wDst;
        drawCall->verts[3].y = y + hDst;
        drawCall->verts[3].s = n_src_x2; // Tex Coord X
        drawCall->verts[3].v = n_src_y2; // Tex Coord Y
    }

    drawCall->verts[0]._r = rgba0[0];
    drawCall->verts[0]._g = rgba0[1];
    drawCall->verts[0]._b = rgba0[2];
    drawCall->verts[0]._a = rgba0[3];

    drawCall->verts[1]._r = rgba1[0];
    drawCall->verts[1]._g = rgba1[1];
    drawCall->verts[1]._b = rgba1[2];
    drawCall->verts[1]._a = rgba1[3];

    drawCall->verts[2]._r = rgba2[0];
    drawCall->verts[2]._g = rgba2[1];
    drawCall->verts[2]._b = rgba2[2];
    drawCall->verts[2]._a = rgba2[3];

    drawCall->verts[3]._r = rgba3[0];
    drawCall->verts[3]._g = rgba3[1];
    drawCall->verts[3]._b = rgba3[2];
    drawCall->verts[3]._a = rgba3[3];
}

/**
 * _Vita_WriteVertices():
 *  Calls @ _Vita_WriteVertices4xColor using
 *  the given rgba values for all 4 vertices.
 *  All other parameters are passed along.
 */
static inline void 
_Vita_WriteVertices(DrawCall *drawCall, 
                   float x, 
                   float y, 
                   float wDst, 
                   float hDst, 
                   float n_src_x, 
                   float n_src_x2,
                   float n_src_y,
                   float n_src_y2,
                   float _r,
                   float _g,
                   float _b,
                   float _a)
{
    float rgba0[4] = {_r, _g, _b, _a};
    _Vita_WriteVertices4xColor(drawCall, 
                              x, y, wDst, hDst, 
                              n_src_x, n_src_x2, n_src_y, n_src_y2, 
                              rgba0, rgba0, rgba0, rgba0);
}

static GLuint Vita_GetVertexBufferID() { return _vertexBufferID; }

unsigned int Vita_GetTotalCalls()
{
    return _DrawCalls;
}

void Vita_ResetTotalCalls()
{
    _DrawCalls = 0;
    _vgl_pending_offset = 0;
}

DrawCall *Vita_GetDrawCallsPending()
{
    return _vgl_pending_calls;
}

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;
    
    shader = glCreateShader(type);

    if(shader == 0)
    { 
        _debugPrintf("!!!!!!!   Unable to create shader: shader returned ID of %d\n", shader);
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {   
        
        _debugPrintf("%s NOT COMPILED.\n", (type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader"));
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char *infoLog = (char*)malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            _debugPrintf("\n\nError Compiling Shader:\n\n%s\n", infoLog);
            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// ------------------------------------------   END INTERNAL FUNCTIONS

// ------------------------------------------   EXPOSED 2D DRAW FUNCTIONS

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
                          float rgba3[4])
{
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    if(_curDrawCall == 0) {return;}

    for(int i = 0; i < 4; i++)
        _curDrawCall->verts[i].obj_ptr = 0;

    _Vita_WriteVertices4xColor(_curDrawCall, x, y, wDst, hDst, 0.f, 1.f, 0.f, 1.f, rgba0, rgba1, rgba2, rgba3);

    _Vita_DoneWithDrawCall();
}


/**
 * Vita_DrawRectColor():
 *  Draws a colored rect of a given wDst and hDst.
 */
void Vita_DrawRectColor(float x, float y,
                        float wDst, float hDst,
                        float _r, 
                        float _g,
                        float _b, 
                        float _a)
{
    float rgba0[4] = {_r, _g, _b, _a};
    Vita_DrawRect4xColor(x, y, wDst, hDst, rgba0, rgba0, rgba0, rgba0);
}

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
                           obj_extra_data *ex_data)
{
    float rgba0[4] = {_r, _g, _b, _a};
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();

    if(ex_data != NULL)
        ex_data->textureID = 0;

    _curDrawCall->verts[0].obj_ptr = ex_data;
    _curDrawCall->verts[1].obj_ptr = ex_data;
    _curDrawCall->verts[2].obj_ptr = ex_data;
    _curDrawCall->verts[3].obj_ptr = ex_data;


    // _curDrawCall->scale = 1.0f;
    // _curDrawCall->rot_x = 0;
    // _curDrawCall->rot_y = 0;
    // _curDrawCall->rot_z = rot;
    // _curDrawCall->piv_x = x + (wDst * .5f);
    // _curDrawCall->piv_y = y + (hDst * .5f);

    _Vita_WriteVertices4xColor(_curDrawCall, x, y, wDst, hDst, 1.f, 1.f, 1.f, 1.f, rgba0, rgba0, rgba0, rgba0);

    _Vita_DoneWithDrawCall();

}

/**
 * Vita_Draw():
 *  You probably don't want this function.
 *  Simply draws a white quad on screen with the
 *  given wDst & hDst.
 */
void Vita_Draw(float x,
               float y,
               float wDst,
               float hDst)
{
    
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    
    for(int i = 0; i < VERTICES_PER_PRIM; i++)
        _curDrawCall->verts[i].obj_ptr = NULL;
#if 0
    _curDrawCall->scale = 1.0f;
#endif

    _Vita_WriteVertices(_curDrawCall, x, y, wDst, hDst, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
    
    _Vita_DoneWithDrawCall();
}

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
        obj_extra_data *ex_data)
{

    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    if(_curDrawCall == NULL) return;

    if(ex_data != NULL && ex_data->textureID != texId)
    {
        if(texId == 0) _debugPrintf("WARNING: Draw Texture called without texture passed.");
        ex_data->textureID = texId;
    }

    for(int i = 0; i < VERTICES_PER_PRIM; i++)
        _curDrawCall->verts[i].obj_ptr = (void*)ex_data;
    
#if 0
    _curDrawCall->piv_x = x + (wDst * .5f);
    _curDrawCall->piv_y = y + (hDst * .5f);
#endif

    float n_src_x = (src_x / tex_w);
    float n_src_x2 = ((src_x + src_w) / tex_w);
    float n_src_y = (src_y / tex_h);
    float n_src_y2 = ((src_y + src_h) / tex_h);

    _Vita_WriteVertices(
        _curDrawCall, 
        x, 
        y, 
        wDst, 
        hDst, 
        n_src_x, 
        n_src_x2, 
        n_src_y, 
        n_src_y2, 
        _r, _g, _b, _a);
    
    _Vita_DoneWithDrawCall();

    // Vita_DrawTextureAnimColorRotScale(x, y, wDst, hDst, texId, tex_w, tex_h, src_x, src_y, src_w, src_h, _r, _g, _b, _a, _rot, 1.f);
}

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
        float _a)
{
    Vita_DrawTextureAnimColorExData(x, y, wDst, hDst, texId, tex_w, tex_h, src_x, src_y, src_w, src_h, _r, _g, _b, _a, NULL);
}

// ------------------------------------------   END EXPOSED 2D DRAW FUNCTIONS

// ------------------------------------------ ADDING PASSES

/**
 * Vita_AddShaderPass():
 *  Adds a shader pass with the given shader source text
 *  of vert_shader and frag_shader. 
 *  The pass will be placed relative to index 2 in the list. 
 * 
 * 
 *  vert_shader and frag_shader are allowed to be NULL.
 *  However, if both are NULL, the pass will not be added.
 *  If either are NULL, their default program ID will be substituted.
 * 
 *  This makes it possible to add a pass with the exact same vertex shader,
 *  but alternative fragment shader.
 */
int Vita_AddShaderPass(char* vert_shader, char* frag_shader, int order)
{
    int add_idx = 3 + order;
    if(add_idx < 0 || add_idx > 5)
        return -1;
    
    _debugPrintf
        ("Adding new custom pass at %d (%d index). Vert Shader Custom: %s; Frag Shader Custom: %s\n", 
        order, 
        add_idx, 
        (vert_shader == NULL ? "No" : "Yes!"),
        (frag_shader == NULL ? "No" : "Yes!")
    );

    // New vert shader.
    GLuint _newProgVertShader;
    if(vert_shader == NULL)
        _newProgVertShader = vertexShaderID;
    else
    {
        _newProgVertShader = LoadShader(GL_VERTEX_SHADER, vert_shader);
        if(_newProgVertShader == 0)
            return -1;
    }
    CHECK_GL_ERROR("Vita_AddShaderPass newProgVertShader");
    
    // New frag shader.
    GLuint _newProgFragShader;
    if(frag_shader == NULL)
        _newProgFragShader = fragmentShaderID;
    else
    {
        _newProgFragShader = LoadShader(GL_FRAGMENT_SHADER, frag_shader);
        if(_newProgFragShader == 0)
            return -1;
    }
    CHECK_GL_ERROR("Vita_AddShaderPass newProgFragShader");
    
    // Linking
    GLuint _newProgProgram = glCreateProgram();
    glAttachShader(_newProgProgram, _newProgVertShader);
    glAttachShader(_newProgProgram, _newProgFragShader);

    glLinkProgram(_newProgProgram);
    CHECK_GL_ERROR("Vita_AddShaderPass link newProgProgram");

    GLint linked;
    glGetProgramiv(_newProgProgram, GL_LINK_STATUS, &linked);
    CHECK_GL_ERROR("Vita_AddShaderPass glGetProgramiv");

    if(!linked)
    {
        _debugPrintf("!!!!! ERROR: Could not link shader.\n");
        GLint length;
	    glGetProgramiv(_newProgProgram,GL_INFO_LOG_LENGTH,&length);
	    char* log = (char*)malloc(length);
		
	    glGetProgramInfoLog(_newProgProgram,200,&length,log);

        _debugPrintf("Error Message: %s\n", log);


        if(vert_shader != NULL)
            glDeleteShader(_newProgVertShader);
        
        if(frag_shader != NULL)
            glDeleteShader(_newProgFragShader);
        
        glDeleteProgram(_newProgProgram);
        
        return -1;
    }

    // 
    ShadingPass passInfo;
    passInfo.offset_x = 4.f;
    passInfo.offset_y = 4.f;
    passInfo.ProgramObjectID = _newProgProgram;
    passInfo.VertexShaderID = _newProgVertShader;
    passInfo.FragmentShaderID = _newProgFragShader;

    Vita_AddPass(passInfo, order);
    return 0;
}

// ------------------------------------------ END PASSES

// ------------------------------------------    INIT FUNCTIONS

/**
 * initGLShading2():
 *  Initializes OpenGL/vitaGL shader support.
 * 
 *  The following will happen:
 *    - The shader source passed in _vShaderString and _fShaderString
 *      will be loaded into OpenGL.
 *    - If they pass, a program will be created and these shaders will
 *      be linked.
 *    - If the program is created successfully, then the proper attribute
 *      and uniform indices will be retrieved from the program.
 *  
 *  returns 0 if the function completed successfully
 */
int initGLShading2(char* _vShaderString, char* _fShaderString)
{
    _debugPrintf("(NOTE): Init GL Shading 2. Initializing GL Shading with shader strings passed to us externally (by the user).\n");
    vertexShaderID = LoadShader(GL_VERTEX_SHADER, _vShaderString);
    if(vertexShaderID == 0)
    {
        _debugPrintf("ERROR: vertex shader ID: %d\n", vertexShaderID);
        return -1;
    }
    else _debugPrintf("V Shader ID: %d\n", vertexShaderID);

    CHECK_GL_ERROR("Vertex Shader");

    fragmentShaderID = LoadShader(GL_FRAGMENT_SHADER, _fShaderString);
    if(fragmentShaderID == 0)
    {
        _debugPrintf("ERROR: frag shader ID: %d\n", fragmentShaderID);
        return -1;
    }
    else _debugPrintf("F Shader ID: %d\n", fragmentShaderID);

    CHECK_GL_ERROR("Frag Shader");

    programObjectID = glCreateProgram();
    CHECK_GL_ERROR("Make Program Shader");

    if(programObjectID == 0)
    {
        _debugPrintf("ERROR: Program object is 0.\n");
        return -1;
    }

    CHECK_GL_ERROR("Pre Shader Attach");
    glAttachShader(programObjectID, vertexShaderID);
    CHECK_GL_ERROR("Vertex Shader Attach");

    glAttachShader(programObjectID, fragmentShaderID);  
    CHECK_GL_ERROR("Frag Shader Attach");

    glLinkProgram(programObjectID);
    CHECK_GL_ERROR("LINK PROGRRAM");

    GLint linked;
    glGetProgramiv(programObjectID, GL_LINK_STATUS, &linked);
    CHECK_GL_ERROR("Get Program Info Value");

    if(!linked)
    {
        // TODO: proper error.
        _debugPrintf("!!!!!! FAILED TO LINK SHADER!\n");
        return -1;
    }
    
#ifdef VITA
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "aPosition");
#else
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "vPosition"); // Vertex position.
#endif
    VERTEX_TEXCOORD_INDEX = glGetAttribLocation(programObjectID, "vTexCoord"); // Vertex Tex Coord.
    VERTEX_COLOR_INDEX = glGetAttribLocation(programObjectID, "vColor"); // Gets passed to the fragment shader.

    // Uniforms
    VERTEX_MVP_INDEX = glGetUniformLocation(programObjectID, "mvp"); // MVP matrix. In our case, this is an ortho matrix for the Vita's screen.

    UNIFORM_ROTMAT_INDEX = glGetUniformLocation(programObjectID, "_rot");
    UNIFORM_SCALE_INDEX = glGetUniformLocation(programObjectID, "_scale");
    
    UNIFORM_USE_TEXTURE_BOOL_INDEX = glGetUniformLocation(programObjectID, "useTexture");

    glm_mat4_identity(_rot);
    glm_mat4_identity(_rot_arb);

    glm_mat4_identity(_scale);
    glm_mat4_identity(_scale_arb);

    _shading_passes[2].ProgramObjectID = programObjectID;
    _shading_passes[2].offset_x = 0;
    _shading_passes[2].offset_y = 0;
    
#ifndef VITA
    if(VERTEX_POS_INDEX <= -1)
    {
        _debugPrintf("VERTEX_POS_INDEX returned invalid value: %d\n", VERTEX_POS_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_POS_INDEX");

    if(VERTEX_TEXCOORD_INDEX <= -1)
    {
        _debugPrintf("VERTEX_TEXCOORD_INDEX returned invalid value: %d\n", VERTEX_TEXCOORD_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_TEXCOORD_INDEX");

    if(VERTEX_MVP_INDEX <= -1)
    {
        _debugPrintf("VERTEX_MVP_INDEX returned invalid value: %d\n", VERTEX_MVP_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_MVP_INDEX");

    if(VERTEX_COLOR_INDEX <= -1)
    {
        _debugPrintf("VERTEX_COLOR_INDEX returned invalid value: %d\n", VERTEX_COLOR_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_COLOR_INDEX");
#endif
    _debugPrintf(
        "[Attrib Location Report]\n\nVERTEX_POS_INDEX: %d\nVERTEX_TEXCOORD_INDEX: %d\nVERTEX_MVP_INDEX: %d\nVERTEX_COLOR_INDEX: %d\n",
        VERTEX_POS_INDEX, VERTEX_TEXCOORD_INDEX, VERTEX_MVP_INDEX, VERTEX_COLOR_INDEX
    );

    
    return 0;
}

int initGLShading()
{
    return initGLShading2((char *)vShaderString, (char *)vFragmentString);
}

int initGLAdv()
{
    _vgl_pending_total_size = sizeof(DrawCall) * MAX_VERTICES;
    _vgl_pending_calls = (DrawCall*)malloc(_vgl_pending_total_size);
    memset(_vgl_pending_calls, 0, _vgl_pending_total_size);

    // Generate vbo
    glGenBuffers(1, &_vertexBufferID);
    _debugPrintf("Gen Buffers with ID %d\n", _vertexBufferID);
    CHECK_GL_ERROR("GEN BUFFERS");

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
    _debugPrintf("bind buffers\n");
    CHECK_GL_ERROR("BIND BUFFERS");

    // Initial data buffer
    glBufferData(GL_ARRAY_BUFFER, _vgl_pending_total_size, 0, GL_DYNAMIC_DRAW);
    _debugPrintf("Initial Buffer Data with %ld bytes (%.2f MB)\n", _vgl_pending_total_size, (_vgl_pending_total_size / 1024.f) / 1024.f);
    CHECK_GL_ERROR("INITIAL BUFFER DATA");

    return 0;
}

int deInitGL()
{
    free(_vgl_pending_calls);
    
#ifdef VITA
    vglEnd();
#else
    glfwTerminate();
#endif

    return 0;
}

#ifndef VITA
static void glfwError(int id, const char* description)
{
    _debugPrintf("[GLFW] ERROR ID %d: %s\n", id, description);
}
#endif

int initGL(void (*dbgPrintFn)(const char*, ...))
{
    if(dbgPrintFn == NULL)
    {
        return -1;
    }

    _debugPrintf = dbgPrintFn;

    for(int i = 0; i < 6; i++)
    {
        _shading_passes[i].ProgramObjectID = 0;
        _shading_passes[i].offset_x = 0;
        _shading_passes[i].offset_y = 0;
    }

#ifdef VITA
    // vglInit(0);
    //0x100000
    int x = 0x4000000;
    _debugPrintf("Initializing with threshold of %d\n", x);
    // vglInitExtended(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, x, SCE_GXM_MULTISAMPLE_NONE);
    vglInitWithCustomSizes(0, 
                           DISPLAY_WIDTH, 
                           DISPLAY_HEIGHT, 
                           8 * 1024 * 1024, 
                           8 * 1024 * 1024, 
                           4 * 1024 * 1024, 
                           SCE_GXM_MULTISAMPLE_NONE);
#endif
#ifndef VITA
    glewExperimental = 1;
    glfwSetErrorCallback(&glfwError);
    int glfwReturnVal = glfwInit();
    if(glfwReturnVal == GLFW_FALSE)
    {
        _debugPrintf("[main] glfwReturnVal is == GLFW_FALSE. glfw init failed.");
        return -1;
    }
    _debugPrintf("[main] GLFWInit: %d\n", glfwReturnVal);

    _game_window = glfwCreateWindow(960,544,"Test", 0, 0);
    _debugPrintf("Window Pointer: %p\n", _game_window);
    glfwMakeContextCurrent(_game_window);

    

    int glewReturnVal = glewInit();
    if(glewReturnVal != GLEW_OK)
    {
        _debugPrintf("[main] GLEWINIT FAILED!\n");
        return -1;
    }
    _debugPrintf("[main] GLEWInit: %d\n", glewReturnVal);
#endif

    glClearColor(0.f, 0.f, 0.f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1);

#ifdef __APPLE__
    // TODO: Check for retina on Apple machines.
    glm_ortho_lh_zo(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1, cpu_mvp);
#else
    glm_ortho_lh_zo(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1, cpu_mvp);
#endif

    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();

    glViewport(
        0,
        -DISPLAY_HEIGHT,
        DISPLAY_WIDTH * 2,
        DISPLAY_HEIGHT * 2
    );

    return 0;
}

// ------------------------------------------ END INIT FUNCTIONS

void Vita_SetClearColor(float r, float g, float b, float a)
{
    _debugPrintf("[vgl_renderer.c] Setting clear color to (%.1f, %.1f, %.1f, %.1f)", r, g, b, a);
    glClearColor(r, g, b, a);
}

/**
 * Vita_Clear(): 
 *  Clears the screen's color buffer using glClear.
 *  Then, calls Vita_ResetTotalCalls() to reset the 
 *  number of draw calls in our queue.
 */
void Vita_Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
    Vita_ResetTotalCalls();
}

/**
 * Vita_Repaint():
 *  Repaint does the following.
 *      1. Buffers the CPU calculated vertices into the GPU.
 *      2. Sets up the vertex attrib pointers for the shader based on the data.
 *      3. Binds the buffers and the default shader and calls glDrawArrays for
 *         each vertex. This sounds bad, but is surprisingly performant 
 *         due to the fact that the data already exists on the GPU at this time.
 *          
 *         Still, this was a workaround due to the fact that glDrawArrays 
 *         tries to render the full list as one quad and the fact that glDrawElements
 *         requires an indices list. 
 */
void Vita_Repaint()
{
    const GLsizei stride = VERTEX_ATTRIB_TOTAL_SIZE_1; // NOT Tightly packed. 4 verts per GL_TRIANGLE_STRIP
    uint32_t draw_calls = Vita_GetTotalCalls();
    if(draw_calls == 0) goto FINISH_DRAWING;
    if(draw_calls > GL_MAX_VERTEX_ATTRIBS)
    {
        _debugPrintf("Too many calls (%d / %d).\n", draw_calls, GL_MAX_VERTEX_ATTRIBS);
    }

    // qsort(_vgl_pending_calls, draw_calls, sizeof(DrawCall), _Vita_SortDrawCalls);

    GLuint _vbo = Vita_GetVertexBufferID(); // Get OpenGL handle to our vbo. (On the GPU)
    
    if(_vbo != 0)
    {
        // Get pointer to the pending drawcalls.
        struct _DrawCall *calls = Vita_GetDrawCallsPending();

        glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Bind our vbo through OpenGL.
        CHECK_GL_ERROR("bind");

        glBufferData(GL_ARRAY_BUFFER, draw_calls * sizeof(DrawCall), calls, GL_DYNAMIC_DRAW);
        // _debugPrintf("Handled %d drawcalls. (%d vertice count)\n", draw_calls, draw_calls * VERTICES_PER_QUAD);
    }
    else return;

    // TODO: Did removing glBindBuffer from here cause issues?

    glUseProgram(programObjectID); // Begin using our vert/frag shader combo (program)

    // ONLY enable these for data that you want to be
    // defined/ passed through the vertex attribute array.
    glEnableVertexAttribArray(VERTEX_POS_INDEX); // Enabling the property on the shader side.
    glEnableVertexAttribArray(VERTEX_TEXCOORD_INDEX); // Enabling TEX_COORD_INDEX
    glEnableVertexAttribArray(VERTEX_COLOR_INDEX); // Enabling color index

    CHECK_GL_ERROR("enable vertex attrib array 0");

    glVertexAttribPointer(VERTEX_POS_INDEX, 2, GL_FLOAT, GL_FALSE, stride, (void*)0); // Binding the data from the vbo to our vertex attrib.
    CHECK_GL_ERROR("vert attrib ptr arrays");

    glVertexAttribPointer(VERTEX_TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, stride, (void*)(0 + (2 * sizeof(float))));
    CHECK_GL_ERROR("vert attrib ptr tex coord.");

    glVertexAttribPointer(VERTEX_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0 + (4 * sizeof(float))));
    CHECK_GL_ERROR("vert attrib ptr color");

    glUniformMatrix4fv(VERTEX_MVP_INDEX, 1, GL_FALSE, (const GLfloat*)cpu_mvp);
    CHECK_GL_ERROR("glUniformMatrix4fv");

    // This is a "hack around".
    // Ideally, I'd be able to batch this all at once.
    GLuint i;
    GLuint _curBoundTex = -1;
    GLuint _curReqTex = 0;
    DrawCall _curDrawCall;

    glm_mat4_identity(_scale_arb);
    glm_mat4_identity(_rot_arb);

    glUniformMatrix4fv(UNIFORM_SCALE_INDEX, 1, GL_FALSE, (const GLfloat *)_scale_arb);
    glUniformMatrix4fv(UNIFORM_ROTMAT_INDEX, 1, GL_FALSE, (const GLfloat *)_rot_arb);

    int thisBatchStart = 0;
    int thisBatch = 0;
    for (i = 0; i < draw_calls; i++)
    {
        _curDrawCall = _vgl_pending_calls[i];

        if (_curDrawCall.verts != NULL && _curDrawCall.verts[0].obj_ptr != NULL)
        {
            obj_extra_data ex_data = *((obj_extra_data *)_curDrawCall.verts[0].obj_ptr);
            _curReqTex = (_curDrawCall.verts[0].obj_ptr != NULL) ? ex_data.textureID : 0;

            // Only re-bind texture when it's different
            // from what's currently bound.
            if (_curBoundTex != _curReqTex)
            {
                if (ex_data.textureID == 0)
                    glUniform1i(UNIFORM_USE_TEXTURE_BOOL_INDEX, 0);
                else
                    glUniform1i(UNIFORM_USE_TEXTURE_BOOL_INDEX, 1);

                // _debugPrintf("[vgl_renderer] repaint(): TODO change bind texture from id %u to id %u\n", _curBoundTex, ex_data.textureID);
                glBindTexture(GL_TEXTURE_2D, ex_data.textureID);
                _curBoundTex = ex_data.textureID;

                

                // reset batch start 
                // thisBatchStart = i * VERTICES_PER_PRIM;
                // thisBatch = 0;
            }
            else
            {
                // Increment to batch vertices that have the same textures
                // thisBatch += VERTICES_PER_PRIM;
            }

            // draw
            glDrawArrays(GL_TRIANGLE_STRIP, i * VERTICES_PER_PRIM, VERTICES_PER_PRIM);

            // TODO: Add offset to the basic shader.
            // glm_translate(cpu_mvp, (vec3){curPass.offset_x, curPass.offset_y, 0.f});
            /*
            glm_mat4_identity(_rot_arb);
            glm_rotate_atm(
                _rot_arb,
                (vec3){ex_data.piv_x, ex_data.piv_y, 0.f},
                glm_rad(ex_data.rot_z),
                (vec3){0.f, 0.f, 1.f});

            vec3 refVector = {ex_data.piv_x, ex_data.piv_y, 0.f};
            vec3 nRefVector = {-ex_data.piv_x, -ex_data.piv_y, 0.f};

            mat4 transRefTo;
            mat4 transRefFrom;
            mat4 transfScale;
            mat4 _temp1;
            glm_mat4_identity(transRefTo);
            glm_mat4_identity(transRefFrom);
            glm_mat4_identity(transfScale);
            glm_mat4_identity(_temp1);
            glm_translate(transRefTo, nRefVector);
            glm_translate(transRefFrom, refVector);

            glm_scale(transfScale, (vec3){ex_data.scale, ex_data.scale, ex_data.scale});
            glm_mat4_mul(transRefFrom, transfScale, _temp1);
            glm_mat4_mul(_temp1, transRefTo, _scale_arb);
            */
        }
    }

    // Revert shader state.
    glUniform1i(UNIFORM_USE_TEXTURE_BOOL_INDEX, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Reverting state.
    glDisableVertexAttribArray(VERTEX_POS_INDEX);
    glDisableVertexAttribArray(VERTEX_TEXCOORD_INDEX);
    glDisableVertexAttribArray(VERTEX_COLOR_INDEX);
    
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glUseProgram(0);

    
    // glFinish();

FINISH_DRAWING:
#ifdef VITA
    vglSwapBuffers(GL_TRUE);
#else
    glfwSwapBuffers(_game_window);
    glfwPollEvents();
#endif

    Vita_ResetTotalCalls();
}

#ifdef __cplusplus
}
#endif
