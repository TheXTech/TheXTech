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
#include "sound.h"
#include "player.h"
#include "config.h"
#include "effect.h"
#include "eff_id.h"

void PlayerMovementX(int A, float& cursed_value_C)
{
    // Modify player's speed if he is running up/down hill
    float speedVar = 1; // Speed var is a percentage of the player's speed
    if(Player[A].Slope > 0)
    {
        if(
                (Player[A].Location.SpeedX > 0 && BlockSlope[Block[Player[A].Slope].Type] == -1) ||
                (Player[A].Location.SpeedX < 0 && BlockSlope[Block[Player[A].Slope].Type] == 1)
                )
            speedVar = (float)(1 - Block[Player[A].Slope].Location.Height / Block[Player[A].Slope].Location.Width * 0.5);
        else if(!Player[A].Slide)
            speedVar = (float)(1 + (Block[Player[A].Slope].Location.Height / Block[Player[A].Slope].Location.Width * 0.5) * 0.5);
    }

    if(Player[A].Stoned) // if statue form reset to normal
        speedVar = 1;

    if(Player[A].Character == 3)
        speedVar = (speedVar * 0.93f);

    if(Player[A].Character == 4)
        speedVar = (speedVar * 1.07f);

    // modify speedvar to slow the player down under water
    if(Player[A].Wet > 0)
    {
        if(Player[A].Location.SpeedY == 0.0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
            speedVar = (float)(speedVar * 0.25f); // if walking go really slow
        else
            speedVar = (float)(speedVar * 0.5f); // if swimming go slower faster the walking
    }


    // ducking for link
    if(Player[A].Duck && Player[A].WetFrame)
    {
        if(Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0)
            UnDuck(Player[A]);
    }

    // the following code controls the players ability to duck
    if(!(Player[A].Character == 5 && ((Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0) || Player[A].FireBallCD != 0))) // Link can't duck/unduck in air
    {
        if(Player[A].Controls.Down && !Player[A].SpinJump &&
           !Player[A].Stoned && Player[A].Vine == 0 && !Player[A].Slide &&
           (Player[A].Slope == 0 || Player[A].Mount > 0 || Player[A].WetFrame ||
            Player[A].Character >= 3 || Player[A].GrabTime > 0) &&
           ((!Player[A].WetFrame || Player[A].Character >= 3) ||
            Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 ||
            Player[A].Slope != 0 || Player[A].Mount == 1) &&
           !Player[A].Fairy && !Player[A].ShellSurf && !Player[A].Driving)
        {
            Player[A].Bumped = false;
            if(Player[A].Mount != 2) // cant duck in the clown car
            {
                if(Player[A].Mount == 3) // duck on a yoshi
                {
                    if(!Player[A].Duck)
                    {
                        Player[A].Location.Y += Player[A].Location.Height;
                        Player[A].Location.Height = 31;
                        Player[A].Location.Y += -Player[A].Location.Height;
                        Player[A].Duck = true;
                        // If nPlay.Online = True And A = nPlay.MySlot + 1 Then Netplay.sendData Netplay.PutPlayerLoc(nPlay.MySlot) & "1q" & A & LB
//                                        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                                            Netplay::sendData "1q" + std::to_string(A) + LB;
                    }
                }
                else // normal duck
                {
                    if((Player[A].State > 1 && Player[A].HoldingNPC <= 0) || (Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5))
                    {
                        if(!Player[A].Duck && Player[A].TailCount == 0) // Player ducks
                        {
                            if(Player[A].Character == 5)
                                Player[A].SwordPoke = 0;
                            Player[A].Duck = true;
                            Player[A].Location.Y += Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y += -Player[A].Location.Height;
//                                            if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                                                Netplay::sendData "1q" + std::to_string(A) + LB;
                        }
                    }
                    else if(Player[A].Mount == 1)
                    {
                        if(!Player[A].Duck && Player[A].TailCount == 0) // Player ducks
                        {
                            Player[A].Duck = true;
                            Player[A].Location.Height = Physics.PlayerDuckHeight[1][2];
                            Player[A].Location.Y += -Physics.PlayerDuckHeight[1][2] + Physics.PlayerHeight[1][2];
//                                            if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                                                Netplay::sendData "1q" + std::to_string(A) + LB;
                        }
                    }
                }
            }
        }
        else
        {
            if(Player[A].Duck)
                UnDuck(Player[A]);
        }
    }

    cursed_value_C = 1;
    // If .Character = 5 Then C = 0.94
    if(Player[A].Character == 5)
        cursed_value_C = 0.95F;
    if(Player[A].Controls.Left &&
       ((!Player[A].Duck && Player[A].GrabTime == 0) ||
        (Player[A].Location.SpeedY != 0.0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) ||
        Player[A].Mount == 1)
    )
    {
        Player[A].Bumped = false;
        if(Player[A].Controls.Run || Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
        {
            if(Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar * cursed_value_C)
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += 0.1 * 0.175;
                if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += 0.05 * 0.175;
                if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += -0.05 * 0.175;
                Player[A].Location.SpeedX += -0.1 * speedVar;
            }
            else // Running
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += 0.05 * 0.175;
                if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += 0.025 * 0.175;
                if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += -0.025 * 0.175;
                if(Player[A].Character == 5) // Link
                    Player[A].Location.SpeedX += -0.025 * speedVar;
                else // Mario
                    Player[A].Location.SpeedX += -0.05 * speedVar;
            }

            if(Player[A].Location.SpeedX > 0)
            {
                Player[A].Location.SpeedX -= 0.18;
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += 0.18 * 0.29;
                if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += 0.09 * 0.29;
                if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += -0.09 * 0.29;
                if(SuperSpeed)
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
            }
        }

        if(SuperSpeed && Player[A].Controls.Run)
            Player[A].Location.SpeedX -= 0.1;
    }
    else if(Player[A].Controls.Right && ((!Player[A].Duck && Player[A].GrabTime == 0) || (Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) || Player[A].Mount == 1))
    {
        Player[A].Bumped = false;
        if(Player[A].Controls.Run || Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
        {
            if(Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * speedVar * cursed_value_C)
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += -0.1 * 0.175;
                if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += -0.05 * 0.175;
                if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += 0.05 * 0.175;
                Player[A].Location.SpeedX += 0.1 * speedVar;
            }
            else
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += -0.05 * 0.175;
                if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += -0.025 * 0.175;
                if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += 0.025 * 0.175;
                if(Player[A].Character == 5) // Link
                    Player[A].Location.SpeedX += 0.025 * speedVar;
                else // Mario
                    Player[A].Location.SpeedX += 0.05 * speedVar;
            }

            if(Player[A].Location.SpeedX < 0)
            {
                Player[A].Location.SpeedX += 0.18;
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += -0.18 * 0.29;
                if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += -0.09 * 0.29;
                if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += 0.09 * 0.29;
                if(SuperSpeed)
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
            }
        }

        if(SuperSpeed && Player[A].Controls.Run)
            Player[A].Location.SpeedX += 0.1;
    }
    else
    {
        if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || Player[A].WetFrame) // Only lose speed when not in the air
        {
            if(Player[A].Location.SpeedX > 0)
                Player[A].Location.SpeedX += -0.07 * speedVar;
            if(Player[A].Location.SpeedX < 0)
                Player[A].Location.SpeedX += 0.07 * speedVar;
            if(Player[A].Character == 2) // LUIGI
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 1.003;
            if(Player[A].Character == 3) // PEACH
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 1.0015;
            if(Player[A].Character == 4) // toad
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.9985;
            if(SuperSpeed)
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
        }

        if(Player[A].Location.SpeedX > -0.18 && Player[A].Location.SpeedX < 0.18)
        {
            Player[A].Bumped = false;
            Player[A].Location.SpeedX = 0;
        }
    }

    if(Player[A].Location.SpeedX < -16)
        Player[A].Location.SpeedX = -16;
    else if(Player[A].Location.SpeedX > 16)
        Player[A].Location.SpeedX = 16;

    if(Player[A].WarpShooted &&
       Player[A].Location.SpeedX < Physics.PlayerRunSpeed * speedVar &&
       Player[A].Location.SpeedX > -Physics.PlayerRunSpeed * speedVar)
    {
        Player[A].WarpShooted = false;
    }

    if(!Player[A].WarpShooted && (Player[A].Controls.Run || Player[A].Character == 5))
    {
        if(Player[A].Location.SpeedX >= Physics.PlayerRunSpeed * speedVar)
        {
            if(!SuperSpeed)
                Player[A].Location.SpeedX = Physics.PlayerRunSpeed * speedVar;
        }
        else if(Player[A].Location.SpeedX <= -Physics.PlayerRunSpeed * speedVar)
        {
            if(!SuperSpeed)
                Player[A].Location.SpeedX = -Physics.PlayerRunSpeed * speedVar;
        }
//                        else  // REDURANT GARBAGE
//                        {
//                        }
    }
    else
    {
        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 0.1 * speedVar)
            Player[A].Location.SpeedX -= 0.1;
        else if(Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 0.1 * speedVar)
            Player[A].Location.SpeedX += 0.1;
        else if(std::abs(Player[A].Location.SpeedX) > Physics.PlayerWalkSpeed * speedVar)
        {
            if(Player[A].Location.SpeedX > 0)
                Player[A].Location.SpeedX = Physics.PlayerWalkSpeed * speedVar;
            else
                Player[A].Location.SpeedX = -Physics.PlayerWalkSpeed * speedVar;
        }
    }

    if(Player[A].Mount == 1 && Player[A].MountType == 3)
    {
        Player[A].CanFly2 = true;
        Player[A].FlyCount = 1000;
    }

    if(Player[A].Mount != 3)
        Player[A].YoshiBlue = false;

    if(FlyForever && !Player[A].GroundPound)
    {
        if(Player[A].Mount == 3)
            Player[A].YoshiBlue = true;

        if((Player[A].State == 4 || Player[A].State == 5) || (Player[A].YoshiBlue && Player[A].Mount == 3) || (Player[A].Mount == 1 && Player[A].MountType == 3))
            Player[A].CanFly2 = true;
        else
        {
            Player[A].CanFly2 = false;
            Player[A].CanFly = false;
            Player[A].FlyCount = 0;
            Player[A].YoshiBlue = false;
        }
    }

    // Racoon/Tanooki Mario.  this handles the ability to fly after running
    if((Player[A].State == 4 || Player[A].State == 5) && Player[A].Wet == 0)
    {
        if( (Player[A].Location.SpeedY == 0.0 ||
             Player[A].CanFly2 ||
             Player[A].StandingOnNPC != 0 ||
             Player[A].Slope > 0) &&
            (std::abs(Player[A].Location.SpeedX) >= double(Physics.PlayerRunSpeed) ||
            (Player[A].Character == 3 && std::abs(Player[A].Location.SpeedX) >= 5.58 - 0.001))) // Rounding error of SpeedX makes an evil here
        {
            Player[A].RunCount += 1;
        }
        else
        {
            if(!(std::abs(Player[A].Location.SpeedX) >= double(Physics.PlayerRunSpeed) ||
                 (Player[A].Character == 3 && std::abs(Player[A].Location.SpeedX) >= 5.58 - 0.001)) )
            {
                Player[A].RunCount -= 0.3f;
            }
        }

        if(Player[A].RunCount >= 35 && Player[A].Character == 1)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 35;
        }
        else if(Player[A].RunCount >= 40 && Player[A].Character == 2)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 40;
        }
        else if(Player[A].RunCount >= 80 && Player[A].Character == 3)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 80;
        }
        else if(Player[A].RunCount >= 60 && Player[A].Character == 4)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 60;
        }
        else if(Player[A].RunCount >= 10 && Player[A].Character == 5) // link flying
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 10;
        }
        else
        {
            Player[A].CanFly = false;
            if(Player[A].RunCount < 0)
                Player[A].RunCount = 0;
        }
    }

    if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0)
        Player[A].FlyCount = 1;

    if(Player[A].FlyCount > 1)
        Player[A].FlyCount -= 1;
    else if(Player[A].FlyCount == 1)
    {
        Player[A].CanFly2 = false;
        Player[A].FlyCount = 0;
    }
}

