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
#include "npc.h"
#include "npc_traits.h"
#include "npc/npc_queues.h"
#include "npc/section_overlap.h"
#include "sound.h"
#include "effect.h"
#include "eff_id.h"
#include "config.h"
#include "collision.h"
#include "player.h"
#include "layers.h"
#include "blocks.h"
#include "graphics.h"

#include "main/trees.h"

void NPCSpecialMaybeHeld(int A)
{
    // Special Code for things that work while held
    if(NPC[A].Type == NPCID_BOMB) // SMB2 Bomb
    {
        // If .Location.SpeedX < -2 Or .Location.SpeedX > 2 Or .Location.SpeedY < -2 Or .Location.SpeedY > 5 Then .Projectile = True
        NPC[A].Special += 1;
        if(NPC[A].Special > 250)
            NPC[A].Special2 = 1;

        if(NPC[A].Special >= 350 || NPC[A].Special < 0)
        {
            Bomb(NPC[A].Location, 2);
            NPC[A].Killed = 9;
            NPCQueues::Killed.push_back(A);
        }
    }
    else if(NPC[A].Type == NPCID_WALK_BOMB_S2) // SMB2 Bob-om
    {
        NPC[A].Special += 1;
        if(NPC[A].Special > 450)
            NPC[A].Special2 = 1;
        if(NPC[A].Special >= 550 || NPC[A].Special < 0)
        {
            Bomb(NPC[A].Location, 2);
            NPC[A].Killed = 9;
            NPCQueues::Killed.push_back(A);
        }
    }
    else if(NPC[A].Type == NPCID_LIT_BOMB_S3) // SMB3 Bomb
    {
        if(!NPC[A].Inert)
            NPC[A].Special += 1;
        if(NPC[A].Special > 250)
            NPC[A].Special2 = 1;
        if(NPC[A].Special >= 350 || NPC[A].Special < 0)
            Bomb(NPC[A].Location, 3);
    }
    else if(NPC[A].Type == NPCID_SKELETON)
    {
        if(NPC[A].Special > 0)
        {
            NPC[A].Special2 += 1;
            if(NPC[A].Special2 >= 400 && NPC[A].Special3 == 0)
            {
                NPC[A].Special = 0;
                NPC[A].Special2 = 0;
                NPC[A].Inert = false;
                NPC[A].Stuck = false;
            }
            else if(NPC[A].Special2 >= 300)
            {
                if(NPC[A].Special3 == 0)
                {
                    NPC[A].Location.X += 2;
                    NPC[A].Special3 = 1;
                }
                else
                {
                    NPC[A].Location.X -= 2;
                    NPC[A].Special3 = 0;
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_VILLAIN_S3) // smb3 bowser
    {
        // special5 - the player
        // special4 - what bowser is doing
        // special3 - counter for what bowser is doing
        // specialY - counter for what bowser needs to do (was Special2)
        if(NPC[A].Legacy)
        {
            if(NPC[A].TimeLeft > 1)
            {
                NPC[A].TimeLeft = 100;
                if(bgMusic[NPC[A].Section] != 21)
                {
                    StopMusic();
                    bgMusic[NPC[A].Section] = 21;
                    StartMusic(NPC[A].Section);
                }
            }
        }

        if(NPC[A].Special4 == 0)
        {
            NPC[A].Special3 = 0; // reset counter when done
            if(NPC[A].Direction < 0)
                NPC[A].Frame = 0;
            else
                NPC[A].Frame = 5;
        }

        if(NPC[A].Special5 == 0) // find player
        {
            bool tempBool = false;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].TimeToLive == 0)
                    tempBool = true;
            }

            if(!tempBool)
            {
                NPC[A].Special5 = 0;
                NPC[A].Special4 = 2;
            }
            else
            {
                int B;
                do
                    B = iRand(numPlayers) + 1;
                while(Player[B].Dead || Player[B].TimeToLive > 0);
                NPC[A].Special5 = B;
            }
        }

        // see if facing the player
        bool tempBool = false;
        if(NPC[A].Special5 > 0)
        {
            if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
            {
                if(NPC[A].Direction < 0)
                    tempBool = true;
            }
            else
            {
                if(NPC[A].Direction > 0)
                    tempBool = true;
            }
        }

        NPC[A].SpecialY += dRand();

        if(NPC[A].Special4 == 0 && tempBool)
        {
            if(NPC[A].SpecialY >= 200 + dRand() * 400 - dRand() * 200) // hop on player
            {
                if(NPC[A].SpecialY >= 200 + dRand() * 600)
                    NPC[A].SpecialY = 0;
                NPC[A].Special4 = 3;
            }
            else if((NPC[A].SpecialY >= 80 && NPC[A].SpecialY <= 130) || (NPC[A].SpecialY >= 160 + dRand() * 300 && NPC[A].SpecialY <= 180 + dRand() * 800)) // shoot fireball
                NPC[A].Special4 = 4;
        }

        if(NPC[A].Inert)
        {
            if(NPC[A].Special4 == 4 || NPC[A].Special4 == 3)
                NPC[A].Special4 = 0;
        }

        if(NPC[A].Special4 == 0) // when not doing anything turn to player
        {
            if(!tempBool)
            {
                if(NPC[A].Direction > 0)
                    NPC[A].Special4 = -1;
                if(NPC[A].Direction < 0)
                    NPC[A].Special4 = 1;
            }
        }

        if(NPC[A].Special4 == 0) // hop
            NPC[A].Special4 = 2;

        if(NPC[A].HoldingPlayer > 0)
        {
            if(NPC[A].Direction == -1)
                NPC[A].Frame = 0;
            else
                NPC[A].Frame = 5;
            NPC[A].Special4 = 9000;
        }
        else if(NPC[A].Special4 == 9000)
        {
            NPC[A].Special5 = NPC[A].CantHurtPlayer;
            NPC[A].Special4 = 0;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
        }

        if(NPC[A].Special4 == -1) // turn left
        {
            NPC[A].Special3 -= 1;
            if(NPC[A].Special3 > -5)
                NPC[A].Frame = 9;
            else if(NPC[A].Special3 > -10)
                NPC[A].Frame = 8;
            else if(NPC[A].Special3 > -15)
                NPC[A].Frame = 12;
            else if(NPC[A].Special3 > -20)
                NPC[A].Frame = 3;
            else if(NPC[A].Special3 > -25)
                NPC[A].Frame = 4;
            else
            {
                NPC[A].Special4 = 0;
                NPC[A].Direction = -1;
            }
        }
        else if(NPC[A].Special4 == 1) // turn right
        {
            NPC[A].Special3 += 1;
            if(NPC[A].Special3 < 5)
                NPC[A].Frame = 4;
            else if(NPC[A].Special3 < 10)
                NPC[A].Frame = 3;
            else if(NPC[A].Special3 < 15)
                NPC[A].Frame = 12;
            else if(NPC[A].Special3 < 20)
                NPC[A].Frame = 8;
            else if(NPC[A].Special3 < 25)
                NPC[A].Frame = 9;
            else
            {
                NPC[A].Special4 = 0;
                NPC[A].Direction = 1;
            }
        }
        else if(NPC[A].Special4 == -10) // look left
        {
            NPC[A].Special3 -= 1;
            if(NPC[A].Special3 > -5)
                NPC[A].Frame = 3;
            else if(NPC[A].Special3 > -10)
                NPC[A].Frame = 4;
            else
            {
                NPC[A].Special4 = 0;
                NPC[A].Direction = -1;
            }
        }
        else if(NPC[A].Special4 == 10) // look right
        {
            NPC[A].Special3 += 1;
            if(NPC[A].Special3 < 5)
                NPC[A].Frame = 8;
            else if(NPC[A].Special3 < 10)
                NPC[A].Frame = 9;
            else
            {
                NPC[A].Special4 = 0;
                NPC[A].Direction = 1;
            }
        }
        else if(NPC[A].Special4 == 2) // hops
        {
            if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
            {
                if(NPC[A].Special3 < 5)
                {
                    NPC[A].Special3 += 1;
                    if(NPC[A].Direction < 0)
                        NPC[A].Frame = 1;
                    else
                        NPC[A].Frame = 6;
                }
                else if(NPC[A].Special3 == 5)
                {
                    NPC[A].Special3 += 1;
                    NPC[A].Location.SpeedY = -3;
                    NPC[A].Location.Y -= 0.1;
                    if(NPC[A].Direction < 0)
                        NPC[A].Frame = 0;
                    else
                        NPC[A].Frame = 5;
                }
                else if(NPC[A].Special3 < 10)
                {
                    NPC[A].Special3 += 1;
                    if(NPC[A].Direction < 0)
                        NPC[A].Frame = 1;
                    else
                        NPC[A].Frame = 6;
                }
                else
                    NPC[A].Special4 = 0;
            }
        }
        else if(NPC[A].Special4 == 3) // jump on player
        {
            if(NPC[A].Special3 < -1)
            {
                if(NPC[A].Special > 1)
                    NPC[A].Special -= 1;
                NPC[A].Special3 += 1;
                if(NPC[A].Special3 == -1)
                    NPC[A].Special3 = 6;
            }
            else if(NPC[A].Special3 < 5)
            {
                NPC[A].Special3 += 1;
                if(NPC[A].Direction < 0)
                    NPC[A].Frame = 1;
                else
                    NPC[A].Frame = 6;
            }
            else if(NPC[A].Special3 == 5)
            {
                auto &sx = NPC[A].Location.SpeedX;
                auto &pl = Player[NPC[A].Special5].Location;
                NPC[A].Special3 += 1;
                NPC[A].Location.SpeedY = -12;
                NPC[A].BeltSpeed = 0;
                NPC[A].Location.Y -= 0.1;
                // This formula got been compacted: If something will glitch, feel free to restore back this crap
                //NPC[A].Location.SpeedX = (static_cast<int>(std::floor(static_cast<double>(((Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 - 16) + 1) / 32))) * 32 + 1 - NPC[A].Location.X) / 50;
                double pCenter = pl.X + pl.Width / 2.0;
                sx = std::floor((pCenter - 16.0 + 1.0) / 32.0) * 32.0 + 1.0;
                sx -= NPC[A].Location.X;
                sx /= 50;
                if(sx > 15)
                    sx = 15;
                else if(sx < -15)
                    sx = -15;
                NPC[A].Special3 = -50;
                NPC[A].Special = 10;
                if(NPC[A].Direction < 0)
                    NPC[A].Frame = 0;
                else
                    NPC[A].Frame = 5;
            }
            else if(NPC[A].Special3 == 6)
            {
                if(NPC[A].Location.SpeedY > 0)
                {
                    NPC[A].Frame = 10;
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.SpeedY = 0;
                    NPC[A].Special3 = 10;
                    NPC[A].Projectile = true;
                }
            }
            else if(NPC[A].Special3 < 13)
            {
                NPC[A].Location.SpeedY = -2;
                NPC[A].Special3 += 1;
            }
            else if(NPC[A].Special3 < 16)
            {
                NPC[A].Location.SpeedY = 2;
                NPC[A].Special3 += 1;
            }
            else if(NPC[A].Special3 < 19)
            {
                NPC[A].Location.SpeedY = -2;
                NPC[A].Special3 += 1;
            }
            else if(NPC[A].Special3 < 21)
            {
                NPC[A].Location.SpeedY = 2;
                NPC[A].Special3 += 1;
            }
            else if(NPC[A].Special3 == 21)
            {
                if(NPC[A].Location.SpeedY != 0.0)
                    NPC[A].Location.SpeedY = 10;
                else
                {
                    bool legacy = /*NPC[A].Legacy &&*/ (NPC[A].Variant == 1);
                    PlaySoundSpatial(SFX_Stone, NPC[A].Location);
                    NPC[A].Special3 = 30;
                    NPC[A].Frame = 11;
                    NPC[A].Projectile = false;
                    Location_t tempLocation = NPC[A].Location;

                    // Useless self-assignment code [PVS Studio]
                    //tempLocation.X = tempLocation.X; // + 16
                    //tempLocation.Width = tempLocation.Width; // - 32

                    tempLocation.Y += tempLocation.Height - 8;
                    tempLocation.Height = 16;
                    // fBlock = FirstBlock[long(NPC[A].Location.X / 32) - 1];
                    // lBlock = LastBlock[long((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];
                    // blockTileGet(NPC[A].Location, fBlock, lBlock);

                    for(BlockRef_t block : treeFLBlockQuery(tempLocation, false))
                    {
                        int B = block;
                        if(Block[B].Type == 186 && CheckCollision(tempLocation, Block[B].Location) && !Block[B].Hidden)
                            KillBlock(B);
                    }

                    if(!legacy && g_config.extra_screen_shake)
                        doShakeScreen(0, 4, SHAKE_SEQUENTIAL, 7, 0.15);

                    if(legacy) // Classic SMBX 1.0's behavior when Bowser stomps a floor
                    {
                        // fBlock = FirstBlock[long(level[NPC[A].Section].X / 32) - 1];
                        // lBlock = LastBlock[long((level[NPC[A].Section].Width) / 32.0) + 2];
                        // {
                        //     auto &sec = level[NPC[A].Section];
                        //     Location_t toShake;
                        //     toShake.X = sec.X;
                        //     toShake.Width = (sec.Width - sec.X);
                        //     blockTileGet(toShake, fBlock, lBlock);
                        // }

                        // Shake all blocks up
                        // for(int B = (int)fBlock; B <= lBlock; B++)
                        //     BlockShakeUp(B);
                        {
                            const auto &sec = level[NPC[A].Section];
                            Location_t toShake;
                            toShake.X = sec.X;
                            toShake.Width = (sec.Width - sec.X);
                            toShake.Y = sec.Y;
                            toShake.Height = (sec.Height - sec.Y);
                            for(BlockRef_t block : treeFLBlockQuery(toShake, false))
                            {
                                int B = block;
                                BlockShakeUp(B);
                            }
                        }

                        // expand down a section at the bottom of destroyed blocks
                        for(int B = 0; B < numSections; B++)
                        {
                            auto &n = NPC[A];
                            auto &s = level[B];

                            if(n.Location.X >= s.X &&
                               n.Location.X + n.Location.Width <= s.Width &&
                               n.Location.Y + n.Location.Height + 48 >= s.Y &&
                               n.Location.Y + n.Location.Height + 48 <= s.Height &&
                               B != n.Section)
                            {
                                n.SpecialY = 0;
                                n.Special3 = 0;
                                n.Special4 = 2;

                                auto &ns = level[n.Section];
                                if(s.X < ns.X)
                                    ns.X = s.X;
                                if(s.Y < ns.Y)
                                    ns.Y = s.Y;
                                if(s.Width > ns.Width)
                                    ns.Width = s.Width;
                                if(s.Height > ns.Height)
                                    ns.Height = s.Height;

                                s.X = 0;
                                s.Y = 0;
                                s.Width = 0;
                                s.Height = 0;

                                UpdateSectionOverlaps(B);
                                UpdateSectionOverlaps(n.Section);

                                for(int C = 1; C <= numNPCs; C++)
                                {
                                    auto &nc = NPC[C];
                                    if(nc.Section == B)
                                        nc.Section = n.Section;
                                }
                            }
                        } // for
                        SoundPause[SFX_Stomp] = 12;
                    }
                }
            }
            else if(NPC[A].Special3 < 35)
            {
                NPC[A].Frame = 11;
                NPC[A].Special3 += 1;
            }
            else if(NPC[A].Special3 < 40)
            {
                NPC[A].Frame = 12;
                NPC[A].Special3 += 1;
                NPC[A].Special5 = 0;
            }
            else
            {
                if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                {
                    NPC[A].Special3 = 0;
                    if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                        NPC[A].Special4 = -10;
                    else
                        NPC[A].Special4 = 10;
                }
            }
        }
        else if(NPC[A].Special4 == 4) // shoot a fireball
        {
            NPC[A].Special3 += 1;
            if(NPC[A].Special3 < 15)
            {
                if(NPC[A].Direction < 0)
                    NPC[A].Frame = 13;
                else
                    NPC[A].Frame = 14;
            }
            else if(NPC[A].Special3 < 30)
            {
                if(NPC[A].Direction < 0)
                    NPC[A].Frame = 2;
                else
                    NPC[A].Frame = 7;

                if(NPC[A].Special3 == 29)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].Type = NPCID_VILLAIN_FIRE;
                    if(NPC[numNPCs].Direction > 0)
                        NPC[numNPCs].Frame = 4;
                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                    if(NPC[numNPCs].Direction < 0)
                        NPC[numNPCs].Location.X = NPC[A].Location.X - 40;
                    else
                        NPC[numNPCs].Location.X = NPC[A].Location.X + 54;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 19;
                    NPC[numNPCs].Location.SpeedX = 4 * double(NPC[numNPCs].Direction);
                    double C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0);
                    double D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2.0);
                    NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                    if(NPC[numNPCs].Location.SpeedY > 1)
                        NPC[numNPCs].Location.SpeedY = 1;
                    else if(NPC[numNPCs].Location.SpeedY < -1)
                        NPC[numNPCs].Location.SpeedY = -1;
                    syncLayers_NPC(numNPCs);
                    PlaySoundSpatial(SFX_BigFireball, NPC[A].Location);
                }
            }
            else if(NPC[A].Special3 < 45)
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 0;
                else
                    NPC[A].Frame = 5;
            }
            else
                NPC[A].Special4 = 0;
        }
    }
    else if(NPC[A].Type == NPCID_HEAVY_THROWER && NPC[A].HoldingPlayer > 0)
    {
        // the throw counter was previously Special3, but it uses a double in the non-held logic, so it has been moved to SpecialX
        if(Player[NPC[A].HoldingPlayer].Effect == PLREFF_NORMAL)
            NPC[A].SpecialX += 1;

        if(NPC[A].SpecialX >= 20)
        {
            PlaySoundSpatial(SFX_HeavyToss, NPC[A].Location);
            NPC[A].SpecialX = 0; // -15
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 32;
            NPC[numNPCs].Location.X = NPC[A].Location.X;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y;
            NPC[numNPCs].Direction = NPC[A].Direction;
            NPC[numNPCs].Type = NPCID_HEAVY_THROWN;
            NPC[numNPCs].Shadow = NPC[A].Shadow;
            NPC[numNPCs].CantHurt = 200;
            NPC[numNPCs].CantHurtPlayer = NPC[A].HoldingPlayer;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Projectile = true;
            NPC[numNPCs].TimeLeft = 50;
            NPC[numNPCs].Location.SpeedY = -8;
            NPC[numNPCs].Location.SpeedX = 3 * Player[NPC[A].HoldingPlayer].Direction + Player[NPC[A].HoldingPlayer].Location.SpeedX * 0.8;
            syncLayers_NPC(numNPCs);
        }
    }
    else if(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM) // Bullet Bill Shooter
    {
        if(NPC[A].Type == NPCID_CANNONENEMY)
        {
            NPC[A].Special += 1;
            if(NPC[A].HoldingPlayer > 0)
            {
                if(Player[NPC[A].HoldingPlayer].Effect == PLREFF_NORMAL)
                    NPC[A].Special += 6;
            }
        }
        else
        {
            int shootStep = 10;
            int shootStepSpin = 20;
            int shootStepCar = 5;
            bool keepProjectile = false;

            int shootBehavior = NPC[A].Variant;

            switch(shootBehavior)
            {
            default:
            case 0:
                // SMBX 1.2.1 and newer (shoot fast, don't shoot while projectile)
                break;
            case 1:
                // SMBX 1.2 (shoot fast, keep shoot while projectile)
                keepProjectile = true;
                break;
            case 2:
                // SMBX older than 1.2 (shoot slow, keep shoot while projectile)
                keepProjectile = true;
                shootStep = 5;
                shootStepSpin = 10;
                break;
            }

            if(NPC[A].HoldingPlayer > 0)
            {
                if(Player[NPC[A].HoldingPlayer].SpinJump)
                {
                    if(NPC[A].Direction != Player[NPC[A].HoldingPlayer].SpinFireDir)
                    {
                        if(Player[NPC[A].HoldingPlayer].Effect == PLREFF_NORMAL)
                            NPC[A].Special += shootStepSpin;
                    }
                }
                else
                {
                    if(Player[NPC[A].HoldingPlayer].Effect == PLREFF_NORMAL)
                        NPC[A].Special += shootStep;
                }
            }
            else if(NPC[A].vehiclePlr > 0)
                NPC[A].Special += shootStepCar;
            else if(NPC[A].Projectile && keepProjectile)
                NPC[A].Special += shootStep;
        }

        if(NPC[A].Special >= 200)
        {
            double C = 0;

            if(NPC[A].HoldingPlayer > 0)
            {
                if(Player[NPC[A].HoldingPlayer].SpinJump)
                    Player[NPC[A].HoldingPlayer].SpinFireDir = int(NPC[A].Direction);
            }

            if(NPC[A].HoldingPlayer == 0 && NPC[A].vehiclePlr == 0 && NPC[A].Type == NPCID_CANNONENEMY)
            {
                C = 0;
                for(int B = 1; B <= numPlayers; B++)
                {
                    if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                    {
                        double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                        if(C == 0.0 || dist < C)
                        {
                            C = dist;
                            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                                NPC[A].Direction = -1;
                            else
                                NPC[A].Direction = 1;
                            if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                C = -1;
                        }
                    }
                }
            }

            if(numNPCs < maxNPCs)
            {
                if(fEqual(C, -1) && NPC[A].HoldingPlayer == 0 && NPC[A].vehiclePlr == 0)
                    NPC[A].Special = 0;
                else if(Player[NPC[A].vehiclePlr].Controls.Run || NPC[A].vehiclePlr == 0)
                {
                    NPC[A].Special = 0;
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Inert = NPC[A].Inert;
                    bool tempBool = false;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].DefaultDirection = NPC[A].Direction;
                    if(NPC[A].HoldingPlayer > 0 || NPC[A].vehiclePlr > 0 || (NPC[A].Type == NPCID_CANNONITEM && NPC[A].Projectile))
                    {
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].CantHurt = 10000;
                        NPC[numNPCs].CantHurtPlayer = NPC[A].HoldingPlayer;
                        NPC[numNPCs].Location.SpeedX = 8 * NPC[numNPCs].Direction;
                    }
                    else if(NPC[A].CantHurtPlayer > 0)
                    {
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].CantHurt = 1000;
                        NPC[numNPCs].CantHurtPlayer = NPC[A].CantHurtPlayer;
                    }
                    else if(NPC[A].Type == NPCID_CANNONITEM)
                    {
                        tempBool = true;
                        numNPCs--;
                    }

                    if(!tempBool)
                    {
                        NPC[numNPCs].Shadow = NPC[A].Shadow;
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].JustActivated = 0;
                        NPC[numNPCs].Section = NPC[A].Section;
                        NPC[numNPCs].Type = NPCID_BULLET;
                        NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                        NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;

                        if(NPC[numNPCs].Direction > 0)
                            NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                        else
                            NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width;

                        if(NPC[numNPCs].Direction > 0)
                            NPC[numNPCs].Frame = 1;
                        else
                            NPC[numNPCs].Frame = 0;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - NPC[numNPCs].Location.Height / 2.0;
                        syncLayers_NPC(numNPCs);

                        Location_t tempLocation = NPC[numNPCs].Location;
                        tempLocation.X = NPC[numNPCs].Location.X + (NPC[numNPCs].Location.Width / 2.0) * NPC[numNPCs].Direction;
                        tempLocation.Y = NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                        NewEffect(EFFID_SMOKE_S3, tempLocation);

                        PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
                    }
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_TOOTHY)
    {
        int B = 0;
        if(NPC[A].Special > 0)
        {
            if(Player[NPC[A].Special].HoldingNPC > 0)
            {
                if(NPC[Player[NPC[A].Special].HoldingNPC].Type == 49)
                    B = 1;
            }
        }
        else if(NPC[NPC[A].Special2].Projectile && NPC[NPC[A].Special2].Active)
        {
            B = 1;
            NPC[A].Projectile = true;
            NPC[A].Direction = NPC[NPC[A].Special2].Direction;
            if(NPC[A].Direction > 0)
                NPC[A].Location.X = NPC[NPC[A].Special2].Location.X + 32;
            else
                NPC[A].Location.X = NPC[NPC[A].Special2].Location.X - NPC[A].Location.Width;
            NPC[A].Location.Y = NPC[NPC[A].Special2].Location.Y;
        }

        if(Player[NPC[A].vehiclePlr].Controls.Run)
            B = 1;

        if(NPC[A].Special2 > 0 && NPC[NPC[A].Special2].Special2 != A)
            B = 0;

        if(NPC[A].Special > 0)
        {
            if(Player[NPC[A].Special].Effect != PLREFF_NORMAL)
                B = 0;
        }

        if(B == 0)
        {
            NPC[A].Killed = 9;
            NPCQueues::Killed.push_back(A);
        }
    }
    else if(NPC[A].Type == NPCID_TOOTHYPIPE)
    {
        if(NPC[A].HoldingPlayer == 0 && NPC[A].vehiclePlr == 0)
            NPC[A].Special = 0;

        if(NPC[A].HoldingPlayer > 0 && NPC[A].Special2 > 0)
            NPC[NPC[A].Special2].Direction = NPC[A].Direction;

        if(Player[NPC[A].HoldingPlayer].Effect != PLREFF_NORMAL)
            NPC[A].Special = 0;
#if 0
        // Important: this also makes a thrown handheld plant harm NPCs, so it is a major balance change.
        // Since it was disabled in SMBX code, better not to change it. -- ds-sloth
        // In original game, this is a dead code because of "And 0" condition at end.
        // In this sample, the "& false" was been commented
        // This code makes Toothy shown off the pipe when the pipe is a projectile, shooted by generator
        if(NPC[A].Projectile && NPC[A].Special2 == 0.0 && NPC[A].Special == 0.0 /*&& false*/)
        {
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[A].Special2 = numNPCs;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Type = NPCID_TOOTHY;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 48;
            NPC[numNPCs].Special = 0;
            NPC[numNPCs].Special2 = A;
            NPC[numNPCs].Projectile = true;
            NPC[numNPCs].Direction = NPC[A].Direction;
            if(NPC[numNPCs].Direction > 0)
            {
                NPC[numNPCs].Location.X = NPC[A].Location.X + 32;
                NPC[numNPCs].Frame = 2;
            }
            else
                NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[numNPCs].Location.Width;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y;
            syncLayers_NPC(numNPCs);
        }
#endif

        if(NPC[NPC[A].Special2].Type == NPCID_TOOTHY && NPC[NPC[A].Special2].Special2 == A)
        {
            NPC[NPC[A].Special2].Projectile = true;
            NPC[NPC[A].Special2].Direction = NPC[A].Direction;
            if(NPC[A].Direction > 0)
                NPC[NPC[A].Special2].Location.X = NPC[A].Location.X + 32;
            else
                NPC[NPC[A].Special2].Location.X = NPC[A].Location.X - NPC[NPC[A].Special2].Location.Width;
            NPC[NPC[A].Special2].Location.Y = NPC[A].Location.Y;

            treeNPCUpdate(NPC[A].Special2);
            if(NPC[NPC[A].Special2].tempBlock != 0)
                treeNPCSplitTempBlock(NPC[A].Special2);
        }

        if(NPC[A].vehiclePlr > 0 && !Player[NPC[A].vehiclePlr].Controls.Run)
            NPC[A].Special = 0;
    }
    else if(NPC[A].Type == NPCID_KEY)
    {
        if(NPC[A].HoldingPlayer > 0)
            KeyholeCheck(NPC[A].HoldingPlayer, NPC[A].Location);
    }
    else if(NPCIsABot(NPC[A]))
    {
        if(NPC[A].Projectile || NPC[A].HoldingPlayer > 0)
        {
            NPC[A].Special = -1;
            NPC[A].Special2 = 0;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98;
        }
        else
        {
            if(NPC[A].Special == 0)
            {
                double C = 0;
                int D = 1;
                for(int B = 1; B <= numPlayers; B++)
                {
                    if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                    {
                        double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                        if(C == 0.0 || dist < C)
                        {
                            C = dist;
                            D = B;
                        }
                    }
                }

                if(Player[D].Location.X + Player[D].Location.Width / 2.0 > NPC[A].Location.X + 16)
                    NPC[A].Direction = 1;
                else
                    NPC[A].Direction = -1;
            }

            if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
            {
                NPC[A].Location.SpeedX = 0;

                if(NPC[A].Special == 0)
                    NPC[A].Special = iRand(3) + 1;

                if(NPC[A].Special == 1)
                {
                    NPC[A].FrameCount += 1;
                    NPC[A].Special2 += 1;
                    NPC[A].Location.SpeedX = 0;
                    if(NPC[A].Special2 >= 90)
                    {
                        NPC[A].Special2 = 0;
                        NPC[A].Special = -1;
                        NPC[A].Location.SpeedY = -7;
                        NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
                    }
                }
                else if(NPC[A].Special == 3)
                {
                    NPC[A].FrameCount += 1;
                    NPC[A].Special2 += 30;
                    NPC[A].Location.SpeedX = 0;
                    if(NPC[A].Special2 >= 30)
                    {
                        NPC[A].Special2 = 0;
                        NPC[A].Special = -1;
                        NPC[A].Location.SpeedY = -3;
                        NPC[A].Location.SpeedX = 2.5 * NPC[A].Direction;
                    }
                }
                else if(NPC[A].Special == 2)
                {
                    NPC[A].Location.SpeedX = 0.5 * NPC[A].Direction;
                    NPC[A].Special2 += 1;
                    if(NPC[A].Special2 == 120)
                    {
                        NPC[A].Special2 = 0;
                        NPC[A].Special = -2;
                    }
                }
                else
                {
                    NPC[A].Special2 += 1;
                    if(NPC[A].Special2 == 30)
                    {
                        NPC[A].Special2 = 0;
                        NPC[A].Special = 0;
                    }
                }
            }
        }
    }
}
