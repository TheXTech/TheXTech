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

#pragma once

#ifndef ENUMNPCID_HHH
#define ENUMNPCID_HHH

enum NPCID
{
    NPCID_FODDER_S3 = 1,
    NPCID_RED_FODDER = 2,
    NPCID_RED_FLY_FODDER = 3,
    NPCID_GRN_TURTLE_S3 = 4,
    NPCID_GRN_SHELL_S3 = 5,
    NPCID_RED_TURTLE_S3 = 6,
    NPCID_RED_SHELL_S3 = 7,
    NPCID_PLANT_S3 = 8,
    NPCID_POWER_S3 = 9,
    NPCID_COIN_S3 = 10,

    NPCID_ITEMGOAL = 11,
    NPCID_LAVABUBBLE = 12,
    NPCID_PLR_FIREBALL = 13,
    NPCID_FIRE_POWER_S3 = 14,
    NPCID_MINIBOSS = 15,
    NPCID_GOALORB_S3 = 16,
    NPCID_BULLET = 17,
    NPCID_BIG_BULLET = 18,
    NPCID_BLU_GUY = 19,
    NPCID_RED_GUY = 20,

    NPCID_CANNONENEMY = 21,
    NPCID_CANNONITEM = 22,
    NPCID_GLASS_TURTLE = 23,
    NPCID_GLASS_SHELL = 24,
    NPCID_JUMPER_S3 = 25,
    NPCID_SPRING = 26,
    NPCID_UNDER_FODDER = 27,
    NPCID_RED_FISH_S1 = 28,
    NPC_HEAVY_THROWER = 29,
    NPC_HEAVY_THROWN = 30,

    NPCID_KEY = 31,
    NPCID_COIN_SWITCH = 32,
    NPCID_COIN_S4 = 33,
    NPCID_LEAF_POWER = 34,
    NPCID_GRN_BOOT = 35,
    NPCID_SPIKY_S3 = 36,
    NPCID_STONE_S3 = 37,
    NPCID_GHOST_S3 = 38,
    NPCID_SPIT_BOSS = 39,
    NPCID_SPIT_BOSS_BALL = 40,

    NPCID_GOALORB_S2 = 41,
    NPCID_GHOST_FAST = 42,
    NPCID_GHOST_S4 = 43,
    NPCID_BIG_GHOST = 44,
    NPCID_SLIDE_BLOCK = 45,
    NPCID_FALL_BLOCK_RED = 46,
    NPCID_SPIKY_THROWER = 47,
    NPCID_SPIKY_BALL_S3 = 48,
    NPCID_TOOTHYPIPE = 49,
    NPCID_TOOTHY = 50,

    NPCID_BOTTOM_PLANT = 51,
    NPCID_SIDE_PLANT = 52,
    NPCID_CRAB = 53,
    NPCID_FLY = 54,
    NPCID_EXT_TURTLE = 55,
    NPCID_VEHICLE = 56,
    NPCID_CONVEYOR = 57,
    NPCID_METALBARREL = 58,

    NPCID_YELSWITCH_FODDER = 59,
    NPCID_YEL_PLATFORM = 60,
    NPCID_BLUSWITCH_FODDER = 61,
    NPCID_BLU_PLATFORM = 62,
    NPCID_GRNSWITCH_FODDER = 63,
    NPCID_GRN_PLATFORM = 64,
    NPCID_REDSWITCH_FODDER = 65,
    NPCID_RED_PLATFORM = 66,

    NPC_HPIPE_SHORT = 67,
    NPC_HPIPE_LONG = 68,
    NPCID_VPIPE_SHORT = 69,
    NPCID_VPIPE_LONG = 70,

    NPCID_BIG_FODDER = 71,
    NPCID_BIG_TURTLE = 72,
    NPCID_BIG_SHELL = 73,
    NPCID_BIG_PLANT = 74,
    NPCID_CIVILIAN_SCARED = 75,
    NPCID_GRN_FLY_TURTLE_S3 = 76,
    NPCID_JUMPER_S4 = 77,
    NPCID_TANK_TREADS = 78,
    NPCID_SHORT_WOOD = 79,
    NPCID_LONG_WOOD = 80,

    NPCID_SLANT_WOOD_L = 81,
    NPCID_SLANT_WOOD_R = 82,
    NPCID_SLANT_WOOD_M = 83,
    NPCID_STATUE_S3 = 84,
    NPCID_STATUE_FIRE = 85,
    NPCID_VILLAIN_S3 = 86,
    NPCID_VILLAIN_FIRE = 87,
    NPCID_COIN_S1 = 88,
    NPCID_FODDER_S1 = 89,
    NPCID_LIFE_S3 = 90,

    NPCID_ITEM_BURIED = 91,
    NPCID_VEGGIE_1 = 92,
    NPCID_PLANT_S1 = 93,
    NPCID_CIVILIAN = 94,
    NPCID_PET_GREEN = 95,
    NPCID_ITEM_POD = 96,
    NPCID_STAR_EXIT = 97,
    NPCID_PET_BLUE = 98,
    NPCID_PET_YELLOW = 99,
    NPCID_PET_RED = 100,

