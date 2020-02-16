#ifndef GFX_H
#define GFX_H

#include "range_arr.hpp"
#include "std_picture.h"
#include <vector>
#include <string>

class GFX_t
{
    std::vector<StdPicture*> m_loadedImages;
    void loadImage(StdPicture &img, std::string path);
public:
    GFX_t();
    void load();
    void unLoad();

    StdPicture BMVs;
    StdPicture BMWin;
    RangeArr<StdPicture, 1, 3> Boot;
    RangeArr<StdPicture, 1, 5> CharacterName;
    StdPicture Chat;
    RangeArr<StdPicture, 0, 2> Container;
    RangeArr<StdPicture, 1, 3> ECursor;
    RangeArr<StdPicture, 0, 9> Font1;
    RangeArr<StdPicture, 1, 3> Font2;
    StdPicture Font2S;
    RangeArr<StdPicture, 1, 2> Heart;
    RangeArr<StdPicture, 0, 8> Interface;
    StdPicture LoadCoin;
    StdPicture Loader;
    RangeArr<StdPicture, 0, 3> MCursor;
    RangeArr<StdPicture, 1, 4> MenuGFX;
    RangeArr<StdPicture, 2, 2> Mount;
    RangeArr<StdPicture, 0, 7> nCursor;
    StdPicture TextBox;
    RangeArr<StdPicture, 1, 2> Tongue;
    StdPicture Warp;
    StdPicture YoshiWings;
};

#endif // GFX_H
