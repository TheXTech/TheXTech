/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <set>

#include <nds.h>

#include <Logger/logger.h>

#include "globals.h"
#include "video.h"
#include "frame_timer.h"

#include "core/render.h"
#include "core/minport/render_minport_shared.h"

#include "core/render_planes.h"

#include "draw_planes.h"

#ifdef DEBUG_PLANES
#    define POLY_FOG_Q POLY_FOG
#else
#    define POLY_FOG_Q 0
#endif

namespace XRender
{

bool g_in_frame;

std::set<StdPicture*> s_texture_bank;
uint32_t s_loadedVRAM = 0;

struct tex_load_data
{
    uint8_t* data = nullptr;
    uint16_t palette[16];
    GL_TEXTURE_SIZE_ENUM w, h;

    inline ~tex_load_data()
    {
        if(data)
            free(data);
    }
};

static bool s_loadTextureToRAM(tex_load_data& tex, const std::string& path, int logical_w, int logical_h, int flags)
{
    if(tex.data != nullptr)
        return false;

    logical_w >>= (1 + (flags & 15));
    logical_h >>= (1 + (flags & 15));

    if(logical_w > 1024)
        return false;
    else if(logical_w > 512)
        tex.w = TEXTURE_SIZE_1024;
    else if(logical_w > 256)
        tex.w = TEXTURE_SIZE_512;
    else if(logical_w > 128)
        tex.w = TEXTURE_SIZE_256;
    else if(logical_w > 64)
        tex.w = TEXTURE_SIZE_128;
    else if(logical_w > 32)
        tex.w = TEXTURE_SIZE_64;
    else if(logical_w > 16)
        tex.w = TEXTURE_SIZE_32;
    else if(logical_w > 8)
        tex.w = TEXTURE_SIZE_16;
    else
        tex.w = TEXTURE_SIZE_8;

    if(logical_h > 1024)
        return false;
    else if(logical_h > 512)
        tex.h = TEXTURE_SIZE_1024;
    else if(logical_h > 256)
        tex.h = TEXTURE_SIZE_512;
    else if(logical_h > 128)
        tex.h = TEXTURE_SIZE_256;
    else if(logical_h > 64)
        tex.h = TEXTURE_SIZE_128;
    else if(logical_h > 32)
        tex.h = TEXTURE_SIZE_64;
    else if(logical_h > 16)
        tex.h = TEXTURE_SIZE_32;
    else if(logical_h > 8)
        tex.h = TEXTURE_SIZE_16;
    else
        tex.h = TEXTURE_SIZE_8;


    // calculate number of bytes desired
    // TEXTURE_SIZE_8 is an int with value 0
    uint16_t w_px = 1 << (3 + tex.w);
    uint16_t h_px = 1 << (3 + tex.h);
    uint32_t data_size = w_px * h_px / 2;


    // don't even try to load a texture that would require >=25% of VRAM
    if(data_size > 65536)
    {
        pLogWarning("Refused to allocate %d bytes", data_size);
        return false;
    }


    // could be allocating up to 64 KiB (excluded 128 KiB case)
    uint8_t* pixels = (uint8_t*) malloc(data_size);
    if(!pixels)
    {
        pLogWarning("Failed to allocate %d bytes", data_size);
        return false;
    }


    // open the file after checking the malloc (allocator cheaper than filesystem)
    FILE* texfile = fopen(path.c_str(), "r");
    if(!texfile)
    {
        pLogWarning("Failed to open file", data_size);
        free(pixels);
        return false;
    }


    // load palette
    uint32_t to_read = 32;
    uint8_t* target = (uint8_t*)(tex.palette);

    while(to_read)
    {
        unsigned int bytes_read = fread(target, 1, to_read, texfile);
        if(!bytes_read)
        {
            pLogWarning("Not enough palette data (needed %d more out of 32)", to_read);
            free(pixels);
            fclose(texfile);
            return false;
        }
        to_read -= bytes_read;
        target += bytes_read;
    }


    // load image data
    to_read = data_size;
    target = pixels;

    while(to_read)
    {
        unsigned int bytes_read = fread(target, 1, to_read, texfile);
        if(!bytes_read)
        {
            pLogWarning("Not enough texture data (needed %d more out of %d; logical w %d, h %d; texture w %d, h %d)", to_read, data_size, logical_w, logical_h, w_px, h_px);
            free(pixels);
            fclose(texfile);
            return false;
        }
        to_read -= bytes_read;
        target += bytes_read;
    }

    fclose(texfile);

    tex.data = pixels;

    return true;
}

static bool s_loadTexture(const std::string& path, int* tex_out, uint16_t* tex_w, uint16_t* tex_h, int logical_w, int logical_h, int flags)
{
    if(!tex_out || !tex_w || !tex_h)
    {
        pLogWarning("Got a null pointer to s_loadTexture");
        return false;
    }

    tex_load_data tex;

    if(logical_h > 2048)
        logical_h = 2048;

    int name;

    if(!glGenTextures(1, &name))
    {
        pLogWarning("Could not generate texture");
        return false;
    }

    if(!s_loadTextureToRAM(tex, path, logical_w, logical_h, flags))
    {
        glDeleteTextures(1, &name);
        return false;
    }

    uint16_t w_px = 1 << (3 + tex.w);
    uint16_t h_px = 1 << (3 + tex.h);
    uint32_t data_size = w_px * h_px / 2;

    int tex_params = TEXGEN_OFF;
    if((flags & 16) == 0)
        tex_params |= GL_TEXTURE_COLOR0_TRANSPARENT;

    glBindTexture(0, name);
    if(!glTexImage2D(0, 0, GL_RGB16, tex.w, tex.h, 0, tex_params, tex.data))
    {
        pLogWarning("Could not load texture (%u bytes) to VRAM (%u/524288 used). Requesting free texture memory.", data_size, s_loadedVRAM);
        minport_freeTextureMemory();
    }

    if(!glTexImage2D(0, 0, GL_RGB16, tex.w, tex.h, 0, tex_params, tex.data))
    {
        pLogWarning("Still could not load texture (%u bytes) to VRAM (%u/524288 used).", data_size, s_loadedVRAM);
        glDeleteTextures(1, &name);
        return false;
    }

    glColorTableEXT(0, 0, 16, 0, 0, tex.palette);

    *tex_out = name;
    *tex_w = w_px;
    *tex_h = h_px;

    s_loadedVRAM += data_size;

    // pLogDebug("Loaded tex from %s!", path.c_str());

    return true;
}

static RenderPlanes_t s_render_planes;
static uint8_t s_poly_id;

// sourced from gl2d
static inline void s_gxTexcoord2i(t16 u, t16 v)
{
    GFX_TEX_COORD = (v << 20) | ( (u << 4) & 0xFFFF );
}

static inline void s_gxVertex3i(v16 x, v16 y, v16 z)
{
    GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
    GFX_VERTEX16 = ((uint32)(uint16)z);
}

static inline void s_gxVertex2i(v16 x, v16 y)
{
    GFX_VERTEX_XY = (y << 16) | (x & 0xFFFF);
}

void minport_RenderBoxFilled( int x1, int y1, int x2, int y2, XTColor color)
{
    x2++;
    y2++;

    glBindTexture(0, 0);
    if((color.a >> 3) < 31)
        glPolyFmt(POLY_ID(s_poly_id++) | POLY_ALPHA((color.a >> 3) + 1) | POLY_CULL_NONE | POLY_FOG_Q);
    glColor3b(color.r, color.g, color.b);

    glBegin( GL_QUADS );
        s_gxVertex3i( x1, y1, s_render_planes.next() );       // use 3i for first vertex so that we increment HW depth
        s_gxVertex2i( x1, y2 );                // no need for 3 vertices as 2i would share last depth call
        s_gxVertex2i( x2, y2 );
        s_gxVertex2i( x2, y1 );
    glEnd();

    if((color.a >> 3) < 31)
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FOG_Q);
}

