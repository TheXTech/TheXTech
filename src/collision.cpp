/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "globals.h"
#include "collision.h"

// 'Normal collisions
bool CheckCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    bool tempCheckCollision = false;
    if(Loc1.Y + Loc1.Height >= Loc2.Y)
    {
        if(Loc1.Y <= Loc2.Y + Loc2.Height)
        {
            if(Loc1.X <= Loc2.X + Loc2.Width)
            {
                if(Loc1.X + Loc1.Width >= Loc2.X)
                {
                    tempCheckCollision = true;
                    return tempCheckCollision;
                }
            }
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.Y + Loc1.Height >= Loc2.Y)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height)
            {
                if(Loc1.X <= Loc2.X - (level[section].Width - level[section].X) + Loc2.Width)
                {
                    if(Loc1.X + Loc1.Width >= Loc2.X - (level[section].Width - level[section].X))
                    {
                        tempCheckCollision = true;
                        return tempCheckCollision;
                    }
                }
                else if(Loc1.X <= Loc2.X + (level[section].Width - level[section].X) + Loc2.Width)
                {
                    if(Loc1.X + Loc1.Width >= Loc2.X + (level[section].Width - level[section].X))
                    {
                        tempCheckCollision = true;
                        return tempCheckCollision;
                    }
                }
            }
        }
    }
    return tempCheckCollision;
}

// Make the game easier for the people who whine about the detection being 'off'
bool n00bCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
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
                        return tempn00bCollision;
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
                        return tempn00bCollision;
                    }
                }
            }
        }
    }
    if(LevelWrap2[section] && !LevelSelect)
    {
        if(float(Loc2.Width) >= 32 - EZ * 2 && float(Loc2.Height) >= 32 - EZ * 2)
        {
            if(float(Loc1.Y) + float(Loc1.Height) - EZ >= float(Loc2.Y))
            {
                if(float(Loc1.Y) + EZ <= float(Loc2.Y) + float(Loc2.Height))
                {
                    if(float(Loc1.X) + EZ <= float(Loc2.X + Loc2.Width - (level[section].Width - level[section].X)))
                    {
                        if(float(Loc1.X) + float(Loc1.Width) - EZ >= float(Loc2.X - (level[section].Width - level[section].X)))
                        {
                            tempn00bCollision = true;
                            return tempn00bCollision;
                        }
                    }
                    else if(float(Loc1.X) + EZ <= float(Loc2.X + Loc2.Width + (level[section].Width - level[section].X)))
                    {
                        if(float(Loc1.X) + float(Loc1.Width) - EZ >= float(Loc2.X + (level[section].Width - level[section].X)))
                        {
                            tempn00bCollision = true;
                            return tempn00bCollision;
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
                    if(Loc1.X <= Loc2.X + Loc2.Width - (level[section].Width - level[section].X))
                    {
                        if(Loc1.X + Loc1.Width >= Loc2.X - (level[section].Width - level[section].X))
                        {
                            tempn00bCollision = true;
                            return tempn00bCollision;
                        }
                    }
                    else if(Loc1.X <= Loc2.X + Loc2.Width + (level[section].Width - level[section].X))
                    {
                        if(Loc1.X + Loc1.Width >= Loc2.X + (level[section].Width - level[section].X))
                        {
                            tempn00bCollision = true;
                            return tempn00bCollision;
                        }
                    }
                }
            }
        }
    }
    return tempn00bCollision;
}

// Used when a NPC is activated to see if it should spawn
bool NPCStartCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
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
                    return tempNPCStartCollision;
                }
            }
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.X < Loc2.X + Loc2.Width - (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width > Loc2.X - (level[section].Width - level[section].X))
            {
                if(Loc1.Y < Loc2.Y + Loc2.Height)
                {
                    if(Loc1.Y + Loc1.Height > Loc2.Y)
                    {
                        tempNPCStartCollision = true;
                        return tempNPCStartCollision;
                    }
                }
            }
        }
        else if(Loc1.X < Loc2.X + Loc2.Width + (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width > Loc2.X + (level[section].Width - level[section].X))
            {
                if(Loc1.Y < Loc2.Y + Loc2.Height)
                {
                    if(Loc1.Y + Loc1.Height > Loc2.Y)
                    {
                        tempNPCStartCollision = true;
                        return tempNPCStartCollision;
                    }
                }
            }
        }
    }
    return tempNPCStartCollision;
}

