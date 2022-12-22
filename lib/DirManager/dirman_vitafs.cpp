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


#ifdef VITA
#include <sys/dirent.h>

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <sys/fcntl.h>
#include <dirent.h>
#endif

#include <unistd.h>
#include <memory.h>



#include "dirman.h"
#include "dirman_private.h"
#include <mutex>
#include <limits.h>

#include <Logger/logger.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifndef MAX_PATH_LENGTH
#define MAX_PATH_LENGTH PATH_MAX
#endif

static constexpr const SceMode gSceDirMode = 0777;


void DirMan::DirMan_private::setPath(const std::string &dirPath)
{
    PUT_THREAD_GUARD();
#ifdef VITA
    if(dirPath.empty())
    {
        pLogWarning("[dirman_vitafs] WARNING: ::setPath called with dirPath of %s", dirPath.c_str());
        return;
    }
    m_dirPath = dirPath;
    delEnd(m_dirPath, '/');
#else
    (void)dirPath;
#endif
}

static inline int hasEndSlash(char* string)
{
    if(string == NULL) return -1;
    return string[strlen(string) - 1] == '/';
}

bool DirMan::DirMan_private::getListOfFiles(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters)
{
    PUT_THREAD_GUARD();
    list.clear();

    // open directory fd
    SceUID dfd = sceIoDopen(m_dirPath.c_str());
    if(dfd >= 0)
    {
        int res = 0;
        do
        {
            SceIoDirent dirEntry;
            memset(&dirEntry, 0, sizeof(SceIoDirent));

            res = sceIoDread(dfd, &dirEntry);
            if(res > 0)
            {
                char *new_path = (char*)malloc(strlen(m_dirPath.c_str()) + strlen(dirEntry.d_name) + 2);
                snprintf(new_path, MAX_PATH_LENGTH, "%s%s%s", m_dirPath.c_str(), hasEndSlash((char*)m_dirPath.c_str()) ? "" : "/", dirEntry.d_name);
                // pLogDebug("Discovered new FILE path `%s`", new_path);

                // matching non-directories
                if (!SCE_S_ISDIR(dirEntry.d_stat.st_mode))
                {
                    if(matchSuffixFilters(dirEntry.d_name , suffix_filters))
                            list.push_back(dirEntry.d_name);
                }

                free(new_path);
            }
            /* code */
        } while (res > 0);
        
        sceIoDclose(dfd);
    }
    return true;
}

static inline int quick_stat_folders(
    char* m_dirPath,
    char* path, 
    std::vector<std::string>& list,
    const std::vector<std::string>& suffix_filters)
{
    SceUID dfd = sceIoDopen(path);
    int _added = -1;
    if(dfd >= 0)
    {
        _added = 0;
        int res = 0;
        do
        {
            SceIoDirent dirEntry;
            memset(&dirEntry, 0, sizeof(SceIoDirent));

            res = sceIoDread(dfd, &dirEntry);
            if(res > 0)
            {
                char *new_path = (char*)malloc(strlen(m_dirPath) + strlen(dirEntry.d_name) + 2);
                snprintf(new_path, MAX_PATH_LENGTH, "%s%s%s", m_dirPath, hasEndSlash(m_dirPath) ? "" : "/", dirEntry.d_name);
                // pLogDebug("Discovered new path `%s`", new_path);

                if (SCE_S_ISDIR(dirEntry.d_stat.st_mode))
                {
                    if(matchSuffixFilters(dirEntry.d_name , suffix_filters))
                    {
                        pLogDebug("(quick_stat) Directory match! `%s`", dirEntry.d_name);
                        list.push_back(dirEntry.d_name);
                        _added++;
                    }
                }
            }
        } while (res > 0);
    }
    
    sceIoDclose(dfd);

    return _added;
}