static void s_glBoxFilledGradient( int x1, int y1, int x2, int y2,
                          int color1, int color2, int color3, int color4)
{

    x2++;
    y2++;

    glBindTexture( 0,0 );
    glBegin( GL_QUADS );
        glColor( color1 ); s_gxVertex3i( x1, y1, s_render_planes.next() );       // use 3i for first vertex so that we increment HW depth
        glColor( color2 ); s_gxVertex2i( x1, y2 );                // no need for 3 vertices as 2i would share last depth call
        glColor( color3 ); s_gxVertex2i( x2, y2 );
        glColor( color4 ); s_gxVertex2i( x2, y1 );
    glEnd();
}

inline bool GL_DrawImage_Custom(int name, int flags,
    int16_t x, int16_t y, uint16_t w, uint16_t h,
    uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h,
    unsigned int flip,
    XTColor color)
{
    uint16_t u1 = src_x >> (flags & 15);
    uint16_t u2 = (src_x + src_w) >> (flags & 15);
    uint16_t v1 = src_y >> (flags & 15);
    uint16_t v2 = (src_y + src_h) >> (flags & 15);

    if(flip & X_FLIP_HORIZONTAL)
        std::swap(u1, u2);
    if(flip & X_FLIP_VERTICAL)
        std::swap(v1, v2);

    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;
    uint8_t a = color.a;

    // GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    // GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // pLogDebug("u1 u2 v1 v2: %u %u %u %u; x1 x2 y1 y2: %d %d %d %d", u1, u2, v1, v2, x, x + w, y, y + h);

    glBindTexture(0, name);
    if((a >> 3) < 31)
        glPolyFmt(/*POLY_ID(s_poly_id++) | */ POLY_ALPHA((a >> 3) + 1) | POLY_CULL_NONE | POLY_FOG_Q);

    glBegin(GL_QUADS);

    glColor3b(r, g, b);

    s_gxTexcoord2i(u1, v1);
    s_gxVertex3i(x, y, s_render_planes.next());
    for(int i = 0; i < 7; ++i)
        s_render_planes.next(); // actually advance 8 slots rather than 1, to handle some imprecision of the DSi depth buffer (unknown cause)

    s_gxTexcoord2i(u1, v2);
    s_gxVertex2i(x, y + h);

    s_gxTexcoord2i(u2, v2);
    s_gxVertex2i(x + w, y + h);

    s_gxTexcoord2i(u2, v1);
    s_gxVertex2i(x + w, y);

    glEnd();

    if((a >> 3) < 31)
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FOG_Q);

    return true;
}

