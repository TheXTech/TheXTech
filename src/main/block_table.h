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

#ifndef BLOCK_TABLE_H
#define BLOCK_TABLE_H

#include "globals.h"

void treeBlockUpdateLayer(int layer, BlockRef_t block);
bool treeBlockLayerActive(int layer);
void treeBlockJoinLayer(int layer);
void treeBlockSplitLayer(int layer);

void treeBackgroundUpdateLayer(int layer, BackgroundRef_t block);
bool treeBackgroundLayerActive(int layer);
void treeBackgroundJoinLayer(int layer);
void treeBackgroundSplitLayer(int layer);

void treeWaterUpdateLayer(int layer, WaterRef_t block);
bool treeWaterLayerActive(int layer);
void treeWaterJoinLayer(int layer);
void treeWaterSplitLayer(int layer);

#endif // #ifndef BLOCK_TABLE_H
