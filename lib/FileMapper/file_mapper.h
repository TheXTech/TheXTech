/*
 * FileMapper - a small C++ class which providing a cross-platform read-only file mapping interface
 * Copyright (c) 2017-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef FILEMAPPER_H
#define FILEMAPPER_H

#include <string>   //std::string
#include <memory>   //std::unique_ptr
#include <cstdint>  //uint64_t

/*!
 * \brief Provides cross-platform file memory mapping interface
 */
class FileMapper
{
    class FileMapper_private;
    std::unique_ptr<FileMapper_private> d;

public:
    //! Pointer to mapped file data
    void*       data() const;

    //! Size of mapped file
    uint64_t    size() const;

    /*!
     * \brief Constructor
     */
    FileMapper();

    /*!
     * \brief Constructor with pre-opened file
     */
    FileMapper(const std::string& file);

    /*!
     * \brief Copy Constructor
     */
    FileMapper(const FileMapper &fm);

    /*!
     * \brief Destructor
     */
    virtual ~FileMapper();

    /*!
     * \brief Opens file
     * \param path to file
     * \return True if success. False if error occouped
     */
    bool open_file(const std::string& path);

    /*!
     * \brief Closes opened file
     * \return True if no errors
     */
    bool close_file();

    /*!
     * \brief Returns recent occouped error info
     * \return recent occouped error info
     */
    std::string error() const;
};

#endif // FILEMAPPER_H
