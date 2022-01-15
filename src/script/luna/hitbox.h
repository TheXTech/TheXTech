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

#pragma once
#ifndef HitBox_HHHHH
#define HitBox_HHHHH

#include <cstddef>

class CSprite;

enum COLLISION_TYPE
{
    COLTYPE_NONE = 0,
    COLTYPE_LEFT,
    COLTYPE_RIGHT,
    COLTYPE_TOP,
    COLTYPE_BOT,
};

struct Hitbox
{
    Hitbox() = default;

    double CalcLeft() const;      // Get the left absolute position (with parent base coords)
    double CalcRight() const;     // Get the right absolute position (with parent base coords)
    double CalcTop() const;       // Get the top absolute position (with parent base coords)
    double CalcBottom() const;    // Get the bottom absolute position (with parent base coords)
    double CenterX() const;       // Get the center X position (from parent base coords)
    double CenterY() const;       // Get the center Y position (from parent base coords)

    bool Test(int left, int up, int iW, int iH) const; // Test hitbox against given rect
    bool Test(int cx, int cy, int radius) const;              // Test hitbox against given circle

    short Left_off = 0;         // Offset from 0,0 on sprite
    short Top_off = 0;          // Offset from 0,0 on sprite
    short W = 0;                // Width
    short H = 0;                // Height

    //COLLISION_TYPE GetCollisionDir(int left, int up, int right, int down); // Get the direction this hitbox is colliding with given rect
    char CollisionType = 0;     // 0 = square aabb  1 = circle/dist
    CSprite *pParent = nullptr;   // Pointer to parent sprite (for calculating actual coordinates + collision area)

};

#endif // HitBox_HHHHH
