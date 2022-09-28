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

#ifndef _WIN32
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory.h>

#include "dirman.h"
#include "dirman_private.h"

void DirMan::DirMan_private::setPath(const std::string &dirPath)
{
    m_dirPath = dirPath;
    delEnd(m_dirPath, '/');
}

bool DirMan::DirMan_private::getListOfFiles(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters)
{
    list.clear();

    dirent *dent = nullptr;
    DIR *srcdir = opendir(m_dirPath.c_str());
    if(srcdir == nullptr)
        return false;

    while((dent = readdir(srcdir)) != nullptr)
    {
        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;

        if(dent->d_type == DT_REG)
        {
            if(matchSuffixFilters(dent->d_name, suffix_filters))
                list.emplace_back(dent->d_name);
        }
    }
    closedir(srcdir);
    return true;
}

bool DirMan::DirMan_private::getListOfFolders(std::vector<std::string>& list, const std::vector<std::string>& suffix_filters)
{
    list.clear();
    dirent *dent = nullptr;
    DIR *srcdir = opendir(m_dirPath.c_str());
    if(srcdir == nullptr)
        return false;

    while((dent = readdir(srcdir)) != nullptr)
    {
        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;

        if (dent->d_type == DT_DIR)
        {
            if(matchSuffixFilters(dent->d_name, suffix_filters))
                list.emplace_back(dent->d_name);
        }
    }
    closedir(srcdir);
    return true;
}

bool DirMan::DirMan_private::fetchListFromWalker(std::string &curPath, std::vector<std::string> &list)
{
    if(m_walkerState.digStack.empty())
        return false;

    list.clear();

    std::string path = m_walkerState.digStack.top();
    m_walkerState.digStack.pop();

    dirent *dent = nullptr;
    DIR *srcdir = opendir(path.c_str());
    if(srcdir == nullptr) //Can't read this directory. Continue
        return true;

    while((dent = readdir(srcdir)) != nullptr)
    {
        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;

        if(dent->d_type == DT_DIR)
            m_walkerState.digStack.push(path + "/" + dent->d_name);
        else if(dent->d_type == DT_REG)
        {
            if(matchSuffixFilters(dent->d_name, m_walkerState.suffix_filters))
                list.emplace_back(dent->d_name);
        }
    }
    closedir(srcdir);
    curPath = path;

    return true;
}

bool DirMan::exists(const std::string &dirPath)
{
    DIR *dir = opendir(dirPath.c_str());
    if(dir)
    {
        closedir(dir);
        return true;
    }
    else
        return false;
}

bool DirMan::mkAbsDir(const std::string &dirPath)
{
    return ::mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG) == 0;
}

bool DirMan::rmAbsDir(const std::string &dirPath)
{
    return ::rmdir(dirPath.c_str()) == 0;
}

bool DirMan::mkAbsPath(const std::string &dirPath)
{
    char tmp[PATH_MAX + 1];
    char *p = nullptr;
    size_t len;

    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, PATH_MAX, "%s", dirPath.c_str());
    len = strlen(tmp);

    if(len > 0 && tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for(p = tmp + 1; *p; p++)
    {
        if(*p == '/')
        {
            *p = 0;
            int err = ::mkdir(tmp, S_IRWXU | S_IRWXG);
            if((err != 0) && (errno != EEXIST))
            {
                *p = '/';
                return false;
            }
            *p = '/';
        }
    }
    return ::mkdir(tmp, S_IRWXU | S_IRWXG) == 0;
}

bool DirMan::rmAbsPath(const std::string &dirPath)
{
    int ret = 0;
    struct DirStackEntry
    {
        std::string path;
        DIR         *d;
        struct dirent *p;
    };

    std::stack<DirStackEntry> dirStack;
    dirStack.push({dirPath, nullptr, nullptr});

    while(!dirStack.empty())
    {
        DirStackEntry *e = &dirStack.top();
        e->d = opendir(e->path.c_str());

        bool walkUp = false;
        if(e->d)
        {
            while((e->p = readdir(e->d)) != nullptr)
            {
                if(strcmp(e->p->d_name, ".") == 0 || strcmp(e->p->d_name, "..") == 0)
                    continue;

                if(e->p->d_type == DT_UNKNOWN)
                    continue;

                std::string path = e->path + "/" + e->p->d_name;

                if(e->p->d_type == DT_DIR)
                {
                    closedir(e->d);
                    e->d = nullptr;
                    dirStack.push({path, nullptr, nullptr});
                    walkUp = true;
                    break;
                }
                else
                {
                    if(::unlink(path.c_str()) != 0)
                        ret = -1;
                }
            }
        }

        if(!walkUp)
        {
            if(e->d)
                closedir(e->d);
            e->d = nullptr;
            if(::rmdir(e->path.c_str()) != 0)
                ret = -1;
            e = nullptr;
            dirStack.pop();
        }
    }

    return (ret == 0);
}

#endif
