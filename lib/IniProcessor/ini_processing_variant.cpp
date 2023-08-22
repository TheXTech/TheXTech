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

#include <cstring>
#include <algorithm>
#include <cctype>
#include <cstdlib>

#ifdef _WIN32
#   include <windows.h>
#   include <stringapiset.h>
#endif

#include "ini_processing_variant.h"


std::string &IniProcessingVariant::data()
{
    if(m_dataP)
        return *m_dataP;
    else
        return m_data;
}

const std::string &IniProcessingVariant::data() const
{
    if(m_dataP)
        return *m_dataP;
    else
        return m_data;
}

IniProcessingVariant::IniProcessingVariant():
    m_data(""),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(const std::string &data):
    m_data(data),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(const char *data):
    m_data(data),
    m_dataP(nullptr)
{}

#ifdef INI_PROCESSING_ALLOW_QT_TYPES
IniProcessingVarian::IniProcessingVariant(const QString &data):
    m_data(data.toStdString()),
    m_dataP(nullptr)
{}
#endif

IniProcessingVariant::IniProcessingVariant(std::string *dataPointer):
    m_data(""),
    m_dataP(dataPointer)
{}

IniProcessingVariant::IniProcessingVariant(const IniProcessingVariant &v):
    m_data(v.m_data),
    m_dataP(v.m_dataP)
{}

IniProcessingVariant& IniProcessingVariant::operator=(const IniProcessingVariant &o)
{
    m_data = o.m_data;
    m_dataP = o.m_dataP;
    return *this;
}

IniProcessingVariant::IniProcessingVariant(char data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(unsigned char data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(bool data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(short data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(unsigned short data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(int data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(unsigned int data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(long data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(unsigned long data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(long long data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(unsigned long long data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(float data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(double data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

IniProcessingVariant::IniProcessingVariant(long double data):
    m_data(std::to_string(data)),
    m_dataP(nullptr)
{}

bool IniProcessingVariant::isNull() const
{
    return (m_data.empty() && !m_dataP);
}

bool IniProcessingVariant::isValid() const
{
    return ((!m_data.empty()) || (static_cast<std::string *>(m_dataP)));
}

std::string IniProcessingVariant::toString() const
{
    std::string out = data();

    if((out.size() > 2) && (out[0] == '"'))
        out.erase(0, 1);

    if((out.size() > 1) && (out[out.size() - 1] == '"'))
        out.erase((out.size() - 1), 1);

    return out;
}

#ifdef _WIN32
std::wstring IniProcessingVariant::toWString() const
{
    std::wstring out16;
    std::string out = data();

    if((out.size() > 2) && (out[0] == '"'))
        out.erase(0, 1);

    if((out.size() > 1) && (out[out.size() - 1] == '"'))
        out.erase((out.size() - 1), 1);

    out16.resize(out.size());
    int newlen = MultiByteToWideChar(CP_UTF8,
                                     0,
                                     out.c_str(),
                                     (int)out.size(),
                                     &out16[0],
                                     (int)out.size());
    out16.resize(newlen);

    return out16;
}
#endif

#ifdef INI_PROCESSING_ALLOW_QT_TYPES
QString IniProcessingVariant::toQString() const
{
    return QString::fromStdString(toString());
}
#endif

bool IniProcessingVariant::toBool() const
{
    size_t i = 0;
    size_t ss = std::min(static_cast<size_t>(4ul), data().size());
    char buff[4] = {0, 0, 0, 0};
    const char *pbufi = data().c_str();
    char *pbuff = buff;

    for(; i < ss; i++)
        (*pbuff++) = static_cast<char>(std::tolower(*pbufi++));

    if(ss < 4)
    {
        if(ss == 0)
            return false;

        if(ss == 1)
            return (buff[0] == '1');

        try
        {
            long num = std::strtol(buff, 0, 0);
            return num != 0l;
        }
        catch(...)
        {
            bool res = (std::memcmp(buff, "yes", 3) == 0) ||
                       (std::memcmp(buff, "on", 2) == 0);
            return res;
        }
    }

    if(std::memcmp(buff, "true", 4) == 0)
        return true;

    try
    {
        long num = std::strtol(buff, 0, 0);
        return num != 0l;
    }
    catch(...)
    {
        return false;
    }
}

int IniProcessingVariant::toInt() const
{
    return std::atoi(data().c_str());
}

unsigned int IniProcessingVariant::toUInt() const
{
    return static_cast<unsigned int>(std::strtoul(data().c_str(), nullptr, 0));
}

long IniProcessingVariant::toLong() const
{
    return std::atol(data().c_str());
}

unsigned long IniProcessingVariant::toULong() const
{
    return std::strtoul(data().c_str(), nullptr, 0);
}

long long IniProcessingVariant::toLongLong() const
{
    return std::atoll(data().c_str());
}

unsigned long long IniProcessingVariant::toULongLong() const
{
    return std::strtoull(data().c_str(), nullptr, 0);
}

float IniProcessingVariant::toFloat() const
{
    return float(std::atof(data().c_str()));
}

double IniProcessingVariant::toDouble() const
{
    return std::atof(data().c_str());
}