    NPCID_CHAR2 = 101,
    NPCID_CHAR5 = 102,
    NPCID_RED_COIN = 103,
    NPCID_PLATFORM_S3 = 104,
    NPCID_CHECKER_PLATFORM = 105,
    NPCID_PLATFORM_S1 = 106,
    NPCID_PINK_CIVILIAN = 107,
    NPCID_PET_FIRE = 108,

    NPCID_GRN_TURTLE_S4 = 109,
    NPCID_RED_TURTLE_S4 = 110,
    NPCID_BLU_TURTLE_S4 = 111,
    NPCID_YEL_TURTLE_S4 = 112,

    NPCID_GRN_SHELL_S4 = 113,
    NPCID_RED_SHELL_S4 = 114,
    NPCID_BLU_SHELL_S4 = 115,
    NPCID_YEL_SHELL_S4 = 116,

    NPCID_GRN_HIT_TURTLE_S4 = 117,
    NPCID_RED_HIT_TURTLE_S4 = 118,
    NPCID_BLU_HIT_TURTLE_S4 = 119,
    NPCID_YEL_HIT_TURTLE_S4 = 120,

    NPCID_GRN_FLY_TURTLE_S4 = 121,
    NPCID_RED_FLY_TURTLE_S4 = 122,
    NPCID_BLU_FLY_TURTLE_S4 = 123,
    NPCID_YEL_FLY_TURTLE_S4 = 124,

    NPCID_KNIGHT = 125,
    NPCID_BLU_SLIME = 126,
    NPCID_CYAN_SLIME = 127,
    NPCID_RED_SLIME = 128,

    NPCID_BIRD = 129,
    NPCID_RED_SPIT_GUY = 130,
    NPCID_BLU_SPIT_GUY = 131,
    NPCID_GRY_SPIT_GUY = 132,

    NPCID_SPIT_GUY_BALL = 133,
    NPCID_BOMB = 134,

    NPCID_WALK_BOMB_S2 = 135,
    NPCID_WALK_BOMB_S3 = 136,
    NPCID_LIT_BOMB_S3 = 137,
    NPCID_COIN_S2 = 138,

    NPCID_VEGGIE_2 = 139,
    NPCID_VEGGIE_3 = 140,
    NPCID_VEGGIE_4 = 141,
    NPCID_VEGGIE_5 = 142,
    NPCID_VEGGIE_6 = 143,
    NPCID_VEGGIE_7 = 144,
    NPCID_VEGGIE_8 = 145,
    NPCID_VEGGIE_9 = 146,
    NPCID_VEGGIE_RANDOM = 147,

    NPCID_PET_BLACK = 148,
    NPCID_PET_PURPLE = 149,
    NPCID_PET_PINK = 150,

    NPCID_SIGN = 151,
    NPCID_RING = 152,
    NPCID_POISON = 153,
    NPCID_CARRY_BLOCK_A = 154,
    NPCID_CARRY_BLOCK_B = 155,
    NPCID_CARRY_BLOCK_C = 156,
    NPCID_CARRY_BLOCK_D = 157,
    NPCID_SATURN = 158,
    NPCID_LIFT_SAND = 159,
    NPCID_ROCKET_WOOD = 160,

    NPCID_RED_FLY_TURTLE_S3 = 161,
    NPCID_BRUTE = 162,
    NPCID_BRUTE_SQUISHED = 163,
    NPCID_BIG_MOLE = 164,
    NPCID_CARRY_FODDER = 165,
    NPC_HIT_CARRY_FODDER = 166,
    NPCID_FLY_CARRY_FODDER = 167,
    NPCID_BULLY = 168,

    NPCID_STATUE_POWER = 169,
    NPC_HEAVY_POWER = 170,
    NPCID_PLR_HEAVY = 171,

    NPCID_GRN_SHELL_S1 = 172,
    NPCID_GRN_TURTLE_S1 = 173,
    NPCID_RED_SHELL_S1 = 174,
    NPCID_RED_TURTLE_S1 = 175,
    NPCID_GRN_FLY_TURTLE_S1 = 176,
    NPCID_RED_FLY_TURTLE_S1 = 177,

    NPCID_AXE = 178,
    NPCID_SAW = 179,

    NPCID_STONE_S4 = 180,

    NPCID_STATUE_S4 = 181,
    NPCID_FIRE_POWER_S1 = 182,
    NPCID_FIRE_POWER_S4 = 183,
    NPCID_POWER_S1 = 184,
    NPCID_POWER_S4 = 185,
    NPCID_LIFE_S1 = 186,
    NPCID_LIFE_S4 = 187,
    NPCID_3_LIFE = 188,
    NPCID_SKELETON = 189,
    NPCID_RAFT = 190,

    NPCID_RED_BOOT = 191,
    NPCID_CHECKPOINT = 192,
    NPCID_BLU_BOOT = 193,
    NPCID_RAINBOW_SHELL = 194,
    NPCID_FLIPPED_RAINBOW_SHELL = 195,
    NPCID_STAR_COLLECT = 196,
    NPCID_GOALTAPE = 197,
    NPCID_CHAR3 = 198,
    NPCID_LAVA_MONSTER = 199,
    NPCID_VILLAIN_S1 = 200,

