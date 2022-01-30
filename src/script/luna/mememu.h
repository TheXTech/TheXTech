/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef MEMEMU_H
#define MEMEMU_H

#include <cstddef>
#include "lunadefs.h"

struct Player_t;
struct NPC_t;

#define GM_BASE             0x00B25000
#define GM_END              0x00B2E000

//Global
void MemAssign(size_t address, double value, OPTYPE operation, FIELDTYPE ftype);
bool CheckMem(size_t address, double value, COMPARETYPE ctype, FIELDTYPE ftype);
double GetMem(size_t addr, FIELDTYPE ftype);

// Player relative
void MemAssign(Player_t *obj, size_t address, double value, OPTYPE operation, FIELDTYPE ftype);
bool CheckMem(Player_t *obj, size_t offset, double value, COMPARETYPE ctype, FIELDTYPE ftype);
double GetMem(Player_t *obj, size_t offset, FIELDTYPE ftype);

// NPC relative
void MemAssign(NPC_t *obj, size_t address, double value, OPTYPE operation, FIELDTYPE ftype);
bool CheckMem(NPC_t *obj, size_t offset, double value, COMPARETYPE ctype, FIELDTYPE ftype);
double GetMem(NPC_t *obj, size_t offset, FIELDTYPE ftype);

#endif // MEMEMU_H
