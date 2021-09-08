/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_video.h>

#include <Utils/files.h>
#include <FileMapper/file_mapper.h>

#include "graphics_funcs.h"
#include <Logger/logger.h>

#ifdef DEBUG_BUILD
#include <Utils/elapsed_timer.h>
#endif

#include "image_size.h"

//#include <common_features/engine_resources.h>

#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif
#include <FreeImageLite.h>

void GraphicsHelps::initFreeImage()
{
    FreeImage_Initialise();
}

void GraphicsHelps::closeFreeImage()
{
    FreeImage_DeInitialise();
}

FIBITMAP *GraphicsHelps::loadImage(std::string file, bool convertTo32bit)
{
#ifdef DEBUG_BUILD
    ElapsedTimer loadingTime;
    ElapsedTimer fReadTime;
    ElapsedTimer imgConvTime;
    loadingTime.start();
    fReadTime.start();
#endif
#if  defined(__unix__) || defined(__APPLE__) || defined(_WIN32) || defined(__HAIKU__)
    FileMapper fileMap;

    if(!fileMap.open_file(file.c_str()))
        return nullptr;

    FIMEMORY *imgMEM = FreeImage_OpenMemory(reinterpret_cast<unsigned char *>(fileMap.data()),
                                            static_cast<unsigned int>(fileMap.size()));
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory(imgMEM);

    if(formato  == FIF_UNKNOWN)
        return nullptr;

    FIBITMAP *img = FreeImage_LoadFromMemory(formato, imgMEM, 0);
    FreeImage_CloseMemory(imgMEM);
    fileMap.close_file();

    if(!img)
        return nullptr;

#else
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(file.c_str(), 0);

    if(formato  == FIF_UNKNOWN)
        return NULL;

    FIBITMAP *img = FreeImage_Load(formato, file.c_str());

    if(!img)
        return NULL;

#endif
#ifdef DEBUG_BUILD
    long long fReadTimeElapsed = static_cast<long long>(fReadTime.nanoelapsed());
    long long imgConvertElapsed = 0;
#endif

    if(convertTo32bit && (FreeImage_GetBPP(img) != 32))
    {
#ifdef DEBUG_BUILD
        imgConvTime.start();
#endif
        FIBITMAP *temp;
        temp = FreeImage_ConvertTo32Bits(img);

        FreeImage_Unload(img);

        if(!temp)
            return nullptr;

        img = temp;
#ifdef DEBUG_BUILD
        imgConvertElapsed = static_cast<long long>(imgConvTime.nanoelapsed());
#endif
    }

#ifdef DEBUG_BUILD
    D_pLogDebug("File read of texture %s passed in %d nanoseconds", file.c_str(), static_cast<int>(fReadTimeElapsed));
    D_pLogDebug("Conv to 32-bit of %s passed in %d nanoseconds", file.c_str(), static_cast<int>(imgConvertElapsed));
    D_pLogDebug("Total Loading of image %s passed in %d nanoseconds", file.c_str(), static_cast<int>(loadingTime.nanoelapsed()));
#endif
    return img;
}

FIBITMAP *GraphicsHelps::loadImage(std::vector<char> &raw, bool convertTo32bit)
{
    FIMEMORY *imgMEM = FreeImage_OpenMemory(reinterpret_cast<unsigned char *>(raw.data()),
                                            static_cast<unsigned int>(raw.size()));
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory(imgMEM);

    if(formato  == FIF_UNKNOWN)
        return nullptr;

    FIBITMAP *img = FreeImage_LoadFromMemory(formato, imgMEM, 0);
    FreeImage_CloseMemory(imgMEM);

    if(!img)
        return nullptr;

    if(convertTo32bit && (FreeImage_GetBPP(img) != 32))
    {
        FIBITMAP *temp;
        temp = FreeImage_ConvertTo32Bits(img);

        FreeImage_Unload(img);

        if(!temp)
            return nullptr;
        img = temp;
    }

    return img;
}

