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

#include <stdlib.h>
#include "xerror.h"
#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_assert.h"
#ifdef SDLRPOXY_NULL
#   include <CrashHandler/crash_handler.h>
#endif

void fatal_assert_rangearr_real(long r_low, long r_high, long index, const char *file, const char *func, int line_number)
{
    char assertion_message[1024];
#ifndef SDLRPOXY_NULL
    SDL_AssertData sdl_assert_data = {0, 0, nullptr, 0, 0, 0, 0};
    SDL_AssertState sdl_assert_state;
#endif

    SDL_snprintf(assertion_message, 1024,
        "RangeArray out of range (%ld <= %ld <= %ld) at %s:%d",
        r_low,
        index,
        r_high,
        func,
        line_number
    );

#ifdef SDLRPOXY_NULL
    CrashHandler::logAssertInfo(assertion_message, file, func, line_number);
    abort();
#else
    sdl_assert_data.condition = assertion_message;
    sdl_assert_data.filename = file;
    sdl_assert_data.function = func;
    sdl_assert_data.linenum = line_number;

    sdl_assert_state = SDL_ReportAssertion(&sdl_assert_data, func, file, line_number);

    if(sdl_assert_state == SDL_ASSERTION_BREAK)
        SDL_TriggerBreakpoint();

    abort();
#endif
}