// Warp point collisions
bool WarpCollision(const Location_t &Loc1, int A, int section)
{
    bool tempWarpCollision = false;
    float X2 = 0;
    float Y2 = 0;

    auto &tempVar = Warp[A];
    if(tempVar.Direction == 3)
    {
        X2 = 0;
        Y2 = 32;
    }
    else if(tempVar.Direction == 1)
    {
        X2 = 0;
        Y2 = -30;
    }
    else if(tempVar.Direction == 2)
    {
        X2 = -31;
        Y2 = 32;
    }
    else if(tempVar.Direction == 4)
    {
        X2 = 31;
        Y2 = 32;
    }

    if(float(Loc1.X) <= float(tempVar.Entrance.X) + float(tempVar.Entrance.Width) + X2)
    {
        if(float(Loc1.X) + float(Loc1.Width) >= float(tempVar.Entrance.X) + X2)
        {
            if(float(Loc1.Y) <= float(tempVar.Entrance.Y) + float(tempVar.Entrance.Height) + Y2)
            {
                if(float(Loc1.Y) + float(Loc1.Height) >= float(tempVar.Entrance.Y) + Y2)
                {
                    tempWarpCollision = true;
                    return tempWarpCollision;
                }
            }
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(float(Loc1.X) <= float(tempVar.Entrance.X) + float(tempVar.Entrance.Width) + X2 - (level[section].Width - level[section].X))
        {
            if(float(Loc1.X) + float(Loc1.Width) >= float(tempVar.Entrance.X) + X2 - (level[section].Width - level[section].X))
            {
                if(float(Loc1.Y) <= float(tempVar.Entrance.Y) + float(tempVar.Entrance.Height) + Y2)
                {
                    if(float(Loc1.Y) + float(Loc1.Height) >= float(tempVar.Entrance.Y) + Y2)
                    {
                        tempWarpCollision = true;
                        return tempWarpCollision;
                    }
                }
            }
        }
        else if(float(Loc1.X) <= float(tempVar.Entrance.X) + float(tempVar.Entrance.Width) + X2 + (level[section].Width - level[section].X))
        {
            if(float(Loc1.X) + float(Loc1.Width) >= float(tempVar.Entrance.X) + X2 + (level[section].Width - level[section].X))
            {
                if(float(Loc1.Y) <= float(tempVar.Entrance.Y) + float(tempVar.Entrance.Height) + Y2)
                {
                    if(float(Loc1.Y) + float(Loc1.Height) >= float(tempVar.Entrance.Y) + Y2)
                    {
                        tempWarpCollision = true;
                        return tempWarpCollision;
                    }
                }
            }
        }
    }
    return tempWarpCollision;
}

// Whats side the collision happened
int FindCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    int tempFindCollision = 5;
    if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
        tempFindCollision = 1;
    else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
        tempFindCollision = 2;
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
        tempFindCollision = 4;
    else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
        tempFindCollision = 3;
    if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
            tempFindCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX - (level[section].Width - level[section].X))
            tempFindCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX - (level[section].Width - level[section].X))
            tempFindCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
            tempFindCollision = 3;

        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
            tempFindCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX + (level[section].Width - level[section].X))
            tempFindCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX + (level[section].Width - level[section].X))
            tempFindCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
            tempFindCollision = 3;
    }
    return tempFindCollision;
}

