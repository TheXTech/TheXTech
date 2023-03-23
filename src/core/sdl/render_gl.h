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

#include <utility>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <unordered_map>

#include <SDL2/SDL_render.h>
#include "core/sdl/gl_inc.h"

#include "core/base/render_base.h"
#include "cmd_line_setup.h"

#include "core/sdl/gl_program_object.h"

struct StdPicture;
struct SDL_Window;

enum BufferIndex_t
{
    BUFFER_GAME,
    BUFFER_FB_READ,
    BUFFER_INIT_PASS,
    BUFFER_PREV_PASS,
    BUFFER_MAX,
};

class RenderGL final : public AbstractRender_t
{
private:
    enum class SupportLevel
    {
        none,
        core,
        oes,
        ext,
    };

    struct Vertex_t
    {
        GLshort position[3];
        GLubyte tint[4];
        GLfloat texcoord[2];
    };

    struct DrawContext_t
    {
        StdPicture* texture;
        GLProgramObject* program;

        constexpr bool operator==(const DrawContext_t& o) const noexcept
        {
            return texture == o.texture && program == o.program;
        }

        constexpr bool operator<(const DrawContext_t& o) const noexcept
        {
            return texture < o.texture || (texture == o.texture && program < o.program);
        }
    };

    struct hash_DrawContext
    {
        std::size_t operator()(const RenderGL::DrawContext_t& c) const noexcept
        {
            return std::hash<GLProgramObject*>()(c.program) ^ (std::hash<StdPicture*>()(c.texture) >> 1);
        }
    };

    struct VertexList
    {
        std::vector<Vertex_t> vertices;
        bool active = false;
    };

    std::unordered_map<DrawContext_t, VertexList, hash_DrawContext> m_unordered_draw_queue;
    std::map<std::pair<int, DrawContext_t>, VertexList> m_ordered_draw_queue;

    std::unordered_map<DrawContext_t, int, hash_DrawContext> m_mask_draw_context_depth;
    DrawContext_t m_recent_draw_context;
    int m_recent_draw_context_depth = 0;

    SDL_Window    *m_window = nullptr;

    SDL_GLContext m_gContext = nullptr;
    bool           m_recentTargetScreen = false;
    std::set<StdPicture *> m_textureBank;

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
    bool m_has_es3_shaders = false;
    bool m_has_npot_texture = false;
    bool m_has_bgra = false;
    bool m_has_fbo = false;

    bool m_client_side_arrays = false;

    // OpenGL state
    std::array<GLuint, BUFFER_MAX> m_buffer_texture = {0};
    std::array<GLuint, BUFFER_MAX> m_buffer_fb = {0};

    const GLuint& m_game_texture = m_buffer_texture[0];
    const GLuint& m_game_texture_fb = m_buffer_fb[0];
    GLuint m_game_depth_rb = 0;

    static constexpr int s_num_buffers = 16;
    GLuint m_vertex_buffer[s_num_buffers] = {0};
    GLsizeiptr m_vertex_buffer_size[s_num_buffers] = {0};
    int m_cur_buffer_index = 0;

#ifdef RENDERGL_HAS_VAO
    GLuint m_glcore_vao = 0;
#endif

    std::array<GLfloat, 16> m_transform_matrix;
    std::array<GLfloat, 4> m_shader_read_viewport;

    GLshort m_cur_depth = 0;

    uint64_t m_current_frame = 0;
    uint64_t m_transform_tick = 0;

    // OpenGL program objects
    GLProgramObject m_standard_program;
    GLProgramObject m_bitmask_program;
    GLProgramObject m_output_program;
    GLProgramObject m_program_rect_filled;
    GLProgramObject m_program_rect_unfilled;
    GLProgramObject m_program_circle;
    GLProgramObject m_program_circle_hole;



private:

    // Source for builtin shaders, defined at render_gl_shaders.cpp
    static const char* const s_es2_standard_vert_src;
    static const char* const s_es2_advanced_vert_src;
    // static const char* const s_es3_standard_vert_src;
    // static const char* const s_es3_advanced_vert_src;

    static const char* const s_es2_standard_frag_src;
    static const char* const s_es2_bitmask_frag_src;
    // static const char* const s_es3_standard_frag_src;
    // static const char* const s_es3_bitmask_frag_src;

    static const char* const s_es2_rect_filled_frag_src;
    static const char* const s_es2_rect_unfilled_frag_src;
    static const char* const s_es2_circle_frag_src;
    static const char* const s_es2_circle_hole_frag_src;


    // Initialization functions, defined at render_gl_init.cpp

    // Returning false indicates a catastrophic failure and results in the OpenGL engine not being created
    // Otherwise, instance flags will be set to reflect actual capabilities

    // initialize the SDL OpenGL bindings according to version preferences and compile-time support,
    // sets the version, and sets profile/version-dependent flags
    bool initOpenGL(const CmdLineSetup_t &setup);

    // initialize the OpenGL debug bindings if requested and possible
    bool initDebug();

    // initialize the global shader objects (also detects ES 2/3 shader support)
    bool initShaders();

    // setup the clear functions, blending, alpha test (legacy), and depth test
    bool initState();

    // initializes the game texture and framebuffer, the shader read texture, and the multipass support framebuffers
    bool initFramebuffers();

    // enable vertex arrays, allocate VBOs if required (Core/Emscripten) or requested and possible; also sets up the VAO if required (Core) or requested and possible
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

    // fill the current vertex array buffer with a vertex attribute array
    void fillVertexBuffer(const Vertex_t* vertex_attribs, int count);
    // deallocate queues unused since previous call
    void refreshDrawQueues();
    // clear queues without drawing or deallocating
    void clearDrawQueues();
    // render and clear queues
    void flushDrawQueues();
    // Choose vertex list for given context and depth pair. Prevents splitting groups for masked draws.
    VertexList& getOrderedDrawVertexList(DrawContext_t context, int depth);

public:
    RenderGL();
    ~RenderGL() override;

    void togglehud() override;


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

    void prepareDrawMask();
    void prepareDrawImage();
    void leaveMaskContext();

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

    void compileShaders(StdPicture &target) override;

    bool textureMaskSupported() override;

    bool userShadersSupported() override;

    void deleteTexture(StdPicture &tx, bool lazyUnload = false) override;
    void clearAllTextures() override;

    void clearBuffer() override;



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




    // Retrieve raw pixel data

    void getScreenPixels(int x, int y, int w, int h, unsigned char *pixels) override;

    void getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels) override;

    int  getPixelDataSize(const StdPicture &tx) override;

    void getPixelData(const StdPicture &tx, unsigned char *pixelData) override;

};


#endif // RenderGL_T_H
