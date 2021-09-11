#include "config_manager.h"

#include <IniProcessor/ini_processing.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <algorithm>

std::string g_ApplicationPath = "./";

static void addSlashToTail(std::string &str)
{
    if(str.empty())
        return;

    if((str.back() != '/') && (str.back() != '\\'))
        str.push_back('/');
}

static void removeDoubleSlash(std::string &dir)
{
    std::string dirN;
    dirN.reserve(dir.size());
    char c_prev = 0;
    for(char c : dir)
    {
        if(c != c_prev)
            dirN.push_back(c);
        c_prev = (c == '/') ? c : 0;
    }
    dir = dirN;
}

ConfigPackMiniManager::ConfigPackMiniManager() :
    m_is_using(false)
{}

void ConfigPackMiniManager::setConfigDir(const std::string &config_dir)
{
    if(config_dir.empty())
        return;

    DirMan confDir(config_dir);

    if(!confDir.exists())
        return;

    if(!Files::fileExists(confDir.absolutePath() + "/main.ini"))
        return;

    m_cp_root_path = confDir.absolutePath() + "/";

    std::string main_ini = m_cp_root_path + "main.ini";
    IniProcessing mainset(main_ini);

    std::string customAppPath = g_ApplicationPath;
    customAppPath.push_back('/');

    m_dir_list.clear();
    m_dir_listUQ.insert(m_cp_root_path);

    mainset.beginGroup("main");
    {
        customAppPath = mainset.value("application-path", customAppPath).toString();
        std::replace(customAppPath.begin(), customAppPath.end(), '\\', '/');

        m_cp_root_path = (mainset.value("application-dir", false).toBool() ?
                              customAppPath + "/" : m_cp_root_path + "/data/" );

        std::string tmpPath;

        mainset.read("graphics-level", tmpPath, "data/graphics/level");
        addIntoDirList(m_cp_root_path + tmpPath);

        mainset.read("graphics-worldmap", tmpPath, "data/graphics/worldmap");
        addIntoDirList(m_cp_root_path + tmpPath);

        mainset.read("graphics-characters", tmpPath, "data/graphics/characters");
        addIntoDirList(m_cp_root_path + tmpPath);

        mainset.read("custom-data", tmpPath, "data-custom");
        addIntoDirList(m_cp_root_path + tmpPath);
    }
    mainset.endGroup();

    m_is_using = true;
}

bool ConfigPackMiniManager::isUsing()
{
    return m_is_using;
}

void ConfigPackMiniManager::addIntoDirList(std::string dir)
{
    appendDir(dir);
    appendDirList(dir);
}

void ConfigPackMiniManager::appendDir(std::string dir)
{
    std::replace(dir.begin(), dir.end(), '\\', '/');
    removeDoubleSlash(dir);
    addSlashToTail(dir);
    if(m_dir_listUQ.find(dir) == m_dir_listUQ.end())
    {
        m_dir_list.push_back(dir);
        m_dir_listUQ.insert(dir);
    }

}

void ConfigPackMiniManager::appendDirList(const std::string& dir)
{
    DirMan dirs(dir);
    std::vector<std::string> folders;
    if(dirs.getListOfFolders(folders))
    {
        for(std::string &f : folders)
        {
            std::string newpath = dirs.absolutePath() + "/" + f;
            appendDir(newpath);
        }
    }
}

std::string ConfigPackMiniManager::getFile(const std::string &file, std::string customPath, bool *isReadonly)
{
    if(!customPath.empty())
        addSlashToTail(customPath);

    if(!m_is_using)
    {
        if(isReadonly)
            *isReadonly = false;
        return customPath + file;
    }

    if(!customPath.empty() && Files::fileExists(customPath + file))
    {
        if(isReadonly)
            *isReadonly = false;
        return customPath + file;
    }

    if(isReadonly)
        *isReadonly = true; //File is not custom, must not be removed!

    for(std::vector<std::string>::reverse_iterator it = m_dir_list.rbegin();
        it != m_dir_list.rend();
        it++)
    {
        std::string path = *it;
        addSlashToTail(path);
        if(Files::fileExists(path + file))
            return path + file;
    }
    return customPath + file;
}
