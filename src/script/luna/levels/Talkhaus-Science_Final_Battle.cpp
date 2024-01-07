/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

/**************************************************
 *  Episode:    A2MBXT, Episode 1: Analog Funk    *
 *  Level:      A Final Thing (Name Pending)      *
 *  Filename:   Talkhaus-Science_Final_Battle.lvl *
 *  Authors:    Septentrion_Pleiades, Docopoper,  *
 *              SAJewers, Sturgyman, Demolition   *
 **************************************************/

#include <vector>

#include "sdl_proxy/sdl_assert.h"
#include "sdl_proxy/sdl_stdinc.h"

#include "Talkhaus-Science_Final_Battle.h"
#include "globals.h"
#include "main/trees.h"
#include "../lunanpc.h"
#include "../lunaplayer.h"


#define NPCID_FIRE_CHAIN 260
#define NPCID_DOUGHNUT 210
#define NPCID_SCIENCE 209

namespace ScienceBattle
{

static NPC_t *FindNPC(short identity);
static std::vector<NPC_t *> FindAllNPC(short identity);
static bool TriggerBox(double x1, double y1, double x2, double y2);
static void HurtPlayer();

static bool init_doonce;
static NPC_t *hurt_npc; //, *science_npc, *friendly_doughnut;
static std::vector<NPC_t *> doughnuts;
static Player_t *demo;
static int hurt_timer;
static int grace_timer;
static int throw_timer;

void ScienceInitCode()
{
    init_doonce = false;
}

void ScienceCode()
{
    if(!init_doonce)
    {
        init_doonce     = true;
        hurt_timer      = 0;
        throw_timer     = 0;
        demo            = PlayerF::Get(1);
        SDL_assert_release(demo);
    }

    hurt_npc = FindNPC(NPCID_FIRE_CHAIN);

    if(!hurt_npc)
        return;


    if(hurt_timer <= 0)
        hurt_npc->Location.Y = demo->Location.Y - 128;
    else
    {
        hurt_timer--;
        hurt_npc ->Location.Y = demo->Location.Y;
    }

    hurt_npc->Location.X = demo->Location.X;
    treeNPCUpdate(hurt_npc);

    doughnuts = FindAllNPC(NPCID_DOUGHNUT);

    if(demo->HoldingNPC > 0)
        throw_timer = 30;

    //Renderer::Get().SafePrint(std::wstring(L"ID: " + std::to_wstring(demo->HeldNPCIndex)), 3, 0, 256);


    if(grace_timer >= 0)
    {
        for(auto doughnut : doughnuts)
        {
            double x_diff, y_diff, m;

            x_diff = doughnut->Location.X - demo->Location.X;
            y_diff = doughnut->Location.Y - demo->Location.Y;
            m = SDL_sqrt(x_diff * x_diff + y_diff * y_diff);

            if(m == 0)
                continue;

            x_diff /= m;
            y_diff /= m;

            doughnut->Location.X += x_diff * 15;
            doughnut->Location.Y += y_diff * 15;
            treeNPCUpdate(doughnut);
        }
        grace_timer--;
    }
    else
    {
        if(throw_timer <= 0)
        {
            for(auto doughnut : doughnuts)
            {
                //Ignore generators
                if(doughnut->Hidden) // if((*((int *)doughnut + 16)) != 0)
                    continue;
                // Explanation why "Hidden":
                //   1) pointer turned into int* format
                //   2) +16 made an offset with 16 steps of int, i.e. 4 bytes. So, offset is 4x16 = 64
                //   3) at 64 (0x40) position, the "Hidden" field, not "generator"

                double x1, x2, y1, y2;

                x1 = doughnut->Location.X + 28 * 0.42;
                y1 = doughnut->Location.Y + 32 * 0.42;
                x2 = doughnut->Location.X + 28 * 0.57;
                y2 = doughnut->Location.Y + 32 * 0.57;

                if(TriggerBox(x1, y1, x2, y2))
                    HurtPlayer();
            }
        }
    }


    if(throw_timer > 0)
        throw_timer--;
}

static NPC_t *FindNPC(short identity)
{
    NPC_t *currentnpc = nullptr;

    for(int i = 0; i < numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if(currentnpc && currentnpc->Type == identity)
            return currentnpc;
    }

    return nullptr;
}

static std::vector<NPC_t *> FindAllNPC(short identity)
{
    std::vector<NPC_t *> npcs_found = std::vector<NPC_t *>();
    NPC_t *currentnpc = nullptr;

    for(int i = 0; i < numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if(currentnpc && currentnpc->Type == identity)
            npcs_found.push_back(currentnpc);
    }

    return npcs_found;
}

static void HurtPlayer()
{
    hurt_timer = 3;
    grace_timer = 120;
}

static bool TriggerBox(double x1, double y1, double x2, double y2)
{
    return (demo->Location.X + demo->Location.Width     > x1 &&
            demo->Location.X                            < x2 &&
            demo->Location.Y + demo->Location.Height    > y1 &&
            demo->Location.Y                            < y2);
}

} // ScienceBattle
