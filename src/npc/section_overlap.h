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
#ifndef SECTION_OVERLAP_H
#define SECTION_OVERLAP_H

#include <array>
#include <cstdint>

#include "global_constants.h"

extern std::array<uint8_t, maxSections + 1> g_SectionFirstOverlap;

//! calculate all section overlaps, called on level load
void CalculateSectionOverlaps();

//! Update section overlaps after section S's bounds are changed.
//  Set shrink if section S's new bounds are contained in its old bounds.
void UpdateSectionOverlaps(int S, bool shrink = false);


#endif
