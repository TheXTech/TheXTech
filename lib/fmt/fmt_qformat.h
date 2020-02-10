/*
    A small wrapper from QString's %1....%99 arguments into fmt::format arguments
*/

#ifndef FMT_QFORMAT_H
#define FMT_QFORMAT_H

#include "fmt_format.h"

namespace fmt
{

inline uint32_t getNum(char* &c)
{
    char numbuff[3] = {0, 0, 0};
    size_t len = 0;
    uint32_t num = 0;
    while((len < 2) && isdigit(*c) && *c != '\0')
        numbuff[len++] = *(c++);
    if(len == 1)
        num = uint32_t(numbuff[0] - '0');
    else if(len == 2)
        num = uint32_t(numbuff[0] - '0') + uint32_t(numbuff[1] - '0') * 10;
    return num;
}

template <typename... Args>
std::string qformat(CStringRef format_str, const Args & ... args)
{
    std::string fmt(format_str.c_str());

    for(size_t i = fmt.size() - 1; true; i--)
    {
        if(fmt[i] == '%')
        {
            char* c = &fmt[i + 1];
            uint32_t num = getNum(c);
            if(c == &fmt[i + 1])
                continue;
            if(num == 0)
                continue;
            num--;
            fmt.erase(i, size_t(c - &fmt[i - 1]));
            fmt.insert(i, "{" + std::to_string(num) + "}");
        }
        if(i == 0)
            break;
    }

    std::string out;
    try
    {
        out = format(fmt, std::forward<const Args&>(args)...);
    }
    catch(const FormatError &e)
    {
        out.append(e.what());
        out.append(" [");
        out.append(fmt);
        out.push_back(']');
    }
    return out;
}

}
#endif // FMT_QFORMAT_H
