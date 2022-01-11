#include "lunanpc.h"
#include "globals.h"
#include "mememu.h"
#include "npc.h"


NPC_t *NpcF::Get(int index)
{
    if(index < 0 || index > numNPCs)
        return nullptr;

    return &NPC[index + 1];
}

NPC_t *NpcF::GetRaw(int index)
{
    return &NPC[index];
}

void NpcF::FindAll(int ID, int section, std::list<NPC_t *> *return_list)
{
    bool anyID = (ID == -1 ? true : false);
    bool anySec = (section == -1 ? true : false);
    NPC_t *thisnpc = NULL;

    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = &NPC[i];
        if(thisnpc->Type == ID || anyID)
        {
            if(thisnpc->Section == section || anySec)
                return_list->push_back(thisnpc);
        }
    }
}

NPC_t *NpcF::GetFirstMatch(int ID, int section)
{
    bool anyID = (ID == -1 ? true : false);
    bool anySec = (section == -1 ? true : false);
    NPC_t *thisnpc = nullptr;

    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = &NPC[i];
        if(thisnpc->Type == ID || anyID)
        {
            if(thisnpc->Section == section || anySec)
                return thisnpc; //matched
        }
    }

    return nullptr; //not matched
}

void NpcF::MemSet(int ID, int offset, double value, OPTYPE operation, FIELDTYPE ftype)
{
    //    char* dbg =  "MemSetDbg";
    if(ftype == FT_INVALID || offset > 0x15C)
        return;

    bool anyID = (ID == -1 ? true : false);
    NPC_t *thisnpc;

    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = &NPC[i];
        if(anyID || thisnpc->Type == ID)
            MemAssign(thisnpc, offset, value, operation, ftype);
    }//for
}

void NpcF::AllSetHits(int identity, int section, float hits)
{
    bool anyID = (identity == -1 ? true : false);
    bool anySec = (section == -1 ? true : false);
    NPC_t *thisnpc;

    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = Get(i);
        if(anyID || thisnpc->Type == identity)
        {
            if(anySec || thisnpc->Section == section)
                thisnpc->Damage = hits;
        }
    }
}

void NpcF::AllFace(int identity, int section, double x)
{
    bool anyID = (identity == -1 ? true : false);
    bool anySec = (section == -1 ? true : false);
    NPC_t *thisnpc;
    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = Get(i);
        if(anyID || thisnpc->Type == identity)
        {
            if(anySec || thisnpc->Section == section)
            {
                if(x < thisnpc->Location.X)
                    thisnpc->TurnAround = -1;
                else
                    thisnpc->TurnAround = 1;
            }
        }
    }
}
