#ifndef STD_PICTURE_H
#define STD_PICTURE_H

typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;

struct PGEColor
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct SDL_Texture;
struct StdPicture
{
    bool inited = false;
    int w = 0;
    int h = 0;
    int frame_w = 0;
    int frame_h = 0;
    SDL_Texture *texture = nullptr;
    GLenum format = 0;
    GLint  nOfColors = 0;
    PGEColor ColorUpper;
    PGEColor ColorLower;
};

#endif // STD_PICTURE_H