bool init()
{
    lcdMainOnBottom();

    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);
    vramSetBankC(VRAM_C_TEXTURE);
    vramSetBankD(VRAM_D_TEXTURE);
    vramSetBankE(VRAM_E_TEX_PALETTE);
    vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
    vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5);

    videoSetMode(MODE_5_3D);

    glInit();

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_POSITION);
    glLoadIdentity();

    glColor(0x7FFF);
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FOG_Q);

#ifdef DEBUG_PLANES
    glEnable(GL_FOG);

    glFogColor(255, 0, 0, 255);
    for(int i = 0; i < 32; i++)
        glFogDensity(i, 0);

    // visibly mark a draw plane for debugging
    glFogDensity(31 - PLANE_LVL_SBLOCK / 8, 127);

    glFogOffset(0);
    glFogShift(0);
#endif

    updateViewport();

    return true;
}

void quit()
{
}

void setTargetTexture()
{
    if(g_in_frame)
    {
        printf("OOPS\n");
        return;
    }

    g_in_frame = true;

    minport_initFrame();

    s_render_planes.reset();
    s_poly_id = 0;
    glClearColor(0, 0, 0, 31);

    minport_ApplyViewport();
}

void setTargetScreen()
{
}

void setDrawPlane(uint8_t plane)
{
    s_render_planes.set_plane(plane);
}

