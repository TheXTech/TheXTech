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

#include "file_mapper.h"
#if defined(__unix__) || defined(__APPLE__) || defined(__HAIKU__) || defined(__ANDROID__)
#   define FileMapper_POSIX
#   if defined(__ANDROID__)
#       define FileMapper_AndroidExtras
#   endif
#elif _WIN32
#   define FileMapper_Windows
#else
#   error Unsupported operating system!
#endif

#include <stdio.h>
#ifdef FileMapper_POSIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif
#ifdef FileMapper_Windows
#include <windows.h>
#endif
#ifdef FileMapper_AndroidExtras
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_assert.h>
#include <vector>
#endif

class FileMapper::FileMapper_private
{
    friend class FileMapper;
#ifdef FileMapper_Windows
    void    *m_File;
    void    *m_Map;
#endif
#ifdef FileMapper_AndroidExtras
    std::vector<uint8_t> m_dump;
#endif
    void    *m_Address;
    off_t   m_size;
    //! Full path to opened file
    std::string m_path;
    //! Recent occouped error description
    std::string m_error;

    bool openFile(const std::string &path);
    bool closeFile();

public:
    FileMapper_private() :
#ifdef FileMapper_Windows
        m_File(NULL),
        m_Map(NULL),
#endif
        m_Address(NULL),
        m_size(0)
    {}
    FileMapper_private(const FileMapper_private &) = default;
    ~FileMapper_private()
    {
        if(m_Address)
            closeFile();
    }
#ifdef FileMapper_AndroidExtras
    bool dumpFile(const std::string &path);
#endif
};

#ifdef FileMapper_AndroidExtras
bool FileMapper::FileMapper_private::dumpFile(const std::string &path)
{
    size_t size;
    SDL_RWops *op = SDL_RWFromFile(path.c_str(), "rb");

    if(op)
    {
        size = static_cast<size_t>(SDL_RWsize(op));
        m_dump.resize(size);
        SDL_RWseek(op, 0, RW_SEEK_SET);
        SDL_RWread(op, m_dump.data(), 1, size);
        SDL_RWclose(op);
        return true;
    }

    return false;
}
#endif

/*
 *   Implementation for POSIX-compatible operating system
 */
#ifdef FileMapper_POSIX
bool FileMapper::FileMapper_private::openFile(const std::string &path)
{
    m_error.clear();
    struct  stat sb;
    int     m_fd;
    m_fd = open(path.c_str(), O_RDONLY);

    if(m_fd == -1)
    {
#ifdef FileMapper_AndroidExtras
        if(dumpFile(path)) // Attempt to load the file from assets via SDL_RWops
        {
            m_Address = m_dump.data();
            m_size = m_dump.size();
            m_path = path;
            return true;
        }
#endif
        return false;
    }

    if(fstat(m_fd, &sb) == -1)
    {
        close(m_fd);
        m_error = "Failed to take state of file " + path;
        return false;
    }

    if(!S_ISREG(sb.st_mode))
    {
        close(m_fd);
        m_error = path + " is not a file";
        return false;
    }

    m_Address = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, m_fd, 0);

    if(m_Address == MAP_FAILED)
    {
        close(m_fd);
        m_error = "Failed to map file " + path;
        return false;
    }

    if(close(m_fd) == -1)
    {
        close(m_fd);
        m_error = "fd is not closed validely " + path;
        return false;
    }

    m_size  = sb.st_size;
    m_path  = path;

    return true;
}

bool FileMapper::FileMapper_private::closeFile()
{
    bool ret = true;
    if(m_Address)
    {
#ifdef FileMapper_AndroidExtras
        if(!m_dump.empty())
        {
            m_dump.clear();
        }
        else
#endif
        if(munmap(m_Address, m_size) == -1)
        {
            m_error = "Fail to unmap";
            ret = false;
        }
    }
    m_path.clear();
    m_Address   = NULL;
    m_size      = 0;
    return ret;
}
#endif

/*
 *   Implementation for Microsoft Windows OS
 */
#ifdef FileMapper_Windows
bool FileMapper::FileMapper_private::openFile(const std::string &path)
{
    m_error.clear();
    std::wstring wpath;
    wpath.resize(path.size());
    int newlen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), static_cast<int>(path.length()), &wpath[0], static_cast<int>(path.length()));
    wpath.resize(newlen);
    m_File = CreateFileW(wpath.c_str(), GENERIC_READ, 1, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(m_File == INVALID_HANDLE_VALUE)
    {
        m_error = "Failed to open file " + path;
        return false;
    }

    m_size = static_cast<off_t>(GetFileSize(m_File, NULL));

    m_Map = CreateFileMappingW(m_File, NULL, PAGE_READONLY, 0, 0, NULL);
    if(m_Map == NULL)
    {
        CloseHandle(m_File);
        m_error = "Failed to map file " + path;
        return false;
    }

    m_Address = MapViewOfFile(m_Map, FILE_MAP_READ, 0, 0, static_cast<long>(m_size));
    if(m_Address == NULL)
    {
        CloseHandle(m_Map);
        CloseHandle(m_File);
        m_error = "Failed to take map address for a file " + path;
        return false;
    }
    m_path  = path;
    return  true;
}

bool FileMapper::FileMapper_private::closeFile()
{
    if(m_Address != NULL)
    {
        try
        {
            UnmapViewOfFile(m_Address);
        }
        catch(void * /*e*/) {}
        m_Address = NULL;
    }

    if(m_Map != NULL)
    {
        try
        {
            CloseHandle(m_Map);
        }
        catch(void * /*e*/) {}
        m_Map = NULL;
    }

    if(m_File != INVALID_HANDLE_VALUE)
    {
        try
        {
            CloseHandle(m_File);
        }
        catch(void * /*e*/) {}
        m_File = NULL;
    }

    m_path.clear();
    return true;
}
#endif

void *FileMapper::data() const
{
    return d->m_Address;
}

uint64_t FileMapper::size() const
{
    return static_cast<uint64_t>(d->m_size);
}

FileMapper::FileMapper() :
    d(new FileMapper_private)
{}

FileMapper::FileMapper(const std::string& file) :
    d(new FileMapper_private)
{
    if(!open_file(file))
        fprintf(stderr, "FileMapper: Failed to open %s file!\n", file.c_str());
}

FileMapper::FileMapper(const FileMapper &fm) :
    d(new FileMapper_private)
{
    if(fm.d->m_Address && !open_file(fm.d->m_path))
        d->m_error = fm.d->m_error;
}

FileMapper::~FileMapper()
{
    close_file();
}

bool FileMapper::open_file(const std::string& path)
{
    return d->openFile(path);
}

bool FileMapper::close_file()
{
    return d->closeFile();
}

std::string FileMapper::error() const
{
    return d->m_error;
}
