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

#include <unordered_map>
#include <string>
#include <functional>
#include "lunalevels.h"

#include "globals.h"

#include "levels/Docopoper-Calleoca.h"
#include "levels/Docopoper-AbstractAssault.h"
#include "levels/Docopoper-TheFloorisLava.h"
#include "levels/SAJewers-QraestoliaCaverns.h"
#include "levels/SAJewers-Snowboardin.h"
#include "levels/Talkhaus-Science_Final_Battle.h"
#include "levels/KilArmoryCode.h"


static void (*levelCodeRun)() = nullptr;

static const std::unordered_map<std::string, std::function<void()>> s_levelInit =
{

// Abtract Assault init block
{
    "Docopoper-AbstractAssault.lvl",
    []()->void
    {
        levelCodeRun = AbstractAssaultCode;
    }
},
// Calleoca init block
{
    "Docopoper-Calleoca.lvl",
    []()->void
    {
        CalleocaInitCode();
        levelCodeRun = CalleocaCode;
    }
},
// The Floor is Lava init block
{
    "Docopoper-TheFloorisLava.lvl",
    []()->void
    {
        levelCodeRun = TheFloorisLavaCode;
        TheFloorisLavaInit();
    }
},

// Qraestolia Caverns init block
{
    "SAJewers-QraestoliaCaverns.lvl",
    []()->void
    {
        levelCodeRun = QraestoliaCavernsCode;
    }
},

// Snowbordin init block
{
    "SAJewers-Snowboardin.lvl",
    []()->void
    {
        levelCodeRun = SAJSnowbordin::SnowbordinCode;
        SAJSnowbordin::SnowbordinInitCode();
    }
},

// Science init block
{
    "Talkhaus-Science_Final_Battle.lvl",
    []()->void
    {
        levelCodeRun = ScienceBattle::ScienceCode;
        ScienceBattle::ScienceInitCode();
    }
},

{
    "LUNA12-thou_starts_a_new_video.lvl",
    []()->void
    {
        levelCodeRun = KilArmoryCode;
        KilArmoryInit();
    }
},

};


void lunaLevelsInit()
{
    levelCodeRun = nullptr;

    auto l = s_levelInit.find(FileNameFull);
    if(l != s_levelInit.end())
        l->second();
}

void lunaLevelsClear()
{
    levelCodeRun = nullptr;
}

void lunaLevelsDo()
{
    if(levelCodeRun)
        levelCodeRun();
}
