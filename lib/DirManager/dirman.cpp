/*
DirMan - A small crossplatform class to manage directories

Copyright (c) 2017 Vitaliy Novichkov <admin@wohlnet.ru>

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

#include <locale>
#include "dirman.h"
#include "dirman_private.h"

bool matchSuffixFilters(const std::string &name, const std::vector<std::string> &suffixFilters)
{
    bool found = false;
    std::locale loc;

    if(suffixFilters.empty())
        return true;//If no filter, grand everything

    for(const std::string &suffix : suffixFilters)
    {
        if(suffix.size() > name.size())
            continue;

        std::string f;
        f.reserve(name.size());
        for(const char &c : name)
            f.push_back(std::tolower(c, loc));

        found |= (f.compare(f.size() - suffix.size(), suffix.size(), suffix) == 0);

        if(found)
            return true;
    }

    return found;
}

DirMan::DirMan(const std::string &dirPath) :
    d(new DirMan_private)
{
    setPath(dirPath);
}

DirMan::DirMan(const DirMan &dir) :
    d(new DirMan_private)
{
    setPath(dir.d->m_dirPath);
}

DirMan::~DirMan()
{}

void DirMan::setPath(const std::string &dirPath)
{
    d->setPath(dirPath);
}

bool DirMan::getListOfFiles(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters)
{
    return d->getListOfFiles(list, suffix_filters);
}

bool DirMan::getListOfFolders(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters)
{
    return d->getListOfFolders(list, suffix_filters);
}

std::string DirMan::absolutePath()
{
    return d->m_dirPath;
}

bool DirMan::exists()
{
    return exists(d->m_dirPath);
}

bool DirMan::existsRel(const std::string &dirPath)
{
    return exists(d->m_dirPath + "/" + dirPath);
}

bool DirMan::mkdir(const std::string &dirPath)
{
    return mkAbsDir(d->m_dirPath + "/" + dirPath);
}

bool DirMan::rmdir(const std::string &dirPath)
{
    return rmAbsDir(d->m_dirPath + "/" + dirPath);
}

bool DirMan::mkpath(const std::string &dirPath)
{
    return mkAbsPath(d->m_dirPath + "/" + dirPath);
}

bool DirMan::rmpath(const std::string &dirPath)
{
    return rmAbsPath(d->m_dirPath + "/" + dirPath);
}

bool DirMan::beginWalking(const std::vector<std::string> &suffix_filters)
{
    std::locale loc;
    #ifdef _WIN32
    std::wstring             &m_dirPath    = d->m_dirPathW;
    #else
    std::string              &m_dirPath    = d->m_dirPath;
    #endif
    DirMan_private::DirWalkerState &m_walkerState   = d->m_walkerState;

    // Clear previous state
    while(!m_walkerState.digStack.empty())
        m_walkerState.digStack.pop();

    // Initialize suffix filters
    m_walkerState.suffix_filters.clear();
    m_walkerState.suffix_filters.reserve(suffix_filters.size());
    for(const std::string &filter : suffix_filters)
    {
        std::string f;
        f.reserve(filter.size());
        for(const char &c : filter)
            f.push_back(std::tolower(c, loc));
        m_walkerState.suffix_filters.push_back(f);
    }

    // Push initial path
    m_walkerState.digStack.push(m_dirPath);
    return true;
}

bool DirMan::fetchListFromWalker(std::string &curPath, std::vector<std::string> &list)
{
    return d->fetchListFromWalker(curPath, list);
}