//FIBITMAP *GraphicsHelps::loadImageRC(const char *file)
//{
//    unsigned char *memory = nullptr;
//    size_t fileSize = 0;
//    SDL_assert_release(RES_getMem(file, memory, fileSize));
//    //{
//        //pLogCritical("Resource file \"%s\" is not found!", file);
//        //return nullptr;
//    //}
//
//    FIMEMORY *imgMEM = FreeImage_OpenMemory(memory, static_cast<FI_DWORD>(fileSize));
//    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory(imgMEM);
//
//    if(formato == FIF_UNKNOWN)
//        return nullptr;
//
//    FIBITMAP *img = FreeImage_LoadFromMemory(formato, imgMEM, 0);
//    FreeImage_CloseMemory(imgMEM);
//
//    if(!img)
//        return nullptr;
//
//    FIBITMAP *temp;
//    temp = FreeImage_ConvertTo32Bits(img);
//
//    if(!temp)
//        return nullptr;
//
//    FreeImage_Unload(img);
//    img = temp;
//    return img;
//}

void GraphicsHelps::closeImage(FIBITMAP *img)
{
    FreeImage_Unload(img);
}

void GraphicsHelps::getMaskFromRGBA(FIBITMAP *&image, FIBITMAP *&mask)
{
    unsigned int img_w   = FreeImage_GetWidth(image);
    unsigned int img_h   = FreeImage_GetHeight(image);

    mask = FreeImage_AllocateT(FIT_BITMAP,
                               int(img_w), int(img_h),
                               int(FreeImage_GetBPP(image)),
                               FreeImage_GetRedMask(image),
                               FreeImage_GetGreenMask(image),
                               FreeImage_GetBlueMask(image));

    RGBQUAD Fpix;
    RGBQUAD Npix = {0x0, 0x0, 0x0, 0xFF};

    for(unsigned int y = 0; (y < img_h); y++)
    {
        for(unsigned int x = 0; (x < img_w); x++)
        {
            FreeImage_GetPixelColor(image, x, y, &Fpix);

            uint8_t grey = (255 - Fpix.rgbReserved);
            Npix.rgbRed  = grey;
            Npix.rgbGreen = grey;
            Npix.rgbBlue = grey;
            Npix.rgbReserved = 0xFF;
            FreeImage_SetPixelColor(mask,  x, y, &Npix);
        }
    }
}

SDL_Surface *GraphicsHelps::fi2sdl(FIBITMAP *img)
{
    int h = static_cast<int>(FreeImage_GetHeight(img));
    int w = static_cast<int>(FreeImage_GetWidth(img));
    FreeImage_FlipVertical(img);
    SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(FreeImage_GetBits(img),
                        w, h, 32, w * 4, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FI_RGBA_ALPHA_MASK);
    return surf;
}

void GraphicsHelps::mergeWithMask(FIBITMAP *image, std::string pathToMask, std::string pathToMaskFallback)
{
    if(!image)
        return;

    if(!Files::fileExists(pathToMask) && pathToMaskFallback.empty())
        return; //Nothing to do

    FIBITMAP *mask = pathToMask.empty() ? nullptr : loadImage(pathToMask, true);
    if(!mask && !pathToMaskFallback.empty())
    {
        FIBITMAP *front = loadImage(pathToMaskFallback, true);
        getMaskFromRGBA(front, mask);
        closeImage(front);
    }

    if(!mask)
        return;//Nothing to do

    mergeWithMask(image, mask);

    FreeImage_Unload(mask);
}

void GraphicsHelps::mergeWithMask(FIBITMAP *image, std::vector<char> &maskRaw, bool maskIsPng)
{
    if(!image)
        return;

    if(maskRaw.empty())
        return; //Nothing to do

    FIBITMAP *mask = loadImage(maskRaw, true);

    if(!mask)
        return;//Nothing to do

    if(maskIsPng)
    {
        FIBITMAP *front = FreeImage_Copy(mask, 0, 0, int(FreeImage_GetWidth(mask)), int(FreeImage_GetHeight(mask)));
        getMaskFromRGBA(front, mask);
        closeImage(front);
    }

    mergeWithMask(image, mask);

    FreeImage_Unload(mask);
}

