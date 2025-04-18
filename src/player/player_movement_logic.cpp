/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "phys_env.h"

#include "player/player_update_priv.h"
#include "npc/npc_cockpit_bits.h"

void s_playerSlopeMomentum(int A);

void PlayerMovementX(int A, tempf_t& cursed_value_C)
{
    bool is_grounded = (Player[A].Location.SpeedY == 0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0);

    // Modify player's speed if he is running up/down hill
    tempf_t speedVar = 1; // Speed var is a percentage of the player's speed
    if(Player[A].Slope > 0)
    {
        if(
                (Player[A].Location.SpeedX > 0 && BlockSlope[Block[Player[A].Slope].Type] == -1) ||
                (Player[A].Location.SpeedX < 0 && BlockSlope[Block[Player[A].Slope].Type] == 1)
                )
            speedVar = (tempf_t)(1 - Block[Player[A].Slope].Location.Height / (int_ok)Block[Player[A].Slope].Location.Width / 2);
        else if(!Player[A].Slide)
            speedVar = (tempf_t)(1 + (Block[Player[A].Slope].Location.Height / (int_ok)Block[Player[A].Slope].Location.Width / 4));
    }

    if(Player[A].Stoned) // if statue form reset to normal
        speedVar = 1;

    if(Player[A].Character == 3)
        speedVar = (tempf_t)((num_t)speedVar * 0.93_r);

    if(Player[A].Character == 4)
        speedVar = (tempf_t)((num_t)speedVar * 1.07_r);

    // modify speedvar to slow the player down under water
    if(Player[A].Wet > 0)
    {
        if(is_grounded)
            speedVar /= 4; // if walking go really slow
        else
            speedVar /= 2; // if swimming go slower faster the walking
    }

    // special logic for shell: just keep going!
    if(((Player[A].State == PLR_STATE_SHELL && !Player[A].Wet) || Player[A].State == PLR_STATE_POLAR) && Player[A].Controls.Run && !Player[A].HoldingNPC && !Player[A].Mount)
    {
        // 7.1 is the NPC shellspeed
        num_t shell_speed = (num_t)speedVar * 7.1_r;

        if(shell_speed >= 7.5_n)
            shell_speed = 7.5_n;

        bool can_begin = is_grounded;
        if(Player[A].Slope && BlockSlope[Block[Player[A].Slope].Type] != Player[A].Direction)
            can_begin = false;

        num_t stop_speed = 0.5_n;
        if(Player[A].Slope)
        {
            // allow turning around on slopes
            stop_speed = 0.0_n;
        }

        num_t start_speed = shell_speed;

        if(Player[A].State == PLR_STATE_POLAR)
        {
            shell_speed -= 0.75_n;
            if(Player[A].Slippy)
                shell_speed += 3;

            // allow starting quickly
            start_speed -= 2;

            // or extremely quickly on a slope or ice
            if(Player[A].Slope || Player[A].Slippy)
                start_speed = 0.5_n;

            // require holding down to start
            if(!Player[A].Controls.Down)
                start_speed = 128;
        }

        // start rolling
        if(can_begin && num_t::abs(Player[A].Location.SpeedX) >= start_speed)
            Player[A].Rolling = true;

        // keep rolling
        if(Player[A].Rolling && num_t::abs(Player[A].Location.SpeedX) > stop_speed)
        {
            Player[A].Direction = (Player[A].Location.SpeedX > 0) ? 1 : -1;

            if(!Player[A].Duck)
            {
                Player[A].Duck = true;
                SizeCheck(Player[A]);
            }

            if(Player[A].Slope)
                s_playerSlopeMomentum(A);
            else
                Player[A].Location.SpeedX = (Player[A].Location.SpeedX * 127 + Player[A].Direction * shell_speed) / 128;

            return;
        }

        // stop rolling
        // allow the player to go faster if they aren't a shell yet
        if(Player[A].State == PLR_STATE_SHELL)
            speedVar = speedVar * 5 / 4;
    }

    if(Player[A].Rolling)
        Player[A].Slide = false;

    Player[A].Rolling = false;

    // ducking for link
    if(Player[A].Duck && Player[A].WetFrame)
    {
        if(!is_grounded)
            UnDuck(Player[A]);
    }

    // the following code controls the players ability to duck
    if(!(Player[A].Character == 5 && (!is_grounded || Player[A].FireBallCD != 0))) // Link can't duck/unduck in air
    {
        if(Player[A].Controls.Down && !Player[A].SpinJump &&
           !Player[A].Stoned && Player[A].Vine == 0 && !Player[A].Slide &&
           (Player[A].Slope == 0 || Player[A].Mount > 0 || Player[A].WetFrame ||
            Player[A].Character >= 3 || Player[A].GrabTime > 0) &&
           ((!Player[A].WetFrame || Player[A].Character >= 3) ||
            is_grounded || Player[A].Mount == 1) &&
           !Player[A].Fairy && !Player[A].ShellSurf && !Player[A].Driving)
        {
            Player[A].Bumped = false;
            if(Player[A].Mount != 2) // cant duck in the clown car
            {
                if(Player[A].Mount == 3) // duck on a yoshi
                {
                    if(!Player[A].Duck)
                    {
                        Player[A].Location.set_height_floor(31);
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
                            Player[A].Location.set_height_floor(Physics.PlayerDuckHeight[Player[A].Character][Player[A].State]);
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

    qdec_t local_C = 1.0_r;
    cursed_value_C = 1;
    // If .Character = 5 Then C = 0.94
    if(Player[A].Character == 5)
    {
        cursed_value_C = (tempf_t)0.95_n;
        local_C = 0.95_r;
    }

    // deduplicated (was previously separate sections for holding Left and Right)
    if((Player[A].Controls.Left || Player[A].Controls.Right) &&
       !Player[A].JumpOffWall &&
       ((!Player[A].Duck && Player[A].GrabTime == 0) ||
        !is_grounded ||
        Player[A].Mount == 1)
    )
    {
        int dir = (Player[A].Controls.Left) ? -1 : 1;
        Player[A].Bumped = false;

        if(Player[A].Controls.Run || dir * Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * (num_t)speedVar || Player[A].Character == 5)
        {
            // turning around or not yet walking
            if(dir * Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * (num_t)speedVar * local_C)
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += dir * num_t::from_double(-0.1 * 0.175);
                else if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += dir * num_t::from_double(-0.05 * 0.175);
                else if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += dir * num_t::from_double(0.05 * 0.175);

                Player[A].Location.SpeedX += dir * (num_t)speedVar / 10;
            }
            // running
            else
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += dir * num_t::from_double(-0.05 * 0.175);
                else if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += dir * num_t::from_double(-0.025 * 0.175);
                else if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += dir * num_t::from_double(0.025 * 0.175);

                if(Player[A].Character == 5) // Link
                    Player[A].Location.SpeedX += dir * (num_t)speedVar / 40;
                else // Mario
                    Player[A].Location.SpeedX += dir * (num_t)speedVar / 20;
            }

            // turning around
            if(dir * Player[A].Location.SpeedX < 0)
            {
                if(Player[A].Character == 2) // LUIGI
                    Player[A].Location.SpeedX += dir * num_t::from_double(-0.18 * 0.29 + 0.18);
                else if(Player[A].Character == 3) // PEACH
                    Player[A].Location.SpeedX += dir * num_t::from_double(-0.09 * 0.29 + 0.18);
                else if(Player[A].Character == 4) // toad
                    Player[A].Location.SpeedX += dir * num_t::from_double(0.09 * 0.29 + 0.18);
                else
                    Player[A].Location.SpeedX += dir * 0.18_n;

                if(SuperSpeed)
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95_r;
            }
        }

        if(SuperSpeed && Player[A].Controls.Run)
            Player[A].Location.SpeedX += dir * 0.1_n;
    }
    else
    {
        if(is_grounded || Player[A].WetFrame) // Only lose speed when not in the air
        {
            if(Player[A].Location.SpeedX > 0)
                Player[A].Location.SpeedX -= (num_t)speedVar * 0.07_r;
            if(Player[A].Location.SpeedX < 0)
                Player[A].Location.SpeedX += (num_t)speedVar * 0.07_r;
            if(Player[A].Character == 2) // LUIGI
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 1.003_r;
            if(Player[A].Character == 3) // PEACH
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 1.0015_r;
            if(Player[A].Character == 4) // toad
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.9985_r;
            if(SuperSpeed)
                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95_r;
        }

        if(Player[A].Location.SpeedX > -0.18_n && Player[A].Location.SpeedX < 0.18_n)
        {
            Player[A].Bumped = false;
            Player[A].Location.SpeedX = 0;
        }
    }

    // hard speed cap
    if(Player[A].Location.SpeedX < -16)
        Player[A].Location.SpeedX = -16;
    else if(Player[A].Location.SpeedX > 16)
        Player[A].Location.SpeedX = 16;

    if(Player[A].WarpShooted &&
       Player[A].Location.SpeedX < Physics.PlayerRunSpeed * (num_t)speedVar &&
       Player[A].Location.SpeedX > -Physics.PlayerRunSpeed * (num_t)speedVar)
    {
        Player[A].WarpShooted = false;
    }

    // soft speed cap
    if(!Player[A].WarpShooted && (Player[A].Controls.Run || Player[A].Character == 5))
    {
        if(Player[A].Location.SpeedX >= Physics.PlayerRunSpeed * (num_t)speedVar)
        {
            if(!SuperSpeed)
                Player[A].Location.SpeedX = Physics.PlayerRunSpeed * (num_t)speedVar;
        }
        else if(Player[A].Location.SpeedX <= -Physics.PlayerRunSpeed * (num_t)speedVar)
        {
            if(!SuperSpeed)
                Player[A].Location.SpeedX = -Physics.PlayerRunSpeed * (num_t)speedVar;
        }
//                        else  // REDURANT GARBAGE
//                        {
//                        }
    }
    else
    {
        // smooth run->walk deceleration
        // (note: this is an SMBX 1.3 bug, the correct expression would be Physics.PlayerWalkSpeed * speedVar + 0.1)
        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + (num_t)speedVar / 10)
            Player[A].Location.SpeedX -= 0.1_n;
        else if(Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - (num_t)speedVar / 10)
            Player[A].Location.SpeedX += 0.1_n;
        else if(num_t::abs(Player[A].Location.SpeedX) > Physics.PlayerWalkSpeed * (num_t)speedVar)
        {
            if(Player[A].Location.SpeedX > 0)
                Player[A].Location.SpeedX = Physics.PlayerWalkSpeed * (num_t)speedVar;
            else
                Player[A].Location.SpeedX = -Physics.PlayerWalkSpeed * (num_t)speedVar;
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
        // note: RunCount was previously a float, so its values have been multiplied by 10 everywhere
        bool is_running = (num_t::abs(Player[A].Location.SpeedX) >= Physics.PlayerRunSpeed ||
            (Player[A].Character == 3 && num_t::abs(Player[A].Location.SpeedX) >= 5.579_n)); // Rounding error of SpeedX makes an evil here (FIXME: does this match VB6?)

        if( (Player[A].CanFly2 ||
             is_grounded) &&
            is_running)
        {
            Player[A].RunCount += 10;
        }
        else if(!is_running)
        {
            Player[A].RunCount -= 3;
        }

        if(Player[A].RunCount >= 350 && Player[A].Character == 1)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 350;
        }
        else if(Player[A].RunCount >= 400 && Player[A].Character == 2)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 400;
        }
        else if(Player[A].RunCount >= 800 && Player[A].Character == 3)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 800;
        }
        else if(Player[A].RunCount >= 600 && Player[A].Character == 4)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 600;
        }
        else if(Player[A].RunCount >= 100 && Player[A].Character == 5) // link flying
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 100;
        }
        else
        {
            Player[A].CanFly = false;
            if(Player[A].RunCount < 0)
                Player[A].RunCount = 0;
        }
    }

    if(is_grounded)
        Player[A].FlyCount = 1;

    if(Player[A].FlyCount > 1)
        Player[A].FlyCount -= 1;
    else if(Player[A].FlyCount == 1)
    {
        Player[A].CanFly2 = false;
        Player[A].FlyCount = 0;
    }
}