void clearBuffer()
{
    if(g_in_frame)
        return;

    setTargetTexture();
    repaint();
}

void repaint()
{
    if(!g_in_frame)
    {
        printf("OOPS2\n");
        return;
    }

    glFlush(0);

    // if(g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR)
    //     swiWaitForVBlank();
    g_in_frame = false;
}

void mapToScreen(int x, int y, int *dx, int *dy)
{
    *dx = (x - g_screen_phys_x) * XRender::TargetW / g_screen_phys_w;
    *dy = (y - g_screen_phys_y) * XRender::TargetH / g_screen_phys_h;
}

void mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    *window_x = (scr_x * g_screen_phys_w / XRender::TargetW) + g_screen_phys_x;
    *window_y = (scr_y * g_screen_phys_h / XRender::TargetH) + g_screen_phys_y;
}

void minport_TransformPhysCoords() {}

void minport_ApplyPhysCoords()
{
    glViewport(g_screen_phys_x,
            g_screen_phys_y,
            g_screen_phys_x + g_screen_phys_w - 1,
            g_screen_phys_y + g_screen_phys_h - 1);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrthof32( 0, XRender::TargetW / 2, XRender::TargetH / 2, 0, -0x7FFF, 0x7FFF );
}

void minport_ApplyViewport()
{
    int phys_offset_x = g_viewport_x * g_screen_phys_w * 2 / XRender::TargetW;
    int phys_width = g_viewport_w * g_screen_phys_w * 2 / XRender::TargetW;

    int phys_offset_y = g_viewport_y * g_screen_phys_h * 2 / XRender::TargetH;
    int phys_height = g_viewport_h * g_screen_phys_h * 2 / XRender::TargetH;

    if(g_screen_phys_x + phys_offset_x < 0)
        phys_offset_x = -g_screen_phys_x;
    if(g_screen_phys_y + phys_offset_y < 0)
        phys_offset_y = -g_screen_phys_y;

    if(g_screen_phys_x + phys_offset_x + phys_width >= 256)
        phys_width = 256 - (g_screen_phys_x + phys_offset_x);
    if(g_screen_phys_y + phys_offset_y + phys_height >= 192)
        phys_height = 192 - (g_screen_phys_y + phys_offset_y);

    glViewport(g_screen_phys_x + phys_offset_x,
            192 - (g_screen_phys_y + phys_offset_y + phys_height),
            g_screen_phys_x + phys_offset_x + phys_width - 1,
            192 - (g_screen_phys_y + phys_offset_y) - 1);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    int off_x = g_viewport_offset_x;
    int off_y = g_viewport_offset_y;
    // if linear mode is set, consider doing flicker scaling. (DOESN'T WORK)
    // if((phys_width < g_viewport_w || phys_height < g_viewport_h) && (g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR) && (s_current_frame & 1))
    // {
    //     // want to know which pixel in world coordinates is pixel 0.5 in screen coordinates
    //     int half_pixel_x = 0.5 * g_viewport_w / phys_width + 0.5;
    //     int half_pixel_y = 0.5 * g_viewport_h / phys_height + 0.5;

    //     off_x += 1;
    //     off_y += 1;
    // }
    glOrthof32( off_x, g_viewport_w + off_x, g_viewport_h + off_y, off_y, -0x7FFF, 0x7FFF );
}

