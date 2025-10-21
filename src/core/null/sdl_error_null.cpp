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

#include "sdl_proxy/sdl_stdinc.h"
#include <cstdarg>
#include <cstdio>

static char s_error[1024] = "";

const char *SDL_GetError()
{
    return s_error;
}

void SDL_ClearError()
{
    s_error[0] = '\0';
}

void SDL_SetError(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    std::vsnprintf(s_error, 1024, fmt, arg);
    va_end(arg);
}
