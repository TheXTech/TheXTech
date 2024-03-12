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

#include "globals.h"
#include "collision.h"

// Intersect collisions
bool CheckCollisionIntersect(const Location_t &Loc1, const Location_t &Loc2)
{
    if(Loc1.Y < Loc2.Y)
        return false;

    if(Loc1.Y + Loc1.Height > Loc2.Y + Loc2.Height)
        return false;

    if(Loc1.X < Loc2.X)
        return false;

    if(Loc1.X + Loc1.Width > Loc2.X + Loc2.Width)
        return false;

    return true;
}


// Make the game easier for the people who whine about the detection being 'off'
bool n00bCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempn00bCollision = false;
    float EZ = 2.f;

    if(float(Loc2.Width) >= 32 - EZ * 2 && float(Loc2.Height) >= 32 - EZ * 2)
    {
        if(float(Loc1.Y) + float(Loc1.Height) - EZ >= float(Loc2.Y))
        {
            if(float(Loc1.Y) + EZ <= float(Loc2.Y) + float(Loc2.Height))
            {
                if(float(Loc1.X) + EZ <= float(Loc2.X + Loc2.Width))
                {
                    if(float(Loc1.X) + float(Loc1.Width) - EZ >= float(Loc2.X))
                    {
                        tempn00bCollision = true;
                    }
                }
            }
        }
    }
    else
    {
        if(Loc1.Y + Loc1.Height >= Loc2.Y)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height)
            {
                if(Loc1.X <= Loc2.X + Loc2.Width)
                {
                    if(Loc1.X + Loc1.Width >= Loc2.X)
                    {
                        tempn00bCollision = true;
                    }
                }
            }
        }
    }

    return tempn00bCollision;
}

// Used when a NPC is activated to see if it should spawn
bool NPCStartCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempNPCStartCollision = false;
    if(Loc1.X < Loc2.X + Loc2.Width)
    {
        if(Loc1.X + Loc1.Width > Loc2.X)
        {
            if(Loc1.Y < Loc2.Y + Loc2.Height)
            {
                if(Loc1.Y + Loc1.Height > Loc2.Y)
                {
                    tempNPCStartCollision = true;
                }
            }
        }
    }
    return tempNPCStartCollision;
}

// Warp point collisions
bool WarpCollision(const Location_t &Loc1, const SpeedlessLocation_t &entrance, int direction)
{
    bool hasCollision = false;

    float X2 = 0;
    float Y2 = 0;

    if(direction == 3)
    {
        X2 = 0;
        Y2 = 32;
    }
    else if(direction == 1)
    {
        X2 = 0;
        Y2 = -30;
    }
    else if(direction == 2)
    {
        X2 = -31;
        Y2 = 32;
    }
    else if(direction == 4)
    {
        X2 = 31;
        Y2 = 32;
    }

    if(float(Loc1.X) <= float(entrance.X) + float(entrance.Width) + X2)
    {
        if(float(Loc1.X) + float(Loc1.Width) >= float(entrance.X) + X2)
        {
            if(float(Loc1.Y) <= float(entrance.Y) + float(entrance.Height) + Y2)
            {
                if(float(Loc1.Y) + float(Loc1.Height) >= float(entrance.Y) + Y2)
                {
                    hasCollision = true;
                }
            }
        }
    }

    return hasCollision;
}

// Whats side the collision happened
int FindCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    int tempFindCollision = COLLISION_NONE;

    if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
    {
        tempFindCollision = COLLISION_TOP;
    }
    else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
    {
        tempFindCollision = COLLISION_RIGHT;
    }
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
    {
        tempFindCollision = COLLISION_LEFT;
    }
    else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
    {
        tempFindCollision = COLLISION_BOTTOM;
    }
    else
    {
        tempFindCollision = COLLISION_CENTER;
    }

    return tempFindCollision;
}

