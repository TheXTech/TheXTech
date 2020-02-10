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

#ifdef _WIN32
#include <windows.h>
#include <algorithm>

#include "dirman.h"
#include "dirman_private.h"

static std::wstring Str2WStr(const std::string &str)
{
    std::wstring dest;
    dest.resize(str.size());
    int newlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &dest[0], static_cast<int>(str.length()));
    dest.resize(newlen);
    return dest;
}

static std::string WStr2Str(const std::wstring &wstr)
{
    std::string dest;
    dest.resize((wstr.size() * 2));
    int newlen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &dest[0], static_cast<int>(dest.size()), NULL, NULL);
    dest.resize(newlen);
    return dest;
}

void DirMan::DirMan_private::setPath(const std::string &dirPath)
{
    m_dirPathW = Str2WStr(dirPath);
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(m_dirPathW.c_str(), MAX_PATH, fullPath, NULL);
    m_dirPathW = fullPath;
    //Force UNIX paths
    std::replace(m_dirPathW.begin(), m_dirPathW.end(), L'\\', L'/');
    m_dirPath = WStr2Str(m_dirPathW);
    delEnd(m_dirPathW, L'/');
    delEnd(m_dirPath, '/');
}

bool DirMan::DirMan_private::getListOfFiles(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters)
{
    list.clear();
    HANDLE hFind;
    WIN32_FIND_DATAW data;

    hFind = FindFirstFileW((m_dirPathW + L"/*").c_str(), &data);
    if(hFind == INVALID_HANDLE_VALUE)
        return false;
    do
    {
        if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if((wcscmp(data.cFileName, L"..") == 0) || (wcscmp(data.cFileName, L".") == 0))
                continue;
        }
        else
        {
            std::string fileName = WStr2Str(data.cFileName);
            if(matchSuffixFilters(fileName, suffix_filters))
                list.push_back(fileName);
        }
    }
    while(FindNextFileW(hFind, &data));
    FindClose(hFind);

    return true;
}

bool DirMan::DirMan_private::getListOfFolders(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters)
{
    list.clear();
    HANDLE hFind;
    WIN32_FIND_DATAW data;

    hFind = FindFirstFileW((m_dirPathW + L"/*").c_str(), &data);
    if(hFind == INVALID_HANDLE_VALUE)
        return false;
    do
    {
        if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if((wcscmp(data.cFileName, L"..") == 0) || (wcscmp(data.cFileName, L".") == 0))
                continue;
            std::string fileName = WStr2Str(data.cFileName);
            if(matchSuffixFilters(fileName, suffix_filters))
                list.push_back(fileName);
        }
    }
    while(FindNextFileW(hFind, &data));
    FindClose(hFind);

    return true;
}

bool DirMan::DirMan_private::fetchListFromWalker(std::string &curPath, std::vector<std::string> &list)
{
    if(m_walkerState.digStack.empty())
        return false;

    list.clear();

    std::wstring path = m_walkerState.digStack.top();
    m_walkerState.digStack.pop();

    HANDLE hFind;
    WIN32_FIND_DATAW data;

    hFind = FindFirstFileW((path + L"/*").c_str(), &data);
    if(hFind == INVALID_HANDLE_VALUE)
        return true; //Can't read this directory. Continue
    do
    {
        if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if((wcscmp(data.cFileName, L"..") == 0) || (wcscmp(data.cFileName, L".") == 0))
                continue;

            m_walkerState.digStack.push(path + L"/" + data.cFileName);
        }
        else
        {
            std::string fileNameU = WStr2Str(data.cFileName);
            if(matchSuffixFilters(fileNameU, m_walkerState.suffix_filters))
                list.push_back(fileNameU);
        }
    }
    while(FindNextFileW(hFind, &data));

    FindClose(hFind);
    curPath = WStr2Str(path);

    return true;
}

bool DirMan::exists(const std::string &dirPath)
{
    DWORD ftyp = GetFileAttributesW(Str2WStr(dirPath).c_str());
    if(ftyp == INVALID_FILE_ATTRIBUTES)
        return false;   //something is wrong with your path!
    if(ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;    // this is a directory!
    return false;       // this is not a directory!
}

bool DirMan::mkAbsDir(const std::string &dirPath)
{
    return (CreateDirectoryW(Str2WStr(dirPath).c_str(), NULL) != FALSE);
}

bool DirMan::rmAbsDir(const std::string &dirPath)
{
    return RemoveDirectoryW(Str2WStr(dirPath).c_str()) != FALSE;
}

bool DirMan::mkAbsPath(const std::string &dirPath)
{
    wchar_t tmp[MAX_PATH];
    wchar_t *p = NULL;
    size_t len;
    wcscpy(tmp, Str2WStr(dirPath).c_str());
    len = wcslen(tmp);

    if(tmp[len - 1] == L'/')
        tmp[len - 1] = 0;

    for(p = tmp + 1; *p; p++)
    {
        if(*p == L'/')
        {
            *p = 0;
            CreateDirectoryW(tmp, NULL);
            *p = '/';
        }
    }
    return (CreateDirectoryW(tmp, NULL) != FALSE);
}

bool DirMan::rmAbsPath(const std::string &dirPath)
{
    BOOL ret = TRUE;
    struct DirStackEntry
    {
        std::wstring path;
        HANDLE hFind;
        WIN32_FIND_DATAW data;
    };

    std::stack<DirStackEntry> dirStack;
    DirStackEntry ds;
    ds.hFind = NULL;
    memset(&ds.data, 0, sizeof(WIN32_FIND_DATAW));
    ds.path = Str2WStr(dirPath);
    dirStack.push(ds);

    while(!dirStack.empty())
    {
        DirStackEntry *e = &dirStack.top();
        e->hFind = FindFirstFileW((e->path + L"/*").c_str(), &e->data);
        bool walkUp = false;
        if(e->hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if((wcscmp(e->data.cFileName, L"..") == 0) || (wcscmp(e->data.cFileName, L".") == 0))
                    continue;
                std::wstring path = e->path + L"/" + e->data.cFileName;

                if((e->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                {
                    FindClose(e->hFind);
                    ds.path = path;
                    dirStack.push(ds);
                    walkUp = true;
                    break;
                }
                else
                {
                    if(DeleteFileW(path.c_str()) == FALSE)
                        ret = FALSE;
                }
            }
            while(FindNextFileW(e->hFind, &e->data));
        }

        if(!walkUp)
        {
            if(e->hFind) FindClose(e->hFind);
            if(RemoveDirectoryW(e->path.c_str()) == FALSE)
                ret = FALSE;
            e = NULL;
            dirStack.pop();
        }
    }
    return (ret == TRUE);
}

#endif
