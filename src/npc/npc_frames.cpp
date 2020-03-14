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

#include "../globals.h"
#include "../npc.h"
#include "../sound.h"
#include "../collision.h"
#include "../effect.h"


void NPCFrames(int A)
{
    double B = 0;
    double C = 0;
    double D = 0;
    Location_t tempLocation;

    if(NPCFrame[NPC[A].Type] > 0) // custom frames
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPCFrameStyle[NPC[A].Type] == 2 && (NPC[A].Projectile != 0 || NPC[A].HoldingPlayer > 0))
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= NPCFrameSpeed[NPC[A].Type])
        {
            if(NPCFrameStyle[NPC[A].Type] == 0)
                NPC[A].Frame = NPC[A].Frame + 1 * NPC[A].Direction;
            else
                NPC[A].Frame = NPC[A].Frame + 1;
            NPC[A].FrameCount = 0;
        }
        if(NPCFrameStyle[NPC[A].Type] == 0)
        {
            if(NPC[A].Frame >= NPCFrame[NPC[A].Type])
                NPC[A].Frame = 0;
            if(NPC[A].Frame < 0)
                NPC[A].Frame = NPCFrame[NPC[A].Type] - 1;
        }
        else if(NPCFrameStyle[NPC[A].Type] == 1)
        {
            if(NPC[A].Direction == -1)
            {
                if(NPC[A].Frame >= NPCFrame[NPC[A].Type])
                    NPC[A].Frame = 0;
                if(NPC[A].Frame < 0)
                    NPC[A].Frame = NPCFrame[NPC[A].Type];
            }
            else
            {
                if(NPC[A].Frame >= NPCFrame[NPC[A].Type] * 2)
                    NPC[A].Frame = NPCFrame[NPC[A].Type];
                if(NPC[A].Frame < NPCFrame[NPC[A].Type])
                    NPC[A].Frame = NPCFrame[NPC[A].Type];
            }
        }
        else if(NPCFrameStyle[NPC[A].Type] == 2)
        {
            if(NPC[A].HoldingPlayer == 0 && NPC[A].Projectile == 0)
            {
                if(NPC[A].Direction == -1)
                {
                    if(NPC[A].Frame >= NPCFrame[NPC[A].Type])
                        NPC[A].Frame = 0;
                    if(NPC[A].Frame < 0)
                        NPC[A].Frame = NPCFrame[NPC[A].Type] - 1;
                }
                else
                {
                    if(NPC[A].Frame >= NPCFrame[NPC[A].Type] * 2)
                        NPC[A].Frame = NPCFrame[NPC[A].Type];
                    if(NPC[A].Frame < NPCFrame[NPC[A].Type])
                        NPC[A].Frame = NPCFrame[NPC[A].Type] * 2 - 1;
                }
            }
            else
            {
                if(NPC[A].Direction == -1)
                {
                    if(NPC[A].Frame >= NPCFrame[NPC[A].Type] * 3)
                        NPC[A].Frame = NPCFrame[NPC[A].Type] * 2;
                    if(NPC[A].Frame < NPCFrame[NPC[A].Type] * 2)
                        NPC[A].Frame = NPCFrame[NPC[A].Type] * 3 - 1;
                }
                else
                {
                    if(NPC[A].Frame >= NPCFrame[NPC[A].Type] * 4)
                        NPC[A].Frame = NPCFrame[NPC[A].Type] * 3;
                    if(NPC[A].Frame < NPCFrame[NPC[A].Type] * 3)
                        NPC[A].Frame = NPCFrame[NPC[A].Type] * 4 - 1;
                }
            }
        }
    }
    else if(NPC[A].Type == 231 || NPC[A].Type == 235 || NPC[A].Type == 86 || NPC[A].Type == 40 ||
            NPC[A].Type == 46 || NPC[A].Type == 212 || NPC[A].Type == 47 || NPC[A].Type == 284 ||
            NPC[A].Type == 58 || NPC[A].Type == 67 || NPC[A].Type == 68 || NPC[A].Type == 69 ||
            NPC[A].Type == 70 || NPC[A].Type == 73 || NPCIsVeggie[NPC[A].Type] || NPC[A].Type == 79 ||
            NPC[A].Type == 80 || NPC[A].Type == 82 || NPC[A].Type == 83 || NPC[A].Type == 104 ||
            NPC[A].Type == 105 || NPC[A].Type == 106 || NPC[A].Type == 133 || NPC[A].Type == 151 ||
            (NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 159 || NPC[A].Type == 192 ||
            NPC[A].Type == 197 || NPCIsAVine[NPC[A].Type] || NPC[A].Type == 237 || NPC[A].Type == 239 ||
            NPC[A].Type == 240 || NPC[A].Type == 250 || NPC[A].Type == 289 || NPC[A].Type == 290) // no frames
    {
        if(!(NPC[A].Type == 86 || NPC[A].Type == 284 || NPC[A].Type == 47) && A == 0) // Reset Frame to 0 unless a specific NPC type
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 169 || NPC[A].Type == 170)
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0)
            {
                if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) + std::abs(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - (Player[B].Location.Y + Player[B].Location.Height / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) + std::abs(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - (Player[B].Location.Y + Player[B].Location.Height / 2.0));
                    if(Player[B].Character == 5)
                        D = 1;
                    else
                        D = 0;
                }
            }
        }
        if(D != NPC[A].Frame)
        {
            if(NPC[A].FrameCount > 0)
                NewEffect(131, NPC[A].Location);
            NPC[A].Frame = D;
        }
        NPC[A].FrameCount = 1;
    }
    else if(NPC[A].Type == 278 || NPC[A].Type == 279) // fly block
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].HoldingPlayer > 0)
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Location.SpeedY != 0)
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 30)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 36)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Type == 279 && NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
    }
    else if(NPC[A].Type == 275) // fire plant thing
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 2;
            else
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 1;
            else
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
        else
            NPC[A].Frame = 3;
    }
    else if(NPC[A].Type == 288) // potion
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].Frame = NPC[A].Frame + 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Frame >= 4)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 283) // bubble
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 18)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 24)
            NPC[A].Frame = 2;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 272) // spider
    {
        if(NPC[A].Projectile != 0 || NPC[A].Location.SpeedY >= 0 || NPC[A].HoldingPlayer > 0)
            NPC[A].Frame = 0;
        else
            NPC[A].Frame = 2;
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount > 15)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount >= 8)
            NPC[A].Frame = NPC[A].Frame + 1;
    }
    else if(NPC[A].Type == 271) // bat thing
    {
        if(NPC[A].Special == 0)
            NPC[A].Frame = 0;
        else
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount > 15)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 8)
                NPC[A].Frame = 2;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 3;


    }
    else if(NPC[A].Type == 270) // jumping plant
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].Frame = NPC[A].Frame + 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Frame >= 4)
            NPC[A].Frame = 0;

    }
    else if(NPC[A].Type == 280) // ludwig koopa
    {
        if(NPC[A].Location.SpeedY != 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 10;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 11;
            else
            {
                NPC[A].Frame = 10;
                NPC[A].FrameCount = 0;
            }
        }
        else
        {
            if(NPC[A].Special == 0)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].Location.SpeedX == 0)
                    NPC[A].FrameCount = 10;
                if(NPC[A].FrameCount < 4)
                    NPC[A].Frame = 0;
                else if(NPC[A].FrameCount < 8)
                    NPC[A].Frame = 1;
                else if(NPC[A].FrameCount < 12)
                    NPC[A].Frame = 2;
                else
                {
                    NPC[A].Frame = 0;
                    NPC[A].FrameCount = 0;
                }
            }
            else if(NPC[A].Special == 1)
                NPC[A].Frame = 3;
            else if(NPC[A].Special == 2)
                NPC[A].Frame = 4;
            if(NPC[A].Direction == 1)
                NPC[A].Frame = NPC[A].Frame + 5;
        }



    }
    else if(NPC[A].Type == 281) // ludwig shell
    {
        if(NPC[A].Location.SpeedX == 0)
        {
            if(NPC[A].Frame > 2)
                NPC[A].Frame = 0;
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].Frame = NPC[A].Frame + NPC[A].Direction;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Frame < 0)
                NPC[A].Frame = 2;
            if(NPC[A].Frame > 2)
                NPC[A].Frame = 0;
        }

    }
    else if(NPC[A].Type == 282) // ludwig fire
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount > 8)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount >= 4)
            NPC[A].Frame = 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 2;

    }
    else if(NPC[A].Type == 269) // larry magic
    {
        if(NPC[A].Special == 0)
            NPC[A].Frame = 2;
        else if(NPC[A].Special == 1)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 268) // larry shell
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].Frame = NPC[A].Frame + NPC[A].Direction;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Frame < 0)
            NPC[A].Frame = 5;
        if(NPC[A].Frame > 5)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 267) // larry koopa
    {

        NPC[A].Frame = 0;
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY == 0)
            {
                if(NPC[A].Location.SpeedX == 0)
                    NPC[A].Frame = 0;
                else
                {
                    NPC[A].FrameCount = NPC[A].FrameCount + 1;
                    if(NPC[A].FrameCount < 8)
                        NPC[A].Frame = 0;
                    else if(NPC[A].FrameCount < 16)
                        NPC[A].Frame = 1;
                    else
                    {
                        NPC[A].Frame = 0;
                        NPC[A].FrameCount = 0;
                    }
                }
            }
            else
                NPC[A].Frame = 1;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 2)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 3;
            else if(NPC[A].FrameCount < 6)
                NPC[A].Frame = 4;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 5;
            else
            {
                NPC[A].Frame = 2;
                NPC[A].FrameCount = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 2)
                NPC[A].Frame = 6;
            else if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 7;
            else if(NPC[A].FrameCount < 6)
                NPC[A].Frame = 8;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 9;
            else
            {
                NPC[A].Frame = 6;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 10;


    }
    else if(NPC[A].Type == 266) // sword beam
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 4;
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 2)
        {
        }
        else if(NPC[A].FrameCount < 4)
            NPC[A].Frame = NPC[A].Frame + 1;
        else if(NPC[A].FrameCount < 6)
            NPC[A].Frame = NPC[A].Frame + 2;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = NPC[A].Frame + 3;
        else
            NPC[A].FrameCount = 0;


    }
    else if(NPC[A].Type == 262) // mouser
    {
        if(NPC[A].Immune > 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 3;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 4;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 5;
            else if(NPC[A].FrameCount < 15)
                NPC[A].Frame = 6;
            else
            {
                NPC[A].Frame = 6;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Direction == 1)
                NPC[A].Frame = NPC[A].Frame + 7;
        }
        else if(NPC[A].Special <= 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 15)
                NPC[A].Frame = 2;
            else
            {
                NPC[A].Frame = 2;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Direction == 1)
                NPC[A].Frame = NPC[A].Frame + 7;
        }
        else
        {
            NPC[A].Frame = 0;
            if(NPC[A].Direction == 1)
                NPC[A].Frame = NPC[A].Frame + 7;
        }

    }
    else if(NPC[A].Type == 261)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 15)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 1;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
        if(NPC[A].Special > 0 && NPC[A].Location.SpeedY <= 0)
            NPC[A].Frame = NPC[A].Frame + 2;

    }
    else if(NPC[A].Type == 260)
    {
        if(NPC[A].Direction == 1)
            NPC[A].Frame = SpecialFrame[2];
        else
            NPC[A].Frame = 3 - SpecialFrame[2];
    }
    else if(NPC[A].Type == 255)
    {
        // NPC has no frames so do nothing
    }
    else if(NPC[A].Type == 259)
    {
        NPC[A].Frame = NPC[A].Frame + 1;
        if(NPC[A].Frame >= 5)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 251 || NPC[A].Type == 252 || NPC[A].Type == 253)
        NPC[A].Frame = SpecialFrame[8];
    else if(NPC[A].Type == 238)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
        }
        if(NPC[A].Frame >= 3)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 247)
    {
        if(NPC[A].Special < 0 && NPC[A].Location.SpeedY == 0)
            NPC[A].Special = NPC[A].Special + 1;
        if(NPC[A].Projectile != 0 || NPC[A].HoldingPlayer > 0)
            NPC[A].Frame = 4;
        else
        {
            if(NPC[A].Special < 0)
                NPC[A].Frame = 1;
            else
                NPC[A].Frame = 0;
        }
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount > 8)
        {
            if(NPC[A].Projectile != 0 || NPC[A].HoldingPlayer > 0)
                NPC[A].Frame = NPC[A].Frame + 1;
            else
                NPC[A].Frame = NPC[A].Frame + 2;
        }
    }
    else if(NPC[A].Type == 245)
    {
        NPC[A].Frame = 0;
        if(Player[NPC[A].Special4].Location.X + Player[NPC[A].Special4].Location.Width / 2.0 > NPC[A].Location.X + NPC[A].Location.Width / 2.0)
            NPC[A].Frame = 2;
        if(Player[NPC[A].Special4].Location.Y + Player[NPC[A].Special4].Location.Height / 2.0 < NPC[A].Location.Y + 16)
            NPC[A].Frame = NPC[A].Frame + 1;
    }
    else if(NPC[A].Type == 243)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 241) // POW block
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame >= 7)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 81) // 1 frame left or right
    {
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 211)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 30)
            NPC[A].Frame = 4;
        else if(NPC[A].FrameCount <= 36)
            NPC[A].Frame = 5;
        else if(NPC[A].FrameCount <= 42)
            NPC[A].Frame = 4;
        else if(NPC[A].FrameCount <= 48)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 54)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 60)
            NPC[A].Frame = 1;
        else
            NPC[A].FrameCount = 0;
    }
    else if(NPC[A].Type == 210)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount <= 8)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 16)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 32)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 38)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 46)
            NPC[A].Frame = 1;
        else
            NPC[A].FrameCount = 0;
    }
    else if(NPC[A].Type == 209)
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special > 0 && NPC[A].Special < 15)
            NPC[A].Frame = 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 2;
    }
    else if(NPC[A].Type == 208)
    {
        if(NPC[A].Damage < 3)
            NPC[A].Frame = 0;
        else if(NPC[A].Damage < 6)
            NPC[A].Frame = 1;
        else if(NPC[A].Damage < 9)
            NPC[A].Frame = 2;
        else if(NPC[A].Damage < 12)
            NPC[A].Frame = 3;
        else
            NPC[A].Frame = 4;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 5;
    }
    else if(NPC[A].Type == 207)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount > 8)
            NPC[A].Frame = 1;
        if(NPC[A].Special == 4)
            NPC[A].Frame = NPC[A].Frame + 4;
        else if(NPC[A].Special == 3)
            NPC[A].Frame = NPC[A].Frame + 8;
        else if(NPC[A].Special == 2)
            NPC[A].Frame = NPC[A].Frame + 12;
        if(NPC[A].Special2 == 1)
            NPC[A].Frame = NPC[A].Frame + 2;


    }
    else if(NPC[A].Type == 205)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 30)
            NPC[A].Frame = 4;
        else
            NPC[A].FrameCount = 0;
        if(NPC[A].Special == 4)
            NPC[A].Frame = NPC[A].Frame + 5;
        else if(NPC[A].Special == 3)
            NPC[A].Frame = NPC[A].Frame + 10;
        else if(NPC[A].Special == 2)
            NPC[A].Frame = NPC[A].Frame + 15;


    }
    else if(NPC[A].Type == 203 || NPC[A].Type == 204)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 1;
        else
            NPC[A].FrameCount = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
    }
    else if(NPC[A].Type == 201)
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 1;
            else
                NPC[A].FrameCount = 0;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 2;

        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 3;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 4;
            else
            {
                NPC[A].Frame = 3;
                NPC[A].FrameCount = 0;
            }
        }

        if(NPC[A].Special == 3 || NPC[A].Special == 2)
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 5;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 6;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 7;
            else
            {
                NPC[A].Frame = 7;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 8;
    }
    else if(NPC[A].Type == 200) // King Koopa
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount <= 8)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount <= 16)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount <= 24)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount <= 32)
                NPC[A].Frame = 0;
            else
                NPC[A].FrameCount = 0;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 3;
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 4;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 5;
    }
    else if(NPC[A].Type == 196)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 180)
    {
        NPC[A].Frame = 0;
        C = 0;
        for(B = 1; B <= numPlayers; ++B)
        {
            if(!CanComeOut(NPC[A].Location, Player[B].Location) && Player[B].Location.Y >= NPC[A].Location.Y)
                C = B;
        }
        if(C > 0)
            NPC[A].Frame = 2;
        else
        {
            for(B = 1; B <= numPlayers; ++B)
            {
                tempLocation = NPC[A].Location;
                tempLocation.Width = NPC[A].Location.Width * 2;
                tempLocation.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                if(!CanComeOut(tempLocation, Player[B].Location) && Player[B].Location.Y >= NPC[A].Location.Y)
                    C = B;
            }
            if(C > 0)
                NPC[A].Frame = 1;
        }
        if(NPC[A].Special == 1)
            NPC[A].Frame = 2;

    }
    else if(NPC[A].Type == 292) // toad boomerang
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 6)
        {
            NPC[A].FrameCount = 0;

            if(NPC[A].Location.SpeedX > 0)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame == 1)
                    NPC[A].Location.X = NPC[A].Location.X + 4;
                else if(NPC[A].Frame == 3)
                    NPC[A].Location.X = NPC[A].Location.X - 4;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.Y = NPC[A].Location.Y + 4;
                else
                    NPC[A].Location.Y = NPC[A].Location.Y - 4;
            }
            else
            {
                NPC[A].Frame = NPC[A].Frame - 1;
                if(NPC[A].Frame == 0)
                    NPC[A].Location.X = NPC[A].Location.X - 4;
                else if(NPC[A].Frame == 1)
                    NPC[A].Location.Y = NPC[A].Location.Y - 4;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.X = NPC[A].Location.X + 4;
                else
                    NPC[A].Location.Y = NPC[A].Location.Y + 4;
            }

            if(NPC[A].Frame > 3)
                NPC[A].Frame = 0;
            else if(NPC[A].Frame < 0)
                NPC[A].Frame = 3;
        }

        if(iRand() % 4 == 0)
        {
            NewEffect(80, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 4, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 4), 1, 0, NPC[A].Shadow);
            Effect[numEffects].Location.SpeedX = dRand() * 1 - 0.5;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }



    }
    else if(NPC[A].Type == 171) // Mario Hammer
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Location.SpeedX > 0)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame == 1)
                    NPC[A].Location.X = NPC[A].Location.X + 8;
                else if(NPC[A].Frame == 3)
                    NPC[A].Location.X = NPC[A].Location.X - 8;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.Y = NPC[A].Location.Y + 12;
                else
                    NPC[A].Location.Y = NPC[A].Location.Y - 12;
            }
            else
            {
                NPC[A].Frame = NPC[A].Frame - 1;
                if(NPC[A].Frame == 0)
                    NPC[A].Location.X = NPC[A].Location.X - 8;
                else if(NPC[A].Frame == 1)
                    NPC[A].Location.Y = NPC[A].Location.Y - 12;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.X = NPC[A].Location.X + 8;
                else
                    NPC[A].Location.Y = NPC[A].Location.Y + 12;
            }
            if(NPC[A].Frame > 3)
                NPC[A].Frame = 0;
            else if(NPC[A].Frame < 0)
                NPC[A].Frame = 3;
            NewEffect(80, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 8, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 8), 1, 0, NPC[A].Shadow);
            Effect[numEffects].Location.SpeedX = dRand() * 1 - 0.5;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }


    }
    else if(NPC[A].Type == 167) // smw paragoomba
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 4;
        else
            NPC[A].Frame = 0;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount >= 8)
            NPC[A].Frame = NPC[A].Frame + 1;

        if(NPC[A].Effect == 0)
        {
            if(NPC[A].Special == 0)
                NPC[A].Special2 = NPC[A].Special2 + 2;
            else if(NPC[A].Special <= 60)
                NPC[A].Special2 = 0;
            else if(NPC[A].Special < 65)
                NPC[A].Special2 = NPC[A].Special2 + 1;
            else
                NPC[A].Special2 = NPC[A].Special2 + 2;
            if(NPC[A].Special2 >= 16)
                NPC[A].Special2 = 0;
            else if(NPC[A].Special2 >= 8)
                NPC[A].Frame = NPC[A].Frame + 2;
        }
    }
    else if(NPC[A].Type == 3 || NPC[A].Type == 244) // Flying Goomba
    {
        if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
            }
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Frame == 0)
                    NPC[A].Frame = 2;
                else if(NPC[A].Frame == 1)
                    NPC[A].Frame = 3;
                else if(NPC[A].Frame == 2)
                    NPC[A].Frame = 1;
                else if(NPC[A].Frame == 3)
                    NPC[A].Frame = 0;
            }
        }
    }
    else if(NPC[A].Type == 134) // bomb
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 11)
            NPC[A].Frame = 2;
        else
            NPC[A].FrameCount = 0;
        if(NPC[A].Special2 == 1)
        {
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 < 4)
            {
            }
            else if(NPC[A].Special3 < 8)
                NPC[A].Frame = NPC[A].Frame + 9;
            else if(NPC[A].Special3 < 12)
                NPC[A].Frame = NPC[A].Frame + 3;
            else if(NPC[A].Special3 < 15)
                NPC[A].Frame = NPC[A].Frame + 6;
            else
                NPC[A].Special3 = 0;
        }
    }
    else if(NPC[A].Type == 291) // heart bomb
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 11)
            NPC[A].Frame = 2;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
        NPC[A].Special3 = NPC[A].Special3 + 1;
        if(NPC[A].Special3 < 4)
        {}
        else if(NPC[A].Special3 < 8)
            NPC[A].Frame = NPC[A].Frame + 3;
        else if(NPC[A].Special3 < 12)
            NPC[A].Frame = NPC[A].Frame + 6;
        else // If .Special3 >= 16 Then
            NPC[A].Special3 = 0;
        if(fRand() * 10 > 9.2)
        {
            NewEffect(80, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 4, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 6), 1, 0, NPC[A].Shadow);
            Effect[numEffects].Location.SpeedX = std::fmod(fRand(), 1.0) - 0.5;
            Effect[numEffects].Location.SpeedY = std::fmod(fRand(), 1.0) - 0.5;
        }
    }
    else if(NPC[A].Type == 91)
        NPC[A].Frame = SpecialFrame[5];
    else if(NPC[A].Type == 96)
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special == 98)
            NPC[A].Frame = 1;
        else if(NPC[A].Special == 99)
            NPC[A].Frame = 2;
        else if(NPC[A].Special == 100)
            NPC[A].Frame = 3;
        else if(NPC[A].Special == 148)
            NPC[A].Frame = 4;
        else if(NPC[A].Special == 149)
            NPC[A].Frame = 5;
        else if(NPC[A].Special == 150)
            NPC[A].Frame = 6;
        else if(NPC[A].Special == 228)
            NPC[A].Frame = 7;
    }
    else if(NPC[A].Type == 194 || NPC[A].Type == 195) // Glowy Shell
    {
        NPC[A].Special5 = NPC[A].Special5 + 1;
        if(NPC[A].Special5 >= 16)
            NPC[A].Special5 = 0;
        if(NPC[A].Location.SpeedX > 0)
        {
            if(NPC[A].Type == 194)
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
            else
                NPC[A].FrameCount = NPC[A].FrameCount - 1;
        }
        else if(NPC[A].Location.SpeedX < 0)
        {
            if(NPC[A].Type == 194)
                NPC[A].FrameCount = NPC[A].FrameCount - 1;
            else
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
        }
        else
        {
            if(NPC[A].Type == 194)
                NPC[A].FrameCount = 0;
            else
                NPC[A].FrameCount = 12;
        }
        if(NPC[A].FrameCount < 0)
            NPC[A].FrameCount = 15;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 12)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount < 16)
            NPC[A].Frame = 3;
        if(NPC[A].Special5 < 4)
        {
        }
        else if(NPC[A].Special5 < 8)
            NPC[A].Frame = NPC[A].Frame + 4;
        else if(NPC[A].Special5 < 12)
            NPC[A].Frame = NPC[A].Frame + 8;
        else if(NPC[A].Special5 < 16)
            NPC[A].Frame = NPC[A].Frame + 12;
    }
    else if(NPCIsAShell[NPC[A].Type]) // Turtle shell
    {
        if(NPC[A].Location.SpeedX == 0)
            NPC[A].Frame = 0;
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 0;
            }
        }
    }
    else if(NPC[A].Type == 77) // black ninja
    {
        if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 12)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 6)
                NPC[A].Frame = 1;
        }
        else if(NPC[A].Location.SpeedY < 0)
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount = 6;
        }
        else
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 2;
    }
    else if(NPC[A].Type == 57) // smb3 belt
    {
        if(NPC[A].Direction == -1)
            NPC[A].Frame = SpecialFrame[4];
        else
            NPC[A].Frame = 3 - SpecialFrame[4];
    }
    else if(NPC[A].Type == 60 || NPC[A].Type == 62 || NPC[A].Type == 64 || NPC[A].Type == 66)
    {
        NPC[A].Frame = 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 168) // Bully
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 3;

        if(NPC[A].Projectile != 0 || NPC[A].Special2 != 0)
        {
            NPC[A].Frame = NPC[A].Frame + 2;
            NPC[A].FrameCount = 0;
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 16)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 8)
                NPC[A].Frame = NPC[A].Frame + 1;
        }


    }
    else if(NPC[A].Type == 78) // tank treads
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount >= 4)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
        if(NPC[A].FrameCount > 12)
            NPC[A].FrameCount = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 3;
    }
    else if(NPC[A].Type == 55) // nekkid koopa
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].Frame = 0;
            if(NPC[A].Direction == 1)
                NPC[A].Frame = 3;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 15)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 8)
                NPC[A].Frame = NPC[A].Frame + 1;
        }
        else
        {
            if(NPC[A].Direction == -1)
                NPC[A].Frame = 2;
            else
                NPC[A].Frame = 5;
        }
    }
    else if(NPC[A].Type >= 117 && NPC[A].Type <= 120) // beach koopa
    {
        if(NPC[A].Projectile != 0)
        {
            if(NPC[A].Location.SpeedX < -0.5 || NPC[A].Location.SpeedX > 0.5)
                NPC[A].Frame = 3;
            else
            {
                NPC[A].Frame = 3;
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 15)
                    NPC[A].FrameCount = 0;
                else if(NPC[A].FrameCount >= 8)
                    NPC[A].Frame = 4;
            }
        }
        else
        {
            if(NPC[A].Special == 0)
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 15)
                    NPC[A].FrameCount = 0;
                else if(NPC[A].FrameCount >= 8)
                    NPC[A].Frame = 1;
            }
            else
                NPC[A].Frame = 2;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 5;
    }
    else if(NPC[A].Type == 54) // bouncy bee
    {
        if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 3)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
    }
    else if(NPC[A].Type == 56)
    {
        NPC[A].Frame = SpecialFrame[2];
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
    }
    else if(NPC[A].Type == 45) // ice block
    {
        if(NPC[A].Special == 0)
            NPC[A].Frame = BlockFrame[4];
        else
        {
            if(NPC[A].Frame < 4)
                NPC[A].Frame = 4;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 6)
                    NPC[A].Frame = 4;
            }
        }
        // bowser fireball
    }
    else if(NPC[A].Type == 87)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 20)
            NPC[A].FrameCount = 0;
        NPC[A].Frame = static_cast<int>(floor(static_cast<double>(NPC[A].FrameCount / 5)));
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
        // statue fireball
    }
    else if(NPC[A].Type == 85)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
            NPC[A].FrameCount = 0;
        NPC[A].Frame = static_cast<int>(floor(static_cast<double>(NPC[A].FrameCount / 2)));
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
        // winged koopa
    }
    else if(NPC[A].Type == 76 || NPC[A].Type == 161)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Direction == -1 && NPC[A].Frame >= 4)
            NPC[A].Frame = 0;
        else if(NPC[A].Direction == 1 && NPC[A].Frame < 4)
            NPC[A].Frame = 4;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Direction == -1)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 8)
                    NPC[A].Frame = 4;
            }
        }
    }
    else if(NPC[A].Type == 137) // SMB3 Bomb
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 15)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 6;
        if(NPC[A].Special2 == 1)
        {
            NPC[A].Special3 = NPC[A].Special3 + 1;
            if(NPC[A].Special3 < 4)
            {
            }
            else if(NPC[A].Special3 < 8)
                NPC[A].Frame = NPC[A].Frame + 2;
            else if(NPC[A].Special3 < 11)
                NPC[A].Frame = NPC[A].Frame + 4;
            else
            {
                NPC[A].Frame = NPC[A].Frame + 4;
                NPC[A].Special3 = 0;
            }
        }
    }
    else if(NPC[A].Type == 160) // Airship Jet
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Direction == -1 && NPC[A].Frame >= 4)
            NPC[A].Frame = 0;
        else if(NPC[A].Direction == 1 && NPC[A].Frame < 4)
            NPC[A].Frame = 8;
        if(NPC[A].FrameCount >= 2)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Direction == -1)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 8)
                    NPC[A].Frame = 4;
            }
        }
    }
    else if(NPC[A].Type == 178)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame >= 3)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 4 || NPC[A].Type == 6 || NPC[A].Type == 23 || NPC[A].Type == 36 || NPC[A].Type == 285 || NPC[A].Type == 42 || NPC[A].Type == 52 || NPC[A].Type == 72 || (NPC[A].Type >= 109 && NPC[A].Type <= 112) || (NPC[A].Type >= 121 && NPC[A].Type <= 124) || NPC[A].Type == 136 || NPC[A].Type == 159 || NPC[A].Type == 162 || NPC[A].Type == 163 || NPC[A].Type == 164 || NPC[A].Type == 165 || NPC[A].Type == 166 || NPC[A].Type == 173 || NPC[A].Type == 175 || NPC[A].Type == 176 || NPC[A].Type == 177 || NPC[A].Type == 199 || NPC[A].Type == 229 || NPC[A].Type == 236 || NPC[A].Type == 230 || NPC[A].Type == 232 || NPC[A].Type == 233) // Walking koopa troopa / hard thing / spiney
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Type == 166 && NPC[A].Special > 360)
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
            NPC[A].Frame = 0;
        else if(NPC[A].Direction == 1 && NPC[A].Frame < 2)
            NPC[A].Frame = 2;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Direction == -1)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 2;
            }
        }

    }
    else if(NPC[A].Type == 234)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 3;

        if(NPC[A].FrameCount > 8)
            NPC[A].Frame = NPC[A].Frame + 1;

        if(NPC[A].FrameCount > 16)
            NPC[A].Frame = NPC[A].Frame + 1;
        if(NPC[A].FrameCount > 24)
            NPC[A].Frame = NPC[A].Frame - 1;
        if(NPC[A].FrameCount > 32)
            NPC[A].FrameCount = 0;

    }
    else if(NPC[A].Type == 189) // dry bones
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].Type == 166 && NPC[A].Special > 360)
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 2)
                NPC[A].Frame = 2;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 2)
                        NPC[A].Frame = 0;
                }
                else
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 4)
                        NPC[A].Frame = 2;
                }
            }
        }
        else
        {
            if(NPC[A].Special2 < 10 || NPC[A].Special2 > 400 - 10)
                NPC[A].Frame = 4;
            else
                NPC[A].Frame = 5;
            if(NPC[A].Direction == 1)
                NPC[A].Frame = NPC[A].Frame + 2;
        }
    }
    else if(NPC[A].Type == 274) // dragon coin
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount < 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount < 30)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount < 36)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
    }
    else if(NPCIsACoin[NPC[A].Type]) // Coin
    {
        NPC[A].Frame = CoinFrame[3];
        if(NPC[A].Type == 138)
            NPC[A].Frame = CoinFrame[2];
        if(NPC[A].Type == 152)
            NPC[A].Frame = CoinFrame[3];
    }
    else if(NPC[A].Type == 11) // Frame finder for Star/Flower/Mushroom Exit
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame == 3)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 50) // killer plant
    {
        // .standingOnPlayer = A
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 2;
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
            NPC[A].Frame = NPC[A].Frame + 1;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
    }
    else if(NPC[A].Type == 49) // killer pipe
    {
        if(NPC[A].HoldingPlayer == 0 && !Player[NPC[A].standingOnPlayer].Controls.Run && NPC[A].Projectile == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].Frame = NPC[A].Frame + 1;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Frame >= 5)
                NPC[A].Frame = 0;
        }
        else
        {
            if(NPC[A].Direction == -1)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 4)
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    NPC[A].FrameCount = 0;
                }
                if(NPC[A].Frame >= 10 || NPC[A].Frame < 5)
                    NPC[A].Frame = 5;
            }
            else
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount >= 4)
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    NPC[A].FrameCount = 0;
                }
                if(NPC[A].Frame >= 15 || NPC[A].Frame < 10)
                    NPC[A].Frame = 10;
            }
        }
    }
    else if(NPC[A].Type == 12) // Frame finder for big fireball
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Location.SpeedY < 0)
            {
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
            }
            else
            {
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 2;
            }
        }
    }
    else if(NPC[A].Type == 13 || NPC[A].Type == 30 || NPC[A].Type == 246 || NPC[A].Type == 265) // Frame finder for Fireball / Hammer
    {
        if((NPC[A].Type == 13 || NPC[A].Type == 265) && NPC[A].Quicksand == 0)
        {
            if(NPC[A].Wet > 0)
            {
                if(dRand() * 100 > 95)
                {
                    tempLocation = newLoc(NPC[A].Location.X + 4, NPC[A].Location.Y + 4, 8, 8);
                    if(!UnderWater[NPC[A].Section])
                        NewEffect(113, tempLocation, 1, 0, NPC[A].Shadow);
                    else
                        NewEffect(113, tempLocation, 1, 1, NPC[A].Shadow);
                }

                if(dRand() * 100 > 85)
                {
                    if(NPC[A].Type == 265)
                    {
                        if(NPC[A].Special == 5)
                        {
                            NewEffect(139, NPC[A].Location, 1, 0, NPC[A].Shadow);
                            if(dRand() * 10 > 8)
                            {
                                tempLocation.Height = EffectHeight[80];
                                tempLocation.Width = EffectWidth[80];
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.X = NPC[A].Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4 - NPC[A].Location.SpeedX * 3;
                                tempLocation.Y = NPC[A].Location.Y + dRand() * 16 - EffectHeight[80] / 2.0 - 4;
                                NewEffect(80, tempLocation);
                                Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.5;
                                Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.5;
                                Effect[numEffects].Frame = iRand() % 3;
                            }
                        }
                        else if(dRand() * 10 > 6)
                        {
                            tempLocation.Height = EffectHeight[80];
                            tempLocation.Width = EffectWidth[80];
                            tempLocation.SpeedX = 0;
                            tempLocation.SpeedY = 0;
                            tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                            tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                            NewEffect(80, tempLocation, 1, 0, NPC[A].Shadow);
                            Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.25;
                            Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.25;
                            Effect[numEffects].Frame = iRand() % 3;
                        }
                    }
                    else
                        NewEffect(77, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
                }
            }
            else
            {
                if(NPC[A].Type == 265)
                {
                    if(NPC[A].Special == 5)
                    {
                        NewEffect(139, NPC[A].Location, 1, 0, NPC[A].Shadow);
                        if(dRand() * 10 > 8)
                        {
                            tempLocation.Height = EffectHeight[80];
                            tempLocation.Width = EffectWidth[80];
                            tempLocation.SpeedX = 0;
                            tempLocation.SpeedY = 0;
                            tempLocation.X = NPC[A].Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4 - NPC[A].Location.SpeedX * 3;
                            tempLocation.Y = NPC[A].Location.Y + dRand() * 16 - EffectHeight[80] / 2.0 - 4;
                            NewEffect(80, tempLocation);
                            Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.5;
                            Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.5;
                            Effect[numEffects].Frame = iRand() % 3;
                        }
                    }
                    else if(dRand() * 10 > 6)
                    {
                        tempLocation.Height = EffectHeight[80];
                        tempLocation.Width = EffectWidth[80];
                        tempLocation.SpeedX = 0;
                        tempLocation.SpeedY = 0;
                        tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                        tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                        NewEffect(80, tempLocation, 1, 0, NPC[A].Shadow);
                        Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.25;
                        Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.25;
                        Effect[numEffects].Frame = iRand() % 3;
                    }
                }
                else
                    NewEffect(77, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
            }
        }
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = NPC[A].Frame - NPC[A].Direction;
        }
        if(NPC[A].Special < 2 || (NPC[A].Type == 265 && NPC[A].Special != 5))
        {
            if(NPC[A].Frame >= 4)
                NPC[A].Frame = 0;
            if(NPC[A].Frame < 0)
                NPC[A].Frame = 3;
        }
        else if(NPC[A].Special == 2 || (NPC[A].Type == 265 && NPC[A].Special == 5))
        {
            if(NPC[A].Frame >= 7)
                NPC[A].Frame = 4;
            if(NPC[A].Frame < 4)
                NPC[A].Frame = 6;
        }
        else if(NPC[A].Special == 3)
        {
            if(NPC[A].Frame >= 11)
                NPC[A].Frame = 8;
            if(NPC[A].Frame < 8)
                NPC[A].Frame = 10;
        }
        else if(NPC[A].Special == 4)
        {
            if(NPC[A].Frame >= 15)
                NPC[A].Frame = 12;
            if(NPC[A].Frame < 12)
                NPC[A].Frame = 14;
        }
        else if(NPC[A].Special == 5)
        {
            if(NPC[A].Frame >= 19)
                NPC[A].Frame = 16;
            if(NPC[A].Frame < 16)
                NPC[A].Frame = 18;
        }
    }
    else if(NPC[A].Type == 15) // Frame finder for Big Koopa
    {
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY != 0)
                NPC[A].Frame = 0;
            else
            {
                if(NPC[A].FrameCount >= 0)
                    NPC[A].FrameCount = NPC[A].FrameCount + 1;
                else
                    NPC[A].FrameCount = NPC[A].FrameCount - 1;
                if(NPC[A].FrameCount >= 5 || NPC[A].FrameCount <= -5)
                {
                    if(NPC[A].FrameCount >= 0)
                    {
                        NPC[A].Frame = NPC[A].Frame + 1;
                        NPC[A].FrameCount = 1;
                    }
                    else
                    {
                        NPC[A].Frame = NPC[A].Frame - 1;
                        NPC[A].FrameCount = -1;
                    }
                    if(NPC[A].Frame >= 5)
                    {
                        NPC[A].Frame = 3;
                        NPC[A].FrameCount = -1;
                    }
                    else if(NPC[A].Frame <= 0)
                    {
                        NPC[A].Frame = 2;
                        NPC[A].FrameCount = 1;
                    }
                }
            }
        }
        else if(NPC[A].Special == 1)
            NPC[A].Frame = 6;
        else if(NPC[A].Special == 4)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].Frame < 7)
                NPC[A].Frame = 7;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Frame == 7)
                    NPC[A].Frame = 8;
                else
                    NPC[A].Frame = 7;
            }
        }
        else
            NPC[A].Frame = 5;
    }
    else if(NPC[A].Type == 37 || NPC[A].Type == 180) // Thwomp
    {
        // Bullet Bills / Key / ONLY DIRECTION FRAMES
    }
    else if(NPC[A].Type == 17 || NPC[A].Type == 18 || NPC[A].Type == 31 || NPC[A].Type == 84 || NPC[A].Type == 94 || NPC[A].Type == 198 || NPCIsYoshi[NPC[A].Type] || NPC[A].Type == 101 || NPC[A].Type == 102 || NPC[A].Type == 181)
    {
        if(NPC[A].Direction == -1)
            NPC[A].Frame = 0;
        else
            NPC[A].Frame = 1;
        // Leaf
    }
    else if(NPC[A].Type == 34)
    {
        if(NPC[A].Direction == -1)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == 135 && NPC[A].Special2 == 1)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 8;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 9;
        else if(NPC[A].FrameCount < 11)
            NPC[A].Frame = 10;
        else
        {
            NPC[A].Frame = 10;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].HoldingPlayer > 0 || NPC[A].Projectile != 0)
            NPC[A].Frame = NPC[A].Frame + 6;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 3;
    }
    else if(NPC[A].Type == 19 || NPC[A].Type == 20 || NPC[A].Type == 28 || (NPC[A].Type >= 129 && NPC[A].Type <= 132) || NPC[A].Type == 135 || NPC[A].Type == 158) // Shy guys / Jumping Fish
    {
        if(NPC[A].HoldingPlayer == 0 && NPC[A].Projectile == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 2)
                NPC[A].Frame = 2;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 2)
                        NPC[A].Frame = 0;
                }
                else
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 4)
                        NPC[A].Frame = 2;
                }
            }
        }
        else
        {
            if(NPC[A].Frame < 4)
                NPC[A].Frame = 4;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 6)
                NPC[A].Frame = 4;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 6)
                NPC[A].Frame = 6;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 6)
                        NPC[A].Frame = 4;
                }
                else
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 8)
                        NPC[A].Frame = 6;
                }
            }
        }
    }
    else if(NPC[A].Type == 25) // Bouncy Star things
    {
        if(NPC[A].HoldingPlayer == 0 && NPC[A].Projectile == 0)
        {
            if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 0;
                else if(NPC[A].Direction == 1)
                    NPC[A].Frame = 2;
            }
            else
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 1;
                else if(NPC[A].Direction == 1)
                    NPC[A].Frame = 3;
            }
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 6)
                NPC[A].Frame = 4;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 6)
                NPC[A].Frame = 6;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 6)
                        NPC[A].Frame = 4;
                }
                else
                {
                    NPC[A].Frame = NPC[A].Frame + 1;
                    if(NPC[A].Frame >= 8)
                        NPC[A].Frame = 6;
                }
            }
        }
    }
    else if(NPC[A].Type == 22) // Bullet bill Gun
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame == 5)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 107)
    {
        if(NPC[A].Location.SpeedX == 0)
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 7)
                NPC[A].Frame = 1;
            else
            {
                NPC[A].Frame = 1;
                NPC[A].FrameCount = 0;
            }
        }
        else
        {
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 15)
                NPC[A].Frame = 3;
            else
            {
                NPC[A].Frame = 3;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 4;
    }
    else if(NPC[A].Type == 26) // Spring thing
    {
        if(!LevelEditor)
        {
            if(NPC[A].Location.Height == 32)
            {
                NPC[A].Location.Height = 16;
                NPC[A].Location.Y = NPC[A].Location.Y + 16;
            }
            if(NPC[A].HoldingPlayer > 0)
                NPC[A].Frame = 0;
            else
            {
                C = 0;
                tempLocation = NPC[A].Location;
                tempLocation.Height = 24;
                tempLocation.Y = tempLocation.Y - 8;
                for(B = 1; B <= numPlayers; ++B)
                {
                    if(CheckCollision(tempLocation, Player[B].Location) && Player[B].Mount != 2 && (Player[B].Location.SpeedY > 0 || Player[B].Location.SpeedY < Physics.PlayerJumpVelocity))
                    {
                        C = 2;
                        break;
                    }
                }
                if(C == 0)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.Height = 32;
                    tempLocation.Y = tempLocation.Y - 16;
                    for(B = 1; B <= numPlayers; ++B)
                    {
                        if(CheckCollision(tempLocation, Player[B].Location) && Player[B].Mount != 2 && (Player[B].Location.SpeedY > 0 || Player[B].Location.SpeedY < Physics.PlayerJumpVelocity))
                        {
                            C = 1;
                            break;
                        }
                    }
                }
                NPC[A].Frame = C;
            }
        }
    }
    else if(NPC[A].Type == 39) // birdo
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 5;
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedX != 0)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].FrameCount > 12)
                    NPC[A].FrameCount = 0;
                else if(NPC[A].FrameCount >= 6)
                    NPC[A].Frame = NPC[A].Frame + 1;
            }
        }
        else if(NPC[A].Special < 0)
        {
            NPC[A].Frame = NPC[A].Frame + 3;
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
            if(NPC[A].FrameCount > 8)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 4)
                NPC[A].Frame = NPC[A].Frame + 1;
        }
        else
            NPC[A].Frame = NPC[A].Frame + 2;
    }
    else if(NPC[A].Type == 125) // Rat Head
    {
        NPC[A].Frame = NPC[A].FrameCount;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 2;
    }
    else if(NPC[A].Type == 29) // SMB Hammer Bro
    {
        if(NPC[A].Special3 >= 0)
        {
            if((NPC[A].Location.SpeedY < 1 && NPC[A].Location.SpeedY >= 0) || NPC[A].Slope > 0 || NPC[A].HoldingPlayer > 0)
            {
                NPC[A].FrameCount = NPC[A].FrameCount + 1;
                if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
                else if(NPC[A].Direction == 1 && NPC[A].Frame < 3)
                    NPC[A].Frame = 3;
                if(NPC[A].FrameCount >= 8)
                {
                    NPC[A].FrameCount = 0;
                    if(NPC[A].Direction == -1)
                    {
                        NPC[A].Frame = NPC[A].Frame + 1;
                        if(NPC[A].Frame >= 2)
                            NPC[A].Frame = 0;
                    }
                    else
                    {
                        NPC[A].Frame = NPC[A].Frame + 1;
                        if(NPC[A].Frame >= 5)
                            NPC[A].Frame = 3;
                    }
                }
            }
            else
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 0;
                else
                    NPC[A].Frame = 3;
            }
        }
        else
        {
            if(NPC[A].Direction == -1)
                NPC[A].Frame = 2;
            else
                NPC[A].Frame = 5;
        }
    }
    else if(NPC[A].Type == 108) // Yoshi Fireball
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = 0;
        }
        else if(NPC[A].FrameCount > 4)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = NPC[A].Frame + 2;
    }
    else if(NPC[A].Type == 35 || NPC[A].Type == 191 || NPC[A].Type == 193) // Goombas Shoe
    {
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 2 + SpecialFrame[1];
        else
            NPC[A].Frame = 0 + SpecialFrame[1];
    }
    else if(NPC[A].Type == 38 || NPC[A].Type == 43 || NPC[A].Type == 44) // Boo
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 2;
        if(NPC[A].Special == 1 || NPC[A].HoldingPlayer > 0)
            NPC[A].Frame = NPC[A].Frame + 1;
    }
    else if(NPC[A].Type == 41) // smb2 birdo exit
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame == 8)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 97) // SMB3 Star
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Special == 0)
        {
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 20)
                NPC[A].Frame = 1;
            else
                NPC[A].FrameCount = 0;
        }
        else
        {
            if(NPC[A].FrameCount < 60)
                NPC[A].Frame = 2;
            // ElseIf .FrameCount < 64 Then
            // .Frame = 1
            else
                NPC[A].FrameCount = 0;
        }
    }
    else if(!(NPCIsABonus[NPC[A].Type] || NPC[A].Type == 21 || NPC[A].Type == 32)) // Frame finder for everything else
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].Type == 48 || NPC[A].Type == 206)
            NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame == 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 183 || NPC[A].Type == 277)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 12)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame == 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == 182)
    {
        NPC[A].FrameCount = NPC[A].FrameCount + 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame = NPC[A].Frame + 1;
            if(NPC[A].Frame == 4)
                NPC[A].Frame = 0;
        }
    }
    else
    {
        if(A == 0)
            NPC[A].Frame = 0;
    }
}
