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
#ifndef RenderGL_T_H
#define RenderGL_T_H

#include "core/sdl/gl_inc.h"

#    ifdef RENDERGL_SUPPORTED

#include <utility>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <unordered_map>

#include <SDL2/SDL_render.h>

#include "core/base/render_base.h"
#include "cmd_line_setup.h"

#include "core/sdl/gl_program_object.h"

struct StdPicture;
struct SDL_Window;

/*!
 * \brief Represents one of the game draw/read buffer textures
 *
 * RenderGL::framebufferCopy is used to copy contents from one buffer to another
 */
enum BufferIndex_t
{
    BUFFER_GAME = 0,  /**< main texture for the game */
    BUFFER_FB_READ,   /**< auxiliary texture used for reading from main buffer in shader */
    BUFFER_INIT_PASS, /**< auxiliary texture used to restore the state after drawing only opaque objects (multipass rendering) */
    BUFFER_PREV_PASS, /**< auxiliary texture used to read the state following the previous draw pass (multipass rendering) */
    BUFFER_LIGHTING,  /**< texture used to store the outcome of the lighting calculations */
    BUFFER_MAX,
};

#ifdef RENDERGL_HAS_FBO
/*!
 * \brief Readable aliases for the texture units
 */
enum TextureUnit_t
{
    TEXTURE_UNIT_IMAGE = GL_TEXTURE0, /**< texture unit for the currently bound StdPicture (default) */
    TEXTURE_UNIT_FB_READ,             /**< texture unit for reading from the FB read buffer (set once) */
    TEXTURE_UNIT_MASK,                /**< texture unit for emulating bitmask rendering (set per bitmasked render) */
    TEXTURE_UNIT_PREV_PASS,           /**< texture unit for reading from the previous render pass (first set to init pass, and then prev pass) */
    TEXTURE_UNIT_DEPTH_READ,          /**< texture unit for reading depth from the initial render pass */
    TEXTURE_UNIT_LIGHT_READ,          /**< texture unit for reading lighting state (set once) */
};
#endif

class RenderGL final : public AbstractRender_t
{
private:
    /*!
     * \brief Takes GL profile int, returns profile string
     */
    static inline const char* get_profile_name(const GLint profile)
    {
        switch(profile)
        {
        case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
            return "Compatibility";
        case SDL_GL_CONTEXT_PROFILE_CORE:
            return "Core";
        case SDL_GL_CONTEXT_PROFILE_ES:
            return "ES";
        default:
            return "";
        }
    }

    // unused for now
    // enum class SupportLevel
    // {
    //     none,
    //     core,
    //     oes,
    //     ext,
    // };

    /*!
     * \struct Vertex_t
     * \brief Represents a single entry in a vertex array
     * \param position: 3 int16_ts in viewport coordinates
     * \param tint: 4 uint8_ts
     * \param texcoord: 2 floats in GL texture-normalized coordinates (not picture coordinates if non-power-of-two)
     */
    struct Vertex_t
    {
        std::array<GLshort, 3> position;
        std::array<GLubyte, 4> tint;
        std::array<GLfloat, 2> texcoord;
    };

    /*!
     * \brief Represents a vertex array that can be rendered in a single glDrawArrays call
     * \param vertices: vector of vertices, frequently resized to zero without deallocation for efficiency
     * \param active: whether this vertex array was non-empty at any flushDrawQueues() since the last cleanupDrawQueues() call
     */
    struct VertexList
    {
        std::vector<Vertex_t> vertices;
        bool active = false;
    };

    /*!
     * \brief Represents the draw state needed for a single glDrawArrays call
     * \param program: non-nullable pointer to the program object for the draw call
     * \param uniform_step: integer index to the index of the program object's current uniform state, indexed by times that uniforms were set
     * \param texture: nullable pointer to a texture for the draw call
     */
    struct DrawContext_t
    {
        GLProgramObject* program;
        int uniform_step;
        StdPicture* texture;

        constexpr DrawContext_t(std::nullptr_t)
            : program(nullptr), uniform_step(0), texture(nullptr) {}

        inline DrawContext_t(GLProgramObject& program)
            : program(&program), uniform_step(program.get_uniform_step()), texture(nullptr) {}

        inline DrawContext_t(GLProgramObject& program, StdPicture* texture)
            : program(&program), uniform_step(program.get_uniform_step()), texture(texture) {}

        constexpr bool operator==(const DrawContext_t& o) const noexcept
        {
            return texture == o.texture && program == o.program && uniform_step == o.uniform_step;
        }

