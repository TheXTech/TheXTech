/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef LOCATION_H
#define LOCATION_H

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
    //! Initial direction [New-added]
    int Direction = 1;
//End Type
};

// creates a temp Location
Location_t newLoc(double X, double Y, double Width = 0, double Height = 0);

#endif // LOCATION_H
