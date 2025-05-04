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

#pragma once

#ifndef NPC_CONSTANT_TRAITS_H
#define NPC_CONSTANT_TRAITS_H

#include "globals.h"

// force-inline these definitions because they are often used in switch blocks depending only on NPC_t::Type
// inlining allows the compiler to seek arithmetic patterns and to know that it is safe to reorder cases if desired
// force-inlining should be used extremely sparingly but this only causes a 3kb codesize increase across the full application
#if (defined(__GNUC__) || defined(__llvm__))
#    define XT_FORCE_INLINE __attribute__((always_inline))
#else
#    define XT_FORCE_INLINE
#endif

// constant traits that determine NPC behavior based on Type
// will be replaced with other mechanisms during the NPC function pointer update

#if 0
// Should turn into trait.
//'Flags the NPC type if it is a shell
constexpr bool NPCIsAShell(int Type)
{
    return (
        Type == 237 ||
        Type == 113 ||
        Type == 114 ||
        Type == 115 ||
        Type == 116 ||
        Type == 172 ||
        Type == 174 ||
        Type == 195 ||
        Type == 5 ||
        Type == 7 ||
        Type == 24 ||
        Type == 73
    );
}

constexpr bool NPCIsAShell(const NPC_t& n)
{
    return NPCIsAShell(n.Type);
}

// Should turn into trait.
//'Flags the NPC type if it is a bonus
constexpr bool NPCIsABonus(int Type)
{
    return (
        Type == 254 ||
        Type == 251 ||
        Type == 252 ||
        Type == 253 ||
        Type == 250 ||
        Type == 240 ||//32;
        Type == 248 ||//32;
        Type == 249 ||
        Type == 274 ||
        Type == 138 ||
        Type == 152 ||
        Type == 169 ||
        Type == 170 ||
        Type == 197 ||
        Type == 182 || // SMB1 Flower
        Type == 183 || // SMW Flower
        Type == 184 || // SMB1 Mushroom
        Type == 185 || // SMW Mushroom
        Type == 186 || // SMB1 1-up
        Type == 187 || // SMW 1-up
        Type == 188 || // SMW 3 up
        Type == 192 ||
        Type == 178 ||
        Type == 9 ||
        Type == 273 ||
        Type == 10 ||
        Type == 11 ||
        Type == 14 ||
        Type == 264 ||
        Type == 277 ||
        Type == 16 ||
        Type == 33 ||
        Type == 258 ||
        Type == 34 ||
        Type == 41 ||
        Type == 75 ||
        Type == 88 ||
        Type == 90 ||
        Type == 94 ||
        Type == 198 ||
        Type == 97 ||
        Type == 101 ||
        Type == 102 ||
        Type == 103 ||
        Type == 107 ||
        Type == 153 ||
        Type == 196
    );
}

constexpr bool NPCIsABonus(const NPC_t& n)
{
    return NPCIsABonus(n.Type);
}

// Should turn into trait.
//'Flags the NPC type if it is a coin
constexpr bool NPCIsACoin(int Type)
{
    return (
        Type == 251 ||
        Type == 252 ||
        Type == 253 ||
        Type == 274 ||
        Type == 138 ||
        Type == 152 ||
        Type == 10 ||
        Type == 33 ||
        Type == 258 ||
        Type == 88 ||
        Type == 103
    );
}

constexpr bool NPCIsACoin(const NPC_t& n)
{
    return NPCIsACoin(n.Type);
}

// Should turn into trait.
//'Flags the NPC type if it is a vine
constexpr bool NPCIsAVine(int Type)
{
    return (Type >= 213 && Type <= 224);
}

constexpr bool NPCIsAVine(const NPC_t& n)
{
    return NPCIsAVine(n.Type);
}
#endif

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'Flags the NPC type if it is a level exit
XT_FORCE_INLINE constexpr bool NPCIsAnExit(NPCID Type)
{
    return (
        Type == NPCID_ITEMGOAL ||
        Type == NPCID_GOALORB_S3 ||
        Type == NPCID_GOALORB_S2 ||
        Type == NPCID_STAR_EXIT ||
        Type == NPCID_STAR_COLLECT ||
        Type == NPCID_FLAG_EXIT
    );
}