// Whats side the collision happened for belts
int FindCollisionBelt(const Location_t &Loc1, const Location_t &Loc2, float BeltSpeed)
{
    int tempFindCollisionBelt = COLLISION_NONE;

    if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
    {
        tempFindCollisionBelt = COLLISION_TOP;
    }
    else if(Loc1.X - Loc1.SpeedX - BeltSpeed >= Loc2.X + Loc2.Width - Loc2.SpeedX)
    {
        tempFindCollisionBelt = COLLISION_RIGHT;
    }
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
    {
        tempFindCollisionBelt = COLLISION_LEFT;
    }
    else if(Loc1.Y - Loc1.SpeedY - BeltSpeed > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
    {
        tempFindCollisionBelt = COLLISION_BOTTOM;
    }
    else
    {
        tempFindCollisionBelt = COLLISION_CENTER;
    }

    return tempFindCollisionBelt;
}

// Whats side the collision happened for NPCs
int NPCFindCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    int tempNPCFindCollision = COLLISION_NONE;

    if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 4)
    {
        tempNPCFindCollision = COLLISION_TOP;
    }
    else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
    {
        tempNPCFindCollision = COLLISION_RIGHT;
    }
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
    {
        tempNPCFindCollision = COLLISION_LEFT;
    }
    else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
    {
        tempNPCFindCollision = COLLISION_BOTTOM;
    }
    else
    {
        tempNPCFindCollision = COLLISION_CENTER;
    }

    return tempNPCFindCollision;
}

// Easy mode collision for jumping on NPCs
int EasyModeCollision(const Location_t &Loc1, const Location_t &Loc2, bool StandOn)
{
    int tempEasyModeCollision = COLLISION_NONE;

    if(!FreezeNPCs)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 10)
        {
            if(Loc1.SpeedY > Loc2.SpeedY || StandOn)
            {
                tempEasyModeCollision = COLLISION_TOP;
            }
            else
            {
                tempEasyModeCollision = COLLISION_NONE;
            }
        }
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
        {
            tempEasyModeCollision = COLLISION_RIGHT;
        }
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
        {
            tempEasyModeCollision = COLLISION_LEFT;
        }
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
        {
            tempEasyModeCollision = COLLISION_BOTTOM;
        }
        else
        {
            tempEasyModeCollision = COLLISION_CENTER;
        }
    }
    else
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 10)
        {
            tempEasyModeCollision = COLLISION_TOP;
        }
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width)
        {
            tempEasyModeCollision = COLLISION_RIGHT;
        }
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X)
        {
            tempEasyModeCollision = COLLISION_LEFT;
        }
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
        {
            tempEasyModeCollision = COLLISION_BOTTOM;
        }
        else
        {
            tempEasyModeCollision = COLLISION_CENTER;
        }
    }

    return tempEasyModeCollision;
}

// Easy mode collision for jumping on NPCs while on yoshi/boot
int BootCollision(const Location_t &Loc1, const Location_t &Loc2, bool StandOn)
{
    int tempBootCollision = COLLISION_NONE;

    if(!FreezeNPCs)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 16)
        {
            if(Loc1.SpeedY > Loc2.SpeedY || StandOn)
            {
                tempBootCollision = COLLISION_TOP;
            }
            else
            {
                tempBootCollision = COLLISION_NONE;
            }
        }
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
        {
            tempBootCollision = COLLISION_RIGHT;
        }
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
        {
            tempBootCollision = COLLISION_LEFT;
        }
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
        {
            tempBootCollision = COLLISION_BOTTOM;
        }
        else
        {
            tempBootCollision = COLLISION_CENTER;
        }
    }
    else
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 16)
        {
            tempBootCollision = COLLISION_TOP;
        }
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width)
        {
            tempBootCollision = COLLISION_RIGHT;
        }
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X)
        {
            tempBootCollision = COLLISION_LEFT;
        }
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
        {
            tempBootCollision = COLLISION_BOTTOM;
        }
        else
        {
            tempBootCollision = COLLISION_CENTER;
        }
    }

    return tempBootCollision;

}

// Cursor collision
bool CursorCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    return (Loc1.X <= Loc2.X + Loc2.Width - 1) &&
           (Loc1.X + Loc1.Width >= Loc2.X + 1) &&
           (Loc1.Y <= Loc2.Y + Loc2.Height - 1) &&
           (Loc1.Y + Loc1.Height >= Loc2.Y + 1);
}
bool CursorCollision(const Location_t &Loc1, const SpeedlessLocation_t &Loc2)
{
    return (Loc1.X <= Loc2.X + Loc2.Width - 1) &&
           (Loc1.X + Loc1.Width >= Loc2.X + 1) &&
           (Loc1.Y <= Loc2.Y + Loc2.Height - 1) &&
           (Loc1.Y + Loc1.Height >= Loc2.Y + 1);
}
bool CursorCollision(const Location_t &Loc1, const TinyLocation_t &Loc2)
{
    return (Loc1.X <= Loc2.X + Loc2.Width - 1) &&
           (Loc1.X + Loc1.Width >= Loc2.X + 1) &&
           (Loc1.Y <= Loc2.Y + Loc2.Height - 1) &&
           (Loc1.Y + Loc1.Height >= Loc2.Y + 1);
}

