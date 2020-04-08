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

    if(SDL_strncmp(magic, GIF1, 6) == 0)
        found = true;

    if(SDL_strncmp(magic, GIF2, 6) == 0)
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

    if(SDL_strncmp(magic, BMP, 2) != 0)
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

    if(SDL_strncmp(magic, PNG, 8) != 0)
        return false;

    if(SDL_RWread(file, magic, 1, 8) != 8)
        return false;

    if(SDL_strncmp(magic + 4, IHDR, 4) != 0)
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


static char *findJpegHead(char *src, size_t src_size)
{
    char *cur = src;
    const char **nd;

    const size_t hSize = 2;

    const char *heads[] =
    {
        /*EXIF*/ "\xFF\xE1", /* needed for bytes to skip */
        /*SOF0*/ "\xff\xc0", /*SOF1*/ "\xff\xc1", /*SOF2*/ "\xff\xc2",
        /*SOF9*/ "\xff\xc9", /*SOF10*/ "\xff\xca",
        nullptr
    };

    while(src_size > 2)
    {
        nd = heads;
        while(*nd)
        {
            if(SDL_memcmp(cur, *nd, hSize) == 0)
            {
#ifdef IMGSIZE_DEBUG
                if(SDL_memcmp(*nd, "\xFF\xE1", 2) != 0)
                    printf("JPEG: got header: [%02X %02X]\n", (*nd)[0], (*nd)[1]);
#endif
                return cur;
            }
            nd++;
        }
        cur++;
        src_size -= 1;
    }
    return nullptr;
}

static bool tryJPEG(SDL_RWops* file, uint32_t *w, uint32_t *h)
{
#define JPEG_BUFFER_SIZE 1024
#define UINT(d) static_cast<unsigned int>(d)
#define BE16(arr, i) (((UINT(arr[i]) << 8) & 0xFF00) | (UINT(arr[i + 1]) & 0xFF))
    const char *JPG1  = "\xFF\xD8\xFF\xDB";
    const char *JPG2_1  = "\xFF\xD8\xFF\xE0", *JPG2_2  = "\x4A\x46\x49\x46\x00\x01";
    const char *JPG3_1  = "\xFF\xD8\xFF\xE1", *JPG3_2  = "\x45\x78\x69\x66\x00\x00";
    char magic[12], raw[JPEG_BUFFER_SIZE];
    char *head = nullptr;
    size_t chunk_size = 0;
    Sint64 pos;

    SDL_RWseek(file, 0, RW_SEEK_SET);

    if(SDL_RWread(file, magic, 1, 10) != 10)
        return false;

    if(SDL_strncmp(magic, JPG1, 4) != 0 &&
      (SDL_strncmp(magic, JPG2_1, 4) != 0 && SDL_strncmp(magic + 6, JPG2_2, 6) != 0) &&
      (SDL_strncmp(magic, JPG3_1, 4) != 0 && SDL_strncmp(magic + 6, JPG3_2, 6) != 0))
        return false;

    do
    {
        SDL_memset(raw, 0, JPEG_BUFFER_SIZE);
        pos = SDL_RWtell(file);
        chunk_size = SDL_RWread(file, raw, 1, JPEG_BUFFER_SIZE);
        if(chunk_size == 0)
            break;

        head = findJpegHead(raw, JPEG_BUFFER_SIZE);
        if(head)
        {
            if(head + 20 >= raw + JPEG_BUFFER_SIZE)
            {
                SDL_RWseek(file, -20, RW_SEEK_CUR);
                continue; /* re-scan this place */
            }

            if(SDL_memcmp(head, "\xFF\xE1", 2) == 0) /* EXIF, skip it!*/
            {
                const Sint64 curPos = pos + (head - raw);
                Sint64 toSkip = BE16(head, 2);
                SDL_RWseek(file, curPos + toSkip + 2, RW_SEEK_SET);
                continue;
            }

            *h = BE16(head, 5);
            *w = BE16(head, 7);
            return true;
        }

    } while(chunk_size > 0);

    return false;
#undef BE16
#undef UINT
#undef JPEG_BUFFER_SIZE
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
    else
    if(tryJPEG(image, w, h))
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

