/*
 * A small set of extra string processing functions
 *
 * Copyright (c) 2017-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef STRINGS_H
#define STRINGS_H

#include <string>
#include <vector>

namespace Strings
{
    typedef std::vector<std::string> List;
    bool endsWith(const std::string &str, char what);
    bool endsWith(const std::string &str, const std::string &what);
    std::string trim(std::string str);
    std::string ltrim(std::string str);
    std::string rtrim(std::string str);
    void doTrim(std::string &str);
    void doLTrim(std::string &str);
    void doRTrim(std::string &str);
    void split(List &out, const std::string &str, char delimiter);
    void split(List &out, const std::string &str, const std::string &delimiter);
    List split(const std::string &str, char delimiter);
    List split(const std::string &str, const std::string &delimiter);
    void replaceInAll(std::string &src, const std::string &from, const std::string &to);
    void removeInAll(std::string &src, const std::string &substr);
    std::string replaceAll(std::string src, const std::string &from, const std::string &to);
    std::string removeAll(std::string src, const std::string &substr);
}
#endif // STRINGS_H