// Whats side the collision happened for belts
int FindCollisionBelt(const Location_t &Loc1, const Location_t &Loc2, float BeltSpeed, int section)
{
    int tempFindCollisionBelt = 5;
    if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
        tempFindCollisionBelt = 1;
    else if(Loc1.X - Loc1.SpeedX - BeltSpeed >= Loc2.X + Loc2.Width - Loc2.SpeedX)
        tempFindCollisionBelt = 2;
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
        tempFindCollisionBelt = 4;
    else if(Loc1.Y - Loc1.SpeedY - BeltSpeed > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
        tempFindCollisionBelt = 3;
    else
    {
        if(LevelWrap2[section] && !LevelSelect)
        {
            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
                tempFindCollisionBelt = 1;
            else if(Loc1.X - Loc1.SpeedX - BeltSpeed >= Loc2.X + Loc2.Width - Loc2.SpeedX - (level[section].Width - level[section].X))
                tempFindCollisionBelt = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX - (level[section].Width - level[section].X))
                tempFindCollisionBelt = 4;
            else if(Loc1.Y - Loc1.SpeedY - BeltSpeed > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
                tempFindCollisionBelt = 3;

            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY)
                tempFindCollisionBelt = 1;
            else if(Loc1.X - Loc1.SpeedX - BeltSpeed >= Loc2.X + Loc2.Width - Loc2.SpeedX + (level[section].Width - level[section].X))
                tempFindCollisionBelt = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX + (level[section].Width - level[section].X))
                tempFindCollisionBelt = 4;
            else if(Loc1.Y - Loc1.SpeedY - BeltSpeed > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
                tempFindCollisionBelt = 3;
        }
    }

    return tempFindCollisionBelt;
}

// Whats side the collision happened for NPCs
int NPCFindCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    int tempNPCFindCollision = 5;
    if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 4)
        tempNPCFindCollision = 1;
    else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
        tempNPCFindCollision = 2;
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
        tempNPCFindCollision = 4;
    else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
        tempNPCFindCollision = 3;
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 4)
            tempNPCFindCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX - (level[section].Width - level[section].X))
            tempNPCFindCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX - (level[section].Width - level[section].X))
            tempNPCFindCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
            tempNPCFindCollision = 3;
        else if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 4)
            tempNPCFindCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX + (level[section].Width - level[section].X))
            tempNPCFindCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX + (level[section].Width - level[section].X))
            tempNPCFindCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY > Loc2.Y + Loc2.Height - Loc2.SpeedY - 0.1)
            tempNPCFindCollision = 3;
    }

    return tempNPCFindCollision;
}

// Easy mode collision for jumping on NPCs
int EasyModeCollision(const Location_t &Loc1, const Location_t &Loc2, bool StandOn, int section)
{
    int tempEasyModeCollision = 5;
    if(!FreezeNPCs)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 10)
        {
            if(Loc1.SpeedY > Loc2.SpeedY || StandOn)
            {
                tempEasyModeCollision = 1;
            }
            else
            {
                tempEasyModeCollision = 0;
            }
        }
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
            tempEasyModeCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
            tempEasyModeCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
            tempEasyModeCollision = 3;
        else if(LevelWrap2[section] && !LevelSelect)
        {
            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 10)
            {
                if(Loc1.SpeedY > Loc2.SpeedY || StandOn)
                {
                    tempEasyModeCollision = 1;
                }
                else
                {
                    tempEasyModeCollision = 0;
                }
            }
            else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX - (level[section].Width - level[section].X))
                tempEasyModeCollision = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX - (level[section].Width - level[section].X))
                tempEasyModeCollision = 4;
            else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
                tempEasyModeCollision = 3;
            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 10)
            {
                if(Loc1.SpeedY > Loc2.SpeedY || StandOn)
                {
                    tempEasyModeCollision = 1;
                }
                else
                {
                    tempEasyModeCollision = 0;
                }
            }
            else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX + (level[section].Width - level[section].X))
                tempEasyModeCollision = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX + (level[section].Width - level[section].X))
                tempEasyModeCollision = 4;
            else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
                tempEasyModeCollision = 3;
        }
    }
    else
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 10)
            tempEasyModeCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width)
            tempEasyModeCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X)
            tempEasyModeCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
            tempEasyModeCollision = 3;
        else
        {
            if(LevelWrap2[section] && !LevelSelect)
            {
                if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 10)
                    tempEasyModeCollision = 1;
                else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - (level[section].Width - level[section].X))
                    tempEasyModeCollision = 2;
                else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - (level[section].Width - level[section].X))
                    tempEasyModeCollision = 4;
                else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
                    tempEasyModeCollision = 3;

                if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 10)
                    tempEasyModeCollision = 1;
                else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width + (level[section].Width - level[section].X))
                    tempEasyModeCollision = 2;
                else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X + (level[section].Width - level[section].X))
                    tempEasyModeCollision = 4;
                else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
                    tempEasyModeCollision = 3;
            }
        }
    }
    return tempEasyModeCollision;
}

