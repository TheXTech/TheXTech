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
#ifndef MAGIC_BLOCK_H
#define MAGIC_BLOCK_H

#include <vector>
#include <array>

#include "globals.h"


namespace MagicBlock
{

enum CrossEffectLevel
{
    LEVEL_ALL,
    LEVEL_FAMILY,
    LEVEL_GROUP
};

extern bool enabled;
extern bool replace_existing;
extern bool advanced_mode;
extern bool count_level_edges;
extern CrossEffectLevel check_level;
extern CrossEffectLevel change_level;

void MagicBlock(BlockRef_t A);
void MagicBlock(int Type, Location_t loc);

void MagicBackground(BackgroundRef_t A);
void MagicBackground(int Type, Location_t loc);

void MagicTile(TileRef_t A);
void MagicTile(int Type, Location_t loc);

} // namespace MagicBlock

#endif // MAGIC_BLOCK_H