        constexpr bool operator<(const DrawContext_t& o) const noexcept
        {
            return (program == o.program && uniform_step == o.uniform_step && texture < o.texture) || program < o.program || (program == o.program && uniform_step < o.uniform_step);
        }
    };

    /*!
     * \brief Utility struct to allow DrawContext_t to be used as a key in a hash-map
     */
    struct hash_DrawContext
    {
        std::size_t operator()(const RenderGL::DrawContext_t& c) const noexcept
        {
            return std::hash<GLProgramObject*>()(c.program) ^ (std::hash<StdPicture*>()(c.texture) >> 1) ^ (std::hash<int>()(c.uniform_step) >> 2);
        }
    };

#ifdef RENDERGL_HAS_SHADERS

    enum class LightType : uint32_t
    {
        none = 0,
        point,
        arc,
        bar,
        box,
    };

#    ifdef THEXTECH_BIG_ENDIAN
    struct LightColor
    {
        constexpr LightColor() = default;
        constexpr LightColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
            : a(a), r(r), g(g), b(b) {}

        uint8_t a = 0x00;
        uint8_t r = 0x00;
        uint8_t g = 0x00;
        uint8_t b = 0x00;
    };
#    else // #ifdef THEXTECH_BIG_ENDIAN
    struct LightColor
    {
        constexpr LightColor() = default;
        constexpr LightColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
            : b(b), g(g), r(r), a(a) {}

        uint8_t b = 0x00;
        uint8_t g = 0x00;
        uint8_t r = 0x00;
        uint8_t a = 0x00;
    };
#    endif // #else (from #ifdef THEXTECH_BIG_ENDIAN)

    using LightPos = std::array<GLfloat, 4>;

    /*!
     * \brief Information parameterizing the overall lighting system
     */
    struct LightHeader
    {};

    /*!
     * \brief Information parameterizing a single light source
     */
    struct Light
    {
        constexpr Light() = default;
        constexpr Light(LightType type, LightColor color, GLfloat radius, GLfloat depth, LightPos pos)
            : type(type), color(color), radius(radius), depth(depth), pos(pos) {}

        static constexpr Light Point(GLfloat x, GLfloat y, GLfloat depth, LightColor color, GLfloat radius)
        {
            return Light(LightType::point, color, radius, depth, {x, y, 0.0f, 0.0f});
        }

        LightType type = LightType::none;
        LightColor color = LightColor{0, 0, 0, 0};
        GLfloat radius = 0.0;
        GLfloat depth = 0.0;
        LightPos pos = {0.0, 0.0, 0.0, 0.0};
    };

    struct LightBuffer
    {
        // LightHeader header;
        std::array<Light, 64> lights;
    };

#endif // #ifdef RENDERGL_HAS_SHADERS

    // unsorted draw queue that stores opaque draw calls in the current viewport state
    std::unordered_map<DrawContext_t, VertexList, hash_DrawContext> m_unordered_draw_queue;

    // sorted draw queue that stores translucent / advanced draw calls in the current viewport state
    std::map<std::pair<int, DrawContext_t>, VertexList> m_ordered_draw_queue;

    // state used to group subsequent ordered draws, even if their depths are not exactly the same
    std::unordered_map<DrawContext_t, int, hash_DrawContext> m_mask_draw_context_depth;
    DrawContext_t m_recent_draw_context = {nullptr};
    int m_recent_draw_context_depth = 0;

    // reference to currently active SDL window
    SDL_Window    *m_window = nullptr;

    // reference to SDL OpenGL context
    SDL_GLContext m_gContext = nullptr;

    // tracks whether the renderer is currently drawing to the screen (instead of the game texture)
    bool           m_recentTargetScreen = false;

    // set of all currently-loaded textures
    std::set<StdPicture *> m_loadedPictures;

    // Offset to shake screen
    int m_viewport_offset_x = 0;
    int m_viewport_offset_y = 0;
    // Keep zero viewport offset while this flag is on
    bool m_viewport_offset_ignore = false;

    //Need to calculate relative hardware viewport position when screen was scaled
    int m_phys_x = 0;
    int m_phys_y = 0;
    int m_phys_w = 0;
    int m_phys_h = 0;

    //Need to calculate in-game vScreen position
    int m_viewport_x = 0;
    int m_viewport_y = 0;
    int m_viewport_w = 0;
    int m_viewport_h = 0;