void GraphicsHelps::mergeWithMask(FIBITMAP *image, FIBITMAP *mask)
{
    unsigned int img_w = FreeImage_GetWidth(image);
    unsigned int img_h = FreeImage_GetHeight(image);
    unsigned int img_pitch = FreeImage_GetPitch(image);
    unsigned int mask_w = FreeImage_GetWidth(mask);
    unsigned int mask_h = FreeImage_GetHeight(mask);
    unsigned int mask_pitch = FreeImage_GetPitch(image);
    BYTE *img_bits  = FreeImage_GetBits(image);
    BYTE *mask_bits = FreeImage_GetBits(mask);
    BYTE *FPixP = nullptr;
    BYTE *SPixP = mask_bits;
    RGBQUAD Npix = {0x00, 0x00, 0x00, 0xFF};   //Destination pixel color
    BYTE Bpix[] = {0x00, 0x00, 0x00, 0xFF};   //Dummy black pixel
    unsigned short newAlpha = 0xFF; //Calculated destination alpha-value

    bool endOfY = false;
    unsigned int ym = mask_h - 1;
    unsigned int y = img_h - 1;

    while(1)
    {
        FPixP = img_bits + (img_pitch * y);
        if(!endOfY)
            SPixP = mask_bits + (mask_pitch * ym);

        for(unsigned int x = 0; (x < img_w); x++)
        {
            Npix.rgbBlue = ((SPixP[FI_RGBA_BLUE] & 0x7F) | FPixP[FI_RGBA_BLUE]);
            Npix.rgbGreen = ((SPixP[FI_RGBA_GREEN] & 0x7F) | FPixP[FI_RGBA_GREEN]);
            Npix.rgbRed = ((SPixP[FI_RGBA_RED] & 0x7F) | FPixP[FI_RGBA_RED]);
            newAlpha = 255 - ((static_cast<unsigned short>(SPixP[FI_RGBA_RED]) +
                               static_cast<unsigned short>(SPixP[FI_RGBA_GREEN]) +
                               static_cast<unsigned short>(SPixP[FI_RGBA_BLUE])) / 3);

            if((SPixP[FI_RGBA_RED] > 240u) //is almost White
               && (SPixP[FI_RGBA_GREEN] > 240u)
               && (SPixP[FI_RGBA_BLUE] > 240u))
                newAlpha = 0;

            newAlpha += ((static_cast<unsigned short>(FPixP[FI_RGBA_RED]) +
                          static_cast<unsigned short>(FPixP[FI_RGBA_GREEN]) +
                          static_cast<unsigned short>(FPixP[FI_RGBA_BLUE])) / 3);

            if(newAlpha > 255) newAlpha = 255;

            FPixP[FI_RGBA_BLUE]  = Npix.rgbBlue;
            FPixP[FI_RGBA_GREEN] = Npix.rgbGreen;
            FPixP[FI_RGBA_RED]   = Npix.rgbRed;
            FPixP[FI_RGBA_ALPHA] = static_cast<BYTE>(newAlpha);
            FPixP += 4;

            if(x >= mask_w - 1 || endOfY)
                SPixP = Bpix;
            else
                SPixP += 4;
        }

        if(ym == 0)
        {
            endOfY = true;
            SPixP = Bpix;
        }
        else
            ym--;

        if(y == 0)
            break;
        y--;
    }
}

bool GraphicsHelps::getImageMetrics(std::string imageFile, PGE_Size* imgSize)
{
    if(!imgSize)
        return false;

    int errorCode;
    uint32_t w, h;

    if(!PGE_ImageInfo::getImageSize(imageFile, &w, &h, &errorCode))
        return false;

    imgSize->setSize(int(w), int(h));
    return true;
}

void GraphicsHelps::getMaskedImageInfo(std::string rootDir, std::string in_imgName, std::string& out_maskName, std::string& out_errStr, PGE_Size* imgSize)
{
    if(in_imgName.empty())
    {
        out_errStr = "Image filename isn't defined";
        return;
    }

    int errorCode;
    uint32_t w, h;

    if(!PGE_ImageInfo::getImageSize(rootDir + in_imgName, &w, &h, &errorCode))
    {
        switch(errorCode)
        {
        case PGE_ImageInfo::ERR_UNSUPPORTED_FILETYPE:
            out_errStr = "Unsupported or corrupted file format: " + rootDir + in_imgName;
            break;

        case PGE_ImageInfo::ERR_NOT_EXISTS:
            out_errStr = "image file is not exist: " + rootDir + in_imgName;
            break;

        case PGE_ImageInfo::ERR_CANT_OPEN:
            out_errStr = "Can't open image file: " + rootDir + in_imgName;
            break;
        }

        return;
    }

    out_maskName = PGE_ImageInfo::getMaskName(in_imgName);
    out_errStr = "";

    if(imgSize)
    {
        imgSize->setWidth(int(w));
        imgSize->setHeight(int(h));
    }
}

