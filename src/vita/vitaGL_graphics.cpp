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

float _colors[] =
{
    1.0, 1.0, 1.0, 1.0,
    0.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0,

    1.0, 1.0, 1.0, 1.0,
    0.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 0.0, 1.0,
};

float _colors_def[] = 
{
    1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0,

    1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0,
};

float _vertices[] = 
{
    0, 0,
    1, 0,
    0, 1,

    1, 0,
    0, 1,
    1, 1,
};

float _texcoords[] =
{
    0, 0,
    1, 0,
    0, 1,
    
    1, 0,
    0, 1,
    1, 1
};

uint16_t _indices[] = 
{
    2,
    0, 
    5,
    1,
};

void DrawRectSolid(int x,
                                 int y,
                                 int width,
                                 int height,
                                 float _r,
                                 float _g,
                                 float _b,
                                 float _a)
{
#if 0
    // pLogDebug("Draw Rect Solid: %d, %d (%d x %d)", x, y, width, height);
    if(width == 0 || height == 0)
    {
        // Don't render quads that don't have proper width/height.
        return;
    }

    float v_x = (x),
        v_x2 = (x + width),
        v_y = (y),
        v_y2 = (y + height);
    
    _vertices[0] = 0;//v_x;
    _vertices[1] = 0;//v_y;

    _vertices[3] = 50;//v_x2;
    _vertices[4] = 0;//v_y;

    _vertices[6] = 0;//v_x;
    _vertices[7] = 50;//v_y2;

    _vertices[9] = 800;
    _vertices[10] = 0;

    _vertices[12] = 850;
    _vertices[13] = 0;

    _vertices[15] = 800;
    _vertices[16] = 200;


    // pLogDebug("DrawRectSolid: @ %.2f, %.2f size: (%.2f x %.2f)", v_x, v_y, float(width), float(height));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, _vertices);
    glColorPointer(3, GL_FLOAT, 0, _colors);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glEnd();
#else
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);
    

    //              left     top
    glColor4f(_r, _g, _b, _a);
    glVertex3f(x, y, 0);

    //              right     top
    glColor4f(_r, _g, _b, _a);
    glVertex3f(x + width, y, 0);

    //              right     bottom
    glColor4f(_r, _g, _b, _a);
    glVertex3f(x + width, y + height, 0);

    //              left     bottom
    glColor4f(_r, _g, _b, _a);
    glVertex3f(x, y + height, 0);

    glEnd();

#endif
}


static void _vglsetup_arrays(float x, float y, float wDst, float hDst, float n_src_x, float n_src_x2, float n_src_y, float n_src_y2)
{
    _vertices[0] = x;
    _vertices[1] = y;
    
    _vertices[2] = x + wDst;
    _vertices[3] = y;

    _vertices[4] = x;
    _vertices[5] = y + hDst;

    _vertices[6] = x + wDst;
    _vertices[7] = y;

    _vertices[8] = x;
    _vertices[9] = y + hDst;

    _vertices[10] = x + hDst;
    _vertices[11] = y + hDst;
///////////////////////////////////
    _texcoords[0] = n_src_x;
    _texcoords[1] = n_src_y;

    _texcoords[2] = n_src_x2;
    _texcoords[3] = n_src_y;

    _texcoords[4] = n_src_x;
    _texcoords[5] = n_src_y2;

    _texcoords[6] = n_src_x2;
    _texcoords[7] = n_src_y;

    _texcoords[8] = n_src_x;
    _texcoords[9] = n_src_y2;

    _texcoords[10] = n_src_x2;
    _texcoords[11] = n_src_y2;
}


void Vita_DrawImage(
    StdPicture& texture, float x, float y, float wDst, float hDst,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip,
    float r, float g, float b, float a)
{
    float tex_w = float(texture.w) / float(2),
          tex_h = float(texture.h) / float(2);
    float n_src_x = (src_x / tex_w);
    float n_src_x2 = ((src_x + src_w) / tex_w);
    float n_src_y = (src_y / tex_h);
    float n_src_y2 = ((src_y + src_h) / tex_h);

#if 0
    _vglsetup_arrays(x, y, wDst, hDst, n_src_x, n_src_x2, n_src_y, n_src_y2);

////////////////////////////////////
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.texture);


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, _vertices);
    glColorPointer(4, GL_FLOAT, 0, _colors_def);
    glTexCoordPointer(2, GL_FLOAT, 0, _texcoords);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, _indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else

    // TODO: Actually use OpenGL ES standards 
    // and fun things like glDrawElements

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
#endif

    // glBindTexture(GL_TEXTURE_2D, 0);
}
