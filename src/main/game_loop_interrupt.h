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
#ifndef GAME_LOOP_INTERRUPT_H
#define GAME_LOOP_INTERRUPT_H

struct GameLoopInterrupt
{
    enum Site
    {
        None = 0,
        UpdatePlayer_MessageNPC,
        UpdatePlayer_TriggerTalk,
        UpdatePlayer_SuperWarp,
        UpdateNPCs_Activation_Generator,
        UpdateNPCs_Activation_Self,
        UpdateNPCs_Activation_Chain,
        UpdateNPCs_FreezeNPCs_KillNPC,
        UpdateNPCs_Normal_KillNPC,
        UpdateEvents,
        UpdateBlocks_KillBlock,
    };

    // variables to help with resume
    Site site = None;
    int A, B, C, D, E, F, G;
    bool bool1, bool2, bool3, bool4;
};

extern GameLoopInterrupt g_gameLoopInterrupt;

#endif // #ifndef GAME_LOOP_INTERRUPT_H
