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

#include <algorithm>

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "../globals.h"
#include "../config.h"
#include "../player.h"
#include "player/player_effect.h"
#include "player/player_update_priv.h"
#include "../collision.h"
#include "../sound.h"
#include "../blocks.h"
#include "../npc.h"
#include "../effect.h"
#include "../layers.h"
#include "../editor.h"
#include "../game_main.h"
#include "../main/trees.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../controls.h"
#include "../script/msg_preprocessor.h"

#include "npc_id.h"
#include "npc_traits.h"
#include "eff_id.h"

#include "npc/npc_queues.h"


void UpdatePlayer()
{
    bool tempSpring = false;
    bool tempShell = false;

    StealBonus(); // allows a dead player to come back to life by using a 1-up
    ClownCar(); // updates players in the clown car


    // A is the current player, numPlayers is the last player. this loop updates all the players
    for(int tmpNumPlayers = numPlayers, A = 1; A <= tmpNumPlayers; A++)
    {
        // reset variables
        Player[A].ShowWarp = 0;
        Player[A].mountBump = 0;

        // this was shared over players in SMBX 1.3 -- if the line marked "MOST CURSED LINE" in player_block_logic.cpp becomes a source of incompatibility, we will need to restore that logic
        float cursed_value_C = 0;

        if(Player[A].GrabTime > 0) // if grabbing something, take control away from the player
        {
            Player[A].Slide = false;
            Player[A].Controls.Run = true;
            Player[A].Controls.Down = true;
            Player[A].Controls.AltRun = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.AltJump = false;
        }

        if(Player[A].Dismount > 0) // count down to being able to hop in a shoe or yoshi
            Player[A].Dismount -= 1;

        if(Player[A].Mount != 0 || Player[A].Stoned || Player[A].Fairy) // if .holdingnpc is -1 then the player can't grab anything. this stops the player from grabbing things while on a yoshi/shoe
            Player[A].HoldingNPC = -1;
        else if(Player[A].HoldingNPC == -1)
            Player[A].HoldingNPC = 0;

        if(Player[A].Controls.Drop && Player[A].DropRelease) // this is for the single player coop cheat code
        {
            if(SingleCoop > 0 && Player[A].Controls.Down)
                SwapCoop();
            else
                DropBonus(A);
        }

        // for dropping something from the container. this makes the player have to let go of the drop button before dropping something else
        Player[A].DropRelease = !Player[A].Controls.Drop;

        // Handle the death effecs
        if(Player[A].TimeToLive > 0)
            UpdatePlayerTimeToLive(A);
        else if(Player[A].Dead)
            UpdatePlayerDead(A);
        else if(!Player[A].Dead)
        {
            if(Player[A].SlideCounter > 0) // for making the slide Effect
                Player[A].SlideCounter -= 1;

            if(Player[A].Effect == PLREFF_NORMAL)
            {
                // for the pound pet mount logic
                PlayerPoundLogic(A);

                SizeCheck(Player[A]); // check that the player is the correct size for it's character/state/mount and set it if not

                if(Player[A].Stoned) // stop the player from climbing/spinning/jumping when in tanooki statue form
                {
                    Player[A].Jump = 0;
                    Player[A].Vine = 0;
                    Player[A].SpinJump = false;
                    Player[A].Controls.Left = false;
                    Player[A].Controls.Right = false;
                    Player[A].Controls.AltJump = false;
                    Player[A].Controls.Jump = false;
                    Player[A].CanAltJump = false;
                    Player[A].CanJump = false;
                }

                // let the player slide if not on a mount and holding something
                if(Player[A].GrabTime > 0)
                    Player[A].Slide = false;

                if(Player[A].Slope > 0 && Player[A].Controls.Down &&
                   Player[A].Mount == 0 && Player[A].HoldingNPC == 0 &&
                   !(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5) &&
                   Player[A].GrabTime == 0)
                {
                    if(Player[A].Duck)
                        UnDuck(Player[A]);
                    Player[A].Slide = true;
                }
                else if(Player[A].Location.SpeedX == 0.0)
                    Player[A].Slide = false;

                if(Player[A].Mount > 0 || Player[A].HoldingNPC > 0)
                    Player[A].Slide = false;

                // unduck a player that shouldn't be able to duck
                if(Player[A].Duck && (Player[A].Character == 1 || Player[A].Character == 2) && Player[A].State == 1 && (Player[A].Mount == 0 || Player[A].Mount == 2))
                    UnDuck(Player[A]);

                if(GameMenu && !Player[A].SpinJump) // force the player to look right when on the game menu
                    Player[A].Direction = 1;

                WaterCheck(A); // This sub handles all the water related stuff

                PowerUps(A); // misc power-up code

                if(Player[A].StandingOnNPC > 0)
                {
                    if(NPC[Player[A].StandingOnNPC].Type == NPCID_ICE_CUBE && NPC[Player[A].StandingOnNPC].Location.SpeedX == 0.0)
                        Player[A].Slippy = true;
                }

                double SlippySpeedX = Player[A].Location.SpeedX;


                // Player's X movement. ---------------------------

                if(Player[A].Slide) // Code used to move the player while sliding down a slope
                {
                    if(Player[A].Slope > 0)
                    {
                        double Angle = 1 / (Block[Player[A].Slope].Location.Width / static_cast<double>(Block[Player[A].Slope].Location.Height));
                        double slideSpeed = 0.1 * Angle * BlockSlope[Block[Player[A].Slope].Type];

                        if(slideSpeed > 0 && Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX += slideSpeed * 2;
                        else if(slideSpeed < 0 && Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX += slideSpeed * 2;
                        else
                            Player[A].Location.SpeedX += slideSpeed;
                    }
                    else if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0)
                    {
                        if(Player[A].Location.SpeedX > 0.2)
                            Player[A].Location.SpeedX -= 0.1;
                        else if(Player[A].Location.SpeedX < -0.2)
                            Player[A].Location.SpeedX += 0.1;
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
                // if not sliding and in the clown car
                else if(Player[A].Mount == 2)
                {
                    if(!Player[A].Controls.Jump)
                        Player[A].CanJump = true;

                    if(!Player[A].Controls.AltJump && g_config.fix_vehicle_altjump_lock)
                        Player[A].CanAltJump = true;

                    if(Player[A].Controls.AltJump && Player[A].CanAltJump) // Jump out of the Clown Car
                    {
                        if(g_config.fix_vehicle_altjump_lock)
                            Player[A].CanAltJump = false;

                        Player[A].CanJump = false;

                        bool dismount_safe = true;

                        Location_t tempLocation = Player[A].Location;
                        tempLocation.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        tempLocation.Y += -Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        tempLocation.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                        tempLocation.X += 64 - tempLocation.Width / 2.0;

                        // fBlock = FirstBlock[(tempLocation.X / 32) - 1];
                        // lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                        // blockTileGet(tempLocation, fBlock, lBlock);

                        for(int B : treeFLBlockQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(!Block[B].Invis && !BlockIsSizable[Block[B].Type] && !BlockOnlyHitspot1[Block[B].Type] &&
                               !BlockNoClipping[Block[B].Type] && !Block[B].Hidden)
                            {
                                if(CheckCollision(tempLocation, Block[B].Location))
                                {
                                    dismount_safe = false;
                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                    break;
                                }
                            }
                        }

                        if(dismount_safe) for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(NPC[B]->IsABlock && !NPC[B]->StandsOnPlayer && NPC[B].Active && NPC[B].Type != NPCID_VEHICLE)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location))
                                {
                                    dismount_safe = false;
                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                    break;
                                }
                            }
                        }

                        if(dismount_safe)
                            PlayerDismount(A);
                    }
                }
                else if(Player[A].Driving) // driving
                {
                    if(Player[A].Duck)
                        UnDuck(Player[A]);

                    Player[A].Driving = false;
                    if(Player[A].StandingOnNPC > 0)
                    {
                        NPC[Player[A].StandingOnNPC].Special4 = 1;

                        if(Player[A].Controls.Left)
                            NPC[Player[A].StandingOnNPC].Special5 = -1;
                        else if(Player[A].Controls.Right)
                            NPC[Player[A].StandingOnNPC].Special5 = 1;
                        else
                            NPC[Player[A].StandingOnNPC].Special5 = 0;

                        if(Player[A].Controls.Up)
                            NPC[Player[A].StandingOnNPC].Special6 = -1;
                        else if(Player[A].Controls.Down)
                            NPC[Player[A].StandingOnNPC].Special6 = 1;
                        else
                            NPC[Player[A].StandingOnNPC].Special6 = 0;
                    }

                    Player[A].Location.SpeedX = 0;
                }
                // if a fairy
                else if(Player[A].Fairy)
                    PlayerFairyMovementX(A);
                // if the player is climbing a vine
                else if(Player[A].Vine > 0)
                    PlayerVineMovement(A);
                // if none of the above apply then the player controls like normal. remeber this is for the players X movement
                else
                    PlayerMovementX(A, cursed_value_C);


                // stop link when stabbing
                if(Player[A].Character == 5)
                {
                    if(Player[A].FireBallCD > 0 && (Player[A].Location.SpeedY == 0.0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0))
                    {
                        if(Player[A].Slippy)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.75;
                        else
                            Player[A].Location.SpeedX = 0;
                    }
                }

                // fairy stuff
                PlayerFairyTimerUpdate(A);


                // the single pinched variable has been always false since SMBX64
                if(Player[A].StandingOnNPC != 0 && /*!NPC[Player[A].StandingOnNPC].Pinched && */ !FreezeNPCs)
                {
                    if(Player[A].StandingOnNPC < 0)
                        NPC[Player[A].StandingOnNPC].Location = Block[(int)NPC[Player[A].StandingOnNPC].Special].Location;

                    Player[A].Location.SpeedX += NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed;
                }

                if(GameOutro) // force the player to walk a specific speed during the credits
                {
                    if(Player[A].Location.SpeedX < -2)
                        Player[A].Location.SpeedX = -2;
                    if(Player[A].Location.SpeedX > 2)
                        Player[A].Location.SpeedX = 2;
                }



                // slippy code
                if(Player[A].Slippy && (!Player[A].Slide || Player[A].Slope == 0))
                {
                    if(Player[A].Slope > 0)
                    {
                        Player[A].Location.SpeedX = (Player[A].Location.SpeedX + SlippySpeedX * 4) / 5;
                        if(Player[A].Location.SpeedX > -0.01 && Player[A].Location.SpeedX < 0.01)
                            Player[A].Location.SpeedX = 0;
                    }
                    else
                    {
                        Player[A].Location.SpeedX = (Player[A].Location.SpeedX + SlippySpeedX * 3) / 4;
                        if(Player[A].Location.SpeedX > -0.01 && Player[A].Location.SpeedX < 0.01)
                            Player[A].Location.SpeedX = 0;
                    }
                }

                bool wasSlippy = Player[A].Slippy;
                Player[A].Slippy = false;

                if(Player[A].Quicksand > 1)
                {
                    Player[A].Slide = false;
                    if(Player[A].Location.SpeedY >= 0)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.5;
                }


                Player[A].Location.X += Player[A].Location.SpeedX; // This is where the actual movement happens

                // Players Y movement.
                if(Block[Player[A].Slope].Location.SpeedY != 0.0 && Player[A].Slope != 0)
                    Player[A].Location.Y += Block[Player[A].Slope].Location.SpeedY;

                if(Player[A].Fairy) // the player is a fairy
                    PlayerFairyMovementY(A);
                else if(Player[A].Wet > 0 && Player[A].Quicksand == 0) // the player is swimming
                    PlayerSwimMovementY(A);
                else if(Player[A].Mount == 2)
                {
                    // vehicle has own Y movement code elsewhere
                }
                else // the player is not swimming
                    PlayerMovementY(A);

                Player[A].Location.Y += Player[A].Location.SpeedY;

                // princess peach and toad stuff
                if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
                {
                    Player[A].HeldBonus = NPCID(0);
                    // power up limiter
                    // If (.Character = 3 Or .Character = 4) And .State > 3 And .State <> 7 Then .State = 2

                    if(Player[A].Mount == 3)
                    {
                        PlayerHurt(A);
                        Player[A].Mount = 0;
                        UpdateYoshiMusic();
                    }

                    if(Player[A].Slide)
                        Player[A].Slide = false;

                    // If .Stoned = True Then .Stoned = False
                    if(Player[A].Hearts == 1 && Player[A].State > 1)
                        Player[A].Hearts = 2;

                    if(Player[A].Hearts > 1 && Player[A].State == 1)
                        Player[A].Hearts = 1;

                    if(Player[A].Hearts == 0)
                    {
                        if(Player[A].State == 1)
                            Player[A].Hearts = 1;

                        if(Player[A].State >= 2)
                            Player[A].Hearts = 2;
                    }
                }

                // link stuff
                if(Player[A].Character == 5)
                {
                    if(Player[A].State == 4 || Player[A].State == 5)
                    {
                        bool hasNoMonts = (g_config.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                           !g_config.fix_char5_vehicle_climb;

                        bool turnFairy = Player[A].FlyCount > 0 ||
                                        ((Player[A].Controls.AltJump || (Player[A].Controls.Jump && Player[A].FloatRelease)) &&
                                          Player[A].Location.SpeedY != Physics.PlayerGravity && Player[A].Slope == 0 &&
                                          Player[A].StandingOnNPC == 0);

                        if(turnFairy && hasNoMonts)
                        {
                            if(Player[A].FlyCount > 0)
                                Player[A].FairyCD = 0;

                            if(!Player[A].Fairy && Player[A].FairyCD == 0 && Player[A].Jump == 0 && Player[A].Wet == 0)
                            {
                                Player[A].Jump = 0;
                                if(Player[A].FlyCount == 0)
                                    Player[A].FlyCount = 50;
                                Player[A].FairyTime = Player[A].FlyCount;
                                Player[A].FairyCD = 1;
                                Player[A].FlyCount = 0;
                                Player[A].Fairy = true;
                                SizeCheck(Player[A]);
                                PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                                Player[A].Immune = 10;
                                Player[A].Effect = PLREFF_WAITING;
                                Player[A].Effect2 = 4;
                                NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                            }
                        }

                        if(Player[A].Controls.Run && Player[A].RunRelease && (Player[A].FairyTime > 0 || Player[A].Effect == PLREFF_WAITING))
                        {
                            Player[A].FairyTime = 0;
                            Player[A].Controls.Run = false;
                        }

                        if(Player[A].Fairy)
                        {
                            if(Player[A].Slope > 0 || Player[A].StandingOnNPC > 0)
                            {
                                Player[A].FairyTime = 0;
                                Player[A].FairyCD = 0;
                            }
                        }
                        // Coins += -1
                        // If Coins < 0 Then
                        // Lives += -1
                        // Coins += 99
                        // If Lives < 0 Then
                        // Lives = 0
                        // Coins = 0
                        // .FairyTime = 0
                        // End If
                        // End If
                        // End If
                    }

                    if(Player[A].HasKey)
                    {
                        for(int B : treeBackgroundQuery(Player[A].Location, SORTMODE_NONE))
                        {
                            if(B > numBackground)
                                continue;

                            if(Background[B].Type == 35)
                            {
                                Location_t tempLocation = Background[B].Location;
                                tempLocation.Width = 16;
                                tempLocation.X += 8;
                                tempLocation.Height = 26;
                                tempLocation.Y += 2;
                                if(CheckCollision(Player[A].Location, tempLocation))
                                {
                                    PlaySound(SFX_Key);
                                    StopMusic();
                                    LevelMacro = LEVELMACRO_KEYHOLE_EXIT;
                                    LevelMacroWhich = B;
                                    break;
                                }
                            }
                        }
                    }

                    if(Player[A].SwordPoke < 0)
                    {
                        Player[A].SwordPoke -= 1;

                        if(Player[A].SwordPoke == -7)
                            Player[A].SwordPoke = 1;

                        if(Player[A].SwordPoke == -40)
                            Player[A].SwordPoke = 0;

                        if(!(wasSlippy && !Player[A].Controls.Left && !Player[A].Controls.Right))
                        {
                            if(Player[A].FireBallCD == 0 && Player[A].Location.SpeedX != 0)
                                Player[A].SwordPoke = 0;
                        }
                    }
                    else if(Player[A].SwordPoke > 0)
                    {
                        if(Player[A].SwordPoke == 1)
                        {
                            TailSwipe(A, true, true);
                            PlaySoundSpatial(SFX_HeroStab, Player[A].Location);

                            if((Player[A].State == 3 || Player[A].State == 7 || Player[A].State == 6) && Player[A].FireBallCD2 == 0)
                            {
                                Player[A].FireBallCD2 = 40;
                                if(Player[A].State == 6)
                                    Player[A].FireBallCD2 = 25;

                                if(Player[A].State == 6)
                                    PlaySoundSpatial(SFX_HeroSwordBeam, Player[A].Location);
                                else if(Player[A].State == 7)
                                    PlaySoundSpatial(SFX_HeroIce, Player[A].Location);
                                else
                                    PlaySoundSpatial(SFX_HeroFireRod, Player[A].Location);

                                numNPCs++;
                                NPC[numNPCs] = NPC_t();

                                if(ShadowMode)
                                    NPC[numNPCs].Shadow = true;

                                NPC[numNPCs].Type = NPCID_PLR_FIREBALL;

                                if(Player[A].State == 7)
                                    NPC[numNPCs].Type = NPCID_PLR_ICEBALL;

                                if(Player[A].State == 6)
                                    NPC[numNPCs].Type = NPCID_SWORDBEAM;

                                NPC[numNPCs].Projectile = true;
                                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                                NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 + (40 * Player[A].Direction) - 8;

                                if(!Player[A].Duck)
                                {
                                    NPC[numNPCs].Location.Y = Player[A].Location.Y + 5;
                                    if(Player[A].State == 6)
                                        NPC[numNPCs].Location.Y += 7;
                                }
                                else
                                {
                                    NPC[numNPCs].Location.Y = Player[A].Location.Y + 18;
                                    if(Player[A].State == 6)
                                        NPC[numNPCs].Location.Y += 4;
                                }


                                NPC[numNPCs].Active = true;
                                NPC[numNPCs].TimeLeft = 100;
                                NPC[numNPCs].Location.SpeedY = 20;
                                NPC[numNPCs].CantHurt = 100;
                                NPC[numNPCs].CantHurtPlayer = A;
                                NPC[numNPCs].Special = Player[A].Character;

                                if(NPC[numNPCs].Type == NPCID_PLR_FIREBALL)
                                    NPC[numNPCs].Frame = 16;

                                NPC[numNPCs].WallDeath = 5;
                                NPC[numNPCs].Location.SpeedY = 0;
                                NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3);

                                if(Player[A].State == 6)
                                    NPC[numNPCs].Location.SpeedX = 9 * Player[A].Direction + (Player[A].Location.SpeedX / 3);

                                if(Player[A].StandingOnNPC != 0)
                                    NPC[numNPCs].Location.Y += -Player[A].Location.SpeedY;

                                syncLayers_NPC(numNPCs);
                                CheckSectionNPC(numNPCs);
                            }
                        }
                        else
                            TailSwipe(A, false, true);

                        Player[A].SwordPoke += 1;

                        if(Player[A].Duck)
                        {
                            if(Player[A].SwordPoke >= 10)
                            {
                                Player[A].SwordPoke = 0;
                                Player[A].FireBallCD = 7;
                            }
                        }
                        else
                        {
                            if(Player[A].SwordPoke >= 10)
                            {
                                Player[A].SwordPoke = -11;
                                Player[A].FireBallCD = 0;
                            }
                        }
                    }

                    if(Player[A].FireBallCD == 0 && Player[A].Wet == 0 && !Player[A].Fairy && Player[A].Mount == 0)
                    {
                        if(!Player[A].Duck && Player[A].Location.SpeedY < Physics.PlayerGravity && Player[A].StandingOnNPC == 0 &&
                            Player[A].Slope == 0 && !Player[A].Controls.Up && !Player[A].Stoned) // Link ducks when jumping
                        {
                            Player[A].SwordPoke = 0;
                            Player[A].Duck = true;
                            Player[A].Location.Y += Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y += -Player[A].Location.Height;
                        }
                        else if(Player[A].Duck && Player[A].Location.SpeedY > Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) // Link stands when falling
                        {
                            Player[A].SwordPoke = 0;
                            UnDuck(Player[A]);
                        }
                    }

                    if(Player[A].Mount > 0 && Player[A].Mount != 2)
                    {
                        PlayerHurt(A);
                        Player[A].Mount = 0;
                    }

                    Player[A].HoldingNPC = -1;
                }

                Player[A].FloatRelease = !Player[A].Controls.Jump;

                // Player interactions
                Player[A].Location.SpeedX += Player[A].Bumped2;
                Player[A].Location.X += Player[A].Bumped2;
                Player[A].Bumped2 = 0;
                if(Player[A].Mount == 0)
                    Player[A].YoshiYellow = false;

                // When it's true - don't check horizontal section's bounds
                bool hBoundsHandled = false;

                // level wrap
                if(LevelWrap[Player[A].Section] || LevelVWrap[Player[A].Section])
                    PlayerLevelWrapLogic(A);

                if(LevelWrap[Player[A].Section])
                    hBoundsHandled = true;

                // Walk offscreen exit
                if(!hBoundsHandled && OffScreenExit[Player[A].Section])
                {
                    PlayerOffscreenExitCheck(A);
                    hBoundsHandled = true;
                }

                if(LevelMacro == LEVELMACRO_CARD_ROULETTE_EXIT || LevelMacro == LEVELMACRO_GOAL_TAPE_EXIT || GameMenu)
                    hBoundsHandled = true;

                if(!hBoundsHandled)
                {
                    // Check edge of levels
                    if(Player[A].Location.X < level[Player[A].Section].X)
                    {
                        Player[A].Location.X = level[Player[A].Section].X;
                        if(Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX = 0;
                        Player[A].Pinched.Left2 = 2;
                        if(AutoX[Player[A].Section] != 0.0f)
                        {
                            Player[A].Pinched.Moving = 2;
                            Player[A].Pinched.MovingLR = true;
                        }
                    }
                    else if(Player[A].Location.X + Player[A].Location.Width > level[Player[A].Section].Width)
                    {
                        Player[A].Location.X = level[Player[A].Section].Width - Player[A].Location.Width;
                        if(Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX = 0;
                        Player[A].Pinched.Right4 = 2;
                        if(AutoX[Player[A].Section] != 0.f)
                        {
                            Player[A].Pinched.Moving = 2;
                            Player[A].Pinched.MovingLR = true;
                        }
                    }
                }

                if(Player[A].Location.Y < level[Player[A].Section].Y - Player[A].Location.Height - 32 && Player[A].StandingOnVehiclePlr == 0)
                {
                    Player[A].Location.Y = level[Player[A].Section].Y - Player[A].Location.Height - 32;
                    if(AutoY[Player[A].Section] != 0.f)
                    {
                        Player[A].Pinched.Moving = 3;
                        Player[A].Pinched.MovingUD = true;
                    }
                }

                // gives the players the sparkles when he is flying
                if(
                        (
                                (!Player[A].YoshiBlue && (Player[A].CanFly || Player[A].CanFly2)) ||
                                (Player[A].Mount == 3 && Player[A].CanFly2)
                        ) || Player[A].FlySparks
                        )
                {
                    if(iRand(4) == 0)
                    {
                        NewEffect(EFFID_SPARKLE,
                                  newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                                  Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)),
                                                  1, 0, ShadowMode);
                        Effect[numEffects].Location.SpeedX = (dRand() * 0.5) - 0.25;
                        Effect[numEffects].Location.SpeedY = (dRand() * 0.5) - 0.25;
                    }
                }

                Tanooki(A); // tanooki suit code

                float oldSpeedY = Player[A].Location.SpeedY; // holds the players previous Y speed

                // this whole section is dead code, since there are no uses of these definitions of PlrMid and Slope
#if 0
                if(Player[A].StandingOnNPC == -A)
                {
                    if(Player[A].Slope != 0)
                    {
                        B = NPC[Player[A].StandingOnNPC].Special;

                        double PlrMid;
                        if(BlockSlope[Block[B].Type] == 1)
                            PlrMid = Player[A].Location.X;
                        else
                            PlrMid = Player[A].Location.X + Player[A].Location.Width;

                        double Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;

                        if(BlockSlope[Block[B].Type] < 0)
                            Slope = 1 - Slope;
                        if(Slope < 0)
                            Slope = 0;
                        if(Slope > 1)
                            Slope = 1;
                    }
                }
#endif

                // decrement pinched timers
                PlayerPinchedTimerUpdate(A);

                // Block collisions.
                bool DontResetGrabTime = false; // helps with grabbing things from the top
                bool movingBlock = false; // helps with collisions for moving blocks
                int tempHit3 = 0;
                PlayerBlockLogic(A, tempHit3, movingBlock, DontResetGrabTime, cursed_value_C);

                // Vine collisions.
                PlayerVineLogic(A);


                // Check NPC collisions
                int MessageNPC = 0;
                PlayerNPCLogic(A, tempSpring, tempShell, MessageNPC, movingBlock, tempHit3, oldSpeedY);

                // reduce player's multiplier
                if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && !Player[A].Slide && !FreezeNPCs)
                    Player[A].Multiplier = 0;

                if(Player[A].Mount == 2)
                    Player[A].Multiplier = 0;

                // Player-player collisions
                PlayerCollide(A);

                // Talk to NPC
                if(MessageNPC > 0)
                {
                    MessageText = GetS(NPC[MessageNPC].Text);
                    preProcessMessage(MessageText, A);
                    PauseGame(PauseCode::Message, A);
                    MessageText.clear();
                    MessageTextMap.clear();

                    if(NPC[MessageNPC].TriggerTalk != EVENT_NONE)
                        ProcEvent(NPC[MessageNPC].TriggerTalk, A);

                    MessageNPC = 0;
                }

                YoshiEatCode(A);

                // pinch code
                if(!GodMode)
                    PlayerPinchedDeathCheck(A);

                SuperWarp(A); // this sub checks warps

                // shell surf
                if(Player[A].ShellSurf && Player[A].StandingOnNPC != 0)
                {
                    Player[A].Location.X = NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                    Player[A].Location.SpeedX = 0; // 1 * .Direction

                    if(NPC[Player[A].StandingOnNPC].Location.SpeedX == 0)
                        Player[A].ShellSurf = false;
                }

                // Check edge of screen
                PlayerSharedScreenLogic(A);

                if(Player[A].Location.Y > level[Player[A].Section].Height + 64)
                    PlayerDead(A);

                if(!NPC[Player[A].StandingOnNPC]->IsAShell)
                    Player[A].ShellSurf = false;

                PlayerGrabCode(A, DontResetGrabTime); // Player holding NPC code **GRAB CODE**

                Player[A].RunRelease = !Player[A].Controls.Run && !Player[A].Controls.AltRun;
                Player[A].JumpRelease = !Player[A].Controls.Jump && !Player[A].Controls.AltJump;

                PlayerFrame(Player[A]); // Update players frames
                Player[A].StandUp = false; // Fixes a block collision bug
                Player[A].StandUp2 = false;
                if(Player[A].ForceHitSpot3)
                    Player[A].StandUp = true;
                Player[A].ForceHitSpot3 = false;
                if(Player[A].ForceHold > 0)
                    Player[A].ForceHold -= 1;
            }
            else // Player special effects
                PlayerEffects(A);
        }

        Player[A].DuckRelease = !Player[A].Controls.Down;
        Player[A].AltRunRelease = !Player[A].Controls.AltRun;
    }

    CleanupVehicleNPCs();
}

void CleanupVehicleNPCs()
{
    // int C = 0;

    // kill the player temp NPCs, from last to first
    std::sort(NPCQueues::PlayerTemp.begin(), NPCQueues::PlayerTemp.end(),
    [](NPCRef_t a, NPCRef_t b)
    {
        return a > b;
    });


    // for(A = numNPCs; A >= 1; A--)
    int last_NPC = numNPCs + 1;
    for(int A : NPCQueues::PlayerTemp)
    {
        // duplicated entry, no problem
        if(A == last_NPC)
            continue;

        if(NPC[A].playerTemp)
        {
            for(int B = 1; B <= numPlayers; B++)
            {
                if(Player[B].StandingOnNPC == A)
                    Player[B].StandingOnVehiclePlr = NPC[A].Variant; // newly stores the player who owns the vehicle
            }
            NPC[0] = NPC[A]; // was NPC[C] = NPC[A] but C was not mutated
            KillNPC(A, 9);
        }
    }

    NPCQueues::PlayerTemp.clear();
}
