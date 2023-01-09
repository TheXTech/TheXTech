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
#ifndef SpriteComponent_HHH
#define SpriteComponent_HHH


#include <string>

struct SpriteComponent;
class CSprite;

typedef void (*pfnSprFunc)(CSprite *, SpriteComponent *obj);

// Class for wrapping a sprite component.
// Sprite manager will run it (calling func) until run_time == 0
struct SpriteComponent
{
    SpriteComponent() = default;

    void Init(int set_time)
    {
        func = nullptr;
        lookup_code = 0;
        data1 = 0;
        data2 = 0;
        data3 = 0;
        data4 = 0;
        run_time = set_time;
        org_time = set_time;
        expired = false;
    }

    void Tick();            // Tick down the timer of this component. Expires self when run_time reaches 1

    double data1 = 0.0;
    double data2 = 0.0;
    double data3 = 0.0;
    double data4 = 0.0;
    pfnSprFunc func = nullptr;
    int lookup_code = 0;        // Which "bucket" of custom components this component is a part of
    int run_time = 0;           // Run time frame countdown. 0 = infinite lifetime, always run
    int org_time = 0;           // Frame count to begin countdown at

    std::string data5;
    bool expired = false;           // If this component should be deleted at the next clean up cycle
};

#endif // SpriteComponent_HHH