    //Need for HiDPI rendering (number of draw pixels per cursor pixel)
    float m_hidpi_x = 1.0f;
    float m_hidpi_y = 1.0f;


    // internal capability trackers
    GLint m_gl_majver = 0;
    GLint m_gl_minver = 0;
    GLint m_gl_profile = 0;

    bool m_use_logicop = false;
    bool m_use_shaders = false;
    bool m_has_fbo = false;
    bool m_use_depth_buffer = false;
    bool m_client_side_arrays = false;

    // unused for now
    bool m_has_es3_shaders = false;
    // bool m_has_npot_texture = false;
    // bool m_has_bgra = false;

    // preferences

    int m_lighting_downscale = 2;


    // OpenGL state

    // depth of most recent draw (reset per frame)
    GLshort m_cur_depth = 0;

    // tick for current frame
    uint64_t m_current_frame = 0;

    // framebuffer object (FBO) state

    // textures for each render-to-texture layer
    std::array<GLuint, BUFFER_MAX> m_buffer_texture = {0};
    // FBOs for each render-to-texture layer (only the first is required)
    std::array<GLuint, BUFFER_MAX> m_buffer_fb = {0};
    // renderbuffer for main game framebuffer's depth component
    GLuint m_game_depth_rb = 0;
    // texture for main game framebuffer's depth component (preferred)
    GLuint m_game_depth_texture = 0;
    // texture for depth buffer read
    GLuint m_depth_read_texture = 0;

    // references to main game framebuffer's texture and FBO
    const GLuint& m_game_texture = m_buffer_texture[0];
    const GLuint& m_game_texture_fb = m_buffer_fb[0];

    // vertex buffer object (VBO) and vertex array object (VAO) state

    // tuned carefully for performance on Mac and Emscripten, which require using VBOs
    static constexpr int s_num_vertex_buffers = 128;

    // VBOs and their currently initialized sizes
    GLuint m_vertex_buffer[s_num_vertex_buffers] = {0};
    GLsizeiptr m_vertex_buffer_size[s_num_vertex_buffers] = {0};

    // most recently filled VBO
    int m_cur_vertex_buffer_index = 0;

#ifdef RENDERGL_HAS_VAO
    // Single VAO (only used in GL Core), never changed. Only touched during init / cleanup functions.
    GLuint m_glcore_vao = 0;
#endif

    // shader state

    // current transform matrix
    std::array<GLfloat, 16> m_transform_matrix;

    // representation of viewport (vScreen) to allow shaders to transform between draw coordinates and framebuffer texture coordinates
    std::array<GLfloat, 4> m_shader_read_viewport;

    // clock variable in seconds for shaders (loops at 60s)
    GLfloat m_shader_clock = 0.0f;

    // number of times the above have been set (used to limit number of uniform update calls)
    uint64_t m_transform_tick = 0;

    // OpenGL program objects
    GLProgramObject m_standard_program;
    GLProgramObject m_bitmask_program;
    GLProgramObject m_output_program;
    GLProgramObject m_program_rect_filled;
    GLProgramObject m_program_rect_unfilled;
    GLProgramObject m_program_circle;
    GLProgramObject m_program_circle_hole;



    // extra state supporting the lighting system

    // performs the light calculation pass
    GLProgramObject m_lighting_program;

    // UBO to support the lighting queue
    GLuint m_light_ubo = 0;

    // filled with lights over the course of a render queue fill
    LightBuffer m_light_queue;

    // tracks how many lights have been added in the current frame
    int m_light_count = 0;



    // Source for builtin shaders, defined at render_gl_shaders.cpp
    static const char* const s_es2_standard_vert_src;
    static const char* const s_es2_advanced_vert_src;

    static const char* const s_es3_standard_vert_src;
    static const char* const s_es3_advanced_vert_src;

    static const char* const s_es2_standard_frag_src;
    static const char* const s_es2_bitmask_frag_src;
    static const char* const s_es3_bitmask_frag_src;

    static const char* const s_es2_rect_filled_frag_src;
    static const char* const s_es2_rect_unfilled_frag_src;
    static const char* const s_es2_circle_frag_src;
    static const char* const s_es2_circle_hole_frag_src;

    static const char* const s_es3_lighting_frag_src;

    // Initialization functions, defined at render_gl_init.cpp

    // For all:
    // Returning false indicates a catastrophic failure and results in the OpenGL engine not being created
    // Otherwise, instance flags will be set to reflect actual capabilities

