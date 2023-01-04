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

#include "lunamisc.h"
#include "globals.h"
#include "collision.h"
#include "rand.h"
#include <Utils/files.h>
#include "global_dirs.h"


void InitIfMissing(std::map<std::string, double> *pMap, const std::string& sought_key, double init_val)
{
    if(pMap->find(sought_key) == pMap->end())
        (*pMap)[sought_key] = init_val;
}

bool FastTestCollision(int L1, int U1, int R1, int D1, int L2, int U2, int R2, int D2)
{
    bool rightcol = true;
    bool leftcol = true;
    bool upcol = true;
    bool downcol = true;

    if(R1 < L2)
        rightcol = false;
    if(L1 > R2)
        leftcol = false;
    if(U1 > D2)
        downcol = false;
    if(D1 < U2)
        upcol = false;

    if(!rightcol || !leftcol || !upcol || !downcol)
        return false;

    return true;
}

int ComputeLevelSection(int x, int y)
{
    Location_t l;
    l.X = x - 64;
    l.Y = y - 64;
    l.Width = 128;
    l.Height = 128;

    int ret = -1;

    for(int i = 0; i <= numSections; ++i)
    {
        if(SectionCollision(i, l))
        {
            ret = i;
            break;
        }
    }

    return ret;
}

void RandomPointInRadius(double *ox, double *oy, double cx, double cy, int radius)
{
    double phase1 = iRand2(360);
    double phase2 = iRand2(360);
    double xoff = std::sin(phase1) * radius;
    double yoff = std::cos(phase2) * radius;

    *ox = cx + xoff;
    *oy = cy + yoff;
}

std::string resolveIfNotAbsolutePath(const std::string &filename)
{
    if(Files::isAbsolute(filename)) {
        return filename;
    }

    std::vector<DirListCI *> dirs =
    {
        &g_dirCustom,
        &g_dirEpisode
    };

    for(DirListCI *nextSearchDir : dirs)
    {
        std::string nextEntry = nextSearchDir->resolveFileCaseExistsAbs(filename);
        if(nextEntry.empty())
            continue;
        return nextEntry;
    }

    return filename;
}