// Easy mode collision for jumping on NPCs while on yoshi/boot
int BootCollision(const Location_t &Loc1, const Location_t &Loc2, bool StandOn, int section)
{
    int tempBootCollision = 5;
    if(FreezeNPCs == false)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 16)
        {
            if(Loc1.SpeedY > Loc2.SpeedY || StandOn == true)
            {
                tempBootCollision = 1;
            }
            else
            {
                tempBootCollision = 0;
            }
        }
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
            tempBootCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
            tempBootCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
            tempBootCollision = 3;
        else if(LevelWrap2[section] && !LevelSelect)
        {
            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 16)
            {
                if(Loc1.SpeedY > Loc2.SpeedY || StandOn == true)
                {
                    tempBootCollision = 1;
                }
                else
                {
                    tempBootCollision = 0;
                }
            }
            else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX - (level[section].Width - level[section].X))
                tempBootCollision = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX - (level[section].Width - level[section].X))
                tempBootCollision = 4;
            else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
                tempBootCollision = 3;
            else if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y - Loc2.SpeedY + 16)
            {
                if(Loc1.SpeedY > Loc2.SpeedY || StandOn == true)
                {
                    tempBootCollision = 1;
                }
                else
                {
                    tempBootCollision = 0;
                }
            }
            else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX + (level[section].Width - level[section].X))
                tempBootCollision = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX + (level[section].Width - level[section].X))
                tempBootCollision = 4;
            else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
                tempBootCollision = 3;
        }
    }
    else
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 16)
            tempBootCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width)
            tempBootCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X)
            tempBootCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
            tempBootCollision = 3;
        else if(LevelWrap2[section] && !LevelSelect)
        {
            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 10)
                tempBootCollision = 1;
            else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - (level[section].Width - level[section].X))
                tempBootCollision = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - (level[section].Width - level[section].X))
                tempBootCollision = 4;
            else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
                tempBootCollision = 3;
            if(Loc1.Y + Loc1.Height - Loc1.SpeedY <= Loc2.Y + 10)
                tempBootCollision = 1;
            else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width + (level[section].Width - level[section].X))
                tempBootCollision = 2;
            else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X + (level[section].Width - level[section].X))
                tempBootCollision = 4;
            else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height)
                tempBootCollision = 3;
        }
    }
    return tempBootCollision;

}

// Cursor collision
bool CursorCollision(const Location_t &Loc1, const Location_t &Loc2)
{
    bool tempCursorCollision = false;
    if(Loc1.X <= Loc2.X + Loc2.Width - 1)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X + 1)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height - 1)
            {
                if(Loc1.Y + Loc1.Height >= Loc2.Y + 1)
                {
                    tempCursorCollision = true;
                }
            }
        }
    }
    return tempCursorCollision;
}


// Shakey block collision
bool ShakeCollision(const Location_t &Loc1, const Location_t &Loc2, int ShakeY3, int section)
{
    bool tempShakeCollision = false;
    if(Loc1.X + 1 <= Loc2.X + Loc2.Width)
    {
        if(Loc1.X + Loc1.Width - 1 >= Loc2.X)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height + ShakeY3)
            {
                if(Loc1.Y + Loc1.Height >= Loc2.Y + ShakeY3)
                {
                    tempShakeCollision = true;
                    return tempShakeCollision;
                }
            }
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {

        if(Loc1.X + 1 <= Loc2.X + Loc2.Width - (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width - 1 >= Loc2.X - (level[section].Width - level[section].X))
            {
                if(Loc1.Y <= Loc2.Y + Loc2.Height + ShakeY3)
                {
                    if(Loc1.Y + Loc1.Height >= Loc2.Y + ShakeY3)
                    {
                        tempShakeCollision = true;
                        return tempShakeCollision;
                    }
                }
            }
        }
        else if(Loc1.X + 1 <= Loc2.X + Loc2.Width + (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width - 1 >= Loc2.X + (level[section].Width - level[section].X))
            {
                if(Loc1.Y <= Loc2.Y + Loc2.Height + ShakeY3)
                {
                    if(Loc1.Y + Loc1.Height >= Loc2.Y + ShakeY3)
                    {
                        tempShakeCollision = true;
                        return tempShakeCollision;
                    }
                }
            }
        }
    }
    return tempShakeCollision;
}

