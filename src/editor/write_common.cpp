/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "write_common.h"


void fwritestr(FILE* f, const std::string& s)
{
    char buf[512];
    int dest = 0;
    for (const char& c : s)
    {
        if(dest == 512)
            break;
        if(c == '\t' || c == '\r' || c == '\n' || c == '"')
            continue;
        buf[dest] = c;
        dest ++;
    }
    buf[dest] = '\0';
    fprintf(f, "\"%s\"\r\n", buf);
}

void fwritestr_multiline(FILE* f, const std::string& s)
{
    char buf[512];
    int dest = 0;
    for (const char& c : s)
    {
        if(dest == 512)
            break;
        if(c == '\t' || c == '\r')
            continue;
        if(c == '"')
            buf[dest] = '\'';
        else if(c == '\n')
        {
            if(dest == 511) break;
            buf[dest] = '\r';
            dest ++;
            buf[dest] = '\n';
        }
        else
            buf[dest] = c;
        dest ++;
    }
    buf[dest] = '\0';
    fprintf(f, "\"%s\"\r\n", buf);
}

void fwritenum(FILE* f, int n)
{
    fprintf(f, "%d\r\n", n);
}

void fwritebool(FILE* f, bool b)
{
    if (b)
        fprintf(f, "#TRUE#\r\n");
    else
        fprintf(f, "#FALSE#\r\n");
}

void fwritefloat(FILE* f, float n)
{
    fprintf(f, "%f\r\n", n);
}