// Shakey block collision
bool ShakeCollision(const Location_t &Loc1, const Block_t &b)
{
    const Location_t &Loc2 = b.Location;

    bool tempShakeCollision = false;

    if(Loc1.X + 1 <= Loc2.X + Loc2.Width)
    {
        if(Loc1.X + Loc1.Width - 1 >= Loc2.X)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height + b.ShakeOffset)
            {
                if(Loc1.Y + Loc1.Height >= Loc2.Y + b.ShakeOffset)
                {
                    tempShakeCollision = true;
                }
            }
        }
    }

    return tempShakeCollision;
}

// vScreen collisions
bool vScreenCollision(int A, const Location_t &Loc2)
{
    if(A == 0)
        return true;

    return (-vScreen[A].X <= Loc2.X + Loc2.Width) &&
           (-vScreen[A].X + vScreen[A].Width >= Loc2.X) &&
           (-vScreen[A].Y <= Loc2.Y + Loc2.Height) &&
           (-vScreen[A].Y + vScreen[A].Height >= Loc2.Y);
}

bool vScreenCollision(int A, const SpeedlessLocation_t &Loc2)
{
    if(A == 0)
        return true;

    return (-vScreen[A].X <= Loc2.X + Loc2.Width) &&
           (-vScreen[A].X + vScreen[A].Width >= Loc2.X) &&
           (-vScreen[A].Y <= Loc2.Y + Loc2.Height) &&
           (-vScreen[A].Y + vScreen[A].Height >= Loc2.Y);
}

bool vScreenCollision(int A, const TinyLocation_t &Loc2)
{
    if(A == 0)
        return true;

    return (-vScreen[A].X <= Loc2.X + Loc2.Width) &&
           (-vScreen[A].X + vScreen[A].Width >= Loc2.X) &&
           (-vScreen[A].Y <= Loc2.Y + Loc2.Height) &&
           (-vScreen[A].Y + vScreen[A].Height >= Loc2.Y);
}

// vScreen collisions 2
bool vScreenCollision2(int A, const Location_t &Loc2)
{
    return (-vScreen[A].X + 64 <= Loc2.X + Loc2.Width) &&
           (-vScreen[A].X + vScreen[A].Width - 64 >= Loc2.X) &&
           (-vScreen[A].Y + 96 <= Loc2.Y + Loc2.Height) &&
           (-vScreen[A].Y + vScreen[A].Height - 64 >= Loc2.Y);
}

bool vScreenCollision2(int A, const SpeedlessLocation_t &Loc2)
{
    return (-vScreen[A].X + 64 <= Loc2.X + Loc2.Width) &&
           (-vScreen[A].X + vScreen[A].Width - 64 >= Loc2.X) &&
           (-vScreen[A].Y + 96 <= Loc2.Y + Loc2.Height) &&
           (-vScreen[A].Y + vScreen[A].Height - 64 >= Loc2.Y);
}

bool vScreenCollision2(int A, const TinyLocation_t &Loc2)
{
    return (-vScreen[A].X + 64 <= Loc2.X + Loc2.Width) &&
           (-vScreen[A].X + vScreen[A].Width - 64 >= Loc2.X) &&
           (-vScreen[A].Y + 96 <= Loc2.Y + Loc2.Height) &&
           (-vScreen[A].Y + vScreen[A].Height - 64 >= Loc2.Y);
}

// Collision detection for blocks. Prevents walking on walls.
bool WalkingCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempWalkingCollision = false;

    if(Loc1.X <= Loc2.X + Loc2.Width + Loc1.SpeedX)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X + Loc1.SpeedX)
        {
            tempWalkingCollision = true;
        }
    }

    return tempWalkingCollision;
}

// Collision detection for blocks. Lets NPCs fall through cracks.
bool WalkingCollision2(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempWalkingCollision2 = false;

    if(Loc1.X <= Loc2.X + Loc2.Width - Loc1.SpeedX - 1)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X - Loc1.SpeedX + 1)
        {
            tempWalkingCollision2 = true;
        }
    }

    return tempWalkingCollision2;
}

