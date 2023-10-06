#include "../DirManager/dirman.h"
#include "strings.h"
#include "dir_list_ci.h"

#include <locale>
#include <cstring>
#include <algorithm>
#include <utility>


static bool matchSuffixFilters(const std::string &name, const std::vector<std::string> &suffixFilters)
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


DirListCI::DirListCI(std::string curDir) noexcept
    : m_curDir(std::move(curDir))
{
    if(!m_curDir.empty() && m_curDir.back() != '/')
        m_curDir.push_back('/');

    rescan();
}

void DirListCI::setCurDir(const std::string &path)
{
    auto nPath = path;

    if(!nPath.empty() && nPath.back() != '/')
        nPath.push_back('/');

    if(nPath != m_curDir)
    {
        m_curDir = std::move(nPath);
        rescan();
    }
}

const std::string& DirListCI::getCurDir()
{
    return m_curDir;
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

bool DirListCI::existsCI(const std::string &in_name)
{
    if(in_name.empty())
        return false;

    std::string name;
    replaceSlashes(name, in_name);

    // For sub-directory path, look deeply
    auto subDir = name.find('/');

    if(subDir != std::string::npos)
    {
        auto sdName = resolveDirCase(name.substr(0, subDir));
        auto file = name.substr(subDir + 1);
        auto sdf = m_subDirs.find(sdName);
        // std::string found;

        if(sdf == m_subDirs.end())
        {
            auto f = m_subDirs.emplace(sdName, DirListCIPtr(new DirListCI(m_curDir + sdName)));
            return f.first->second->existsCI(file);
        }
        else
            return sdf->second->existsCI(file);
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
        return found != m_fileMap.end();
    }
    else
    {
        std::string uppercase_string;
        uppercase_string.resize(name.length());
        std::transform(name.begin(), name.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        auto found = m_fileMap.find(uppercase_string);
        return found != m_fileMap.end();
    }
}

bool DirListCI::dirExistsCI(const std::string& in_name)
{
    if(in_name.empty())
        return false;

    std::string name;
    replaceSlashes(name, in_name);

    // For sub-directory path, look deeply
    auto subDir = name.find('/');

    if(subDir != std::string::npos)
    {
        auto sdName = resolveDirCase(name.substr(0, subDir));
        auto sdir = name.substr(subDir + 1);
        auto sdf = m_subDirs.find(sdName);
        // std::string found;

        if(sdf == m_subDirs.end())
        {
            auto f = m_subDirs.emplace(sdName, DirListCIPtr(new DirListCI(m_curDir + sdName)));
            return f.first->second->dirExistsCI(sdir);
        }
        else
            return sdf->second->dirExistsCI(sdir);
    }

    std::string uppercase_string;
    uppercase_string.resize(name.length());
    std::transform(name.begin(), name.end(), uppercase_string.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    auto found = m_dirMap.find(uppercase_string);
    return found != m_dirMap.end();
}

std::vector<std::string> DirListCI::getFilesList(const std::string& subDir,
                                                 const std::vector<std::string>& suffix_filters)
{
    std::vector<std::string> ret;

    std::string name;
    replaceSlashes(name, subDir);

    if(!subDir.empty())
    {
        std::string sdName, sdir;
        // For sub-directory path, look deeply
        auto subDirI = name.find('/');

        if(subDirI != std::string::npos)
        {
            sdName = resolveDirCase(name.substr(0, subDirI));
            sdir = name.substr(subDirI + 1);
        }
        else
            sdName = name;

        auto sdf = m_subDirs.find(sdName);
        // std::string found;

        if(sdf == m_subDirs.end())
        {
            auto f = m_subDirs.emplace(sdName, DirListCIPtr(new DirListCI(m_curDir + sdName)));
            return f.first->second->getFilesList(sdir);
        }
        else
            return sdf->second->getFilesList(sdir);
    }

    for(auto &f : m_fileMap)
    {
        if(matchSuffixFilters(f.second, suffix_filters))
            ret.push_back(f.second);
    }

    return ret;
}

std::vector<std::string> DirListCI::getFilesList(const std::vector<std::string>& suffix_filters)
{
    return getFilesList(std::string(), suffix_filters);
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
        auto file = name.substr(subDir + 1);
        auto sdf = m_subDirs.find(sdName);
        std::string found;

        if(sdf == m_subDirs.end())
        {
            auto f = m_subDirs.emplace(sdName, DirListCIPtr(new DirListCI(m_curDir + sdName)));
            found = f.first->second->resolveFileCaseExists(file);
        }
        else
            found = sdf->second->resolveFileCaseExists(file);

        if(!sdName.empty() && sdName.back() != '/')
            sdName.push_back('/');

        if(found.empty())
            return std::string();
        else
            return sdName + found;
    }

    // keep MixerX path arguments untouched
    bool hasArgs = true;
    auto fnLen = name.find('|');
    if(fnLen == std::string::npos)
    {
        fnLen = name.size();
        hasArgs = false;
    }

    std::string uppercase_string;
    uppercase_string.resize(fnLen);
    std::transform(name.begin(), name.begin() + fnLen, uppercase_string.begin(),
        [](unsigned char c){ return std::toupper(c); });

    auto found = m_fileMap.find(uppercase_string);
    if(found != m_fileMap.end())
    {
        if(hasArgs)
            return found->second + name.substr(fnLen);
        else
            return found->second;
    }

    return std::string();
}

std::string DirListCI::resolveFileCase(const std::string &in_name)
{
    if(in_name.empty())
        return in_name;

    std::string found = resolveFileCaseExists(in_name);

    // if not found, overwrite with the replace-slash version of the name
    // and return
    if(found.empty())
        replaceSlashes(found, in_name);

    return found;
}

std::string DirListCI::resolveFileCaseAbs(const std::string &in_name)
{
    if(in_name.empty())
        return in_name;

    std::string found = resolveFileCaseExists(in_name);

    // if not found, overwrite with the replace-slash version of the name
    // and return
    if(found.empty())
        replaceSlashes(found, in_name);

    return m_curDir + found;
}

std::string DirListCI::resolveFileCaseExistsAbs(const std::string &in_name)
{
    if(in_name.empty())
        return in_name;

    std::string found = resolveFileCaseExists(in_name);

    // if not found, overwrite with the replace-slash version of the name
    // and return
    if(found.empty())
        return found;

    return m_curDir + found;
}

std::string DirListCI::resolveDirCase(const std::string &name)
{
    if(name.empty())
        return name;

    std::string uppercase_string;
    uppercase_string.resize(name.length());
    std::transform(name.begin(), name.end(), uppercase_string.begin(),
        [](unsigned char c){ return std::toupper(c); });

    auto found = m_dirMap.find(uppercase_string);
    if(found == m_dirMap.end())
        return name;

    return found->second;
}

void DirListCI::rescan()
{
    m_fileMap.clear();
    m_dirMap.clear();
    m_subDirs.clear();

    if(m_curDir.empty())
        return;

    DirMan d(m_curDir);
    std::vector<std::string> fileList;
    std::vector<std::string> dirList;
    d.getListOfFiles(fileList);
    d.getListOfFolders(dirList);

    std::string uppercase_string;

    for(std::string& file : fileList)
    {
        uppercase_string.resize(file.length());
        std::transform(file.begin(), file.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        m_fileMap.emplace(std::make_pair(uppercase_string, file));
    }

    for(std::string& dir : dirList)
    {
        uppercase_string.resize(dir.length());
        std::transform(dir.begin(), dir.end(), uppercase_string.begin(),
            [](unsigned char c){ return std::toupper(c); });
        m_dirMap.emplace(std::make_pair(uppercase_string, dir));
    }
}
