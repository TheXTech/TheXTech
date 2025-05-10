/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <bitset>
#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "layers.h"
#include "config.h"
#include "collision.h"

#include "npc_id.h"
#include "npc_traits.h"
#include "npc_queues.h"

#include "main/trees.h"

inline static bool s_Event_SoundOnly(const Events_t& evt, int test_section)
{
    if(!(evt.Text == STRINGINDEX_NONE
        && evt.HideLayer.empty()
        && evt.ShowLayer.empty()
        && evt.ToggleLayer.empty()
        && evt.MoveLayer == LAYER_NONE
        && evt.TriggerEvent == EVENT_NONE
        && evt.EndGame == 0))
    {
        return false;
    }

    bool autoscroll_okay = !AutoUseModern || evt.AutoSection != test_section
        || (g_config.fix_autoscroll_speed
            ? (!(evt.AutoX != 0 || evt.AutoY != 0)
                || (AutoX[evt.AutoSection] == evt.AutoX && AutoY[evt.AutoSection] == evt.AutoY))
            : (!IF_INRANGE(evt.AutoSection, 0, SDL_min(maxSections, maxEvents))
                || (AutoX[evt.AutoSection] == Events[evt.AutoSection].AutoX
                    && AutoY[evt.AutoSection] == Events[evt.AutoSection].AutoY)));

    if(!autoscroll_okay)
        return false;

    const EventSection_t& s = const_cast<Events_t&>(evt).section[test_section];
    bool section_okay = s.music_id == EventSection_t::LESet_Nothing
        && s.background_id == EventSection_t::LESet_Nothing
        && s.position.X == EventSection_t::LESet_Nothing
        && s.autoscroll == false;

    return section_okay;
}

// from section of UpdateNPCs called "process chain activations"
static bool s_NPC_CanActivate(const NPC_t& n)
{
    return n.Type != NPCID_CONVEYOR && n.Type != NPCID_FALL_BLOCK_RED &&
        n.Type != NPCID_FALL_BLOCK_BROWN && !n->IsACoin;
}

static bool s_NPC_MustBeCanonical_internal(const NPC_t& n)
{
    if(!s_NPC_CanActivate(n))
        return n.Generator;

    return n.Generator
        || n->UseDefaultCam
        || (n->IsFish && n.Special == 2)
        || n.AttLayer != LAYER_NONE
        || (n.Legacy && (n.Type == NPCID_MINIBOSS || n.Type == NPCID_SPIT_BOSS || n.Type == NPCID_VILLAIN_S3))
        || (n.TriggerActivate != EVENT_NONE && !s_Event_SoundOnly(Events[n.TriggerActivate], n.Section));
}

bool NPC_MustBeCanonical(NPCRef_t n)
{
    return n->_priv_force_canonical;
}

bool NPC_InactiveIgnore(const NPC_t& n)
{
    return (n->IsFish && n.Special == 2)
        || n->InactiveRender == NPCTraits_t::SKIP
        || n._priv_self_hide_event;
}

bool NPC_InactiveRender(const NPC_t& n)
{
    if(n->InactiveRender == NPCTraits_t::SMOKE)
        return false;

    return n.Inert
        || n.Stuck
        || n->InactiveRender == NPCTraits_t::SHOW_ALWAYS
        || n->InactiveRender == NPCTraits_t::SHOW_STATIC;
}

bool NPC_InactiveSmoke(const NPC_t& n)
{
    return n->InactiveRender == NPCTraits_t::SMOKE;
}

void NPC_ConstructCanonicalSet()
{
    std::vector<int16_t> to_check;
    to_check.reserve(64);

    for(int16_t n = 1; n <= numNPCs; n++)
    {
        if(s_NPC_MustBeCanonical_internal(NPC[n]) || NPC[n]._priv_force_canonical)
        {
            NPC[n]._priv_force_canonical = true;
            to_check.push_back(n);
        }

        // check if NPC has a hide self event
        if(NPC[n].TriggerActivate != EVENT_NONE && NPC[n].Layer != LAYER_NONE && NPC[n].Layer != LAYER_DEFAULT)
        {
            const Events_t& e = Events[NPC[n].TriggerActivate];

            for(layerindex_t l : e.HideLayer)
            {
                if(l == NPC[n].Layer)
                {
                    NPC[n]._priv_self_hide_event = true;
                    break;
                }
            }
        }
    }

    // find all NPCs that could cause the above NPCs to activate via chain activation
    for(size_t i = 0; i < to_check.size(); i++)
    {
        int16_t n = to_check[i];

        Location_t tempLocation = NPC[n].Location;
        tempLocation.Y -= 32;
        tempLocation.X -= 32;
        tempLocation.Width += 64;
        tempLocation.Height += 64;

        bool n_is_generator = NPC[n].Generator;

        for(int16_t B : treeNPCQuery(tempLocation, SORTMODE_NONE))
        {
            if(B != n && CheckCollision(tempLocation, NPC[B].Location))
            {
                // check that B is capable of chain-activation (or that n is a generator, in which case B would block it)
                if(!n_is_generator && !s_NPC_CanActivate(NPC[B]))
                    continue;

                if(!NPC[B]._priv_force_canonical)
                {
                    NPC[B]._priv_force_canonical = true;
                    to_check.push_back(B);
                }
            }
        }
    }
}