bool DirMan::DirMan_private::getListOfFolders(std::vector<std::string>& list, const std::vector<std::string>& suffix_filters)
{
    PUT_THREAD_GUARD();
    list.clear();
    
    SceUID dfd = sceIoDopen(m_dirPath.c_str());
    if(dfd >= 0)
    {
        int res = 0;
        do
        {
            SceIoDirent dirEntry;
            memset(&dirEntry, 0, sizeof(SceIoDirent));

            res = sceIoDread(dfd, &dirEntry);
            if(res > 0)
            {
                char *new_path = (char*)malloc(strlen(m_dirPath.c_str()) + strlen(dirEntry.d_name) + 2);
                snprintf(new_path, MAX_PATH_LENGTH, "%s%s%s", m_dirPath.c_str(), hasEndSlash((char*)m_dirPath.c_str()) ? "" : "/", dirEntry.d_name);
                // pLogDebug("Discovered new FOLDER path `%s`", new_path);

                if (SCE_S_ISDIR(dirEntry.d_stat.st_mode))
                {
                    if(matchSuffixFilters(dirEntry.d_name , suffix_filters))
                    {
                        list.push_back(dirEntry.d_name);
                    }
                    // // Get Path info 
                    // int ret = quick_stat_folders(m_dirPath.c_str(), dirEntry.d_name, list, suffix_filters);
                    // if (ret <= 0)
                    // {
                    //     free(new_path);
                    //     sceIoDclose(dfd);
                    //     return ret;
                    // }
                }

                free(new_path);
            }
            /* code */
        } while (res > 0);
        
        sceIoDclose(dfd);
    }

    // dirent *dent = NULL;
    // DIR *srcdir = opendir(m_dirPath.c_str());
    // if(srcdir == NULL)
    // {
    //     pLogWarning("Unable to open directory %s", m_dirPath.c_str());
    //     return false;
    // }

    // while((dent = readdir(srcdir)) != NULL)
    // {
    //     struct stat st;
    //     if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
    //         continue;

    //     if(fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0)
    //         continue;

    //     if(S_ISDIR(st.st_mode))
    //     {
    //         if(matchSuffixFilters(dent->d_name, suffix_filters))
    //             list.push_back(dent->d_name);
    //     }
    //     else
    //     {
    //         pLogDebug("%s st.st_mode is not a dir (st.st_mode = %d)", st.st_mode, dent->d_name);
            
    //     }
    // }
    // closedir(srcdir);
    return true;
}

bool DirMan::DirMan_private::fetchListFromWalker(std::string &curPath, std::vector<std::string> &list)
{
    PUT_THREAD_GUARD();
    pLogWarning("[dirman_vitafs] ::fetchListFromWalker called. CurPath: %s", curPath.c_str());


    // if(m_walkerState.digStack.empty())
    //     return false;

    // list.clear();

    // std::string path = m_walkerState.digStack.top();
    // m_walkerState.digStack.pop();

    // dirent *dent = NULL;
    // DIR *srcdir = opendir(path.c_str());
    // if(srcdir == NULL) //Can't read this directory. Continue
    //     return true;

    // while((dent = readdir(srcdir)) != NULL)
    // {
    //     struct stat st;
    //     if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
    //         continue;

    //     if(fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0)
    //         continue;

    //     if(S_ISDIR(st.st_mode))
    //         m_walkerState.digStack.push(path + "/" + dent->d_name);
    //     else if(S_ISREG(st.st_mode))
    //     {
    //         if(matchSuffixFilters(dent->d_name, m_walkerState.suffix_filters))
    //             list.push_back(dent->d_name);
    //     }
    // }
    // closedir(srcdir);
    // curPath = path;

    return true;
}

bool DirMan::exists(const std::string &dirPath)
{
    PUT_THREAD_GUARD();

    SceIoStat _stat;

    if(dirPath == "/")
    {
        pLogWarning("[dirman_vitafs] WARNING: dirPath was / (dirPath: `%s`)", dirPath.c_str());
        return false;
    }
    if(sceIoGetstat(dirPath.c_str(), &_stat) < 0)
    {
        pLogWarning("[dirman_vitafs]  File at path %s doesn't exist.", dirPath.c_str());
        return false;
    }

    return true;



    // DIR *dir = opendir(dirPath.c_str());
    // if(dir)
    // {
    //     closedir(dir);
    //     return true;
    // }
    // else
    //     return false;
}

