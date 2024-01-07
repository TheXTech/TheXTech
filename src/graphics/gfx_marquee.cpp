/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <Logger/logger.h>
#include "sdl_proxy/sdl_stdinc.h"

#include "graphics.h"
#include "graphics/gfx_marquee.h"

#include "fontman/font_manager.h"


// implementation for MarqueeState


void MarqueeState::reset_width(uint16_t text_width)
{
    if(m_text_width != text_width)
    {
        m_text_width = text_width;
        m_cur_frame = 0;
    }
}

void MarqueeState::advance(MarqueeSpec spec)
{
    if(m_text_width < spec.marquee_width)
    {
        m_cur_frame = 0;
        return;
    }

    m_cur_frame++;

    // figure out how many frames there are to decide whether or not to wrap

    // animation requires scrolling through the text
    uint16_t scroll_pixels = m_text_width - spec.marquee_width;

    // animation speed is measured in pixels per 10 frames
    uint16_t anim_frames = (scroll_pixels * 10) / spec.anim_speed;

    // there are also hold frames at start and end
    uint16_t hold_frames = spec.hold_frames * 2;

    uint16_t total_frames = anim_frames + hold_frames;

    if(m_cur_frame >= total_frames)
        m_cur_frame = 0;
}

CropInfo MarqueeState::crop_info(MarqueeSpec spec) const
{
    uint16_t scroll_pixels = m_text_width - spec.marquee_width;

    // need to figure out current pixel in full animation sequence (fade + scroll + fade)
    int cur_pixel = 0;

    if(m_cur_frame < spec.hold_frames)
        cur_pixel = 0;
    else
    {
        int cur_frame = m_cur_frame - spec.hold_frames;
        cur_pixel = (cur_frame * spec.anim_speed) / 10;
    }

    if(cur_pixel > scroll_pixels)
        cur_pixel = scroll_pixels;

    // begin filling in a CropInfo object
    CropInfo crop_info;
    crop_info.draw_width = spec.marquee_width;
    crop_info.offset = cur_pixel;

    // set fade values
    if(cur_pixel < spec.fade_pixels)
        crop_info.fade_left = cur_pixel;
    else
        crop_info.fade_left = spec.fade_pixels;

    if(scroll_pixels - cur_pixel < spec.fade_pixels)
        crop_info.fade_right = (scroll_pixels - cur_pixel);
    else
        crop_info.fade_right = spec.fade_pixels;

    return crop_info;
}


void SuperPrintMarquee(int SuperN, const char* SuperChars, int Font, float X, float Y,
                    MarqueeSpec marquee_spec, MarqueeState& marquee_state,
                    XTColor color)
{
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = FontManager::fontIdFromSmbxFont(Font);
    if(dFont < 0)
    {
        pLogWarning("SuperPrint: Invalid font %d is specified", Font);
        return; // Invalid font specified
    }

    if(!marquee_state.check_width())
        marquee_state.reset_width(SuperTextPixLen(SuperN, SuperChars, Font));

    if(marquee_state.check_width() < marquee_spec.marquee_width)
    {
        if(marquee_spec.align > 0)
            X += (marquee_spec.marquee_width - marquee_state.check_width());
        else if(marquee_spec.align == 0)
            X += (marquee_spec.marquee_width - marquee_state.check_width()) / 2;

        marquee_spec.marquee_width = marquee_state.check_width();
    }

    CropInfo ci = marquee_state.crop_info(marquee_spec);

    FontManager::printText(SuperChars, SuperN, X, Y, dFont, color, FontManager::fontSizeFromSmbxFont(Font), outline, {0, 0, 0}, &ci);

    // update text width if unexpected result occurred (didn't terminate while text left, terminated when no text left)
    bool text_left = (ci.offset < marquee_state.check_width() - marquee_spec.marquee_width);

    if(ci.did_terminate ^ text_left)
    {
        marquee_state.reset_width(SuperTextPixLen(SuperN, SuperChars, Font));
        pLogDebug("Text width changed, resetting to %d", (int)marquee_state.check_width());
    }
}

void SuperPrintMarquee(const char* SuperChars, int Font, float X, float Y,
                    MarqueeSpec marquee_spec, MarqueeState& marquee_state,
                    XTColor color)
{
    int len = (int)SDL_strlen(SuperChars);

    return SuperPrintMarquee(len, SuperChars, Font, X, Y,
        marquee_spec, marquee_state,
        color);
}

void SuperPrintMarquee(const std::string& SuperWords, int Font, float X, float Y,
                    MarqueeSpec marquee_spec, MarqueeState& marquee_state,
                    XTColor color)
{
    int len = (int)SuperWords.size();

    return SuperPrintMarquee(len, SuperWords.c_str(), Font, X, Y,
        marquee_spec, marquee_state,
        color);
}
