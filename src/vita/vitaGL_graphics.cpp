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
    float w = texture.w / 2,
          h = texture.h / 2;
          

    float n_src_x = (src_x / w);
    float n_src_x2 = ((src_x + wDst) / w);
    float n_src_y = (src_y / h);
    float n_src_y2 = ((src_y + hDst) / h);


    pLogDebug(
        "\n\nDraw X From %.2f w: %.2f, Y from %.2f h:%.2f\nSrc X: %.2f Src Y: %.2f\nSrc W: %.2f Src H: %.2f",
        x, wDst,
        y, hDst,
        src_x, src_y,
        src_w, src_h
    );
    
    pLogDebug(
        "\n Texture Reference Size: %.2f x %.2f",
        w,
        h
    );

    pLogDebug("\nx1: %.2f x2: %.2f\n     y1: %.2f y2: %.2f", n_src_x, n_src_x2, n_src_y, n_src_y2);

    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glBegin(GL_QUADS);

    // glTexCoord2f(0.f, 0.f);
    // glVertex3f(x, y, 0);

    // glTexCoord2f(1.f, 0.f);
    // glVertex3f(x + src_w, y, 0);

    // glTexCoord2f(1.f, 1.0f);
    // glVertex3f(x + src_w, y + src_h, 0);

    // glTexCoord2f(0.f, 1.0f);
    // glVertex3f(x, y + src_h, 0);

    glTexCoord2f(n_src_x, n_src_y);
    glColor4f(r, g, b, a);
    glVertex3f(x, y, 0);

    glTexCoord2f(n_src_x2, n_src_y);
    glColor4f(r, g, b, a);
    glVertex3f(x + src_w, y, 0);

    glTexCoord2f(n_src_x2, n_src_y2);
    glColor4f(r, g, b, a);
    glVertex3f(x + src_w, y + src_h, 0);

    glTexCoord2f(n_src_x, n_src_y2);
    glColor4f(r, g, b, a);
    glVertex3f(x, y + src_h, 0);

    glEnd();
}
