#ifndef DIRLISTCI_H
#define DIRLISTCI_H

#include <string>
#include <vector>

/**
 * @brief Case-Insensitive directory list
 */
class DirListCI
{
    std::string m_curDir;
    std::vector<std::string> m_fileList;
    std::vector<std::string> m_dirList;
public:
    DirListCI(const std::string &curDir = std::string());
    void setCurDir(const std::string &path);

    std::string resolveFileCase(const std::string &name);
    std::string resolveDirCase(const std::string &name);

    void rescan();
};

#endif // DIRLISTCI_H
