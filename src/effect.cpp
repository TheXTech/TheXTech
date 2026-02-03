/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "config.h"

#include "effect.h"
#include "npc.h"
#include "npc_id.h"
#include "eff_id.h"
#include "npc_traits.h"
#include "sound.h"
#include "game_main.h"
#include "collision.h"
#include "layers.h"
#include "graphics/gfx_update.h"

#include "main/trees.h"

// should be tuned based on profiling of Effect-heavy cases (consider Col.'s Cathedral in SRW2)
static constexpr int s_kill_stack_size = 16;

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

    vbint_t killed_effects[s_kill_stack_size];
    int num_killed = 0;

    For(A, 1, numEffects)
    {
        auto &e = Effect[A];
        e.Life -= 1;

        // moved code for Life == 0 for EFFID_MINIBOSS_DIE below

        e.Location.X += e.Location.SpeedX;
        e.Location.Y += e.Location.SpeedY;

        if(e.Type == EFFID_FODDER_S3_SQUISH || e.Type == EFFID_FODDER_S5_SQUISH || e.Type == EFFID_RED_FODDER_SQUISH || e.Type == EFFID_UNDER_FODDER_SQUISH || e.Type == EFFID_EXT_TURTLE_SQUISH || e.Type == EFFID_YELSWITCH_FODDER_SQUISH || e.Type == EFFID_BLUSWITCH_FODDER_SQUISH || e.Type == EFFID_GRNSWITCH_FODDER_SQUISH || e.Type == EFFID_REDSWITCH_FODDER_SQUISH || e.Type == EFFID_BIG_FODDER_SQUISH || e.Type == EFFID_FODDER_S1_SQUISH || e.Type == EFFID_HIT_TURTLE_S4_SQUISH) // Stomped Goombas
        {
            e.Location.SpeedY = 0;
            e.Location.SpeedX = 0;
        }
        else if(e.Type == EFFID_BOSS_FRAGILE_DIE)
        {
            if(e.Life % 5 == 0)
            {
                NewEffect(EFFID_BOSS_FRAGILE_EXPLODE, newLoc(e.Location.X + (dRand() * (int_ok)e.Location.Width),
                                      e.Location.Y + (dRand() * (int_ok)e.Location.Height)));
            }
        }
        else if(e.Type == EFFID_BOSS_CASE_BREAK)
        {
            e.Location.SpeedY += 0.5_n;
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
        else if(e.Type == EFFID_BOSS_FRAGILE_EXPLODE)
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
        else if(e.Type == EFFID_FIRE_DISK_DIE) // RotoDisk
        {
            if(e.Location.SpeedX != 0 || e.Location.SpeedY != 0)
                e.Location.SpeedY += 0.5_n;
            e.Frame += 1;
            if(e.Frame >= 5)
                e.Frame = 0;
        }
        else if(e.Type == EFFID_BOMB_S2_EXPLODE) // bomb
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
        else if(e.Type == EFFID_BLOCK_SMASH || e.Type == EFFID_BLU_BLOCK_SMASH || e.Type == EFFID_SLIDE_BLOCK_SMASH
            || e.Type == EFFID_BLOCK_S1_SMASH || e.Type == EFFID_GRY_BLOCK_SMASH || e.Type == EFFID_DIRT_BLOCK_SMASH
            || e.Type == EFFID_ITEM_POD_BREAK) // Block break
        {
            e.Location.SpeedY += 0.6_n;
            e.Location.SpeedX = e.Location.SpeedX * 0.99_r;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;

            if(e.Type != EFFID_ITEM_POD_BREAK)
            {
                e.FrameCount += 1;
                if(e.FrameCount >= 3)
                {
                    e.FrameCount = 0;
                    e.Frame += 1;
                    if(e.Frame == 4)
                        e.Frame = 0;
                }
            }
        }
        else if(e.Type == EFFID_MAGIC_BOSS_DIE) // larry shell
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.Frame += 1;
                e.FrameCount = 0;
            }

            if(e.Frame > 7)
                e.Frame = 0;

            if(HasSound(SFX_MagicBossShell) && g_config.sfx_modern) // When new sound is presented, do animation a bit differently
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
            else if(e.Life == 100) // Old sound
            {
                e.Location.SpeedY = -8;
                PlaySoundSpatial(SFX_SickBossKilled, e.Location);
            }
        }
        else if(e.Type == EFFID_WATER_SPLASH) // Splash
        {
            e.FrameCount += 1;

            // simplified logic
            if(e.FrameCount < 40)
                e.Frame = e.FrameCount / 8;
            else
                e.Life = 0;

            if(e.FrameCount % 3 == 0)
                e.Frame = 5;
        }
        else if(e.Type == EFFID_AIR_BUBBLE) // Water Bubbles
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
            e.Location.X += dRand() * 2 - 1;
        }
        else if(e.Type == EFFID_CHAR1_DIE || e.Type == EFFID_CHAR2_DIE || e.Type == EFFID_CHAR3_DIE || e.Type == EFFID_CHAR4_DIE || e.Type == EFFID_CHAR5_DIE) // Mario & Luigi death
            e.Location.SpeedY += 0.25_n;
        else if(e.Type == EFFID_ITEM_THROWER_DIE || e.Type == EFFID_WALL_SPARK_DIE || e.Type == EFFID_FODDER_S5_DIE || e.Type == EFFID_FODDER_S3_DIE || e.Type == EFFID_FIRE_BOSS_DIE || e.Type == EFFID_VINE_BUG_DIE || e.Type == EFFID_RED_FODDER_DIE || e.Type == EFFID_UNDER_FODDER_DIE || e.Type == EFFID_SPIKY_BALL_S3_DIE || e.Type == EFFID_CRAB_DIE || e.Type == EFFID_FLY_DIE || e.Type == EFFID_YELSWITCH_FODDER_DIE || e.Type == EFFID_BLUSWITCH_FODDER_DIE || e.Type == EFFID_GRNSWITCH_FODDER_DIE || e.Type == EFFID_REDSWITCH_FODDER_DIE || e.Type == EFFID_BIG_FODDER_DIE || e.Type == EFFID_FODDER_S1_DIE || e.Type == EFFID_SQUID_S3_DIE) // Goomba air ride of dooom
        {
            e.Location.SpeedY += 0.5_n;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.Type == EFFID_WALL_SPARK_DIE || e.Type == EFFID_FIRE_BOSS_DIE)
                e.FrameCount += 1;
            if(e.FrameCount >= 8)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 2)
                    e.Frame = 0;
            }
        }
        else if(e.Type == EFFID_LAVA_MONSTER_LOOK) // Blaarg eyes
        {
            e.Life += 2;
            if(e.Life <= 30)
                e.Location.SpeedY = -2.8_n;
            else if(e.Life <= 40)
                e.Location.SpeedY = 0.5_n;
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
        else if(e.Type == EFFID_HIT_TURTLE_S4_DIE) // Beack Koopa
        {
            e.Location.SpeedY += 0.5_n;
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
        // these are non-animated standard death effects
        else if(e.Type == EFFID_GRN_SHELL_S3_DIE || e.Type == EFFID_RED_SHELL_S3_DIE || e.Type == EFFID_BULLET_DIE
            || e.Type == EFFID_BIG_BULLET_DIE || e.Type == EFFID_GLASS_SHELL_DIE || e.Type == EFFID_SPIKY_S3_DIE
            || e.Type == EFFID_SPIKY_S4_DIE || e.Type == EFFID_SPIT_BOSS_BALL_DIE || e.Type == EFFID_SPIT_BOSS_DIE
            || e.Type == EFFID_SPIKY_THROWER_DIE || e.Type == EFFID_EXT_TURTLE_DIE || e.Type == EFFID_BIG_SHELL_DIE
            || e.Type == EFFID_SHELL_S4_DIE || e.Type == EFFID_GRN_SHELL_S1_DIE || e.Type == EFFID_RED_SHELL_S1_DIE
            || e.Type == EFFID_WALL_TURTLE_DIE || e.Type == EFFID_HEAVY_THROWER_DIE || e.Type == EFFID_POWER_S3_DIE
            || e.Type == EFFID_JUMPER_S4_DIE || e.Type == EFFID_VILLAIN_S3_DIE || e.Type == EFFID_SPIT_GUY_BALL_DIE
            || e.Type == EFFID_WALK_BOMB_S3_DIE || e.Type == EFFID_CHASER_DIE || e.Type == EFFID_STONE_S3_DIE
            || e.Type == EFFID_BIG_GHOST_DIE || e.Type == EFFID_GHOST_S4_DIE || e.Type == EFFID_GHOST_FAST_DIE
            || e.Type == EFFID_GHOST_S3_DIE || e.Type == EFFID_STONE_S4_DIE || e.Type == EFFID_SAW_DIE
            || e.Type == EFFID_VILLAIN_S1_DIE || e.Type == EFFID_BOMBER_BOSS_DIE || e.Type == EFFID_SICK_BOSS_DIE
            || e.Type == EFFID_BAT_DIE) // Flying turtle shell / Bullet bill /hard thing, combined with section with comment "Bullet Bill / Hammer Bro"
        {
            e.Location.SpeedY += 0.5_n;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
        }
        else if(e.Type == EFFID_GRN_BOOT_DIE || e.Type == EFFID_RED_BOOT_DIE || e.Type == EFFID_BLU_BOOT_DIE) // Goombas shoes
        {
            e.Location.SpeedY += 0.5_n;

            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;

            if(e.Location.SpeedX > 0)
                e.Frame = 0 + SpecialFrame[1];
            else
                e.Frame = 2 + SpecialFrame[1];
        }
        else if(e.Type == EFFID_SMOKE_S3 || e.Type == EFFID_SMOKE_S4) // SMW / SMB3 Puff of smoke
        {
            // no longer double SpeedX for SMOKE_S3 or SMOKE_S4
            // multiplied SpeedX constants by 2 elsewhere.

            // e.Location.X += e.Location.SpeedX;
            e.FrameCount += 1;
            if(e.FrameCount >= 3)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Life = 0;
            }
        }
        else if(e.Type == EFFID_SMOKE_S2) // SMB2 Puff of smoke
        {
            // this did nothing in SMBX 1.3 because SpeedX was always 0
            // e.Location.X += e.Location.SpeedX;

            e.FrameCount += 1;
            if(e.FrameCount >= 6)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Life = 0;
            }
        }
        else if(e.Type == EFFID_STOMP_INIT) // stomp stars
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 3)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 2)
                {
                    e.Life = 0;
                    NewEffect(EFFID_STOMP_STAR, e.Location);
                }
            }
        }
        else if(e.Type == EFFID_STOMP_STAR) // stomp stars
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
        else if(e.Type == EFFID_WHIP) // Tail whack
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
        else if(e.Type == EFFID_WHACK) // Whack
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
        else if(e.Type == EFFID_BOOT_STOMP)
        {
            // doubled speed in NewEffect
            // e.Location.X += e.Location.SpeedX;
            // e.Location.Y += e.Location.SpeedY;
        }
        else if(e.Type == EFFID_COIN_SWITCH_PRESS || e.Type == EFFID_TIME_SWITCH_PRESS || e.Type == EFFID_TNT_PRESS) // P Switch
        {
            if(e.Life == 1)
                NewEffect(EFFID_SMOKE_S3_CENTER, e.Location);
        }
        else if(e.Type == EFFID_SKID_DUST) // Slide Smoke
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.Frame += 1;
                e.FrameCount = 0;
                if(e.Frame > 2)
                    e.Life = 0;
            }
        }
        else if(e.Type == EFFID_SMOKE_S5) // Zelda Smoke
        {
            // this had no effect because SpeedX/SpeedY was always zero for EFFID_SMOKE_S5 in SMBX 1.3
            // e.Location.X += e.Location.SpeedX;
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame == 4)
                    e.Life = 0;
            }
        }
        else if(e.Type == EFFID_COIN_BLOCK_S3) // Coin out of block effect
        {
            if(e.Life == 1)
            {
                CoinCount += 1;
                if(CoinCount > 13)
                    CoinCount = 10;
                MoreScore(CoinCount, e.Location);
            }

            // simplified logic
            if(e.Life <= 6)
            {
                // NOTE: 4, 5, 6 are valid frames.
                // This sets Frame to 7 on the frame when Life is 0.
                e.Frame = 4 + (6 - e.Life) / 2;
                // NOTE: The same behavior at VB6 code: just checks the boolean expression and writes 0 or -1
                e.Location.SpeedY = (e.Location.SpeedY == 0) ? 0 : -1;
            }
            else
            {
                e.Location.SpeedY += 0.4_n;
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
        else if(e.Type == EFFID_BIG_FIREBALL_TAIL) // Big Fireball Tail
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
        else if(e.Type == EFFID_COIN_COLLECT) // Coin
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
        else if(e.Type == EFFID_SPINBLOCK) // Spinning block
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
        else if(e.Type == EFFID_SPARKLE) // Twinkle
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
        else if(e.Type == EFFID_PLR_FIREBALL_TRAIL || e.Type == EFFID_PLR_ICEBALL_TRAIL) // Small Fireball Tail
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
        else if(e.Type == EFFID_LAVA_SPLASH) // Big Fireball Tail
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
        else if(e.Type == EFFID_MINIBOSS_DIE) // Dead Big Koopa
        {
            // moved from top
            if(e.Life == 0)
            {
                if(e.NewNpc > 0)
                {
                    numNPCs++;
                    auto &nn = NPC[numNPCs];
                    nn.Type = NPCID(e.NewNpc);

                    nn.Location.Height = nn->THeight;
                    nn.Location.Width = nn->TWidth;
                    nn.Location.X = e.Location.X + (e.Location.Width - NPC[numNPCs].Location.Width) / 2;
                    nn.Location.Y = e.Location.Y - 1;
                    nn.Location.SpeedY = -6;

                    // this would fix the fact that the code was moved from before the effect's speed was applied, but SpeedX / SpeedY is always 0 for EFFID_MINIBOSS_DIE
                    // nn.Location.X -= e.Location.SpeedX;
                    // nn.Location.Y -= e.Location.SpeedY;

                    nn.Active = true;
                    nn.TimeLeft = 100;
                    nn.Frame = 0;
                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);
                    PlaySoundSpatial(SFX_BossBeat, e.Location);
                }
            }

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
        else if(e.Type == EFFID_BOMB_S3_EXPLODE_SEED) // SMB3 Bomb Part 1
        {
            if(e.FrameCount == 0)
            {
                NewEffect(EFFID_BOMB_S3_EXPLODE, e.Location, e.Frame);
                e.Frame += 1;
                if(e.Frame >= 4)
                    e.Frame = 0;
            }
            else if(e.FrameCount >= 6)
                e.FrameCount = -1;
            e.FrameCount += 1;
        }
        else if(e.Type == EFFID_BOMB_S3_EXPLODE || e.Type == EFFID_CHAR3_HEAVY_EXPLODE) // SMB3 Bomb Part 2
        {
            e.FrameCount += 1;
            if(e.FrameCount >= 4)
            {
                e.FrameCount = 0;
                e.Frame += 1;
                if(e.Frame >= 4)
                    e.Frame = 0;
            }

            if(e.Type == EFFID_CHAR3_HEAVY_EXPLODE && iRand(10) >= 8)
            {
                NewEffect(EFFID_PLR_FIREBALL_TRAIL, e.Location, 3);
                Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5_n;
                Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5_n;
            }
        }
        else if(e.Type == EFFID_EARTHQUAKE_BLOCK_HIT) // POW Block
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
        else if(e.Type == EFFID_DOOR_S2_OPEN || e.Type == EFFID_DOOR_DOUBLE_S3_OPEN || e.Type == EFFID_DOOR_SIDE_S3_OPEN || e.Type == EFFID_BIG_DOOR_OPEN) // door
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
        else if(e.Type == EFFID_STACKER_DIE)
        {
            e.Location.SpeedY += 0.5_n;
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

            // don't combine with below, because it's lopsided: 9 final values (0-8) use Frame 5, and 7 final values (9-15) use Frame 4
        }
        else if(e.Type == EFFID_RED_GUY_DIE || e.Type == EFFID_BLU_GUY_DIE || e.Type == EFFID_JUMPER_S3_DIE
            || e.Type == EFFID_RED_FISH_S1_DIE || (e.Type >= EFFID_BIRD_DIE && e.Type <= EFFID_GRY_SPIT_GUY_DIE)
            || e.Type == EFFID_CARRY_BUDDY_DIE // Shy guy free falling (uses frames 4-7)
            || e.Type == EFFID_BRUTE_SQUISHED_DIE || e.Type == EFFID_BRUTE_DIE || e.Type == EFFID_BIG_GUY_DIE
            || e.Type == EFFID_CARRY_FODDER_DIE || e.Type == EFFID_SKELETON_DIE || e.Type == EFFID_GRN_FISH_S3_DIE
            || e.Type == EFFID_YEL_FISH_S4_DIE || e.Type == EFFID_RED_FISH_S3_DIE || e.Type == EFFID_GRN_FISH_S4_DIE
            || e.Type == EFFID_GRN_FISH_S1_DIE || e.Type == EFFID_BONE_FISH_DIE || e.Type == EFFID_SQUID_S1_DIE
            || e.Type == EFFID_WALK_PLANT_DIE // Rex / mega mole / smw goomba free falling
            )
        {
            e.Location.SpeedY += 0.5_n;
            if(e.Location.SpeedY >= 10)
                e.Location.SpeedY = 10;
            e.FrameCount += 1;
            if(e.FrameCount >= 8)
            {
                e.FrameCount = 0;
                // flip least significant bit to switch frame while keeping same frame set
                e.Frame ^= 1;
            }
        }
        else if(e.Type == EFFID_ITEM_POD_OPEN) // Egg
        {
            if(e.NewNpc == 0 && e.FrameCount < 19)
                e.FrameCount = 19;
            e.FrameCount += 1;
            if(e.FrameCount == 10)
                e.Frame += 1;
            else if(e.FrameCount == 20)
            {
                e.Frame = 2;
                NewEffect(EFFID_ITEM_POD_BREAK, e.Location);
            }
            else if(e.FrameCount == 30)
            {
                e.Life = 0;
                if(!LevelEditor && e.NewNpc != NPCID_ITEM_POD)
                {
                    if(NPCIsYoshi((NPCID)e.NewNpc))
                    {
                        if(NewEffect(EFFID_PET_BIRTH, e.Location, 1))
                            Effect[numEffects].NewNpc = e.NewNpc;
                    }
                    else if(e.NewNpc > 0)
                    {
                        numNPCs++;
                        auto &nn = NPC[numNPCs];
                        nn.Location = e.Location;
                        nn.Active = true;
                        nn.TimeLeft = 100;
                        nn.Direction = 0;
                        nn.Type = NPCID(e.NewNpc);
                        nn.Location.Height = nn->THeight;
                        nn.Location.Width = nn->TWidth;
                        nn.Location.Y += 32 - nn.Location.Height;
                        nn.Location.X += -nn.Location.Width / 2 + 16;

                        if(nn.Type == NPCID_LEAF_POWER)
                            nn.Location.SpeedY = -6;

                        // this is new TheXTech logic
                        if(NPCTraits[e.NewNpc].IsFish || NPCIsAParaTroopa((NPCID)e.NewNpc) || e.NewNpc == NPCID_FIRE_CHAIN)
                        {
                            nn.Special = e.NewNpcSpecial;
                            nn.DefaultSpecial = e.NewNpcSpecial;
                        }

                        // this is new TheXTech logic
                        if(e.NewNpc == NPCID_STAR_EXIT || e.NewNpc == NPCID_STAR_COLLECT || e.NewNpc == NPCID_MEDAL)
                            nn.Variant = e.NewNpcSpecial;

                        syncLayers_NPC(numNPCs);
                        CheckSectionNPC(numNPCs);
                    }
                }
            }
        }
        else if(e.Type == EFFID_SPACE_BLOCK_SMASH)
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
        else if(e.Type == EFFID_PET_BIRTH) // yoshi grow
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
                nn.Location = e.Location;
                nn.Active = true;
                nn.TimeLeft = 100;
                nn.Direction = 1;
                nn.Type = NPCID(e.NewNpc);
                nn.Location.Height = nn->THeight;
                nn.Location.Width = nn->TWidth;
                syncLayers_NPC(numNPCs);
                CheckSectionNPC(numNPCs);
            }

            if(e.NewNpc == NPCID_PET_BLUE)
                e.Frame += 2;
            else if(e.NewNpc == NPCID_PET_YELLOW)
                e.Frame += 4;
            else if(e.NewNpc == NPCID_PET_RED)
                e.Frame += 6;
            else if(e.NewNpc == NPCID_PET_BLACK)
                e.Frame += 8;
            else if(e.NewNpc == NPCID_PET_PURPLE)
                e.Frame += 10;
            else if(e.NewNpc == NPCID_PET_PINK)
                e.Frame += 12;
            else if(e.NewNpc == NPCID_PET_CYAN)
                e.Frame += 14;
        }
        else if(e.Type == EFFID_SCORE)
            e.Location.SpeedY = e.Location.SpeedY * 0.97_r;

        // check for killed (lets us only do a single loop over effects)
        if(e.Life <= 0)
        {
            if(num_killed < s_kill_stack_size)
                killed_effects[num_killed] = A;

            num_killed++;
        }
    } //for

    if(num_killed > s_kill_stack_size)
    {
        for(int A = numEffects; A >= 1; --A)
        {
            if(Effect[A].Life <= 0)
                KillEffect(A);
        }
    }
    else
    {
        for(int A = num_killed - 1; A >= 0; --A)
            KillEffect(killed_effects[A]);
    }
}

