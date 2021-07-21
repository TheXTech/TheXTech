#include "vitaGL_graphics.h"

#include <Logger/logger.h>
#include "math.h"

// TODO: Backport Moondust's common rectangle to TheXTech for PS Vita?

// RectF MakeRectF(float x, float y, float w, float h)
// {
//     RectF _rect;
//     _rect.x = x;
//     _rect.y = y;
//     _rect.w = w;
//     _rect.h = h;
//     _rect.left = x;
//     _rect.right = x + w;
//     _rect.top = y;
//     _rect.bottom = y + h;
//     return _rect;
// }

// inline RectF MapToGl(float x, 
//                               float y, 
//                               float w, 
//                               float h)
// {
//     RectF rect;
//     rect.setLeft(static_cast<double>(roundf(x) / (viewport_w_half) - 1.0f));
//     rect.setTop(static_cast<double>((viewport_h - (roundf(y))) / viewport_h_half - 1.0f));
//     rect.setRight(static_cast<double>(roundf(x + w) / (viewport_w_half) - 1.0f));
//     rect.setBottom(static_cast<double>((viewport_h - (roundf(y + h))) / viewport_h_half - 1.0f));
//     return rect;
// }

static inline void DrawRectSolid(int x,
                                 int y,
                                 int width,
                                 int height,
                                 float _r,
                                 float _g,
                                 float _b,
                                 float _a)
{
    if(width == 0 || height == 0)
    {
        // Don't render quads that don't have proper width/height.
        return;
    }

    float v_x = x,
        v_x2 = x + width,
        v_y = y,
        v_y2 = y + height;
    pLogDebug("DrawRectSolid: @ %.2f, %.2f size: (%.2f x %.2f)", v_x, v_y, width, height);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);

    glColor4f(_r, _g, _b, _a);
    glVertex2f(v_x, v_y);
    glVertex2f(v_x2, v_y);
    glVertex2f(v_x2, v_y2);
    glVertex2f(v_x, v_y2);

    glEnd();
}

static inline void Vita_DrawImage(
    StdPicture& texture, float x, float y, float wDst, float hDst,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip,
    float r, float g, float b, float a)
{
    float tex_w = texture.w / float(2),
          tex_h = texture.h / float(2);
    float n_src_x = (src_x / tex_w);
    float n_src_x2 = ((src_x + src_w) / tex_w);
    float n_src_y = (src_y / tex_h);
    float n_src_y2 = ((src_y + src_h) / tex_h);

    // TODO: Flip?

    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glBegin(GL_QUADS);

    //              left     top
    glTexCoord2f(n_src_x, n_src_y);
    glVertex2f(x, y);

    //              right     top
    glTexCoord2f(n_src_x2, n_src_y);
    glVertex2f((x + (wDst * 1)), y);

    //              right     bottom
    glTexCoord2f(n_src_x2, n_src_y2);
    glVertex2f((x + (wDst * 1)), (y + (hDst * 1)));

    //              left     bottom
    glTexCoord2f(n_src_x, n_src_y2);
    glVertex2f(x, (y + (hDst * 1)));

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}
