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

#ifndef NPC_TRAITS_H
#define NPC_TRAITS_H

#include <cstdint>

#include "range_arr.hpp"

#include "globals.h"
#include "npc_constant_traits.h"

struct NPCTraits_t
{
    //Public NPCFrameOffsetX(0 To maxNPCType) As Integer 'NPC frame offset X
    int16_t FrameOffsetX = 0;
    //Public NPCFrameOffsetY(0 To maxNPCType) As Integer 'NPC frame offset Y
    int16_t FrameOffsetY = 0;
    //Public NPCWidth(0 To maxNPCType) As Integer 'NPC width
    int16_t Width = 0;
    //Public NPCHeight(0 To maxNPCType) As Integer 'NPC height
    int16_t Height = 0;
    //Public NPCWidthGFX(0 To maxNPCType) As Integer 'NPC gfx width
    int16_t WidthGFX = 0;
    //Public NPCHeightGFX(0 To maxNPCType) As Integer 'NPC gfx height
    int16_t HeightGFX = 0;
    //Public NPCSpeedvar(0 To maxNPCType) As Single 'NPC Speed Change
    float Speedvar = 0.0;

    //Public NPCIsAShell(0 To maxNPCType) As Boolean 'Flags the NPC type if it is a shell
    bool IsAShell = false;
    //Public NPCIsABlock(0 To maxNPCType) As Boolean 'Flag NPC as a block
    bool IsABlock = false;
    //Public NPCIsAHit1Block(0 To maxNPCType) As Boolean 'Flag NPC as a hit1 block
    bool IsAHit1Block = false;
    //Public NPCIsABonus(0 To maxNPCType) As Boolean 'Flags the NPC type if it is a bonus
    bool IsABonus = false;
    //Public NPCIsACoin(0 To maxNPCType) As Boolean 'Flags the NPC type if it is a coin
    bool IsACoin;
    //Public NPCIsAVine(0 To maxNPCType) As Boolean 'Flags the NPC type if it is a vine
    bool IsAVine;
    //Public NPCIsAnExit(0 To maxNPCType) As Boolean 'Flags the NPC type if it is a level exit
    bool IsAnExit;
    //Public NPCIsAParaTroopa(0 To maxNPCType) As Boolean 'Flags the NPC type as a para-troopa
    bool IsAParaTroopa;
    //Public NPCIsCheep(0 To maxNPCType) As Boolean 'Flags the NPC type as a cheep cheep
    bool IsCheep;
    //Public NPCJumpHurt(0 To maxNPCType) As Boolean 'Hurts the player even if it jumps on the NPC
    bool JumpHurt;
    //Public NPCNoClipping(0 To maxNPCType) As Boolean 'NPC can go through blocks
    bool NoClipping;
    //Public NPCScore(0 To maxNPCType) As Integer 'NPC score value
    int8_t Score;
    //Public NPCCanWalkOn(0 To maxNPCType) As Boolean  'NPC can be walked on
    bool CanWalkOn;
    //Public NPCGrabFromTop(0 To maxNPCType) As Boolean  'NPC can be grabbed from the top
    bool GrabFromTop;
    //Public NPCTurnsAtCliffs(0 To maxNPCType) As Boolean  'NPC turns around at cliffs
    bool TurnsAtCliffs;
    //Public NPCWontHurt(0 To maxNPCType) As Boolean  'NPC wont hurt the player
    bool WontHurt;
    //Public NPCMovesPlayer(0 To maxNPCType) As Boolean 'Player can not walk through the NPC
    bool MovesPlayer;
    //Public NPCStandsOnPlayer(0 To maxNPCType) As Boolean 'for the clown car
    bool StandsOnPlayer;
    //Public NPCIsGrabbable(0 To maxNPCType) As Boolean 'Player can grab the NPC
    bool IsGrabbable;
    //Public NPCIsBoot(0 To maxNPCType) As Boolean 'npc is a kurbo's shoe
    bool IsBoot;
    //Public NPCIsYoshi(0 To maxNPCType) As Boolean 'npc is a yoshi
    bool IsYoshi;
    //Public NPCIsToad(0 To maxNPCType) As Boolean 'npc is a toad
    bool IsToad;
    //Public NPCNoYoshi(0 To maxNPCType) As Boolean 'Player can't eat the NPC
    bool NoYoshi;
    //Public NPCForeground(0 To maxNPCType) As Boolean 'draw the npc in front
    bool Foreground;
    //Public NPCIsABot(0 To maxNPCType) As Boolean 'Zelda 2 Bot monster
    bool IsABot;
    //Public NPCDefaultMovement(0 To maxNPCType) As Boolean 'default NPC movement
    bool DefaultMovement;
    //Public NPCIsVeggie(0 To maxNPCType) As Boolean 'turnips
    bool IsVeggie;
    //Public NPCNoFireBall(0 To maxNPCType) As Boolean 'not hurt by fireball
    bool NoFireBall;
    //Public NPCNoIceBall(0 To maxNPCType) As Boolean 'not hurt by fireball
    bool NoIceBall;
    //Public NPCNoGravity(0 To maxNPCType) As Boolean 'not affected by gravity
    bool NoGravity;

    //Public NPCFrame(0 To maxNPCType) As Integer
    int16_t Frame;
    //Public NPCFrameSpeed(0 To maxNPCType) As Integer
    int16_t FrameSpeed;
    //Public NPCFrameStyle(0 To maxNPCType) As Integer
    int16_t FrameStyle;
};

extern RangeArr<NPCTraits_t, 0, maxNPCType> NPCTraits;

#if 0
inline const NPCTraits_t* NPC_t::operator->() const
{
    return &NPCTraits[Type];
}
#endif

#endif // #ifndef NPC_TRAITS_H