void lazyLoadPicture(StdPicture_Sub& target, const std::string& path, int scaleFactor, const std::string& maskPath, const std::string& maskFallbackPath)
{
    (void)scaleFactor;
    (void)maskPath;
    (void)maskFallbackPath;

    if(!GameIsActive)
        return; // do nothing when game is closed

    target.inited = true;
    target.l.path = path;
    target.l.lazyLoaded = true;

    // We need to figure out the height and width!
    std::string sizePath = path + ".size";
    FILE* fs = fopen(sizePath.c_str(), "r");

    if(fs != nullptr)
    {
        int w, h, flags;
        if(fscanf(fs, "%d\n%d\n%d\n", &w, &h, &flags) != 3 || w < 0 || w > 8192 || h < 0 || h > 8192)
        {
            pLogWarning("Could not load image %s dimensions from size file", path.c_str());
            target.inited = false;
        }
        else
        {
            target.w = w;
            target.h = h;
            target.l.flags = flags;
        }

        if(fclose(fs))
            pLogWarning("loadPicture: Couldn't close file.");
    }
    else
    {
        pLogWarning("loadPicture: Couldn't open size file.");
        target.inited = false;
    }
}

void lazyLoadPictureFromList(StdPicture_Sub& target, FILE* f, const std::string& dir)
{
    if(!GameIsActive)
        return; // do nothing when game is closed

    int length;

    char filename[256];
    if(fscanf(f, "%255[^\n]%n%*[^\n]\n", filename, &length) != 1)
    {
        pLogWarning("Could not load image path from load list");
        return;
    }

    if(length == 255)
    {
        pLogWarning("Image path %s was truncated in load list, aborting", filename);
        return;
    }

    target.inited = true;
    target.l.path = dir;
    target.l.path += filename;
    target.l.lazyLoaded = true;

    int w, h, flags;
    if(fscanf(f, "%d\n%d\n%d\n", &w, &h, &flags) != 3 || w < 0 || w > 8192 || h < 0 || h > 8192)
    {
        pLogWarning("Could not load image %s dimensions from load list", filename);
        target.inited = false;
        return;
    }

    // pLogDebug("Successfully loaded %s (%d %d)", target.l.path.c_str(), w, h);

    target.w = w;
    target.h = h;
    target.l.flags = flags;

    return;
}

void lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || target.d.hasTexture())
        return;

    target.d.attempted_load = true;
    s_texture_bank.insert(&target);

    std::string suppPath;

    if(!s_loadTexture(target.l.path, &target.d.texture[0], &target.d.tex_w[0], &target.d.tex_h[0], target.w, target.h, target.l.flags))
    {
        pLogWarning("Permanently failed to load %s", target.l.path.c_str());
        return;
    }

    if(target.h > 2048)
    {
        suppPath = target.l.path + '1';
        if(!s_loadTexture(suppPath, &target.d.texture[1], &target.d.tex_w[1], &target.d.tex_h[1], target.w, target.h - 2048, target.l.flags))
        {
            pLogWarning("Permanently failed to load %s", suppPath.c_str());
            return;
        }
    }
    if(target.h > 4096)
    {
        suppPath = target.l.path + '2';
        if(!s_loadTexture(suppPath, &target.d.texture[2], &target.d.tex_w[2], &target.d.tex_h[2], target.w, target.h - 4096, target.l.flags))
        {
            pLogWarning("Permanently failed to load %s", suppPath.c_str());
            return;
        }
    }

    return;
}

void lazyPreLoad(StdPicture &target)
{
    lazyLoad(target);
}

void clearAllTextures()
{
    for(StdPicture* tx : s_texture_bank)
    {
        s_loadedVRAM -= (tx->d.tex_w[0] * tx->d.tex_h[0]
            + tx->d.tex_w[1] * tx->d.tex_h[1]
            + tx->d.tex_w[2] * tx->d.tex_h[2]) / 2;

        tx->d.destroy();
    }

    s_texture_bank.clear();

    if(s_loadedVRAM != 0)
        pLogWarning("VRAM use not 0 after clear (%u instead). At risk of use-after-free.", s_loadedVRAM);

    // useful to combat fragmentation
    s_loadedVRAM = 0;
    glResetTextures();
}