// vScreen collisions
bool vScreenCollision(int A, const Location_t &Loc2)
{
    bool tempvScreenCollision = false;
    if(A == 0)
    {
        return true;
    }
    if(-vScreenX[A] <= Loc2.X + Loc2.Width)
    {
        if(-vScreenX[A] + vScreen[A].Width >= Loc2.X)
        {
            if(-vScreenY[A] <= Loc2.Y + Loc2.Height)
            {
                if(-vScreenY[A] + vScreen[A].Height >= Loc2.Y)
                {
                    tempvScreenCollision = true;
                }
            }
        }
    }

    if(LevelWrap2[Player[A].Section])
    {
        if(-vScreenX[A] <= Loc2.X + Loc2.Width - (level[Player[A].Section].Width - level[Player[A].Section].X))
        {
            if(-vScreenX[A] + vScreen[A].Width >= Loc2.X - (level[Player[A].Section].Width - level[Player[A].Section].X))
            {
                if(-vScreenY[A] <= Loc2.Y + Loc2.Height)
                {
                    if(-vScreenY[A] + vScreen[A].Height >= Loc2.Y)
                    {
                        tempvScreenCollision = true;
                    }
                }
            }
        }
        if(-vScreenX[A] <= Loc2.X + Loc2.Width + (level[Player[A].Section].Width - level[Player[A].Section].X))
        {
            if(-vScreenX[A] + vScreen[A].Width >= Loc2.X + (level[Player[A].Section].Width - level[Player[A].Section].X))
            {
                if(-vScreenY[A] <= Loc2.Y + Loc2.Height)
                {
                    if(-vScreenY[A] + vScreen[A].Height >= Loc2.Y)
                    {
                        tempvScreenCollision = true;
                    }
                }
            }
        }
    }

    return tempvScreenCollision;

}

// vScreen collisions 2
bool vScreenCollision2(int A, const Location_t &Loc2)
{
    bool tempvScreenCollision2 = false;
    if(-vScreenX[A] + 64 <= Loc2.X + Loc2.Width)
    {
        if(-vScreenX[A] + vScreen[A].Width - 64 >= Loc2.X)
        {
            if(-vScreenY[A] + 96 <= Loc2.Y + Loc2.Height)
            {
                if(-vScreenY[A] + vScreen[A].Height - 64 >= Loc2.Y)
                {
                    tempvScreenCollision2 = true;
                }
            }
        }
    }
    if(LevelWrap2[Player[A].Section])
    {
        if(-vScreenX[A] + 64 <= Loc2.X + Loc2.Width - (level[Player[A].Section].Width - level[Player[A].Section].X))
        {
            if(-vScreenX[A] + vScreen[A].Width - 64 >= Loc2.X - (level[Player[A].Section].Width - level[Player[A].Section].X))
            {
                if(-vScreenY[A] + 96 <= Loc2.Y + Loc2.Height)
                {
                    if(-vScreenY[A] + vScreen[A].Height - 64 >= Loc2.Y)
                    {
                        tempvScreenCollision2 = true;
                    }
                }
            }
        }
        if(-vScreenX[A] + 64 <= Loc2.X + Loc2.Width + (level[Player[A].Section].Width - level[Player[A].Section].X))
        {
            if(-vScreenX[A] + vScreen[A].Width - 64 >= Loc2.X + (level[Player[A].Section].Width - level[Player[A].Section].X))
            {
                if(-vScreenY[A] + 96 <= Loc2.Y + Loc2.Height)
                {
                    if(-vScreenY[A] + vScreen[A].Height - 64 >= Loc2.Y)
                    {
                        tempvScreenCollision2 = true;
                    }
                }
            }
        }
    }
    return tempvScreenCollision2;
}

