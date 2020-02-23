/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include "image_size.h"
#include <SDL2/SDL_rwops.h>
#include <Utils/files.h>

static bool tryGIF(SDL_RWops* file, uint32_t *w, uint32_t *h)
{
    SDL_RWseek(file, 0, RW_SEEK_SET);
    const char *GIF1 = "GIF87a";
    const char *GIF2 = "GIF89a";
    char magic[6];

    if(SDL_RWread(file, magic, 1, 6) != 6)
        return false;

    bool found = false;

    if(strncmp(magic, GIF1, 6) == 0)
        found = true;

    if(strncmp(magic, GIF2, 6) == 0)
        found = true;

    if(!found)
        return false;

    unsigned char size[4];

    if(SDL_RWread(file, reinterpret_cast<char *>(size), 1, 4) != 4)
        return false;

#define UINT(d) static_cast<unsigned int>(d)
    *w = ((UINT(size[0]) & 0x00FF) | ((UINT(size[1]) << 8) & 0xFF00));
    *h = ((UINT(size[2]) & 0x00FF) | ((UINT(size[3]) << 8) & 0xFF00));
#undef UINT

    return true;
}

static bool tryBMP(SDL_RWops* file, uint32_t *w, uint32_t *h)
{
    SDL_RWseek(file, 0, RW_SEEK_SET);
    const char *BMP = "BM";
    char magic[2];

    if(SDL_RWread(file, magic, 1, 2) != 2)
        return false;

    if(strncmp(magic, BMP, 2) != 0)
        return false;

    unsigned char size[8];

    if(SDL_RWseek(file, 18, RW_SEEK_SET) < 0)
        return false;

    if(SDL_RWread(file, reinterpret_cast<char *>(size), 1, 8) != 8)
        return false;

#define UINT(d) static_cast<unsigned int>(d)
    *w = ((UINT(size[0]) & 0xFF) | ((UINT(size[1]) << 8) & 0xFF00) |
         ((UINT(size[2]) << 16) & 0xFF0000) | ((UINT(size[3]) << 24) & 0xFF000000));

    *h = ((UINT(size[4]) & 0xFF) | ((UINT(size[5]) << 8) & 0xFF00) |
         ((UINT(size[6]) << 16) & 0xFF0000) | ((UINT(size[7]) << 24) & 0xFF000000));
#undef UINT

    return true;
}

static bool tryPNG(SDL_RWops* file, uint32_t *w, uint32_t *h)
{
    SDL_RWseek(file, 0, RW_SEEK_SET);
    const char *PNG  = "\211PNG\r\n\032\n";
    const char *IHDR = "IHDR";
    char magic[8];

    if(SDL_RWread(file, magic, 1, 8) != 8)
        return false;

    if(strncmp(magic, PNG, 8) != 0)
        return false;

    if(SDL_RWread(file, magic, 1, 8) != 8)
        return false;

    if(strncmp(magic + 4, IHDR, 4) != 0)
        return false;

    unsigned char size[8];

    if(SDL_RWread(file, reinterpret_cast<char *>(size), 1, 8) != 8)
        return false;

#define UINT(d) static_cast<unsigned int>(d)
    *w = ((UINT(size[3]) & 0xFF) | ((UINT(size[2]) << 8) & 0xFF00) | ((UINT(size[1]) << 16) & 0xFF0000) | ((UINT(size[0]) << 24) & 0xFF000000));
    *h = ((UINT(size[7]) & 0xFF) | ((UINT(size[6]) << 8) & 0xFF00) | ((UINT(size[5]) << 16) & 0xFF0000) | ((UINT(size[4]) << 24) & 0xFF000000));
#undef UINT

    return true;
}

bool PGE_ImageInfo::getImageSizeRW(SDL_RWops *image, uint32_t *w, uint32_t *h, int *errCode)
{
    bool ret = false;
    if(tryGIF(image, w, h))
        ret = true;
    else
    if(tryPNG(image, w, h))
        ret = true;
    else
    if(tryBMP(image, w, h))
        ret = true;

    SDL_RWclose(image);

    if(!ret)
    {
        if(errCode)
            *errCode = ERR_UNSUPPORTED_FILETYPE;
    }

    return ret;
}

bool PGE_ImageInfo::getImageSize(PGEString imagePath, uint32_t *w, uint32_t *h, int *errCode)
{
    if(errCode)
        *errCode = ERR_OK;

    std::string imgPath = PGEStringToStd(imagePath);

    if(!Files::fileExists(imgPath))
    {
        if(errCode)
            *errCode = ERR_NOT_EXISTS;

        return false;
    }

    SDL_RWops* image = SDL_RWFromFile(imgPath.c_str(), "rb");

    if(!image)
    {
        if(errCode)
            *errCode = ERR_CANT_OPEN;

        return false;
    }

    return getImageSizeRW(image, w, h, errCode);
}

bool PGE_ImageInfo::getImageSizeFromMem(const char *mem, size_t size, uint32_t *w, uint32_t *h, int *errCode)
{
    if(errCode)
        *errCode = ERR_OK;

    SDL_RWops* image = SDL_RWFromConstMem(mem, int(size));

    if(!image)
    {
        if(errCode)
            *errCode = ERR_CANT_OPEN;

        return false;
    }

    return getImageSizeRW(image, w, h, errCode);
}


PGEString PGE_ImageInfo::getMaskName(PGEString imageFileName)
{
    std::string mask = PGEStringToStd(imageFileName);
    //Make mask filename
    size_t dotPos = mask.find_last_of('.');
    if(dotPos == std::string::npos)
        mask.push_back('m');
    else
        mask.insert(mask.begin() + std::string::difference_type(dotPos), 'm');
    return StdToPGEString(mask);
}