// Factors in beltspeed
bool WalkingCollision3(const Location_t &Loc1, const Location_t &Loc2, float BeltSpeed)
{
    bool tempWalkingCollision3 = false;

    if(Loc1.X <= Loc2.X + Loc2.Width - (Loc1.SpeedX + BeltSpeed) - 1)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X - (Loc1.SpeedX + BeltSpeed) + 1)
        {
            tempWalkingCollision3 = true;
        }
    }

    return tempWalkingCollision3;
}

// Helps the player to walk over 1 unit cracks
int FindRunningCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    int tempFindRunningCollision = COLLISION_NONE;

    if(Loc1.Y + Loc1.Height - Loc1.SpeedY - 2.5 <= Loc2.Y - Loc2.SpeedY)
    {
        tempFindRunningCollision = COLLISION_TOP;
    }
    else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
    {
        tempFindRunningCollision = COLLISION_RIGHT;
    }
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
    {
        tempFindRunningCollision = COLLISION_LEFT;
    }
    else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
    {
        tempFindRunningCollision = COLLISION_BOTTOM;
    }
    else
    {
        tempFindRunningCollision = COLLISION_CENTER;
    }

    return tempFindRunningCollision;
}

// Determines if an NPC should turnaround
bool ShouldTurnAround(const Location_t &Loc1, const Location_t &Loc2, float Direction)
{
    bool tempShouldTurnAround = false;
    tempShouldTurnAround = true;

    if(Loc1.Y + Loc1.Height + 8 <= Loc2.Y + Loc2.Height)
    {
        if(Loc1.Y + Loc1.Height + 8 >= Loc2.Y)
        {
            if(Loc1.X + Loc1.Width * 0.5 + (8 * Direction) <= Loc2.X + Loc2.Width)
            {
                if(Loc1.X + Loc1.Width * 0.5 + (8 * Direction) >= Loc2.X)
                {
                    if(Loc2.Y > Loc1.Y + Loc1.Height - 8)
                    {
                        tempShouldTurnAround = false;
                    }
                }
            }
        }
    }

    return tempShouldTurnAround;
}

// Determines if an NPC can come out of a pipe
bool CanComeOut(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempCanComeOut = false;
    tempCanComeOut = true;

    if(Loc1.X <= Loc2.X + Loc2.Width + 32)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X - 32)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height + 300)
            {
                if(Loc1.Y + Loc1.Height >= Loc2.Y - 300)
                {
                    tempCanComeOut = false;
                }
            }
        }
    }

    return tempCanComeOut;
}

// Fixes NPCs sinking through the ground
bool CheckHitSpot1(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempCheckHitSpot1 = false;

    if(Loc1.Y + Loc1.Height - Loc1.SpeedY - Physics.NPCGravity <= Loc2.Y - Loc2.SpeedY)
    {
        tempCheckHitSpot1 = true;
    }

    return tempCheckHitSpot1;
}

static inline double blockGetTopYTouching(const Block_t &block, const Location_t& loc)
{
    // Get slope type
    int blockType = block.Type;
    int slopeDirection;

    if((blockType >= 1) && (blockType <= maxBlockType))
    {
        slopeDirection = BlockSlope[blockType];
    }
    else
    {
        slopeDirection = 0;
    }

    // The simple case, no slope
    if(slopeDirection == 0)
    {
        return block.Location.Y;
    }

    // The degenerate case, no width
    if(block.Location.Width <= 0)
    {
        return block.Location.Y;
    }

    // The following uses a slope calculation like 1.3 does

    // Get right or left x coordinate as relevant for the slope direction
    double refX = loc.X;
    if(slopeDirection < 0)
        refX += loc.Width;

    // Get how far along the slope we are in the x direction
    double slope = (refX - block.Location.X) / block.Location.Width;
    if(slopeDirection < 0) slope = 1.0 - slope;
    if(slope < 0.0) slope = 0.0;
    if(slope > 1.0) slope = 1.0;

    // Determine the y coordinate
    return block.Location.Y + block.Location.Height * slope;
}

