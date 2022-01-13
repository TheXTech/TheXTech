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

/*************************************************
 *  Episode:    A2MBXT, Episode 1: Analog Funk   *
 *  Level:      Custom C++ Code, Example #3      *
 *  Filename:   Docopoper-Calleoca.lvl           *
 *  Author:     Docopoper                        *
 *************************************************/

#include <vector>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include "Docopoper-Calleoca.h"
#include "globals.h"
#include "../lunaplayer.h"
#include "../lunanpc.h"

#define PI 3.1415926535897932384626433832795

#define NPC_SIGN 151
#define NPC_COIN 10
#define NPC_FIREBAR 260
#define NPC_GOAL 16

#define OFFSCREEN_DIST 400
#define MIDPOINT_X -194620
#define END_X -188792

using namespace std;

NPC_t* FindNPC(short identity);
void HurtPlayer();
bool TriggerBox(double x1, double y1, double x2, double y2);
int AngleDifference(int angle1, int angle2);

bool init_doonce;

Player_t* demo;

NPC_t* calleoca_npc1 = NULL;
NPC_t* calleoca_npc2 = NULL;
NPC_t* hurt_npc = NULL;
NPC_t* goal_npc = NULL;

int win_timer;
int freeze_timer = 0;
int phase = 0;
double calleoca_x, calleoca_y, storage_x, storage_y;

double thwomp_hspeed = 0;
double thwomp_vspeed = 0;
double thwomp_height, thwomp_bottom;

double missile_direction = 0;
double missile_hspeed = 0;
double missile_vspeed = 0;
double missile_top, missile_bottom;
int missile_fuel = 0;

double fishingboo_hspeed = 0;
double fishingboo_vspeed = 0;
double fishingboo_ferocity = 1;

template <typename T> T Clamp(const T& value, const T& low, const T& high) 
{
  return value < low ? low : (value > high ? high : value); 
}

void CalleocaInitCode()
{
    init_doonce = false;

    phase = 0;

    win_timer = 100;

    thwomp_hspeed = 0;
    thwomp_vspeed = 0;

    freeze_timer = 0;

    missile_direction = 0;
    missile_hspeed = 0;
    missile_vspeed = 0;

    fishingboo_hspeed = 0;
    fishingboo_vspeed = 0;
    fishingboo_ferocity = 1;
}

void Phase0()
{
    calleoca_npc1->Frame = 0;

    if (demo->Location.X > calleoca_x + 128)
    {
        freeze_timer = 45;
        phase = 1;
    }

    if (demo->Location.X >= MIDPOINT_X)
    {
        phase = 3;
        calleoca_x = MIDPOINT_X - 256;
        calleoca_y -= 256;
    }
}

/*************************************************************************************************/
//Standing there
void Phase1()
{
    thwomp_hspeed += (demo->Location.X - calleoca_x) * 0.001;
    if (abs(thwomp_height - calleoca_y) < 8)
    {
        thwomp_hspeed = Clamp<double>(thwomp_hspeed, -9, 9);

        if (TriggerBox(calleoca_x, calleoca_y + 64, calleoca_x + 64, calleoca_y + 512))
            phase = 2;
    }
    else
        thwomp_hspeed = 0;

    thwomp_vspeed = (thwomp_height - calleoca_y) * 0.1;
    thwomp_vspeed = Clamp<double>(thwomp_vspeed, -6.0, 6.0);

    if (freeze_timer > 0)
    {
        thwomp_vspeed = 0;
        freeze_timer--;
    }
        

    calleoca_x += thwomp_hspeed;
    calleoca_y += thwomp_vspeed;

    calleoca_npc1->Frame = 2;
    if (TriggerBox(calleoca_x + 10, calleoca_y + 0, calleoca_x + 54, calleoca_y + 64))
        HurtPlayer();

    if (demo->Location.X >= MIDPOINT_X)
    {
        freeze_timer = 150;
        phase = 3;
    }
}

/*************************************************************************************************/
//Thwomp rising / moving
void Phase2()
{
    thwomp_hspeed *= 0.9;
    if (demo->Location.Y <= calleoca_y)
        thwomp_hspeed *= 0.9;

    thwomp_vspeed += (thwomp_bottom - calleoca_y) * 0.05;
    thwomp_vspeed = Clamp<double>(thwomp_vspeed, -8.0, 8.0);
    
    calleoca_x += thwomp_hspeed;
    calleoca_y += thwomp_vspeed;
    calleoca_npc1->Frame = 1;
    if (TriggerBox(calleoca_x + 10, calleoca_y + 0, calleoca_x + 54, calleoca_y + 64))
        HurtPlayer();

    if (calleoca_y >= thwomp_bottom)
    {
        freeze_timer = 22;
        phase = 1;
    }

    if (demo->Location.X >= MIDPOINT_X)
    {
        freeze_timer = 150;
        phase = 3;
    }
}

