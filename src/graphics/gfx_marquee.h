/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef GFX_MARQUEE_H
#define GFX_MARQUEE_H

#include <cstdint>
#include <string>

#include "fontman/crop_info.h"

struct MarqueeSpec
{
    //! width of marquee
    uint16_t marquee_width = 0;

    //! speed of animation in pixels per ten frames
    uint8_t anim_speed = 0;

    //! pixels to fade text on each side
    uint8_t fade_pixels = 0;

    //! frames to hold text on left and right
    uint8_t hold_frames = 0;

    inline constexpr MarqueeSpec(uint16_t marquee_width, uint8_t anim_speed, uint8_t fade_pixels, uint8_t hold_frames)
        : marquee_width(marquee_width), anim_speed(anim_speed), fade_pixels(fade_pixels), hold_frames(hold_frames) {}
};

class MarqueeState
{
    //! width of currently loaded text in pixels
    uint16_t m_text_width = 0;

    //! current animation frame
    uint16_t m_cur_frame = 0;

public:
    /**
     * \brief: returns marquee's text width
     */
    inline uint16_t check_width() const
    {
        return m_text_width;
    }

    /**
     * \brief: resets the marquee state for a new text width
     */
    void reset_width(uint16_t text_width);

    /**
     * \brief: advance the marquee state by one frame
     */
    void advance(MarqueeSpec spec);

    /**
     * \brief: provides the current crop info for rendering
     */
    CropInfo crop_info(MarqueeSpec spec) const;
};

void SuperPrintMarquee(int SuperN, const char* SuperChars, int Font, float X, float Y, MarqueeSpec spec, MarqueeState& marquee_state, float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f);
void SuperPrintMarquee(const char* SuperChars, int Font, float X, float Y, MarqueeSpec spec, MarqueeState& marquee_state, float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f);
void SuperPrintMarquee(const std::string &SuperWords, int Font, float X, float Y, MarqueeSpec spec, MarqueeState& marquee_state, float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f);

#endif // GFX_MARQUEE_H
