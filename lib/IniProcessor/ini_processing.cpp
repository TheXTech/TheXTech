/*
 * INI Processor - a small library which allows you parsing INI-files
 *
 * Copyright (c) 2015-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

//#define USE_FILE_MAPPER

/* Stop parsing on first error (default is to keep parsing). */
//#define INI_STOP_ON_FIRST_ERROR

#ifdef _MSC_VER
#pragma warning (disable : 4127)
#pragma warning (disable : 4244)
#endif

#include "ini_processing.h"
#include <cstdio>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <clocale>
#include <sstream>
#include <algorithm>
#include <assert.h>
#ifdef _WIN32
#   ifdef _MSC_VER
#       ifdef _WIN64
        typedef __int64 ssize_t;
#       else
            typedef __int32 ssize_t;
#       endif
#       ifndef NOMINMAX
#           define NOMINMAX //Don't override std::min and std::max
#       endif
#   endif
#   include <windows.h>
#endif

#ifdef USE_FILE_MAPPER
/*****Replace this with right path to file mapper class*****/
#include "../fileMapper/file_mapper.h"
#endif

static const unsigned char utfbom[3] = {0xEF, 0xBB, 0xBF};

enum { Space = 0x01, Special = 0x02, INIParamEq = 0x04 };

static const unsigned char charTraits[256] =
{
    // Space: '\t', '\n', '\r', ' '
    // Special: '\n', '\r', '"', ';', '=', '\\'
    // INIParamEq: ':', '='

    0, 0, 0, 0, 0, 0, 0, 0, 0, Space, Space | Special, 0, 0, Space | Special,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Space, 0, Special,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, INIParamEq,
    Special, 0, Special | INIParamEq, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Special, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#if 0//for speed comparison who faster - macro or inline function. Seems speeds are same
#define IS_SPACE(c) (charTraits[static_cast<unsigned char>(c)] & Space)
#define IS_SPECIAL(c) (charTraits[static_cast<unsigned char>(c)] & Special)
#define IS_INIEQUAL(c) (charTraits[static_cast<unsigned char>(c)] & INIParamEq)
#else
inline unsigned char IS_SPACE(char &c)
{
    return (charTraits[static_cast<unsigned char>(c)] & Space);
}
inline unsigned char IS_SPECIAL(char &c)
{
    return (charTraits[static_cast<unsigned char>(c)] & Special);
}
inline unsigned char IS_INIEQUAL(char &c)
{
    return (charTraits[static_cast<unsigned char>(c)] & INIParamEq);
}
#endif

/* Strip whitespace chars off end of given string, in place. Return s. */
inline char *rstrip(char *s)
{
    char *p = s + strlen(s);

    while(p > s && IS_SPACE(*--p))
        *p = '\0';

    return s;
}

/* Return pointer to first non-whitespace char in given string. */
inline char *lskip(char *s)
{
    while(*s && IS_SPACE(*s))
        s++;

    return reinterpret_cast<char *>(s);
}

inline char *lrtrim(char *s)
{
    while(*s && IS_SPACE(*s))
        s++;

    char *p = s + strlen(s);

    while(p > s && IS_SPACE(*--p))
        *p = '\0';

    return s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
inline char *find_char_or_comment(char *s, char c)
{
    unsigned char was_whitespace = 0;

    while(*s && *s != c && !(was_whitespace && *s == ';'))
    {
        was_whitespace = IS_SPACE(*s);
        s++;
    }

    return s;
}

inline char *find_inieq_or_comment(char *s)
{
    unsigned char was_whitespace = 0;

    while(*s && (!IS_INIEQUAL(*s)) && !(was_whitespace && *s == ';'))
    {
        was_whitespace = IS_SPACE(*s);
        s++;
    }

    return s;
}

inline char *removeQuotes(char *begin, char *end)
{
    if((*begin == '\0') || (begin == end))
        return begin;

    if((*begin == '"') && (begin + 1 != end))
        begin++;
    else
        return begin;

    if(*(end - 1) == '"')
        *(end - 1) = '\0';

    return begin;
}

inline char *unescapeString(char* str)
{
    char *src, *dst;
    src = str;
    dst = str;
    while(*src)
    {
        if(*src == '\\')
        {
            src++;
            switch(*src)
            {
            case 'n': *dst = '\n'; break;
            case 'r': *dst = '\r'; break;
            case 't': *dst = '\t'; break;
            default:  *dst = *src; break;
            }
        }
        else
        if(src != dst)
        {
            *dst = *src;
        }
        src++; dst++;
    }
    *dst = '\0';
    return str;
}

//Remove comment line from a tail of value
inline void skipcomment(char *value)
{
    unsigned char quoteDepth = 0;

    while(*value)
    {
        if(quoteDepth > 0)
        {
            if(*value == '\\')
            {
                value++;
                continue;
            }

            if(*value == '"')
                --quoteDepth;
        }
        else if(*value == '"')
            ++quoteDepth;

        if((quoteDepth == 0) && (*value == ';'))
        {
            *value = '\0';
            break;
        }

        value++;
    }
}

inline bool memfgets(char *&line, const char *data, char *&pos, const char *end)
{
    line = pos;

    while(pos != end)
    {
        if(*pos == '\n')
        {
            if((pos > data) && (*(pos - 1) == '\r'))
                *((pos++) - 1) = '\0';//Support CRLF too
            else
                *(pos++) = '\0';

            break;
        }

        ++pos;
    }

    return (pos != line);
    //EOF is a moment when position wasn't changed.
    //If do check "pos != end", will be an inability to read last line.
    //this logic allows detect true EOF when line is really eof
}

/* See documentation in header file. */
std::string IniProcessing::to_string_with_precision_private(const double a_value)
{
    char buf[35] = "";
    std::memset(buf, 0, 35);
    std::snprintf(buf, 34, "%.15g", a_value);
    return buf;
}

bool IniProcessing::parseHelper(char *data, size_t size)
{
    char *section = nullptr;
#if defined(INI_ALLOW_MULTILINE)
    char *prev_name = nullptr;
#endif
    char *start;
    char *end;
    char *name;
    char *value;
    int lineno = 0;
    int error = 0;
    char *line;
    char *pos_end = data + size;
    char *pos_cur = data;
    params::IniKeys *recentKeys = nullptr;

    /* Scan through file line by line */
    //while (fgets(line, INI_MAX_LINE, file) != NULL)
    while(memfgets(line, data, pos_cur, pos_end))
    {
        lineno++;
        start = line;

        if((lineno == 1) && (size >= 3) && (std::memcmp(start, utfbom, 3) == 0))
            start += 3;

        start = lrtrim(start);

        if(!*start)//if empty line - skip it away!
            continue;

        switch(*start)
        {
        case ';':
        case '#':
            //if (*start == ';' || *start == '#') {
            //    /* Per Python ConfigParser, allow '#' comments at start of line */
            //}
            continue;

        case '[':
        {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');

            if(*end == ']')
            {
                *end = '\0';
                section = start + 1;
                //#if defined(INI_ALLOW_MULTILINE)
                //                prev_name = nullptr;
                //#endif
                recentKeys = &m_params.iniData[section];
            }
            else if(!error)
            {
                /* No ']' found on section line */
                m_params.errorCode = ERR_SECTION_SYNTAX;
                error = lineno;
            }
        }
        break;

        default:
        {
            /* Not a comment, must be a name[=:]value pair */
            end = find_inieq_or_comment(start);

            if(IS_INIEQUAL(*end))
            {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');

#ifndef CASE_SENSITIVE_KEYS
                for(char *iter = name; *iter != '\0'; ++iter)
                    *iter = (char)tolower(*iter);
#endif

                if(*end == ';')
                    *end = '\0';

                rstrip(value);
                {
                    char *v = value;
                    skipcomment(v);
                    v = rstrip(v);

                    if(!recentKeys)
                        recentKeys = &m_params.iniData["General"];

#ifdef INIDEBUG
                    printf("-> [%s]; %s = %s\n", section, name, v);
#endif
                    (*recentKeys)[name] = unescapeString( removeQuotes(v, v + strlen(v)) );
                }
            }
            else if(!error)
            {
                /* No '=' or ':' found on name[=:]value line */
                m_params.errorCode = ERR_KEY_SYNTAX;
                error = lineno;
            }

            break;
        }
        }//switch(*start)

#if defined(INI_STOP_ON_FIRST_ERROR)
        if(error)
            break;
#endif
    }

    m_params.lineWithError = error;
    return (error == 0);
}

#ifdef _WIN32
static std::string s_wstr_to_str(const std::wstring &in)
{
    std::string out;
    out.resize(in.size() * 4);
    int newSize = WideCharToMultiByte(CP_UTF8,
                                      0,
                                      &in[0],
                                      (int)in.size(),
                                      &out[0],
                                      (int)out.size(),
                                      0, 0);
    out.resize(newSize);
    return out;
}

static std::wstring s_str_to_wstr(const std::string &in)
{
    std::wstring out;
    out.resize(in.size());
    int newlen = MultiByteToWideChar(CP_UTF8,
                                     0,
                                     in.c_str(),
                                     (int)in.size(),
                                     &out[0],
                                     (int)in.size());
    out.resize(newlen);
    return out;
}
#endif

/* See documentation in header file. */
bool IniProcessing::parseFile(const char *filename)
{
    bool valid = true;
    char *tmp = nullptr;

#ifdef USE_FILE_MAPPER
    //By mystical reasons, reading whole file form fread() is faster than mapper :-P
    PGE_FileMapper file(filename);

    if(!file.data)
    {
        m_params.errorCode = ERR_NOFILE;
        return -1;
    }

    tmp = reinterpret_cast<char *>(malloc(static_cast<size_t>(file.size + 1)));

    if(!tmp)
    {
        m_params.errorCode = ERR_NO_MEMORY;
        return false;
    }

    memcpy(tmp, file.data, static_cast<size_t>(file.size));
    *(tmp + file.size) = '\0';//null terminate last line
    valid = ini_parse_file(tmp, static_cast<size_t>(file.size));
#else
#   ifdef _WIN32
    //Convert UTF8 file path into UTF16 to support non-ASCII paths on Windows
    std::wstring dest = s_str_to_wstr(filename);
    FILE *cFile = _wfopen(dest.c_str(), L"rb");
#   else
    FILE *cFile = fopen(filename, "rb");
#   endif

    if(!cFile)
    {
        m_params.errorCode = ERR_NOFILE;
        return false;
    }

    fseek(cFile, 0, SEEK_END);
    ssize_t size = static_cast<ssize_t>(ftell(cFile));
    if(size < 0)
    {
        m_params.errorCode = ERR_KEY_SYNTAX;
        fclose(cFile);
        return false;
    }

    fseek(cFile, 0, SEEK_SET);
    tmp = reinterpret_cast<char *>(malloc(static_cast<size_t>(size + 2)));
    if(!tmp)
    {
        fclose(cFile);
        m_params.errorCode = ERR_NO_MEMORY;
        return false;
    }

    if(fread(tmp, 1, static_cast<size_t>(size), cFile) != static_cast<size_t>(size))
        valid = false;

    fclose(cFile);

    if(valid)
    {
        *(tmp + size) = '\0'; // null terminate last line
#ifdef _WIN32
        if(size > 2 && static_cast<uint8_t>(tmp[0]) == 0xFF && static_cast<uint8_t>(tmp[1]) == 0xFE) // is UTF16
        {
            *(tmp + size + 1) = '\0';
            // Convert entire stuff into UTF-8
            std::string tmp8 = s_wstr_to_str(std::wstring((wchar_t*)(tmp + 2), (size - 2) / 2));
            std::memcpy(tmp, tmp8.c_str(), tmp8.size());
            size = tmp8.size();
            *(tmp + size) = '\0';//null terminate last line
            m_params.is_utf16 = true;
        }
#endif

        try
        {
            valid = parseHelper(tmp, static_cast<size_t>(size));
        }
        catch(...)
        {
            valid = false;
            m_params.errorCode = ERR_SECTION_SYNTAX;
        }
    }
#endif

    free(tmp);
    return valid;
}

bool IniProcessing::parseMemory(char *mem, size_t size)
{
    bool valid = true;
    char *tmp = nullptr;
    tmp = reinterpret_cast<char *>(malloc(size + 2));

    if(!tmp)
    {
        m_params.errorCode = ERR_NO_MEMORY;
        return false;
    }

    memcpy(tmp, mem, static_cast<size_t>(size));
    *(tmp + size) = '\0';//null terminate last line
#ifdef _WIN32
    if(size > 2 && static_cast<uint8_t>(tmp[0]) == 0xFF && static_cast<uint8_t>(tmp[1]) == 0xFE) // is UTF16
    {
        *(tmp + size + 1) = '\0';
        // Convert entire stuff into UTF-8
        std::string tmp8 = s_wstr_to_str(std::wstring((wchar_t*)(tmp + 2), (size - 2) / 2));
        std::memcpy(tmp, tmp8.c_str(), tmp8.size());
        size = tmp8.size();
        *(tmp + size) = '\0';//null terminate last line
        m_params.is_utf16 = true;
    }
#endif

    valid = parseHelper(tmp, size);
    free(tmp);
    return valid;
}


IniProcessing::params::IniKeys::iterator IniProcessing_readHelper(IniProcessing *self, const char *key, bool &ok)
{
    IniProcessing::params &m_params = self->m_params;

    if(!m_params.opened)
        return IniProcessing::params::IniKeys::iterator();

    if(!m_params.currentGroup)
        return IniProcessing::params::IniKeys::iterator();

#ifndef CASE_SENSITIVE_KEYS
    std::string key1(key);
    for(char *iter = &key1[0]; *iter != '\0'; ++iter)
        *iter = (char)tolower(*iter);
#else
    auto &key1 = key;
#endif

    IniProcessing::params::IniKeys::iterator e = m_params.currentGroup->find(key1);

    if(e != m_params.currentGroup->end())
        ok = true;

    return e;
}



IniProcessing::IniProcessing() :
    m_params
    {
        "", false, -1, ERR_OK, false, params::IniSections(), nullptr, ""
#ifdef _WIN32
        , false
#endif
    }
{}

IniProcessing::IniProcessing(const char *iniFileName, int) :
    m_params
    {
        iniFileName, false, -1, ERR_OK, false, params::IniSections(), nullptr, ""
#ifdef _WIN32
        , false
#endif
    }
{
    open(iniFileName);
}

IniProcessing::IniProcessing(const std::string &iniFileName, int) :
    m_params
    {
        iniFileName, false, -1, ERR_OK, false, params::IniSections(), nullptr, ""
#ifdef _WIN32
        , false
#endif
    }
{
    open(iniFileName);
}

#ifdef _WIN32
IniProcessing::IniProcessing(const std::wstring &iniFileName, int) :
    m_params{"", false, -1, ERR_OK, false, params::IniSections(), nullptr, "", true}
{
    std::string iniFileNameU8 = s_wstr_to_str(iniFileName);
    open(iniFileNameU8);
}
#endif

IniProcessing::IniProcessing(char *memory, size_t size) :
    m_params
    {
        "", false, -1, ERR_OK, false, params::IniSections(), nullptr, ""
#ifdef _WIN32
        , false
#endif
    }
{
    openMem(memory, size);
}

#ifdef INI_PROCESSING_ALLOW_QT_TYPES
IniProcessing::IniProcessing(const QString &iniFileName, int) :
    m_params
    {
        iniFileName.toStdString(), false, -1, ERR_OK, false, params::IniSections(), nullptr, ""
#ifdef _WIN32
        , false
#endif
    }
{
    open(m_params.filePath);
}
#endif

IniProcessing::IniProcessing(const IniProcessing &ip) :
    m_params(ip.m_params)
{}

bool IniProcessing::open(const std::string &iniFileName)
{
    std::string prevLocale;
    char *prevLocaleC = std::setlocale(LC_NUMERIC, "C");
    if(prevLocaleC)
        prevLocale.assign(prevLocaleC);

    if(!iniFileName.empty())
    {
        close();
        m_params.errorCode = ERR_OK;
        m_params.filePath  = iniFileName;
        bool res = parseFile(m_params.filePath.c_str());

#ifdef INIDEBUG
        if(res)
            printf("\n==========WOOHOO!!!==============\n\n");
        else
            printf("\n==========OOOUCH!!!==============\n\n");
#endif

        m_params.opened = res;
        if(!prevLocale.empty())
            std::setlocale(LC_NUMERIC, prevLocale.c_str());
        return res;
    }

    if(!prevLocale.empty())
        std::setlocale(LC_NUMERIC, prevLocale.c_str());
    m_params.errorCode = ERR_NOFILE;
    return false;
}

#ifdef _WIN32
bool IniProcessing::open(const std::wstring &iniFileName)
{
    std::string iniFileName8 = s_wstr_to_str(iniFileName);
    return open(iniFileName8);
}
#endif

bool IniProcessing::openMem(char *memory, size_t size)
{
    std::string prevLocale;
    char *prevLocaleC = std::setlocale(LC_NUMERIC, "C");
    if(prevLocaleC)
        prevLocale.assign(prevLocaleC);

    if((memory != nullptr) && (size > 0))
    {
        close();
        m_params.errorCode = ERR_OK;
        m_params.filePath.clear();
        bool res = parseMemory(memory, size);
        m_params.opened = res;
        if(!prevLocale.empty())
            std::setlocale(LC_NUMERIC, prevLocale.c_str());
        return res;
    }

    if(!prevLocale.empty())
        std::setlocale(LC_NUMERIC, prevLocale.c_str());

    m_params.errorCode = ERR_NOFILE;

    return false;
}

void IniProcessing::close()
{
    m_params.errorCode = ERR_OK;
    m_params.iniData.clear();
    m_params.opened = false;
    m_params.lineWithError = -1;
}

IniProcessing::ErrCode IniProcessing::lastError() const
{
    return m_params.errorCode;
}

int IniProcessing::lineWithError() const
{
    return m_params.lineWithError;
}

bool IniProcessing::isOpened() const
{
    return m_params.opened;
}

bool IniProcessing::beginGroup(const std::string &groupName)
{
    //Keep the group name. If not exist, will be created on value write
    m_params.currentGroupName = groupName;

    auto e = m_params.iniData.find(groupName);

    if(e == m_params.iniData.end())
        return false;

    params::IniKeys &k = e->second;
    m_params.currentGroup = &k;
    return true;
}

bool IniProcessing::contains(const std::string &groupName)
{
    if(!m_params.opened)
        return false;

    auto e = m_params.iniData.find(groupName);
    return (e != m_params.iniData.end());
}

#ifdef _WIN32
bool IniProcessing::beginGroup(const std::wstring &groupName)
{
    std::string groupName8 = s_wstr_to_str(groupName);
    return beginGroup(groupName8);
}

bool IniProcessing::contains(const std::wstring &groupName)
{
    std::string groupName8 = s_wstr_to_str(groupName);
    return contains(groupName8);
}
#endif

std::string IniProcessing::fileName() const
{
    return m_params.filePath;
}

std::string IniProcessing::group() const
{
    return m_params.currentGroupName;
}

#ifdef _WIN32
std::wstring IniProcessing::wgroup() const
{
    return s_str_to_wstr(m_params.currentGroupName);
}
#endif

std::vector<std::string> IniProcessing::childGroups()
{
    std::vector<std::string> groups;
    groups.reserve(m_params.iniData.size());

    for(auto & e : m_params.iniData)
        groups.push_back(e.first);

    return groups;
}

#ifdef _WIN32
std::vector<std::wstring> IniProcessing::wchildGroups()
{
    std::vector<std::wstring> groups;
    groups.reserve(m_params.iniData.size());

    for(auto & e : m_params.iniData)
        groups.push_back(s_str_to_wstr(e.first));

    return groups;
}
#endif

bool IniProcessing::hasKey(const std::string &keyName) const
{
    if(!m_params.opened)
        return false;

    if(!m_params.currentGroup)
        return false;

#ifndef CASE_SENSITIVE_KEYS
    std::string keyName1(keyName);
    for(char *iter = &keyName1[0]; *iter != '\0'; ++iter)
        *iter = (char)tolower(*iter);
#else
    auto &keyName1 = keyName;
#endif

    auto e = m_params.currentGroup->find(keyName1);
    return (e != m_params.currentGroup->end());
}

#ifdef _WIN32
bool IniProcessing::hasKey(const std::wstring &keyName) const
{
    return hasKey(s_wstr_to_str(keyName));
}
#endif

bool IniProcessing::renameKey(const std::string &oldName, const std::string &newName)
{
    if(!m_params.opened)
        return false;

    if(!m_params.currentGroup)
        return false;

    if(oldName.empty() || newName.empty())
        return false; // Empty keys are invalid

#ifndef CASE_SENSITIVE_KEYS
    std::string oldName1(oldName);
    for(char *iter = &oldName1[0]; *iter != '\0'; ++iter)
        *iter = (char)tolower(*iter);

    std::string newName1(newName);
    for(char *iter = &newName1[0]; *iter != '\0'; ++iter)
        *iter = (char)tolower(*iter);
#else
    auto &oldName1 = oldName;
    auto &newName1 = newName;
#endif

    if(oldName1 == newName1)
        return true; /* Do nothing, keys are equal */

    auto it = m_params.currentGroup->find(oldName1);
    if(it != m_params.currentGroup->end())
    {
        std::swap((*m_params.currentGroup)[newName1], it->second);
        m_params.currentGroup->erase(it);
        return true;
    }

    return false;
}

#ifdef _WIN32
bool IniProcessing::renameKey(const std::wstring &oldName, const std::wstring &newName)
{
    return renameKey(s_wstr_to_str(oldName), s_wstr_to_str(newName));
}
#endif

bool IniProcessing::deleteKey(const std::string &keyName)
{
    if(!m_params.opened)
        return false;

    if(!m_params.currentGroup)
        return false;

    if(keyName.empty())
        return false; // Empty keys are invalid

#ifndef CASE_SENSITIVE_KEYS
    std::string keyName1(keyName);
    for(char *iter = &keyName1[0]; *iter != '\0'; ++iter)
        *iter = (char)tolower(*iter);
#else
    auto &keyName1 = keyName;
#endif

    auto it = m_params.currentGroup->find(keyName1);
    if(it != m_params.currentGroup->end())
    {
        m_params.currentGroup->erase(it);
        return true;
    }

    return false;
}

#ifdef _WIN32
bool IniProcessing::deleteKey(const std::wstring &keyName)
{
    return deleteKey(s_wstr_to_str(keyName));
}
#endif

std::vector<std::string> IniProcessing::allKeys() const
{
    std::vector<std::string> keys;
    if(!m_params.opened)
        return keys;
    if(!m_params.currentGroup)
        return keys;

    keys.reserve(m_params.currentGroup->size());

    for(auto &it : *m_params.currentGroup)
        keys.push_back( it.first );

    return keys;
}

#ifdef _WIN32
std::vector<std::wstring> IniProcessing::wallKeys() const
{
    std::vector<std::wstring> keys;
    if(!m_params.opened)
        return keys;
    if(!m_params.currentGroup)
        return keys;

    keys.reserve(m_params.currentGroup->size());

    for(auto &it : *m_params.currentGroup)
        keys.push_back(s_str_to_wstr(it.first));

    return keys;
}
#endif

void IniProcessing::endGroup()
{
    m_params.currentGroup = nullptr;
    m_params.currentGroupName.clear();
}

void IniProcessing::read(const char *key, bool &dest, bool defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    std::string &k = e->second;
    size_t i = 0;
    size_t ss = std::min(static_cast<size_t>(4ul), k.size());
    char buff[4] = {0, 0, 0, 0};
    const char *pbufi = k.c_str();
    char *pbuff = buff;

    for(; i < ss; i++)
        (*pbuff++) = static_cast<char>(std::tolower(*pbufi++));

    if(ss < 4)
    {
        if(ss == 0)
        {
            dest = false;
            return;
        }

        if(ss == 1)
        {
            dest = (buff[0] == '1');
            return;
        }

        bool isNum = true;
        isNum = isNum && (std::isdigit(buff[i]) || (buff[i] == '-') || (buff[i] == '+'));

        for(size_t j = 1; j < ss; j++)
            isNum = (isNum && std::isdigit(buff[j]));

        if(isNum)
        {
            long num = std::strtol(buff, 0, 0);
            dest = num != 0l;
            return;
        }
        else
        {
            dest = (std::memcmp(buff, "yes", 3) == 0) ||
                   (std::memcmp(buff, "on", 2) == 0);
            return;
        }
    }

    if(std::memcmp(buff, "true", 4) == 0)
    {
        dest = true;
        return;
    }

    try
    {
        long num = std::strtol(buff, 0, 0);
        dest = num != 0l;
        return;
    }
    catch(...)
    {
        dest = false;
        return;
    }
}

void IniProcessing::read(const char *key, unsigned char &dest, unsigned char defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    std::string &k = e->second;

    if(k.size() >= 1)
        dest = static_cast<unsigned char>(k[0]);
    else
        dest = defVal;
}

void IniProcessing::read(const char *key, char &dest, char defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    std::string &k = e->second;

    if(k.size() >= 1)
        dest = k[0];
    else
        dest = defVal;
}

void IniProcessing::read(const char *key, unsigned short &dest, unsigned short defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = static_cast<unsigned short>(std::strtoul(e->second.c_str(), nullptr, 0));
}

void IniProcessing::read(const char *key, short &dest, short defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = static_cast<short>(std::strtol(e->second.c_str(), nullptr, 0));
}

void IniProcessing::read(const char *key, unsigned int &dest, unsigned int defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = static_cast<unsigned int>(std::strtoul(e->second.c_str(), nullptr, 0));
}

void IniProcessing::read(const char *key, int &dest, int defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = static_cast<int>(std::strtol(e->second.c_str(), nullptr, 0));
}

void IniProcessing::read(const char *key, unsigned long &dest, unsigned long defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtoul(e->second.c_str(), nullptr, 0);
}

void IniProcessing::read(const char *key, long &dest, long defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtol(e->second.c_str(), nullptr, 0);
}

void IniProcessing::read(const char *key, unsigned long long &dest, unsigned long long defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtoull(e->second.c_str(), nullptr, 0);
}

void IniProcessing::read(const char *key, long long &dest, long long defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtoll(e->second.c_str(), nullptr, 0);
}

void IniProcessing::read(const char *key, float &dest, float defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);
    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtof(e->second.c_str(), nullptr);
}

void IniProcessing::read(const char *key, double &dest, double defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtod(e->second.c_str(), nullptr);
}

void IniProcessing::read(const char *key, long double &dest, long double defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);
    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = std::strtold(e->second.c_str(), nullptr);
}

