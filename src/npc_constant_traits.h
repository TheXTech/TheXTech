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

#pragma once

#ifndef NPC_CONSTANT_TRAITS_H
#define NPC_CONSTANT_TRAITS_H

#include "globals.h"


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
constexpr bool NPCIsAnExit(int Type)
{
    return (
        Type == 11 ||
        Type == 16 ||
        Type == 41 ||
        Type == 97 ||
        Type == 196
    );
}

constexpr bool NPCIsAnExit(const NPC_t& n)
{
    return NPCIsAnExit(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages in UpdatePlayer may become an OnPlayerCollide NPC method.
//'Flags the NPC type as a para-troopa
constexpr bool NPCIsAParaTroopa(int Type)
{
    return (
        Type == 244 ||
        Type == 243 ||
        Type == 76 ||
        Type == 161 ||
        Type == 121 ||
        Type == 122 ||
        Type == 123 ||
        Type == 124 ||
        Type == 176 ||
        Type == 177
    );
}

constexpr bool NPCIsAParaTroopa(const NPC_t& n)
{
    return NPCIsAParaTroopa(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages in UpdatePlayer may become an OnPlayerCollide NPC method, usages in Blocks should become a new flag.
//'npc is a kurbo's shoe
constexpr bool NPCIsBoot(int Type)
{
    return (Type == 35 || Type == 191 || Type == 193);
}

constexpr bool NPCIsBoot(const NPC_t& n)
{
    return NPCIsBoot(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages in UpdatePlayer may become an OnPlayerCollide NPC method, usages in Blocks should become a new flag.
//'npc is a yoshi
constexpr bool NPCIsYoshi(int Type)
{
    return (
        Type == 95 ||
        Type == 98 ||
        Type == 99 ||
        Type == 100 ||
        Type == 148 ||
        Type == 149 ||
        Type == 150 ||
        Type == 228
    );
}

constexpr bool NPCIsYoshi(const NPC_t& n)
{
    return NPCIsYoshi(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'npc is a toad
constexpr bool NPCIsToad(int Type)
{
    return (
        Type == 75 ||
        Type == 94 ||
        Type == 198 ||
        Type == 101 ||
        Type == 102 ||
        Type == 107
    );
}

constexpr bool NPCIsToad(const NPC_t& n)
{
    return NPCIsToad(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'Zelda 2 Bot monster
constexpr bool NPCIsABot(int Type)
{
    return (Type == 126 || Type == 127 || Type == 128);
}

constexpr bool NPCIsABot(const NPC_t& n)
{
    return NPCIsABot(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Unused outside of NPC methods.
//'default NPC movement
constexpr bool NPCDefaultMovement(int Type)
{
    return (
        Type == 242 ||
        Type == 109 ||
        Type == 110 ||
        Type == 111 ||
        Type == 112 ||
        Type == 117 ||
        Type == 118 ||
        Type == 119 ||
        Type == 120 ||
        Type == 125 ||
        Type == 129 ||
        Type == 130 ||
        Type == 131 ||
        Type == 132 ||
        Type == 135 ||
        Type == 136 ||
        Type == 189 ||
        Type == 173 ||
        Type == 175 ||
        Type == 162 ||
        Type == 165 ||
        Type == 167 ||
        Type == 160 ||
        Type == 1 ||
        Type == 2 ||
        Type == 3 ||
        Type == 4 ||
        Type == 6 ||
        Type == 19 ||
        Type == 20 ||
        Type == 247 ||
        Type == 23 ||
        Type == 27 ||
        Type == 36 ||
        Type == 285 ||
        Type == 50 ||
        Type == 53 ||
        Type == 55 ||
        Type == 59 ||
        Type == 61 ||
        Type == 63 ||
        Type == 65 ||
        Type == 71 ||
        Type == 72 ||
        Type == 77 ||
        Type == 78 ||
        Type == 89 ||
        Type == 91 ||
        Type == 107 ||
        Type == 158
    );
}

constexpr bool NPCDefaultMovement(const NPC_t& n)
{
    return NPCDefaultMovement(n.Type);
}

// OKAY TO KEEP AS FUNCTION. Usages outside of NPC methods should be replaced with new usage-specific NPC traits.
//'turnips
constexpr bool NPCIsVeggie(int Type)
{
    return Type == 92 || (Type >= 139 && Type <= 147);
}

constexpr bool NPCIsVeggie(const NPC_t& n)
{
    return NPCIsVeggie(n.Type);
}

#endif // #ifndef NPC_CONSTANT_TRAITS_H
