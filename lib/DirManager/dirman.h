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

#ifndef DIRMAN_H
#define DIRMAN_H

#include <string>
#include <stack>
#include <vector>
#include <memory>

class DirMan
{
    class DirMan_private;
    std::unique_ptr<DirMan_private> d;
public:

    explicit DirMan(const std::string &dirPath = "./");
    DirMan(const DirMan &dir);

    virtual ~DirMan();

    /**
     * @brief Change root path
     * @param dirPath absolute or relative to current application path
     */
    void     setPath(const std::string &dirPath);

    /**
     * @brief Get list of files in this directory
     * @param list target list to output
     * @return true if success, false if any error has occouped
     */
    bool     getListOfFiles(std::vector<std::string> &list,
                            const std::vector<std::string> &suffix_filters = std::vector<std::string>());

    /**
     * @brief Get list of directories in this directory
     * @param list target list to output
     * @return true if success, false if any error has occouped
     */
    bool     getListOfFolders(std::vector<std::string> &list,
                              const std::vector<std::string> &suffix_filters = std::vector<std::string>());

    /**
     * @brief Absolude directory path
     * @return string
     */
    std::string absolutePath();

    /**
     * @brief Is this directory exists
     * @return true if current directory is exists
     */
    bool        exists();

    /**
     * @brief Is directory relative to current is exists
     * @return true if directory is exists
     */
    bool        existsRel(const std::string &dirPath);

    /**
     * @brief Check if any directory exists
     * @param dirPath path of directory
     * @return true if directory is exists
     */
    static bool exists(const std::string &dirPath);

    /**
     * @brief Make directory relative to current
     * @param dirPath Relative directory path
     * @return true if directory successfully creaetd
     */
    bool mkdir(const std::string &dirPath = "");

    /**
     * @brief Remove directory (which must be empty) relative to current
     * @param dirPath Relative directory path
     * @return true if directory successfully removed
     */
    bool rmdir(const std::string &dirPath = "");

    /**
     * @brief Make directory with relative to current path with making middle folders which are not exists
     * @param dirPath Relative directory path to the new directory
     * @return true if directories are successfully created
     */
    bool mkpath(const std::string &dirPath = "");

    /**
     * @brief Recursively remove directory and all files inside it
     * @param dirPath Relative path to directory to delete
     * @return true if everything is success, false on any error of deletion (write protection or access denied)
     */
    bool rmpath(const std::string &dirPath = "");

    /**
     * @brief Make directory with absolute path
     * @param dirPath Absolute path to the new directory
     * @return true if directory successfully created
     */
    static bool mkAbsDir(const std::string &dirPath);

    /**
     * @brief Remove directory (which must be empty) with absolute path
     * @param dirPath Absolute path to the directory to remove
     * @return true if directory successfully removed
     */
    static bool rmAbsDir(const std::string &dirPath);

    /**
     * @brief Make directory with absolute path with making middle folders which are not exists
     * @param dirPath Absolute path to the new directory
     * @return true if directories are successfully created
     */
    static bool mkAbsPath(const std::string &dirPath);

    /**
     * @brief Recursively remove directory and all files inside it
     * @param dirPath Absolute path to directory to delete
     * @return true if everything is success, false on any error of deletion (write protection or access denied)
     */
    static bool rmAbsPath(const std::string &dirPath);

    /**
     * @brief Starts directory walking
     * @param suffix_filters list of suffix (filename ends) filters (if not defined, look for all files)
     * @return true if Walker successfully initialized
     */
    bool        beginWalking(const std::vector<std::string> &suffix_filters = std::vector<std::string>());

    /**
     * @brief Fetch list of files of the next directory
     * @param curPath Current directory path
     * @param list List of the files in the current directory
     * @return false when directory walking has been completed
     */
    bool        fetchListFromWalker(std::string &curPath, std::vector<std::string> &list);
};

#endif // DIRMAN_H
