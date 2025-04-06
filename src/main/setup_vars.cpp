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

#include "../globals.h"
#include "../game_main.h"
#include "../custom.h"
#include "npc_traits.h"
#include "npc_id.h"

void SetupVars()
{
    //int A;
    SetupPlayerFrames();
    for(int A = 1; A <= maxEffectType; A++)
    {
        EffectWidth[A] = 32;
        EffectHeight[A] = 32;
    }
    // now part of the default NPCTraits_t initializer'
    // for(int A = 1; A <= maxNPCType; ++A)
    //     NPCTraits[A].Speedvar = 1;
    for(int A = 174; A <= 186; ++A)
        BackgroundFence[A] = true;

    GFXLevelBig[21] = true;
    GFXLevelBig[22] = true;
    GFXLevelBig[23] = true;
    GFXLevelBig[24] = true;
    GFXLevelBig[28] = true;

    EffectWidth[145] = 32;
    EffectHeight[145] = 60;

    EffectWidth[71] = 16;
    EffectHeight[71] = 16;
    EffectWidth[148] = 16;
    EffectHeight[148] = 16;


    EffectWidth[111] = 16;
    EffectHeight[111] = 16;
    EffectHeight[121] = 48;

    EffectWidth[144] = 54;
    EffectHeight[144] = 42;


    EffectWidth[143] = 64;
    EffectHeight[143] = 64;

    EffectWidth[120] = 48;
    EffectHeight[121] = 48;


    EffectWidth[138] = 48;
    EffectHeight[138] = 64;


    EffectWidth[125] = 80;
    EffectHeight[125] = 34;

    EffectWidth[134] = 32;
    EffectHeight[134] = 58;

    EffectWidth[130] = 30;
    EffectHeight[130] = 34;

    EffectWidth[132] = 32;
    EffectHeight[132] = 32;
    EffectWidth[133] = 10;
    EffectHeight[133] = 8;


    EffectWidth[1] = 16;
    EffectHeight[1] = 16;

    EffectWidth[135] = 16;
    EffectHeight[135] = 16;

    EffectWidth[104] = 32;
    EffectHeight[104] = 32;
    EffectWidth[105] = 64;
    EffectHeight[105] = 72;

    EffectWidth[129] = 30;
    EffectHeight[129] = 42;

    EffectWidth[106] = 80;
    EffectHeight[106] = 94;

    EffectWidth[108] = 64;
    EffectHeight[108] = 64;

    EffectWidth[112] = 96;
    EffectHeight[112] = 106;

    EffectWidth[4] = 32;
    EffectHeight[4] = 32;
    EffectWidth[8] = 32;
    EffectHeight[8] = 32;
    EffectWidth[9] = 32;
    EffectHeight[9] = 32;
    EffectWidth[10] = 32;
    EffectHeight[10] = 32;
    EffectWidth[11] = 32;
    EffectHeight[11] = 32;
    EffectWidth[12] = 8;
    EffectHeight[12] = 8;
    EffectWidth[13] = 84;
    EffectHeight[13] = 26;
    EffectWidth[14] = 62;
    EffectHeight[14] = 32;
    EffectWidth[15] = 32;
    EffectHeight[15] = 28;
    EffectWidth[16] = 128;
    EffectHeight[16] = 128;
    EffectWidth[17] = 32;
    EffectHeight[17] = 32;
    EffectWidth[18] = 32;
    EffectHeight[18] = 32;
    EffectWidth[19] = 32;
    EffectHeight[19] = 32;
    EffectWidth[20] = 32;
    EffectHeight[20] = 32;
    EffectWidth[21] = 16;
    EffectHeight[21] = 16;
    EffectWidth[22] = 32;
    EffectHeight[22] = 32;
    EffectWidth[23] = 32;
    EffectHeight[23] = 32;
    EffectWidth[24] = 32;
    EffectHeight[24] = 32;
    EffectWidth[25] = 32;
    EffectHeight[25] = 48;
    EffectWidth[26] = 32;
    EffectHeight[26] = 32;
    EffectWidth[29] = 40;
    EffectHeight[29] = 64;
    EffectWidth[30] = 16;
    EffectHeight[30] = 16;
    EffectWidth[31] = 32;
    EffectHeight[31] = 32;
    EffectWidth[32] = 32;
    EffectHeight[32] = 62;
    EffectWidth[33] = 32;
    EffectHeight[33] = 32;
    EffectWidth[34] = 32;
    EffectHeight[34] = 32;
    EffectWidth[35] = 32;
    EffectHeight[35] = 32;
    EffectWidth[36] = 32;
    EffectHeight[36] = 32;
    EffectWidth[37] = 32;
    EffectHeight[37] = 32;
    EffectWidth[38] = 32;
    EffectHeight[38] = 32;
    EffectWidth[45] = 48;
    EffectHeight[45] = 46;
    EffectWidth[46] = 48;
    EffectHeight[46] = 46;
    EffectWidth[47] = 44;
    EffectHeight[47] = 44;
    EffectWidth[48] = 32;
    EffectHeight[48] = 32;
    EffectWidth[50] = 64;
    EffectHeight[50] = 80;
    EffectWidth[51] = 16;
    EffectHeight[51] = 16;
    EffectWidth[54] = 32;
    EffectHeight[54] = 64;
    EffectWidth[55] = 32;
    EffectHeight[55] = 64;
    EffectWidth[56] = 32;
    EffectHeight[56] = 32;
    EffectWidth[57] = 16;
    EffectHeight[57] = 16;
    EffectWidth[58] = 32;
    EffectHeight[58] = 32;
    EffectWidth[59] = 32;
    EffectHeight[59] = 64;
    EffectWidth[60] = 32;
    EffectHeight[60] = 32;
    EffectWidth[61] = 32;
    EffectHeight[61] = 32;
    EffectWidth[62] = 32;
    EffectHeight[62] = 32;
    EffectWidth[63] = 48;
    EffectHeight[63] = 48;
    EffectWidth[64] = 32;
    EffectHeight[64] = 32;
    EffectWidth[65] = 32;
    EffectHeight[65] = 32;
    EffectWidth[66] = 32;
    EffectHeight[66] = 32;
    EffectWidth[67] = 32;
    EffectHeight[67] = 32;
    EffectWidth[68] = 16;
    EffectHeight[68] = 16;
    EffectWidth[69] = 64;
    EffectHeight[69] = 64;
    EffectWidth[70] = 16;
    EffectHeight[70] = 16;
    EffectWidth[71] = 16;
    EffectHeight[71] = 16;
    EffectWidth[72] = 32;
    EffectHeight[72] = 28;
    EffectWidth[73] = 32;
    EffectHeight[73] = 32;
    EffectWidth[74] = 8;
    EffectHeight[74] = 10;
    EffectWidth[75] = 32;
    EffectHeight[75] = 32;
    EffectWidth[76] = 14;
    EffectHeight[76] = 14;
    EffectWidth[77] = 8;
    EffectHeight[77] = 8;
    EffectWidth[139] = 8;
    EffectHeight[139] = 8;
    EffectWidth[78] = 10;
    EffectHeight[78] = 10;
    EffectWidth[79] = 50;
    EffectHeight[79] = 24;
    EffectWidth[80] = 16;
    EffectHeight[80] = 16;
    EffectWidth[81] = 32;
    EffectHeight[81] = 32;
    EffectWidth[82] = 32;
    EffectHeight[82] = 32;
    EffectWidth[83] = 32;
    EffectHeight[83] = 42;
    EffectWidth[84] = 32;
    EffectHeight[84] = 32;
    EffectWidth[85] = 32;
    EffectHeight[85] = 32;
    EffectWidth[86] = 40;
    EffectHeight[86] = 64;
    EffectWidth[87] = 64;
    EffectHeight[87] = 64;
    EffectWidth[88] = 32;
    EffectHeight[88] = 32;
    EffectWidth[89] = 26;
    EffectHeight[89] = 46;
    EffectWidth[90] = 48;
    EffectHeight[90] = 96;
    EffectWidth[91] = 140;
    EffectHeight[91] = 128;
    EffectWidth[92] = 32;
    EffectHeight[92] = 32;
    EffectWidth[93] = 32;
    EffectHeight[93] = 32;
    EffectWidth[94] = 32;
    EffectHeight[94] = 32;
    EffectWidth[97] = 48;
    EffectHeight[97] = 64;
    EffectWidth[98] = 64;
    EffectHeight[98] = 64;
    EffectWidth[99] = 48;
    EffectHeight[99] = 64;
    EffectWidth[100] = 16;
    EffectHeight[100] = 16;

    EffectWidth[113] = 8;
    EffectHeight[113] = 8;

    EffectWidth[140] = 32;
    EffectHeight[140] = 40;


    EffectWidth[114] = 32;
    EffectHeight[114] = 32;

    EffectWidth[103] = 120;
    EffectHeight[103] = 96;

    EffectWidth[123] = 32;
    EffectHeight[123] = 32;
    EffectWidth[124] = 32;
    EffectHeight[124] = 32;
    EffectHeight[95] = 28;
    EffectHeight[96] = 28;

    EffectHeight[46] = 46;
    EffectWidth[46] = 48;
    EffectHeight[47] = 44;
    EffectWidth[47] = 44;

    // NEW: reset all variables to their original values (it will be nice to convert all NPC traits to a single struct)
    NPCTraits.fill(NPCTraits_t());

    // SMBX64 logic:
    // now included as part of default NPCTraits_t initializer
    // for(int A = 1; A <= maxNPCType; ++A)
    // {
    //     NPCTraits[A].Score = 2;
    //     NPCTraits[A].TWidth = 32;
    //     NPCTraits[A].THeight = 32;
    // }

    NPCTraits[NPCID_CHAR3_HEAVY].TWidth = 22;
    NPCTraits[NPCID_CHAR3_HEAVY].THeight = 18;
    NPCTraits[NPCID_CHAR3_HEAVY].WidthGFX = 42;
    NPCTraits[NPCID_CHAR3_HEAVY].HeightGFX = 34;
    NPCTraits[NPCID_CHAR3_HEAVY].WontHurt = true;
    NPCTraits[NPCID_CHAR3_HEAVY].JumpHurt = true;
    NPCTraits[NPCID_CHAR3_HEAVY].NoYoshi = true;



    NPCTraits[NPCID_LONG_PLANT_UP].NoYoshi = true;
    NPCTraits[NPCID_LONG_PLANT_DOWN].NoYoshi = true;
    NPCTraits[NPCID_TNT].NoYoshi = true;


    NPCTraits[NPCID_BOSS_CASE].Score = 0;
    NPCTraits[NPCID_MINIBOSS].Score = 7;
    NPCTraits[NPCID_SPIT_BOSS].Score = 7;
    NPCTraits[NPCID_VILLAIN_S3].Score = 9;
    NPCTraits[NPCID_VILLAIN_S1].Score = 9;
    NPCTraits[NPCID_SICK_BOSS].Score = 9;
    NPCTraits[NPCID_BOSS_FRAGILE].Score = 9;
    NPCTraits[NPCID_HEAVY_THROWER].Score = 5;
    NPCTraits[NPCID_SPIKY_THROWER].Score = 6;
    NPCTraits[NPCID_ITEM_THROWER].Score = 6;
    NPCTraits[NPCID_LONG_PLANT_UP].Score = 5;
    NPCTraits[NPCID_LONG_PLANT_DOWN].Score = 5;
    NPCTraits[NPCID_BOMBER_BOSS].Score = 8;
    NPCTraits[NPCID_MAGIC_BOSS].Score = 8;
    NPCTraits[NPCID_MAGIC_BOSS_SHELL].Score = 8;
    NPCTraits[NPCID_FIRE_BOSS].Score = 8;
    NPCTraits[NPCID_FIRE_BOSS_SHELL].Score = 8;
    NPCTraits[NPCID_GEM_1].FrameOffsetY = 2;
    NPCTraits[NPCID_GEM_5].FrameOffsetY = 2;
    NPCTraits[NPCID_GEM_20].FrameOffsetY = 2;
    NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].FrameOffsetY = 2;
    NPCTraits[NPCID_JUMPER_S4].FrameOffsetY = 2;

    NPCTraits[NPCID_DOOR_MAKER].WontHurt = true;
    NPCTraits[NPCID_DOOR_MAKER].JumpHurt = true;

    NPCTraits[NPCID_RANDOM_POWER].WontHurt = true;
    NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].WontHurt = true;
    NPCTraits[NPCID_HOMING_BALL].Foreground = true;
    NPCTraits[NPCID_RED_FISH_S3].Foreground = true;

    NPCTraits[NPCID_RED_FISH_S1].IsFish = true;
    NPCTraits[NPCID_GRN_FISH_S3].IsFish = true;
    NPCTraits[NPCID_RED_FISH_S3].IsFish = true;
    NPCTraits[NPCID_GOGGLE_FISH].IsFish = true;
    NPCTraits[NPCID_GRN_FISH_S1].IsFish = true;
    NPCTraits[NPCID_BONE_FISH].IsFish = true;
    NPCTraits[NPCID_FISH_S4].IsFish = true;


    NPCTraits[NPCID_MAGIC_DOOR].NoClipping = true;
    NPCTraits[NPCID_MAGIC_DOOR].WontHurt = true;
    NPCTraits[NPCID_MAGIC_DOOR].JumpHurt = true;
    NPCTraits[NPCID_MAGIC_DOOR].TWidth = 32;
    NPCTraits[NPCID_MAGIC_DOOR].THeight = 32;
    NPCTraits[NPCID_MAGIC_DOOR].WidthGFX = 32;
    NPCTraits[NPCID_MAGIC_DOOR].HeightGFX = 64;


    NPCTraits[NPCID_ITEM_BUBBLE].TWidth = 48;
    NPCTraits[NPCID_ITEM_BUBBLE].THeight = 48;
    NPCTraits[NPCID_ITEM_BUBBLE].WidthGFX = 64;
    NPCTraits[NPCID_ITEM_BUBBLE].HeightGFX = 64;
    NPCTraits[NPCID_ITEM_BUBBLE].FrameOffsetY = 8;
    NPCTraits[NPCID_ITEM_BUBBLE].WontHurt = true;
    NPCTraits[NPCID_ITEM_BUBBLE].JumpHurt = true;

    NPCTraits[NPCID_COCKPIT].WidthGFX = 32;
    NPCTraits[NPCID_COCKPIT].HeightGFX = 64;
    NPCTraits[NPCID_COCKPIT].Foreground = true;
    NPCTraits[NPCID_COCKPIT].CanWalkOn = true;
    NPCTraits[NPCID_COCKPIT].IsABlock = true;
    NPCTraits[NPCID_COCKPIT].WontHurt = true;
    NPCTraits[NPCID_COCKPIT].NoClipping = true;


    NPCTraits[NPCID_ICE_CUBE].CanWalkOn = true;
    NPCTraits[NPCID_ICE_CUBE].WontHurt = true;
    NPCTraits[NPCID_ICE_CUBE].MovesPlayer = true;
    NPCTraits[NPCID_ICE_CUBE].IsGrabbable = true;
    NPCTraits[NPCID_ICE_CUBE].GrabFromTop = true;
    NPCTraits[NPCID_ICE_CUBE].IsABlock = true;

    NPCTraits[NPCID_BOMBER_BOSS].CanWalkOn = true;
    NPCTraits[NPCID_BOMBER_BOSS].TWidth = 40;
    NPCTraits[NPCID_BOMBER_BOSS].THeight = 56;
    NPCTraits[NPCID_BOMBER_BOSS].WidthGFX = 48;
    NPCTraits[NPCID_BOMBER_BOSS].HeightGFX = 64;



    NPCTraits[NPCID_FLY_POWER].IsABonus = true;
    NPCTraits[NPCID_ICE_BLOCK].IsAShell = true;
    NPCTraits[NPCID_ICE_BLOCK].WontHurt = true;
    NPCTraits[NPCID_ICE_BLOCK].CanWalkOn = true;
    NPCTraits[NPCID_PLR_FIREBALL].JumpHurt = true;
    NPCTraits[NPCID_VILLAIN_S3].JumpHurt = true;
    NPCTraits[NPCID_FIRE_CHAIN].JumpHurt = true;



    NPCTraits[NPCID_ICE_CUBE].NoYoshi = true;
    NPCTraits[NPCID_PLR_ICEBALL].NoYoshi = true;
    NPCTraits[NPCID_FIRE_CHAIN].NoYoshi = true;
    NPCTraits[NPCID_LOCK_DOOR].NoYoshi = true;

    NPCTraits[NPCID_JUMP_PLANT].NoClipping = true;

    NPCTraits[NPCID_PLANT_FIRE].JumpHurt = true;
    NPCTraits[NPCID_PLANT_FIRE].NoClipping = true;
    NPCTraits[NPCID_PLANT_FIRE].NoYoshi = true;
    NPCTraits[NPCID_PLANT_FIRE].TWidth = 16;
    NPCTraits[NPCID_PLANT_FIRE].THeight = 16;

    NPCTraits[NPCID_GEM_1].TWidth = 18; // TLOZ Rupee
    NPCTraits[NPCID_GEM_1].THeight = 32;
    NPCTraits[NPCID_GEM_1].IsABonus = true;
    NPCTraits[NPCID_GEM_1].IsACoin = true;

    NPCTraits[NPCID_GEM_5].TWidth = 18; // TLOZ Rupee
    NPCTraits[NPCID_GEM_5].THeight = 32;
    NPCTraits[NPCID_GEM_5].IsABonus = true;
    NPCTraits[NPCID_GEM_5].IsACoin = true;

    NPCTraits[NPCID_GEM_20].TWidth = 18; // TLOZ Rupee
    NPCTraits[NPCID_GEM_20].THeight = 32;
    NPCTraits[NPCID_GEM_20].IsABonus = true;
    NPCTraits[NPCID_GEM_20].IsACoin = true;

    NPCTraits[NPCID_POWER_S5].TWidth = 32; // TLOZ Heart
    NPCTraits[NPCID_POWER_S5].THeight = 32;
    NPCTraits[NPCID_POWER_S5].IsABonus = true;

    // vines
    for(int A = NPCID_GRN_VINE_S3; A <= NPCID_GRN_VINE_S4; ++A)
    {
        NPCTraits[A].IsAVine = true;
        NPCTraits[A].WontHurt = true;
        NPCTraits[A].NoClipping = true;
        NPCTraits[A].JumpHurt = true;
        NPCTraits[A].NoYoshi = true;
        NPCTraits[A].TWidth = 16;
        NPCTraits[A].WidthGFX = 32;
        NPCTraits[A].HeightGFX = 32;
    }
    NPCTraits[NPCID_GRN_VINE_TOP_S1].THeight = 24;
    NPCTraits[NPCID_GRN_VINE_TOP_S1].HeightGFX = 24;
    NPCTraits[NPCID_SQUID_S3].JumpHurt = true;
    NPCTraits[NPCID_RED_VINE_TOP_S3].WontHurt = true;
    NPCTraits[NPCID_RED_VINE_TOP_S3].JumpHurt = true;
    NPCTraits[NPCID_RED_VINE_TOP_S3].NoYoshi = true;
    NPCTraits[NPCID_GRN_VINE_TOP_S3].WontHurt = true;
    NPCTraits[NPCID_GRN_VINE_TOP_S3].JumpHurt = true;
    NPCTraits[NPCID_GRN_VINE_TOP_S3].NoYoshi = true;
    NPCTraits[NPCID_GRN_VINE_TOP_S4].WontHurt = true;
    NPCTraits[NPCID_GRN_VINE_TOP_S4].JumpHurt = true;
    NPCTraits[NPCID_GRN_VINE_TOP_S4].NoYoshi = true;

    NPCTraits[NPCID_HOMING_BALL_GEN].NoYoshi = true;

    NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].IsGrabbable = true;
    NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].GrabFromTop = true;

    NPCTraits[NPCID_TIMER_S2].TWidth = 32;
    NPCTraits[NPCID_TIMER_S2].THeight = 32;
    NPCTraits[NPCID_TIMER_S2].IsABonus = true;//32;

    NPCTraits[NPCID_TIMER_S3].TWidth = 32;
    NPCTraits[NPCID_TIMER_S3].THeight = 32;
    NPCTraits[NPCID_TIMER_S3].IsABonus = true;//32;

    NPCTraits[NPCID_VILLAIN_S1].TWidth = 64;
    NPCTraits[NPCID_VILLAIN_S1].THeight = 72;
    NPCTraits[NPCID_VILLAIN_S1].FrameOffsetY = 2;
    NPCTraits[NPCID_VILLAIN_S1].JumpHurt = true;
    NPCTraits[NPCID_HOMING_BALL].JumpHurt = true;
    NPCTraits[NPCID_FIRE_DISK].NoClipping = true;
    NPCTraits[NPCID_FIRE_DISK].NoYoshi = true;
    NPCTraits[NPCID_FIRE_DISK].JumpHurt = true;
    NPCTraits[NPCID_FIRE_CHAIN].TWidth = 16;
    NPCTraits[NPCID_FIRE_CHAIN].THeight = 16;
    NPCTraits[NPCID_FIRE_CHAIN].NoClipping = true;

    NPCTraits[NPCID_POISON].TWidth = 32; // Poison Mushroom
    NPCTraits[NPCID_POISON].THeight = 32;
    NPCTraits[NPCID_POISON].FrameOffsetY = 2;

    NPCTraits[NPCID_FODDER_S5].TWidth = 32; // SML2 Goomba
    NPCTraits[NPCID_FODDER_S5].THeight = 32;
    NPCTraits[NPCID_FODDER_S5].FrameOffsetY = 2;
    // NPCDefaultMovement(NPCID_FODDER_S5) = true;

    NPCTraits[NPCID_FLY_FODDER_S5].TWidth = 32; // SML2 Flying Goomba
    NPCTraits[NPCID_FLY_FODDER_S5].THeight = 32;
    NPCTraits[NPCID_FLY_FODDER_S5].FrameOffsetY = 2;
    NPCTraits[NPCID_FLY_FODDER_S5].WidthGFX = 56;
    NPCTraits[NPCID_FLY_FODDER_S5].HeightGFX = 36;

    NPCTraits[NPCID_FLY_FODDER_S3].TWidth = 32; // SMB3 Flying Goomba
    NPCTraits[NPCID_FLY_FODDER_S3].THeight = 32;
    NPCTraits[NPCID_FLY_FODDER_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_FLY_FODDER_S3].WidthGFX = 40;
    NPCTraits[NPCID_FLY_FODDER_S3].HeightGFX = 48;

    NPCTraits[NPCID_LOCK_DOOR].TWidth = 20;
    NPCTraits[NPCID_LOCK_DOOR].THeight = 96;
    NPCTraits[NPCID_LOCK_DOOR].WontHurt = true;
    NPCTraits[NPCID_LOCK_DOOR].IsABlock = true;
    NPCTraits[NPCID_LOCK_DOOR].CanWalkOn = true;
    NPCTraits[NPCID_LOCK_DOOR].MovesPlayer = true;

    // NPCIsAParaTroopa(NPCID_FLY_FODDER_S3) = true;
    // NPCIsAParaTroopa(NPCID_FLY_FODDER_S5) = true;

    NPCTraits[NPCID_FODDER_S3].TWidth = 32; // Goomba
    NPCTraits[NPCID_FODDER_S3].THeight = 32;
    NPCTraits[NPCID_FODDER_S3].FrameOffsetY = 2;

    NPCTraits[NPCID_FODDER_S3].TWidth = 32; // Goomba
    NPCTraits[NPCID_FODDER_S3].THeight = 32;
    NPCTraits[NPCID_FODDER_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_FODDER].TWidth = 32; // Red goomba
    NPCTraits[NPCID_RED_FODDER].THeight = 32;
    NPCTraits[NPCID_RED_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_FLY_FODDER].TWidth = 32; // Flying goomba
    NPCTraits[NPCID_RED_FLY_FODDER].THeight = 32;
    NPCTraits[NPCID_RED_FLY_FODDER].WidthGFX = 40;
    NPCTraits[NPCID_RED_FLY_FODDER].HeightGFX = 48;
    NPCTraits[NPCID_RED_FLY_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_GRN_TURTLE_S3].TWidth = 32; // Green koopa
    NPCTraits[NPCID_GRN_TURTLE_S3].THeight = 32;
    NPCTraits[NPCID_GRN_TURTLE_S3].WidthGFX = 32;
    NPCTraits[NPCID_GRN_TURTLE_S3].HeightGFX = 54;
    NPCTraits[NPCID_GRN_TURTLE_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_GRN_SHELL_S3].TWidth = 32; // Green shell
    NPCTraits[NPCID_GRN_SHELL_S3].THeight = 32;
    NPCTraits[NPCID_GRN_SHELL_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_TURTLE_S3].TWidth = 32; // Red koopa
    NPCTraits[NPCID_RED_TURTLE_S3].THeight = 32;
    NPCTraits[NPCID_RED_TURTLE_S3].WidthGFX = 32;
    NPCTraits[NPCID_RED_TURTLE_S3].HeightGFX = 54;
    NPCTraits[NPCID_RED_TURTLE_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_SHELL_S3].TWidth = 32; // Red shell
    NPCTraits[NPCID_RED_SHELL_S3].THeight = 32;
    NPCTraits[NPCID_RED_SHELL_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_PLANT_S3].TWidth = 32; // Plant
    NPCTraits[NPCID_PLANT_S3].THeight = 48;
    NPCTraits[NPCID_PLANT_S3].WidthGFX = 32;
    NPCTraits[NPCID_PLANT_S3].HeightGFX = 48;
    NPCTraits[NPCID_FIRE_PLANT].TWidth = 32; // SMB 3 Fire Plant
    NPCTraits[NPCID_FIRE_PLANT].THeight = 64;
    NPCTraits[NPCID_FIRE_PLANT].WidthGFX = 32;
    NPCTraits[NPCID_FIRE_PLANT].HeightGFX = 64;
    NPCTraits[NPCID_PLANT_S3].FrameOffsetY = 1;
    NPCTraits[NPCID_POWER_S3].TWidth = 32; // Mushroom
    NPCTraits[NPCID_POWER_S3].THeight = 32;
    NPCTraits[NPCID_POWER_S3].FrameOffsetY =2;

    // Reversed incompatible powerup score customizability implementation at v1.3.6.1.
    // These scores are now found in loadNpcSetupFixes();

    NPCTraits[NPCID_SWAP_POWER].TWidth = 32; // ? Mushroom
    NPCTraits[NPCID_SWAP_POWER].THeight = 32;
    NPCTraits[NPCID_SWAP_POWER].FrameOffsetY = 2;

    NPCTraits[NPCID_POWER_S2].TWidth = 32; // SMB2 Mushroom
    NPCTraits[NPCID_POWER_S2].THeight = 32;
    NPCTraits[NPCID_POWER_S2].IsABonus = true;

    NPCTraits[NPCID_MEDAL].TWidth = 32; // dragon coin
    NPCTraits[NPCID_MEDAL].THeight = 50;
    NPCTraits[NPCID_MEDAL].IsABonus = true;
    NPCTraits[NPCID_MEDAL].IsACoin = true;
    NPCTraits[NPCID_MEDAL].Score = 6;

    NPCTraits[NPCID_COIN_S3].TWidth = 28; // SMB3 Coin
    NPCTraits[NPCID_COIN_S3].THeight = 32;
    NPCTraits[NPCID_ITEMGOAL].TWidth = 32; // SMB3 Level exit
    NPCTraits[NPCID_ITEMGOAL].THeight = 32;
    NPCTraits[NPCID_LAVABUBBLE].TWidth = 28; // Big Fireball
    NPCTraits[NPCID_LAVABUBBLE].THeight = 32;
    NPCTraits[NPCID_PLR_FIREBALL].TWidth = 16; // Small Fireball
    NPCTraits[NPCID_PLR_FIREBALL].THeight = 16;
    NPCTraits[NPCID_PLR_ICEBALL].TWidth = 16; // Ice Bolt
    NPCTraits[NPCID_PLR_ICEBALL].THeight = 16;
    NPCTraits[NPCID_PLR_ICEBALL].NoIceBall = true;
    NPCTraits[NPCID_PLR_FIREBALL].NoYoshi = true;
    NPCTraits[NPCID_FIRE_POWER_S3].TWidth = 32; // Fire Flower
    NPCTraits[NPCID_FIRE_POWER_S3].THeight = 32;
    NPCTraits[NPCID_FIRE_POWER_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_ICE_POWER_S3].TWidth = 32; // Ice Flower
    NPCTraits[NPCID_ICE_POWER_S3].THeight = 32;
    NPCTraits[NPCID_ICE_POWER_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_ICE_POWER_S4].TWidth = 32; // Ice Flower
    NPCTraits[NPCID_ICE_POWER_S4].THeight = 32;
    NPCTraits[NPCID_ICE_POWER_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_MINIBOSS].TWidth = 60; // Big Koopa
    NPCTraits[NPCID_MINIBOSS].THeight = 54;
    NPCTraits[NPCID_MINIBOSS].WidthGFX = 68;
    NPCTraits[NPCID_MINIBOSS].HeightGFX = 54;
    NPCTraits[NPCID_MINIBOSS].FrameOffsetY = 2;
    NPCTraits[NPCID_MINIBOSS].NoYoshi = true;
    NPCTraits[NPCID_GOALORB_S3].TWidth = 32; // Boss Exit
    NPCTraits[NPCID_GOALORB_S3].THeight = 32;
    NPCTraits[NPCID_BULLET].TWidth = 32; // Bullet Bill
    NPCTraits[NPCID_BULLET].THeight = 28;
    NPCTraits[NPCID_BIG_BULLET].TWidth = 128; // Giant Bullet Bill
    NPCTraits[NPCID_BIG_BULLET].THeight = 128;
    NPCTraits[NPCID_BIG_BULLET].NoYoshi = true;
    NPCTraits[NPCID_BLU_GUY].TWidth = 32; // Red Shy guy
    NPCTraits[NPCID_BLU_GUY].THeight = 32;
    NPCTraits[NPCID_RED_GUY].TWidth = 32; // Blue Shy guy
    NPCTraits[NPCID_RED_GUY].THeight = 32;
    NPCTraits[NPCID_STACKER].TWidth = 32; // Cactus Thing
    NPCTraits[NPCID_STACKER].THeight = 32;
    NPCTraits[NPCID_CANNONENEMY].TWidth = 32; // Bullet Bill Shooter
    NPCTraits[NPCID_CANNONENEMY].THeight = 32;
    NPCTraits[NPCID_CANNONENEMY].NoYoshi = true;
    NPCTraits[NPCID_CANNONITEM].TWidth = 32; // Bullet Bill Gun
    NPCTraits[NPCID_CANNONITEM].THeight = 32;
    NPCTraits[NPCID_CANNONITEM].FrameOffsetY = 2;
    NPCTraits[NPCID_GLASS_TURTLE].TWidth = 32; // Hard thing
    NPCTraits[NPCID_GLASS_TURTLE].THeight = 32;
    NPCTraits[NPCID_GLASS_TURTLE].FrameOffsetY = 2;
    NPCTraits[NPCID_GLASS_SHELL].TWidth = 32; // Hard Thing shell
    NPCTraits[NPCID_GLASS_SHELL].THeight = 32;
    NPCTraits[NPCID_GLASS_SHELL].FrameOffsetY = 2;
    NPCTraits[NPCID_JUMPER_S3].TWidth = 32; // Bouncy Start Thing
    NPCTraits[NPCID_JUMPER_S3].THeight = 32;
    NPCTraits[NPCID_SPRING].TWidth = 32; // Spring
    NPCTraits[NPCID_SPRING].THeight = 32;
    NPCTraits[NPCID_SPRING].WidthGFX = 32;
    NPCTraits[NPCID_SPRING].HeightGFX = 32;
    NPCTraits[NPCID_SPRING].FrameOffsetY = 2;
    NPCTraits[NPCID_UNDER_FODDER].TWidth = 32; // Grey goomba
    NPCTraits[NPCID_UNDER_FODDER].THeight = 32;
    NPCTraits[NPCID_RED_FISH_S1].TWidth = 32; // Red Jumping Fish
    NPCTraits[NPCID_RED_FISH_S1].THeight = 32;
    NPCTraits[NPCID_HEAVY_THROWER].TWidth = 32; // Hammer Bro
    NPCTraits[NPCID_HEAVY_THROWER].THeight = 48;
    NPCTraits[NPCID_HEAVY_THROWER].FrameOffsetY = 2;
    NPCTraits[NPCID_HEAVY_THROWN].TWidth = 32; // Hammer
    NPCTraits[NPCID_HEAVY_THROWN].THeight = 32;
    NPCTraits[NPCID_HEAVY_THROWN].NoYoshi = true;
    NPCTraits[NPCID_KEY].TWidth = 32; // Key
    NPCTraits[NPCID_KEY].THeight = 32;
    NPCTraits[NPCID_KEY].FrameOffsetY = 1;
    NPCTraits[NPCID_COIN_SWITCH].TWidth = 32; // P Switch
    NPCTraits[NPCID_COIN_SWITCH].THeight = 32;
    NPCTraits[NPCID_COIN_SWITCH].FrameOffsetY = 2;
    NPCTraits[NPCID_TIME_SWITCH].TWidth = 32; // P Switch Time
    NPCTraits[NPCID_TIME_SWITCH].THeight = 32;
    NPCTraits[NPCID_TIME_SWITCH].FrameOffsetY = 2;
    NPCTraits[NPCID_TNT].TWidth = 32; // Push down thing
    NPCTraits[NPCID_TNT].THeight = 32;
    NPCTraits[NPCID_TNT].FrameOffsetY = 2;
    NPCTraits[NPCID_COIN_S4].TWidth = 24; // SMW Coin
    NPCTraits[NPCID_COIN_S4].THeight = 32;
    NPCTraits[NPCID_COIN_5].TWidth = 24; // SMW Blue Coin
    NPCTraits[NPCID_COIN_5].THeight = 32;
    NPCTraits[NPCID_LEAF_POWER].TWidth = 32; // Leaf
    NPCTraits[NPCID_LEAF_POWER].THeight = 32;
    NPCTraits[NPCID_GRN_BOOT].TWidth = 32; // Goombas Shoe
    NPCTraits[NPCID_GRN_BOOT].THeight = 32;
    NPCTraits[NPCID_GRN_BOOT].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_BOOT].FrameOffsetY = 2;
    NPCTraits[NPCID_BLU_BOOT].FrameOffsetY = 2;

    NPCTraits[NPCID_SPIKY_S4].TWidth = 32; // Spiney
    NPCTraits[NPCID_SPIKY_S4].THeight = 32;
    NPCTraits[NPCID_SPIKY_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_SPIKY_BALL_S4].TWidth = 32; // Falling Spiney
    NPCTraits[NPCID_SPIKY_BALL_S4].THeight = 32;
    NPCTraits[NPCID_SPIKY_BALL_S4].FrameOffsetY = 2;

    NPCTraits[NPCID_SPIKY_S3].TWidth = 32; // Spiney
    NPCTraits[NPCID_SPIKY_S3].THeight = 32;
    NPCTraits[NPCID_SPIKY_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_STONE_S3].TWidth = 48; // Thwomp
    NPCTraits[NPCID_STONE_S3].THeight = 64;
    NPCTraits[NPCID_STONE_S3].NoYoshi = true;
    NPCTraits[NPCID_GHOST_S3].TWidth = 32; // Boo
    NPCTraits[NPCID_GHOST_S3].THeight = 32;
    NPCTraits[NPCID_GHOST_S3].NoYoshi = true;
    NPCTraits[NPCID_SPIT_BOSS].TWidth = 32; // Birdo
    NPCTraits[NPCID_SPIT_BOSS].THeight = 60;
    NPCTraits[NPCID_SPIT_BOSS].WidthGFX = 40;
    NPCTraits[NPCID_SPIT_BOSS].HeightGFX = 72;
    NPCTraits[NPCID_SPIT_BOSS].FrameOffsetY = 2;
    NPCTraits[NPCID_SPIT_BOSS].FrameOffsetX = 3;
    NPCTraits[NPCID_SPIT_BOSS].NoYoshi = true;
    NPCTraits[NPCID_SPIT_BOSS_BALL].TWidth = 32; // egg
    NPCTraits[NPCID_SPIT_BOSS_BALL].THeight = 24;
    NPCTraits[NPCID_GOALORB_S2].TWidth = 32; // smb2 exit(birdo)
    NPCTraits[NPCID_GOALORB_S2].THeight = 32;
    NPCTraits[NPCID_GHOST_FAST].TWidth = 32; // ghost 1
    NPCTraits[NPCID_GHOST_FAST].THeight = 32;
    NPCTraits[NPCID_GHOST_FAST].NoYoshi = true;
    NPCTraits[NPCID_GHOST_S4].TWidth = 32; // ghost 2
    NPCTraits[NPCID_GHOST_S4].THeight = 32;
    NPCTraits[NPCID_GHOST_S4].NoYoshi = true;
    NPCTraits[NPCID_BIG_GHOST].TWidth = 128; // big ghost
    NPCTraits[NPCID_BIG_GHOST].THeight = 120;
    NPCTraits[NPCID_BIG_GHOST].WidthGFX = 140;
    NPCTraits[NPCID_BIG_GHOST].HeightGFX = 128;
    NPCTraits[NPCID_BIG_GHOST].NoYoshi = true;
    NPCTraits[NPCID_SLIDE_BLOCK].TWidth = 32; // ice block
    NPCTraits[NPCID_SLIDE_BLOCK].THeight = 32;
    NPCTraits[NPCID_FALL_BLOCK_RED].TWidth = 32; // falling block
    NPCTraits[NPCID_FALL_BLOCK_RED].THeight = 32;
    NPCTraits[NPCID_FALL_BLOCK_RED].NoYoshi = true;
    NPCTraits[NPCID_FALL_BLOCK_BROWN].NoYoshi = true;
    NPCTraits[NPCID_SPIKY_THROWER].TWidth = 32; // lakitu
    NPCTraits[NPCID_SPIKY_THROWER].THeight = 48;
    NPCTraits[NPCID_SPIKY_THROWER].WidthGFX = 32;
    NPCTraits[NPCID_SPIKY_THROWER].HeightGFX = 64;
    NPCTraits[NPCID_ITEM_THROWER].TWidth = 40; // smw lakitu
    NPCTraits[NPCID_ITEM_THROWER].THeight = 48;
    NPCTraits[NPCID_ITEM_THROWER].FrameOffsetY = 6;
    NPCTraits[NPCID_ITEM_THROWER].WidthGFX = 56;
    NPCTraits[NPCID_ITEM_THROWER].HeightGFX = 72;

    NPCTraits[NPCID_SPIKY_BALL_S3].TWidth = 32; // unripe spiney
    NPCTraits[NPCID_SPIKY_BALL_S3].THeight = 32;
    NPCTraits[NPCID_TOOTHYPIPE].TWidth = 32; // killer pipe
    NPCTraits[NPCID_TOOTHYPIPE].THeight = 32;
    NPCTraits[NPCID_TOOTHYPIPE].FrameOffsetY = 2;
    NPCTraits[NPCID_TOOTHY].TWidth = 48; // killer plant
    NPCTraits[NPCID_TOOTHY].THeight = 32;
    NPCTraits[NPCID_TOOTHY].FrameOffsetY = 2;
    NPCTraits[NPCID_TOOTHY].NoYoshi = true;
    NPCTraits[NPCID_BOTTOM_PLANT].TWidth = 32; // down piranha plant
    NPCTraits[NPCID_BOTTOM_PLANT].THeight = 64;
    NPCTraits[NPCID_SIDE_PLANT].TWidth = 48; // left.right piranha plant
    NPCTraits[NPCID_SIDE_PLANT].THeight = 32;
    NPCTraits[NPCID_CRAB].TWidth = 32; // mr crabs
    NPCTraits[NPCID_CRAB].THeight = 32;
    NPCTraits[NPCID_CRAB].FrameOffsetY = 2;
    NPCTraits[NPCID_FLY].TWidth = 32; // bee thing
    NPCTraits[NPCID_FLY].THeight = 32;
    NPCTraits[NPCID_FLY].FrameOffsetY = 2;
    NPCTraits[NPCID_EXT_TURTLE].TWidth = 32; // nekkid koopa
    NPCTraits[NPCID_EXT_TURTLE].THeight = 32;
    NPCTraits[NPCID_EXT_TURTLE].FrameOffsetY = 2;
    NPCTraits[NPCID_VEHICLE].TWidth = 128; // koopa clown car
    NPCTraits[NPCID_VEHICLE].THeight = 128;
    NPCTraits[NPCID_VEHICLE].NoYoshi = true;
    NPCTraits[NPCID_CONVEYOR].TWidth = 32; // smb3 conveyer belt
    NPCTraits[NPCID_CONVEYOR].THeight = 32;
    NPCTraits[NPCID_CONVEYOR].NoYoshi = true;
    NPCTraits[NPCID_METALBARREL].TWidth = 32; // smb3 barrel
    NPCTraits[NPCID_METALBARREL].THeight = 32;
    NPCTraits[NPCID_METALBARREL].NoYoshi = true;
    NPCTraits[NPCID_YELSWITCH_FODDER].TWidth = 32; // purple goomba
    NPCTraits[NPCID_YELSWITCH_FODDER].THeight = 32;
    NPCTraits[NPCID_YELSWITCH_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_YEL_PLATFORM].TWidth = 96; // purple platform
    NPCTraits[NPCID_YEL_PLATFORM].THeight = 32;
    NPCTraits[NPCID_YEL_PLATFORM].NoYoshi = true;
    NPCTraits[NPCID_BLUSWITCH_FODDER].TWidth = 32; // blue goomba
    NPCTraits[NPCID_BLUSWITCH_FODDER].THeight = 32;
    NPCTraits[NPCID_BLUSWITCH_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_BLU_PLATFORM].TWidth = 96; // blue platform
    NPCTraits[NPCID_BLU_PLATFORM].THeight = 32;
    NPCTraits[NPCID_BLU_PLATFORM].NoYoshi = true;
    NPCTraits[NPCID_GRNSWITCH_FODDER].TWidth = 32; // green goomba
    NPCTraits[NPCID_GRNSWITCH_FODDER].THeight = 32;
    NPCTraits[NPCID_GRNSWITCH_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_GRN_PLATFORM].TWidth = 96; // green platform
    NPCTraits[NPCID_GRN_PLATFORM].THeight = 32;
    NPCTraits[NPCID_GRN_PLATFORM].NoYoshi = true;
    NPCTraits[NPCID_REDSWITCH_FODDER].TWidth = 32; // red goomba
    NPCTraits[NPCID_REDSWITCH_FODDER].THeight = 32;
    NPCTraits[NPCID_REDSWITCH_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_PLATFORM].TWidth = 96; // red platform
    NPCTraits[NPCID_RED_PLATFORM].THeight = 32;
    NPCTraits[NPCID_RED_PLATFORM].NoYoshi = true;
    NPCTraits[NPCID_HPIPE_SHORT].TWidth = 128; // grey pipe x
    NPCTraits[NPCID_HPIPE_SHORT].THeight = 32;
    NPCTraits[NPCID_HPIPE_SHORT].NoYoshi = true;
    NPCTraits[NPCID_HPIPE_LONG].TWidth = 256; // big grey pipe x
    NPCTraits[NPCID_HPIPE_LONG].THeight = 32;
    NPCTraits[NPCID_HPIPE_LONG].NoYoshi = true;
    NPCTraits[NPCID_VPIPE_SHORT].TWidth = 32; // grey pipe y
    NPCTraits[NPCID_VPIPE_SHORT].THeight = 127; // FIXME: was set to 127.9 in VB6, which has round semantics for implicit casts
    NPCTraits[NPCID_VPIPE_SHORT].NoYoshi = true;
    NPCTraits[NPCID_VPIPE_LONG].TWidth = 32; // big grey pipe y
    NPCTraits[NPCID_VPIPE_LONG].THeight = 255; // FIXME: was set to 255.9 in VB6
    NPCTraits[NPCID_VPIPE_LONG].NoYoshi = true;
    NPCTraits[NPCID_BIG_FODDER].TWidth = 48; // giant goomba
    NPCTraits[NPCID_BIG_FODDER].THeight = 46;
    NPCTraits[NPCID_BIG_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_BIG_TURTLE].TWidth = 48; // giant green koopa
    NPCTraits[NPCID_BIG_TURTLE].THeight = 48;
    NPCTraits[NPCID_BIG_TURTLE].FrameOffsetY = 2;
    NPCTraits[NPCID_BIG_TURTLE].WidthGFX = 48;
    NPCTraits[NPCID_BIG_TURTLE].HeightGFX = 62;
    NPCTraits[NPCID_BIG_SHELL].TWidth = 44; // giant green shell
    NPCTraits[NPCID_BIG_SHELL].THeight = 44;
    NPCTraits[NPCID_BIG_SHELL].FrameOffsetY = 2;
    NPCTraits[NPCID_BIG_PLANT].TWidth = 48; // giant pirhana plant
    NPCTraits[NPCID_BIG_PLANT].THeight = 64;
    NPCTraits[NPCID_BIG_PLANT].FrameOffsetY = 2;

    NPCTraits[NPCID_LONG_PLANT_UP].TWidth = 48; // gianter pirhana plant
    NPCTraits[NPCID_LONG_PLANT_UP].THeight = 128;
    NPCTraits[NPCID_LONG_PLANT_UP].FrameOffsetY = 2;

    NPCTraits[NPCID_LONG_PLANT_DOWN].TWidth = 48; // gianter pirhana plant
    NPCTraits[NPCID_LONG_PLANT_DOWN].THeight = 128;

    NPCTraits[NPCID_CIVILIAN_SCARED].TWidth = 38; // toad
    NPCTraits[NPCID_CIVILIAN_SCARED].THeight = 54;
    NPCTraits[NPCID_CIVILIAN_SCARED].FrameOffsetY = 2;
    NPCTraits[NPCID_CIVILIAN_SCARED].WidthGFX = 38;
    NPCTraits[NPCID_CIVILIAN_SCARED].HeightGFX = 58;
    // NPCIsToad(NPCID_CIVILIAN_SCARED) = true;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S3].TWidth = 32; // flying green koopa
    NPCTraits[NPCID_GRN_FLY_TURTLE_S3].THeight = 32;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S3].WidthGFX = 32;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S3].HeightGFX = 56;
    // NPCIsAParaTroopa(NPCID_GRN_FLY_TURTLE_S3) = true;
    NPCTraits[NPCID_RED_FLY_TURTLE_S3].TWidth = 32; // flying red koopa
    NPCTraits[NPCID_RED_FLY_TURTLE_S3].THeight = 32;
    NPCTraits[NPCID_RED_FLY_TURTLE_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_FLY_TURTLE_S3].WidthGFX = 32;
    NPCTraits[NPCID_RED_FLY_TURTLE_S3].HeightGFX = 56;
    // NPCIsAParaTroopa(NPCID_RED_FLY_TURTLE_S3) = true;
    NPCTraits[NPCID_JUMPER_S4].TWidth = 32; // black ninja
    NPCTraits[NPCID_JUMPER_S4].THeight = 32;
    NPCTraits[NPCID_BIG_SHELL].FrameOffsetY = 2;
    NPCTraits[NPCID_TANK_TREADS].TWidth = 128; // tank treads
    NPCTraits[NPCID_TANK_TREADS].THeight = 32;
    NPCTraits[NPCID_TANK_TREADS].NoYoshi = true;
    NPCTraits[NPCID_SHORT_WOOD].TWidth = 64; // tank parts
    NPCTraits[NPCID_SHORT_WOOD].THeight = 32;
    NPCTraits[NPCID_SHORT_WOOD].NoYoshi = true;
    NPCTraits[NPCID_LONG_WOOD].TWidth = 128; // tank parts
    NPCTraits[NPCID_LONG_WOOD].THeight = 32;
    NPCTraits[NPCID_LONG_WOOD].NoYoshi = true;
    NPCTraits[NPCID_SLANT_WOOD_L].TWidth = 128; // tank parts
    NPCTraits[NPCID_SLANT_WOOD_L].THeight = 32;
    NPCTraits[NPCID_SLANT_WOOD_L].NoYoshi = true;
    NPCTraits[NPCID_SLANT_WOOD_R].TWidth = 128; // tank parts
    NPCTraits[NPCID_SLANT_WOOD_R].THeight = 32;
    NPCTraits[NPCID_SLANT_WOOD_R].NoYoshi = true;
    NPCTraits[NPCID_SLANT_WOOD_M].TWidth = 256; // tank parts
    NPCTraits[NPCID_SLANT_WOOD_M].THeight = 32;
    NPCTraits[NPCID_SLANT_WOOD_M].NoYoshi = true;
    NPCTraits[NPCID_STATUE_S3].TWidth = 32; // bowser statue
    NPCTraits[NPCID_STATUE_S3].THeight = 64;
    NPCTraits[NPCID_STATUE_S3].NoYoshi = true;
    NPCTraits[NPCID_STATUE_FIRE].TWidth = 32; // statue fireball
    NPCTraits[NPCID_STATUE_FIRE].THeight = 16;
    NPCTraits[NPCID_STATUE_FIRE].NoYoshi = true;
    NPCTraits[NPCID_VILLAIN_S3].TWidth = 62; // smb3 bowser
    NPCTraits[NPCID_VILLAIN_S3].THeight = 80;
    NPCTraits[NPCID_VILLAIN_S3].WidthGFX = 64;
    NPCTraits[NPCID_VILLAIN_S3].HeightGFX = 80;
    NPCTraits[NPCID_VILLAIN_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_VILLAIN_S3].NoYoshi = true;
    NPCTraits[NPCID_VILLAIN_FIRE].TWidth = 48; // smb3 bowser fireball
    NPCTraits[NPCID_VILLAIN_FIRE].THeight = 32;
    NPCTraits[NPCID_VILLAIN_FIRE].NoYoshi = true;
    NPCTraits[NPCID_COIN_S1].TWidth = 20; // smb1 coin
    NPCTraits[NPCID_COIN_S1].THeight = 32;
    NPCTraits[NPCID_FODDER_S1].TWidth = 32; // smb1 brown goomba
    NPCTraits[NPCID_FODDER_S1].THeight = 32;
    NPCTraits[NPCID_FODDER_S1].FrameOffsetY = 2;
    NPCTraits[NPCID_LIFE_S3].TWidth = 32; // 1 up
    NPCTraits[NPCID_LIFE_S3].THeight = 32;
    NPCTraits[NPCID_LIFE_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_LIFE_S3].IsAHit1Block = true;
    NPCTraits[NPCID_ITEM_BURIED].TWidth = 32; // grab grass
    NPCTraits[NPCID_ITEM_BURIED].THeight = 16;
    NPCTraits[NPCID_ITEM_BURIED].FrameOffsetY = -16;
    NPCTraits[NPCID_VEGGIE_1].TWidth = 32; // turnip
    NPCTraits[NPCID_VEGGIE_1].THeight = 32;
    // NPCIsVeggie(NPCID_VEGGIE_1) = true;
    NPCTraits[NPCID_PLANT_S1].TWidth = 32; // SMB1 Plant
    NPCTraits[NPCID_PLANT_S1].THeight = 48;
    NPCTraits[NPCID_CIVILIAN].TWidth = 32; // Inert Toad
    NPCTraits[NPCID_CIVILIAN].THeight = 54;

    NPCTraits[NPCID_CHAR3].TWidth = 32; // Peach
    NPCTraits[NPCID_CHAR3].THeight = 64;
    NPCTraits[NPCID_CHAR3].FrameOffsetY = 2;
    NPCTraits[NPCID_CIVILIAN].FrameOffsetY = 2;
    // NPCIsToad(NPCID_CIVILIAN) = true;
    // NPCIsToad(NPCID_CHAR3) = true;
    NPCTraits[NPCID_PET_GREEN].TWidth = 32; // Green Yoshi
    NPCTraits[NPCID_PET_GREEN].THeight = 32;
    NPCTraits[NPCID_PET_GREEN].WidthGFX = 74;
    NPCTraits[NPCID_PET_GREEN].HeightGFX = 56;
    NPCTraits[NPCID_PET_GREEN].FrameOffsetY = 2;
    NPCTraits[NPCID_PET_GREEN].NoYoshi = true;
    NPCTraits[NPCID_ITEM_POD].TWidth = 32; // Yoshi Egg
    NPCTraits[NPCID_ITEM_POD].THeight = 32;
    NPCTraits[NPCID_STAR_EXIT].TWidth = 32; // SMB3 Star
    NPCTraits[NPCID_STAR_EXIT].THeight = 32;
    NPCTraits[NPCID_PET_BLUE].TWidth = 32; // Blue Yoshi
    NPCTraits[NPCID_PET_BLUE].THeight = 32;
    NPCTraits[NPCID_PET_BLUE].WidthGFX = 74;
    NPCTraits[NPCID_PET_BLUE].HeightGFX = 56;
    NPCTraits[NPCID_PET_BLUE].FrameOffsetY = 2;
    NPCTraits[NPCID_PET_YELLOW].TWidth = 32; // Yellow Yoshi
    NPCTraits[NPCID_PET_YELLOW].THeight = 32;
    NPCTraits[NPCID_PET_YELLOW].WidthGFX = 74;
    NPCTraits[NPCID_PET_YELLOW].HeightGFX = 56;
    NPCTraits[NPCID_PET_YELLOW].FrameOffsetY = 2;
    NPCTraits[NPCID_PET_RED].TWidth = 32; // Red Yoshi
    NPCTraits[NPCID_PET_RED].THeight = 32;
    NPCTraits[NPCID_PET_RED].WidthGFX = 74;
    NPCTraits[NPCID_PET_RED].HeightGFX = 56;
    NPCTraits[NPCID_PET_RED].FrameOffsetY = 2;
    NPCTraits[NPCID_CHAR2].TWidth = 28; // Luigi
    NPCTraits[NPCID_CHAR2].THeight = 62;
    NPCTraits[NPCID_CHAR2].FrameOffsetY = 2;
    // NPCIsToad(NPCID_CHAR2) = true;
    NPCTraits[NPCID_CHAR5].TWidth = 32; // Link
    NPCTraits[NPCID_CHAR5].THeight = 64;
    NPCTraits[NPCID_CHAR5].FrameOffsetY = 2;
    // NPCIsToad(NPCID_CHAR5) = true;
    NPCTraits[NPCID_RED_COIN].TWidth = 28; // SMB3 Red Coin
    NPCTraits[NPCID_RED_COIN].THeight = 32;
    NPCTraits[NPCID_PLATFORM_S3].TWidth = 96; // SMB3 Platform
    NPCTraits[NPCID_PLATFORM_S3].THeight = 32;
    NPCTraits[NPCID_PLATFORM_S3].NoYoshi = true;
    NPCTraits[NPCID_CHECKER_PLATFORM].TWidth = 128; // SMW Falling Platform
    NPCTraits[NPCID_CHECKER_PLATFORM].THeight = 22;
    NPCTraits[NPCID_CHECKER_PLATFORM].NoYoshi = true;
    NPCTraits[NPCID_PLATFORM_S1].TWidth = 128; // SMB Platform
    NPCTraits[NPCID_PLATFORM_S1].THeight = 16;
    NPCTraits[NPCID_PLATFORM_S1].NoYoshi = true;
    NPCTraits[NPCID_PINK_CIVILIAN].TWidth = 24; // Bob-omb buddy
    NPCTraits[NPCID_PINK_CIVILIAN].THeight = 38;
    NPCTraits[NPCID_PINK_CIVILIAN].WidthGFX = 48;
    NPCTraits[NPCID_PINK_CIVILIAN].HeightGFX = 38;
    NPCTraits[NPCID_PINK_CIVILIAN].FrameOffsetY = 2;
    // NPCIsToad(NPCID_PINK_CIVILIAN) = true;
    NPCTraits[NPCID_PET_FIRE].TWidth = 32; // Yoshi Fireball
    NPCTraits[NPCID_PET_FIRE].THeight = 32;
    NPCTraits[NPCID_PET_FIRE].NoYoshi = true;
    NPCTraits[NPCID_GRN_TURTLE_S4].TWidth = 32; // SMW Green Koopa
    NPCTraits[NPCID_GRN_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_GRN_TURTLE_S4].WidthGFX = 32;
    NPCTraits[NPCID_GRN_TURTLE_S4].HeightGFX = 54;
    NPCTraits[NPCID_GRN_TURTLE_S4].FrameOffsetY = 2;
    // NPCDefaultMovement(NPCID_GRN_TURTLE_S4) = true;
    NPCTraits[NPCID_RED_TURTLE_S4].TWidth = 32; // SMW Red Koopa
    NPCTraits[NPCID_RED_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_RED_TURTLE_S4].WidthGFX = 32;
    NPCTraits[NPCID_RED_TURTLE_S4].HeightGFX = 54;
    NPCTraits[NPCID_RED_TURTLE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_TURTLE_S4].TurnsAtCliffs = true;
    // NPCDefaultMovement(NPCID_RED_TURTLE_S4) = true;
    NPCTraits[NPCID_BLU_TURTLE_S4].TWidth = 32; // SMW Blue Koopa
    NPCTraits[NPCID_BLU_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_BLU_TURTLE_S4].WidthGFX = 32;
    NPCTraits[NPCID_BLU_TURTLE_S4].HeightGFX = 54;
    NPCTraits[NPCID_BLU_TURTLE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_BLU_TURTLE_S4].TurnsAtCliffs = true;
    // NPCDefaultMovement(NPCID_BLU_TURTLE_S4) = true;
    NPCTraits[NPCID_YEL_TURTLE_S4].TWidth = 32; // SMW Yellow Koopa
    NPCTraits[NPCID_YEL_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_YEL_TURTLE_S4].WidthGFX = 32;
    NPCTraits[NPCID_YEL_TURTLE_S4].HeightGFX = 54;
    NPCTraits[NPCID_YEL_TURTLE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_YEL_TURTLE_S4].TurnsAtCliffs = true;
    // NPCDefaultMovement(NPCID_YEL_TURTLE_S4) = true;
    NPCTraits[NPCID_GRN_SHELL_S4].TWidth = 32; // SMW Green Shell
    NPCTraits[NPCID_GRN_SHELL_S4].THeight = 32;
    NPCTraits[NPCID_GRN_SHELL_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_GRN_SHELL_S4].IsAShell = true;
    NPCTraits[NPCID_RED_SHELL_S4].TWidth = 32; // SMW Red Shell
    NPCTraits[NPCID_RED_SHELL_S4].THeight = 32;
    NPCTraits[NPCID_RED_SHELL_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_SHELL_S4].IsAShell = true;
    NPCTraits[NPCID_BLU_SHELL_S4].TWidth = 32; // SMW Blue Shell
    NPCTraits[NPCID_BLU_SHELL_S4].THeight = 32;
    NPCTraits[NPCID_BLU_SHELL_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_BLU_SHELL_S4].IsAShell = true;
    NPCTraits[NPCID_YEL_SHELL_S4].TWidth = 32; // SMW Yellow Shell
    NPCTraits[NPCID_YEL_SHELL_S4].THeight = 32;
    NPCTraits[NPCID_YEL_SHELL_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_YEL_SHELL_S4].IsAShell = true;
    NPCTraits[NPCID_GRN_HIT_TURTLE_S4].TWidth = 32; // SMW Green Beach Koopa
    NPCTraits[NPCID_GRN_HIT_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_GRN_HIT_TURTLE_S4].FrameOffsetY = 2;
    // NPCDefaultMovement(NPCID_GRN_HIT_TURTLE_S4) = true;
    NPCTraits[NPCID_RED_HIT_TURTLE_S4].TWidth = 32; // SMW Red Beach Koopa
    NPCTraits[NPCID_RED_HIT_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_RED_HIT_TURTLE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_RED_HIT_TURTLE_S4].TurnsAtCliffs = true;
    // NPCDefaultMovement(NPCID_RED_HIT_TURTLE_S4) = true;
    NPCTraits[NPCID_BLU_HIT_TURTLE_S4].TWidth = 32; // SMW Blue Beach Koopa
    NPCTraits[NPCID_BLU_HIT_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_BLU_HIT_TURTLE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_BLU_HIT_TURTLE_S4].TurnsAtCliffs = true;
    // NPCDefaultMovement(NPCID_BLU_HIT_TURTLE_S4) = true;
    NPCTraits[NPCID_YEL_HIT_TURTLE_S4].TWidth = 32; // SMW Yellow Beach Koopa
    NPCTraits[NPCID_YEL_HIT_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_YEL_HIT_TURTLE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_YEL_HIT_TURTLE_S4].TurnsAtCliffs = true;
    // NPCDefaultMovement(NPCID_YEL_HIT_TURTLE_S4) = true;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S4].TWidth = 32; // SMW Green Para-Koopa
    NPCTraits[NPCID_GRN_FLY_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S4].WidthGFX = 56;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S4].HeightGFX = 56;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S4].FrameOffsetY = 2;
    // NPCIsAParaTroopa(NPCID_GRN_FLY_TURTLE_S4) = true;
    NPCTraits[NPCID_RED_FLY_TURTLE_S4].TWidth = 32; // SMW Red Para-Koopa
    NPCTraits[NPCID_RED_FLY_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_RED_FLY_TURTLE_S4].WidthGFX = 56;
    NPCTraits[NPCID_RED_FLY_TURTLE_S4].HeightGFX = 56;
    NPCTraits[NPCID_RED_FLY_TURTLE_S4].FrameOffsetY = 2;
    // NPCIsAParaTroopa(NPCID_RED_FLY_TURTLE_S4) = true;
    NPCTraits[NPCID_BLU_FLY_TURTLE_S4].TWidth = 32; // SMW Blue Para-Koopa
    NPCTraits[NPCID_BLU_FLY_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_BLU_FLY_TURTLE_S4].WidthGFX = 56;
    NPCTraits[NPCID_BLU_FLY_TURTLE_S4].HeightGFX = 56;
    NPCTraits[NPCID_BLU_FLY_TURTLE_S4].FrameOffsetY = 2;
    // NPCIsAParaTroopa(NPCID_BLU_FLY_TURTLE_S4) = true;
    NPCTraits[NPCID_YEL_FLY_TURTLE_S4].TWidth = 32; // SMW Yellow Para-Koopa
    NPCTraits[NPCID_YEL_FLY_TURTLE_S4].THeight = 32;
    NPCTraits[NPCID_YEL_FLY_TURTLE_S4].WidthGFX = 56;
    NPCTraits[NPCID_YEL_FLY_TURTLE_S4].HeightGFX = 56;
    NPCTraits[NPCID_YEL_FLY_TURTLE_S4].FrameOffsetY = 2;
    // NPCIsAParaTroopa(NPCID_YEL_FLY_TURTLE_S4) = true;
    NPCTraits[NPCID_KNIGHT].TWidth = 36; // Rat Head
    NPCTraits[NPCID_KNIGHT].THeight = 56;
    NPCTraits[NPCID_KNIGHT].WidthGFX = 36;
    NPCTraits[NPCID_KNIGHT].HeightGFX = 66;
    NPCTraits[NPCID_KNIGHT].FrameOffsetY = 2;
    NPCTraits[NPCID_WALK_PLANT].JumpHurt = true;
    NPCTraits[NPCID_WALK_PLANT].NoFireBall = true;
    NPCTraits[NPCID_BULLET].NoFireBall = true;
    // NPCDefaultMovement(NPCID_KNIGHT) = true;
    NPCTraits[NPCID_KNIGHT].NoYoshi = true;
    NPCTraits[NPCID_BLU_SLIME].TWidth = 32; // Blue Bot
    NPCTraits[NPCID_BLU_SLIME].THeight = 34;
    NPCTraits[NPCID_BLU_SLIME].WidthGFX = 32;
    NPCTraits[NPCID_BLU_SLIME].HeightGFX = 26;
    // NPCIsABot(NPCID_BLU_SLIME) = true;
    NPCTraits[NPCID_CYAN_SLIME].TWidth = 32; // Cyan Bot
    NPCTraits[NPCID_CYAN_SLIME].THeight = 34;
    NPCTraits[NPCID_CYAN_SLIME].WidthGFX = 32;
    NPCTraits[NPCID_CYAN_SLIME].HeightGFX = 26;
    // NPCIsABot(NPCID_CYAN_SLIME) = true;
    NPCTraits[NPCID_RED_SLIME].TWidth = 32; // Red Bot
    NPCTraits[NPCID_RED_SLIME].THeight = 34;
    NPCTraits[NPCID_RED_SLIME].WidthGFX = 32;
    NPCTraits[NPCID_RED_SLIME].HeightGFX = 26;
    NPCTraits[NPCID_BIRD].TWidth = 32; // SMB2 Jumpy guy
    NPCTraits[NPCID_BIRD].THeight = 32;
    NPCTraits[NPCID_BIRD].CanWalkOn = true;
    NPCTraits[NPCID_BIRD].GrabFromTop = true;
    // NPCDefaultMovement(NPCID_BIRD) = true;
    NPCTraits[NPCID_RED_SPIT_GUY].TWidth = 32; // Red Sniffit
    NPCTraits[NPCID_RED_SPIT_GUY].THeight = 32;
    NPCTraits[NPCID_RED_SPIT_GUY].CanWalkOn = true;
    NPCTraits[NPCID_RED_SPIT_GUY].GrabFromTop = true;
    // NPCDefaultMovement(NPCID_RED_SPIT_GUY) = true;
    NPCTraits[NPCID_BLU_SPIT_GUY].TWidth = 32; // Blue Sniffit
    NPCTraits[NPCID_BLU_SPIT_GUY].THeight = 32;
    NPCTraits[NPCID_BLU_SPIT_GUY].CanWalkOn = true;
    NPCTraits[NPCID_BLU_SPIT_GUY].GrabFromTop = true;
    // NPCDefaultMovement(NPCID_BLU_SPIT_GUY) = true;
    NPCTraits[NPCID_BLU_SPIT_GUY].TurnsAtCliffs = true;
    NPCTraits[NPCID_GRY_SPIT_GUY].TWidth = 32; // Grey Sniffit
    NPCTraits[NPCID_GRY_SPIT_GUY].THeight = 32;
    NPCTraits[NPCID_GRY_SPIT_GUY].CanWalkOn = true;
    NPCTraits[NPCID_GRY_SPIT_GUY].GrabFromTop = true;
    // NPCDefaultMovement(NPCID_GRY_SPIT_GUY) = true;
    NPCTraits[NPCID_SPIT_GUY_BALL].TWidth = 16; // Bullet
    NPCTraits[NPCID_SPIT_GUY_BALL].THeight = 16;
    NPCTraits[NPCID_SPIT_GUY_BALL].NoYoshi = true;
    NPCTraits[NPCID_SPIT_GUY_BALL].JumpHurt = true;
    NPCTraits[NPCID_BOMB].TWidth = 24; // SMB2 Bomb
    NPCTraits[NPCID_BOMB].THeight = 24;
    NPCTraits[NPCID_BOMB].WidthGFX = 40;
    NPCTraits[NPCID_BOMB].HeightGFX = 40;
    NPCTraits[NPCID_BOMB].WontHurt = true;
    NPCTraits[NPCID_BOMB].IsGrabbable = true;
    NPCTraits[NPCID_BOMB].GrabFromTop = true;
    NPCTraits[NPCID_BOMB].CanWalkOn = true;
    NPCTraits[NPCID_WALK_BOMB_S2].TWidth = 32; // SMB2 Bob-om
    NPCTraits[NPCID_WALK_BOMB_S2].THeight = 32;
    NPCTraits[NPCID_WALK_BOMB_S2].GrabFromTop = true;
    NPCTraits[NPCID_WALK_BOMB_S2].CanWalkOn = true;
    // NPCDefaultMovement(NPCID_WALK_BOMB_S2) = true;
    NPCTraits[NPCID_WALK_BOMB_S3].TWidth = 32; // SMB3 Bob-om
    NPCTraits[NPCID_WALK_BOMB_S3].THeight = 32;
    NPCTraits[NPCID_WALK_BOMB_S3].FrameOffsetY = 2;
    // NPCDefaultMovement(NPCID_WALK_BOMB_S3) = true;
    NPCTraits[NPCID_WALK_BOMB_S3].TurnsAtCliffs = true;
    NPCTraits[NPCID_LIT_BOMB_S3].TWidth = 32; // SMB3 Bomb
    NPCTraits[NPCID_LIT_BOMB_S3].THeight = 28;
    NPCTraits[NPCID_LIT_BOMB_S3].FrameOffsetY = 2;
    NPCTraits[NPCID_LIT_BOMB_S3].IsGrabbable = true;
    NPCTraits[NPCID_COIN_S2].TWidth = 28; // SMB 2 Coin
    NPCTraits[NPCID_COIN_S2].THeight = 32;
    NPCTraits[NPCID_COIN_S2].IsABonus = true;
    NPCTraits[NPCID_COIN_S2].IsACoin = true;
    NPCTraits[NPCID_RING].TWidth = 32; // Sonic Ring
    NPCTraits[NPCID_RING].THeight = 32;
    NPCTraits[NPCID_RING].IsABonus = true;
    NPCTraits[NPCID_RING].IsACoin = true;
    For(A, NPCID_VEGGIE_2, NPCID_VEGGIE_RANDOM) // Veggies
    {
        NPCTraits[A].TWidth = GFXNPC[A].w;
        NPCTraits[A].THeight = GFXNPC[A].h;
        NPCTraits[A].CanWalkOn = true;
        NPCTraits[A].GrabFromTop = true;
        NPCTraits[A].WontHurt = true;
        NPCTraits[A].IsGrabbable = true;
        // NPCIsVeggie(A) = true;
    }
    NPCTraits[NPCID_PET_BLACK].TWidth = 32; // Black Yoshi
    NPCTraits[NPCID_PET_BLACK].THeight = 32;
    NPCTraits[NPCID_PET_BLACK].WidthGFX = 74;
    NPCTraits[NPCID_PET_BLACK].HeightGFX = 56;
    NPCTraits[NPCID_PET_BLACK].FrameOffsetY = 2;
    NPCTraits[NPCID_PET_BLACK].NoYoshi = true;
    NPCTraits[NPCID_PET_PURPLE].TWidth = 32; // Purple Yoshi
    NPCTraits[NPCID_PET_PURPLE].THeight = 32;
    NPCTraits[NPCID_PET_PURPLE].WidthGFX = 74;
    NPCTraits[NPCID_PET_PURPLE].HeightGFX = 56;
    NPCTraits[NPCID_PET_PURPLE].FrameOffsetY = 2;
    NPCTraits[NPCID_PET_PURPLE].NoYoshi = true;
    NPCTraits[NPCID_PET_PINK].TWidth = 32; // Pink Yoshi
    NPCTraits[NPCID_PET_PINK].THeight = 32;
    NPCTraits[NPCID_PET_PINK].WidthGFX = 74;
    NPCTraits[NPCID_PET_PINK].HeightGFX = 56;
    NPCTraits[NPCID_PET_PINK].FrameOffsetY = 2;
    NPCTraits[NPCID_PET_CYAN].TWidth = 32; // Ice Yoshi
    NPCTraits[NPCID_PET_CYAN].THeight = 32;
    NPCTraits[NPCID_PET_CYAN].WidthGFX = 74;
    NPCTraits[NPCID_PET_CYAN].HeightGFX = 56;
    NPCTraits[NPCID_PET_CYAN].FrameOffsetY = 2;
    NPCTraits[NPCID_SIGN].TWidth = 48; // SMW Sign
    NPCTraits[NPCID_SIGN].THeight = 48;
    NPCTraits[NPCID_SIGN].NoYoshi = true;
    NPCTraits[NPCID_SIGN].WontHurt = true;
    NPCTraits[NPCID_SIGN].NoClipping = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[NPCID_CARRY_BLOCK_A].THeight = 32;
    NPCTraits[NPCID_CARRY_BLOCK_B].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[NPCID_CARRY_BLOCK_B].THeight = 32;
    NPCTraits[NPCID_CARRY_BLOCK_C].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[NPCID_CARRY_BLOCK_C].THeight = 32;
    NPCTraits[NPCID_CARRY_BLOCK_D].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[NPCID_CARRY_BLOCK_D].THeight = 32;
    NPCTraits[NPCID_SATURN].TWidth = 32; // Mr Saturn
    NPCTraits[NPCID_SATURN].THeight = 42;
    NPCTraits[NPCID_SATURN].FrameOffsetY = 2;
    NPCTraits[NPCID_SATURN].JumpHurt = true;
    NPCTraits[NPCID_LIFT_SAND].TWidth = 32; // Diggable Dirt
    NPCTraits[NPCID_LIFT_SAND].THeight = 32;
    NPCTraits[NPCID_ROCKET_WOOD].TWidth = 128; // Airship Rocket
    NPCTraits[NPCID_ROCKET_WOOD].THeight = 32;
    NPCTraits[NPCID_ROCKET_WOOD].WidthGFX = 316;
    NPCTraits[NPCID_ROCKET_WOOD].HeightGFX = 32;
    NPCTraits[NPCID_ROCKET_WOOD].NoYoshi = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].TWidth = 32; // SMB2 Mushroom Block
    NPCTraits[NPCID_CARRY_BLOCK_A].THeight = 32;
    NPCTraits[NPCID_BRUTE].TWidth = 32; // SMW Rex
    NPCTraits[NPCID_BRUTE].THeight = 60;
    NPCTraits[NPCID_BRUTE].WidthGFX = 40;
    NPCTraits[NPCID_BRUTE].HeightGFX = 64;
    NPCTraits[NPCID_BRUTE].FrameOffsetY = 2;
    NPCTraits[NPCID_BRUTE_SQUISHED].TWidth = 32; // SMW Rex Smashed
    NPCTraits[NPCID_BRUTE_SQUISHED].THeight = 32;
    NPCTraits[NPCID_BRUTE_SQUISHED].WidthGFX = 32;
    NPCTraits[NPCID_BRUTE_SQUISHED].HeightGFX = 32;
    NPCTraits[NPCID_BRUTE_SQUISHED].FrameOffsetY = 2;
    NPCTraits[NPCID_BIG_MOLE].TWidth = 58; // SMW Mega Mole
    NPCTraits[NPCID_BIG_MOLE].THeight = 58;
    NPCTraits[NPCID_BIG_MOLE].WidthGFX = 64;
    NPCTraits[NPCID_BIG_MOLE].HeightGFX = 64;
    NPCTraits[NPCID_BIG_MOLE].FrameOffsetY = 2;
    NPCTraits[NPCID_CARRY_FODDER].TWidth = 32; // SMW Goomba
    NPCTraits[NPCID_CARRY_FODDER].THeight = 32;
    NPCTraits[NPCID_CARRY_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_HIT_CARRY_FODDER].TWidth = 32; // SMW Stomped Goomba Goomba
    NPCTraits[NPCID_HIT_CARRY_FODDER].THeight = 32;
    NPCTraits[NPCID_HIT_CARRY_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_FLY_CARRY_FODDER].TWidth = 32; // SMW Para-Goomba
    NPCTraits[NPCID_FLY_CARRY_FODDER].THeight = 32;
    NPCTraits[NPCID_FLY_CARRY_FODDER].WidthGFX = 66;
    NPCTraits[NPCID_FLY_CARRY_FODDER].HeightGFX = 50;
    NPCTraits[NPCID_FLY_CARRY_FODDER].FrameOffsetY = 2;
    NPCTraits[NPCID_BULLY].TWidth = 26; // Bully
    NPCTraits[NPCID_BULLY].THeight = 30;
    NPCTraits[NPCID_BULLY].WidthGFX = 26;
    NPCTraits[NPCID_BULLY].HeightGFX = 46;
    NPCTraits[NPCID_BULLY].FrameOffsetY = 2;
    NPCTraits[NPCID_STATUE_POWER].TWidth = 32; // Tanooki Suit
    NPCTraits[NPCID_STATUE_POWER].THeight = 32;
    NPCTraits[NPCID_STATUE_POWER].FrameOffsetY = 2;
    NPCTraits[NPCID_STATUE_POWER].IsABonus = true;
    NPCTraits[NPCID_HEAVY_POWER].TWidth = 32; // Hammer Suit
    NPCTraits[NPCID_HEAVY_POWER].THeight = 32;
    NPCTraits[NPCID_HEAVY_POWER].FrameOffsetY = 2;
    NPCTraits[NPCID_HEAVY_POWER].IsABonus = true;

    NPCTraits[NPCID_CHAR4_HEAVY].WontHurt = true; // Boomerang
    NPCTraits[NPCID_CHAR4_HEAVY].JumpHurt = true;
    NPCTraits[NPCID_CHAR4_HEAVY].NoClipping = true;
    NPCTraits[NPCID_CHAR4_HEAVY].NoYoshi = true;
    NPCTraits[NPCID_CHAR4_HEAVY].Foreground = true;
    NPCTraits[NPCID_CHAR4_HEAVY].NoIceBall = true;

    NPCTraits[NPCID_STACKER].NoIceBall = true;

    NPCTraits[NPCID_PLR_HEAVY].TWidth = 16; // Hammer Suit Hammer
    NPCTraits[NPCID_PLR_HEAVY].THeight = 28;
    NPCTraits[NPCID_PLR_HEAVY].WontHurt = true;
    NPCTraits[NPCID_PLR_HEAVY].NoClipping = true;
    NPCTraits[NPCID_PLR_HEAVY].NoYoshi = true;

    NPCTraits[NPCID_GRN_SHELL_S1].THeight = 28; //  Green SMB1 Shell
    NPCTraits[NPCID_GRN_SHELL_S1].IsAShell = true;
    NPCTraits[NPCID_RED_SHELL_S1].THeight = 28; //  Red SMB1 Shell
    NPCTraits[NPCID_RED_SHELL_S1].IsAShell = true;
    NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].IsAShell = true;

    NPCTraits[NPCID_SAW].JumpHurt = true; // Razor Blade
    NPCTraits[NPCID_SAW].WidthGFX = 64;
    NPCTraits[NPCID_SAW].HeightGFX = 64;
    NPCTraits[NPCID_SAW].TWidth = 48;
    NPCTraits[NPCID_SAW].THeight = 48;
    NPCTraits[NPCID_SAW].FrameOffsetY = 8;
    NPCTraits[NPCID_STONE_S4].JumpHurt = true; // SMW Thwomp
    NPCTraits[NPCID_STONE_S4].TWidth = 48;
    NPCTraits[NPCID_STONE_S4].THeight = 64;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].TWidth = 32; // POW
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].THeight = 32; // POW
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].CanWalkOn = true;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].MovesPlayer = true;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].WontHurt = true;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].IsABlock = true;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].GrabFromTop = true;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].IsGrabbable = true;

    NPCTraits[NPCID_STATUE_S4].WontHurt = true; // SMW Bowser Statue
    NPCTraits[NPCID_STATUE_S4].IsABlock = true;
    NPCTraits[NPCID_STATUE_S4].CanWalkOn = true;
    NPCTraits[NPCID_STATUE_S4].MovesPlayer = true;
    NPCTraits[NPCID_STATUE_S4].TWidth = 48;
    NPCTraits[NPCID_STATUE_S4].THeight = 46;
    NPCTraits[NPCID_STATUE_S4].FrameOffsetY = 2;
    NPCTraits[NPCID_GOALTAPE].TWidth = 48;
    NPCTraits[NPCID_GOALTAPE].THeight = 16;
    NPCTraits[NPCID_GOALTAPE].IsABonus = true;
    NPCTraits[NPCID_FIRE_POWER_S1].IsABonus = true; // SMB1 Flower
    NPCTraits[NPCID_FIRE_POWER_S4].IsABonus = true; // SMW Flower
    NPCTraits[NPCID_POWER_S1].IsABonus = true; // SMB1 Mushroom
    NPCTraits[NPCID_POWER_S4].IsABonus = true; // SMW Mushroom
    NPCTraits[NPCID_LIFE_S1].IsABonus = true; // SMB1 1-up
    NPCTraits[NPCID_LIFE_S4].IsABonus = true; // SMW 1-up
    NPCTraits[NPCID_3_LIFE].IsABonus = true; // SMW 3 up
    NPCTraits[NPCID_3_LIFE].TWidth = 30;
    NPCTraits[NPCID_SKELETON].TWidth = 32;

    NPCTraits[NPCID_SICK_BOSS].NoYoshi = true;
    NPCTraits[NPCID_SICK_BOSS_BALL].NoYoshi = true;
    NPCTraits[NPCID_SICK_BOSS_BALL].JumpHurt = true;
    NPCTraits[NPCID_SICK_BOSS].TWidth = 80;
    NPCTraits[NPCID_SICK_BOSS].THeight = 94;

    NPCTraits[NPCID_LAVA_MONSTER].TWidth = 56;
    NPCTraits[NPCID_LAVA_MONSTER].THeight = 60;
    NPCTraits[NPCID_LAVA_MONSTER].WidthGFX = 130;
    NPCTraits[NPCID_LAVA_MONSTER].HeightGFX = 64;
    NPCTraits[NPCID_LAVA_MONSTER].JumpHurt = true;
    NPCTraits[NPCID_LAVA_MONSTER].NoClipping = true;
    NPCTraits[NPCID_FLIER].NoYoshi = true;
    NPCTraits[NPCID_ROCKET_FLIER].NoYoshi = true;
    NPCTraits[NPCID_WALL_BUG].NoYoshi = true;
    NPCTraits[NPCID_WALL_SPARK].NoYoshi = true;

    NPCTraits[NPCID_FLIER].TWidth = 46;
    NPCTraits[NPCID_FLIER].THeight = 26;
    NPCTraits[NPCID_FLIER].NoYoshi = true;
    NPCTraits[NPCID_FLIER].CanWalkOn = true;
    NPCTraits[NPCID_ROCKET_FLIER].NoYoshi = true;
    NPCTraits[NPCID_ROCKET_FLIER].CanWalkOn = true;
    NPCTraits[NPCID_SICK_BOSS].JumpHurt = true;

    NPCTraits[NPCID_BOSS_CASE].NoYoshi = true;
    NPCTraits[NPCID_BOSS_FRAGILE].NoYoshi = true;
    NPCTraits[NPCID_BOSS_FRAGILE].CanWalkOn = true;
    NPCTraits[NPCID_BOSS_FRAGILE].MovesPlayer = true;
    NPCTraits[NPCID_BOSS_FRAGILE].IsABlock = true;
    NPCTraits[NPCID_BOSS_FRAGILE].NoClipping = true;
    NPCTraits[NPCID_BOSS_FRAGILE].TWidth = 96;
    NPCTraits[NPCID_BOSS_FRAGILE].THeight = 106;

    NPCTraits[NPCID_SICK_BOSS_BALL].NoYoshi = true;
    NPCTraits[NPCID_SICK_BOSS].NoYoshi = true;
    NPCTraits[NPCID_VILLAIN_S1].NoYoshi = true;

    NPCTraits[NPCID_SKELETON].WidthGFX = 48;
    NPCTraits[NPCID_SKELETON].HeightGFX = 64;
    NPCTraits[NPCID_SKELETON].THeight = 64;
    NPCTraits[NPCID_SKELETON].TurnsAtCliffs = true;
    NPCTraits[NPCID_SKELETON].FrameOffsetY = 2;
    // NPCDefaultMovement(189) = true;
    NPCTraits[NPCID_RED_TURTLE_S1].WidthGFX = 32;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S1].WidthGFX = 32;
    NPCTraits[NPCID_RED_FLY_TURTLE_S1].WidthGFX = 32;
    NPCTraits[NPCID_GRN_TURTLE_S1].WidthGFX = 32;
    NPCTraits[NPCID_RED_TURTLE_S1].HeightGFX = 48;
    NPCTraits[NPCID_GRN_FLY_TURTLE_S1].HeightGFX = 48;
    NPCTraits[NPCID_RED_FLY_TURTLE_S1].HeightGFX = 48;
    NPCTraits[NPCID_GRN_TURTLE_S1].HeightGFX = 48;

    NPCTraits[NPCID_RAFT].WidthGFX = 32;
    NPCTraits[NPCID_RAFT].HeightGFX = 32;

    NPCTraits[NPCID_BONE_FISH].TWidth = 48;

    NPCTraits[NPCID_SQUID_S1].HeightGFX = 48;
    NPCTraits[NPCID_SQUID_S1].WidthGFX = 32;
    NPCTraits[NPCID_SQUID_S1].FrameOffsetY = 16;

    NPCTraits[NPCID_SQUID_S1].JumpHurt = true;
    NPCTraits[NPCID_BONE_FISH].JumpHurt = true;

    NPCTraits[NPCID_RAFT].THeight = 20;
    NPCTraits[NPCID_RAFT].FrameOffsetY = 12;
    NPCTraits[NPCID_RAINBOW_SHELL].FrameOffsetY = 2;

    NPCTraits[NPCID_RAFT].MovesPlayer = true;

    NPCTraits[NPCID_RED_TURTLE_S1].TurnsAtCliffs = true;

    NPCTraits[NPCID_AXE].NoYoshi = true;
    NPCTraits[NPCID_SAW].NoYoshi = true;
    NPCTraits[NPCID_STONE_S4].NoYoshi = true;
    NPCTraits[NPCID_STATUE_S4].NoYoshi = true;
    NPCTraits[NPCID_SKELETON].NoYoshi = true;

    NPCTraits[NPCID_RAFT].NoYoshi = true;
    NPCTraits[NPCID_CHECKPOINT].NoYoshi = true;
    NPCTraits[NPCID_CHECKPOINT].IsABonus = true;



    // NPCIsAParaTroopa(NPCID_GRN_FLY_TURTLE_S1) = true;
    // NPCIsAParaTroopa(NPCID_RED_FLY_TURTLE_S1) = true;
    NPCTraits[NPCID_AXE].IsABonus = true;
    // NPCDefaultMovement(NPCID_GRN_TURTLE_S1) = true;
    // NPCDefaultMovement(NPCID_RED_TURTLE_S1) = true;
    // NPCDefaultMovement(NPCID_BRUTE) = true;
    // 'NPCDefaultMovement(NPCID_BRUTE_SQUISHED) = true;
    // 'NPCDefaultMovement(NPCID_BIG_MOLE) = true;
    // NPCDefaultMovement(NPCID_CARRY_FODDER) = true;
    // NPCDefaultMovement(NPCID_FLY_CARRY_FODDER) = true;
    NPCTraits[NPCID_BIG_MOLE].CanWalkOn = true;
    NPCTraits[NPCID_BIG_MOLE].IsAHit1Block = true;
    NPCTraits[NPCID_ROCKET_WOOD].WontHurt = true;
    NPCTraits[NPCID_HIT_CARRY_FODDER].WontHurt = true;
    NPCTraits[NPCID_ROCKET_WOOD].CanWalkOn = true;
    NPCTraits[NPCID_ROCKET_WOOD].MovesPlayer = true;
    NPCTraits[NPCID_ROCKET_WOOD].IsABlock = true;
    // NPCDefaultMovement(NPCID_ROCKET_WOOD) = true;
    NPCTraits[NPCID_LIFT_SAND].WontHurt = true;
    NPCTraits[NPCID_LIFT_SAND].CanWalkOn = true;
    NPCTraits[NPCID_LIFT_SAND].IsAHit1Block = true;
    NPCTraits[NPCID_LIFT_SAND].GrabFromTop = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].WontHurt = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].CanWalkOn = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].IsABlock = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].MovesPlayer = true;
    NPCTraits[NPCID_CARRY_BLOCK_B].WontHurt = true;
    NPCTraits[NPCID_CARRY_BLOCK_B].CanWalkOn = true;
    NPCTraits[NPCID_CARRY_BLOCK_B].IsABlock = true;
    NPCTraits[NPCID_CARRY_BLOCK_B].MovesPlayer = true;
    NPCTraits[NPCID_CARRY_BLOCK_C].WontHurt = true;
    NPCTraits[NPCID_CARRY_BLOCK_C].CanWalkOn = true;
    NPCTraits[NPCID_CARRY_BLOCK_C].IsABlock = true;
    NPCTraits[NPCID_CARRY_BLOCK_C].MovesPlayer = true;
    NPCTraits[NPCID_CARRY_BLOCK_D].WontHurt = true;
    NPCTraits[NPCID_CARRY_BLOCK_D].CanWalkOn = true;
    NPCTraits[NPCID_CARRY_BLOCK_D].IsABlock = true;
    NPCTraits[NPCID_CARRY_BLOCK_D].MovesPlayer = true;
    NPCTraits[NPCID_LIFT_SAND].NoYoshi = true;


    NPCTraits[NPCID_GOALTAPE].WontHurt = true;
    NPCTraits[NPCID_GOALTAPE].NoYoshi = true;


    NPCTraits[NPCID_CARRY_BLOCK_A].IsGrabbable = true;
    NPCTraits[NPCID_HIT_CARRY_FODDER].IsGrabbable = true;
    NPCTraits[NPCID_CARRY_BLOCK_A].GrabFromTop = true;
    NPCTraits[NPCID_CARRY_BLOCK_B].IsGrabbable = true;
    NPCTraits[NPCID_CARRY_BLOCK_B].GrabFromTop = true;
    NPCTraits[NPCID_CARRY_BLOCK_C].IsGrabbable = true;
    NPCTraits[NPCID_CARRY_BLOCK_C].GrabFromTop = true;
    NPCTraits[NPCID_CARRY_BLOCK_D].IsGrabbable = true;
    NPCTraits[NPCID_CARRY_BLOCK_D].GrabFromTop = true;
    // NPCIsABot(NPCID_RED_SLIME) = true;
    // NPCIsYoshi(NPCID_PET_GREEN) = true;
    // NPCIsYoshi(NPCID_PET_BLUE) = true;
    // NPCIsYoshi(NPCID_PET_YELLOW) = true;
    // NPCIsYoshi(NPCID_PET_RED) = true;
    // NPCIsYoshi(NPCID_PET_BLACK) = true;
    // NPCIsYoshi(NPCID_PET_PURPLE) = true;
    // NPCIsYoshi(NPCID_PET_PINK) = true;
    // NPCIsYoshi(NPCID_PET_CYAN) = true;
    // NPCIsBoot(NPCID_GRN_BOOT) = true;
    // NPCIsBoot(NPCID_RED_BOOT) = true;
    // NPCIsBoot(NPCID_BLU_BOOT) = true;
    NPCTraits[NPCID_GRN_SHELL_S3].IsAShell = true;
    NPCTraits[NPCID_RED_SHELL_S3].IsAShell = true;
    NPCTraits[NPCID_GLASS_SHELL].IsAShell = true;
    NPCTraits[NPCID_BIG_SHELL].IsAShell = true;
    NPCTraits[NPCID_POWER_S3].IsABonus = true;
    NPCTraits[NPCID_SWAP_POWER].IsABonus = true;
    NPCTraits[NPCID_COIN_S3].IsABonus = true;
    NPCTraits[NPCID_ITEMGOAL].IsABonus = true;
    NPCTraits[NPCID_FIRE_POWER_S3].IsABonus = true;
    NPCTraits[NPCID_ICE_POWER_S3].IsABonus = true;
    NPCTraits[NPCID_ICE_POWER_S4].IsABonus = true;
    NPCTraits[NPCID_GOALORB_S3].IsABonus = true;
    NPCTraits[NPCID_COIN_S4].IsABonus = true;
    NPCTraits[NPCID_COIN_5].IsABonus = true;
    NPCTraits[NPCID_LEAF_POWER].IsABonus = true;
    NPCTraits[NPCID_GOALORB_S2].IsABonus = true;
    NPCTraits[NPCID_CIVILIAN_SCARED].IsABonus = true;
    NPCTraits[NPCID_COIN_S1].IsABonus = true;
    NPCTraits[NPCID_LIFE_S3].IsABonus = true;
    NPCTraits[NPCID_CIVILIAN].IsABonus = true;
    NPCTraits[NPCID_CHAR3].IsABonus = true;
    NPCTraits[NPCID_STAR_EXIT].IsABonus = true;
    NPCTraits[NPCID_CHAR2].IsABonus = true;
    NPCTraits[NPCID_CHAR5].IsABonus = true;
    NPCTraits[NPCID_RED_COIN].IsABonus = true;
    NPCTraits[NPCID_PINK_CIVILIAN].IsABonus = true;
    NPCTraits[NPCID_POISON].IsABonus = true;
    NPCTraits[NPCID_STAR_COLLECT].IsABonus = true;
    NPCTraits[NPCID_INVINCIBILITY_POWER].IsABonus = true;
    NPCTraits[NPCID_INVINCIBILITY_POWER].TFrames = 4;
    NPCTraits[NPCID_INVINCIBILITY_POWER].FrameSpeed = 4;
    NPCTraits[NPCID_COIN_S3].IsACoin = true;
    NPCTraits[NPCID_COIN_S4].IsACoin = true;
    NPCTraits[NPCID_COIN_5].IsACoin = true;
    NPCTraits[NPCID_COIN_S1].IsACoin = true;
    NPCTraits[NPCID_RED_COIN].IsACoin = true;
    // NPCIsAnExit(NPCID_ITEMGOAL) = true;
    // NPCIsAnExit(NPCID_GOALORB_S3) = true;
    // NPCIsAnExit(NPCID_GOALORB_S2) = true;
    // NPCIsAnExit(NPCID_STAR_EXIT) = true;
    // NPCIsAnExit(NPCID_STAR_COLLECT) = true;
    NPCTraits[NPCID_PLANT_S3].JumpHurt = true;
    NPCTraits[NPCID_FIRE_PLANT].JumpHurt = true;
    NPCTraits[NPCID_LAVABUBBLE].JumpHurt = true;
    NPCTraits[NPCID_HEAVY_THROWN].JumpHurt = true;
    NPCTraits[NPCID_SPIKY_S3].JumpHurt = true;
    NPCTraits[NPCID_SPIKY_S4].JumpHurt = true;
    NPCTraits[NPCID_SPIKY_BALL_S4].JumpHurt = true;
    NPCTraits[NPCID_STONE_S3].JumpHurt = true;
    NPCTraits[NPCID_GHOST_S3].JumpHurt = true;
    NPCTraits[NPCID_GHOST_FAST].JumpHurt = true;
    NPCTraits[NPCID_GHOST_S4].JumpHurt = true;
    NPCTraits[NPCID_BIG_GHOST].JumpHurt = true;
    NPCTraits[NPCID_SPIKY_BALL_S3].JumpHurt = true;
    NPCTraits[NPCID_BOTTOM_PLANT].JumpHurt = true;
    NPCTraits[NPCID_SIDE_PLANT].JumpHurt = true;
    NPCTraits[NPCID_CRAB].JumpHurt = true;
    NPCTraits[NPCID_FLY].JumpHurt = true;
    NPCTraits[NPCID_BIG_PLANT].JumpHurt = true;
    NPCTraits[NPCID_LONG_PLANT_UP].JumpHurt = true;
    NPCTraits[NPCID_LONG_PLANT_DOWN].JumpHurt = true;
    NPCTraits[NPCID_STATUE_FIRE].JumpHurt = true;
    NPCTraits[NPCID_VILLAIN_FIRE].JumpHurt = true;
    NPCTraits[NPCID_PLANT_S1].JumpHurt = true;
    NPCTraits[NPCID_BLU_GUY].CanWalkOn = true;
    NPCTraits[NPCID_RED_GUY].CanWalkOn = true;
    NPCTraits[NPCID_STACKER].CanWalkOn = true;
    NPCTraits[NPCID_CANNONENEMY].CanWalkOn = true;
    NPCTraits[NPCID_JUMPER_S3].CanWalkOn = true;
    // NPCTraits[NPCID_RED_FISH_S1].CanWalkOn = true;
    NPCTraits[NPCID_KEY].CanWalkOn = true;
    NPCTraits[NPCID_SPIT_BOSS].CanWalkOn = true;
    NPCTraits[NPCID_SPIT_BOSS_BALL].CanWalkOn = true;
    NPCTraits[NPCID_SLIDE_BLOCK].CanWalkOn = true;
    NPCTraits[NPCID_FALL_BLOCK_RED].CanWalkOn = true;
    NPCTraits[NPCID_FALL_BLOCK_BROWN].CanWalkOn = true;
    NPCTraits[NPCID_VEHICLE].CanWalkOn = true;
    NPCTraits[NPCID_CONVEYOR].CanWalkOn = true;
    NPCTraits[NPCID_METALBARREL].CanWalkOn = true;
    NPCTraits[NPCID_YEL_PLATFORM].CanWalkOn = true;
    NPCTraits[NPCID_BLU_PLATFORM].CanWalkOn = true;
    NPCTraits[NPCID_GRN_PLATFORM].CanWalkOn = true;
    NPCTraits[NPCID_RED_PLATFORM].CanWalkOn = true;
    NPCTraits[NPCID_HPIPE_SHORT].CanWalkOn = true;
    NPCTraits[NPCID_HPIPE_LONG].CanWalkOn = true;
    NPCTraits[NPCID_VPIPE_SHORT].CanWalkOn = true;
    NPCTraits[NPCID_VPIPE_LONG].CanWalkOn = true;
    NPCTraits[NPCID_TANK_TREADS].CanWalkOn = true;
    NPCTraits[NPCID_SHORT_WOOD].CanWalkOn = true;
    NPCTraits[NPCID_LONG_WOOD].CanWalkOn = true;
    NPCTraits[NPCID_SLANT_WOOD_L].CanWalkOn = true;
    NPCTraits[NPCID_SLANT_WOOD_R].CanWalkOn = true;
    NPCTraits[NPCID_SLANT_WOOD_M].CanWalkOn = true;
    NPCTraits[NPCID_STATUE_S3].CanWalkOn = true;
    NPCTraits[NPCID_ITEM_BURIED].CanWalkOn = true;
    NPCTraits[NPCID_VEGGIE_1].CanWalkOn = true;
    NPCTraits[NPCID_PLATFORM_S3].CanWalkOn = true;
    NPCTraits[NPCID_CHECKER_PLATFORM].CanWalkOn = true;
    NPCTraits[NPCID_PLATFORM_S1].CanWalkOn = true;
    NPCTraits[NPCID_BLU_GUY].GrabFromTop = true;
    NPCTraits[NPCID_RED_GUY].GrabFromTop = true;
    NPCTraits[NPCID_STACKER].GrabFromTop = true;
    NPCTraits[NPCID_JUMPER_S3].GrabFromTop = true;
    // NPCTraits[NPCID_RED_FISH_S1].GrabFromTop = true;
    NPCTraits[NPCID_KEY].GrabFromTop = true;
    NPCTraits[NPCID_SPIT_BOSS_BALL].GrabFromTop = true;
    NPCTraits[NPCID_SLIDE_BLOCK].GrabFromTop = true;
    NPCTraits[NPCID_ITEM_BURIED].GrabFromTop = true;
    NPCTraits[NPCID_VEGGIE_1].GrabFromTop = true;
    NPCTraits[NPCID_PLANT_S3].NoClipping = true;
    NPCTraits[NPCID_FIRE_PLANT].NoClipping = true;
    // NPCTraits[NPCID_COIN_S3].NoClipping = true;
    NPCTraits[NPCID_LAVABUBBLE].NoClipping = true;
    NPCTraits[NPCID_BULLET].NoClipping = true;
    NPCTraits[NPCID_BIG_BULLET].NoClipping = true;
    NPCTraits[NPCID_HEAVY_THROWN].NoClipping = true;
    NPCTraits[NPCID_LEAF_POWER].NoClipping = true;
    NPCTraits[NPCID_GHOST_S3].NoClipping = true;
    NPCTraits[NPCID_GHOST_FAST].NoClipping = true;
    NPCTraits[NPCID_GHOST_S4].NoClipping = true;
    NPCTraits[NPCID_BIG_GHOST].NoClipping = true;
    NPCTraits[NPCID_FALL_BLOCK_RED].NoClipping = true;
    NPCTraits[NPCID_FALL_BLOCK_BROWN].NoClipping = true;
    NPCTraits[NPCID_SPIKY_THROWER].NoClipping = true;
    NPCTraits[NPCID_ITEM_THROWER].NoClipping = true;
    NPCTraits[NPCID_TOOTHY].NoClipping = true;
    NPCTraits[NPCID_BOTTOM_PLANT].NoClipping = true;
    NPCTraits[NPCID_SIDE_PLANT].NoClipping = true;
    // NPCTraits[NPCID_VEHICLE].NoClipping = true;
    NPCTraits[NPCID_CONVEYOR].NoClipping = true;
    NPCTraits[NPCID_YEL_PLATFORM].NoClipping = true;
    NPCTraits[NPCID_BLU_PLATFORM].NoClipping = true;
    NPCTraits[NPCID_GRN_PLATFORM].NoClipping = true;
    NPCTraits[NPCID_RED_PLATFORM].NoClipping = true;
    NPCTraits[NPCID_BIG_PLANT].NoClipping = true;
    NPCTraits[NPCID_LONG_PLANT_UP].NoClipping = true;
    NPCTraits[NPCID_LONG_PLANT_DOWN].NoClipping = true;
    NPCTraits[NPCID_STATUE_FIRE].NoClipping = true;
    NPCTraits[NPCID_VILLAIN_FIRE].NoClipping = true;
    NPCTraits[NPCID_ITEM_BURIED].NoClipping = true;
    NPCTraits[NPCID_PLANT_S1].NoClipping = true;
    NPCTraits[NPCID_PLATFORM_S3].NoClipping = true;
    NPCTraits[NPCID_CHECKER_PLATFORM].NoClipping = true;
    NPCTraits[NPCID_PLATFORM_S1].NoClipping = true;
    NPCTraits[NPCID_PET_FIRE].NoClipping = true;
    NPCTraits[NPCID_RED_TURTLE_S3].TurnsAtCliffs = true;
    NPCTraits[NPCID_RED_GUY].TurnsAtCliffs = true;
    NPCTraits[NPCID_STACKER].TurnsAtCliffs = true;
    NPCTraits[NPCID_EXT_TURTLE].TurnsAtCliffs = true;
    NPCTraits[NPCID_YELSWITCH_FODDER].TurnsAtCliffs = true;
    NPCTraits[NPCID_BLUSWITCH_FODDER].TurnsAtCliffs = true;
    NPCTraits[NPCID_GRNSWITCH_FODDER].TurnsAtCliffs = true;
    NPCTraits[NPCID_REDSWITCH_FODDER].TurnsAtCliffs = true;
    NPCTraits[NPCID_PINK_CIVILIAN].TurnsAtCliffs = true;
    NPCTraits[NPCID_SATURN].TurnsAtCliffs = true;
    NPCTraits[NPCID_CANNONENEMY].MovesPlayer = true;
    NPCTraits[NPCID_KEY].MovesPlayer = true;
    NPCTraits[NPCID_COIN_SWITCH].MovesPlayer = true;
    NPCTraits[NPCID_TIME_SWITCH].MovesPlayer = true;
    NPCTraits[NPCID_TNT].MovesPlayer = true;
    NPCTraits[NPCID_SLIDE_BLOCK].MovesPlayer = true;
    NPCTraits[NPCID_FALL_BLOCK_RED].MovesPlayer = true;
    NPCTraits[NPCID_FALL_BLOCK_BROWN].MovesPlayer = true;
    NPCTraits[NPCID_CONVEYOR].MovesPlayer = true;
    NPCTraits[NPCID_METALBARREL].MovesPlayer = true;
    NPCTraits[NPCID_HPIPE_SHORT].MovesPlayer = true;
    NPCTraits[NPCID_HPIPE_LONG].MovesPlayer = true;
    NPCTraits[NPCID_VPIPE_SHORT].MovesPlayer = true;
    NPCTraits[NPCID_VPIPE_LONG].MovesPlayer = true;
    NPCTraits[NPCID_TANK_TREADS].MovesPlayer = true;
    NPCTraits[NPCID_SHORT_WOOD].MovesPlayer = true;
    NPCTraits[NPCID_LONG_WOOD].MovesPlayer = true;
    NPCTraits[NPCID_SLANT_WOOD_L].MovesPlayer = true;
    NPCTraits[NPCID_SLANT_WOOD_R].MovesPlayer = true;
    NPCTraits[NPCID_SLANT_WOOD_M].MovesPlayer = true;
    NPCTraits[NPCID_STATUE_S3].MovesPlayer = true;
    NPCTraits[NPCID_CANNONENEMY].WontHurt = true;
    NPCTraits[NPCID_CANNONITEM].WontHurt = true;
    NPCTraits[NPCID_SPRING].WontHurt = true;
    NPCTraits[NPCID_KEY].WontHurt = true;
    NPCTraits[NPCID_COIN_SWITCH].WontHurt = true;
    NPCTraits[NPCID_TIME_SWITCH].WontHurt = true;
    NPCTraits[NPCID_TNT].WontHurt = true;
    NPCTraits[NPCID_GRN_BOOT].WontHurt = true;
    NPCTraits[NPCID_RED_BOOT].WontHurt = true;
    NPCTraits[NPCID_BLU_BOOT].WontHurt = true;
    NPCTraits[NPCID_SLIDE_BLOCK].WontHurt = true;
    NPCTraits[NPCID_FALL_BLOCK_RED].WontHurt = true;
    NPCTraits[NPCID_FALL_BLOCK_BROWN].WontHurt = true;
    NPCTraits[NPCID_TOOTHYPIPE].WontHurt = true;
    NPCTraits[NPCID_TOOTHY].WontHurt = true;
    NPCTraits[NPCID_VEHICLE].WontHurt = true;
    NPCTraits[NPCID_CONVEYOR].WontHurt = true;
    NPCTraits[NPCID_METALBARREL].WontHurt = true;
    NPCTraits[NPCID_YEL_PLATFORM].WontHurt = true;
    NPCTraits[NPCID_BLU_PLATFORM].WontHurt = true;
    NPCTraits[NPCID_GRN_PLATFORM].WontHurt = true;
    NPCTraits[NPCID_RED_PLATFORM].WontHurt = true;
    NPCTraits[NPCID_HPIPE_SHORT].WontHurt = true;
    NPCTraits[NPCID_HPIPE_LONG].WontHurt = true;
    NPCTraits[NPCID_VPIPE_SHORT].WontHurt = true;
    NPCTraits[NPCID_VPIPE_LONG].WontHurt = true;
    NPCTraits[NPCID_TANK_TREADS].WontHurt = true;
    NPCTraits[NPCID_SHORT_WOOD].WontHurt = true;
    NPCTraits[NPCID_LONG_WOOD].WontHurt = true;
    NPCTraits[NPCID_SLANT_WOOD_L].WontHurt = true;
    NPCTraits[NPCID_SLANT_WOOD_R].WontHurt = true;
    NPCTraits[NPCID_SLANT_WOOD_M].WontHurt = true;
    NPCTraits[NPCID_STATUE_S3].WontHurt = true;
    NPCTraits[NPCID_ITEM_BURIED].WontHurt = true;
    NPCTraits[NPCID_VEGGIE_1].WontHurt = true;
    NPCTraits[NPCID_PET_GREEN].WontHurt = true;
    NPCTraits[NPCID_ITEM_POD].WontHurt = true;
    NPCTraits[NPCID_PET_BLUE].WontHurt = true;
    NPCTraits[NPCID_PET_YELLOW].WontHurt = true;
    NPCTraits[NPCID_PET_RED].WontHurt = true;
    NPCTraits[NPCID_PLATFORM_S3].WontHurt = true;
    NPCTraits[NPCID_CHECKER_PLATFORM].WontHurt = true;
    NPCTraits[NPCID_PLATFORM_S1].WontHurt = true;
    NPCTraits[NPCID_PINK_CIVILIAN].WontHurt = true;
    NPCTraits[NPCID_PET_FIRE].WontHurt = true;
    NPCTraits[NPCID_PET_BLACK].WontHurt = true;
    NPCTraits[NPCID_PET_PURPLE].WontHurt = true;
    NPCTraits[NPCID_PET_PINK].WontHurt = true;
    NPCTraits[NPCID_PET_CYAN].WontHurt = true;
    NPCTraits[NPCID_SATURN].WontHurt = true;
    NPCTraits[NPCID_CANNONITEM].IsGrabbable = true;
    NPCTraits[NPCID_SPRING].IsGrabbable = true;
    NPCTraits[NPCID_KEY].IsGrabbable = true;
    NPCTraits[NPCID_COIN_SWITCH].IsGrabbable = true;
    NPCTraits[NPCID_TIME_SWITCH].IsGrabbable = true;
    NPCTraits[NPCID_GRN_BOOT].IsGrabbable = true;
    NPCTraits[NPCID_RED_BOOT].IsGrabbable = true;
    NPCTraits[NPCID_BLU_BOOT].IsGrabbable = true;
    NPCTraits[NPCID_SLIDE_BLOCK].IsGrabbable = true;
    NPCTraits[NPCID_TOOTHYPIPE].IsGrabbable = true;
    NPCTraits[NPCID_VEGGIE_1].IsGrabbable = true;
    NPCTraits[NPCID_ITEM_POD].IsGrabbable = true;
    NPCTraits[NPCID_SATURN].IsGrabbable = true;
    NPCTraits[NPCID_CANNONENEMY].IsABlock = true;
    NPCTraits[NPCID_KEY].IsABlock = true;
    NPCTraits[NPCID_COIN_SWITCH].IsABlock = true;
    NPCTraits[NPCID_TIME_SWITCH].IsABlock = true;
    NPCTraits[NPCID_TNT].IsABlock = true;
    NPCTraits[NPCID_SLIDE_BLOCK].IsABlock = true;
    NPCTraits[NPCID_FALL_BLOCK_RED].IsABlock = true;
    NPCTraits[NPCID_FALL_BLOCK_BROWN].IsABlock = true;
    NPCTraits[NPCID_VEHICLE].IsABlock = true;
    NPCTraits[NPCID_CONVEYOR].IsABlock = true;
    NPCTraits[NPCID_METALBARREL].IsABlock = true;
    NPCTraits[NPCID_HPIPE_SHORT].IsABlock = true;
    NPCTraits[NPCID_HPIPE_LONG].IsABlock = true;
    NPCTraits[NPCID_VPIPE_SHORT].IsABlock = true;
    NPCTraits[NPCID_VPIPE_LONG].IsABlock = true;
    NPCTraits[NPCID_TANK_TREADS].IsABlock = true;
    NPCTraits[NPCID_SHORT_WOOD].IsABlock = true;
    NPCTraits[NPCID_LONG_WOOD].IsABlock = true;
    NPCTraits[NPCID_SLANT_WOOD_L].IsABlock = true;
    NPCTraits[NPCID_SLANT_WOOD_R].IsABlock = true;
    NPCTraits[NPCID_SLANT_WOOD_M].IsABlock = true;
    NPCTraits[NPCID_STATUE_S3].IsABlock = true;
    NPCTraits[NPCID_RAFT].IsABlock = true;
    NPCTraits[NPCID_GRN_BOOT].IsAHit1Block = true;
    NPCTraits[NPCID_RED_BOOT].IsAHit1Block = true;
    NPCTraits[NPCID_BLU_BOOT].IsAHit1Block = true;
    NPCTraits[NPCID_CANNONITEM].IsAHit1Block = true;
    NPCTraits[NPCID_TOOTHYPIPE].IsAHit1Block = true;
    NPCTraits[NPCID_SPRING].IsAHit1Block = true;
    NPCTraits[NPCID_YEL_PLATFORM].IsAHit1Block = true;
    NPCTraits[NPCID_BLU_PLATFORM].IsAHit1Block = true;
    NPCTraits[NPCID_GRN_PLATFORM].IsAHit1Block = true;
    NPCTraits[NPCID_RED_PLATFORM].IsAHit1Block = true;
    NPCTraits[NPCID_PLATFORM_S3].IsAHit1Block = true;
    NPCTraits[NPCID_CHECKER_PLATFORM].IsAHit1Block = true;
    NPCTraits[NPCID_PLATFORM_S1].IsAHit1Block = true;
    NPCTraits[NPCID_RAFT].WontHurt = true;
    NPCTraits[NPCID_RAFT].CanWalkOn = true;
    NPCTraits[NPCID_CANNONITEM].StandsOnPlayer = true;
    NPCTraits[NPCID_SPRING].StandsOnPlayer = true;
    NPCTraits[NPCID_KEY].StandsOnPlayer = true;
    NPCTraits[NPCID_COIN_SWITCH].StandsOnPlayer = true;
    NPCTraits[NPCID_TIME_SWITCH].StandsOnPlayer = true;
    NPCTraits[NPCID_TNT].StandsOnPlayer = true;
    NPCTraits[NPCID_GRN_BOOT].StandsOnPlayer = true;
    NPCTraits[NPCID_RED_BOOT].StandsOnPlayer = true;
    NPCTraits[NPCID_BLU_BOOT].StandsOnPlayer = true;
    NPCTraits[NPCID_TOOTHYPIPE].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_GREEN].StandsOnPlayer = true;
    NPCTraits[NPCID_ITEM_POD].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_BLUE].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_YELLOW].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_RED].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_BLACK].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_PURPLE].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_PINK].StandsOnPlayer = true;
    NPCTraits[NPCID_PET_CYAN].StandsOnPlayer = true;


    NPCTraits[NPCID_VINE_BUG].CanWalkOn = true;
    NPCTraits[NPCID_VINE_BUG].GrabFromTop = true;
    NPCTraits[NPCID_VINE_BUG].NoFireBall = true;
    NPCTraits[NPCID_VINE_BUG].NoClipping = true;
    NPCTraits[NPCID_VINE_BUG].TWidth = 32;
    NPCTraits[NPCID_VINE_BUG].THeight = 32;

    NPCTraits[NPCID_BOSS_CASE].CanWalkOn = true;
    NPCTraits[NPCID_BOSS_CASE].WontHurt = true;
    NPCTraits[NPCID_BOSS_CASE].MovesPlayer = true;
    NPCTraits[NPCID_BOSS_CASE].TWidth = 128;
    NPCTraits[NPCID_BOSS_CASE].THeight = 128;
    NPCTraits[NPCID_BOSS_CASE].IsABlock = true;

    NPCTraits[NPCID_BOSS_CASE].Foreground = true;
    NPCTraits[NPCID_WALL_TURTLE].JumpHurt = true;
    NPCTraits[NPCID_WALL_SPARK].JumpHurt = true;
    NPCTraits[NPCID_WALL_BUG].JumpHurt = true;

    NPCTraits[NPCID_ROCKET_FLIER].TWidth = 48;
    NPCTraits[NPCID_ROCKET_FLIER].THeight = 28;
    NPCTraits[NPCID_ROCKET_FLIER].WidthGFX = 112;
    NPCTraits[NPCID_ROCKET_FLIER].HeightGFX = 28;

    NPCTraits[NPCID_WALL_BUG].WidthGFX = 44;
    NPCTraits[NPCID_WALL_BUG].HeightGFX = 44;
    NPCTraits[NPCID_WALL_BUG].FrameOffsetY = 6;


    NPCTraits[NPCID_JUMP_PLANT].TWidth = 32;
    NPCTraits[NPCID_JUMP_PLANT].THeight = 42;
    NPCTraits[NPCID_JUMP_PLANT].JumpHurt = true;
    NPCTraits[NPCID_JUMP_PLANT].NoClipping = true;

    NPCTraits[NPCID_BAT].TWidth = 32;
    NPCTraits[NPCID_BAT].THeight = 32;
    NPCTraits[NPCID_BAT].NoClipping = true;


    NPCTraits[NPCID_HOMING_BALL].TWidth = 28;
    NPCTraits[NPCID_HOMING_BALL].NoClipping = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].IsABlock = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].NoClipping = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].MovesPlayer = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].CanWalkOn = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].WontHurt = true;





    NPCTraits[NPCID_LAVABUBBLE].Foreground = true;
    NPCTraits[NPCID_BULLET].Foreground = true;
    NPCTraits[NPCID_BIG_BULLET].Foreground = true;
    NPCTraits[NPCID_RED_FISH_S1].Foreground = true;
    NPCTraits[NPCID_HEAVY_THROWN].Foreground = true;
    NPCTraits[NPCID_LEAF_POWER].Foreground = true;
    NPCTraits[NPCID_GHOST_S3].Foreground = true;
    NPCTraits[NPCID_GHOST_FAST].Foreground = true;
    NPCTraits[NPCID_GHOST_S4].Foreground = true;
    NPCTraits[NPCID_BIG_GHOST].Foreground = true;
    NPCTraits[NPCID_SPIKY_THROWER].Foreground = true;
    NPCTraits[NPCID_STATUE_FIRE].Foreground = true;
    NPCTraits[NPCID_VILLAIN_S3].Foreground = true;
    NPCTraits[NPCID_VILLAIN_FIRE].Foreground = true;
    NPCTraits[NPCID_PET_FIRE].Foreground = true;
    NPCTraits[NPCID_PLR_HEAVY].Foreground = true;
    // NPCDefaultMovement(NPCID_FODDER_S3) = true;
    // NPCDefaultMovement(NPCID_RED_FODDER) = true;
    // NPCDefaultMovement(NPCID_RED_FLY_FODDER) = true;
    // NPCDefaultMovement(NPCID_GRN_TURTLE_S3) = true;
    // NPCDefaultMovement(NPCID_RED_TURTLE_S3) = true;
    // NPCDefaultMovement(NPCID_BLU_GUY) = true;
    // NPCDefaultMovement(NPCID_RED_GUY) = true;
    // NPCDefaultMovement(NPCID_STACKER) = true;
    // NPCDefaultMovement(NPCID_GLASS_TURTLE) = true;
    // NPCDefaultMovement(NPCID_UNDER_FODDER) = true;
    // NPCDefaultMovement(NPCID_SPIKY_S3) = true;
    // NPCDefaultMovement(NPCID_SPIKY_S4) = true;
    // NPCDefaultMovement(NPCID_TOOTHY) = true;
    // NPCDefaultMovement(NPCID_CRAB) = true;
    // NPCDefaultMovement(NPCID_EXT_TURTLE) = true;
    // NPCDefaultMovement(NPCID_YELSWITCH_FODDER) = true;
    // NPCDefaultMovement(NPCID_BLUSWITCH_FODDER) = true;
    // NPCDefaultMovement(NPCID_GRNSWITCH_FODDER) = true;
    // NPCDefaultMovement(NPCID_REDSWITCH_FODDER) = true;
    // NPCDefaultMovement(NPCID_BIG_FODDER) = true;
    // NPCDefaultMovement(NPCID_BIG_TURTLE) = true;
    // NPCDefaultMovement(NPCID_JUMPER_S4) = true;
    // NPCDefaultMovement(NPCID_TANK_TREADS) = true;
    // NPCDefaultMovement(NPCID_FODDER_S1) = true;
    // NPCDefaultMovement(NPCID_ITEM_BURIED) = true;
    // NPCDefaultMovement(NPCID_PINK_CIVILIAN) = true;

    NPCTraits[NPCID_BLU_GUY].NoFireBall = true;
    NPCTraits[NPCID_RED_GUY].NoFireBall = true;
    NPCTraits[NPCID_GLASS_TURTLE].NoFireBall = true;
    NPCTraits[NPCID_GLASS_SHELL].NoFireBall = true;
    NPCTraits[NPCID_LIT_BOMB_S3].NoFireBall = true;
    NPCTraits[NPCID_WALK_BOMB_S3].NoFireBall = true;
    NPCTraits[NPCID_BIRD].NoFireBall = true;
    NPCTraits[NPCID_RED_SPIT_GUY].NoFireBall = true;
    NPCTraits[NPCID_BLU_SPIT_GUY].NoFireBall = true;
    NPCTraits[NPCID_GRY_SPIT_GUY].NoFireBall = true;
    NPCTraits[NPCID_LAVABUBBLE].NoFireBall = true;
    NPCTraits[NPCID_JUMPER_S3].NoFireBall = true;


    // link sword beam
    NPCTraits[NPCID_SWORDBEAM].TWidth = 16;
    NPCTraits[NPCID_SWORDBEAM].THeight = 8;
    NPCTraits[NPCID_SWORDBEAM].WontHurt = true;
    NPCTraits[NPCID_SWORDBEAM].JumpHurt = true;
    NPCTraits[NPCID_SWORDBEAM].NoGravity = true;

    NPCTraits[NPCID_MAGIC_BOSS].NoYoshi = true;
    NPCTraits[NPCID_MAGIC_BOSS_SHELL].NoYoshi = true;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].NoYoshi = true;
    NPCTraits[NPCID_FIRE_BOSS].NoYoshi = true;
    NPCTraits[NPCID_FIRE_BOSS_SHELL].NoYoshi = true;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].NoYoshi = true;


    NPCTraits[NPCID_QUAD_BALL].NoClipping = true;
    NPCTraits[NPCID_QUAD_BALL].TWidth = 16;
    NPCTraits[NPCID_QUAD_BALL].THeight = 16;
    NPCTraits[NPCID_QUAD_BALL].NoYoshi = true;
    NPCTraits[NPCID_QUAD_BALL].JumpHurt = true;

    NPCTraits[NPCID_QUAD_SPITTER].TWidth = 60;
    NPCTraits[NPCID_QUAD_SPITTER].THeight = 24;
    NPCTraits[NPCID_QUAD_SPITTER].WidthGFX = 64;
    NPCTraits[NPCID_QUAD_SPITTER].HeightGFX = 32;
    NPCTraits[NPCID_QUAD_SPITTER].NoFireBall = true;
    NPCTraits[NPCID_QUAD_SPITTER].JumpHurt = true;

    NPCTraits[NPCID_FLY_BLOCK].TWidth = 32;
    NPCTraits[NPCID_FLY_BLOCK].THeight = 44;
    NPCTraits[NPCID_FLY_BLOCK].WontHurt = true;
    NPCTraits[NPCID_FLY_BLOCK].JumpHurt = true;
    NPCTraits[NPCID_FLY_BLOCK].IsGrabbable = true;

    NPCTraits[NPCID_FLY_CANNON].TWidth = 32;
    NPCTraits[NPCID_FLY_CANNON].THeight = 44;
    NPCTraits[NPCID_FLY_CANNON].WidthGFX = 96;
    NPCTraits[NPCID_FLY_CANNON].HeightGFX = 44;
    NPCTraits[NPCID_FLY_CANNON].WontHurt = true;
    NPCTraits[NPCID_FLY_CANNON].JumpHurt = true;
    NPCTraits[NPCID_FLY_CANNON].IsGrabbable = true;


