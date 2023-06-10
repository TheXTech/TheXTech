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

#include "globals.h"
#include "compat.h"
#include "effect.h"
#include "npc.h"
#include "npc_id.h"
#include "sound.h"
#include "game_main.h"
#include "collision.h"
#include "layers.h"
#include "graphics/gfx_update.h"

#include "main/trees.h"

// Updates the effects
void UpdateEffects()
{
// please reference the /graphics/effect folder to see what the effects are

//    int A = 0;
    int B = 0;
//    bool DontSpawnExit = false;
//    bool DontResetMusic = false;
    bool tempBool = false;
    int CoinCount = 0;

    if(FreezeNPCs)
        return;

    For(A, 1, numEffects)
    {
        auto &e = Effect[A];
        e.Life -= 1;

        if(e.Life == 0)
        {
            if(e.Type == 14)
            {
                if(e.NewNpc > 0)
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];
                    nn = NPC_t();
                    nn.Type = e.NewNpc;
                    nn.Location.Height = NPCHeight[nn.Type];
                    nn.Location.Width = NPCWidth[nn.Type];
                    nn.Location.X = e.Location.X + e.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                    nn.Location.Y = e.Location.Y - 1;
                    nn.Location.SpeedY = -6;
                    nn.Active = true;
                    nn.TimeLeft = 100;
                    nn.Frame = 0;
                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);
                    PlaySound(SFX_BossBeat);
                }
            }
        }

        e.Location.X += e.Location.SpeedX;
        e.Location.Y += e.Location.SpeedY;

        if(e.Type == 2 || e.Type == 126 || e.Type == 6 || e.Type == 23 || e.Type == 35 || e.Type == 37 || e.Type == 39 || e.Type == 41 || e.Type == 43 || e.Type == 45 || e.Type == 52 || e.Type == 62) // Stomped Goombas
        {
            e.Location.SpeedY = 0;
            e.Location.SpeedX = 0;
        }
        else if(e.Type == 112)
        {
            if(e.Life % 5 == 0)
            {
                NewEffect(108, newLoc(e.Location.X + (dRand() * e.Location.Width),
                                      e.Location.Y + (dRand() * e.Location.Height)));
            }
        }
        else if(e.Type == 111)
        {
            e.Location.SpeedY += 0.5;
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 7)
                    e.Frame = 0;
                if(e.Frame >= 14)
                    e.Frame = 7;
            }
        }
        else if(e.Type == 108)
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
            }
            if(e.Frame >= 7)
                e.Life = 0;
        }
        else if(e.Type == 136) // RotoDisk
        {
            if(e.Location.SpeedX != 0.0 || e.Location.SpeedY != 0.0)
                e.Location.SpeedY += 0.5;
            e.Frame += 1;
            if(e.Frame >= 5)
                e.Frame = 0;
        }
        else if(e.Type == 69) // bomb
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 2)
            {
                e.FrameCount = 0;
                if(e.Frame == 0)
                    e.Frame = 1;
                else
                    e.Frame = 0;
            }
        }
        else if(e.Type == 1 || e.Type == 21 || e.Type == 30 || e.Type == 51 || e.Type == 100 || e.Type == 135) // Block break
        {
            e.Location.SpeedY += 0.6;
            e.Location.SpeedX = e.Location.SpeedX * 0.99;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.FrameCount >= 3)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Frame = 0;
            }
        }
        else if(e.Type == 140) // larry shell
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.Frame += 1;
                e.FrameCount = 0;
            }
            if(e.Frame > 7)
                e.Frame = 0;

            if(HasSound(SFX_LarryShell)) // When new sound is presented, do animation a bit differently
            {
                if(e.Life == 75)
                    e.Location.SpeedY = -5;
                else if(e.Life == 65)
                    e.Location.SpeedY = -8;
                else if(e.Life == 60)
                    e.Location.SpeedY = -11;
                else if(e.Life == 52)
                    e.Location.SpeedY = -14;
            }
            else if(!HasSound(SFX_LarryShell) && e.Life == 100) // Old sound
            {
                e.Location.SpeedY = -8;
                PlaySound(SFX_WartKilled);
            }
        }
        else if(e.Type == 114) // Splash
        {
            e.FrameCount += 1;
            if(e.FrameCount < 8)
                e.Frame = 0;
            else if(e.FrameCount < 16)
                e.Frame = 1;
            else if(e.FrameCount < 24)
                e.Frame = 2;
            else if(e.FrameCount < 32)
                e.Frame = 3;
            else if(e.FrameCount < 40)
                e.Frame = 4;
            else
                e.Life = 0;
            if(e.FrameCount % 3 == 0)
            {
                e.Frame = 5;
            }
        }
        else if(e.Type == 113) // Water Bubbles
        {
            if(e.NewNpc == 0)
            {
                tempBool = false;
                for(B = 1; B <= numWater; B++)
                {
                    if(CheckCollision(e.Location, Water[B].Location) && !Water[B].Hidden)
                    {
                        tempBool = true;
                        break;
                    }
                }
                if(!tempBool)
                    e.Life = 0;
            }
            e.FrameCount += 1;
            if(e.FrameCount < 4)
                e.Frame = 0;
            else if(e.FrameCount < 6)
                e.Frame = 1;
            else
            {
                e.FrameCount = 0;
                e.Frame = 0;
            }
            e.Location.Y -= 2;
            e.Location.X += dRand() * 2.0 - 1.0;
        }
        else if(e.Type == 57) // egg shells
        {
            e.Location.SpeedY += 0.6;
            e.Location.SpeedX = e.Location.SpeedX * 0.99;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
        }
        else if(e.Type == 3 || e.Type == 5 || e.Type == 129 || e.Type == 130 || e.Type == 134) // Mario & Luigi death
            e.Location.SpeedY += 0.25;
        else if(e.Type == 145 || e.Type == 110 || e.Type == 127 || e.Type == 4 || e.Type == 143 || e.Type == 142 || e.Type == 7 || e.Type == 22 || e.Type == 31 || e.Type == 33 || e.Type == 34 || e.Type == 38 || e.Type == 40 || e.Type == 42 || e.Type == 44 || e.Type == 46 || e.Type == 53 || e.Type == 117) // Goomba air ride of dooom
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.Type == 110 || e.Type == 143)
                e.FrameCount += 1;
            if(e.FrameCount >= 8)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 2)
                    e.Frame = 0;
            }
        }
        else if(e.Type == 104) // Blaarg eyes
        {
            e.Life += 2;
            if(e.Life <= 30)
                e.Location.SpeedY = -2.8;
            else if(e.Life <= 40)
                e.Location.SpeedY = 0.5;
            else if(e.Life <= 80)
                e.Location.SpeedY = 0;
            else if(e.Life <= 100)
                e.Location.SpeedY = 2;
            else
                e.Life = 0;
            e.FrameCount += 1;
            if(e.FrameCount >= 16)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 2)
                    e.Frame = 0;
            }
        }
        else if(e.Type == 61) // Beack Koopa
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.FrameCount >= 15)
            {
                e.FrameCount = 0;
                e.Frame -= 1;
            }
            else if(e.FrameCount == 8)
                e.Frame += 1;
        }
        else if(e.Type == 8 || e.Type == 9 || e.Type == 15 || e.Type == 16 || e.Type == 19 || e.Type == 27 || e.Type == 146 || e.Type == 28 || e.Type == 29 || e.Type == 32 || e.Type == 36 || e.Type == 47 || e.Type == 60 || e.Type == 95 || e.Type == 96 || e.Type == 109) // Flying turtle shell / Bullet bill /hard thing
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
        }
        else if(e.Type == 26 || e.Type == 101 || e.Type == 102) // Goombas shoes
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            if(e.Location.SpeedX > 0)
                e.Frame = 0 + SpecialFrame[1];
            else
                e.Frame = 2 + SpecialFrame[1];
        }
        else if(e.Type == 10 || e.Type == 131) // SMW / SMB3 Puff of smoke
        {
            e.Location.X += e.Location.SpeedX;
            e.FrameCount += 1;
            if(e.FrameCount >= 3)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Life = 0;
            }
        }
        else if(e.Type == 147) // SMB2 Puff of smoke
        {
            e.Location.X += e.Location.SpeedX;
            e.FrameCount += 1;
            if(e.FrameCount >= 6)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Life = 0;
            }
        }
        else if(e.Type == 132) // stomp stars
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 3)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 2)
                {
                    e.Life = 0;
                    NewEffect(133, e.Location);
                }
            }
        }
        else if(e.Type == 133) // stomp stars
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 1)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Frame = 0;
            }
        }
        else if(e.Type == 73) // Tail whack
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 2)
            {
                // If .Frame = 0 Then
                    // .Frame = 2
                // ElseIf .Frame = 2 Then
                    // .Frame = 1
                // ElseIf .Frame = 1 Then
                    // .Frame = 3
                // Else
                e.Frame += 1;
                if(e.Frame > 3)
                {
                    e.Frame = 0;
                    e.Life = 0;
                }
                e.FrameCount = 0;
            }
        }
        else if(e.Type == 75) // Whack
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.Frame += 1;
                if(e.Frame > 1)
                    e.Life = 0;
                e.FrameCount = 0;
            }
        }
        else if(e.Type == 76)
        {
            e.Location.X += e.Location.SpeedX;
            e.Location.Y += e.Location.SpeedY;
        }
        else if(e.Type == 81 || e.Type == 123 || e.Type == 124) // P Switch
        {
            if(e.Life == 1)
            {
                e.Location.X += e.Location.Width / 2.0 - EffectWidth[10] / 2.0;
                e.Location.Y += e.Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(10, e.Location);
            }
        }
        else if(e.Type == 74) // Slide Smoke
        {
            e.FrameCount += 1;
            e.Location.Y -= 0.1;
            if(e.FrameCount >= 4)
            {
                e.Frame += 1;
                e.FrameCount = 0;
                if(e.Frame > 2)
                    e.Life = 0;
            }
        }
        else if(e.Type == 63) // Zelda Smoke
        {
            e.Location.X += e.Location.SpeedX;
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Life = 0;
            }
        }
        else if(e.Type == 11) // Coin out of block effect
        {
            if(e.Life == 1)
            {
                CoinCount += 1;
                if(CoinCount > 13)
                    CoinCount = 10;
                MoreScore(CoinCount, e.Location);
            }

            if(e.Life <= 2)
            {
                e.Frame = 6;
                // NOTE: The same behavior at VB6 code: just checks the boolean expression and writes 0 or -1
                e.Location.SpeedY = (e.Location.SpeedY == 0.0) ? 0.0 : -1.0;
            }
            else if(e.Life <= 4)
            {
                e.Frame = 5;
                // NOTE: The same behavior at VB6 code: just checks the boolean expression and writes 0 or -1
                e.Location.SpeedY = (e.Location.SpeedY == 0.0) ? 0.0 : -1.0;
            }
            else if(e.Life <= 6)
            {
                e.Frame = 4;
                // NOTE: The same behavior at VB6 code: just checks the boolean expression and writes 0 or -1
                e.Location.SpeedY = (e.Location.SpeedY == 0.0) ? 0.0 : -1.0;
            }
            else
            {
                e.Location.SpeedY += 0.4;
                e.FrameCount += 1;
                if(e.FrameCount >= 3)
                {
                    e.FrameCount = 0;
                    e.Frame += 1;
                    if(e.Frame >= 4)
                        e.Frame = 0;
                }
            }
        }
        else if(e.Type == 12) // Big Fireball Tail
        {
            // .Location.SpeedX = 0
            // .Location.SpeedY = 0
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
            }
        }
        else if(e.Type == 78) // Coin
        {
            e.Location.SpeedX = 0;
            e.Location.SpeedY = 0;
            e.FrameCount += 1;
            if(e.FrameCount >= 5)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 3)
                    e.Life = 0;
            }
        }
        else if(e.Type == 82) // Spinning block
        {
            e.Frame = SpecialFrame[3];
            if(e.Life < 10)
            {
                tempBool = false;
                for(B = 1; B <= numPlayers; B++)
                {
                    if(!Player[B].Dead && Player[B].TimeToLive == 0)
                    {
                        if(CheckCollision(e.Location, Player[B].Location))
                        {
                            tempBool = true;
                            break;
                        }
                    }
                }
                // tempBool = True
                if(!tempBool)
                {
                    e.Life = 0;
                    e.Frame = 3;
                    Block[e.NewNpc].Hidden = false;
                    invalidateDrawBlocks();
                }
                else
                    e.Life = 10;
            }
        }
        else if(e.Type == 80) // Twinkle
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 8)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 3)
                    e.Life = 0;
            }
        }
        else if(e.Type == 77 || e.Type == 139) // Small Fireball Tail
        {
            e.Location.X += dRand() * 2 - 1;
            e.Location.Y += dRand() * 2 - 1;
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 3 || e.Frame == 6 || e.Frame == 9 || e.Frame == 12 || e.Frame == 15)
                    e.Life = 0;
            }
        }
        else if(e.Type == 13) // Big Fireball Tail
        {
            e.Location.SpeedX = 0;
            e.Location.SpeedY = 0;
            e.FrameCount += 1;
            if(e.FrameCount >= 6)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                // e.FrameCount = 0;
            }
        }
        else if(e.Type == 14) // Dead Big Koopa
        {
            e.Location.SpeedX = 0;
            e.Location.SpeedY = 0;
            e.FrameCount += 1;
            if(e.FrameCount >= 2)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                // e.FrameCount = 0;
                if(e.Frame >= 4)
                    e.Frame = 0;
            }
        }
        else if(e.Type == 70) // SMB3 Bomb Part 1
        {
            if(e.FrameCount == 0)
            {
                NewEffect(71, e.Location, static_cast<float>(e.Frame));
                e.Frame += 1;
                if(e.Frame >= 4)
                    e.Frame = 0;
            }
            else if(e.FrameCount >= 6)
                e.FrameCount = -1;
            e.FrameCount += 1;
        }
        else if(e.Type == 71 || e.Type == 148) // SMB3 Bomb Part 2
        {
            e.FrameCount += 1;
                if(e.FrameCount >= 4)
                {
                    e.FrameCount = 0;
                    e.Frame += 1;
                    if(e.Frame >= 4)
                        e.Frame = 0;
                }
            if(e.Type == 148 && iRand(10) >= 8)
            {
                NewEffect(77, e.Location, 3);
                Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5;
                Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5;
            }
        }
        else if(e.Type == 125) // POW Block
        {
            e.FrameCount += 1;
                if(e.FrameCount >= 4)
                {
                    e.FrameCount = 0;
                    e.Frame += 1;
                    if(e.Frame >= 4)
                    {
                        e.Life = 0;
                        e.Frame = 3;
                    }
                }
        }
        else if(e.Type == 54 || e.Type == 55 || e.Type == 59 || e.Type == 103) // door
        {
            e.FrameCount += 1;
            if(e.FrameCount > 60)
                e.Life = 0;
            else if(e.FrameCount > 55)
                e.Frame = 0;
            else if(e.FrameCount > 50)
                e.Frame = 1;
            else if(e.FrameCount > 45)
                e.Frame = 2;
            else if(e.FrameCount > 40)
                e.Frame = 3;
            else if(e.FrameCount > 20)
                e.Frame = 4;
            else if(e.FrameCount > 15)
                e.Frame = 3;
            else if(e.FrameCount > 10)
                e.Frame = 2;
            else if(e.FrameCount > 5)
                e.Frame = 1;
        }
        else if(e.Type == 15 || e.Type == 16 || e.Type == 25 || e.Type == 48 || e.Type == 49 || e.Type == 50 || e.Type == 68 || e.Type == 72 || e.Type == 89 || e.Type == 90 || e.Type == 91 || e.Type == 92 || e.Type == 93 || e.Type == 94 || e.Type == 98 || e.Type == 99 || e.Type == 105 || e.Type == 138 || e.Type == 106 || e.Type == 141) // Bullet Bill / Hammer Bro
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
        }
        else if(e.Type == 128)
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            e.Frame = 5;
            if(e.FrameCount >= 16)
            {
                e.FrameCount = 0;
                //e.Frame = 5; // Already 5!
            }
            else if(e.FrameCount > 8)
                e.Frame = 4;
        }
        else if(e.Type == 17 || e.Type == 18 || e.Type == 20 || e.Type == 24 || (e.Type >= 64 && e.Type <= 67) || e.Type == 83) // Shy guy free falling
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.FrameCount >= 8)
            {
                e.FrameCount = 0;
                if(e.Frame == 4)
                    e.Frame = 5;
                else if(e.Frame == 5)
                    e.Frame = 4;
                else if(e.Frame == 6)
                    e.Frame = 7;
                else
                    e.Frame = 6;
            }
        }
        else if(e.Type == 85 || e.Type == 86 || e.Type == 87 || e.Type == 88 || e.Type == 97 || e.Type == 115 || e.Type == 122 || e.Type == 116 || e.Type == 118 || e.Type == 119 || e.Type == 120 || e.Type == 121 || e.Type == 137) // Rex / mega mole / smw goomba free falling
        {
            e.Location.SpeedY += 0.5;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.FrameCount >= 8)
            {
                e.FrameCount = 0;
                if(e.Frame == 0)
                    e.Frame = 1;
                else if(e.Frame == 1)
                    e.Frame = 0;
                else if(e.Frame == 2)
                    e.Frame = 3;
                else
                    e.Frame = 2;
            }
        }
        else if(e.Type == 56) // Egg
        {
            if(e.NewNpc == 0 && e.FrameCount < 19)
                e.FrameCount = 19;
            e.FrameCount += 1;
            if(e.FrameCount == 10)
                e.Frame += 1;
            else if(e.FrameCount == 20)
            {
                e.Frame = 2;
                NewEffect(57, e.Location);
            }
            else if(e.FrameCount == 30)
            {
                e.Life = 0;
                if(!LevelEditor && e.NewNpc != 96)
                {
                    if(NPCIsYoshi[e.NewNpc])
                        NewEffect(58, e.Location, 1, static_cast<float>(e.NewNpc));
                    else if(e.NewNpc > 0)
                    {
                        numNPCs++;
                        auto &nn = NPC[numNPCs];
                        nn = NPC_t();
                        nn.Location = e.Location;
                        nn.Active = true;
                        nn.TimeLeft = 100;
                        nn.Direction = 0;
                        nn.Type = e.NewNpc;
                        nn.Location.Height = NPCHeight[nn.Type];
                        nn.Location.Width = NPCWidth[nn.Type];
                        nn.Location.Y += 32 - nn.Location.Height;
                        nn.Location.X += -nn.Location.Width / 2.0 + 16;

                        if(nn.Type == NPCID_LEAF)
                            nn.Location.SpeedY = -6;

                        if(NPCIsCheep[e.NewNpc] || NPCIsAParaTroopa[e.NewNpc] || e.NewNpc == NPCID_FIREBAR)
                        {
                            nn.Special = e.NewNpcSpecial;
                            nn.DefaultSpecial = int(nn.Special);
                        }

                        if(e.NewNpc == NPCID_STAR_SMB3 || e.NewNpc == NPCID_STAR_SMW)
                            nn.Variant = (uint16_t)e.NewNpcSpecial;

                        syncLayers_NPC(numNPCs);
                        CheckSectionNPC(numNPCs);
                    }
                }
            }
        }
        else if(e.Type == 107)
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.Frame += 1;
                e.FrameCount = 0;
            }
            if(e.Frame >= 3)
                e.Life = 0;
        }
        else if(e.Type == 58) // yoshi grow
        {
            e.FrameCount += 1;
            if(e.FrameCount < 10)
                e.Frame = 0;
            else if(e.FrameCount < 20)
                e.Frame = 1;
            else if(e.FrameCount < 30)
                e.Frame = 0;
            else if(e.FrameCount < 40)
                e.Frame = 1;
            else if(e.FrameCount < 50)
                e.Frame = 0;
            else if(e.FrameCount < 60)
                e.Frame = 1;
            else
            {
                e.Frame = 1;
                e.Life = 0;
                numNPCs++;
                auto &nn = NPC[numNPCs];
                nn = NPC_t();
                nn.Location = e.Location;
                nn.Active = true;
                nn.TimeLeft = 100;
                nn.Direction = 1;
                nn.Type = e.NewNpc;
                nn.Location.Height = NPCHeight[nn.Type];
                nn.Location.Width = NPCWidth[nn.Type];
                syncLayers_NPC(numNPCs);
                CheckSectionNPC(numNPCs);
            }
            if(e.NewNpc == 98)
                e.Frame += 2;
            else if(e.NewNpc == 99)
                e.Frame += 4;
            else if(e.NewNpc == 100)
                e.Frame += 6;
            else if(e.NewNpc == 148)
                e.Frame += 8;
            else if(e.NewNpc == 149)
                e.Frame += 10;
            else if(e.NewNpc == 150)
                e.Frame += 12;
            else if(e.NewNpc == 228)
                e.Frame += 14;
        }
        else if(e.Type == 79)
            e.Location.SpeedY = e.Location.SpeedY * 0.97;
    } //for

    for(int A = numEffects; A >= 1; --A)
    {
        if(Effect[A].Life <= 0)
            KillEffect(A);
    }
}