void IniProcessing::read(const char *key, std::string &dest, const std::string &defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = e->second;
}

#ifdef _WIN32
void IniProcessing::read(const char *key, std::wstring &dest, const std::wstring &defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = s_str_to_wstr(e->second);
}
#endif

#ifdef INI_PROCESSING_ALLOW_QT_TYPES
void IniProcessing::read(const char *key, QString &dest, const QString &defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = QString::fromStdString(e->second);
}
#endif

template<class TList>
inline void StrToNumVectorHelper(const std::string &source, TList &dest, const typename TList::value_type &def)
{
    typedef typename TList::value_type T;
    dest.clear();

    if(!source.empty())
    {
        std::stringstream ss(source);
        std::string item;
        while(std::getline(ss, item, ','))
        {
            auto remove_ret = std::remove(item.begin(), item.end(), ' '); //-V530
            (void)remove_ret;

            try
            {
                if(std::is_same<T, int>::value ||
                   std::is_same<T, long>::value ||
                   std::is_same<T, short>::value)
                    dest.push_back(static_cast<T>(std::strtol(item.c_str(), NULL, 0)));
                else if(std::is_same<T, unsigned int>::value ||
                        std::is_same<T, unsigned long>::value ||
                        std::is_same<T, unsigned short>::value)
                    dest.push_back(static_cast<T>(std::strtoul(item.c_str(), NULL, 0)));
                else if(std::is_same<T, float>::value)
                    dest.push_back(std::strtof(item.c_str(), NULL));
                else
                    dest.push_back(std::strtod(item.c_str(), NULL));
            }
            catch(...)
            {
                dest.pop_back();
            }
        }

        if(dest.empty())
            dest.push_back(def);
    }
    else
        dest.push_back(def);
}

