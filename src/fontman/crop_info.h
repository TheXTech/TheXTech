/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once
#ifndef CROP_INFO_H
#define CROP_INFO_H

#include <cstdint>

struct CropInfo
{
    //! width of current text in pixels
    // uint16_t text_width = 0;

    //! width to draw the marquee effect in pixels
    uint16_t draw_width = 0;

    //! draw offset from left in pixels
    uint16_t offset = 0;

    //! fade amount in pixels (left)
    uint8_t fade_left = 0;

    //! fade amount in pixels (right)
    uint8_t fade_right = 0;

    //! use tweaked alpha curve for text outline
    bool for_outline = false;

    //! OUTPUT variable: did the text overrun the crop window?
    bool did_terminate = false;

    /**
     * \brief: calculates alpha multiplier given left and right bounds of letter
     *
     * \param letter_alpha output to store the letter's alpha value
     * \param text_alpha the text's maximum alpha value between zero and one
     * \param left left bound of letter in pixels from the left of the cropped region
     * \param right right bound of letter in pixels from the left of the cropped region
     *
     * \returns false if the right bound has been passed and drawing should be interrupted
     *
     * Side effect: marks the did_terminate flag if returning false
     */
    inline bool letter_alpha(uint8_t& letter_alpha, uint16_t text_alpha, int left, int right)
    {
        if(left < 0)
        {
            letter_alpha = 0;
            return true;
        }

        if((uint16_t)right > draw_width)
        {
            did_terminate = true;
            return false;
        }

        uint16_t fade_zone_right = draw_width - fade_right;

        if((uint16_t)left < fade_left)
        {
            letter_alpha = (text_alpha * left) / fade_left;

            // blend based on proportion in fade zone
            if((uint16_t)right > fade_left)
                letter_alpha = (text_alpha * (right - fade_left) + letter_alpha * (fade_left - left)) / (right - left);
        }
        else if((uint16_t)right > fade_zone_right)
        {
            letter_alpha = (text_alpha * (draw_width - (uint16_t)right)) / fade_right;

            // blend based on proportion in fade zone
            if((uint16_t)left < fade_zone_right)
                letter_alpha = (text_alpha * (fade_zone_right - left) + letter_alpha * (right - fade_zone_right)) / (right - left);
        }
        else
            letter_alpha = text_alpha;

        if(for_outline)
            letter_alpha = letter_alpha * letter_alpha / text_alpha;

        return true;
    }
};

#endif // CROP_INFO_H
