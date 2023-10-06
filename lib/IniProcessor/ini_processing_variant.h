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

/*
 * A QVariant-like thing created just like a proxy between
 * INI Processor and target value (to be compatible with QSettings)
 */

#ifndef INI_PROCESSING_VARIANT_H
#define INI_PROCESSING_VARIANT_H

#include <string>
#ifdef INI_PROCESSING_ALLOW_QT_TYPES
#   include <QString>
#endif

class IniProcessingVariant
{
    std::string  m_data;
    std::string *m_dataP;
    std::string &data();
    const std::string &data() const;

public:
    // Constructors
    IniProcessingVariant();
    IniProcessingVariant(const std::string &data);
    IniProcessingVariant(const char *data);
#ifdef INI_PROCESSING_ALLOW_QT_TYPES
    IniProcessingVariant(const QString &data);
#endif
    IniProcessingVariant(std::string *dataPointer);
    IniProcessingVariant(const IniProcessingVariant &v);

    IniProcessingVariant& operator=(const IniProcessingVariant &o);

    IniProcessingVariant(char data);
    IniProcessingVariant(unsigned char data);
    IniProcessingVariant(bool data);
    IniProcessingVariant(short data);
    IniProcessingVariant(unsigned short data);
    IniProcessingVariant(int data);
    IniProcessingVariant(unsigned int data);
    IniProcessingVariant(long data);
    IniProcessingVariant(unsigned long data);
    IniProcessingVariant(long long data);
    IniProcessingVariant(unsigned long long data);
    IniProcessingVariant(float data);
    IniProcessingVariant(double data);
    IniProcessingVariant(long double data);

    // Validate

    bool isNull() const;
    bool isValid() const;

    // Get data as specified type

    std::string         toString() const;
#ifdef _WIN32
    std::wstring        toWString() const;
#endif
#ifdef INI_PROCESSING_ALLOW_QT_TYPES
    QString             toQString() const;
#endif
    bool                toBool() const;
    int                 toInt() const;
    unsigned int        toUInt() const;
    long                toLong() const;
    unsigned long       toULong() const;
    long long           toLongLong() const;
    unsigned long long  toULongLong() const;
    float               toFloat() const;
    double              toDouble() const;
};


#endif // INI_PROCESSING_VARIANT_H
