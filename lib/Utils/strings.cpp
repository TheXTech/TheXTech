/*
 * A small set of extra string processing functions
 *
 * Copyright (c) 2017-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "strings.h"
#include <algorithm>
#include <functional>
#include <string>
#include <cstring>
#include <cctype>
#include <locale>

bool Strings::endsWith(const std::string& str, char what)
{
    if(str.empty())
        return false;
    return (str.back() == what);
}

bool Strings::endsWith(const std::string& str, const std::string& what)
{
    if(str.size() < what.size())
        return false;
    return (str.substr( str.size() - what.size(), what.size()).compare(what) == 0);
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](int c) {return !std::isspace(c); }
    ));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](int c) {return !std::isspace(c); }
    ).base(), s.end());
}

std::string Strings::trim(std::string str)
{
    ::ltrim(str);
    ::rtrim(str);
    return str;
}
std::string Strings::ltrim(std::string str)
{
    ::rtrim(str);
    return str;
}

std::string Strings::rtrim(std::string str)
{
    ::ltrim(str);
    ::rtrim(str);
    return str;
}

void Strings::doTrim(std::string& str)
{
    ::ltrim(str);
    ::rtrim(str);
}

void Strings::doLTrim(std::string &str)
{
    ::ltrim(str);
}

void Strings::doRTrim(std::string &str)
{
    ::rtrim(str);
}


void Strings::split(Strings::List& out, const std::string& str, char delimiter)
{
    std::string::size_type beg = 0;
    std::string::size_type end = 0;
    do
    {
        end = str.find(delimiter, beg);
        if(end == std::string::npos)
            end = str.size();
        out.emplace_back(str.substr(beg, end-beg));
        beg = end + 1;
    }
    while(end < str.size() - 1);
}

void Strings::split(Strings::List& out, const std::string& str, const std::string& delimiter)
{
    std::string::size_type beg = 0;
    std::string::size_type end = 0;
    do
    {
        end = str.find(delimiter, beg);
        if(end == std::string::npos)
            end = str.size();
        out.emplace_back(str.substr(beg, end-beg));
        beg = end + delimiter.size();
    }
    while(end < str.size() - 1);
}

Strings::List Strings::split(const std::string& str, char delimiter)
{
    List res;
    split(res, str, delimiter);
    return res;
}

Strings::List Strings::split(const std::string& str, const std::string& delimiter)
{
    List res;
    split(res, str, delimiter);
    return res;
}

void Strings::replaceInAll(std::string &src, const std::string &from, const std::string &to)
{
    if(from.empty())
        return;

    size_t start_pos = 0;
    while((start_pos = src.find(from, start_pos)) != std::string::npos)
    {
        src.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void Strings::removeInAll(std::string &src, const std::string &substr)
{
    std::string::size_type foundpos = src.find(substr);
    if(foundpos != std::string::npos)
        src.erase(src.begin() + std::string::difference_type(foundpos),
                     src.begin() + std::string::difference_type(foundpos + substr.length()));
}

std::string Strings::removeAll(std::string src, const std::string &substr)
{
    removeInAll(src, substr);
    return src;
}

std::string Strings::replaceAll(std::string src, const std::string &from, const std::string &to)
{
    replaceInAll(src, from, to);
    return src;
}