template<class TList, typename T>
void readNumArrHelper(IniProcessing *self, const char *key, TList &dest, const TList &defVal)
{
    bool ok = false;
    IniProcessing::params::IniKeys::iterator e = IniProcessing_readHelper(self, key, ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    StrToNumVectorHelper(e->second, dest, static_cast<T>(0));
}

void IniProcessing::read(const char *key, std::vector<unsigned short> &dest, const std::vector<unsigned short> &defVal)
{
    readNumArrHelper<std::vector<unsigned short>, unsigned short>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<short> &dest, const std::vector<short> &defVal)
{
    readNumArrHelper<std::vector<short>, short>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<unsigned int> &dest, const std::vector<unsigned int> &defVal)
{
    readNumArrHelper<std::vector<unsigned int>, unsigned int>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<int> &dest, const std::vector<int> &defVal)
{
    readNumArrHelper<std::vector<int>, int>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<unsigned long> &dest, const std::vector<unsigned long> &defVal)
{
    readNumArrHelper<std::vector<unsigned long>, unsigned long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<long> &dest, const std::vector<long> &defVal)
{
    readNumArrHelper<std::vector<long>, long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<unsigned long long> &dest, const std::vector<unsigned long long> &defVal)
{
    readNumArrHelper<std::vector<unsigned long long>, unsigned long long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<long long> &dest, const std::vector<long long> &defVal)
{
    readNumArrHelper<std::vector<long long>, long long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<float> &dest, const std::vector<float> &defVal)
{
    readNumArrHelper<std::vector<float>, float>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, std::vector<double> &dest, const std::vector<double> &defVal)
{
    readNumArrHelper<std::vector<double>, double>(this, key, dest, defVal);
}

void IniProcessing::read(const char *key, std::vector<long double> &dest, const std::vector<long double> &defVal)
{
    readNumArrHelper<std::vector<long double>, long double>(this, key, dest, defVal);
}

#ifdef _WIN32
void IniProcessing::read(const wchar_t *key, bool &dest, bool defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, unsigned char &dest, unsigned char defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, char &dest, char defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, unsigned short &dest, unsigned short defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, short &dest, short defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, unsigned int &dest, unsigned int defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, int &dest, int defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, unsigned long &dest, unsigned long defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, long &dest, long defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, unsigned long long &dest, unsigned long long defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, long long &dest, long long defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, float &dest, float defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, double &dest, double defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, long double &dest, long double defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::string &dest, const std::string &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key8.c_str(), ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = e->second;
}

void IniProcessing::read(const wchar_t *key, std::wstring &dest, const std::wstring &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key8.c_str(), ok);

    if(!ok)
    {
        dest = defVal;
        return;
    }

    dest = s_str_to_wstr(e->second);
}

