/*
 * A small crossplatform set of file manipulation functions.
 * All input/output strings are UTF-8 encoded, even on Windows!
 *
 * Copyright (c) 2017-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef FILES_H
#define FILES_H

#include <string>

struct SDL_RWops;

#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#   define FILES_NODISCARD_ATTR [[nodiscard]]
#else
#   define FILES_NODISCARD_ATTR // Nothing!
#endif

namespace Files
{
    // Points to memory representing a loaded file. Does not need to own the memory.
    struct Data
    {
private:
        const unsigned char* m_data = nullptr;
        long long int m_length = -1;
        bool m_free_me = false;
public:
        Data() = default;
        Data(const Data&) = delete;
        Data(Data&&);
        ~Data();

        const Data& operator=(const Data&) = delete;
        const Data& operator=(Data&&);

        void init_from_mem(const unsigned char* data, size_t size);

        inline void take_ownership_of_mem(const unsigned char* data, size_t size)
        {
            init_from_mem(data, size);
            m_free_me = true;
        }

        // if the buffer is malloc-allocated, disowns it and allows the client to take management of it
        FILES_NODISCARD_ATTR void* disown();

        inline bool valid() const
        {
            return m_length >= 0;
        }

        inline const unsigned char* begin() const
        {
            return m_data;
        }

        inline const unsigned char* end() const
        {
            return m_data + m_length;
        }

        inline const char* c_str() const
        {
            return reinterpret_cast<const char*>(m_data);
        }

        inline size_t size() const
        {
            return (m_length >= 0) ? (size_t)m_length : 0;
        }

        inline size_t empty() const
        {
            return m_length <= 0;
        }

        friend Data load_file(const char *filePath);
    };

    FILE *utf8_fopen(const char *filePath, const char *modes);
    SDL_RWops *open_file(const char *filePath, const char *modes);
    Data load_file(const char *filePath);

    inline SDL_RWops *open_file(const std::string& filePath, const char *modes)
    {
        return open_file(filePath.c_str(), modes);
    }
    inline Data load_file(const std::string& filePath)
    {
        return load_file(filePath.c_str());
    }

    void flush_file(SDL_RWops *f);

    enum Charsets
    {
        CHARSET_UTF8 = 0,
        CHARSET_UTF16BE,
        CHARSET_UTF16LE,
        CHARSET_UTF32BE,
        CHARSET_UTF32LE
    };
    int skipBom(SDL_RWops *file, const char **charset = nullptr);

    bool fileExists(const std::string &path);
    bool deleteFile(const std::string &path);
    bool copyFile(const std::string &to, const std::string &from, bool override = false);
    bool moveFile(const std::string &to, const std::string &from, bool override = false);
    bool isAbsolute(const std::string &path);
    std::string basename(std::string path);
    std::string basenameNoSuffix(std::string path);
    std::string dirname(std::string path);
    std::string changeSuffix(std::string path, const std::string& suffix);
    bool hasSuffix(const std::string &path, const std::string &suffix);
    //Appends "m" into basename of the file name before last dot
    void getGifMask(std::string &mask, const std::string &front);
}

#endif // FILES_H
