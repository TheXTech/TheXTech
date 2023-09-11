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

#pragma once
#ifndef LOCATION_H
#define LOCATION_H

#include <cstdint>

//Public Type Location    'Holds location information for objects
struct Location_t
{
//    X As Double
    double X = 0.0;
//    Y As Double
    double Y = 0.0;
//    Height As Double
    double Height = 0.0;
//    Width As Double
    double Width = 0.0;
//    SpeedX As Double
    double SpeedX = 0.0;
//    SpeedY As Double
    double SpeedY = 0.0;
//End Type
};

//NEW: 'Holds location information for an object without speed at integer coordinates, with width / height values below 32767
struct TinyLocation_t
{
    int32_t X = 0;
    int32_t Y = 0;
    int16_t Height = 0;
    int16_t Width = 0;

    inline TinyLocation_t() = default;
    inline explicit TinyLocation_t(const Location_t& loc) : X(loc.X), Y(loc.Y), Height(loc.Height), Width(loc.Width) {}

    inline explicit operator Location_t() const
    {
        Location_t ret;
        ret.X = X;
        ret.Y = Y;
        ret.Height = Height;
        ret.Width = Width;

        return ret;
    }
};

//NEW: 'Holds location information for an object without speed
struct SpeedlessLocation_t
{
//    X As Double
    double X = 0.0;
//    Y As Double
    double Y = 0.0;
//    Height As Double
    double Height = 0.0;
//    Width As Double
    double Width = 0.0;

    inline SpeedlessLocation_t() = default;
    inline explicit SpeedlessLocation_t(const Location_t& loc) : X(loc.X), Y(loc.Y), Height(loc.Height), Width(loc.Width) {}

    inline explicit operator Location_t() const
    {
        Location_t ret;
        ret.X = X;
        ret.Y = Y;
        ret.Height = Height;
        ret.Width = Width;

        return ret;
    }
};

//NEW: 'Holds location information for player start location, including Direction
struct PlayerStart_t
{
//    X As Double
    double X = 0.0;
//    Y As Double
    double Y = 0.0;
//    Height As Double
    double Height = 0.0;
//    Width As Double
    double Width = 0.0;
//! Initial direction [New-added]
    int Direction = 1;
//End Type

    inline PlayerStart_t() = default;
    inline PlayerStart_t(const Location_t& loc) : X(loc.X), Y(loc.Y), Height(loc.Height), Width(loc.Width) {}

    inline bool isNull() const
    {
        return int(X * 100000) == 0 &&
               int(Y * 100000) == 0 &&
               int(Width * 100000) == 0 &&
               int(Height * 100000) == 0;
    }

    inline operator Location_t() const
    {
        Location_t ret;
        ret.X = X;
        ret.Y = Y;
        ret.Height = Height;
        ret.Width = Width;

        return ret;
    }
};

// creates a temp Location
Location_t newLoc(double X, double Y, double Width = 0, double Height = 0);
// creates a copy of location with a grid rounding
Location_t roundLoc(const Location_t &inLoc, double grid);

#endif // LOCATION_H
