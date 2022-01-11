#include "lunablock.h"
#include "collision.h"


Block_t *BlocksF::Get(int index)
{
    return &Block[index];
}

int BlocksF::TestCollision(Player_t *pMobPOS, Block_t *pBlockPOS)
{
    return FindCollision(pMobPOS->Location, pBlockPOS->Location);
}

void BlocksF::SetAll(int type1, int type2)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type1)
            Block[i].Type = type2;
    }
}

void BlocksF::SwapAll(int type1, int type2)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type1)
            Block[i].Type = type2;
        else if(Block[i].Type == type2)
            Block[i].Type = type1;
    }
}

void BlocksF::ShowAll(int type)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type)
            Block[i].Invis = false;
    }
}

void BlocksF::HideAll(int type)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type)
            Block[i].Invis = true;
    }
}

bool BlocksF::IsPlayerTouchingType(int type, int sought, Player_t *demo)
{
    //    Block* blocks = Blocks::GetBase();
    Block_t *block = 0;
    double playerX = demo->Location.X - 0.20;
    double playerY = demo->Location.Y - 0.20;
    double playerX2 = demo->Location.X + demo->Location.Width + 0.20;
    double playerY2 = demo->Location.Y + demo->Location.Height + 0.20;

    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type)
        {
            block = &Block[i];

            if(playerX > block->Location.X + block->Location.Width ||
               playerX2 < block->Location.X  ||
               playerY > block->Location.Y + block->Location.Height ||
               playerY2 < block->Location.Y)
                continue;

            if(TestCollision(demo, block) == sought)
                return true;
        }
    }

    return false; // no collision
}
