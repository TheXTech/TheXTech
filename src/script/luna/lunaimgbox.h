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

#pragma once
#ifndef LUNAIMGBOX_H
#define LUNAIMGBOX_H

#include <string>
#include "std_picture.h"

#define DEFAULT_TRANS_COLOR 0xFF00DC

// A user-loaded bitmap container
class LunaImage
{
    static uint64_t getNewUID();
    uint64_t m_uid = 0;

    //! Height of bitmap
    int m_H = 0;
    //! Width of bitmap
    int m_W = 0;

public:
    /// Functions ///
    LunaImage();
    ~LunaImage();

    LunaImage(const std::string &filename);
    LunaImage(const LunaImage &o);
    LunaImage &operator=(const LunaImage &o);

    void Init();

    void Unload();

    bool ImageLoaded();

    inline uint64_t getUID() const
    {
        return m_uid;
    }

    inline int32_t getW() const
    {
        return m_W;
    }

    inline int32_t getH() const
    {
        return m_H;
    }

    //! Value that represents transparency (will write nothing)
    int m_TransColor = DEFAULT_TRANS_COLOR;

    //! Image handler
    StdPicture m_image;
};

#endif // LUNAIMGBOX_H