void IniProcessing::read(const wchar_t *key, std::vector<unsigned short> &dest, const std::vector<unsigned short> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<short> &dest, const std::vector<short> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<unsigned int> &dest, const std::vector<unsigned int> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<int> &dest, const std::vector<int> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<unsigned long> &dest, const std::vector<unsigned long> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<long> &dest, const std::vector<long> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<unsigned long long> &dest, const std::vector<unsigned long long> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<long long> &dest, const std::vector<long long> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<float> &dest, const std::vector<float> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<double> &dest, const std::vector<double> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}

void IniProcessing::read(const wchar_t *key, std::vector<long double> &dest, const std::vector<long double> &defVal)
{
    std::string key8 = s_wstr_to_str(key);
    read(key8.c_str(), dest, defVal);
}
#endif // WIN32

#ifdef INI_PROCESSING_ALLOW_QT_TYPES
void IniProcessing::read(const char *key, QList<short> &dest, const QList<short> &defVal)
{
    readNumArrHelper<QList<short>, short>(this, key, dest, defVal);
}

void IniProcessing::read(const char *key, QList<unsigned short> &dest, const QList<unsigned short> &defVal)
{
    readNumArrHelper<QList<unsigned short>, unsigned short>(this, key, dest, defVal);
}

void IniProcessing::read(const char *key, QList<int> &dest, const QList<int> &defVal)
{
    readNumArrHelper<QList<int>, int>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<unsigned int> &dest, const QList<unsigned int> &defVal)
{
    readNumArrHelper<QList<unsigned int>, unsigned int>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<long> &dest, const QList<long> &defVal)
{
    readNumArrHelper<QList<long>, long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<unsigned long> &dest, const QList<unsigned long> &defVal)
{
    readNumArrHelper<QList<unsigned long>, unsigned long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<long long> &dest, const QList<long long> &defVal)
{
    readNumArrHelper<QList<long long>, long long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<unsigned long long> &dest, const QList<unsigned long long> &defVal)
{
    readNumArrHelper<QList<unsigned long long>, unsigned long long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<float> &dest, const QList<float> &defVal)
{
    readNumArrHelper<QList<float>, float>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QList<double> &dest, const QList<double> &defVal)
{
    readNumArrHelper<QList<double>, double>(this, key, dest, defVal);
}