// Collision detection for blocks. Prevents walking on walls.
bool WalkingCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    bool tempWalkingCollision = false;
    if(Loc1.X <= Loc2.X + Loc2.Width + Loc1.SpeedX)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X + Loc1.SpeedX)
        {
            tempWalkingCollision = true;
            return tempWalkingCollision;
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.X <= Loc2.X + Loc2.Width + Loc1.SpeedX - (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X + Loc1.SpeedX - (level[section].Width - level[section].X))
            {
                tempWalkingCollision = true;
                return tempWalkingCollision;
            }
        }
        else if(Loc1.X <= Loc2.X + Loc2.Width + Loc1.SpeedX + (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X + Loc1.SpeedX + (level[section].Width - level[section].X))
            {
                tempWalkingCollision = true;
                return tempWalkingCollision;
            }
        }
    }
    return tempWalkingCollision;
}

// Collision detection for blocks. Lets NPCs fall through cracks.
bool WalkingCollision2(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    bool tempWalkingCollision2 = false;
    if(Loc1.X <= Loc2.X + Loc2.Width - Loc1.SpeedX - 1)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X - Loc1.SpeedX + 1)
        {
            tempWalkingCollision2 = true;
            return tempWalkingCollision2;
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.X <= Loc2.X + Loc2.Width - Loc1.SpeedX - 1 - (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X - Loc1.SpeedX + 1 - (level[section].Width - level[section].X))
            {
                tempWalkingCollision2 = true;
                return tempWalkingCollision2;
            }
        }
        else if(Loc1.X <= Loc2.X + Loc2.Width - Loc1.SpeedX - 1 + (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X - Loc1.SpeedX + 1 + (level[section].Width - level[section].X))
            {
                tempWalkingCollision2 = true;
                return tempWalkingCollision2;
            }
        }
    }
    return tempWalkingCollision2;
}

// Factors in beltspeed
bool WalkingCollision3(const Location_t &Loc1, const Location_t &Loc2, float BeltSpeed, int section)
{
    bool tempWalkingCollision3 = false;
    if(Loc1.X <= Loc2.X + Loc2.Width - (Loc1.SpeedX + BeltSpeed) - 1)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X - (Loc1.SpeedX + BeltSpeed) + 1)
        {
            tempWalkingCollision3 = true;
            return tempWalkingCollision3;
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.X <= Loc2.X + Loc2.Width - (Loc1.SpeedX + BeltSpeed) - 1 - (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X - (Loc1.SpeedX + BeltSpeed) + 1 - (level[section].Width - level[section].X))
            {
                tempWalkingCollision3 = true;
                return tempWalkingCollision3;
            }
        }
        else if(Loc1.X <= Loc2.X + Loc2.Width - (Loc1.SpeedX + BeltSpeed) - 1 + (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X - (Loc1.SpeedX + BeltSpeed) + 1 + (level[section].Width - level[section].X))
            {
                tempWalkingCollision3 = true;
                return tempWalkingCollision3;
            }
        }
    }
    return tempWalkingCollision3;
}

// Helps the player to walk over 1 unit cracks
int FindRunningCollision(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    int tempFindRunningCollision = 5;
    if(Loc1.Y + Loc1.Height - Loc1.SpeedY - 2.5 <= Loc2.Y - Loc2.SpeedY)
        tempFindRunningCollision = 1;
    else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX)
        tempFindRunningCollision = 2;
    else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX)
        tempFindRunningCollision = 4;
    else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
        tempFindRunningCollision = 3;
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY - 2.5 <= Loc2.Y - Loc2.SpeedY)
            tempFindRunningCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX - (level[section].Width - level[section].X))
            tempFindRunningCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX - (level[section].Width - level[section].X))
            tempFindRunningCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
            tempFindRunningCollision = 3;
        if(Loc1.Y + Loc1.Height - Loc1.SpeedY - 2.5 <= Loc2.Y - Loc2.SpeedY)
            tempFindRunningCollision = 1;
        else if(Loc1.X - Loc1.SpeedX >= Loc2.X + Loc2.Width - Loc2.SpeedX + (level[section].Width - level[section].X))
            tempFindRunningCollision = 2;
        else if(Loc1.X + Loc1.Width - Loc1.SpeedX <= Loc2.X - Loc2.SpeedX + (level[section].Width - level[section].X))
            tempFindRunningCollision = 4;
        else if(Loc1.Y - Loc1.SpeedY >= Loc2.Y + Loc2.Height - Loc2.SpeedY)
            tempFindRunningCollision = 3;
    }

    return tempFindRunningCollision;
}

