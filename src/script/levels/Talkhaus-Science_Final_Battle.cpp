#include <vector>

#include "Talkhaus-Science_Final_Battle.h"
#include "globals.h"
#include "../lunanpc.h"
#include "../lunaplayer.h"

#define NPC_FIREBAR 260
#define NPC_DOUGHNUT 210
#define NPC_SCIENCE 209

namespace ScienceBattle
{

static NPC_t *FindNPC(short identity);
static std::vector<NPC_t *> FindAllNPC(short identity);
static bool TriggerBox(double x1, double y1, double x2, double y2);
static void HurtPlayer();

static bool init_doonce;
static NPC_t *hurt_npc; //, *science_npc, *friendly_doughnut;
static std::vector<NPC_t *> doughnuts;
static Player_t *demo;
static int hurt_timer;
static int grace_timer;
static int throw_timer;

void ScienceInitCode()
{
    init_doonce = false;
}

void ScienceCode()
{
    if(!init_doonce)
    {
        init_doonce     = true;
        hurt_timer      = 0;
        throw_timer     = 0;
        demo            = PlayerF::Get(1);
    }

    hurt_npc = FindNPC(NPC_FIREBAR);

    if(hurt_npc == NULL)
        return;


    if(hurt_timer <= 0)
        hurt_npc->Location.Y = demo->Location.Y - 128;
    else
    {
        hurt_timer--;
        hurt_npc ->Location.Y = demo->Location.Y;
    }
    hurt_npc->Location.X = demo->Location.X;

    doughnuts = FindAllNPC(NPC_DOUGHNUT);

    if(demo->HoldingNPC > 0)
        throw_timer = 30;

    //Renderer::Get().SafePrint(std::wstring(L"ID: " + std::to_wstring(demo->HeldNPCIndex)), 3, 0, 256);


    if(grace_timer >= 0)
    {
        for(std::vector<NPC_t *>::const_iterator it = doughnuts.begin(); it != doughnuts.end(); it++)
        {
            NPC_t *doughnut = *it;
            double x_diff, y_diff, m;

            x_diff = doughnut->Location.X - demo->Location.X;
            y_diff = doughnut->Location.Y - demo->Location.Y;
            m = sqrt(x_diff * x_diff + y_diff * y_diff);

            if(m == 0)
                continue;

            x_diff /= m;
            y_diff /= m;

            doughnut->Location.X += x_diff * 15;
            doughnut->Location.Y += y_diff * 15;
        }
        grace_timer--;
    }
    else
    {
        if(throw_timer <= 0)
        {
            for(std::vector<NPC_t *>::const_iterator it = doughnuts.begin(); it != doughnuts.end(); it++)
            {
                NPC_t *doughnut = *it;
                //Ignore generators
                if((*((int *)doughnut + 16)) != 0)
                    continue;

                double x1, x2, y1, y2;

                x1 = doughnut->Location.X + 28 * 0.42;
                y1 = doughnut->Location.Y + 32 * 0.42;
                x2 = doughnut->Location.X + 28 * 0.57;
                y2 = doughnut->Location.Y + 32 * 0.57;

                if(TriggerBox(x1, y1, x2, y2))
                    HurtPlayer();
            }
        }
    }


    if(throw_timer > 0)
        throw_timer--;
}

static NPC_t *FindNPC(short identity)
{
    NPC_t *currentnpc = nullptr;

    for(int i = 0; i <= numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if(currentnpc->Type == identity)
            return currentnpc;
    }

    return nullptr;
}

static std:: vector<NPC_t *> FindAllNPC(short identity)
{
    std::vector<NPC_t *> npcs_found = std::vector<NPC_t *>();
    NPC_t *currentnpc = nullptr;

    for(int i = 0; i <= numNPCs; i++)
    {
        currentnpc = NpcF::Get(i);
        if(currentnpc->Type == identity)
            npcs_found.push_back(currentnpc);
    }

    return npcs_found;
}

static void HurtPlayer()
{
    hurt_timer = 3;
    grace_timer = 120;
}

static bool TriggerBox(double x1, double y1, double x2, double y2)
{
    return (demo->Location.X + demo->Location.Width     > x1 &&
            demo->Location.X                            < x2 &&
            demo->Location.Y + demo->Location.Height    > y1 &&
            demo->Location.Y                            < y2);
}

}