void PlayerMovementY(int A)
{
    if(Player[A].Mount == 1) // this gives the player the bounce when in the kurbio's shoe
    {
        if(Player[A].Controls.Left || Player[A].Controls.Right)
        {
            if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
            {
                if(Player[A].Controls.Left && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                    Player[A].Location.SpeedY = -4.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else if(Player[A].Controls.Right && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
                    Player[A].Location.SpeedY = -4.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else
                    PlaySoundSpatial(SFX_Skid, Player[A].Location);
                Player[A].MountSpecial = 1;
            }
        }

        if(Player[A].Location.SpeedY < -4.1)
            Player[A].MountSpecial = 0;
        else if(Player[A].Location.SpeedY > 4.1)
            Player[A].MountSpecial = 0;

        if(Player[A].Controls.Jump && Player[A].MountSpecial == 1 && Player[A].CanJump)
        {
            Player[A].Location.SpeedY = 0;
            Player[A].StandUp = true;
        }
    }

    if(Player[A].Mount == 1)
    {
        if(Player[A].Controls.AltJump && Player[A].CanAltJump) // check to see if the player should jump out of the shoe
            PlayerDismount(A);
    }
    else if(Player[A].Mount == 3)
    {
        if(Player[A].Controls.AltJump && Player[A].CanAltJump) // jump off of yoshi
            PlayerDismount(A);
    }

    if((Player[A].Location.SpeedY == 0.0 || Player[A].Jump > 0 || Player[A].Vine > 0) && Player[A].FloatTime == 0) // princess float
        Player[A].CanFloat = true;

    if(Player[A].Wet > 0 || Player[A].WetFrame)
        Player[A].CanFloat = false;

    // handles the regular jump
    if(Player[A].Controls.Jump || (Player[A].Controls.AltJump &&
       ((Player[A].Character > 2 && Player[A].Character != 4) || Player[A].Quicksand > 0 || g_config.disable_spin_jump) &&
       Player[A].CanAltJump))
    {
        double tempSpeed;
        if(Player[A].Location.SpeedX > 0)
            tempSpeed = Player[A].Location.SpeedX * 0.2; // tempSpeed gives the player a height boost when jumping while running, based off his SpeedX
        else
            tempSpeed = -Player[A].Location.SpeedX * 0.2;

        if(Player[A].ShellSurf) // this code modifies the jump based on him riding a shell
        {
            if(NPC[Player[A].StandingOnNPC].Location.SpeedY == 0 || NPC[Player[A].StandingOnNPC].Slope > 0)
            {
                if(Player[A].CanJump)
                {
                    PlaySoundSpatial(SFX_Whip, Player[A].Location); // Jump sound
                    Player[A].Jump = Physics.PlayerJumpHeight * 0.6;
                    NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9;
                }
            }
            else if(Player[A].Jump > 0)
                NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9;
        }
        // if not surfing a shell then proceed like normal
        else
        {
            if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 ||
                MultiHop || Player[A].Slope > 0 || (Player[A].Location.SpeedY > 0 && Player[A].Quicksand > 0)) && Player[A].CanJump)
            {
                PlaySoundSpatial(SFX_Jump, Player[A].Location); // Jump sound
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                Player[A].Jump = Physics.PlayerJumpHeight;

                if(Player[A].Character == 4 && (Player[A].State == 4 || Player[A].State == 5) && !Player[A].SpinJump)
                    Player[A].DoubleJump = true;

                if(Player[A].Character == 2)
                    Player[A].Jump += 3;

                if(Player[A].SpinJump)
                    Player[A].Jump -= 6;

                if(Player[A].StandingOnNPC > 0 && !FreezeNPCs)
                {
                    if(NPC[Player[A].StandingOnNPC].Type != 91)
                        Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX;
                }

                Player[A].StandingOnNPC = 0; // the player can't stand on an NPC after jumping

                if(Player[A].CanFly) // let's the player fly if the conditions are met
                {
                    Player[A].StandingOnNPC = 0;
                    Player[A].Jump = 30;
                    if(Player[A].Character == 2)
                        Player[A].Jump += 3;
                    if(Player[A].SpinJump)
                        Player[A].Jump -= 6;
                    Player[A].CanFly = false;
                    Player[A].RunCount = 0;
                    Player[A].CanFly2 = true;

                    if(Player[A].Character == 2) // luigi doesn't fly as long as mario
                        Player[A].FlyCount = 300; // Length of flight time
                    else if(Player[A].Character == 3) // special handling for peach
                    {
                        Player[A].FlyCount = 0;
                        Player[A].RunCount = 80;
                        Player[A].CanFly2 = false;
                        Player[A].Jump = 70;
                        Player[A].CanFloat = true;
                        Player[A].FlySparks = true;
                    }
#if 0
                    // FIXME: Duplicated "Character == 3" condition branch [PVS Studio]
                    else if(Player[A].Character == 3) // special handling for peach
                        Player[A].FlyCount = 280; // Length of flight time
#endif
                    else
                        Player[A].FlyCount = 320; // Length of flight time
                }
            }
            else if(Player[A].Jump > 0) // controls the height of the jump
            {
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                if(Player[A].Jump > 20)
                {
                    if(Player[A].Jump > 40)
                        Player[A].Location.SpeedY += -(40 - 20) * 0.2;
                    else
                        Player[A].Location.SpeedY += -(Player[A].Jump - 20) * 0.2;
                }
            }
            else if(Player[A].CanFly2)
            {
                if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
                {
                    Player[A].Location.SpeedY -= 1;
                    Player[A].CanPound = true;
                    if(Player[A].YoshiBlue || (Player[A].Mount == 1 && Player[A].MountType == 3))
                        PlaySoundSpatial(SFX_PetTongue, Player[A].Location);
                }
            }
        }
        Player[A].CanJump = false;
    }
    else
        Player[A].CanJump = true;

    if(Player[A].Jump > 0)
        Player[A].Slope = 0;

    if(Player[A].SpinJump || (Player[A].State != 4 && Player[A].State != 5) || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].Location.SpeedY == 0)
        Player[A].DoubleJump = false;

    // double jump code
    if(Player[A].DoubleJump && Player[A].Jump == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 &&
       Player[A].StandingOnNPC == 0 && Player[A].Wet == 0 && Player[A].Vine == 0 &&
       !Player[A].WetFrame && !Player[A].Fairy && !Player[A].CanFly2)
    {
        if(Player[A].Controls.Jump && Player[A].JumpRelease)
        {
            PlaySoundSpatial(SFX_Jump, Player[A].Location);
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
            Player[A].Jump = 10;
            Player[A].DoubleJump = false;
            Location_t tempLocation = Player[A].Location;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - EffectHeight[80] / 2.0 + Player[A].Location.SpeedY;
            tempLocation.Height = EffectHeight[80];
            tempLocation.Width = EffectWidth[80];
            tempLocation.X = Player[A].Location.X;

            for(int B = 1; B <= 10; B++)
            {
                NewEffect(EFFID_SPARKLE, tempLocation);
                Effect[numEffects].Location.SpeedX = (dRand() * 3) - 1.5;
                Effect[numEffects].Location.SpeedY = (dRand() * 0.5) + (1.5 - std::abs(Effect[numEffects].Location.SpeedX)) * 0.5;
                Effect[numEffects].Location.SpeedX += -Player[A].Location.SpeedX * 0.2;
            }
        }
    }



    if(Player[A].NoShellKick > 0) // countdown for the next time the player kicks a turtle shell
        Player[A].NoShellKick--;

    if(Player[A].ShellSurf)
    {
        if(Player[A].Mount != 0)
            Player[A].ShellSurf = false;

        // FIXME: SOME DEAD CODE BECAUSE OF "1 == 2"
#if 0
        if(Player[A].Direction != NPC[Player[A].StandingOnNPC].Direction && 1 == 2)
        {
            Player[A].ShellSurf = false;
            Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
            if(Player[A].Location.SpeedY > 0)
                Player[A].Location.SpeedY = 0;
            PlaySoundSpatial(SFX_Skid, Player[A].Location);
            NPC[Player[A].StandingOnNPC].CantHurt = 30;
            NPC[Player[A].StandingOnNPC].CantHurtPlayer = A;
            Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX / 2;
            Player[A].StandingOnNPC = 0;
            Player[A].NoShellKick = 30;
        }
        else
#endif
        {
            if(iRand(10) >= 3)
            {
                Location_t tempLocation;
                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2 + dRand() * (NPC[Player[A].StandingOnNPC].Location.Height - 8) + 4;
                tempLocation.X = Player[A].Location.X - 4 + dRand() * (Player[A].Location.Width - 8) + 4 - 8 * Player[A].Direction;
                NewEffect(EFFID_SPARKLE, tempLocation, 1, 0, ShadowMode);
                Effect[numEffects].Frame = iRand(3);
                Effect[numEffects].Location.SpeedY = (Player[A].Location.Y + Player[A].Location.Height + NPC[Player[A].StandingOnNPC].Location.Height / 32.0 - tempLocation.Y + 12) * 0.05;
            }
        }

        if(NPC[Player[A].StandingOnNPC].Wet == 2)
        {
            if(NPC[Player[A].StandingOnNPC].Type == 195)
                NPC[Player[A].StandingOnNPC].Special4 = 1;
            NPC[Player[A].StandingOnNPC].Location.SpeedY += -Physics.NPCGravity * 1.5;
        }
    }

    // START ALT JUMP - this code does the player's spin jump
    if(Player[A].Controls.AltJump && (Player[A].Character == 1 || Player[A].Character == 2 || Player[A].Character == 4 ||
                                      (g_config.fix_char3_escape_shell_surf && Player[A].Character == 3 && Player[A].ShellSurf))
                                  && (!g_config.disable_spin_jump || Player[A].ShellSurf))
    {
        double tempSpeed;
        if(Player[A].Location.SpeedX > 0)
            tempSpeed = Player[A].Location.SpeedX * 0.2;
        else
            tempSpeed = -Player[A].Location.SpeedX * 0.2;

        if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || MultiHop) && Player[A].CanAltJump) // Player Jumped
        {
            if(!Player[A].Duck)
            {
                Player[A].Slope = 0;
                Player[A].SpinFireDir = Player[A].Direction;
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                Player[A].Jump = Physics.PlayerJumpHeight;
                if(Player[A].Character == 2)
                    Player[A].Jump += 3;

                if(Player[A].StandingOnNPC > 0 && !FreezeNPCs)
                {
                    if(NPC[Player[A].StandingOnNPC].Type != 91)
                        Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX;
                }

                PlaySoundSpatial(SFX_Whip, Player[A].Location); // Jump sound
                Player[A].Jump -= 6;
                if(Player[A].Direction == 1)
                    Player[A].SpinFrame = 0;
                else
                    Player[A].SpinFrame = 6;
                Player[A].SpinJump = true;
//                                    if(nPlay.Online == true && nPlay.MySlot + 1 == A)
//                                        Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1l" + std::to_string(A) + LB;
                if(Player[A].Duck)
                    UnDuck(Player[A]);

                if(Player[A].ShellSurf)
                {
                    Player[A].ShellSurf = false;
                    Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed * 0.8;
                    Player[A].Jump = 0;

                    if(g_config.disable_spin_jump)
                        Player[A].SpinJump = false;
                }

                Player[A].StandingOnNPC = 0;

                if(Player[A].CanFly)
                {
                    Player[A].StandingOnNPC = 0;
                    Player[A].Jump = 30;
                    if(Player[A].Character == 2)
                        Player[A].Jump += 3;
                    if(Player[A].SpinJump)
                        Player[A].Jump -= 6;
                    Player[A].CanFly = false;
                    Player[A].RunCount = 0;
                    Player[A].CanFly2 = true;
                    Player[A].FlyCount = 150; // Length of flight time
                }
            }
        }
        else if(Player[A].Jump > 0)
        {
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
            if(Player[A].Jump > 20)
                Player[A].Location.SpeedY += -(Player[A].Jump - 20) * 0.2;
        }
        else if(Player[A].CanFly2)
        {
            if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
            {
                Player[A].Location.SpeedY -= 1;
                Player[A].CanPound = true;
                if(Player[A].YoshiBlue)
                    PlaySoundSpatial(SFX_PetTongue, Player[A].Location);
            }
        }
        // End If
        Player[A].CanAltJump = false;

    }
    else
        Player[A].CanAltJump = true;
    // END ALT JUMP


    if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && Player[A].SpinJump)
    {
        Player[A].SpinJump = false;
//                            if(nPlay.Online == true && nPlay.MySlot + 1 == A)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1m" + std::to_string(A) + LB;
        Player[A].TailCount = 0;
    }

    if(Player[A].Mount > 0)
        Player[A].SpinJump = false;

    if(!Player[A].Controls.AltJump && !Player[A].Controls.Jump)
        Player[A].Jump = 0;

    if(Player[A].Jump > 0)
        Player[A].Jump -= 1;

    if(Player[A].Jump > 0)
        Player[A].Vine = 0;


    if(Player[A].Quicksand > 1)
    {
        Player[A].Slide = false;
        if(Player[A].Location.SpeedY < -0.7)
        {
            Player[A].Location.SpeedY = -0.7;
            Player[A].Jump -= 1;
        }
        else if(Player[A].Location.SpeedY < 0)
        {
            Player[A].Location.SpeedY += 0.1;
            Player[A].Jump = 0;
        }

        if(Player[A].Location.SpeedY >= 0.1)
            Player[A].Location.SpeedY = 0.1;
        Player[A].Location.Y += Player[A].Location.SpeedY;
    }


    // gravity
    if(Player[A].Vine == 0)
    {
        if(Player[A].NoGravity == 0)
        {
            if(Player[A].Character == 2)
                Player[A].Location.SpeedY += Physics.PlayerGravity * 0.9;
            else
                Player[A].Location.SpeedY += Physics.PlayerGravity;

            if(Player[A].HoldingNPC > 0)
            {
                if(NPC[Player[A].HoldingNPC].Type == 278 || NPC[Player[A].HoldingNPC].Type == 279)
                {
                    if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                    {
                        if(Player[A].Character == 2)
                            Player[A].Location.SpeedY += -Physics.PlayerGravity * 0.9 * 0.8;
                        else
                            Player[A].Location.SpeedY += -Physics.PlayerGravity * 0.8;

                        if(Player[A].Location.SpeedY > Physics.PlayerGravity * 3)
                            Player[A].Location.SpeedY = Physics.PlayerGravity * 3;
                    }
                    else
                        NPC[Player[A].HoldingNPC].Special = 0;
                }
            }

            if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;
        }
        else
            Player[A].NoGravity -= 1;
    }

    // princess float
    if(Player[A].Character == 3 && Player[A].Wet == 0 && !Player[A].WetFrame)
    {
        if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].CanFly2)
            Player[A].CanFloat = true;
        else if(Player[A].CanFloat)
        {
            if(Player[A].Jump == 0 && ((Player[A].Controls.Jump && Player[A].FloatRelease) ||
              (Player[A].Controls.AltJump && Player[A].Location.SpeedY > 0)))
            {
                if(Player[A].State == 4 || Player[A].State == 5)
                {
                    Player[A].FloatTime = 100;
                    Player[A].FlySparks = true;
                }
                else
                    Player[A].FloatTime = 65;

                Player[A].FloatDir = 1;

                if(Player[A].Location.SpeedY < -0.5)
                    Player[A].FloatSpeed = 0.5;
                else if(Player[A].Location.SpeedY > 0.5)
                    Player[A].FloatSpeed = 0.5;
                else
                    Player[A].FloatSpeed = Player[A].Location.SpeedY;

                Player[A].CanFloat = false;
            }
        }
    }

    if(Player[A].Character == 3 && Player[A].FlySparks)
    {
        if(Player[A].FloatTime == 0 && Player[A].Location.SpeedY >= 0)
            Player[A].FlySparks = false;
    }

    if(Player[A].CanFloat)
        Player[A].FloatTime = 0;

    if(Player[A].FloatTime > 0 && Player[A].Character == 3)
    {
        if((Player[A].Controls.Jump || Player[A].Controls.AltJump) && Player[A].Vine == 0)
        {
            Player[A].FloatTime -= 1;
            Player[A].FloatSpeed += Player[A].FloatDir * 0.1;

            if(Player[A].FloatSpeed > 0.8)
                Player[A].FloatDir = -1;

            if(Player[A].FloatSpeed < -0.8)
                Player[A].FloatDir = 1;

            Player[A].Location.SpeedY = Player[A].FloatSpeed;

            if(Player[A].FloatTime == 0 && Player[A].Location.SpeedY == 0)
                Player[A].Location.SpeedY = 0.1;
        }
        else
            Player[A].FloatTime = 0;
    }


    // Racoon Mario
    if((Player[A].State == 4 || Player[A].State == 5) || Player[A].YoshiBlue || (Player[A].Mount == 1 && Player[A].MountType == 3))
    {
        if((Player[A].Controls.Jump || Player[A].Controls.AltJump) &&
          ((Player[A].Location.SpeedY > Physics.PlayerGravity * 5 && Player[A].Character != 3 && Player[A].Character != 4) ||
            (Player[A].Location.SpeedY > Physics.PlayerGravity * 10 && Player[A].Character == 3) ||
            (Player[A].Location.SpeedY > Physics.PlayerGravity * 7.5 && Player[A].Character == 4)) &&
            !Player[A].GroundPound && Player[A].Slope == 0 && Player[A].Character != 5)
        {
            if(!Player[A].ShellSurf)
            {
                if(Player[A].Character == 3)
                    Player[A].Location.SpeedY = Physics.PlayerGravity * 10;
                else if(Player[A].Character == 4)
                    Player[A].Location.SpeedY = Physics.PlayerGravity * 7.5;
                else
                    Player[A].Location.SpeedY = Physics.PlayerGravity * 5;
            }
            else
            {
                if(NPC[Player[A].StandingOnNPC].Location.SpeedY > Physics.PlayerGravity * 5)
                    NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerGravity * 5;
            }

            if(
                !(
                    (!Player[A].YoshiBlue && (Player[A].CanFly || Player[A].CanFly2)) ||
                    (Player[A].Mount == 3 && Player[A].CanFly2)
                )
            )
            {
                if(iRand(10) == 0)
                {
                    NewEffect(EFFID_SPARKLE, newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                         Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                    Effect[numEffects].Location.SpeedX = (dRand() * 0.5) - 0.25;
                    Effect[numEffects].Location.SpeedY = (dRand() * 0.5) - 0.25;
                    Effect[numEffects].Frame = 1;
                }
            }
        }
    }
}