    NPCID_SICK_BOSS = 201,
    NPCID_SICK_BOSS_BALL = 202,

    NPCID_FLIER = 203,
    NPCID_ROCKET_FLIER = 204,
    NPCID_WALL_BUG = 205,
    NPCID_WALL_SPARK = 206,
    NPCID_WALL_TURTLE = 207,
    NPCID_BOSS_CASE = 208,
    NPCID_BOSS_FRAGILE = 209,
    NPC_HOMING_BALL = 210,

    NPC_HOMING_BALL_GEN = 211,
    NPCID_FALL_BLOCK_BROWN = 212,
    NPCID_GRN_VINE_S3 = 213,
    NPCID_RED_VINE_S3 = 214,
    NPCID_GRN_VINE_S2 = 215,
    NPCID_YEL_VINE_S2 = 216,
    NPCID_BLU_VINE_S2 = 217,
    NPCID_GRN_VINE_BASE_S2 = 218,
    NPCID_YEL_VINE_BASE_S2 = 219,
    NPCID_BLU_VINE_BASE_S2 = 220,

    NPCID_LADDER = 221,
    NPCID_GRN_VINE_S1 = 222,
    NPCID_GRN_VINE_TOP_S1 = 223,
    NPCID_GRN_VINE_S4 = 224,
    NPCID_RED_VINE_TOP_S3 = 225,
    NPCID_GRN_VINE_TOP_S3 = 226,
    NPCID_GRN_VINE_TOP_S4 = 227,
    NPCID_PET_CYAN = 228,
    NPCID_GRN_FISH_S3 = 229,
    NPCID_RED_FISH_S3 = 230,

    NPCID_SQUID_S3 = 231,
    NPCID_GOGGLE_FISH = 232,
    NPCID_GRN_FISH_S1 = 233,
    NPCID_BONE_FISH = 234,
    NPCID_SQUID_S1 = 235,
    NPCID_FISH_S4 = 236,
    NPCID_ICE_BLOCK = 237,
    NPCID_TIME_SWITCH = 238,
    NPCID_TNT = 239,
    NPCID_TIMER_S2 = 240,

    NPCID_EARTHQUAKE_BLOCK = 241,
    NPCID_FODDER_S5 = 242,
    NPCID_FLY_FODDER_S5 = 243,
    NPCID_FLY_FODDER_S3 = 244,
    NPCID_FIRE_PLANT = 245,
    NPCID_PLANT_FIRE = 246,
    NPCID_STACKER = 247,
    NPCID_TIMER_S3 = 248,
    NPCID_POWER_S2 = 249,
    NPCID_POWER_S5 = 250,

    NPCID_GEM_1 = 251,
    NPCID_GEM_5 = 252,
    NPCID_GEM_20 = 253,
    NPCID_FLY_POWER = 254,
    NPCID_LOCK_DOOR = 255,
    NPCID_LONG_PLANT_UP = 256,
    NPCID_LONG_PLANT_DOWN = 257,
    NPCID_COIN_5 = 258,
    NPCID_FIRE_DISK = 259,
    NPCID_FIRE_CHAIN = 260,

    NPCID_WALK_PLANT = 261,
    NPCID_BOMBER_BOSS = 262,
    NPCID_ICE_CUBE = 263,
    NPCID_ICE_POWER_S3 = 264,
    NPCID_PLR_ICEBALL = 265,
    NPCID_SWORDBEAM = 266,
    NPCID_MAGIC_BOSS = 267,
    NPCID_MAGIC_BOSS_SHELL = 268,
    NPCID_MAGIC_BOSS_BALL = 269,
    NPCID_JUMP_PLANT = 270,

    NPCID_BAT = 271,
    NPCID_VINE_BUG = 272,
    NPCID_SWAP_POWER = 273,
    NPCID_MEDAL = 274,
    NPCID_QUAD_SPITTER = 275,
    NPCID_QUAD_BALL = 276,
    NPCID_ICE_POWER_S4 = 277,
    NPCID_FLY_BLOCK = 278,
    NPCID_FLY_CANNON = 279,

    NPCID_FIRE_BOSS = 280,
    NPCID_FIRE_BOSS_SHELL = 281,
    NPCID_FIRE_BOSS_FIRE = 282,
    NPCID_ITEM_BUBBLE = 283,
    NPCID_ITEM_THROWER = 284,
    NPCID_SPIKY_S4 = 285,
    NPCID_SPIKY_BALL_S4 = 286,
    NPCID_RANDOM_POWER = 287,
    NPCID_DOOR_MAKER = 288,
    NPCID_MAGIC_DOOR = 289,
    NPCID_COCKPIT = 290,

    NPCID_CHAR3_HEAVY = 291,
    NPCID_CHAR4_HEAVY = 292,
};


#endif // ENUMNPCID_HHH