void IniProcessing::read(const char *key, QList<long double> &dest, const QList<long double> &defVal)
{
    readNumArrHelper<QList<long double>, long double>(this, key, dest, defVal);
}

#   if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void IniProcessing::read(const char *key, QVector<short> &dest, const QVector<short> &defVal)
{
    readNumArrHelper<QVector<short>, short>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<unsigned short> &dest, const QVector<unsigned short> &defVal)
{
    readNumArrHelper<QVector<unsigned short>, unsigned short>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<int> &dest, const QVector<int> &defVal)
{
    readNumArrHelper<QVector<int>, int>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<unsigned int> &dest, const QVector<unsigned int> &defVal)
{
    readNumArrHelper<QVector<unsigned int>, unsigned int>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<long> &dest, const QVector<long> &defVal)
{
    readNumArrHelper<QVector<long>, long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<unsigned long> &dest, const QVector<unsigned long> &defVal)
{
    readNumArrHelper<QVector<unsigned long>, unsigned long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<long long> &dest, const QVector<long long> &defVal)
{
    readNumArrHelper<QVector<long long>, long long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<unsigned long long> &dest, const QVector<unsigned long long> &defVal)
{
    readNumArrHelper<QVector<unsigned long long>, unsigned long long>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<float> &dest, const QVector<float> &defVal)
{
    readNumArrHelper<QVector<float>, float>(this, key, dest, defVal);
}
void IniProcessing::read(const char *key, QVector<double> &dest, const QVector<double> &defVal)
{
    readNumArrHelper<QVector<double>, double>(this, key, dest, defVal);
}

