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

#ifndef IMAGESIZE_H
#define IMAGESIZE_H

#include "PGEString.h"

struct SDL_RWops;

/**
 * @brief Basic image information misc. functions
 */
namespace PGE_ImageInfo
{

/**
 * @brief Error codes of image information retrieving
 */
enum ErrCodes
{
    //! No errors, everything is OK
    ERR_OK,
    //! Image file type is not supported
    ERR_UNSUPPORTED_FILETYPE,
    //! File not exists
    ERR_NOT_EXISTS,
    //! Can't open the file
    ERR_CANT_OPEN
};

bool getImageSizeRW(SDL_RWops *image, uint32_t *w, uint32_t *h, int *errCode=nullptr);

/**
 * @brief Quickly get image size (width and height) from image file.
 * @param [in] imagePath Path to image file
 * @param [out] w Width of image
 * @param [out] h Height of image
 * @param [out] errCode Error code
 * @return true if successfully finished, false if error occouped
 */
bool getImageSize(PGEString imagePath, uint32_t *w, uint32_t *h, int *errCode=nullptr);

bool getImageSizeFromMem(const char *mem, size_t size, uint32_t *w, uint32_t *h, int *errCode=nullptr);

/**
 * @brief Returns filename of masked image
 * @param [in] imageFileName foreground image file-name
 * @return masked image filename
 */
PGEString getMaskName(PGEString imageFileName);

}//PGE_ImageInfo

#endif // IMAGESIZE_H
