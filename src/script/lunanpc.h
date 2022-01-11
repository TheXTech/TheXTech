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