void NPCActivationLogic(int A)
{
    if(NPC[A].Active)
    {
        if((NPC[A]->IsFish && NPC[A].Special == 2) || NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET || NPC[A].Type == NPCID_GHOST_FAST) // Special Start for Jumping Fish and Bullet Bills
        {
            if(NPC[A].TimeLeft <= 1)
            {
                NPC[A].Active = false;
                NPC[A].TimeLeft = 0;
            }
            else if(NPC[A].Direction == -1 && NPC[A].Location.X < Player[NPC[A].JustActivated].Location.X)
            {
                NPC[A].Active = false;
                NPC[A].TimeLeft = 0;
            }
            else if(NPC[A].Direction == 1 && NPC[A].Location.X > Player[NPC[A].JustActivated].Location.X)
            {
                NPC[A].Active = false;
                NPC[A].TimeLeft = 0;
            }
            else if(NPC[A]->IsFish && NPC[A].Special == 2)
            {
                NPC[A].Location.Y = level[Player[NPC[A].JustActivated].Section].Height - 0.1_n;
                NPC[A].Location.SpeedX = (1 + (NPC[A].Location.Y - NPC[A].DefaultLocationY) / 200) * NPC[A].Direction;
                NPC[A].Special5 = 1;
                treeNPCUpdate(A);
                if(NPC[A].tempBlock > 0)
                    treeNPCSplitTempBlock(A);
            }
            else if(NPC[A].Type != NPCID_GHOST_FAST && !FreezeNPCs)
                PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_GOALTAPE)
        {
            Location_t tempLocation = NPC[A].Location;
            tempLocation.Height = 8000;
            int C = 0;
            for(int B : treeBlockQuery(tempLocation, SORTMODE_COMPAT))
            {
                if(CheckCollision(tempLocation, Block[B].Location))
                {
                    if(C == 0)
                        C = B;
                    else
                    {
                        if(Block[B].Location.Y < Block[C].Location.Y)
                            C = B;
                    }
                }
            }

            if(C > 0)
            {
                // save location of ground below goal tape (was previously Special2)
                NPC[A].SpecialY = Block[C].Location.Y + 4;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height;
                NPC[A].Special = 1;

                treeNPCUpdate(A);
                if(NPC[A].tempBlock > 0)
                    treeNPCSplitTempBlock(A);
            }
        }
        else if(NPC[A].Type == NPCID_LAVA_MONSTER) // blaarg
        {
            NPC[A].Location.Y = NPC[A].DefaultLocationY + NPC[A].Location.Height + 36;
            treeNPCUpdate(A);
            if(NPC[A].tempBlock > 0)
                treeNPCSplitTempBlock(A);
        }
        else if(NPC[A].Type == NPCID_CANNONENEMY)
            NPC[A].Special = 100;
    }

    // NOTE: these were not guarded by the Active check above in SMBX 1.3
    // they can't be safely moved into it because the iRand call has side effects
    if(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM)
        NPC[A].Projectile = false;
    else if(NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4)
        NPC[A].Special = iRand(200);
    // new, clip wings from buried items
    else if(NPC[A].Type == NPCID_ITEM_BURIED)
        NPC[A].Wings = WING_NONE;

    NPC[A].JustActivated = 0;
    NPC[A].CantHurt = 0;
    NPC[A].CantHurtPlayer = 0;

    // in addition to removing cancelled NPCs from the Active list,
    // this also allows us to exclude Vines from the set of active NPCs
    // (without checking their status every frame if they do need to be active)
    if(!NPCQueues::check_active(NPC[A]))
        NPCQueues::Active.erase(A);
}
