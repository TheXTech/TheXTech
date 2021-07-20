#include "vitaGL_graphics.h"

#include <Logger/logger.h>

static inline void DrawRectSolid(int x,
                          int y,
                          int width,
                          int height,
                          float _r,
                          float _g,
                          float _b,
                          float _a)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);

    glColor4f(_r, _g, _b, _a);
    glTexCoord2f(0.f, 0.f);
    glVertex3f(x, y, 0);

    glColor4f(_r, _g, _b, _a);
    glTexCoord2f(1.f, 0.f);
    glVertex3f(x + width, y, 0);

    glColor4f(_r, _g, _b, _a);
    glTexCoord2f(1.f, 1.0f);
    glVertex3f(x + width, y + height, 0);

    glColor4f(_r, _g, _b, _a);
    glTexCoord2f(0.f, 1.0f);
    glVertex3f(x, y + height, 0);

    glEnd();
}

static inline void Vita_DrawImage(
    StdPicture& texture, float x, float y, float wDst, float hDst,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip,
    float r, float g, float b, float a)
{



#ifdef USE_STBI_RESIZE
    float tex_w = texture.w,
          tex_h = texture.h;
          wDst *= 2;
          hDst *= 2;
#else
    // src_w *= 2;
    // src_h *= 2;
    float tex_w = texture.w / 2,
          tex_h = texture.h / 2;
#endif


    float n_src_x = (src_x / tex_w);
    float n_src_x2 = ((src_x + src_w) / tex_w);
    // float n_src_y = (src_y / tex_h);
    // float n_src_y2 = ((src_y + src_h) / tex_h);

    float n_src_y = ((src_y + src_h) / tex_h);
    float n_src_y2 = (src_y / tex_h);

    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glBegin(GL_QUADS);

    glTexCoord2f(n_src_x, n_src_y);
    glColor4f(r, g, b, a);
    glVertex3f(x, y, 0);

    glTexCoord2f(n_src_x2, n_src_y);
    glColor4f(r, g, b, a);
    glVertex3f((x + (wDst * 1)), y, 0);

    glTexCoord2f(n_src_x2, n_src_y2);
    glColor4f(r, g, b, a);
    glVertex3f((x + (wDst * 1)), (y + (hDst * 1)), 0);

    glTexCoord2f(n_src_x, n_src_y2);
    glColor4f(r, g, b, a);
    glVertex3f(x, (y + (hDst * 1)), 0);

    glEnd();
}
