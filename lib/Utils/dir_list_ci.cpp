#include "dir_list_ci.h"

#include "../DirManager/dirman.h"
#include "strings.h"
#include <cstring>
#include <algorithm>
#include <utility>
// #include <SDL2/SDL_stdinc.h>


DirListCI::DirListCI(const std::string &curDir)
    : m_curDir(curDir)
{
    rescan();
}

void DirListCI::setCurDir(const std::string &path)
{
    if (m_curDir != path)
    {
        m_curDir = path;
        rescan();
    }
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

std::string DirListCI::resolveFileCaseExists(const std::string &in_name)
{
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
        std::string found = sd.resolveFileCase(name.substr(subDir + 1));
        if (found.empty())
            return "";
        else
            return sdName + "/" + found;
    }

    // keep MixerX path arguments untouched
    auto pathArgs = name.find('|');
    if(pathArgs != std::string::npos)
    {
        auto n = name.substr(0, pathArgs);
        std::string uppercase_string;
        uppercase_string.resize(n.length());
        std::transform(n.begin(), n.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        auto found = m_fileMap.find(uppercase_string);
        if (found != m_fileMap.end())
            return found->second + name.substr(pathArgs);
    }
    else
    {
        std::string uppercase_string;
        uppercase_string.resize(name.length());
        std::transform(name.begin(), name.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        auto found = m_fileMap.find(uppercase_string);
        if (found != m_fileMap.end())
            return found->second;
    }

    return "";
}

std::string DirListCI::resolveFileCase(const std::string &in_name)
{
#ifdef _WIN32
    return in_name; // no need on Windows
#else
    if(in_name.empty())
        return in_name;

    std::string found = resolveFileCaseExists(in_name);

    // if not found, overwrite with the replace-slash version of the name
    // and return
    if (found.empty())
        replaceSlashes(found, in_name);

    return found;
#endif
}

std::string DirListCI::resolveDirCase(const std::string &name)
{
#ifdef _WIN32
    return name; // no need on Windows
#else
    if(name.empty())
        return name;

    std::string uppercase_string;
    uppercase_string.resize(name.length());
    std::transform(name.begin(), name.end(), uppercase_string.begin(),
        [](unsigned char c){ return std::toupper(c); });

    auto found = m_dirMap.find(uppercase_string);
    if (found == m_dirMap.end())
        return name;

    return found->second;
#endif
}

void DirListCI::rescan()
{
    m_fileMap.clear();
    m_dirMap.clear();
    if(m_curDir.empty())
        return;

    DirMan d(m_curDir);
    std::vector<std::string> fileList;
    std::vector<std::string> dirList;
    d.getListOfFiles(fileList);
    d.getListOfFolders(dirList);

    std::string uppercase_string;

    for (std::string& file : fileList)
    {
        uppercase_string.resize(file.length());
        std::transform(file.begin(), file.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        m_fileMap.insert(std::make_pair(uppercase_string, file));
    }
    for (std::string& dir : dirList)
    {
        uppercase_string.resize(dir.length());
        std::transform(dir.begin(), dir.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        m_dirMap.insert(std::make_pair(uppercase_string, dir));
    }
}