void IniProcessing::read(const char *key, QVector<long double> &dest, const QVector<long double> &defVal)
{
    readNumArrHelper<QVector<long double>, long double>(this, key, dest, defVal);
}
#   endif
#endif

IniProcessingVariant IniProcessing::value(const char *key, const IniProcessingVariant &defVal)
{
    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);

    if(!ok)
        return defVal;

    std::string &k = e->second;
    return IniProcessingVariant(&k);
}

void IniProcessing::writeIniParam(const char *key, const std::string &value)
{
    if(m_params.currentGroupName.empty())
        return;

    bool ok = false;
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key, ok);
    if(ok)
    {
        e->second = value;
    }
    else
    {
        if(!m_params.currentGroup)
        {
            m_params.iniData.insert({m_params.currentGroupName, params::IniKeys()});
            m_params.currentGroup = &m_params.iniData[m_params.currentGroupName];
        }
        m_params.currentGroup->insert({std::string(key), value});
        //Mark as opened
        m_params.opened = true;
    }
}

#ifdef _WIN32
void IniProcessing::writeIniParam(const wchar_t *key, const std::string &value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), value);
}
#endif

void IniProcessing::setValue(const char *key, unsigned short value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, short value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, unsigned int value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, int value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, unsigned long value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, long value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, unsigned long long value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, long long value)
{
    writeIniParam(key, std::to_string(value));
}