void unloadTexture(StdPicture &tx)
{
    if(!tx.inited)
        return;

    if(s_texture_bank.find(&tx) != s_texture_bank.end())
        s_texture_bank.erase(&tx);

    minport_unlinkTexture(&tx);

    if(tx.d.reallyHasTexture())
    {
        D_pLogDebug("Freeing %d bytes from %s", (tx.d.tex_w[0] * tx.d.tex_h[0]
            + tx.d.tex_w[1] * tx.d.tex_h[1]
            + tx.d.tex_w[2] * tx.d.tex_h[2]) / 2, tx.l.path.c_str());
        s_loadedVRAM -= (tx.d.tex_w[0] * tx.d.tex_h[0]
            + tx.d.tex_w[1] * tx.d.tex_h[1]
            + tx.d.tex_w[2] * tx.d.tex_h[2]) / 2;
    }

    tx.d.destroy();

    if(tx.l.path.empty())
        static_cast<StdPicture_Sub&>(tx) = StdPicture_Sub();
}

void minport_RenderTexturePrivate(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                             StdPicture &tx,
                             int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                             float rotateAngle, FPoint_t *center, unsigned int flip,
                             XTColor color)
{
    if(!tx.inited)
        return;

    if(tx.l.lazyLoaded && !tx.d.hasTexture() && !tx.d.reallyHasTexture())
        lazyLoad(tx);

    // handle rotation NOW
    if(rotateAngle)
    {
        int16_t cx, cy;
        glPushMatrix();

        if(center)
        {
            cx = center->x / 2.0f + 0.5f;
            cy = center->y / 2.0f + 0.5f;
        }
        else
        {
            cx = wDst / 2;
            cy = hDst / 2;
        }

        glTranslatef32(xDst + cx, yDst + cy, 0);
        glRotateZ(rotateAngle);

        xDst = -cx;
        yDst = -cy;
    }

    if(!tx.d.reallyHasTexture())
    {
        s_glBoxFilledGradient( xDst, yDst, xDst + wDst - 1, yDst + hDst - 1,
                             RGB15( 15, 2, 1 ),
                             RGB15( 1, 15, 1 ),
                             RGB15( 1, 2, 15 ),
                             RGB15( 1, 15, 15 )
                           );

        return;
    }

    int to_draw = 0;
    int to_draw_2 = 0;

    if(ySrc + hSrc > 1024)
    {
        if(ySrc + hSrc > 2048)
        {
            if(tx.d.texture[2])
                to_draw = tx.d.texture[2];
            if(ySrc < 2048 && tx.d.texture[1])
                to_draw_2 = tx.d.texture[1];
            ySrc -= 1024;
        }
        else
        {
            if(tx.d.texture[1])
                to_draw = tx.d.texture[1];
            if(ySrc < 1024)
                to_draw_2 = tx.d.texture[0];
        }
        // draw the top pic
        if(to_draw_2)
        {
            GL_DrawImage_Custom(to_draw_2, tx.l.flags, xDst, yDst, wDst, (1024 - ySrc) * hDst / hSrc,
                                xSrc, ySrc, wSrc, 1024 - ySrc, flip, color);
            yDst += (1024 - ySrc) * hDst / hSrc;
            hDst -= (1024 - ySrc) * hDst / hSrc;
            hSrc -= (1024 - ySrc);
            ySrc = 0;
        }
        else
            ySrc -= 1024;
    }
    else
    {
        to_draw = tx.d.texture[0];
    }

    if(!to_draw) return;

    GL_DrawImage_Custom(to_draw, tx.l.flags, xDst, yDst, wDst, hDst,
                        xSrc, ySrc, wSrc, hSrc, flip, color);

    // finalize rotation HERE
    if(rotateAngle)
        glPopMatrix(1);
}

}; // namespace XRender
