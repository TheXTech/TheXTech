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
    DirListCI(std::string curDir = std::string()) noexcept;
    void setCurDir(const std::string &path);
    const std::string& getCurDir();

    // checks whether a file exists (case-insensitive)
    bool existsCI(const std::string &name);

    // resolves the file's case and returns the original string if failed
    std::string resolveFileCase(const std::string &name);

    // resolves the file's case and returns the original string if failed.
    // returns an absolute path in either case.
    std::string resolveFileCaseAbs(const std::string &name);

    // resolves the file's case and returns an empty string if failed
    std::string resolveFileCaseExists(const std::string &name);

    // resolves the file's case. resturns an absolute path if succeeded, an empty string if failed
    std::string resolveFileCaseExistsAbs(const std::string &name);

    // resolves the dir's case and returns the original string if failed
    std::string resolveDirCase(const std::string &name);

    void rescan();
};

#endif // DIRLISTCI_H
