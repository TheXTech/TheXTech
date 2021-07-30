#ifndef __VITAGL_GRAPHICS__
#define __VITAGL_GRAPHICS__

#ifdef USE_VITA2D
#include <vita2d.h>
#else
#include <vitaGL.h>
#endif

#include "../std_picture.h"

extern void DrawRectSolid(int x, 
        int y, 
        int width, 
        int height, 
        float _r, 
        float _g, 
        float _b, 
        float _a);

extern void Vita_DrawImage(
    StdPicture& texture, float x, float y, float wDst, float hDst,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip,
    float r, float g, float b, float a
);
#endif