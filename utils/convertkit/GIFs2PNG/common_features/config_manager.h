#pragma once
#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <vector>
#include <set>

extern std::string g_ApplicationPath;

class ConfigPackMiniManager
{
public:
    typedef std::vector<std::string> StringList;
    ConfigPackMiniManager();
    ~ConfigPackMiniManager() {}

    /**
     * @brief Initialize config pack's directory tree
     * @param config_dir path to config pack's root directory
     */
    void setConfigDir(const std::string &config_dir);
    /**
     * @brief Is config pack initialized and in use?
     * @return true if config pack is initialized
     */
    bool isUsing();

    /**
     * @brief Add a data directory with all it's sub-directories with depth 1
     * @param dir Path to data directory to add
     */
    void addIntoDirList(std::string dir);

    /**
     * @brief Add a single data directory
     * @param dir Path to data directory to add
     */
    void appendDir(std::string dir);
    /**
     * @brief Append all sub-directories in this directory
     * @param dir Path to data directory to add it's sub-directories
     */
    void appendDirList(const std::string &dir);

    /**
     * @brief Find file in added data directories
     * @param [__in] file file to find
     * @param [__in] customPath custom directory path
     * @param [__out] isReadonly Is file a part of config pack (true) or is a custom file which located in same folder as custom Path (false)
     * @return absolute path to found file. Empty string if no files are found.
     */
    std::string getFile(const std::string &file, std::string customPath = "", bool *isReadonly = nullptr);

private:
    bool                    m_is_using;
    std::string             m_cp_root_path;
    std::string             m_custom_path;
    //! Unique checking list
    std::set<std::string>    m_dir_listUQ;
    //! Ordered list of folderd by priority sequence
    std::vector<std::string> m_dir_list;
};

#endif // CONFIGMANAGER_H
