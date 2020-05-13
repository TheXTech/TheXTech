#include "dir_list_ci.h"

#include "../DirManager/dirman.h"
#include "strings.h"
#include <SDL2/SDL_stdinc.h>


DirListCI::DirListCI(const std::string &curDir)
    : m_curDir(curDir)
{
    rescan();
}

void DirListCI::setCurDir(const std::string &path)
{
    m_curDir = path;
    rescan();
}

static void replaceSlashes(std::string &str, const std::string &from)
{
    str.clear();
    if(from.empty())
        return;

    str.reserve(from.size());

    char prevC = '\0';

    for(char c : from)
    {
        if(c == '\\')
            c = '/'; // Replace backslashes
        if(c == '/' && prevC == '/')
            continue; // skip duplicated slashes
        prevC = c;
        str.push_back(c);
    }
}

std::string DirListCI::resolveFileCase(const std::string &in_name)
{
#ifdef _WIN32
    std::string name;
    replaceSlashes(name, in_name);
    return name;
#else
    if(in_name.empty())
        return in_name;

    std::string name;
    replaceSlashes(name, in_name);

    // For sub-directory path, look deeply
    auto subDir = name.find('/');
    if(subDir != std::string::npos)
    {
        auto sdName = resolveDirCase(name.substr(0, subDir));
        DirListCI sd(m_curDir + "/" + sdName);
        return sdName + "/" + sd.resolveFileCase(name.substr(subDir + 1));
    }

    // keep MixerX path arguments untouched
    auto pathArgs = name.find('|');
    if(pathArgs != std::string::npos)
    {
        auto n = name.substr(0, pathArgs);
        for(std::string &c : m_fileList)
        {
            if(SDL_strcasecmp(c.c_str(), n.c_str()) == 0)
                return c + name.substr(pathArgs);
        }
    }
    else
    for(std::string &c : m_fileList)
    {
        if(SDL_strcasecmp(c.c_str(), name.c_str()) == 0)
            return c;
    }

    return name;
#endif
}

std::string DirListCI::resolveDirCase(const std::string &name)
{
#ifdef _WIN32
    return name; // no need on Windows
#else
    if(name.empty())
        return name;

    for(std::string &c : m_dirList)
    {
        if(SDL_strcasecmp(c.c_str(), name.c_str()) == 0)
            return c;
    }

    return name;
#endif
}

void DirListCI::rescan()
{
    m_fileList.clear();
    m_dirList.clear();
    if(m_curDir.empty())
        return;

    DirMan d(m_curDir);
    d.getListOfFiles(m_fileList);
    d.getListOfFolders(m_dirList);
}