void IniProcessing::setValue(const char *key, float value)
{
    writeIniParam(key, IniProcessing::to_string_with_precision(value));
}

void IniProcessing::setValue(const char *key, double value)
{
    writeIniParam(key, IniProcessing::to_string_with_precision(value));
}

void IniProcessing::setValue(const char *key, long double value)
{
    writeIniParam(key, IniProcessing::to_string_with_precision(value));
}


#ifdef _WIN32
IniProcessingVariant IniProcessing::value(const wchar_t *key, const IniProcessingVariant &defVal)
{
    bool ok = false;
    std::string key8 = s_wstr_to_str(key);
    params::IniKeys::iterator e = IniProcessing_readHelper(this, key8.c_str(), ok);

    if(!ok)
        return defVal;

    std::string &k = e->second;
    return IniProcessingVariant(&k);
}

void IniProcessing::setValue(const wchar_t *key, unsigned short value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, short value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, unsigned int value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, int value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, unsigned long value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, long value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, unsigned long long value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, long long value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), std::to_string(value));
}

void IniProcessing::setValue(const wchar_t *key, float value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), IniProcessing::to_string_with_precision(value));
}

void IniProcessing::setValue(const wchar_t *key, double value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), IniProcessing::to_string_with_precision(value));
}

void IniProcessing::setValue(const wchar_t *key, long double value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), IniProcessing::to_string_with_precision(value));
}
#endif // _WIN32

void IniProcessing::setValue(const char *key, const char *value)
{
    writeIniParam(key, value);
}

void IniProcessing::setValue(const char *key, const std::string &value)
{
    writeIniParam(key, value);
}

#ifdef _WIN32
void IniProcessing::setValue(const wchar_t *key, const wchar_t *value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), s_wstr_to_str(value));
}