XT_FORCE_INLINE constexpr bool NPCIsAnExit(const NPC_t& n)
{
    return NPCIsAnExit(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages in UpdatePlayer may become an OnPlayerCollide NPC method.
//'Flags the NPC type as a para-troopa
XT_FORCE_INLINE constexpr bool NPCIsAParaTroopa(NPCID Type)
{
    return (
        Type == NPCID_FLY_FODDER_S3 ||
        Type == NPCID_FLY_FODDER_S5 ||
        Type == NPCID_GRN_FLY_TURTLE_S3 ||
        Type == NPCID_RED_FLY_TURTLE_S3 ||
        Type == NPCID_GRN_FLY_TURTLE_S4 ||
        Type == NPCID_RED_FLY_TURTLE_S4 ||
        Type == NPCID_BLU_FLY_TURTLE_S4 ||
        Type == NPCID_YEL_FLY_TURTLE_S4 ||
        Type == NPCID_GRN_FLY_TURTLE_S1 ||
        Type == NPCID_RED_FLY_TURTLE_S1
    );
}

XT_FORCE_INLINE constexpr bool NPCIsAParaTroopa(const NPC_t& n)
{
    return NPCIsAParaTroopa(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages in UpdatePlayer may become an OnPlayerCollide NPC method, usages in Blocks should become a new flag.
//'npc is a kurbo's shoe
XT_FORCE_INLINE constexpr bool NPCIsBoot(NPCID Type)
{
    return (Type == NPCID_GRN_BOOT || Type == NPCID_RED_BOOT || Type == NPCID_BLU_BOOT);
}

XT_FORCE_INLINE constexpr bool NPCIsBoot(const NPC_t& n)
{
    return NPCIsBoot(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages in UpdatePlayer may become an OnPlayerCollide NPC method, usages in Blocks should become a new flag.
//'npc is a yoshi
XT_FORCE_INLINE constexpr bool NPCIsYoshi(NPCID Type)
{
    return (
        Type == NPCID_PET_GREEN ||
        Type == NPCID_PET_BLUE ||
        Type == NPCID_PET_YELLOW ||
        Type == NPCID_PET_RED ||
        Type == NPCID_PET_BLACK ||
        Type == NPCID_PET_PURPLE ||
        Type == NPCID_PET_PINK ||
        Type == NPCID_PET_CYAN
    );
}

XT_FORCE_INLINE constexpr bool NPCIsYoshi(const NPC_t& n)
{
    return NPCIsYoshi(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'npc is a toad
XT_FORCE_INLINE constexpr bool NPCIsToad(NPCID Type)
{
    return (
        Type == NPCID_CIVILIAN_SCARED ||
        Type == NPCID_CIVILIAN ||
        Type == NPCID_CHAR3 ||
        Type == NPCID_CHAR2 ||
        Type == NPCID_CHAR5 ||
        Type == NPCID_PINK_CIVILIAN
    );
}

XT_FORCE_INLINE constexpr bool NPCIsToad(const NPC_t& n)
{
    return NPCIsToad(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'Zelda 2 Bot monster
XT_FORCE_INLINE constexpr bool NPCIsABot(NPCID Type)
{
    return (Type == NPCID_BLU_SLIME || Type == NPCID_CYAN_SLIME || Type == NPCID_RED_SLIME);
}

XT_FORCE_INLINE constexpr bool NPCIsABot(const NPC_t& n)
{
    return NPCIsABot(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Unused outside of NPC methods.
//'default NPC movement
XT_FORCE_INLINE constexpr bool NPCDefaultMovement(NPCID Type)
{
    return (
        Type == NPCID_FODDER_S5 ||
        Type == NPCID_GRN_TURTLE_S4 ||
        Type == NPCID_RED_TURTLE_S4 ||
        Type == NPCID_BLU_TURTLE_S4 ||
        Type == NPCID_YEL_TURTLE_S4 ||
        Type == NPCID_GRN_HIT_TURTLE_S4 ||
        Type == NPCID_RED_HIT_TURTLE_S4 ||
        Type == NPCID_BLU_HIT_TURTLE_S4 ||
        Type == NPCID_YEL_HIT_TURTLE_S4 ||
        Type == NPCID_KNIGHT ||
        Type == NPCID_BIRD ||
        Type == NPCID_RED_SPIT_GUY ||
        Type == NPCID_BLU_SPIT_GUY ||
        Type == NPCID_GRY_SPIT_GUY ||
        Type == NPCID_WALK_BOMB_S2 ||
        Type == NPCID_WALK_BOMB_S3 ||
        Type == NPCID_SKELETON ||
        Type == NPCID_GRN_TURTLE_S1 ||
        Type == NPCID_RED_TURTLE_S1 ||
        Type == NPCID_BRUTE ||
        Type == NPCID_CARRY_FODDER ||
        Type == NPCID_FLY_CARRY_FODDER ||
        Type == NPCID_ROCKET_WOOD ||
        Type == NPCID_FODDER_S3 ||
        Type == NPCID_RED_FODDER ||
        Type == NPCID_RED_FLY_FODDER ||
        Type == NPCID_GRN_TURTLE_S3 ||
        Type == NPCID_RED_TURTLE_S3 ||
        Type == NPCID_BLU_GUY ||
        Type == NPCID_RED_GUY ||
        Type == NPCID_STACKER ||
        Type == NPCID_GLASS_TURTLE ||
        Type == NPCID_UNDER_FODDER ||
        Type == NPCID_SPIKY_S3 ||
        Type == NPCID_SPIKY_S4 ||
        Type == NPCID_TOOTHY ||
        Type == NPCID_CRAB ||
        Type == NPCID_EXT_TURTLE ||
        Type == NPCID_YELSWITCH_FODDER ||
        Type == NPCID_BLUSWITCH_FODDER ||
        Type == NPCID_GRNSWITCH_FODDER ||
        Type == NPCID_REDSWITCH_FODDER ||
        Type == NPCID_BIG_FODDER ||
        Type == NPCID_BIG_TURTLE ||
        Type == NPCID_JUMPER_S4 ||
        Type == NPCID_TANK_TREADS ||
        Type == NPCID_FODDER_S1 ||
        Type == NPCID_ITEM_BURIED ||
        Type == NPCID_PINK_CIVILIAN ||
        Type == NPCID_CARRY_BUDDY
    );
}

XT_FORCE_INLINE constexpr bool NPCDefaultMovement(const NPC_t& n)
{
    return NPCDefaultMovement(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'turnips
XT_FORCE_INLINE constexpr bool NPCIsVeggie(NPCID Type)
{
    return Type == NPCID_VEGGIE_1 || (Type >= NPCID_VEGGIE_2 && Type <= NPCID_VEGGIE_RANDOM);
}

XT_FORCE_INLINE constexpr bool NPCIsVeggie(const NPC_t& n)
{
    return NPCIsVeggie(n.Type);
}

// factored out of block hit / UpdateGraphics code
constexpr inline bool NPCLongLife(NPCID Type)
{
    return (NPCIsYoshi(Type) || NPCIsBoot(Type) || Type == NPCID_POWER_S3
        || Type == NPCID_FIRE_POWER_S3 || Type == NPCID_CANNONITEM || Type == NPCID_LIFE_S3
        || Type == NPCID_POISON || Type == NPCID_STATUE_POWER || Type == NPCID_HEAVY_POWER || Type == NPCID_FIRE_POWER_S1
        || Type == NPCID_FIRE_POWER_S4 || Type == NPCID_POWER_S1 || Type == NPCID_POWER_S4
        || Type == NPCID_LIFE_S1 || Type == NPCID_LIFE_S4 || Type == NPCID_3_LIFE || Type == NPCID_FLIPPED_RAINBOW_SHELL
        // || Type == NPCID_PLATFORM_S3 // exclusive to UpdateGraphics
        // TheXTech-exclusive Types with long lives
        || Type == NPCID_INVINCIBILITY_POWER || Type == NPCID_AQUATIC_POWER
        || Type == NPCID_POLAR_POWER || Type == NPCID_CYCLONE_POWER || Type == NPCID_SHELL_POWER);
}

#endif // #ifndef NPC_CONSTANT_TRAITS_H