void NewEffect(int A, const Location_t &Location, float Direction, int NewNpc, bool Shadow, vbint_t newNpcSpecial)
{
// this sub creates effects
// please reference the /graphics/effect folder to see what the effects are
// A is the effect type

    int B = 0;
    bool tempBool = false;
    double tempDoub = 0;

    if(numEffects >= maxEffects - 4)
        return;

    if(A == 1 || A == 21 || A == 30 || A == 51 || A == 100 || A == 135) // Block break effect
    {
        for(B = 1; B <= 4; B++)
        {
            numEffects++;
            auto &ne = Effect[numEffects];
            ne.Shadow = Shadow;
            ne.Location.Width = EffectWidth[A];
            ne.Location.Height = EffectHeight[A];
            ne.Type = A;
            ne.Location.SpeedX = 3;
            ne.Life = 200;
            if(B == 1 || B == 3)
                ne.Location.SpeedX = -ne.Location.SpeedX;
            if(B == 1 || B == 2)
                ne.Location.SpeedY = -11;
            else
                ne.Location.SpeedY = -7;
            ne.Location.SpeedX += dRand() * 2 - 1;
            ne.Location.SpeedY += dRand() * 4 - 2;
            if(B == 1)
            {
                ne.Location.X = Location.X;
                ne.Location.Y = Location.Y;
            }
            else if(B == 2)
            {
                ne.Location.X = Location.X + Location.Width / 2.0;
                ne.Location.Y = Location.Y;
            }
            else if(B == 3)
            {
                ne.Location.X = Location.X;
                ne.Location.Y = Location.Y + Location.Height / 2.0;
            }
            else
            {
                ne.Location.X = Location.X + Location.Width / 2.0;
                ne.Location.Y = Location.Y + Location.Height / 2.0;
            }
        }
    }
    else if(A == 140) // larry shell
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.NewNpc = NewNpc;
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 160;
        ne.Type = A;
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        PlaySound(SFX_LarryKilled);
    }
    else if(A == 104) // Blaarg eyes
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.NewNpc = NewNpc;
        ne.Shadow = Shadow;
        if(ne.NewNpc == 96)
            ne.NewNpc = 0;
        if(Direction == -1)
            ne.Location.X = Location.X + Location.Width / 2.0 + 16 + 48 * Direction;
        else
            ne.Location.X = Location.X + Location.Width / 2.0 + 16; // + 48 * Direction

        ne.Location.Y = Location.Y;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 10;
        ne.Type = A;
    }
    else if(A == 56 || A == 58) // Egg break / Yoshi grow
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.NewNpc = NewNpc;
        ne.NewNpcSpecial = newNpcSpecial;
        if(ne.NewNpc == 96)
            ne.NewNpc = 0;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 100;
        ne.Type = A;

        if(A == 56)
        {
            if(ne.NewNpc != 0 /*&& ne.NewNpc != 96*/) // never 96, because of condition above that replaces 96 with zero
                PlaySound(SFX_YoshiEgg);
            else
                PlaySound(SFX_Smash);
        }
        else if(A == 58)
            PlaySound(SFX_Yoshi);
    }
    else if(A == 136) // Roto Disk
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedX = Location.SpeedX;
        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Width;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 10;
        ne.Type = A;

    }
    else if(A == 125) // pow
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.NewNpc = NewNpc;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == 107) // Metroid Block
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.NewNpc = NewNpc;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == 57) // Egg shells
    {
        for(B = 1; B <= 4; B++)
        {
            numEffects++;
            auto &ne = Effect[numEffects];
            ne.Shadow = Shadow;
            ne.Location.Width = EffectWidth[A];
            ne.Location.Height = EffectHeight[A];
            ne.Type = A;
            ne.Location.SpeedX = 2;
            ne.Life = 200;
            if(B == 1 || B == 2)
                ne.Location.SpeedY = -11;
            else
            {
                ne.Location.SpeedY = -7;
                ne.Location.SpeedX = 1.5;
            }
            if(B == 1 || B == 3)
                ne.Location.SpeedX = -ne.Location.SpeedX;
            ne.Location.SpeedX += ((dRand() * 0.5) - 0.25);
            ne.Location.SpeedY += ((dRand() * 1) - 0.5);
            if(B == 1)
                ne.Frame = 0;
            else if(B == 2)
                ne.Frame = 1;
            else if(B == 3)
                ne.Frame = 3;
            else
                ne.Frame = 2;

            // .Location.SpeedX += Rnd * 2 - 1
            // .Location.SpeedY += Rnd * 4 - 2
            if(B == 1)
            {
                ne.Location.X = Location.X;
                ne.Location.Y = Location.Y;
            }
            else if(B == 2)
            {
                ne.Location.X = Location.X + Location.Width / 2.0;
                ne.Location.Y = Location.Y;
            }
            else if(B == 3)
            {
                ne.Location.X = Location.X;
                ne.Location.Y = Location.Y + Location.Height / 2.0;
            }
            else
            {
                ne.Location.X = Location.X + Location.Width / 2.0;
                ne.Location.Y = Location.Y + Location.Height / 2.0;
            }
        }
    }
    else if(A == 2 || A == 6 || A == 23 || A == 35 || A == 37 || A == 39 || A == 41 || A == 43 || A == 45 || A == 52 || A == 62 || A == 84 || A == 126) // Goomba smash effect
    {
        PlaySound(SFX_Stomp); // Stomp sound
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.Width = 32;
        ne.Location.Height = 34;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.Life = 20;
        ne.Type = A;
        if(A == 45)
        {
            ne.Location.Height = 46;
            ne.Location.Width = 48;
        }
        if(A == 84)
        {
            if(Direction == 1)
                ne.Frame = 1;
        }
    }
    else if(A == 81 || A == 123 || A == 124) // P Switch
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 113 || A == 114) // Water Bubble / Splash
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectWidth[A];
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Shadow = Shadow;
        tempBool = false;

        if(A == 114) // Change height for the background
        {
            for(const Background_t& b : treeBackgroundQuery(ne.Location, SORTMODE_ID))
            {
                if(b.Type == 82 || b.Type == 26 || b.Type == 65 || b.Type == 159 || b.Type == 166 || b.Type == 168)
                {
                    // note: NOT a reference
                    auto t = b.Location;
                    if(t.Height > 8 && g_compatibility.fix_submerged_splash_effect)
                        t.Height = 8; // Limit the height
                    if(CheckCollision(ne.Location, t))
                    {
                        if(b.Type == 82 || b.Type == 159)
                            ne.Location.Y = b.Location.Y - ne.Location.Height + 12;
                        if(b.Type == 26)
                            ne.Location.Y = b.Location.Y - ne.Location.Height + 6;
                        if(b.Type == 168)
                            ne.Location.Y = b.Location.Y - ne.Location.Height + 8;
                        if(b.Type == 166)
                            ne.Location.Y = b.Location.Y - ne.Location.Height + 10;
                        if(b.Type == 65)
                            ne.Location.Y = b.Location.Y - ne.Location.Height + 16;
                        tempBool = true;
                        break;
                    }
                }
            }
        }

        ne.Frame = 0;
        ne.Life = 300;
        ne.NewNpc = NewNpc;
        ne.Type = A;

        if(!tempBool && A == 114)
            numEffects -= 1;
    }
    else if(A == 109) // Spike Top
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = -12;
        ne.Location.SpeedX = Location.SpeedX;
        ne.Frame = vb6Round(Direction);
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 108) // Metroid
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 64;
        ne.Location.Height = 64;
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 200;
        ne.Type = A;
    }
    else if(A == 82) // Block Spin
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.NewNpc = NewNpc;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 300;
        ne.Type = A;
    }
    else if(A == 3 || A == 5 || A == 129 || A == 130 || A == 134) // Mario & Luigi died effect
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X - ne.Location.Width * 0.5 + Location.Width * 0.5;
        ne.Location.Y = Location.Y - ne.Location.Height * 0.5 + Location.Height * 0.5;
        ne.Location.SpeedY = -11;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        if(A == 134)
        {
            if(Direction == 1)
                ne.Frame = 1;
        }
        ne.Life = 150;
        ne.Type = A;
    }
    else if(A == 79) // Score
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Type = A;
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[ne.Type];
        ne.Location.Height = EffectHeight[ne.Type];
        ne.Location.X = Location.X - ne.Location.Width * 0.5 + Location.Width * 0.5;
        ne.Location.Y = Location.Y - ne.Location.Height * 0.5 + Location.Height * 0.5;
        ne.Location.X += dRand() * 32 - 16;
        ne.Location.Y += dRand() * 32 - 16;
        ne.Location.SpeedY = -2;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 60;
    }
    else if(A == 76) // SMW Smashed
    {
        for(B = 1; B <= 4; B++)
        {
            if(numEffects < maxEffects)
            {
                numEffects++;
                auto &ne = Effect[numEffects];
                ne.Shadow = Shadow;
                ne.Type = A;
                ne.Location.Width = EffectWidth[A];
                ne.Location.Height = EffectHeight[A];
                ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
                ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
                // ne.Location.SpeedY = -0; // Assigned below
                // ne.Location.SpeedX = 0; // Assigned below
                ne.Life = 15;

                ne.Location.SpeedX = 3 * 0.8;
                ne.Location.SpeedY = 1.5 * 0.8;

                if(B == 1 || B == 2)
                    ne.Location.SpeedY = -ne.Location.SpeedY;
                if(B == 1 || B == 3)
                    ne.Location.SpeedX = -ne.Location.SpeedX;

                ne.Frame = 0;
            }
        }
    }
    else if(A == 133) // stomp star part 2
    {
        for(B = 1; B <= 4; B++)
        {
            if(numEffects < maxEffects)
            {
                numEffects++;
                auto &ne = Effect[numEffects];
                ne.Shadow = Shadow;
                ne.Type = A;
                ne.Location.Width = EffectWidth[A];
                ne.Location.Height = EffectHeight[A];
                ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
                ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
                ne.Life = 8;
                ne.Location.SpeedX = 2;
                ne.Location.SpeedY = 2;
                if(B == 1 || B == 2)
                    ne.Location.SpeedY = -ne.Location.SpeedY;
                if(B == 1 || B == 3)
                    ne.Location.SpeedX = -ne.Location.SpeedX;
                ne.Location.Y += ne.Location.SpeedY * 6;
                ne.Location.X += ne.Location.SpeedX * 6;
                ne.Frame = 0;
            }
        }
    }
    else if(A == 70) // SMB3 Bomb Part 1
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 16;
        ne.Location.Height = 16;
        ne.Location.X = Location.X - ne.Location.Width * 0.5 + Location.Width * 0.5;
        ne.Location.Y = Location.Y - ne.Location.Height * 0.5 + Location.Height * 0.5;
        ne.Location.SpeedY = -0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 46;
        ne.Type = A;

    }
    else if(A == 148) // Heart Bomb
    {
        for(B = 1; B <= 6; B++)
        {
            if(numEffects < maxEffects)
            {
                numEffects++;
                auto &ne = Effect[numEffects];
                ne.Shadow = Shadow;
                ne.Location.Width = EffectWidth[A];
                ne.Location.Height = EffectHeight[A];
                ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
                ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
                ne.Location.SpeedY = -0;
                ne.Location.SpeedX = 0;
                ne.Life = 10;
                if(B == 1 || B == 3 || B == 4 || B == 6)
                {
                    ne.Location.SpeedY = 3.5;
                    ne.Location.SpeedX = 2;
                }
                else
                {
                    ne.Life = 11;
                    ne.Location.SpeedY = 0;
                    ne.Location.SpeedX = 4;
                }
                if(B <= 3)
                    ne.Location.SpeedX = -ne.Location.SpeedX;
                if(B == 1 || B == 6)
                    ne.Location.SpeedY = -ne.Location.SpeedY;
                if(int(Direction) % 2 == 0)
                {
                    tempDoub = ne.Location.SpeedX;
                    ne.Location.SpeedX = ne.Location.SpeedY;
                    ne.Location.SpeedY = tempDoub;
                }
                ne.Location.SpeedX = ne.Location.SpeedX * 0.5;
                ne.Location.SpeedY = ne.Location.SpeedY * 0.5;

                ne.Location.X += ne.Location.SpeedX * 3;
                ne.Location.Y += ne.Location.SpeedY * 3;

                ne.Frame = iRand(4);
                ne.Type = A;
            }
        }


    }
    else if(A == 71) // SMB3 Bomb Part 2
    {
        for(B = 1; B <= 6; B++)
        {
            if(numEffects < maxEffects)
            {
                numEffects++;
                auto &ne = Effect[numEffects];
                ne.Shadow = Shadow;
                ne.Location.Width = 16;
                ne.Location.Height = 16;
                ne.Location.X = Location.X;
                ne.Location.Y = Location.Y;
                ne.Location.SpeedY = -0;
                ne.Location.SpeedX = 0;
                ne.Life = 13;
                if(B == 1 || B == 3 || B == 4 || B == 6)
                {
                    ne.Location.SpeedY = 3;
                    ne.Location.SpeedX = 2;
                    ne.Life = 14;
                }
                else
                {
                    ne.Location.SpeedY = 0;
                    ne.Location.SpeedX = 4;
                }
                if(B <= 3)
                    ne.Location.SpeedX = -ne.Location.SpeedX;
                if(B == 1 || B == 6)
                    ne.Location.SpeedY = -ne.Location.SpeedY;
                if(int(Direction) % 2 == 0)
                {
                    tempDoub = ne.Location.SpeedX;
                    ne.Location.SpeedX = ne.Location.SpeedY;
                    ne.Location.SpeedY = tempDoub;
                }
                ne.Location.SpeedX = ne.Location.SpeedX * 1.5;
                ne.Location.SpeedY = ne.Location.SpeedY * 1.5;
                ne.Frame = vb6Round(Direction);
                ne.Type = A;
            }
        }
    }
    else if(A == 54 || A == 55 || A == 59 || A == 103) // Door Effect
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 150;
        ne.Type = A;
    }
    else if(A == 4 || A == 7 || A == 8 || A == 9 || A == 19 || A == 22 ||
            A == 26 || A == 101 || A == 102 ||
            A == 27 || A == 146 || A == 28 || A == 29 || A == 31 ||
            A == 32 || A == 145 || A == 33 || A == 34 ||
            A == 36 || A == 38 || A == 40 || A == 42 ||
            A == 44 || A == 46 || A == 47 || A == 53 || A == 60 ||
            A == 6 || A == 95 || A == 96 || A == 110 ||
            A == 117 || A == 121 || A == 127 || A == 142) // Flying goomba / turtle shell / hard thing shell /*A == 9 || - duplicated*/
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;

        if(fEqual(Location.SpeedY, 0.123))
        {
            ne.Location.SpeedY = 1;
            ne.Location.SpeedX = 0;
        }
        else if(Location.SpeedY != -5.1)
        {
            ne.Location.SpeedY = -11;
            ne.Location.SpeedX = Location.SpeedX;
        }
        else
        {
            ne.Location.SpeedY = -5.1;
            ne.Location.SpeedX = Location.SpeedX * 0.6;
        }

        ne.Frame = 0;
        ne.Life = 150;
        ne.Type = A;

        if(ne.Type == 29 && Direction == -1)
            ne.Frame = 1;
        if((ne.Type == 27 || ne.Type == 146) && Direction == 1)
            ne.Frame = 2;
        if(ne.Type == 36 && Direction == 1)
            ne.Frame = 1;
    }
    else if(A == 78) // Coins
    {
        for(B = 1; B <= 4; B++)
        {
            numEffects++;
            auto &ne = Effect[numEffects];
            ne.Shadow = Shadow;
            ne.Location.Width = EffectWidth[A];
            ne.Location.Height = EffectHeight[A];
            ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
            ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
            ne.Location.SpeedY = 0;
            ne.Location.SpeedX = 0;
            if(B == 1)
                ne.Location.X -= 10;
            if(B == 3)
                ne.Location.X += 10;
            if(B == 2)
                ne.Location.Y += 16;
            if(B == 4)
                ne.Location.Y -= 16;
            ne.Frame = 0 - B;
            ne.Life = 20 * B;
            ne.Type = A;
        }
    }
    else if(A == 10 || A == 73 || A == 74 || A == 75 || A == 131 || A == 132 || A == 147) // Puff of smoke
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        if(A == 132)
        {
            ne.Location.Y += dRand() * 16 - 8;
            ne.Location.X += dRand() * 16 - 8;
        }
        ne.Frame = 0;
        ne.Life = 12;
        ne.Type = A;

        if(ne.Type == 147)
            ne.Life = 24;

        if(A == 73 || A == 75)
        {
            ne.Location.X += dRand() * 16 - 8;
            ne.Location.Y += dRand() * 16 - 8;
        }
        else if(A == 74)
        {
            ne.Location.X += dRand() * 4 - 2;
            ne.Location.Y += dRand() * 4 - 2;
        }
    }
    else if(A == 144) // bubble pop
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 6;
        ne.Type = A;
    }
    else if(A == 63) // Zelda Style Smoke
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 48;
        ne.Location.Height = 48;
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height / 2.0 - ne.Location.Height / 2.0;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == 11) // Coin hit out of block
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y - 32;
        ne.Location.SpeedY = -8;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 46;
        ne.Type = A;
    }
    else if(A == 12) // Big Fireball Tail
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 8;
        ne.Location.Height = 8;
        ne.Location.X = Location.X + 4 + (dRand() * 12.0);
        ne.Location.Y = Location.Y + 40;
        // .Location.SpeedY = -8
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 12;
        ne.Type = A;
    }
    else if(A == 111) // Glass Shatter
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 16;
        ne.Location.Height = 16;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = -2 - dRand() * 10;
        ne.Location.SpeedX = dRand() * 8 - 4;
        ne.Frame = 0;
        if(iRand(2) == 0)
            ne.Frame = 7;
        ne.Frame += iRand(7);
        ne.Life = 300;
        ne.Type = A;
    }
    else if(A == 112) // Mother Brain
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        if(int(Direction) == 1)
            ne.Frame = 1;
        ne.Life = 360;
        ne.Type = A;
    }
    else if(A == 77 || A == 139) // Small Fireball Tail
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - EffectWidth[A] / 2.0 + dRand() * 4 - 2;
        ne.Location.Y = Location.Y + Location.Height / 2.0 - EffectHeight[A] / 2.0 + dRand() * 4 - 2;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;

        if(int(Direction) == 2)
            ne.Frame = 3;
        if(int(Direction) == 3)
            ne.Frame = 6;
        if(int(Direction) == 4)
            ne.Frame = 9;
        if(int(Direction) == 5)
            ne.Frame = 12;

        ne.Life = 60;
        ne.Type = A;
    }
    else if(A == 80) // Twinkle
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 16;
        ne.Location.Height = 16;
        ne.Location.X = Location.X + Location.Width / 2.0 - 4 + dRand() * 4 - 2;
        ne.Location.Y = Location.Y + Location.Height / 2.0 - 4 + dRand() * 4 - 2;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 60;
        ne.Type = A;
    }
    else if(A == 13) // Lava Splash
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[13];
        ne.Location.Height = EffectHeight[13];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + 24;
        ne.Location.SpeedY = -8;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == 14) // Dead Big Koopa
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + 22;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.NewNpc = NewNpc;
        ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 15 || A == 68) // Dead Bullet Bill
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.SpeedX = -Location.SpeedX;
        if(int(Direction) == -1)
            ne.Frame = 0;
        else
            ne.Frame = 1;
        if(A == 68)
            ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 61) // Flying Beach Koopa
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = -11;
        ne.Location.SpeedX = -Location.SpeedX;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 48) // Dead toad
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.SpeedX = Location.SpeedX;
        ne.Location.X += ne.Location.Width / 2.0 - 16;
        ne.Location.Y += ne.Location.Height / 2.0 - 16;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Frame = 0;
        ne.Location.SpeedY = -8;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 16) // Dead Giant Bullet Bill
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.SpeedX = Location.SpeedX;

        if(int(Direction) == -1)
            ne.Frame = 0;
        else
            ne.Frame = 1;

