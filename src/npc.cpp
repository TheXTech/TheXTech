#include "globals.h"
#include "npc.h"

void UpdateNPCs()
{

}

void DropBonus(int A)
{

}

void TouchBonus(int A, int B)
{

}

void NPCHit(int A, int B, int C)
{

}

void KillNPC(int A, int B)
{

}

void CheckSectionNPC(int A)
{

}

void Deactivate(int A)
{

}

void Bomb(Location location, int Game, int ImmunePlayer)
{

}

void DropNPC(int A, int NPCType)
{

}

void TurnNPCsIntoCoins()
{

}

void NPCFrames(int A)
{

}

void SkullRide(int A)
{

}

void NPCSpecial(int A)
{

}

void SpecialNPC(int A)
{

}

void CharStuff(int WhatNPC, bool CheckEggs)
{

}

int RandomBonus()
{
    int B = std::rand() % 6;
    switch(B)
    {
    case 0:
        return 9;
    case 1:
        return 14;
    case 2:
        return 34;
    case 3:
        return 169;
    case 4:
        return 170;
    case 5:
        return 264;
    }
    return 0;
}