    /*!
     * \brief Helper function to attempt to initialize an SDL OpenGL context of the desired profile and version
     * \param[inout] context: reference to nullable SDL_GLContext. If non-null, no-op (already inited). If null, attempts initialization and signals success by leaving context non-null.
     * \param[in] window: SDL_Window to use for the context
     * \param profile: OpenGL profile to initialize
     * \param majver: minimum OpenGL major version to initialize
     * \param minver: minimum OpenGL minor version to initialize
     */
    static void try_init_gl(SDL_GLContext& context, SDL_Window* window, GLint profile, GLint majver, GLint minver);

    /*!
     * \brief Initialize the SDL OpenGL bindings according to version preferences and compile-time support, sets the version, and sets profile/version-dependent flags
     * \returns false no supported OpenGL profile can be initialized
     */
    bool initOpenGL(const CmdLineSetup_t &setup);

    /*!
     * \brief Initialize the OpenGL debug bindings if requested and possible
     */
    bool initDebug();

    /*!
     * \brief Initialize the global shader objects (also detects ES 2/3 shader support)
     * \returns false if ES 2 shaders cannot be successfully compiled, but fixed-function pipeline is also unavailable
     */
    bool initShaders();

    /*!
     * \brief Initializes the game texture and framebuffer, the shader read texture, and the multipass support framebuffers
     *
     * No-op if FBO is unsupported
     *
     * Only attempts to initialize shader read buffer if using shaders to emulate logic ops or supporting user shaders
     * Only attempts to initialize other auxiliary buffers if supporting user shaders
     */
    bool initFramebuffers();

    /*!
     * \brief Setup the clear functions, blending, alpha test (legacy), and depth test
     *
     * Clear functions: sets the clear color and depth
     * Blending: sets standard alpha blending
     * Alpha test: in fixed-function pipeline, enables alpha test with 0x08 cutoff
     * Depth test: enables depth test by default and sets the pass function to >= (greater-than-or-equal)
     */
    bool initState();

    /*!
     * \brief Enable vertex arrays
     *
     * allocate VBOs if required (Core/Emscripten) or requested and possible
     * also sets up the VAO if required (Core) or requested and possible
     *
     * \returns false if required VBO/VAO cannot be created
     */
    bool initVertexArrays();

    // Private draw management functions

    // initializes a single framebuffer with the game's current screen resolution
    // if BUFFER_GAME, will include depth
    // if not BUFFER_GAME, will accept case where FBO cannot be allocated as long as texture is okay
    void createFramebuffer(BufferIndex_t buffer);
    // destroys a single framebuffer
    void destroyFramebuffer(BufferIndex_t buffer);
    // perform a framebuffer->framebuffer copy
    void framebufferCopy(BufferIndex_t dest, BufferIndex_t source, int x, int y, int w, int h);

    // sets required GL logicOp state for masks
    void prepareDrawMask();
    // sets required GL logicOp state for front images
    void prepareDrawImage();
    // unsets GL logicOp state for ordinary drawing
    void leaveMaskContext();

    // fill a vertex array buffer with a vertex attribute array. prepares all state needed for call to glDrawArrays
    void fillVertexBuffer(const Vertex_t* vertex_attribs, int count);
    // deallocate render queues unused since previous call to cleanupDrawQueues
    void cleanupDrawQueues();
    // clear render queues without drawing or deallocating
    void clearDrawQueues();

    // executes and clears all vertex lists in the unordered draw queue
    void flushUnorderedDrawQueue();
    // calculates the lighting state using depth buffer information
    void calculateLighting();
    // prepares for the next pass during multipass drawing
    void prepareMultipassState(int pass);
    // executes and optionally clears all vertex lists in the ordered draw queue
    void executeOrderedDrawQueue(bool clear);
    // Draws and clears all render queues. Called prior to changing GL context.
    void flushDrawQueues();

    // Selects efficient ordered vertex list for given context and depth pair. Batches across subsequent draws and masks.
    VertexList& getOrderedDrawVertexList(DrawContext_t context, int depth);

protected:
    // Compiles user fragment shader and assembles program in target.
    void compileShaders(StdPicture &target) override;

public:
    RenderGL();
    ~RenderGL() override;


    unsigned int SDL_InitFlags() override;

    bool isWorking() override;

    bool initRender(const CmdLineSetup_t &setup, SDL_Window *window) override;

    /*!
     * \brief Close the renderer
     */
    void close() override;

