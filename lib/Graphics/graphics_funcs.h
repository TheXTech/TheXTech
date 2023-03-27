/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"

#include <string>
#include <vector>

#include "size.h"

#ifndef GRAPHICS_FUNCS_H
#define GRAPHICS_FUNCS_H

struct PGE_Pix
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Surface SDL_Surface;

/*!
 * \brief Helpful graphical functions which are doing various work: I/O, Front+Mask blending, etc.
 */
struct FIBITMAP;
class GraphicsHelps
{
public:
    /*!
     * \brief Initializes FreeImage
     */
    static void  initFreeImage();
    /*!
     * \brief DeInitializes FreeImage
     */
    static void  closeFreeImage();

    /*!
     * \brief Loads image from a disk
     * \param file full or relative path to the file
     * \param convertTo32bit need to convert image into 32bit RGBA
     * \return FreeImage descriptor to loaded image
     */
    static FIBITMAP *loadImage(const std::string &file, bool convertTo32bit = true);
    static FIBITMAP *loadImage(std::vector<char> &raw, bool convertTo32bit = true);

    /*!
     * \brief Loads mask image from a disk
     * \param file full or relative path to the file
     * \param maskIsPng if set, set the RGB channels of the mask to the inverse alpha channel of the loaded image
     * \param convertTo32bit need to convert image into 32bit RGBA
     * \return FreeImage descriptor to loaded mask
     */
    static FIBITMAP *loadMask(const std::string &file, bool maskIsPng, bool convertTo32bit = true);
    static FIBITMAP *loadMask(std::vector<char> &raw, bool maskIsPng, bool convertTo32bit = true);

    /*!
     * \brief Loads image from application resources
     * \param file in-resource path to the file
     * \return FreeImage descriptor to loaded image
     */
    //static FIBITMAP *loadImageRC(const char *file);

    /*!
     * \brief Converts FreeImage into SDL_Surface
     * \param img Source FreeImage descriptor to loaded image
     * \return SDL_Surface pointer
     */
    static SDL_Surface *fi2sdl(FIBITMAP *img);

    /*!
     * \brief Closes image and frees memory
     * \param FreeImage descriptor to loaded image
     */
    static void closeImage(FIBITMAP *img);

    /**
     * @brief Generate mask from off RGBA source
     * @param image [in] Source Image
     * @param mask [out] Target image to write a mask
     */
    static void getMaskFromRGBA(FIBITMAP*&image, FIBITMAP *&mask);

    /*!
     * \brief Merges mask and foreground image with bit blitting algorithm
     * 1) draw mask over grey-filled image with bitwise AND per each pixel (white pixels are will not change background)
     * 2) draw foreground over same image with bitwise OR per each pixel (black pixels are will not change background)
     * 3) Calculate alpha-channel level dependent to black-white difference on the mask and on the foreground:
     *    white on the mask is a full transparency, black - is a solid pixels area.
     * \param image
     * \param pathToMask
     * \param pathToMaskFallback
     */
    static void mergeWithMask(FIBITMAP *image,
                              const std::string &pathToMask,
                              const std::string &pathToMaskFallback = std::string());
    static void mergeWithMask(FIBITMAP *image, std::vector<char> &maskRaw, bool maskIsPng = false);
    static void mergeWithMask(FIBITMAP *image, FIBITMAP *mask);

    static void setBitBlitBG(uint8_t red, uint8_t green, uint8_t blue);
    static void resetBitBlitBG();

    /*!
     * \brief Replaces one color with another
     * \param image Image
     * \param src Original pixel color
     * \param dst New pixel color
     */
    static void replaceColor(FIBITMAP *image, const PGE_Pix &src, const PGE_Pix &dst);

    /*!
     * \brief Gets metric from image file
     * \param [__in] imageFile Path to image file
     * \param [__out] imgSize Pointer to PGE_Size value
     */
    static bool getImageMetrics(const std::string &imageFile, PGE_Size *imgSize);
    static void getMaskedImageInfo(const std::string &rootDir,
                                   const std::string &in_imgName,
                                   std::string &out_maskName,
                                   std::string &out_errStr,
                                   PGE_Size *imgSize = nullptr);

    /*!
     * \brief Checks the possibility to shrink down the image with 2x factor without losses
     * This call checks the ability to scale-down an image without getting distortions after process.
     * There are next conditions required:
     * - Image should have metrics being multiple 2
     * - Every 2x2 sector at the image should contain the same color.
     * This means, this image was been scaled up with 2x factor in the past to get this image
     * \param image Image to check
     * \param origPath The optional original path to the texture, needed for log printing
     * \return true - image will be shrank without damages
     */
    static bool validateFor2xScaleDown(FIBITMAP *image, const std::string &origPath = std::string());

    /*!
     * \brief Quickly scales down the image 2x using nearest-neighbor downsampling
     * \param image Image to downscale
     * \return FreeImage descriptor to scaled image
     */
    static FIBITMAP *fast2xScaleDown(FIBITMAP *image);

    /*!
     * \brief Check textures are they require the bitmask render or not
     * \param image Front image
     * \param mask Mask image
     * \param origPath The optional original path to the texture, needed for log printing
     * \return true if bitmask render suggested for better accuracy
     */
    static bool validateBitmaskRequired(FIBITMAP *image, FIBITMAP *mask, const std::string &origPath = std::string());

    /*!
     * \brief Checks the possibility to render the image using a depth test without losses
     * There are next conditions required:
     * - Every pixel in the image must be either fully transparent or fully opaque
     * \param image Image to check
     * \param origPath The optional original path to the texture, needed for log printing
     * \return true - image may be included in draws using the depth test
     */
    static bool validateForDepthTest(FIBITMAP *image, const std::string &origPath = std::string());

    /*!
     * \brief Set the icon for the SDL Window
     * \param window Target window instance
     * \param img Icon image to set
     * \param iconSize The desired size for the icon
     * \return true if success
     */
    static bool setWindowIcon(SDL_Window *window, FIBITMAP *img, int iconSize);
};

#endif // GRAPHICS_FUNCS_H
