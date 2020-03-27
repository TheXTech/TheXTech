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


#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

//#include <luabind/luabind.hpp>
//#include <lua_includes/lua.hpp>

#ifdef Q_CC_GNU
#define gcc_force_inline __attribute__((always_inline, gnu_inline))
#else
#define gcc_force_inline
#endif

namespace util
{
std::string filePath(std::string s);
bool strempty(const char *str);

std::string resolveRelativeOrAbsolute(const std::string &path, const std::vector<std::string> &relativeLookup);

void CSV2IntArr(std::string source, std::vector<int> &dest);
void CSV2DoubleArr(std::string source, std::vector<double> &dest);
void base64_encode(std::string &ret, const unsigned char *bytes_to_encode, size_t in_len);
void base64_decode(std::string &ret, std::string const &encoded_string);

}

namespace varadic_util
{
template<int ...>
struct seq { };

template<int N, int ...S>
struct gens : gens < N - 1, N - 1, S... > { };

template<int ...S>
struct gens<0, S...>
{
    typedef seq<S...> type;
};
}



//namespace luabind_utils
//{
//template<typename T>
//static inline gcc_force_inline std::vector<T> convArrayTo(luabind::object &obj)
//{
//    std::vector<T> container;
//    for(luabind::iterator it(obj), end; it != end; ++it)
//    {
//        try
//        {
//            container.push_back(luabind::object_cast<T>(*it));
//        }
//        catch(luabind::cast_failed & /*e*/) { }
//    }
//    return container;
//}
//}

namespace charsets_utils
{
/*!
 * \brief returns length of UTF8 string line
 * \param Input 8-bit string in UTF8 codepage
 * \return number of characters (not a bytes!)
 */
size_t utf8len(const char *s);
int UTF8Str_To_WStr(std::wstring &dest, const std::string  &source);
int WStr_To_UTF8Str(std::string  &dest, const std::wstring &source);
}

#endif // UTIL_H