bool NewEffect(EFFID A, const Location_t &Location, int Direction, bool Shadow)
{
// this sub creates effects
// please reference the /graphics/effect folder to see what the effects are
// A is the effect type

    int B = 0;
    bool tempBool = false;
    num_t tempDoub = 0;

    if(numEffects >= maxEffects - 4)
        return false;

    if(A == EFFID_BLOCK_SMASH || A == EFFID_BLU_BLOCK_SMASH || A == EFFID_SLIDE_BLOCK_SMASH || A == EFFID_BLOCK_S1_SMASH || A == EFFID_GRY_BLOCK_SMASH || A == EFFID_DIRT_BLOCK_SMASH || A == EFFID_ITEM_POD_BREAK) // Block break effect
    {
        for(B = 1; B <= 4; B++)
        {
            numEffects++;
            auto &ne = Effect[numEffects];
            ne.Shadow = Shadow;
            ne.Location.Width = EffectWidth[A];
            ne.Location.Height = EffectHeight[A];
            ne.Type = A;
            ne.Life = 200;

            ne.Location.X = Location.X;
            ne.Location.Y = Location.Y;
            ne.Location.SpeedX = 3;

            if(B == 1 || B == 2)
                ne.Location.SpeedY = -11;
            else
            {
                ne.Location.SpeedY = -7;
                ne.Location.Y += Location.Height / 2;
            }

            if(A == EFFID_ITEM_POD_BREAK)
            {
                if(B == 1 || B == 2)
                    ne.Location.SpeedX = 2;
                else
                    ne.Location.SpeedX = 1.5_n;

                if(B == 1)
                    ne.Frame = 0;
                else if(B == 2)
                    ne.Frame = 1;
                else if(B == 3)
                    ne.Frame = 3;
                else
                    ne.Frame = 2;
            }

            if(B == 1 || B == 3)
                ne.Location.SpeedX = -ne.Location.SpeedX;
            else
                ne.Location.X += Location.Width / 2;

            auto rand_mul = (A == EFFID_ITEM_POD_BREAK) ? 0.25_rb : 1.0_rb;
            ne.Location.SpeedX += (dRand() * 2 - 1) * rand_mul;
            ne.Location.SpeedY += (dRand() * 4 - 2) * rand_mul;
        }
    }
    else if(A == EFFID_MAGIC_BOSS_DIE) // larry shell
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        // ne.NewNpc = NewNpc; (unused)
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 160;
        ne.Type = A;
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        PlaySoundSpatial(SFX_MagicBossKilled, Location);
    }
    else if(A == EFFID_LAVA_MONSTER_LOOK) // Blaarg eyes
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        // ne.NewNpc = NewNpc; (unused)
        ne.Shadow = Shadow;
        if(ne.NewNpc == NPCID_ITEM_POD)
            ne.NewNpc = 0;
        if(Direction == -1)
            ne.Location.X = Location.X + Location.Width / 2 + 16 + 48 * Direction;
        else
            ne.Location.X = Location.X + Location.Width / 2 + 16; // + 48 * Direction

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
    else if(A == EFFID_ITEM_POD_OPEN || A == EFFID_PET_BIRTH) // Egg break / Yoshi grow
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.NewNpc = 0; // NewNpc -- now gets set at callsite
        ne.NewNpcSpecial = 0; // NewNpc -- now gets set at callsite
        // if(ne.NewNpc == NPCID_ITEM_POD) (logic moved to callsite)
        //     ne.NewNpc = 0;
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

        if(A == EFFID_ITEM_POD_OPEN)
        {
            // moved to callsite
            // if(ne.NewNpc != 0 /*&& ne.NewNpc != NPCID_ITEM_POD*/) // never 96, because of condition above that replaces 96 with zero
            //     PlaySoundSpatial(SFX_PetBirth, Location);
            // else
            //     PlaySoundSpatial(SFX_Smash, Location);
        }
        else if(A == EFFID_PET_BIRTH)
            PlaySoundSpatial(SFX_Pet, Location);
    }
    else if(A == EFFID_FIRE_DISK_DIE) // Roto Disk
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedX = Location.SpeedX;
        ne.Location.SpeedY = Location.SpeedY;
        ne.Location.Width = Location.Width;
        ne.Location.Height = Location.Width; // this is a bug
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 10;
        ne.Type = A;

    }
    else if(A == EFFID_EARTHQUAKE_BLOCK_HIT) // pow
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        // ne.NewNpc = NewNpc; (unused)
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Frame = 0;
        ne.FrameCount = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == EFFID_SPACE_BLOCK_SMASH) // Metroid Block
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        // ne.NewNpc = NewNpc; (unused)
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
    else if(A == EFFID_FODDER_S3_SQUISH || A == EFFID_RED_FODDER_SQUISH || A == EFFID_UNDER_FODDER_SQUISH
        || A == EFFID_EXT_TURTLE_SQUISH || A == EFFID_YELSWITCH_FODDER_SQUISH || A == EFFID_BLUSWITCH_FODDER_SQUISH
        || A == EFFID_GRNSWITCH_FODDER_SQUISH || A == EFFID_REDSWITCH_FODDER_SQUISH || A == EFFID_BIG_FODDER_SQUISH
        || A == EFFID_FODDER_S1_SQUISH || A == EFFID_HIT_TURTLE_S4_SQUISH || A == EFFID_BRUTE_SQUISH
        || A == EFFID_FODDER_S5_SQUISH) // Goomba smash effect
    {
        PlaySoundSpatial(SFX_Stomp, Location); // Stomp sound
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

        if(A == EFFID_BIG_FODDER_SQUISH)
        {
            ne.Location.Height = 46;
            ne.Location.Width = 48;
        }

        if(A == EFFID_BRUTE_SQUISH)
        {
            if(Direction == 1)
                ne.Frame = 1;
        }
    }
    else if(A == EFFID_COIN_SWITCH_PRESS || A == EFFID_TIME_SWITCH_PRESS || A == EFFID_TNT_PRESS) // P Switch
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_AIR_BUBBLE || A == EFFID_WATER_SPLASH) // Water Bubble / Splash
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

        if(A == EFFID_WATER_SPLASH) // Change height for the background
        {
            for(const Background_t& b : treeBackgroundQuery(ne.Location, SORTMODE_ID))
            {
                if(b.Type == 82 || b.Type == 26 || b.Type == 65 || b.Type == 159 || b.Type == 166 || b.Type == 168)
                {
                    // note: NOT a reference
                    auto t = b.Location;
                    if(t.Height > 8 && g_config.fix_submerged_splash_effect)
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
        ne.NewNpc = 0; // NewNpc -- now gets set at callsite
        ne.Type = A;

        if(!tempBool && A == EFFID_WATER_SPLASH)
            numEffects -= 1;
    }
    else if(A == EFFID_WALL_TURTLE_DIE) // Spike Top
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
        ne.Frame = Direction;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_BOSS_FRAGILE_EXPLODE) // Metroid
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 64;
        ne.Location.Height = 64;
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 200;
        ne.Type = A;
    }
    else if(A == EFFID_SPINBLOCK) // Block Spin
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.NewNpc = 0; // NewNpc -- now gets set at callsite
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
    else if(A == EFFID_CHAR1_DIE || A == EFFID_CHAR2_DIE || A == EFFID_CHAR3_DIE || A == EFFID_CHAR4_DIE || A == EFFID_CHAR5_DIE) // Mario & Luigi died effect
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
        ne.Location.SpeedY = -11;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        if(A == EFFID_CHAR5_DIE)
        {
            if(Direction == 1)
                ne.Frame = 1;
        }
        ne.Life = 150;
        ne.Type = A;
    }
    else if(A == EFFID_SCORE) // Score
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Type = A;
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[ne.Type];
        ne.Location.Height = EffectHeight[ne.Type];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
        ne.Location.X += dRand() * 32 - 16;
        ne.Location.Y += dRand() * 32 - 16;
        ne.Location.SpeedY = -2;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 60;
    }
    else if(A == EFFID_BOOT_STOMP || A == EFFID_STOMP_STAR) // "SMW Smashed", combined with "stomp star part 2"
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
                ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
                ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;

                if(A == EFFID_BOOT_STOMP)
                {
                    ne.Life = 15;
                    ne.Location.SpeedX = 4.8_n; // 3 * 0.8; * 2 because SpeedX got applied twice for EFFID_BOOT_STOMP in SMBX 1.3
                    ne.Location.SpeedY = 2.4_n; // 1.5 * 0.8; * 2 because SpeedY got applied twice for EFFID_BOOT_STOMP in SMBX 1.3
                }
                else
                {
                    ne.Life = 8;
                    ne.Location.SpeedX = 2;
                    ne.Location.SpeedY = 2;
                }

                if(B == 1 || B == 2)
                    ne.Location.SpeedY = -ne.Location.SpeedY;
                if(B == 1 || B == 3)
                    ne.Location.SpeedX = -ne.Location.SpeedX;

                if(A == EFFID_STOMP_STAR)
                {
                    ne.Location.Y += ne.Location.SpeedY * 6;
                    ne.Location.X += ne.Location.SpeedX * 6;
                }

                ne.Frame = 0;
            }
        }
    }
    else if(A == EFFID_BOMB_S3_EXPLODE_SEED) // SMB3 Bomb Part 1
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 16;
        ne.Location.Height = 16;
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
        ne.Location.SpeedY = -0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 46;
        ne.Type = A;

    }
    else if(A == EFFID_CHAR3_HEAVY_EXPLODE) // Heart Bomb
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
                ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
                ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
                ne.Location.SpeedY = -0;
                ne.Location.SpeedX = 0;
                ne.Life = 10;
                if(B == 1 || B == 3 || B == 4 || B == 6)
                {
                    ne.Location.SpeedY = 1.75_n; // 3.5 * 0.5
                    ne.Location.SpeedX = 1; // 2 * 0.5
                }
                else
                {
                    ne.Life = 11;
                    ne.Location.SpeedY = 0;
                    ne.Location.SpeedX = 2; // 4 * 0.5
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
                // ne.Location.SpeedX = ne.Location.SpeedX * 0.5;
                // ne.Location.SpeedY = ne.Location.SpeedY * 0.5;

                ne.Location.X += ne.Location.SpeedX * 3;
                ne.Location.Y += ne.Location.SpeedY * 3;

                ne.Frame = iRand(4);
                ne.Type = A;
            }
        }


    }
    else if(A == EFFID_BOMB_S3_EXPLODE) // SMB3 Bomb Part 2
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
                    ne.Location.SpeedY = 4.5_n; // 3 * 1.5
                    ne.Location.SpeedX = 3; // 2 * 1.5
                    ne.Life = 14;
                }
                else
                {
                    ne.Location.SpeedY = 0;
                    ne.Location.SpeedX = 6; // 4 * 1.5
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
                // ne.Location.SpeedX = ne.Location.SpeedX * 1.5;
                // ne.Location.SpeedY = ne.Location.SpeedY * 1.5;
                ne.Frame = Direction;
                ne.Type = A;
            }
        }
    }
    else if(A == EFFID_DOOR_S2_OPEN || A == EFFID_DOOR_DOUBLE_S3_OPEN
        || A == EFFID_DOOR_SIDE_S3_OPEN || A == EFFID_BIG_DOOR_OPEN) // Door Effect
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
    else if(A == EFFID_FODDER_S3_DIE || A == EFFID_RED_FODDER_DIE || A == EFFID_GRN_SHELL_S3_DIE || A == EFFID_RED_SHELL_S3_DIE || A == EFFID_GLASS_SHELL_DIE
        || A == EFFID_UNDER_FODDER_DIE || A == EFFID_GRN_BOOT_DIE || A == EFFID_RED_BOOT_DIE || A == EFFID_BLU_BOOT_DIE
        || A == EFFID_SPIKY_S3_DIE || A == EFFID_SPIKY_S4_DIE || A == EFFID_SPIT_BOSS_BALL_DIE || A == EFFID_SPIT_BOSS_DIE
        || A == EFFID_SPIKY_BALL_S3_DIE || A == EFFID_SPIKY_THROWER_DIE || A == EFFID_ITEM_THROWER_DIE || A == EFFID_CRAB_DIE
        || A == EFFID_FLY_DIE || A == EFFID_EXT_TURTLE_DIE || A == EFFID_YELSWITCH_FODDER_DIE || A == EFFID_BLUSWITCH_FODDER_DIE
        || A == EFFID_GRNSWITCH_FODDER_DIE || A == EFFID_REDSWITCH_FODDER_DIE || A == EFFID_BIG_FODDER_DIE || A == EFFID_BIG_SHELL_DIE
        || A == EFFID_FODDER_S1_DIE || A == EFFID_SHELL_S4_DIE  /* || A == EFFID_RED_FODDER_SQUISH -- mistake, repeated case from above */
        || A == EFFID_GRN_SHELL_S1_DIE || A == EFFID_RED_SHELL_S1_DIE || A == EFFID_WALL_SPARK_DIE || A == EFFID_SQUID_S3_DIE
        || A == EFFID_SQUID_S1_DIE || A == EFFID_FODDER_S5_DIE || A == EFFID_VINE_BUG_DIE) // Flying goomba / turtle shell / hard thing shell /*A == 9 || - duplicated*/
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;

        if(num_t::fEqual_d(Location.SpeedY, 0.123_n))
        {
            ne.Location.SpeedY = 1;
            ne.Location.SpeedX = 0;
        }
        else if(Location.SpeedY != -5.1_n)
        {
            ne.Location.SpeedY = -11;
            ne.Location.SpeedX = Location.SpeedX;
        }
        else
        {
            ne.Location.SpeedY = -5.1_n;
            ne.Location.SpeedX = Location.SpeedX * 0.6_r;
        }

        ne.Frame = 0;
        ne.Life = 150;
        ne.Type = A;

        if(ne.Type == EFFID_SPIT_BOSS_DIE && Direction == -1)
            ne.Frame = 1;
        if((ne.Type == EFFID_SPIKY_S3_DIE || ne.Type == EFFID_SPIKY_S4_DIE) && Direction == 1)
            ne.Frame = 2;
        if(ne.Type == EFFID_EXT_TURTLE_DIE && Direction == 1)
            ne.Frame = 1;
    }
    else if(A == EFFID_COIN_COLLECT) // Coins
    {
        for(B = 1; B <= 4; B++)
        {
            numEffects++;
            auto &ne = Effect[numEffects];
            ne.Shadow = Shadow;
            ne.Location.Width = EffectWidth[A];
            ne.Location.Height = EffectHeight[A];
            ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
            ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
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
    else if(A == EFFID_SMOKE_S3_CENTER || A == EFFID_SMOKE_S3 || A == EFFID_WHIP || A == EFFID_SKID_DUST
        || A == EFFID_WHACK || A == EFFID_SMOKE_S4 || A == EFFID_STOMP_INIT || A == EFFID_SMOKE_S2) // Puff of smoke
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;

        if(A == EFFID_SMOKE_S3_CENTER || A == EFFID_SMOKE_S4)
        {
            if(A == EFFID_SMOKE_S3_CENTER)
                A = EFFID_SMOKE_S3;

            ne.Location.X += (Location.Width - EffectWidth[A]) / 2;
            ne.Location.Y += (Location.Height - EffectHeight[A]) / 2;
        }

        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;

        ne.Frame = 0;
        ne.Life = 12;
        ne.Type = A;

        if(ne.Type == EFFID_SMOKE_S2)
            ne.Life = 24;

        if(A == EFFID_STOMP_INIT || A == EFFID_WHIP || A == EFFID_WHACK)
        {
            ne.Location.X += dRand() * 16 - 8;
            ne.Location.Y += dRand() * 16 - 8;
        }
        else if(A == EFFID_SKID_DUST)
        {
            ne.Location.X += dRand() * 4 - 2;
            ne.Location.Y += dRand() * 4 - 2;

            // moved from UpdateEffects
            ne.Location.SpeedY = -0.1_n;
        }
    }
    else if(A == EFFID_BUBBLE_POP) // bubble pop
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 6;
        ne.Type = A;
    }
    else if(A == EFFID_SMOKE_S5) // Zelda Style Smoke
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 48;
        ne.Location.Height = 48;
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + (Location.Height - ne.Location.Height) / 2;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == EFFID_COIN_BLOCK_S3) // Coin hit out of block
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y - 32;
        ne.Location.SpeedY = -8;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 46;
        ne.Type = A;
    }
    else if(A == EFFID_BIG_FIREBALL_TAIL) // Big Fireball Tail
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 8;
        ne.Location.Height = 8;
        ne.Location.X = Location.X + 4 + (dRand() * 12);
        ne.Location.Y = Location.Y + 40;
        // .Location.SpeedY = -8
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 12;
        ne.Type = A;
    }
    else if(A == EFFID_BOSS_CASE_BREAK) // Glass Shatter
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
    else if(A == EFFID_BOSS_FRAGILE_DIE) // Mother Brain
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
    else if(A == EFFID_PLR_FIREBALL_TRAIL || A == EFFID_PLR_ICEBALL_TRAIL) // Small Fireball Tail
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - EffectWidth[A]) / 2 + dRand() * 4 - 2;
        ne.Location.Y = Location.Y + (Location.Height - EffectHeight[A]) / 2 + dRand() * 4 - 2;
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
    else if(A == EFFID_SPARKLE) // Twinkle
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 16;
        ne.Location.Height = 16;
        ne.Location.X = Location.X + Location.Width / 2 - 4 + dRand() * 4 - 2;
        ne.Location.Y = Location.Y + Location.Height / 2 - 4 + dRand() * 4 - 2;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 60;
        ne.Type = A;
    }
    else if(A == EFFID_LAVA_SPLASH) // Lava Splash
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[EFFID_LAVA_SPLASH];
        ne.Location.Height = EffectHeight[EFFID_LAVA_SPLASH];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + 24;
        ne.Location.SpeedY = -8;
        ne.Location.SpeedX = 0;
        ne.Frame = 0;
        ne.Life = 100;
        ne.Type = A;
    }
    else if(A == EFFID_MINIBOSS_DIE) // Dead Big Koopa
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + 22;
        ne.Location.SpeedY = 0;
        ne.Location.SpeedX = 0;
        ne.NewNpc = 0; // NewNpc -- now gets set at callsite
        ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_BULLET_DIE || A == EFFID_SPIT_GUY_BALL_DIE) // Dead Bullet Bill
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
        if(A == EFFID_SPIT_GUY_BALL_DIE)
            ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_HIT_TURTLE_S4_DIE) // Flying Beach Koopa
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
        // NOTE: Frame set at callsite
    }
    else if(A == EFFID_POWER_S3_DIE) // Dead toad
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
        ne.Location.X += ne.Location.Width / 2 - 16;
        ne.Location.Y += ne.Location.Height / 2 - 16;
        ne.Location.Width = 32;
        ne.Location.Height = 32;
        ne.Frame = 0;
        ne.Location.SpeedY = -8;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_BIG_BULLET_DIE) // Dead Giant Bullet Bill
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
        if(A == EFFID_POWER_S3_DIE)
            ne.Location.SpeedY = -8;