    /*!
     * \brief Call the repaint
     */
    void repaint() override;

    void applyViewport();

    /*!
     * \brief Update viewport (mainly after screen resize)
     */
    void updateViewport() override;

    /*!
     * \brief Reset viewport into default state
     */
    void resetViewport() override;

    /*!
     * \brief Set the viewport area
     * \param x X position
     * \param y Y position
     * \param w Viewport Width
     * \param h Viewport Height
     */
    void setViewport(int x, int y, int w, int h) override;

    /*!
     * \brief Set the render offset
     * \param x X offset
     * \param y Y offset
     *
     * All drawing objects will be drawn with a small offset
     */
    void offsetViewport(int x, int y) override; // for screen-shaking

    /*!
     * \brief Set temporary ignore of render offset
     * \param en Enable viewport offset ignore
     *
     * Use this to draw certain objects with ignorign of the GFX offset
     */
    void offsetViewportIgnore(bool en) override;

    /*!
     * \brief Make any subsequent draws invisible to any previous draws (reflections)
     *
     * Only has an effect for OpenGL renderer and other batched renderers.
     *
     * Note: may result in subsequent transparent draw being inaccurately drawn above previous transparent draw.
     */
    inline void splitFrame() override
    {
        flushDrawQueues();
    }

    void mapToScreen(int x, int y, int *dx, int *dy) override;

    void mapFromScreen(int x, int y, int *dx, int *dy) override;

    /*!
     * \brief Get the current size of the window in render pixels
     * \param w Width
     * \param h Height
     */
    void getRenderSize(int *w, int *h) override;

    /*!
     * \brief Set render target into the virtual in-game screen (use to render in-game world)
     */
    void setTargetTexture() override;

    /*!
     * \brief Set render target into the real window or screen (use to render on-screen buttons and other meta-info)
     */
    void setTargetScreen() override;

    void loadTexture(StdPicture &target,
                     uint32_t width,
                     uint32_t height,
                     uint8_t *RGBApixels,
                     uint32_t pitch,
                     bool is_mask,
                     uint32_t least_width,
                     uint32_t least_height);

    void loadTexture(StdPicture &target,
                     uint32_t width,
                     uint32_t height,
                     uint8_t *RGBApixels,
                     uint32_t pitch) override;

    void loadTextureMask(StdPicture &target,
                     uint32_t width,
                     uint32_t height,
                     uint8_t *RGBApixels,
                     uint32_t pitch,
                     uint32_t image_width,
                     uint32_t image_height) override;

    bool textureMaskSupported() override;

    bool userShadersSupported() override;

    bool depthTestSupported() override;

    void unloadTexture(StdPicture &tx) override;
    void clearAllTextures() override;

    void clearBuffer() override;

    int registerUniform(StdPicture &target, const char* name) override;
    void assignUniform(StdPicture &target, int index, const UniformValue_t& value) override;
    void spawnParticle(StdPicture &target, double worldX, double worldY, ParticleVertexAttrs_t attrs) override;


    // Draw primitives

    void renderRect(int x, int y, int w, int h,
                    float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                    bool filled = true) override;

    void renderRectBR(int _left, int _top, int _right,
                      int _bottom, float red, float green, float blue, float alpha) override;

    void renderCircle(int cx, int cy,
                      int radius,
                      float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f,
                      bool filled = true) override;

    void renderCircleHole(int cx, int cy,
                          int radius,
                          float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;




    // Draw texture

    void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                              StdPicture &tx,
                              int xSrc, int ySrc,
                              int wSrc, int hSrc,
                              double rotateAngle =.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                              float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTexture(double xDst, double yDst, double wDst, double hDst,
                       StdPicture &tx,
                       int xSrc, int ySrc,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                         StdPicture &tx,
                         int xSrc, int ySrc,
                         double rotateAngle =.0, FPoint_t *center = nullptr, unsigned int flip = X_FLIP_NONE,
                         float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderTexture(float xDst, float yDst, StdPicture &tx,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) override;

    void renderParticleSystem(StdPicture &tx,
                              double camX,
                              double camY) override;



    // Retrieve raw pixel data

    void getScreenPixels(int x, int y, int w, int h, unsigned char *pixels) override;

    void getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels) override;

    int  getPixelDataSize(const StdPicture &tx) override;

    void getPixelData(const StdPicture &tx, unsigned char *pixelData) override;

};


#endif // RENDERGL_SUPPORTED

#endif // RenderGL_T_H