/*************************************************************************************************/
//Missile
void Phase3()
{
    calleoca_npc1->Frame = 3 + int(0.5 + missile_direction / 45) % 8;

    int dir = (int)(std::atan2((demo->Location.Y + demo->Location.Height / 2) - (calleoca_y + 32),
                              -(demo->Location.X + demo->Location.Width / 2) + (calleoca_x + 32))
                                * 180 / PI);

    missile_direction += AngleDifference((int)missile_direction, dir) * 0.015;

    if (missile_direction >= 360)
        missile_direction -= 360;
    else
    if (missile_direction < 0)
        missile_direction += 360;

    missile_hspeed += cos(missile_direction * PI / 180) * 0.25;
    missile_vspeed -= sin(missile_direction * PI / 180) * 0.25;

    missile_hspeed = Clamp<double>(missile_hspeed * 0.975, -15, 15);
    missile_vspeed = Clamp<double>(missile_vspeed * 0.975, -15, 15);

    if (freeze_timer > 0)
    {
        freeze_timer--;
        missile_hspeed = 0;
        missile_vspeed = 0;
        missile_fuel = 600;
    }
    else
    {
        missile_fuel--;
        if (missile_fuel <= 0)
            if (!TriggerBox(calleoca_x - 100, calleoca_y - 100, calleoca_x + 164, calleoca_y + 164))
                freeze_timer = 110;
    }

    
    calleoca_x = Clamp<double>(calleoca_x + missile_hspeed, demo->Location.X - 464, demo->Location.X + 464);
    calleoca_y = Clamp<double>(calleoca_y + missile_vspeed, missile_top, missile_bottom);

    if (TriggerBox(calleoca_x + 18, calleoca_y + 18, calleoca_x + 46, calleoca_y + 46))
        HurtPlayer();

    if (demo->Location.X >= END_X)
    {
        if (calleoca_x > END_X - 64)
            calleoca_x = END_X - 64;
        freeze_timer = 150;
        phase = 4;
    }
}

void Phase4()
{
    if (freeze_timer > 0)
    {
        freeze_timer--;
    }
    else
    {
        fishingboo_hspeed += (demo->Location.X - calleoca_x) * 0.001 * fishingboo_ferocity;
        fishingboo_vspeed += (demo->Location.Y - calleoca_y) * 0.001 * fishingboo_ferocity;
        fishingboo_hspeed = Clamp<double>(fishingboo_hspeed, -7 * fishingboo_ferocity, 7 * fishingboo_ferocity);
        fishingboo_vspeed = Clamp<double>(fishingboo_vspeed, -0.5 * fishingboo_ferocity, 0.5 * fishingboo_ferocity);

        calleoca_x += fishingboo_hspeed;
        calleoca_y += fishingboo_vspeed;

        fishingboo_ferocity += 0.0002;
    }

    calleoca_x = Clamp<double>(calleoca_x, demo->Location.X - 512, demo->Location.X + 464);
}

void CalleocaCode()
{
    demo = PlayerF::Get(1);
    
    demo->Character = 1;

    if (calleoca_npc2 == NULL && init_doonce)
    {
        if (win_timer > 0)
            win_timer--;
        else
        {
            goal_npc = FindNPC(NPC_GOAL);
            if (goal_npc != NULL)
            {
                goal_npc->Location.X = demo->Location.X;
                goal_npc->Location.Y = demo->Location.Y;
            }
        }
        
        return; //boss beaten
    }
    
    if (!init_doonce)
    {
        calleoca_npc1 = FindNPC(NPC_SIGN);
        calleoca_npc2 = FindNPC(NPC_COIN);
        hurt_npc	  = FindNPC(NPC_FIREBAR);

        calleoca_x = calleoca_npc1->Location.X;
        calleoca_y = calleoca_npc1->Location.Y;
        storage_x  = calleoca_npc2->Location.X;
        storage_y  = calleoca_npc2->Location.Y;

        thwomp_height = calleoca_y - 64 * 6 + 16;
        thwomp_bottom = calleoca_y + 128;

        missile_top = calleoca_y - 64 * 7 - 16;
        missile_bottom = calleoca_y + 180;

        init_doonce = true;
    }

    if (calleoca_npc1->Type != NPC_SIGN)
        calleoca_npc1 = FindNPC(NPC_SIGN);

    if (calleoca_npc2->Type != NPC_COIN)
        calleoca_npc2 = FindNPC(NPC_COIN);

    if (hurt_npc->Type != NPC_FIREBAR)
        hurt_npc = FindNPC(NPC_FIREBAR);

    hurt_npc->Location.X = demo->Location.X;
    hurt_npc->Location.Y = demo->Location.Y - 128;

    switch (phase)
    {
        case 0: //Standing there
            Phase0();
        break;

        case 1: //Thwomp rising
            Phase1();
        break;

        case 2: //Thwomp attacking
            Phase2();
        break;

        case 3: //Missile
            Phase3();
        break;

        case 4: //Fishing Boo
            Phase4();
        break;
    }

    if (calleoca_npc2 == NULL)
        return; //boss beaten

    if (phase < 4)
    {
        calleoca_npc1->Location.X = calleoca_x;
        calleoca_npc1->Location.Y = calleoca_y;
        calleoca_npc2->Location.X = storage_x;
        calleoca_npc2->Location.Y = storage_y;
    }
    else
    {
        calleoca_npc2->Location.X = calleoca_x;
        calleoca_npc2->Location.Y = calleoca_y;
        calleoca_npc1->Location.X = storage_x;
        calleoca_npc1->Location.Y = storage_y;
    }

    //Renderer::Get().SafePrint(std::wstring(L"FUEL: " + std::to_wstring(missile_fuel)), 3, 0, 256);
    //Renderer::Get().SafePrint(std::wstring(L"DEMO X: " + std::to_wstring(demo->CurYPos)), 3, 0, 256 + 32);
}

NPC_t* FindNPC(short identity)
{
    NPC_t* currentnpc = NULL;

    for(int i = 0; i <= numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if (currentnpc->Type == identity)
            return currentnpc;
    }

    return NULL;
}

bool TriggerBox(double x1, double y1, double x2, double y2)
{
    return (demo->Location.X + demo->Location.Width		> x1 &&
            demo->Location.X					< x2 &&
            demo->Location.Y + demo->Location.Height	> y1 &&
            demo->Location.Y					< y2);
}

void HurtPlayer()
{
    hurt_npc->Location.Y = demo->Location.Y;
}

int AngleDifference(int angle1, int angle2)
{
    return ((((angle1 - angle2) % 360) + 540) % 360) - 180;
}