#endif

        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_BOMB_S2_EXPLODE) // Bomb
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = 64;
        ne.Location.Height = 64;
        ne.Location.X = Location.X + Location.Width / 2 - 32;
        ne.Location.Y = Location.Y + Location.Height / 2 - 32;
        ne.Location.SpeedX = 0;
        ne.Location.SpeedY = 0;
        ne.Life = 60;
        ne.Type = A;
    }
    else if(A == EFFID_STACKER_DIE) // pokey
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
    else if(A == EFFID_RED_GUY_DIE || A == EFFID_BLU_GUY_DIE || A == EFFID_JUMPER_S3_DIE || A == EFFID_RED_FISH_S1_DIE
            || (A >= EFFID_BIRD_DIE && A <= EFFID_GRY_SPIT_GUY_DIE) || A == EFFID_CARRY_BUDDY_DIE || A == EFFID_BRUTE_SQUISHED_DIE
            || A == EFFID_BRUTE_DIE || A == EFFID_BIG_GUY_DIE || A == EFFID_CARRY_FODDER_DIE || A == EFFID_SKELETON_DIE
            || A == EFFID_GRN_FISH_S3_DIE || A == EFFID_YEL_FISH_S4_DIE || A == EFFID_RED_FISH_S3_DIE || A == EFFID_GRN_FISH_S4_DIE
            || A == EFFID_GRN_FISH_S1_DIE || A == EFFID_BONE_FISH_DIE || A == EFFID_WALK_PLANT_DIE) // Shy guy / Star Thing /Red Jumping Fish
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Frame = 0;

        if(A != EFFID_RED_FISH_S1_DIE && A != EFFID_GRN_FISH_S3_DIE && A != EFFID_RED_FISH_S3_DIE)
            ne.Location.SpeedY = -11;
        else
            ne.Location.SpeedY = Location.SpeedY;

        ne.Location.SpeedX = Location.SpeedX;
        if(Location.SpeedY == 0.123_n)
        {
            ne.Location.SpeedY = 1;
            ne.Location.SpeedX = 0;
        }

        if(A == EFFID_BRUTE_SQUISHED_DIE || A == EFFID_BRUTE_DIE || A == EFFID_BIG_GUY_DIE || A == EFFID_CARRY_FODDER_DIE
            || A == EFFID_SKELETON_DIE || A == EFFID_GRN_FISH_S3_DIE || A == EFFID_RED_FISH_S3_DIE || A == EFFID_GRN_FISH_S4_DIE
            || A == EFFID_GRN_FISH_S1_DIE || A == EFFID_BONE_FISH_DIE || A == EFFID_YEL_FISH_S4_DIE || A == EFFID_WALK_PLANT_DIE)
        {
            ne.Frame = 0;
        }
        else
            ne.Frame = 4;

        if(int(Direction) != -1)
            ne.Frame += 2;

        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_STONE_S3_DIE || A == EFFID_BIG_GHOST_DIE || A == EFFID_GHOST_S4_DIE || A == EFFID_GHOST_FAST_DIE
        || A == EFFID_GHOST_S3_DIE || A == EFFID_STONE_S4_DIE || A == EFFID_SAW_DIE) // Boo / thwomps
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

        if(A == EFFID_BIG_GHOST_DIE)
        {
            ne.Location.set_width_center(EffectWidth[A]);
            ne.Location.set_height_center(EffectHeight[A]);
        }

        if(ne.Location.SpeedX != 0 && ne.Location.SpeedX > -2 && ne.Location.SpeedX < 2)
            ne.Location.SpeedX = 2 * -Direction;
        if(Direction == -1)
            ne.Frame = 0;
        else
            ne.Frame = 2;
        if(A == EFFID_STONE_S3_DIE || A == EFFID_STONE_S4_DIE || A == EFFID_SAW_DIE)
            ne.Frame = 0;
        ne.Life = 120;
        ne.Type = A;
    }
    else if(A == EFFID_HEAVY_THROWER_DIE || A == EFFID_JUMPER_S4_DIE || A == EFFID_VILLAIN_S3_DIE || A == EFFID_WALK_BOMB_S3_DIE
        || A == EFFID_CHASER_DIE || A == EFFID_VILLAIN_S1_DIE || A == EFFID_SICK_BOSS_DIE || A == EFFID_BOMBER_BOSS_DIE
        || A == EFFID_BAT_DIE) // Hammer Bro
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;

        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
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
    else if(A == EFFID_FIRE_BOSS_DIE) // ludwig dead
    {
        numEffects++;
        auto &ne = Effect[numEffects];
        ne.Shadow = Shadow;
        ne.Location.Width = EffectWidth[A];
        ne.Location.Height = EffectHeight[A];
        // ne.Location.X = Location.X + (Location.Width - ne.Location.Width) / 2;
        // ne.Location.Y = Location.Y + Location.Height - ne.Location.Height;
        ne.Location.X = Location.X;
        ne.Location.Y = Location.Y;
        ne.Location.SpeedY = -14;
        ne.Location.SpeedX = 3 * -Direction;
        ne.Life = 200;
        ne.Type = A;
        PlaySoundSpatial(SFX_FireBossKilled, Location);
    }

    return true;
}

void NewEffect_IceSparkle(const NPC_t& n, Location_t& tempLocation)
{
    tempLocation.Height = EffectHeight[EFFID_SPARKLE];
    tempLocation.Width = EffectWidth[EFFID_SPARKLE];
    tempLocation.SpeedX = 0;
    tempLocation.SpeedY = 0;
    tempLocation.X = n.Location.X - tempLocation.Width / 2 + dRand().times(n.Location.Width) - 4;
    tempLocation.Y = n.Location.Y - tempLocation.Height / 2 + dRand().times(n.Location.Height) - 4;
    NewEffect(EFFID_SPARKLE, tempLocation, 1, n.Shadow);
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
    e.Type = EFFID(0);
    numEffects -= 1;
}
