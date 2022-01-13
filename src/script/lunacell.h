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
#ifndef CELLMANAGER_H
#define CELLMANAGER_H

#include <list>

#define DEF_CELL_H 96
#define DEF_CELL_W 96
#define HASH_PRIME_1 0x8da6b343
#define HASH_PRIME_2 0xd8163841
#define BUCKET_COUNT 1987

enum CELL_OBJ_TYPE
{
    CLOBJ_INVALID = 0,
    CLOBJ_SMBXBLOCK = 1
};


struct CellObj   // Objects that can be contained in a Cell, such as block/NPC references
{
    CELL_OBJ_TYPE Type;
    void *pObj;
};


struct Cell   // Cells that hold references to objects and are singly linked
{
    Cell(int _x, int _y);
    int x, y;
    Cell *pNext;
    std::list<CellObj> ContainedObjs;

    int CountDownward(int *oObjCount);  // Return count of downard linked cells, including this one as head + optionally count of total objs
    bool AddUnique(CellObj obj);        // Add an object to this cell ONLY if it doesn't already exist in it
};


struct Bucket   // Map buckets that hold (sometimes multiple) cells
{
    Cell *ContainedCellsHead;
};


// Spatial partitioning & collision detection manager
class CellManager
{
public:
    CellManager();

    /// Functions ///
    void Reset();                                   // Re-initialize cell manager
    void ClearAllBuckets();
    void ClearBucket(int bucket_index);             // Clear bucket by successively deleting its contained cells

    void CountAll(int *oFilledBuckets, int *oCellCount, int *oObjReferences);

    int ComputeHashBucketIndex(int x, int y);               // Get index into bucket array the given coords would have
    void ScanLevel(bool update_blocks);                     // Rebuild cell grid, scan in objs of the specified types
    void AddObj(void *pObj, CELL_OBJ_TYPE);                 // Add object to map at coords, creating new cell if necessary

    void AddCell(int bucket_index, Cell *pcell);            // Add cell at the end of cell list in bucket
    Cell *FindCell(int bucket_index, int x, int y);         // Finds cell in given bucket, or returns null

    void GetObjectsOfInterest(std::list<CellObj> *objlist, double x, double y, int w, int h);   // Get objs that might be intersecting a rectangle
    void GetUniqueObjs(std::list<CellObj> *objlist, double cellx, double celly);        // Get objs from cell (don't add any that are already in the list)

    static void SortByNearest(std::list<CellObj> *objlist, double cx, double cy); // Sort a list of cell objects by which is closest to cx/cy

    /// Members ///
    Bucket m_BucketArray[BUCKET_COUNT];
};

inline double SnapToGrid(double coord, double span);

extern CellManager  gCellMan;

#endif // CELLMANAGER_H
