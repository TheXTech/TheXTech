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
#ifndef LUNANPC_H
#define LUNANPC_H

#include <list>

#include "lunadefs.h"

struct NPC_t;

namespace NpcF
{

NPC_t* Get(int index); //Get ptr to an NPC
NPC_t* GetRaw(int index);

void FindAll(int ID, int section, std::list<NPC_t *> *return_list);

// GET FIRST MATCH
NPC_t *GetFirstMatch(int ID, int section);

void MemSet(int ID, int offset, double value, OPTYPE operation, FIELDTYPE ftype); // ID -1 for ALL

// ITERATORS
void AllSetHits(int identity, int section, float hits);		// Set all specified NPC hits
void AllFace(int identity, int section, double x);	// All specified NPCs face the supplied x/y point

} // NpcF

#endif // LUNANPC_H
