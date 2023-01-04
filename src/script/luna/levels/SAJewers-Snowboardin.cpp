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

/*************************************************
 *  Episode:    A2MBXT, Episode 1: Analog Funk   *
 *  Level:      Snowboardin'                     *
 *  Filename:   SAJewers-Snowboardin.lvl         *
 *  Author:     SAJewers                         *
 *************************************************/

#include "globals.h"
#include "SAJewers-Snowboardin.h"
#include "../lunanpc.h"

#define NPC_SHELL 195

namespace SAJSnowbordin
{

NPC_t *FindNPC(short identity);
int combo_start;

void SnowbordinInitCode()
{
    combo_start = 0;
}

void SnowbordinCode()
{
    NPC_t *shell_npc = FindNPC(NPC_SHELL);

    if(!shell_npc)
        return;

    int &shell_kills = shell_npc->Multiplier;

    if(shell_kills >= 9)
    {
        if(combo_start < 6)
            combo_start += 2;
        shell_kills = combo_start;
    }

    //Renderer::Get().SafePrint(std::wstring(L"KILLS: " + std::to_wstring(*shell_kills)), 3, 0, 256);
}

NPC_t *FindNPC(short identity)
{
    NPC_t *currentnpc = nullptr;

    for(int i = 0; i <= numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if(currentnpc->Type == identity)
            return currentnpc;
    }

    return nullptr;
}

} // SAJSnowbordin
