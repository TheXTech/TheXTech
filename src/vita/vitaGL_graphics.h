#ifndef __VITAGL_GRAPHICS__
#define __VITAGL_GRAPHICS__

#include <vitaGL.h>

#include "../std_picture.h"

typedef struct _RectF_
{
    float x,
          y,
          w,
          h,
          top,
          left,
          bottom,
          right;
} RectF;

extern _RectF_ MakeRectF(float x,
                         float y,
                         float w,
                         float h);

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

// inline void DrawRectSolid(int x, 
//     int y, 
//     int width, 
//     int height, 
//     float _r, 
//     float _g, 
//     float _b, 
//     float _a)
// {
//     glBegin(GL_QUADS);

//     glColor4f(_r, _g, _b, _a);
//     glTexCoord2f(0.f, 0.f);
//     glVertex3f(x, y, 0);

//     glColor4f(_r, _g, _b, _a);
//     glTexCoord2f(1.f, 0.f);
//     glVertex3f(x + width, y, 0);

//     glColor4f(_r, _g, _b, _a);
//     glTexCoord2f(1.f, 1.0f);
//     glVertex3f(x + width, y + height, 0);

//     glColor4f(_r, _g, _b, _a);
//     glTexCoord2f(0.f, 1.0f);
//     glVertex3f(x, y + height, 0);

//     glEnd();
// }

// inline void Vita_DrawImage(
//     GLuint texture, float x, float y, float w, float h,
//     float src_x, float src_y, float src_w, float src_h,
//     unsigned int flip,
//     float r, float g, float b, float a
// )
// {
//     float n_src_x = src_x / w;
//     float n_src_x2 = (src_w / w);
//     float n_src_y = src_y / h;
//     float n_src_y2 = (src_h / h);

//     float n_src_w = src_w / w, 
//           n_src_h = src_h / h;

//     pLogDebug("Draw @ %.2f %.2f (w: %.2f src_w: %.2f)", x, y, w, src_w);
    
//     pLogDebug("x1: %.2f x2: %.2f\n     y1: %.2f y2: %.2f", n_src_x, n_src_x2, n_src_y, n_src_y2);


//     glBindTexture(GL_TEXTURE_2D, texture);
//     glBegin(GL_QUADS);

//     // glTexCoord2f(0.f, 0.f);
//     // glVertex3f(x, y, 0);

//     // glTexCoord2f(1.f, 0.f);
//     // glVertex3f(x + src_w, y, 0);

//     // glTexCoord2f(1.f, 1.0f);
//     // glVertex3f(x + src_w, y + src_h, 0);

//     // glTexCoord2f(0.f, 1.0f);
//     // glVertex3f(x, y + src_h, 0);

//     glTexCoord2f(n_src_x, n_src_y);
//     glColor4f(r, g, b, a);
//     glVertex3f(x, y, 0);

//     glTexCoord2f(n_src_x2, n_src_y);
//     glColor4f(r, g, b, a);
//     glVertex3f(x + src_w, y, 0);

//     glTexCoord2f(n_src_x2, n_src_h);
//     glColor4f(r, g, b, a);
//     glVertex3f(x + src_w, y + src_h, 0);

//     glTexCoord2f(n_src_x, n_src_h);
//     glColor4f(r, g, b, a);
//     glVertex3f(x, y + src_h, 0);

//     glEnd();
// }



#endif