void IniProcessing::setValue(const wchar_t *key, const std::wstring &value)
{
    std::string key8 = s_wstr_to_str(key);
    writeIniParam(key8.c_str(), s_wstr_to_str(value));
}
#endif // _WIN32

#ifdef INI_PROCESSING_ALLOW_QT_TYPES
void IniProcessing::setValue(const char *key, const QString &value)
{
    writeIniParam(key, value.toStdString());
}
#endif

static inline bool isFloatValue(const std::string &str)
{
    enum State
    {
        ST_SIGN = 0,
        ST_DOT,
        ST_EXPONENT,
        ST_EXPONENT_SIGN,
        ST_TAIL
    } st = ST_SIGN;

    for(const char &c : str)
    {
        if(c < 0 || c > 127)
            return false; // Totally not a valid part of a floating-point number

        if(!isdigit(c))
        {
            switch(st)
            {
            case ST_SIGN:
                if(c != '-')
                    return false;
                st = ST_DOT;
                continue;

            case ST_DOT:
                if((c == 'E') || (c == 'e'))
                    st = ST_EXPONENT_SIGN;
                else if(c != '.')
                    return false;
                else
                    st = ST_EXPONENT;
                continue;

            case ST_EXPONENT:
                if((c != 'E') && (c != 'e'))
                    return false;
                st = ST_EXPONENT_SIGN;
                continue;

            case ST_EXPONENT_SIGN:
                if(c != '-')
                    return false;
                st = ST_TAIL;
                continue;

            case ST_TAIL:
                return false;
            }
            return false;
        }
        else
        {
            if(st == ST_SIGN)
                st = ST_DOT;
        }
    }
    return true;
}

bool IniProcessing::writeIniFile()
{
#ifdef _WIN32
    if(m_params.is_utf16) // it's UTF-16 file, save as UTF-16 back
        return writeIniFileU16();

    //Convert UTF8 file path into UTF16 to support non-ASCII paths on Windows
    std::wstring dest = s_str_to_wstr(m_params.filePath);
    FILE *cFile = _wfopen(dest.c_str(), L"wb");
#else
    FILE *cFile = fopen(m_params.filePath.c_str(), "wb");
#endif

    if(!cFile)
        return false;

    // Sort the data before to output
    params::IniSectionsOrdered outData;
    for(auto &group : m_params.iniData)
    {
        auto &outKeys = outData[group.first];
        for(auto &key : group.second)
            outKeys.insert(key);
    }

    for(auto &group : outData)
    {
        fprintf(cFile, "[%s]\n", group.first.c_str());

        for(auto &key : group.second)
        {
            if(isFloatValue(key.second))
            {
                //Store as-is without quoting
                fprintf(cFile, "%s = %s\n", key.first.c_str(), key.second.c_str());
            }
            else
            {
                //Set escape quotes and put the string with a quotes
                std::string &s = key.second;
                std::string escaped;
                escaped.reserve(s.length() * 2);

                for(char &c : s)
                {
                    switch(c)
                    {
                    case '\n': escaped += "\\n"; break;
                    case '\r': escaped += "\\r"; break;
                    case '\t': escaped += "\\t"; break;
                    default:
                        if((c == '\\') || (c == '"'))
                            escaped.push_back('\\');
                        escaped.push_back(c);
                    }
                }

                fprintf(cFile, "%s = \"%s\"\n", key.first.c_str(), escaped.c_str());
            }
        }

        fprintf(cFile, "\n");
        fflush(cFile);
    }

    fclose(cFile);

    return true;
}

#ifdef _WIN32
bool IniProcessing::writeIniFileU16()
{
    //Convert UTF8 file path into UTF16 to support non-ASCII paths on Windows
    std::wstring dest = s_str_to_wstr(m_params.filePath);
    FILE *cFile = _wfopen(dest.c_str(), L"wb");

    if(!cFile)
        return false;

    // Sort the data before to output
    params::IniSectionsOrdered outData;
    for(auto &group : m_params.iniData)
    {
        auto &outKeys = outData[group.first];
        for(auto &key : group.second)
            outKeys.insert(key);
    }

    // Write BOM
    fwrite("\xFF\xFE", 1, 2, cFile);

    for(auto &group : outData)
    {
        std::wstring grouName = s_str_to_wstr(group.first);
        fwprintf(cFile, L"[%ls]\r\n", grouName.c_str());

        for(auto &key : group.second)
        {
            if(isFloatValue(key.second))
            {
                std::wstring wkey = s_str_to_wstr(key.first);
                std::wstring wvalue = s_str_to_wstr(key.second);
                //Store as-is without quoting
                fwprintf(cFile, L"%ls = %ls\r\n", wkey.c_str(), wvalue.c_str());
            }
            else
            {
                //Set escape quotes and put the string with a quotes
                std::string &s = key.second;
                std::string escaped;
                escaped.reserve(s.length() * 2);

                for(char &c : s)
                {
                    switch(c)
                    {
                    case '\n': escaped += "\\n"; break;
                    case '\r': escaped += "\\r"; break;
                    case '\t': escaped += "\\t"; break;
                    default:
                        if((c == '\\') || (c == '"'))
                            escaped.push_back('\\');
                        escaped.push_back(c);
                    }
                }

                std::wstring wkey = s_str_to_wstr(key.first);
                std::wstring wvalue = s_str_to_wstr(escaped);

                fwprintf(cFile, L"%ls = \"%ls\"\r\n", wkey.c_str(), wvalue.c_str());
            }
        }

        fwprintf(cFile, L"\r\n");
        fflush(cFile);
    }

    fclose(cFile);

    return true;
}
#endif