// Determines if an NPC should turnaround
bool ShouldTurnAround(const Location_t &Loc1, const Location_t &Loc2, float Direction, int section)
{
    bool tempShouldTurnAround = true;
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
                        return tempShouldTurnAround;
                    }
                }
            }
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.Y + Loc1.Height + 8 <= Loc2.Y + Loc2.Height)
        {
            if(Loc1.Y + Loc1.Height + 8 >= Loc2.Y)
            {
                if(Loc1.X + Loc1.Width * 0.5 + (8 * Direction) <= Loc2.X + Loc2.Width - (level[section].Width - level[section].X))
                {
                    if(Loc1.X + Loc1.Width * 0.5 + (8 * Direction) >= Loc2.X - (level[section].Width - level[section].X))
                    {
                        if(Loc2.Y > Loc1.Y + Loc1.Height - 8)
                        {
                            tempShouldTurnAround = false;
                            return tempShouldTurnAround;
                        }
                    }
                }
            }
        }
        else if(Loc1.Y + Loc1.Height + 8 <= Loc2.Y + Loc2.Height)
        {
            if(Loc1.Y + Loc1.Height + 8 >= Loc2.Y)
            {
                if(Loc1.X + Loc1.Width * 0.5 + (8 * Direction) <= Loc2.X + Loc2.Width + (level[section].Width - level[section].X))
                {
                    if(Loc1.X + Loc1.Width * 0.5 + (8 * Direction) >= Loc2.X + (level[section].Width - level[section].X))
                    {
                        if(Loc2.Y > Loc1.Y + Loc1.Height - 8)
                        {
                            tempShouldTurnAround = false;
                            return tempShouldTurnAround;
                        }
                    }
                }
            }
        }
    }
    return tempShouldTurnAround;
}

// Determines if an NPC can come out of a pipe
bool CanComeOut(const Location_t &Loc1, const Location_t &Loc2, int section)
{
    bool tempCanComeOut = true;
    if(Loc1.X <= Loc2.X + Loc2.Width + 32)
    {
        if(Loc1.X + Loc1.Width >= Loc2.X - 32)
        {
            if(Loc1.Y <= Loc2.Y + Loc2.Height + 300)
            {
                if(Loc1.Y + Loc1.Height >= Loc2.Y - 300)
                {
                    tempCanComeOut = false;
                    return tempCanComeOut;
                }
            }
        }
    }
    else if(LevelWrap2[section] && !LevelSelect)
    {
        if(Loc1.X <= Loc2.X + Loc2.Width + 32 - (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X - 32 - (level[section].Width - level[section].X))
            {
                if(Loc1.Y <= Loc2.Y + Loc2.Height + 300)
                {
                    if(Loc1.Y + Loc1.Height >= Loc2.Y - 300)
                    {
                        tempCanComeOut = false;
                        return tempCanComeOut;
                    }
                }
            }
        }
        else if(Loc1.X <= Loc2.X + Loc2.Width + 32 + (level[section].Width - level[section].X))
        {
            if(Loc1.X + Loc1.Width >= Loc2.X - 32 + (level[section].Width - level[section].X))
            {
                if(Loc1.Y <= Loc2.Y + Loc2.Height + 300)
                {
                    if(Loc1.Y + Loc1.Height >= Loc2.Y - 300)
                    {
                        tempCanComeOut = false;
                        return tempCanComeOut;
                    }
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