// set ice
    NPCTraits[NPCID_GOALTAPE].NoIceBall = true;
    NPCTraits[NPCID_CHECKPOINT].NoIceBall = true;

    NPCTraits[NPCID_MAGIC_BOSS].NoIceBall = true;
    NPCTraits[NPCID_MAGIC_BOSS_SHELL].NoIceBall = true;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].NoIceBall = true;
    NPCTraits[NPCID_QUAD_SPITTER].NoIceBall = true;
    NPCTraits[NPCID_FIRE_BOSS].NoIceBall = true;
    NPCTraits[NPCID_FIRE_BOSS_SHELL].NoIceBall = true;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].NoIceBall = true;
    NPCTraits[NPCID_MINIBOSS].NoIceBall = true;
    NPCTraits[NPCID_BIG_BULLET].NoIceBall = true;
    NPCTraits[NPCID_SPRING].NoIceBall = true;
    NPCTraits[NPCID_HEAVY_THROWN].NoIceBall = true;
    NPCTraits[NPCID_KEY].NoIceBall = true;
    NPCTraits[NPCID_COIN_SWITCH].NoIceBall = true;
    NPCTraits[NPCID_GRN_BOOT].NoIceBall = true;
    NPCTraits[NPCID_STONE_S3].NoIceBall = true;
    NPCTraits[NPCID_GHOST_S3].NoIceBall = true;
    NPCTraits[NPCID_SPIT_BOSS].NoIceBall = true;
    NPCTraits[NPCID_SPIT_BOSS_BALL].NoIceBall = true;
    NPCTraits[NPCID_GHOST_FAST].NoIceBall = true;
    NPCTraits[NPCID_GHOST_S4].NoIceBall = true;
    NPCTraits[NPCID_BIG_GHOST].NoIceBall = true;
    NPCTraits[NPCID_SLIDE_BLOCK].NoIceBall = true;
    NPCTraits[NPCID_FALL_BLOCK_RED].NoIceBall = true;
    NPCTraits[NPCID_VEHICLE].NoIceBall = true;
    For(A, NPCID_CONVEYOR, NPCID_VPIPE_LONG)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[NPCID_CIVILIAN_SCARED].NoIceBall = true;
    For(A, NPCID_TANK_TREADS, NPCID_COIN_S1)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[NPCID_ITEM_BURIED].NoIceBall = true;
    NPCTraits[NPCID_VEGGIE_1].NoIceBall = true;
    NPCTraits[NPCID_CIVILIAN].NoIceBall = true;
    For(A, NPCID_PET_GREEN, NPCID_PET_FIRE)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[NPCID_SPIT_GUY_BALL].NoIceBall = true;
    NPCTraits[NPCID_BOMB].NoIceBall = true;
    For(A, NPCID_COIN_S2, NPCID_ROCKET_WOOD)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[NPCID_PLR_HEAVY].NoIceBall = true;
    NPCTraits[NPCID_AXE].NoIceBall = true;
    NPCTraits[NPCID_SAW].NoIceBall = true;
    NPCTraits[NPCID_STONE_S4].NoIceBall = true;
    NPCTraits[NPCID_STATUE_S4].NoIceBall = true;
    NPCTraits[NPCID_RAFT].NoIceBall = true;
    NPCTraits[NPCID_RED_BOOT].NoIceBall = true;
    NPCTraits[NPCID_CHECKPOINT].NoIceBall = true;
    NPCTraits[NPCID_BLU_BOOT].NoIceBall = true;
    NPCTraits[NPCID_FLIPPED_RAINBOW_SHELL].NoIceBall = true;
    NPCTraits[NPCID_LAVA_MONSTER].NoIceBall = true;
    NPCTraits[NPCID_VILLAIN_S1].NoIceBall = true;
    NPCTraits[NPCID_SICK_BOSS].NoIceBall = true;
    NPCTraits[NPCID_BOSS_CASE].NoIceBall = true;
    NPCTraits[NPCID_BOSS_FRAGILE].NoIceBall = true;
    NPCTraits[NPCID_HOMING_BALL].NoIceBall = true;
    For(A, NPCID_HOMING_BALL_GEN, NPCID_PET_CYAN)
    {
        NPCTraits[A].NoIceBall = true;
    }
    NPCTraits[NPCID_ICE_BLOCK].NoIceBall = true;
    NPCTraits[NPCID_TIME_SWITCH].NoIceBall = true;
    NPCTraits[NPCID_TNT].NoIceBall = true;
    NPCTraits[NPCID_EARTHQUAKE_BLOCK].NoIceBall = true;
    NPCTraits[NPCID_PLANT_FIRE].NoIceBall = true;
    NPCTraits[NPCID_FLY_POWER].NoIceBall = true;
    NPCTraits[NPCID_LOCK_DOOR].NoIceBall = true;
    NPCTraits[NPCID_LONG_PLANT_UP].NoIceBall = true;
    NPCTraits[NPCID_LONG_PLANT_DOWN].NoIceBall = true;
    NPCTraits[NPCID_FIRE_DISK].NoIceBall = true;
    NPCTraits[NPCID_FIRE_CHAIN].NoIceBall = true;
    NPCTraits[NPCID_BOMBER_BOSS].NoIceBall = true;
    NPCTraits[NPCID_GEM_1].NoIceBall = true;
    NPCTraits[NPCID_GEM_5].NoIceBall = true;
    NPCTraits[NPCID_GEM_20].NoIceBall = true;
    NPCTraits[NPCID_COIN_5].NoIceBall = true;
    NPCTraits[NPCID_COIN_S4].NoIceBall = true;
    NPCTraits[NPCID_MEDAL].NoIceBall = true;
    NPCTraits[NPCID_FLY_BLOCK].NoIceBall = true;
    NPCTraits[NPCID_FLY_CANNON].NoIceBall = true;

    for(int A = NPCID_FIRE_BOSS_FIRE; A <= NPCID_CHAR4_HEAVY; ++A)
    {
        if(A != NPCID_SPIKY_S4 && A != NPCID_SPIKY_BALL_S4)
            NPCTraits[A].NoIceBall = true;
    }


    NPCTraits[NPCID_STACKER].THeight = 30;
    NPCTraits[NPCID_STACKER].HeightGFX = 32;
    NPCTraits[NPCID_STACKER].WidthGFX = 32;


    // NPCDefaultMovement(NPCID_SATURN) = true;


    NPCTraits[NPCID_MAGIC_BOSS].WidthGFX = 84;
    NPCTraits[NPCID_MAGIC_BOSS].HeightGFX = 62;
    NPCTraits[NPCID_MAGIC_BOSS].TWidth = 44;
    NPCTraits[NPCID_MAGIC_BOSS].THeight = 50;

    NPCTraits[NPCID_MAGIC_BOSS_SHELL].WidthGFX = 44;
    NPCTraits[NPCID_MAGIC_BOSS_SHELL].HeightGFX = 32;
    NPCTraits[NPCID_MAGIC_BOSS_SHELL].TWidth = 32;
    NPCTraits[NPCID_MAGIC_BOSS_SHELL].THeight = 28;

    NPCTraits[NPCID_MAGIC_BOSS_BALL].WidthGFX = 16;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].HeightGFX = 32;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].TWidth = 16;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].THeight = 32;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].NoClipping = true;
    NPCTraits[NPCID_MAGIC_BOSS_BALL].JumpHurt = true;

    NPCTraits[NPCID_FIRE_BOSS].WidthGFX = 64;
    NPCTraits[NPCID_FIRE_BOSS].HeightGFX = 64;
    NPCTraits[NPCID_FIRE_BOSS].TWidth = 48;
    NPCTraits[NPCID_FIRE_BOSS].THeight = 48;

    NPCTraits[NPCID_FIRE_BOSS_SHELL].WidthGFX = 40;
    NPCTraits[NPCID_FIRE_BOSS_SHELL].HeightGFX = 36;
    NPCTraits[NPCID_FIRE_BOSS_SHELL].TWidth = 36;
    NPCTraits[NPCID_FIRE_BOSS_SHELL].THeight = 32;

    NPCTraits[NPCID_FIRE_BOSS_FIRE].WidthGFX = 64;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].HeightGFX = 32;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].TWidth = 64;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].THeight = 26;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].FrameOffsetY = 4;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].NoClipping = true;
    NPCTraits[NPCID_FIRE_BOSS_FIRE].JumpHurt = true;

    // Default NPCs that must use the canonical camera
    NPCTraits[NPCID_STONE_S3].UseDefaultCam = true;
    NPCTraits[NPCID_STONE_S4].UseDefaultCam = true;
    NPCTraits[NPCID_METALBARREL].UseDefaultCam = true;
    NPCTraits[NPCID_CANNONENEMY].UseDefaultCam = true;
    NPCTraits[NPCID_BULLET].UseDefaultCam = true;
    NPCTraits[NPCID_BIG_BULLET].UseDefaultCam = true;
    NPCTraits[NPCID_GHOST_FAST].UseDefaultCam = true;
    NPCTraits[NPCID_STATUE_S3].UseDefaultCam = true;
    NPCTraits[NPCID_STATUE_S4].UseDefaultCam = true;
    NPCTraits[NPCID_HOMING_BALL_GEN].UseDefaultCam = true;
    NPCTraits[NPCID_LAVA_MONSTER].UseDefaultCam = true;
    NPCTraits[NPCID_SPIKY_THROWER].UseDefaultCam = true;
    NPCTraits[NPCID_ITEM_THROWER].UseDefaultCam = true;
    // NPCTraits[NPCID_SAW].UseDefaultCam = true; // held back for 1.3.7
    // NPCTraits[NPCID_BOMB].UseDefaultCam = true; // held back for 1.3.7

    // Default NPCs that render differently when inactive
    for(int A = 1; A <= maxNPCType; A++)
    {
        if(NPCTraits[A].IsABlock || NPCTraits[A].IsAHit1Block || NPCTraits[A].IsACoin || NPCTraits[A].IsAVine || NPCTraits[A].IsABonus)
            NPCTraits[A].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    }

    NPCTraits[NPCID_CHECKPOINT].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ITEM_BURIED].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_CONVEYOR].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STONE_S3].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STONE_S4].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_HOMING_BALL_GEN].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ITEMGOAL].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_CANNONENEMY].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STATUE_S3].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_STATUE_S4].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ITEM_POD].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_SPRING].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_CANNONITEM].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_KEY].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_TIME_SWITCH].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_COIN_SWITCH].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_ICE_BLOCK].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_VEHICLE].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_AXE].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_WALK_PLANT].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_FLY_BLOCK].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_FLY_CANNON].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_MAGIC_DOOR].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    NPCTraits[NPCID_DOOR_MAKER].InactiveRender = NPCTraits_t::SHOW_ALWAYS;
    // NPCTraits[NPCID_SAW].InactiveRender = NPCTraits_t::SHOW_STATIC;
    // NPCTraits[NPCID_BOMB].InactiveRender = NPCTraits_t::SHOW_STATIC;

    NPCTraits[NPCID_LAVABUBBLE].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_PLANT_S3].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_BOTTOM_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_SIDE_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_BIG_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_PLANT_S1].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_FIRE_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_LONG_PLANT_UP].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_LONG_PLANT_DOWN].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_JUMP_PLANT].InactiveRender = NPCTraits_t::SKIP;
    NPCTraits[NPCID_LAVA_MONSTER].InactiveRender = NPCTraits_t::SKIP;

    NPCTraits[NPCID_BULLET].InactiveRender = NPCTraits_t::SMOKE;
    NPCTraits[NPCID_BIG_BULLET].InactiveRender = NPCTraits_t::SMOKE;
    NPCTraits[NPCID_GHOST_FAST].InactiveRender = NPCTraits_t::SMOKE;


    For(A, 1, maxBlockType)
    {
        BlockWidth[A] = 32;
        BlockHeight[A] = 32;
    }

    BlockWidth[571] = 64;
    BlockWidth[572] = 64;
    BlockWidth[615] = 64;

    BlockWidth[634] = 64;
    BlockHeight[634] = 64;

    BlockHeight[595] = 64;
    BlockHeight[596] = 64;
    BlockHeight[597] = 64;

    BlockHeight[569] = 64;
    BlockHeight[570] = 64;
    BlockHeight[571] = 64;
    BlockHeight[572] = 64;
    BlockHeight[575] = 64;

    BlockOnlyHitspot1[572] = true;
    BlockWidth[21] = 64;
    BlockWidth[22] = 64;
    BlockHeight[23] = 64;
    BlockHeight[24] = 64;
    BlockWidth[34] = 64;
    BlockWidth[35] = 64;
    BlockWidth[36] = 64;
    BlockWidth[37] = 64;
    BlockWidth[61] = 128;
    BlockHeight[61] = 128;
    BlockWidth[78] = 64;
    BlockWidth[91] = 64;
    BlockHeight[91] = 64;
    BlockWidth[92] = 128;
    BlockHeight[92] = 128;
    BlockWidth[93] = 128;
    BlockHeight[93] = 128;
    BlockWidth[103] = 64;
    BlockWidth[104] = 64;
    BlockWidth[113] = 64;
    BlockWidth[114] = 64;
    BlockWidth[125] = 64;
    BlockHeight[125] = 64;
    BlockWidth[182] = 96;
    BlockHeight[182] = 96;
    BlockWidth[184] = 64;
    BlockHeight[184] = 64;
    BlockWidth[185] = 128;
    BlockHeight[187] = 128;
    BlockWidth[187] = 128;
    BlockWidth[194] = 64;
    BlockWidth[195] = 64;
    BlockWidth[196] = 64;
    BlockWidth[197] = 64;
    BlockWidth[206] = 64;
    BlockHeight[206] = 64;
    BlockHeight[211] = 64;
    BlockHeight[212] = 64;
    BlockWidth[224] = 64;
    BlockHeight[224] = 64;
    BlockWidth[225] = 64;
    BlockHeight[225] = 64;
    BlockWidth[226] = 64;
    BlockHeight[226] = 64;
    BlockWidth[262] = 128;
    BlockHeight[262] = 128;

    BlockWidth[616] = 64;
    BlockWidth[617] = 64;
    BlockSlope[616] = -1;
    BlockSlope[617] = 1;

    BlockSlope[635] = -1;
    BlockSlope[636] = -1;
    BlockWidth[636] = 64;
    BlockSlope[637] = 1;
    BlockSlope[638] = 1;
    BlockWidth[638] = 64;


    For(A, 137, 146)
    {
        BlockWidth[A] = 64;
    }

    For(A, 147, 158)
    {
        BlockHeight[A] = 64;
    }
    BlockWidth[301] = 128;
    BlockWidth[302] = 128;
    BlockWidth[303] = 128;
    BlockWidth[304] = 128;
    BlockWidth[306] = 64;
    BlockWidth[308] = 64;
    BlockWidth[312] = 64;
    BlockWidth[314] = 64;
    BlockWidth[319] = 128;
    BlockWidth[320] = 128;
    BlockWidth[321] = 128;
    BlockWidth[322] = 128;
    BlockWidth[325] = 64;
    BlockWidth[324] = 64;
    BlockWidth[325] = 64;
    BlockWidth[324] = 64;
    BlockWidth[336] = 64;
    BlockWidth[338] = 64;
    BlockWidth[340] = 64;
    BlockWidth[342] = 64;
    BlockWidth[357] = 64;
    BlockWidth[360] = 64;
    BlockWidth[361] = 64;
    BlockWidth[364] = 64;
    BlockWidth[365] = 64;
    BlockWidth[366] = 64;
    BlockWidth[367] = 64;
    BlockWidth[368] = 64;
    BlockHeight[376] = 64;
    BlockHeight[377] = 64;
    BlockWidth[378] = 64;
    BlockHeight[378] = 64;
    BlockWidth[472] = 64;
    BlockHeight[472] = 32;
    BlockWidth[474] = 64;
    BlockHeight[474] = 32;
    BlockWidth[476] = 64;
    BlockHeight[476] = 32;
    BlockWidth[479] = 64;
    BlockHeight[479] = 32;
    BlockWidth[505] = 64;
    BlockHeight[505] = 32;
    BlockWidth[506] = 64;
    BlockHeight[506] = 64;

    BlockWidth[613] = 64;

    BlockWidth[507] = 64;
    BlockHeight[507] = 32;

    BlockWidth[599] = 64;
    BlockHeight[599] = 64;


    BlockWidth[508] = 64;
    BlockHeight[508] = 32;

    BlockWidth[529] = 32;
    BlockHeight[529] = 64;

    BlockWidth[527] = 64;
    BlockHeight[527] = 96;




    BlockWidth[534] = 48;
    BlockHeight[534] = 128;
    BlockWidth[535] = 48;
    BlockHeight[535] = 128;
    BlockWidth[536] = 128;
    BlockHeight[536] = 48;
    BlockWidth[537] = 128;
    BlockHeight[537] = 48;
    BlockWidth[540] = 64;




    BlockSlope[472] = -1;
    BlockSlope[474] = 1;
    BlockSlope2[476] = 1;
    BlockSlope2[479] = -1;


    BlockSlope2[77] = 1;
    BlockSlope2[78] = 1;
    BlockSlope2[613] = -1;
    BlockSlope2[614] = -1;


    BlockSlope[480] = -1;
    BlockSlope[482] = 1;
    BlockSlope2[486] = 1;
    BlockSlope2[485] = -1;


    BlockOnlyHitspot1[372] = true;
    BlockOnlyHitspot1[373] = true;
    BlockOnlyHitspot1[374] = true;
    BlockOnlyHitspot1[375] = true;
    BlockOnlyHitspot1[379] = true;
    BlockOnlyHitspot1[380] = true;
    BlockOnlyHitspot1[381] = true;
    BlockOnlyHitspot1[382] = true;
    BlockOnlyHitspot1[389] = true;
    BlockOnlyHitspot1[391] = true;
    BlockOnlyHitspot1[392] = true;

    BlockOnlyHitspot1[506] = true;
    BlockOnlyHitspot1[507] = true;
    BlockOnlyHitspot1[508] = true;

    BlockKills[371] = true;
    BlockKills[404] = true;
    BlockKills[406] = true;
    BlockKills[405] = true;
    BlockKills[420] = true;
    BlockHasNoMask[336] = true;
    BlockHasNoMask[337] = true;
    BlockHasNoMask[338] = true;
    BlockHasNoMask[339] = true;
    BlockHasNoMask[303] = true;
    BlockHasNoMask[304] = true;
    BlockHasNoMask[348] = true;
    BlockHasNoMask[353] = true;
    BlockHasNoMask[354] = true;
    BlockHasNoMask[355] = true;
    BlockHasNoMask[356] = true;
    BlockHasNoMask[3] = true;
    BlockHasNoMask[4] = true;
    BlockHasNoMask[13] = true;
    BlockHasNoMask[15] = true;
    BlockHasNoMask[16] = true;
    BlockHasNoMask[17] = true;
    BlockHasNoMask[19] = true;
    BlockHasNoMask[21] = true;
    BlockHasNoMask[34] = true;
    BlockHasNoMask[36] = true;
    BlockHasNoMask[23] = true;
    BlockHasNoMask[29] = true;
    BlockHasNoMask[40] = true;
    BlockHasNoMask[43] = true;
    BlockHasNoMask[46] = true;
    BlockHasNoMask[47] = true;
    BlockHasNoMask[48] = true;
    BlockHasNoMask[49] = true;
    BlockHasNoMask[50] = true;
    BlockHasNoMask[51] = true;
    BlockHasNoMask[52] = true;
    BlockHasNoMask[53] = true;
    BlockHasNoMask[54] = true;
    BlockHasNoMask[59] = true;
    BlockHasNoMask[60] = true;
    BlockHasNoMask[61] = true;
    BlockHasNoMask[63] = true;
    BlockHasNoMask[65] = true;
    BlockHasNoMask[67] = true;
    BlockHasNoMask[70] = true;
    BlockHasNoMask[71] = true;
    BlockHasNoMask[72] = true;
    BlockHasNoMask[73] = true;
    BlockHasNoMask[74] = true;
    BlockHasNoMask[75] = true;
    BlockHasNoMask[76] = true;
    BlockHasNoMask[81] = true;
    BlockHasNoMask[83] = true;
    BlockHasNoMask[84] = true;
    BlockHasNoMask[85] = true;
    BlockHasNoMask[86] = true;
    BlockHasNoMask[87] = true;
    BlockHasNoMask[91] = true;
    BlockHasNoMask[93] = true;
    BlockHasNoMask[94] = true;
    BlockHasNoMask[95] = true;
    BlockHasNoMask[96] = true;
    BlockHasNoMask[97] = true;
    BlockHasNoMask[98] = true;
    BlockHasNoMask[99] = true;
    BlockHasNoMask[100] = true;
    BlockHasNoMask[101] = true;
    BlockHasNoMask[102] = true;
    BlockHasNoMask[103] = true;
    BlockHasNoMask[111] = true;
    BlockHasNoMask[118] = true;
    BlockHasNoMask[119] = true;
    BlockHasNoMask[120] = true;
    BlockHasNoMask[121] = true;
    BlockHasNoMask[122] = true;
    BlockHasNoMask[123] = true;
    BlockHasNoMask[124] = true;
    BlockHasNoMask[125] = true;
    BlockHasNoMask[126] = true;
    BlockHasNoMask[127] = true;
    BlockHasNoMask[131] = true;
    BlockHasNoMask[134] = true;
    BlockHasNoMask[136] = true;
    BlockHasNoMask[159] = true;
    BlockHasNoMask[160] = true;
    BlockHasNoMask[166] = true;
    BlockHasNoMask[183] = true;
    BlockHasNoMask[184] = true;
    BlockHasNoMask[186] = true;
    BlockHasNoMask[187] = true;
    BlockHasNoMask[188] = true;
    BlockHasNoMask[190] = true;
    BlockHasNoMask[198] = true;
    BlockHasNoMask[199] = true;
    BlockHasNoMask[200] = true;
    BlockHasNoMask[201] = true;
    BlockHasNoMask[202] = true;
    BlockHasNoMask[203] = true;
    BlockHasNoMask[204] = true;
    BlockHasNoMask[205] = true;
    BlockHasNoMask[206] = true;
    BlockHasNoMask[216] = true;
    BlockHasNoMask[217] = true;
    BlockHasNoMask[218] = true;
    BlockHasNoMask[223] = true;
    BlockHasNoMask[226] = true;
    BlockHasNoMask[227] = true;
    BlockHasNoMask[228] = true;
    BlockHasNoMask[229] = true;
    BlockHasNoMask[230] = true;
    BlockHasNoMask[231] = true;
    BlockHasNoMask[232] = true;
    BlockHasNoMask[233] = true;
    BlockHasNoMask[234] = true;
    BlockHasNoMask[235] = true;
    BlockHasNoMask[236] = true;
    BlockHasNoMask[237] = true;
    BlockHasNoMask[238] = true;
    BlockHasNoMask[239] = true;
    BlockHasNoMask[250] = true;
    BlockHasNoMask[251] = true;
    BlockHasNoMask[252] = true;
    BlockHasNoMask[253] = true;
    BlockHasNoMask[254] = true;
    BlockHasNoMask[255] = true;
    BlockHasNoMask[256] = true;
    BlockHasNoMask[257] = true;
    BlockHasNoMask[258] = true;
    // BlockHasNoMask[261] = true; // invalid, Block 261 is sizable and this flag is never read for sizable blocks in SMBX 1.3
    BlockHasNoMask[262] = true;
    BlockHasNoMask[264] = true;
    BlockHasNoMask[263] = true;
    BlockHasNoMask[273] = true;
    BlockHasNoMask[272] = true;
    BlockHasNoMask[276] = true;
    BlockHasNoMask[281] = true;
    BlockHasNoMask[282] = true;
    BlockHasNoMask[283] = true;
    BlockHasNoMask[291] = true;
    BlockHasNoMask[292] = true;
    BlockHasNoMask[320] = true;
    BlockHasNoMask[322] = true;
    BlockHasNoMask[323] = true;
    BlockHasNoMask[330] = true;
    BlockHasNoMask[331] = true;
    BlockHasNoMask[369] = true;
    BlockHasNoMask[370] = true;
    BlockOnlyHitspot1[8] = true;
    BlockOnlyHitspot1[121] = true;
    BlockOnlyHitspot1[122] = true;
    BlockOnlyHitspot1[123] = true;
    BlockOnlyHitspot1[168] = true;
    BlockOnlyHitspot1[289] = true;
    BlockOnlyHitspot1[290] = true;
    BlockOnlyHitspot1[370] = true;

    BlockKills[30] = true;
    BlockHurts[109] = true;
    BlockHurts[598] = true;
    BlockHurts[110] = true;
    BlockHurts[267] = true;
    BlockHurts[268] = true;
    BlockHurts[269] = true;
    BlockHurts[511] = true;
    BlockPSwitch[4] = true;
    BlockPSwitch[60] = true;
    BlockPSwitch[89] = true;
    BlockPSwitch[188] = true;
    BlockPSwitch[280] = true;
    BlockPSwitch[293] = true;
    BlockNoClipping[172] = true;
    BlockNoClipping[175] = true;
    BlockNoClipping[178] = true;
    BlockNoClipping[181] = true;
    BlockSlope[299] = -1;
    BlockSlope[300] = 1;
    BlockSlope[301] = 1;
    BlockSlope[302] = -1;
    BlockSlope[305] = -1;
    BlockSlope[306] = -1;
    BlockSlope[307] = 1;
    BlockSlope[308] = 1;
    BlockSlope[324] = -1;
    BlockSlope[325] = 1;

    BlockSlope2[528] = 1;
    BlockSlope2[523] = -1;

    BlockSlope2[309] = 1;
    BlockSlope2[310] = -1;
    BlockSlope2[311] = 1;
    BlockSlope2[312] = 1;
    BlockSlope2[313] = -1;
    BlockSlope2[314] = -1;
    BlockSlope[315] = 1;
    BlockSlope[316] = -1;
    BlockSlope2[317] = 1;
    BlockSlope2[318] = -1;
    BlockSlope[319] = 1;
    BlockSlope[321] = -1;
    BlockSlope[326] = -1;
    BlockSlope[327] = 1;
    BlockSlope2[328] = -1;
    BlockSlope2[329] = 1;
    BlockSlope[332] = -1;
    BlockSlope[333] = 1;
    BlockSlope2[334] = -1;
    BlockSlope2[335] = 1;
    BlockSlope[340] = -1;
    BlockSlope[341] = -1;
    BlockSlope[342] = 1;
    BlockSlope[343] = 1;
    BlockSlope[357] = -1;
    BlockSlope[358] = -1;
    BlockSlope[359] = 1;
    BlockSlope[360] = 1;
    BlockSlope2[361] = 1;
    BlockSlope2[362] = 1;
    BlockSlope2[363] = -1;
    BlockSlope2[364] = -1;
    BlockSlope[365] = -1;
    BlockSlope[366] = 1;
    BlockSlope2[367] = -1;
    BlockSlope2[368] = 1;

    BlockKills[459] = true;

    BlockKills[460] = true;
    BlockKills[461] = true;
    BlockKills[462] = true;
    BlockKills[463] = true;
    BlockKills[464] = true;
    BlockKills[465] = true;
    BlockKills[466] = true;
    BlockKills[467] = true;
    BlockKills[468] = true;
    BlockKills[469] = true;
    BlockKills[470] = true;
    BlockKills[471] = true;

    BlockKills2[460] = true;
    BlockKills2[461] = true;
    BlockKills2[462] = true;
    BlockKills2[463] = true;
    BlockKills2[464] = true;
    BlockKills2[465] = true;
    BlockKills2[466] = true;
    BlockKills2[467] = true;
    BlockKills[472] = true;
    BlockKills[473] = true;
    BlockKills[474] = true;
    BlockKills[475] = true;
    BlockKills[476] = true;
    BlockKills[478] = true;
    BlockKills[479] = true;
    BlockKills2[472] = true;
    BlockKills2[474] = true;
    BlockKills2[476] = true;
    BlockKills2[479] = true;

    BlockKills[480] = true;
    BlockKills[481] = true;
    BlockKills[482] = true;
    BlockKills[483] = true;
    BlockKills[484] = true;
    BlockKills[485] = true;
    BlockKills[486] = true;
    BlockKills[487] = true;
    BlockKills2[480] = true;
    BlockKills2[482] = true;
    BlockKills2[485] = true;
    BlockKills2[486] = true;



    BlockHurts[407] = true;
    BlockHurts[408] = true;
    BlockHurts[428] = true;
    BlockHurts[429] = true;
    BlockHurts[430] = true;
    BlockHurts[431] = true;
    BlockHurts[511] = true;
    BlockOnlyHitspot1[447] = true;
    BlockOnlyHitspot1[446] = true;
    BlockOnlyHitspot1[448] = true;




    BlockWidth[604] = 64;
    BlockWidth[605] = 64;

    BlockSlope[600] = -1;
    BlockSlope[604] = -1;
    BlockSlope[601] = 1;
    BlockSlope[605] = 1;


    BlockSlope[451] = 1;
    BlockSlope[452] = -1;
    For(A, 459, 487)
    {
        BlockKills[A] = true;
    }
    BackgroundHasNoMask[187] = true;
    BackgroundHasNoMask[188] = true;
    BackgroundHasNoMask[189] = true;
    BackgroundHasNoMask[190] = true;

    BackgroundHasNoMask[172] = true;
    BackgroundHasNoMask[167] = true;
    BackgroundHasNoMask[164] = true;
    BackgroundHasNoMask[165] = true;
    BackgroundHasNoMask[158] = true;
    BackgroundHasNoMask[146] = true;
    BackgroundHasNoMask[12] = true;
    BackgroundHasNoMask[14] = true;
    BackgroundHasNoMask[15] = true;
    BackgroundHasNoMask[22] = true;
    BackgroundHasNoMask[30] = true;
    BackgroundHasNoMask[39] = true;
    BackgroundHasNoMask[40] = true;
    BackgroundHasNoMask[41] = true;
    BackgroundHasNoMask[42] = true;
    BackgroundHasNoMask[43] = true;
    BackgroundHasNoMask[44] = true;
    BackgroundHasNoMask[47] = true;
    BackgroundHasNoMask[52] = true;
    BackgroundHasNoMask[53] = true;
    BackgroundHasNoMask[55] = true;
    BackgroundHasNoMask[56] = true;
    BackgroundHasNoMask[60] = true;
    BackgroundHasNoMask[61] = true;
    BackgroundHasNoMask[64] = true;
    BackgroundHasNoMask[75] = true;
    BackgroundHasNoMask[76] = true;
    BackgroundHasNoMask[77] = true;
    BackgroundHasNoMask[78] = true;
    BackgroundHasNoMask[79] = true;
    BackgroundHasNoMask[83] = true;
    BackgroundHasNoMask[87] = true;
    BackgroundHasNoMask[88] = true;
    BackgroundHasNoMask[91] = true;
    BackgroundHasNoMask[98] = true;
    BackgroundHasNoMask[99] = true;
    BackgroundHasNoMask[107] = true;
    BackgroundHasNoMask[115] = true;
    BackgroundHasNoMask[116] = true;
    BackgroundHasNoMask[117] = true;
    BackgroundHasNoMask[118] = true;
    BackgroundHasNoMask[119] = true;
    BackgroundHasNoMask[122] = true;
    BackgroundHasNoMask[123] = true;
    BackgroundHasNoMask[124] = true;

    BackgroundHasNoMask[139] = true;
    BackgroundHasNoMask[140] = true;
    BackgroundHasNoMask[141] = true;
    BackgroundHasNoMask[144] = true;
    BackgroundHasNoMask[145] = true;
    BlockOnlyHitspot1[69] = true;


    Foreground[187] = true;
    Foreground[188] = true;
    Foreground[143] = true;
    // Foreground[165] = true;
    Foreground[145] = true;
    Foreground[23] = true;
    Foreground[24] = true;
    Foreground[25] = true;
    Foreground[45] = true;
    Foreground[46] = true;
    Foreground[49] = true;
    Foreground[50] = true;
    Foreground[51] = true;
    // Foreground[65] = true;
    Foreground[68] = true;
    Foreground[69] = true;
    Foreground[106] = true;
    Foreground[137] = true;
    Foreground[138] = true;

    Foreground[154] = true;
    Foreground[155] = true;
    Foreground[156] = true;
    Foreground[157] = true;

    BackgroundHeight[158] = 32;
    BackgroundHeight[159] = 32;

    BackgroundHeight[187] = 32;
    BackgroundHeight[188] = 32;
    BackgroundHeight[189] = 32;
    BackgroundHeight[190] = 32;



    BackgroundHeight[170] = 32;
    BackgroundHeight[171] = 32;

    BackgroundHeight[26] = 64;
    BackgroundHeight[18] = 32;
    BackgroundHeight[19] = 32;
    BackgroundHeight[20] = 32;
    BackgroundHeight[36] = 96;
    BackgroundHeight[65] = 96;

    BackgroundHeight[66] = 32;

    BackgroundHeight[68] = 64;
    BackgroundHeight[70] = 32;
    BackgroundHeight[172] = 32;
    BackgroundHeight[82] = 32;
    BackgroundHeight[100] = 32;
    BackgroundHeight[125] = 64;
    BackgroundHeight[134] = 32;
    BackgroundHeight[135] = 32;
    BackgroundHeight[136] = 32;
    BackgroundHeight[137] = 32;
    BackgroundHeight[138] = 32;
    BackgroundWidth[163] = 64;

    BackgroundWidth[173] = 112;
    BackgroundHeight[173] = 120;


    BackgroundWidth[160] = 24;
    BackgroundHeight[160] = 24;

    BackgroundWidth[161] = 64;
    BackgroundHeight[161] = 48;

    BackgroundHeight[168] = 32;
    BackgroundWidth[168] = 64;

    BackgroundHeight[169] = 64;
    BackgroundWidth[169] = 64;
    BackgroundHasNoMask[169] = true;



    For(A, 1, maxSceneType)
    {
        SceneWidth[A] = 32;
        SceneHeight[A] = 32;
        if((A >= 15 && A <= 18) || (A == 21) || (A == 24) || (A == 58) || (A == 59) || (A == 63))
        {
            SceneWidth[A] = 16;
            SceneHeight[A] = 16;
        }
    }
    SceneWidth[20] = 64;
    SceneHeight[20] = 64;
    SceneWidth[27] = 48;
    SceneHeight[27] = 16;
    SceneWidth[28] = 48;
    SceneHeight[28] = 16;
    SceneWidth[29] = 64;
    SceneHeight[29] = 16;
    SceneWidth[30] = 64;
    SceneHeight[30] = 16;
    SceneWidth[33] = 14;
    SceneHeight[33] = 14;
    SceneWidth[34] = 14;
    SceneHeight[34] = 14;
    SceneWidth[44] = 64;
    SceneHeight[50] = 48;
    SceneWidth[50] = 64;
    SceneWidth[54] = 30;
    SceneHeight[54] = 24;
    SceneWidth[55] = 30;
    SceneHeight[55] = 24;
    SceneWidth[57] = 64;
    SceneHeight[57] = 64;
    SceneWidth[60] = 48;
    SceneHeight[60] = 48;
    SceneWidth[61] = 64;
    SceneHeight[61] = 76;

    For(A, 1, maxTileType)
    {
        TileWidth[A] = 32;
        TileHeight[A] = 32;
    }
    TileWidth[8] = 64;
    TileHeight[8] = 64;
    TileWidth[9] = 96;
    TileHeight[9] = 96;
    TileWidth[12] = 64;
    TileHeight[12] = 64;
    TileWidth[13] = 96;
    TileHeight[13] = 96;
    TileWidth[27] = 128;
    TileHeight[27] = 128;
    TileWidth[325] = 64;
    TileHeight[325] = 64;
    Points[1] = 10;
    Points[2] = 100;
    Points[3] = 200;
    Points[4] = 400;
    Points[5] = 800;
    Points[6] = 1000;
    Points[7] = 2000;
    Points[8] = 4000;
    Points[9] = 8000;
    Points[10] = 1;
    Points[11] = 2;
    Points[12] = 3;
    Points[13] = 5;
    For(A, 1, maxNPCType)
    {
        if(NPCTraits[A].IsFish)
            NPCTraits[A].Foreground = true;
    }
    For(A, 1, maxEffectType)
    {
#ifndef LOW_MEM
        // kept only for debugging that it's safe to remove them
        EffectDefaults.EffectHeight[A] = EffectHeight[A];
        EffectDefaults.EffectWidth[A] = EffectWidth[A];
#endif // #ifndef LOW_MEM

        if(EffectHeight[A] > 0)
        {
            // EffectDefaults.EffectFrames[A] = vb6Round(double(GFXEffectBMP[A].h) / EffectHeight[A]);
            EffectDefaults.EffectFrames[A] = (GFXEffectBMP[A].h * 2 + EffectHeight[A]) / (EffectHeight[A] * 2);
            if(EffectDefaults.EffectFrames[A] <= 0)
                EffectDefaults.EffectFrames[A] = 1;
        }
        else
            EffectDefaults.EffectFrames[A] = 1;
    }
    SaveNPCDefaults();
    SavePlayerDefaults();
}