void PlayerSwimMovementY(int A)
{
    if(Player[A].Mount == 1)
    {
        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
            PlayerDismount(A);
    }
    else if(Player[A].Mount == 3)
    {
        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
            PlayerDismount(A);
    }

    if(Player[A].Duck)
    {
        if(Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY != 0 && Player[A].Mount != 1)
        {
            if(Player[A].Character <= 2) // unduck wet players that aren't peach o toad
                UnDuck(Player[A]);
        }
    }

    Player[A].Location.SpeedY += Physics.PlayerGravity * 0.1;

    if(Player[A].Location.SpeedY >= 3) // Terminal Velocity in water
        Player[A].Location.SpeedY = 3;

    if(Player[A].Mount == 1)
    {
        if(Player[A].Controls.Left || Player[A].Controls.Right)
        {
            if(Player[A].Location.SpeedY == Physics.PlayerGravity * 0.1 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
            {
                if(Player[A].Controls.Left && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                    Player[A].Location.SpeedY = -1.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else if(Player[A].Controls.Right && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
                    Player[A].Location.SpeedY = -1.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else
                    PlaySoundSpatial(SFX_Skid, Player[A].Location);

                Player[A].MountSpecial = 1;
            }
        }

        if(Player[A].Location.SpeedY < -1.1)
            Player[A].MountSpecial = 0;
        else if(Player[A].Location.SpeedY > 1.1)
            Player[A].MountSpecial = 0;
        else if(Player[A].FloatTime >= 0)
            Player[A].MountSpecial = 0;


        if(Player[A].Controls.Jump && Player[A].MountSpecial == 1 && Player[A].CanJump)
        {
            Player[A].Location.SpeedY = Physics.PlayerGravity * 0.1;
            Player[A].MountSpecial = 0;
            Player[A].StandUp = true;
        }
    }

    if(Player[A].SwimCount > 0)
        Player[A].SwimCount -= 1;

    if(Player[A].SwimCount == 0)
    {
        if(Player[A].Mount != 1 || Player[A].Location.SpeedY == Physics.PlayerGravity * 0.1 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0)
        {
            if((Player[A].Controls.Jump && Player[A].CanJump) ||
               (Player[A].Controls.AltJump && Player[A].CanAltJump))
            {
                if(Player[A].Duck && Player[A].Mount != 1 && Player[A].Character <= 2)
                    UnDuck(Player[A]);

                if(Player[A].Slope != 0)
                    Player[A].Location.SpeedY = 0;

                Player[A].Vine = 0;
                if(Player[A].StandingOnNPC != 0)
                {
                    Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                    Player[A].StandingOnNPC = 0;
                }

                Player[A].SwimCount = 15;
                // If .Location.SpeedY = 0 Then .Location.Y += -1
                if(Player[A].Controls.Down)
                {
                    if(Player[A].Location.SpeedY >= Physics.PlayerJumpVelocity * 0.2)
                    {
                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.2;
                        if(Player[A].Location.SpeedY < Physics.PlayerJumpVelocity * 0.2)
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.2;
                    }
                }
                else
                {
                    if(Player[A].Controls.Up)
                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.5;
                    else
                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.4;

                    if(Player[A].Mount == 1)
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                }

                if(Player[A].Location.SpeedY > 0)
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.2;

                PlaySoundSpatial(SFX_Swim, Player[A].Location);
            }
        }
    }

    Player[A].CanJump = !Player[A].Controls.Jump;
    Player[A].CanAltJump = !Player[A].Controls.AltJump;

    if(Player[A].Controls.Up)
    {
        if(Player[A].Location.SpeedY < -4)
            Player[A].Location.SpeedY = -4;
    }
    else
    {
        if(Player[A].Location.SpeedY < -3)
            Player[A].Location.SpeedY = -3;
    }
}
