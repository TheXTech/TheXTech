/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef EFFECT_H
#define EFFECT_H

#include "location.h"

// Public Sub UpdateEffects() 'Updates the effects
// Updates the effects
void UpdateEffects();
// Public Sub NewEffect(A As Integer, Location As Location, Optional Direction As Single = 1, Optional NewNpc As Integer = 0, Optional Shadow As Boolean = False)  'Create an effect
// Create an effect
void NewEffect(int A, const Location_t &Location_t, float Direction = 1, int NewNpc = 0, bool Shadow = false);
// Public Sub KillEffect(A As Integer) 'Remove the effect
// Remove the effect
void KillEffect(int A);


#endif // EFFECT_H