void s_playerSlopeMomentum(int A)
{
    // Angle = 1 / (Block[Player[A].Slope].Location.Width / Block[Player[A].Slope].Location.Height);
    num_t Angle = Block[Player[A].Slope].Location.Height / (int_ok)Block[Player[A].Slope].Location.Width;
    num_t slideSpeed = Angle * BlockSlope[Block[Player[A].Slope].Type] / 10;

    num_t add_uphill = (Player[A].Rolling) ? slideSpeed : slideSpeed * 2;

    if(slideSpeed > 0 && Player[A].Location.SpeedX < 0)
        Player[A].Location.SpeedX += add_uphill;
    else if(slideSpeed < 0 && Player[A].Location.SpeedX > 0)
        Player[A].Location.SpeedX += add_uphill;
    else
        Player[A].Location.SpeedX += slideSpeed;
}

void PlayerSlideMovementX(int A)
{
    if(Player[A].Slope > 0)
        s_playerSlopeMomentum(A);
    else if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0)
    {
        if(Player[A].Location.SpeedX > 0.2_n)
            Player[A].Location.SpeedX -= 0.1_n;
        else if(Player[A].Location.SpeedX < -0.2_n)
            Player[A].Location.SpeedX += 0.1_n;
        else
        {
            Player[A].Location.SpeedX = 0;
            Player[A].Slide = false;
        }
    }

    if(Player[A].Location.SpeedX > 11)
        Player[A].Location.SpeedX = 11;

    if(Player[A].Location.SpeedX < -11)
        Player[A].Location.SpeedX = -11;

    if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
        Player[A].Slide = false;
}

