/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "lunacounter_record.h"
#include <Logger/logger.h>

#include "core/std.h"

void DeathRecord::Save(FILE *openfile)
{
    // Write character count
    auto tempint = (uint32_t)m_levelName.size();

    tempint = XStd::SwapLE32(tempint);
    std::fwrite(&tempint, 1, sizeof(uint32_t), openfile);

    // Write string data
    int16_t nullt = 0;
    std::fwrite(m_levelName.data(), 1, tempint, openfile);
    std::fwrite(&nullt, 1, sizeof(int16_t), openfile);

    // Write death count
    int32_t tempsint = XStd::SwapLE32(m_deaths);
    std::fwrite(&tempsint, 1, sizeof(int32_t), openfile);
}

bool DeathRecord::Load(FILE *openfile)
{
    uint32_t tempint;
    int32_t tempsint;
    size_t got;
    char buf[151];
    XStd::memset(buf, 0, 151);

    // Read string length
    uint32_t length;
    uint32_t skip = 0;

    got = std::fread(&tempint, 1, sizeof(uint32_t), openfile);
    if(got != sizeof(uint32_t))
    {
        pLogWarning("Demos counter Record: Failed to read the length of the level name");
        return false;
    }

    length = XStd::SwapLE32(tempint);

    if(length > 150)
    {
        skip = length - 150;
        length = 150;
    }

    // Read string data
    got = std::fread(buf, 1, length, openfile);
    if(got != length)
    {
        pLogWarning("Demos counter Record: Failed to read the level name");
        return false;
    }

    if(skip > 0)
        std::fseek(openfile, skip, SEEK_CUR);

    m_levelName = std::string(buf);
    std::fseek(openfile, 2, SEEK_CUR);

    // Read death count
    got = std::fread(&tempsint, 1, sizeof(int32_t), openfile);
    if(got != sizeof(int32_t))
    {
        pLogWarning("Demos counter Record: Failed to read the counter value");
        return false;
    }

    m_deaths = XStd::SwapLE32(tempsint);

    return true;
}
