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
#ifndef GFX_H
#define GFX_H

#include "range_arr.hpp"
#include "std_picture.h"
#include <vector>
#include <string>

/*!
 * \brief Holder of commonly-used textures such as interface, font, etc.
 */
class GFX_t
{
    //! Holder of loaded textures for easier clean-up
    std::vector<StdPicture*> m_loadedImages;
    //! Capacity of the m_isCustom array
    static constexpr size_t m_isCustomVolume = 66;
    //! Holder of "is custom" flag
    bool m_isCustom[m_isCustomVolume];

    /*!
     * \brief Internal function of the texture loading
     * \param img Target texture
     * \param path Path to the texture file
     */
    void loadImage(StdPicture &img, const std::string &path);

    //! Counter of loading errors
    int m_loadErrors = 0;
public:
    GFX_t() noexcept;
    bool load();
    void unLoad();

    StdPicture BMVs;
    StdPicture BMWin;
    RangeArr<StdPicture, 1, 3> Boot;
    RangeArr<StdPicture, 1, 5> CharacterName;
    StdPicture Chat;
    RangeArr<StdPicture, 0, 2> Container;
    RangeArr<StdPicture, 1, 3> ECursor;
    RangeArr<StdPicture, 0, 9> Font1;
    RangeArr<StdPicture, 1, 3> Font2;
    StdPicture Font2S;
    RangeArr<StdPicture, 1, 2> Heart;
    RangeArr<StdPicture, 0, 8> Interface;
    StdPicture LoadCoin;
    StdPicture Loader;
    RangeArr<StdPicture, 0, 3> MCursor;
    RangeArr<StdPicture, 1, 4> MenuGFX;
    RangeArr<StdPicture, 2, 2> Mount;
    RangeArr<StdPicture, 0, 7> nCursor;
    StdPicture TextBox;
    RangeArr<StdPicture, 1, 2> Tongue;
    StdPicture Warp;
    StdPicture YoshiWings;

    bool &isCustom(size_t i);
};

//! Container of "hardcoded" (no more) graphics
extern GFX_t GFX;

#endif // GFX_H
