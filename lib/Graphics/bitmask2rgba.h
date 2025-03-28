/*
 * Bitmask to RGBA converter: allows to convert graphics with a front (image with
 * a black background) and mask (a dark shape of image on a white background)
 * into RGBA with a transparent background
 *
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#ifdef __cplusplus
extern "C" {
#endif

typedef struct FIBITMAP FIBITMAP;

/**
 * @brief Merge front and mask image into united RGBA image
 * @param [InOut] front
 * @param [In] mask
 */
extern void bitmask_to_rgba(FIBITMAP *front, FIBITMAP *mask);

/**
 * @brief Extract mask from RGBA image
 * @param [In] image RGBA image
 * @param [Out] outmask Output mask image
 */
extern void bitmask_get_mask_from_rgba(FIBITMAP *image, FIBITMAP **outmask);

#ifdef __cplusplus
}
#endif
