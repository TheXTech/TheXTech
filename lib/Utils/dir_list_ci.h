#ifndef DIRLISTCI_H
#define DIRLISTCI_H

#include <string>
#include <unordered_map>

/**
 * @brief Case-Insensitive directory list
 */
class DirListCI
{
    std::string m_curDir;
    std::unordered_map<std::string, std::string> m_fileMap;
    std::unordered_map<std::string, std::string> m_dirMap;
public:
    DirListCI(const std::string &curDir = std::string());
    void setCurDir(const std::string &path);

    std::string resolveFileCase(const std::string &name);
    std::string resolveFileCaseExists(const std::string &name);
    std::string resolveDirCase(const std::string &name);

    void rescan();
};

#endif // DIRLISTCI_H
