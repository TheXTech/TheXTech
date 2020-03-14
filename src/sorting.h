/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SORTING_H
#define SORTING_H

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
void FindBlocks();
// Public Sub BlockSort() 'sizable block sorting
// sizable block sorting
void BlockSort();
// Public Sub BlockSort2() 'Super Block sorting / slow and only used when saving
// Super Block sorting / slow and only used when saving
void BlockSort2();
// Public Sub BackgroundSort()
void BackgroundSort();
// Public Function BackGroundPri(A As Integer) As Double 'finds where the backgrounds should be put to set drawing priority
// finds where the backgrounds should be put to set drawing priority
double BackGroundPri(int A);
// Public Sub NPCSort()
void NPCSort();
// Public Sub FindSBlocks() 'sorts sizable blocks
// sorts sizable blocks
void FindSBlocks();
// Public Sub qSortSBlocks(min As Integer, max As Integer)
void qSortSBlocks(int min, int max);
// Public Sub qSortNPCsY(min As Integer, max As Integer)
void qSortNPCsY(int min, int max);
// Public Sub UpdateBackgrounds()
void UpdateBackgrounds();
// Public Sub qSortTempBlocksX(min As Integer, max As Integer)
void qSortTempBlocksX(int min, int max);


#endif // SORTING_H
