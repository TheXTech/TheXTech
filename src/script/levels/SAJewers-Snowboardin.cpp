#include "globals.h"
#include "SAJewers-Snowboardin.h"
#include "../lunanpc.h"

#define NPC_SHELL 195

namespace SAJSnowbordin
{
NPC_t *FindNPC(short identity);
int combo_start;

void SnowbordinInitCode()
{
    combo_start = 0;
}

void SnowbordinCode()
{
    NPC_t *shell_npc;
    shell_npc = FindNPC(NPC_SHELL);

    if(shell_npc == NULL)
        return;

    int &shell_kills = shell_npc->Multiplier;

    if(shell_kills >= 9)
    {
        if(combo_start < 6)
            combo_start += 2;
        shell_kills = combo_start;
    }

    //Renderer::Get().SafePrint(std::wstring(L"KILLS: " + std::to_wstring(*shell_kills)), 3, 0, 256);

}

NPC_t *FindNPC(short identity)
{
    NPC_t *currentnpc = nullptr;

    for(int i = 0; i < numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if(currentnpc->Type == identity)
            return currentnpc;
    }

    return nullptr;
}

} // SAJSnowbordin