void PlayerCockpitMovementX(int A)
{
    if(Player[A].Duck)
        UnDuck(Player[A]);

    Player[A].Driving = false;
    if(Player[A].StandingOnNPC > 0)
    {
        NPC[Player[A].StandingOnNPC].Special4 = NPC_COCKPIT_DRIVING;

        if(Player[A].Controls.Left)
            NPC[Player[A].StandingOnNPC].Special4 |= NPC_COCKPIT_LEFT;
        else if(Player[A].Controls.Right)
            NPC[Player[A].StandingOnNPC].Special4 |= NPC_COCKPIT_RIGHT;

        if(Player[A].Controls.Up)
            NPC[Player[A].StandingOnNPC].Special4 |= NPC_COCKPIT_UP;
        else if(Player[A].Controls.Down)
            NPC[Player[A].StandingOnNPC].Special4 |= NPC_COCKPIT_DOWN;
    }

    Player[A].Location.SpeedX = 0;
}

void PlayerMovementY(int A)
{
    if(Player[A].Mount == 1) // this gives the player the bounce when in the kurbio's shoe
    {
        if(Player[A].Controls.Left || Player[A].Controls.Right)
        {
            if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
            {
                if(Player[A].Controls.Left && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                    Player[A].Location.SpeedY = -4.1_n + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else if(Player[A].Controls.Right && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
                    Player[A].Location.SpeedY = -4.1_n + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else
                    PlaySoundSpatial(SFX_Skid, Player[A].Location);
                Player[A].MountSpecial = 1;
            }
        }

        if(Player[A].Location.SpeedY < -4.1_n)
            Player[A].MountSpecial = 0;
        else if(Player[A].Location.SpeedY > 4.1_n)
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

    if((Player[A].Location.SpeedY == 0 || (Player[A].Jump > 0 && !Player[A].JumpOffWall) || Player[A].Vine > 0) && Player[A].FloatTime == 0 && !(Player[A].State == PLR_STATE_CYCLONE && Player[A].SpinJump)) // princess float
        Player[A].CanFloat = true;

    if(Player[A].Wet > 0 || Player[A].WetFrame)
        Player[A].CanFloat = false;

    bool has_wall_traction = CanWallJump && (Player[A].Pinched.Left2 == 2 || Player[A].Pinched.Right4 == 2) && !Player[A].SpinJump && (!Player[A].SlippyWall || Player[A].State == PLR_STATE_POLAR) && Player[A].HoldingNPC == 0 && Player[A].Mount == 0 && !Player[A].Duck;

    // handles the regular jump
    if(Player[A].Controls.Jump || (Player[A].Controls.AltJump &&
       ((Player[A].Character > 2 && Player[A].Character != 4) || Player[A].Quicksand > 0 || Player[A].Rolling || g_config.disable_spin_jump) &&
       Player[A].CanAltJump))
    {
        num_t tempSpeed;
        if(Player[A].Location.SpeedX > 0)
            tempSpeed = Player[A].Location.SpeedX / 5; // tempSpeed gives the player a height boost when jumping while running, based off his SpeedX
        else
            tempSpeed = -Player[A].Location.SpeedX / 5;

        if(Player[A].ShellSurf) // this code modifies the jump based on him riding a shell
        {
            if(NPC[Player[A].StandingOnNPC].Location.SpeedY == 0 || NPC[Player[A].StandingOnNPC].Slope > 0)
            {
                if(Player[A].CanJump)
                {
                    PlaySoundSpatial(SFX_Whip, Player[A].Location); // Jump sound
                    Player[A].Jump = Physics.PlayerJumpHeight * 3 / 5;
                    NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9_r;
                }
            }
            else if(Player[A].Jump > 0)
                NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9_r;
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
                    if(NPC[Player[A].StandingOnNPC].Type != NPCID_ITEM_BURIED)
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
                        Player[A].RunCount = 800; // multiplied by 10 vs VB6 code, since it's an int now
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
            else if(has_wall_traction && Player[A].CanJump)
            {
                NewEffect(EFFID_WHACK, newLoc(Player[A].Location.X + Player[A].Location.Width / 2 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
                PlaySoundSpatial(SFX_Jump, Player[A].Location); // Jump sound
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                Player[A].Location.SpeedX -= 5 * Player[A].Direction;
                Player[A].Location.X += Player[A].Location.SpeedX;
                Player[A].Jump = 8;
                Player[A].JumpOffWall = true;

                if(Player[A].Character == 2)
                    Player[A].Jump += 3;
            }
            else if(Player[A].Jump > 0) // controls the height of the jump
            {
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                if(Player[A].Jump > 20)
                {
                    if(Player[A].Jump > 40)
                        Player[A].Location.SpeedY += -(40 - 20) * 0.2_n;
                    else
                        Player[A].Location.SpeedY += -(Player[A].Jump - 20) * 0.2_n;
                }
            }
            else if(Player[A].CanFly2)
            {
                if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity / 2)
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

    bool is_supported = (Player[A].StandingOnNPC != 0 || Player[A].Slope != 0 || Player[A].Location.SpeedY == 0 || Player[A].Wet || Player[A].Vine || Player[A].WetFrame);
    // cyclone: allow double jump as save after falling off cliff
    if(Player[A].State == PLR_STATE_CYCLONE && (!Player[A].SpinJump || Player[A].HoldingNPC))
    {
        if(is_supported)
            Player[A].DoubleJump = true;
    }
    // this is the old condition which subtly differs from the supported condition tested above and below
    else if(Player[A].SpinJump || (Player[A].State != 4 && Player[A].State != 5) || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].Location.SpeedY == 0)
        Player[A].DoubleJump = false;

    // double jump code
    if(Player[A].DoubleJump && Player[A].Jump == 0 && !is_supported && !Player[A].Fairy && !Player[A].CanFly2 && Player[A].JumpRelease)
    {
        if(Player[A].State == PLR_STATE_CYCLONE && (Player[A].Controls.Jump || Player[A].Controls.AltJump))
        {
            if(!Player[A].Mount && !Player[A].HoldingNPC)
            {
                PlaySoundSpatial(SFX_Whip, Player[A].Location);
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                Player[A].Jump = 16;
                Player[A].DoubleJump = false;

                if(Player[A].Character != 5)
                {
                    Player[A].SpinJump = true;
                    UnDuck(Player[A]);
                }
            }
        }
        else if(Player[A].Controls.Jump)
        {
            PlaySoundSpatial(SFX_Jump, Player[A].Location);
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
            Player[A].Jump = 10;
            Player[A].DoubleJump = false;
            Location_t tempLocation = Player[A].Location;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - EffectHeight[EFFID_SPARKLE] * 0.5_n + Player[A].Location.SpeedY;
            tempLocation.Height = EffectHeight[EFFID_SPARKLE];
            tempLocation.Width = EffectWidth[EFFID_SPARKLE];
            tempLocation.X = Player[A].Location.X;

            for(int B = 1; B <= 10; B++)
            {
                NewEffect(EFFID_SPARKLE, tempLocation);
                Effect[numEffects].Location.SpeedX = (dRand() * 3) - 1.5_n;
                Effect[numEffects].Location.SpeedY = (dRand() / 2) + (1.5_n - num_t::abs(Effect[numEffects].Location.SpeedX)) / 2;
                Effect[numEffects].Location.SpeedX += -Player[A].Location.SpeedX / 5;
            }
        }
    }



#if 0
    // never set since SMBX 1.3, see dead code below
    if(Player[A].NoShellKick > 0) // countdown for the next time the player kicks a turtle shell
        Player[A].NoShellKick--;
#endif

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
                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2 + dRand().times(NPC[Player[A].StandingOnNPC].Location.Height - 8) + 4;
                tempLocation.X = Player[A].Location.X - 4 + dRand() * ((int)Player[A].Location.Width - 8) + 4 - 8 * Player[A].Direction;
                NewEffect(EFFID_SPARKLE, tempLocation, 1, 0, ShadowMode);
                Effect[numEffects].Frame = iRand(3);
                Effect[numEffects].Location.SpeedY = (Player[A].Location.Y + Player[A].Location.Height + NPC[Player[A].StandingOnNPC].Location.Height / 32 - tempLocation.Y + 12) / 20;
            }
        }

        if(NPC[Player[A].StandingOnNPC].Wet == 2)
        {
            if(NPC[Player[A].StandingOnNPC].Type == NPCID_FLIPPED_RAINBOW_SHELL)
                NPC[Player[A].StandingOnNPC].Special4 = 1;
            NPC[Player[A].StandingOnNPC].Location.SpeedY += -Physics.NPCGravity * 1.5_rb;
        }
    }

    // START ALT JUMP - this code does the player's spin jump
    if(Player[A].Controls.AltJump && (Player[A].Character == 1 || Player[A].Character == 2 || Player[A].Character == 4 ||
                                      (g_config.fix_char3_escape_shell_surf && Player[A].Character == 3 && Player[A].ShellSurf))
                                  && (!g_config.disable_spin_jump || Player[A].ShellSurf))
    {
        num_t tempSpeed;
        if(Player[A].Location.SpeedX > 0)
            tempSpeed = Player[A].Location.SpeedX / 5;
        else
            tempSpeed = -Player[A].Location.SpeedX / 5;

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
                    if(NPC[Player[A].StandingOnNPC].Type != NPCID_ITEM_BURIED)
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

                // just checked that Player[A].Duck wasn't true!
                // if(Player[A].Duck)
                //     UnDuck(Player[A]);

                if(Player[A].ShellSurf)
                {
                    Player[A].ShellSurf = false;
                    Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX + (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed * 0.8_r;
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
        else if(has_wall_traction && Player[A].CanAltJump)
        {
            NewEffect(EFFID_WHACK, newLoc(Player[A].Location.X + Player[A].Location.Width / 2 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
            PlaySoundSpatial(SFX_Whip, Player[A].Location); // Jump sound
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
            Player[A].Location.SpeedX -= 7 * Player[A].Direction;
            Player[A].Location.X += Player[A].Location.SpeedX;
            Player[A].Jump = 5;
            Player[A].JumpOffWall = true;
            Player[A].SpinJump = true;
        }
        else if(Player[A].Jump > 0)
        {
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
            if(Player[A].Jump > 20)
                Player[A].Location.SpeedY += -(Player[A].Jump - 20) * 0.2_n;
        }
        else if(Player[A].CanFly2)
        {
            if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity / 2)
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
    else
        Player[A].JumpOffWall = false;


    if(Player[A].Quicksand > 1)
    {
        Player[A].Slide = false;
        if(Player[A].Location.SpeedY < -0.7_n)
        {
            Player[A].Location.SpeedY = -0.7_n;
            Player[A].Jump -= 1;
        }
        else if(Player[A].Location.SpeedY < 0)
        {
            Player[A].Location.SpeedY += 0.1_n;
            Player[A].Jump = 0;
        }

        if(Player[A].Location.SpeedY >= 0.1_n)
            Player[A].Location.SpeedY = 0.1_n;
        Player[A].Location.Y += Player[A].Location.SpeedY;
    }


    // gravity
    if(Player[A].Vine == 0)
    {
        if(Player[A].NoGravity == 0)
        {
            if(has_wall_traction && Player[A].Location.SpeedY > 0)
                Player[A].Location.SpeedY += Physics.PlayerGravity / 2;
            else if(Player[A].Character == 2)
                Player[A].Location.SpeedY += Physics.PlayerGravity * 0.9_r;
            else
                Player[A].Location.SpeedY += Physics.PlayerGravity;

            bool has_fly_block = (Player[A].HoldingNPC > 0) && (NPC[Player[A].HoldingNPC].Type == NPCID_FLY_BLOCK || NPC[Player[A].HoldingNPC].Type == NPCID_FLY_CANNON);
            bool no_cyclone_glide = (Player[A].Location.SpeedY >= 0) && (Player[A].Mount || Player[A].HoldingNPC); // glide down in cases where player can't cyclone

            if(has_fly_block || (Player[A].State == PLR_STATE_CYCLONE && (!Player[A].DoubleJump || no_cyclone_glide)))
            {
                if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                {
                    if(Player[A].Character == 2)
                        Player[A].Location.SpeedY += -Physics.PlayerGravity * 0.72_r;
                    else
                        Player[A].Location.SpeedY += -Physics.PlayerGravity * 0.8_r;

                    if(Player[A].Location.SpeedY > Physics.PlayerGravity * 3)
                        Player[A].Location.SpeedY = Physics.PlayerGravity * 3;
                }
                else if(has_fly_block)
                    NPC[Player[A].HoldingNPC].Special = 0;
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
                // float time is longer during glide
                if(Player[A].State == 4 || Player[A].State == 5)
                {
                    Player[A].FloatTime = 100;
                    Player[A].FlySparks = true;
                }
                else
                    Player[A].FloatTime = 65;

                Player[A].FloatDir = 1;

                if(Player[A].Location.SpeedY < -0.5_n)
                    Player[A].FloatSpeed = 0.5_nf;
                else if(Player[A].Location.SpeedY > 0.5_n)
                    Player[A].FloatSpeed = 0.5_nf;
                else
                    Player[A].FloatSpeed = (numf_t)(Player[A].Location.SpeedY);

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
            tempf_t floatSpeed = (tempf_t)Player[A].FloatSpeed;
            floatSpeed += Player[A].FloatDir * (tempf_t)0.1_n;

            if(floatSpeed > (tempf_t)0.8_n)
                Player[A].FloatDir = -1;

            if(floatSpeed < -(tempf_t)0.8_n)
                Player[A].FloatDir = 1;

            Player[A].Location.SpeedY = (num_t)floatSpeed;
            Player[A].FloatSpeed = (numf_t)floatSpeed;

            Player[A].FloatTime -= 1;
            if(Player[A].FloatTime == 0 && Player[A].Location.SpeedY == 0)
                Player[A].Location.SpeedY = 0.1_n;
        }
        else
            Player[A].FloatTime = 0;
    }


    // glide ' Racoon Mario
    if((Player[A].State == PLR_STATE_LEAF || Player[A].State == PLR_STATE_STATUE) || Player[A].YoshiBlue || (Player[A].Mount == 1 && Player[A].MountType == 3))
    {
        if((Player[A].Controls.Jump || Player[A].Controls.AltJump) &&
          ((Player[A].Location.SpeedY > Physics.PlayerGravity * 5 && Player[A].Character != 3 && Player[A].Character != 4) ||
            (Player[A].Location.SpeedY > Physics.PlayerGravity * 10 && Player[A].Character == 3) ||
            (Player[A].Location.SpeedY > Physics.PlayerGravity * 7.5_rb && Player[A].Character == 4)) &&
            !Player[A].GroundPound && Player[A].Slope == 0 && Player[A].Character != 5)
        {
            if(!Player[A].ShellSurf)
            {
                if(Player[A].Character == 3)
                    Player[A].Location.SpeedY = Physics.PlayerGravity * 10;
                else if(Player[A].Character == 4)
                    Player[A].Location.SpeedY = Physics.PlayerGravity * 7.5_rb;
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
                    p_PlayerMakeFlySparkle(Player[A].Location);
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

    Player[A].Location.SpeedY += Physics.PlayerGravity / 10;

    if(Player[A].Location.SpeedY >= 3) // Terminal Velocity in water
        Player[A].Location.SpeedY = 3;

    if(Player[A].Mount == 1)
    {
        if(Player[A].Controls.Left || Player[A].Controls.Right)
        {
            if(Player[A].Location.SpeedY == Physics.PlayerGravity / 10 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
            {
                if(Player[A].Controls.Left && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                    Player[A].Location.SpeedY = -1.1_n + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else if(Player[A].Controls.Right && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
                    Player[A].Location.SpeedY = -1.1_n + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                else
                    PlaySoundSpatial(SFX_Skid, Player[A].Location);

                Player[A].MountSpecial = 1;
            }
        }

        if(Player[A].Location.SpeedY < -1.1_n)
            Player[A].MountSpecial = 0;
        else if(Player[A].Location.SpeedY > 1.1_n)
            Player[A].MountSpecial = 0;
        else if(Player[A].FloatTime >= 0)
            Player[A].MountSpecial = 0;


        if(Player[A].Controls.Jump && Player[A].MountSpecial == 1 && Player[A].CanJump)
        {
            Player[A].Location.SpeedY = Physics.PlayerGravity / 10;
            Player[A].MountSpecial = 0;
            Player[A].StandUp = true;
        }
    }

    if(Player[A].SwimCount > 0)
        Player[A].SwimCount -= 1;

    if(Player[A].SwimCount == 0)
    {
        if(Player[A].Mount != 1 || Player[A].Location.SpeedY == Physics.PlayerGravity / 10 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0)
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
                    if(Player[A].Location.SpeedY >= Physics.PlayerJumpVelocity / 5)
                    {
                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity / 5;
                        if(Player[A].Location.SpeedY < Physics.PlayerJumpVelocity / 5)
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity / 5;
                    }
                }
                else
                {
                    if(Player[A].Controls.Up)
                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity / 2;
                    else
                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.4_r;

                    if(Player[A].Mount == 1)
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                }

                if(Player[A].Location.SpeedY > 0)
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity / 5;

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

void PlayerAquaticSwimMovement(int A)
{
    Player[A].Rolling = false;

    if(!Player[A].Duck)
    {
        Player[A].Duck = true;
        SizeCheck(Player[A]);
    }

    if(!Player[A].Wet)
    {
        if(Player[A].Pinched.Bottom1)
            Player[A].WetFrame = false;

        tempf_t C = 0;

        PlayerMovementX(A, C);
        return;
    }

    int current_swim_dir = -1;
    if(Player[A].SwimCount > 0)
    {
        current_swim_dir = Player[A].SwimCount / 16;

        Player[A].SwimCount -= 1;

        if((Player[A].SwimCount & 15) == 0)
            Player[A].SwimCount = 0;
    }

    num_t base_speed = (Player[A].State == PLR_STATE_POLAR) ? 1_n : 2.5_n;

    num_t target_speed = base_speed;
    int rate = 16; // out of 256

    if(Player[A].Controls.Run)
        target_speed += 1;

    if(Player[A].Character == 2)
    {
        target_speed *= 0.9_r;
        rate = rate / 2;
    }
    else if(Player[A].Character == 3)
        target_speed *= 0.85_r;
    else if(Player[A].Character == 4)
        target_speed *= 1.1_r;
    else if(Player[A].Character == 5)
    {
        target_speed *= 0.95_r;
        rate = rate * 3 / 4;
    }

    num_t target_speed_x = 0;
    num_t target_speed_y = 0;
    int rate_x = rate / 2;
    int rate_y = rate / 2;

    int new_swim_dir = (Player[A].Direction > 0) ? MAZE_DIR_RIGHT : MAZE_DIR_LEFT;

    // keep old direction if present
    if(Player[A].Frame == 19 || Player[A].Frame == 20 || Player[A].Frame == 21)
        new_swim_dir = MAZE_DIR_DOWN;
    else if(Player[A].Frame == 40 || Player[A].Frame == 41 || Player[A].Frame == 42)
        new_swim_dir = MAZE_DIR_UP;

    if((Player[A].Controls.Left && current_swim_dir != MAZE_DIR_RIGHT) || current_swim_dir == MAZE_DIR_LEFT)
    {
        new_swim_dir = MAZE_DIR_LEFT;
        target_speed_x = -target_speed;

        if(Player[A].Location.SpeedX < 0)
            rate_x = rate;
        else
            rate_x = rate * 2;

        if(current_swim_dir == MAZE_DIR_LEFT)
        {
            target_speed_x -= base_speed;
            rate_x *= 2;
        }

        Player[A].Direction = -1;
    }
    else if(Player[A].Controls.Right || current_swim_dir == MAZE_DIR_RIGHT)
    {
        new_swim_dir = MAZE_DIR_RIGHT;
        target_speed_x = target_speed;

        if(Player[A].Location.SpeedX > 0)
            rate_x = rate;
        else
            rate_x = rate * 2;

        if(current_swim_dir == MAZE_DIR_RIGHT)
        {
            target_speed_x += base_speed;
            rate_x *= 2;
        }

        Player[A].Direction = 1;
    }

    if((Player[A].Controls.Up && current_swim_dir != MAZE_DIR_DOWN) || current_swim_dir == MAZE_DIR_UP)
    {
        new_swim_dir = MAZE_DIR_UP;
        target_speed_y = -target_speed;

        if(Player[A].Location.SpeedY < 0)
            rate_y = rate;
        else
            rate_y = rate * 2;

        if(current_swim_dir == MAZE_DIR_UP)
        {
            target_speed_y -= base_speed;
            rate_y *= 2;
        }
    }
    else if(Player[A].Controls.Down || current_swim_dir == MAZE_DIR_DOWN)
    {
        new_swim_dir = MAZE_DIR_DOWN;
        target_speed_y = target_speed;

        if(Player[A].Location.SpeedY > 0)
            rate_y = rate;
        else
            rate_y = rate * 2;

        if(current_swim_dir == MAZE_DIR_DOWN)
        {
            target_speed_y += base_speed;
            rate_y *= 2;
        }
    }

    // go a bit slower vertically
    target_speed_y = target_speed_y * 3 / 4;

    Player[A].Location.SpeedX = (target_speed_x * rate_x + Player[A].Location.SpeedX * (256 - rate_x)) / 256;
    Player[A].Location.SpeedY = (target_speed_y * rate_y + Player[A].Location.SpeedY * (256 - rate_y)) / 256;

    // stop (X)
    if(num_t::abs(Player[A].Location.SpeedX) < 0.03125_n)
        Player[A].Location.SpeedX = 0;

    // stop (Y)
    if(num_t::abs(Player[A].Location.SpeedY) < 0.03125_n)
        Player[A].Location.SpeedY = 0;

    // apply gravity if about to stop being wet -- this keeps us in the water
    if(Player[A].Wet == 1)
        Player[A].Location.SpeedY += Physics.PlayerGravity;

    if(Player[A].SwimCount == 0)
    {
        if((Player[A].Controls.Jump && Player[A].CanJump) ||
           (Player[A].Controls.AltJump && Player[A].CanAltJump))
        {
            Player[A].SwimCount = 16 * new_swim_dir + 15;
            PlaySoundSpatial(SFX_Swim, Player[A].Location);
        }
    }

    Player[A].CanJump = !Player[A].Controls.Jump;
    Player[A].CanAltJump = !Player[A].Controls.AltJump;
}

void PlayerMazeZoneMovement(int A)
{
    if(Player[A].Mount == 3)
    {
        Player[A].Duck = true;
        SizeCheck(Player[A]);
    }
    else if(!Player[A].Rolling && !Player[A].AquaticSwim)
    {
        UnDuck(Player[A]);
        Player[A].WetFrame = true;
    }

    Player[A].Bumped = 0;
    Player[A].Bumped2 = 0;

    if(Player[A].MazeZoneStatus & MAZE_PLAYER_FLIP)
        Player[A].MazeZoneStatus = (Player[A].MazeZoneStatus & 3) ^ MAZE_DIR_FLIP_BIT;

    Player[A].SpinJump = false;

    PhysEnv_Maze(Player[A].Location, Player[A].CurMazeZone, Player[A].MazeZoneStatus, 0, A, Player[A].Rolling ? 6 : (Player[A].Quicksand ? 1 : (Player[A].Wet && !Player[A].AquaticSwim ? 2 : 4)), {Player[A].Controls.Left, Player[A].Controls.Up, Player[A].Controls.Right, Player[A].Controls.Down});

    if(!Player[A].CurMazeZone)
    {
        // prevent unexpected block clipping
        if(Player[A].MazeZoneStatus % 4 == MAZE_DIR_DOWN)
        {
            PlayerPush(A, 1);
        }
        else if(Player[A].MazeZoneStatus % 4 == MAZE_DIR_UP)
        {
            Player[A].StandUp = true;
            Player[A].StandUp2 = true;
            Player[A].ForceHitSpot3 = true;
        }
        // don't allow jumping
        else
        {
            Player[A].Location.SpeedY = 0.01_n;
        }

        PlaySoundSpatial(SFX_HeroDash, Player[A].Location);
    }
}
