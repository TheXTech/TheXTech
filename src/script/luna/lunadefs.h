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
#ifndef LUNADEFS_H
#define LUNADEFS_H

#define LUNA_VERSION    9

enum FIELDTYPE
{
    FT_INVALID = 0,
    FT_BYTE = 1,
    FT_WORD = 2,
    FT_DWORD = 3,
    FT_FLOAT = 4,
    FT_DFLOAT = 5,

    FT_MAX = 5
};

enum OPTYPE
{
    OP_Assign = 0,
    OP_Add = 1,
    OP_Sub = 2,
    OP_Mult = 3,
    OP_Div = 4,
    OP_XOR = 5,
    OP_ABS = 6
};

enum COMPARETYPE
{
    CMPT_EQUALS = 0,
    CMPT_GREATER = 1,
    CMPT_LESS = 2,
    CMPT_NOTEQ = 3
};

enum DIRECTION
{
    DIR_UP = 1,
    DIR_RIGHT = 2,
    DIR_DOWN = 3,
    DIR_LEFT = 4
};

enum PRIORITY
{
    PRI_LOW = 0,
    PRI_MID,
    PRI_HIGH
};

enum WORLD_HUD_CONTROL
{
    WHUD_ALL,
    WHUD_ONLY_OVERLAY,
    WHUD_NONE
};

enum LEVEL_HUD_CONTROL
{
    LHUD_UNKNOWN1 // Only temporary
};

struct LunaRect
{
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

class LunaImage;
class CSprite;
struct CSpriteRequest;
struct SpriteComponent;

typedef void (*pfnSprFunc)(CSprite*, SpriteComponent* obj);
typedef void (*pfnSprDraw)(CSprite*);

#endif // LUNADEFS_H
