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

#ifndef FMT_FORMAT_NE_H
#define FMT_FORMAT_NE_H

#include <fmt/fmt_format.h>
#include <fmt/fmt_printf.h>
#include "Logger/logger.h"

namespace fmt
{
/*
    Exception-less fmt::format version. Instead of exception, the error message will be logged into file
*/
template <typename... Args>
std::string format_ne(CStringRef format_str, const Args & ... args)
{
    try
    {
        return format(format_str, std::forward<const Args&>(args)...);
    }
    catch(const FormatError &e)
    {
        std::string out;
        pLogWarning("fmt::format error: Thrown exception [%s] on attempt to process string [%s]",
                    e.what(),
                    format_str.c_str());
        out.append(e.what());
        out.append(" [");
        out.append(format_str.c_str());
        out.push_back(']');
        return out;
    }
    catch(...)
    {
        pLogWarning("fmt::format error: Thrown unknown exception on attempt to process string [%s]", format_str.c_str());
        return "<ERROR OF " + std::string(format_str.c_str()) + ">";
    }
}

template <typename... Args>
std::string sprintf_ne(CStringRef format_str, const Args & ... args)
{
    try
    {
        return sprintf(format_str, std::forward<const Args&>(args)...);
    }
    catch(const FormatError &e)
    {
        std::string out;
        pLogWarning("fmt::sprintf error: Thrown exception [%s] on attempt to process string [%s]",
                    e.what(),
                    format_str.c_str());
        out.append(e.what());
        out.append(" [");
        out.append(format_str.c_str());
        out.push_back(']');
        return out;
    }
    catch(...)
    {
        pLogWarning("fmt::sprintf error: Thrown unknown exception on attempt to process string [%s]", format_str.c_str());
        return "<ERROR OF " + std::string(format_str.c_str()) + ">";
    }
}

}//namespace fmt

#endif // FMT_FORMAT_NE_H
