/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <stdarg.h>
#include <fmt/fmt_time.h>
#include <fmt/fmt_printf.h>
#include <fmt/fmt_format.h>
#include <fmt/fmt_printf.h>
#include "sdl_proxy/sdl_stdinc.h"
#include "Logger/logger.h"

#include "fmt_format_ne.h"
#include "fmt_time_ne.h"

namespace fmt
{

std::tm localtime_ne(std::time_t time)
{
    try
    {
        return localtime(time);
    }
    catch(const FormatError &e)
    {
        std::tm t;
        std::memset(&t, 0, sizeof(std::tm));
        pLogFatal("fmt::format error: Thrown exception [%s] on attempt to process localtime", e.what());
        return t;
    }
    catch(...)
    {
        std::tm t;
        std::memset(&t, 0, sizeof(std::tm));
        pLogFatal("fmt::format error: Thrown unknown exception on attempt to process localtime");
        return t;
    }
}

std::string sprintf_ne(const char *fstr, ...)
{
    std::string ret;
    va_list arg;
    int len = SDL_strlen(fstr);
    int argLen = 0;

    // Calculate coarse length of arguments
    for(const char *b = fstr; *b; ++b)
    {
        if(*b == '%')
        {
            ++b;
            if(*b == 's')
                argLen += 1000; // Reserve ~1000 for every string
            else if(*b == '%')
                continue; // Not an arg!
            else
                argLen += 10;
        }
    }

    ret.resize(len + argLen);

    va_start(arg, fstr);
    len = SDL_vsnprintf(&ret[0], ret.size(), fstr, arg);
    va_end(arg);

    ret.resize(len);

    return ret;
}

}
