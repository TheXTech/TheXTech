/*
DirMan - A small crossplatform class to manage directories

Copyright (c) 2017-2022 Vitaliy Novichkov <admin@wohlnet.ru>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef DIRMAN_PRIVATE_H
#define DIRMAN_PRIVATE_H

#include <string>
#include <stack>
#include <vector>
#include <stdlib.h>

#include "dirman.h"

#ifdef _WIN32
typedef std::wstring    PathString;
#else
typedef std::string     PathString;
#endif

template<class CHAR>
static inline void delEnd(std::basic_string<CHAR> &dirPath, CHAR ch)
{
    if(!dirPath.empty())
    {
        CHAR last = dirPath[dirPath.size() - 1];
        if(last == ch)
            dirPath.resize(dirPath.size() - 1);
    }
}

extern bool matchSuffixFilters(const std::string &name, const std::vector<std::string> &suffixFilters);

class DirMan::DirMan_private
{
    friend class DirMan;

    std::string     m_dirPath;
#ifdef _WIN32
    std::wstring    m_dirPathW;
#endif

    struct DirWalkerState
    {
        std::stack<PathString>      digStack;
        std::vector<std::string>    suffix_filters;
    } m_walkerState;

    void setPath(const std::string &dirPath);
    bool getListOfFiles(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters);
    bool getListOfFolders(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters);
    bool fetchListFromWalker(std::string &curPath, std::vector<std::string> &list);

public:
    DirMan_private() = default;
    DirMan_private(const DirMan_private &) = default;
};

#endif // DIRMAN_PRIVATE_H
