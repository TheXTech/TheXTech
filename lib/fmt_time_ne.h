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

#include <fmt/fmt_time.h>
#include <fmt/fmt_printf.h>
#include <Logger/logger.h>

namespace fmt
{

inline std::tm localtime_ne(std::time_t time)
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

}