bool DirMan::mkAbsDir(const std::string &dirPath)
{
    PUT_THREAD_GUARD();
    return (sceIoMkdir(dirPath.c_str(), gSceDirMode) == 0);
}

bool DirMan::rmAbsDir(const std::string &dirPath)
{
    PUT_THREAD_GUARD();
    return (sceIoRmdir(dirPath.c_str()) == 0);
}

bool DirMan::mkAbsPath(const std::string &dirPath)
{
    PUT_THREAD_GUARD();
    pLogWarning("TODO: NEED TO MAKE ABSOLUTE PATH FOR `%s`", dirPath.c_str());
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;
    uint16_t first_slash = 0;

    snprintf(tmp, sizeof(tmp), "%s", dirPath.c_str());
    len = strlen(tmp);

    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    
    for(size_t i = 0; i < len; i++)
    {
        if(tmp[i] == '/')
            first_slash = i;
    }

    for(p = (tmp + first_slash + 1); *p; p++)
    {
        if(*p == '/')
        {
            *p = 0;
            int err = sceIoMkdir(tmp, gSceDirMode);
            if((err != 0))
            {
                pLogDebug("err != 0 when calling sceIoMkdir for mkabspath (%s was the path)", tmp);
                *p = '/';
                return false;
            }
            *p = '/';
        }
    }

    bool rv = sceIoMkdir(tmp, gSceDirMode) == 0;
    pLogDebug("    mkAbsPath: `%s`. Success: %s", tmp, (rv ? "TRUE" : "FALSE"));
    return rv;
}

bool DirMan::rmAbsPath(const std::string &dirPath)
{
    PUT_THREAD_GUARD();
    pLogWarning("TODO: need to remove abs path for %s", dirPath.c_str());
    return -1;


    // int ret = 0;
    // struct DirStackEntry
    // {
    //     std::string path;
    //     DIR         *d;
    //     struct dirent *p;
    // };

    // std::stack<DirStackEntry> dirStack;
    // dirStack.push({dirPath, NULL, NULL});

    // while(!dirStack.empty())
    // {
    //     DirStackEntry *e = &dirStack.top();
    //     e->d = opendir(e->path.c_str());

    //     bool walkUp = false;
    //     if(e->d)
    //     {
    //         while((e->p = readdir(e->d)) != NULL)
    //         {
    //             struct stat st;
    //             if(strcmp(e->p->d_name, ".") == 0 || strcmp(e->p->d_name, "..") == 0)
    //                 continue;

    //             if(fstatat(dirfd(e->d), e->p->d_name, &st, 0) < 0)
    //                 continue;

    //             std::string path = e->path + "/" + e->p->d_name;

    //             if(S_ISDIR(st.st_mode))
    //             {
    //                 closedir(e->d);
    //                 dirStack.push({path, NULL, NULL});
    //                 walkUp = true;
    //                 break;
    //             }
    //             else
    //             {
    //                 if(::unlink(path.c_str()) != 0)
    //                     ret = -1;
    //             }
    //         }
    //     }

    //     if(!walkUp)
    //     {
    //         if(e->d) closedir(e->d);
    //         if(::rmdir(e->path.c_str()) != 0)
    //             ret = -1;
    //         e = NULL;
    //         dirStack.pop();
    //     }
    // }

    // return (ret == 0);
}


extern "C" {
    /*
    int mkdir(const char *path, mode_t mode)
    {
        return sceIoMkdir(path, mode);
    }

    int rmdir(const char *__path)
    {
        return sceIoRmdir(__path);
    }
    */

    int dup2(int oldfd, int newfd)
    {
        (void)oldfd;
        (void)newfd;
        return oldfd;
    }

    int pipe(int pipefd[2])
    {
        (void)pipefd;
        return 0;
    }

    long sysconf(int name)
    {
        (void)name;
        return 0;
    }
}
