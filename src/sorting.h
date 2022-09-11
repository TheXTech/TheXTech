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
#ifndef SORTING_H
#define SORTING_H

// most functions here have been deprecated by the corresponding quadtree functions, but should be preserved for historical comparison

// Public Sub qSortBlocksY(min As Integer, max As Integer) 'quicksort the blocks Y
// quicksort the blocks Y
void qSortBlocksY(int min, int max);
// Public Sub qSortBlocksX(min As Integer, max As Integer) 'quicksort the blocks X
// quicksort the blocks X
void qSortBlocksX(int min, int max);
// Public Sub qSortBackgrounds(min As Integer, max As Integer) 'quicksort the backgrounds
// quicksort the backgrounds
void qSortBackgrounds(int min, int max);

// Public Sub FindBlocks() 'create a table of contents for blocks for an optimization
// create a table of contents for blocks for an optimization
// void FindBlocks();

// Public Sub BlockSort() 'sizable block sorting
// sizable block sorting
// void BlockSort();
// Public Sub BlockSort2() 'Super Block sorting / slow and only used when saving
// Super Block sorting / slow and only used when saving
// void BlockSort2();
// Public Sub BackgroundSort()
void BackgroundSort();
// Public Function BackGroundPri(A As Integer) As Double 'finds where the backgrounds should be put to set drawing priority
// finds where the backgrounds should be put to set drawing priority
double BackGroundPri(int A);
// Public Sub NPCSort()
void NPCSort();
// Public Sub FindSBlocks() 'sorts sizable blocks
// sorts sizable blocks
// void FindSBlocks();
// Public Sub qSortSBlocks(min As Integer, max As Integer)
// void qSortSBlocks(int min, int max);
// Public Sub qSortNPCsY(min As Integer, max As Integer)
void qSortNPCsY(int min, int max);
// Public Sub UpdateBackgrounds()
void UpdateBackgrounds();
// Public Sub qSortTempBlocksX(min As Integer, max As Integer)
// void qSortTempBlocksX(int min, int max);


#endif // SORTING_H