#if 0 // FIXME: This condition never works [PVS Studio]
        if(A == 48)
            ne.Location.SpeedY = -8;
#endif

        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 69) // Bomb
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 64;
        ne.Location.Height = 64;
        ne.Location.X = Location.X + Location.Width / 2.0 - 32;
        ne.Location.Y = Location.Y + Location.Height / 2.0 - 32;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Life = 60;
        ne.Type = A;
    }
    else if(A == 128) // pokey
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = -11;
        ne.Location.SpeedX = Location.SpeedX;
        ne.Life = 120;
        ne.Type = A;
        ne.Frame = 5;

    }
    else if(A == 17 || A == 18 || A == 20 || A == 24 || (A >= 64 && A <= 67) ||
            A == 83 || A == 85 || A == 86 || A == 87 || A == 88 || A == 97 || A == 115 ||
            A == 122 || A == 116 || A == 118 || A == 119 || A == 120 || A == 137) // Shy guy / Star Thing /Red Jumping Fish
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Frame = 0;

        if(A != 24 && A != 115 && A != 116)
            ne.Location.SpeedY = -11;
        else
            ne.Location.SpeedY = Location.SpeedY;

        ne.Location.SpeedX = Location.SpeedX;
        if(Location.SpeedY == 0.123)
        {
            ne.Location.SpeedY = 1;
            ne.Location.SpeedX = 0;
        }

        if(int(Direction) == -1)
        {
            if(A == 85 || A == 86 || A == 87 || A == 88 || A == 97 || A == 115 ||
               A == 116 || A == 118 || A == 119 || A == 120 || A == 122 || A == 137)
                ne.Frame = 0;
            else
                ne.Frame = 4;
        }
        else
        {
            if(A == 85 || A == 86 || A == 87 || A == 88 || A == 97 || A == 115 ||
               A == 116 || A == 118 || A == 119 || A == 120 || A == 122 || A == 137)
                ne.Frame = 2;
            else
                ne.Frame = 6;
        }

        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 90 || A == 91 || A == 92 || A == 93 || A == 94 || A == 98 || A == 99) // Boo / thwomps
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.SpeedX = -Location.SpeedX;

        if(A == 91)
        {
            ne.Location.X += ne.Location.Width / 2.0;
            ne.Location.Y += ne.Location.Height / 2.0;
            ne.Location.Width = EffectWidth[A];
            ne.Location.Height = EffectHeight[A];
            ne.Location.X += -ne.Location.Width / 2.0;
            ne.Location.Y += -ne.Location.Height / 2.0;
        }

        if(ne.Location.SpeedX != 0 && ne.Location.SpeedX > -2 && ne.Location.SpeedX < 2)
            ne.Location.SpeedX = 2 * -Direction;
        if(Direction == -1)
            ne.Frame = 0;
        else
            ne.Frame = 2;
        if(A == 90 || A == 98 || A == 99)
            ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 25 || A == 49 || A == 50 || A == 72 || A == 89 || A == 105 || A == 106 || A == 138 || A == 141) // Hammer Bro
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;

        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;

        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;

        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.SpeedX = -Location.SpeedX;
        if(ne.Location.SpeedX != 0 && ne.Location.SpeedX > -2 && ne.Location.SpeedX < 2)
            ne.Location.SpeedX = 2 * -Direction;
        if(int(Direction) == -1)
            ne.Frame = 0;
        else
            ne.Frame = 1;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == 143) // ludwig dead
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + Location.Width / 2.0 - ne.Location.Width / 2.0;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = -14;
        ne.Location.SpeedX = 3 * -Direction;
        ne.Life = 200;
        ne.Type = A;
        PlaySound(SFX_LudwigKilled);
    }
}

// Remove the effect
void KillEffect(int A)
{
    if(numEffects == 0 || A > maxEffects)
        return;

    Effect_t &e = Effect[numEffects];
    Effect[A] = e;
    e.Frame = 0;
    e.FrameCount = 0;
    e.Life = 0;
    e.Type = 0;
    numEffects -= 1;
}