bool GraphicsHelps::validateFor2xScaleDown(FIBITMAP *image, const std::string &origPath)
{
    if(!image)
        return false;

    (void)origPath; // supress warning when build the release build

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(image));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(image));
    uint32_t pitch = static_cast<uint32_t>(FreeImage_GetPitch(image));
    BYTE *img_bits  = FreeImage_GetBits(image);

    if(w % 2 || h % 2)
    {
        D_pLogWarning("Texture can't be shrank, non-multiple size: %u x %u (%s)", w, h, origPath.c_str());
        return false; // Not multiple two!
    }

    BYTE *line1 = img_bits;
    BYTE *line2 = (img_bits + pitch);

    for(uint32_t y = 0; y < h; y += 2)
    {
        for(uint32_t x = 0; x < w; x += 2)
        {
            BYTE *p1 = line1 + (y * pitch) + (x * 4);
            BYTE *p2 = line2 + (y * pitch) + (x * 4);
            if(SDL_memcmp(p1, p1 + 4, 4) != 0 ||
               SDL_memcmp(p1, p2, 4) != 0 ||
               SDL_memcmp(p1, p2 + 4, 4) != 0)
            {
                D_pLogWarning("Texture can't be shrank: Pixel colors at the %u x %u sector (2x2 square) aren't equal (%s)", x, y, origPath.c_str());
                return false;
            }
        }
    }

    D_pLogDebug("Texture CAN be shrank (%s)", origPath.c_str());
    return true;
}

bool GraphicsHelps::setWindowIcon(SDL_Window *window, FIBITMAP *img, int iconSize)
{
#ifdef _WIN32
    struct SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version)

    if(SDL_FALSE == SDL_GetWindowWMInfo(window, &wmInfo))
        return false;

    if(wmInfo.subsystem != SDL_SYSWM_WINDOWS)
        return false;

    HWND windowH = wmInfo.info.win.window;
    HICON hicon = nullptr;
    BYTE *icon_bmp = nullptr;
    unsigned int icon_len, y;
    SDL_RWops *dst;
    unsigned int w = FreeImage_GetWidth(img);
    unsigned int h = FreeImage_GetWidth(img);
    Uint8 *bits = (Uint8 *)FreeImage_GetBits(img);
    unsigned int pitch = FreeImage_GetPitch(img);
    /* Create temporary bitmap buffer */
    icon_len = 40 + h * w * 4;
    icon_bmp = SDL_stack_alloc(BYTE, icon_len);
    dst = SDL_RWFromMem(icon_bmp, icon_len);

    if(!dst)
    {
        SDL_stack_free(icon_bmp);
        return false;
    }

    /* Write the BITMAPINFO header */
    SDL_WriteLE32(dst, 40);
    SDL_WriteLE32(dst, w);
    SDL_WriteLE32(dst, h * 2);
    SDL_WriteLE16(dst, 1);
    SDL_WriteLE16(dst, 32);
    SDL_WriteLE32(dst, BI_RGB);
    SDL_WriteLE32(dst, h * w * 4);
    SDL_WriteLE32(dst, 0);
    SDL_WriteLE32(dst, 0);
    SDL_WriteLE32(dst, 0);
    SDL_WriteLE32(dst, 0);
    y = 0;

    do
    {
        Uint8 *src = bits + y * pitch;
        SDL_RWwrite(dst, src, pitch, 1);
    }
    while(++y < h);

    hicon = CreateIconFromResource(icon_bmp, icon_len, TRUE, 0x00030000);
    SDL_RWclose(dst);
    SDL_stack_free(icon_bmp);
    /* Set the icon for the window */
    SendMessage(windowH, WM_SETICON, (iconSize < 32) ? ICON_SMALL : ICON_BIG, (LPARAM) hicon);
#else
    (void)iconSize;
    SDL_Surface *sicon = GraphicsHelps::fi2sdl(img);
    SDL_SetWindowIcon(window, sicon);
    SDL_FreeSurface(sicon);
    const char *error = SDL_GetError();

    if(*error != '\0')
        return false;

#endif
    return true;
}


/*********************Code from Qt**end****************/
