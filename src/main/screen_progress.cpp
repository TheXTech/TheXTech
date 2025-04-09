/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cstdint>
#include <string>

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_timer.h"

#include "gfx.h"
#include "graphics.h" // SuperPrint

#include "core/render.h"
#include "core/events.h"

#include "main/game_strings.h"

static const uint32_t c_progress_screen_min_elapsed = 1000;
static const uint32_t c_progress_screen_tick_duration = 250;

void IndicateProgress(uint32_t start_time, num_t progress, const std::string& message)
{
#ifdef USE_RENDER_BLOCKING
    if(XRender::renderBlocked())
        return;
#endif

    bool progress_valid = (progress > 0);

    if(progress > 1)
        progress = 1;

    uint32_t cur_time = SDL_GetTicks();
    uint32_t elapsed = cur_time - start_time;
    uint32_t total = progress_valid ? uint32_t(1024 * elapsed / uint16_t(1024 * progress)) : 0;

    if(elapsed < c_progress_screen_min_elapsed && total < c_progress_screen_min_elapsed)
        return;

    int load_coins_new = (elapsed / c_progress_screen_tick_duration) % 4;

    if(load_coins_new == LoadCoins)
        return;

    LoadCoins = load_coins_new;

    XRender::setTargetTexture();
    XRender::clearBuffer();

    SuperPrintScreenCenter(message.c_str(), 4,
               XRender::TargetH / 2 - 40,
               {255, 255, 0, 127});

    int time_y = XRender::TargetH / 2 - 20;

    // outline
    XRender::renderRect(XRender::TargetW / 4, time_y + 4, XRender::TargetW / 2, 32, {255, 255, 255});
    // empty progress
    XRender::renderRect(XRender::TargetW / 4 + 2, time_y + 6, XRender::TargetW / 2 - 4, 28, {0, 0, 0});
    // progress fill
    if(progress > 0)
        XRender::renderRect(XRender::TargetW / 4 + 2, time_y + 6, (int)((XRender::TargetW / 2 - 4) * progress), 28, {127, 255, 127});
    // push text down
    time_y += 60;

    std::string time_message = fmt::format_ne(g_gameStrings.formatMinutesSeconds, elapsed / 60000, (elapsed / 1000) % 60);
    if(progress_valid)
    {
        time_message += " / ";
        time_message += fmt::format_ne(g_gameStrings.formatMinutesSeconds, total / 60000, (total / 1000) % 60);
    }

    SuperPrintScreenCenter(time_message.c_str(), 4,
               time_y,
               {255, 255, 255, 127});

    XRender::renderTextureBasic(XRender::TargetW / 2 - GFX.LoadCoin.w / 2, time_y + 40, GFX.LoadCoin.w, GFX.LoadCoin.h / 4, GFX.LoadCoin, 0, 32 * LoadCoins);

    XRender::repaint();
    XRender::setTargetScreen();
    XEvents::doEvents();

#ifdef __EMSCRIPTEN__
    emscripten_sleep(1); // To repaint screenn, it's required to send a sleep signal
#endif
}