bool CompareWalkBlock(int oldBlockIdx, int newBlockIdx, const Location_t &referenceLoc)
{
    if(oldBlockIdx > numBlock)
        return false;
    if(newBlockIdx > numBlock)
        return false;

    const Block_t& oldBlock = Block[oldBlockIdx];
    const Block_t& newBlock = Block[newBlockIdx];

    double newBlockY = blockGetTopYTouching(newBlock, referenceLoc);
    double oldBlockY = blockGetTopYTouching(oldBlock, referenceLoc);

    if(newBlockY < oldBlockY)
    {
        // New block is higher, replace
        return true;
    }
    else if(newBlockY > oldBlockY)
    {
        // New block is lower, don't replace
        return false;
    }

    // Break tie based on if one is moving upward faster
    double newBlockSpeedY = newBlock.Location.SpeedY;
    double oldBlockSpeedY = oldBlock.Location.SpeedY;

    if(newBlockSpeedY < oldBlockSpeedY)
    {
        // New block is moving more upward, replace
        return true;
    }
    else if(newBlockSpeedY > oldBlockSpeedY)
    {
        // New block is moving more downward, don't replace
        return false;
    }

    double refX = referenceLoc.X + referenceLoc.Width * 0.5;

    // Break tie based on which of blocks intersects the center point
    bool oldIntersects = referenceLoc.X >= oldBlock.Location.X && referenceLoc.X + referenceLoc.Width <= oldBlock.Location.X + oldBlock.Location.Width;
    bool newIntersects = referenceLoc.X >= newBlock.Location.X && referenceLoc.X + referenceLoc.Width <= newBlock.Location.X + newBlock.Location.Width;

    if(newIntersects && !oldIntersects)
    {
        // New block intersects
        return true;
    }
    else if(!newIntersects && oldIntersects)
    {
        // Old block intersects
        return false;
    }

    // Break tie based on x-proximity
    double newBlockDist = abs((newBlock.Location.X + newBlock.Location.Width * 0.5) - refX);
    double oldBlockDist = abs((oldBlock.Location.X + oldBlock.Location.Width * 0.5) - refX);

    if(newBlockDist < oldBlockDist)
    {
        // New block is closer, replace
        return true;
    }
    if(newBlockDist > oldBlockDist)
    {
        // New block further, don't replace
        return false;
    }

    // Break tie based on narrower width (more specific match)
    double newBlockWidth = newBlock.Location.Width;
    double oldBlockWidth = oldBlock.Location.Width;

    if(newBlockWidth < oldBlockWidth)
    {
        // New block is narrower, replace
        return true;
    }

    if(newBlockWidth > oldBlockWidth)
    {
        // New block wider, don't replace
        return false;
    }

    // Still tied? Let's just not replace
    return false;
}

void CompareNpcWalkBlock(int &tempHitBlock, int &tempHitOldBlock,
                         double &tempHit,   double &tempHitOld,
                         int &tempHitIsSlope, NPC_t *npc)
{
    int oldBlockIdx  = tempHitOldBlock;
    int newBlockIdx  = tempHitBlock;
    Location_t &loc = npc->Location;

    // If no temp block was already set, just exit
    if(tempHitOldBlock == 0)
    {
        // tempHitBlock is set, don't revert
        tempHitIsSlope = 0;
        if(npc->Slope && (npc->Slope != newBlockIdx))
        {
            if(CompareWalkBlock(npc->Slope, newBlockIdx, loc) != 0)
            {
                npc->Slope = 0;
            }
        }
        return;
    }

    // Compare blocks
    int compareResult = CompareWalkBlock(oldBlockIdx, newBlockIdx, loc);

    // Revert to the old block if the comparison says we shouldn't replace
    if(compareResult == 0)
    {
        // We shouldn't replace, so revert variables
        tempHitBlock = tempHitOldBlock;
        tempHit = tempHitOld;
    }
    else
    {
        // tempHitBlock is set, don't revert
        tempHitIsSlope = 0;
        if(npc->Slope && (npc->Slope != newBlockIdx))
        {
            if(CompareWalkBlock(npc->Slope, newBlockIdx, loc) != 0)
            {
                npc->Slope = 0;
            }
        }
    }
}

bool SectionCollision(const int section, const Location_t &loc)
{
    const auto &sec = level[section];
    const double gap = 64.0;

    if(loc.X + loc.Width < sec.X - gap)
        return false;

    if(loc.X > sec.Width + gap)
        return false;

    if(loc.Y + loc.Height < sec.Y - gap)
        return false;

    if(loc.Y > sec.Height)
        return false;

    return true;
}
