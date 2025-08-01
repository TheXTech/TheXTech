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

#include "sdl_proxy/sdl_stdinc.h"
#include "globals.h"
#include "npc.h"
#include "sound.h"
#include "graphics.h"
#include "effect.h"
#include "game_main.h"
#include "player.h"
#include "collision.h"
#include "editor.h"
#include "blocks.h"
#include "controls.h"
#include "config.h"
#include "main/trees.h"
#include "core/events.h"
#include "npc_id.h"
#include "eff_id.h"
#include "npc_traits.h"
#include "layers.h"

#include "npc/npc_queues.h"
#include "npc/npc_activation.h"
#include "npc/section_overlap.h"
#include "npc/npc_cockpit_bits.h"
#include "npc/npc_update/npc_update_priv.h"

void NPC_t::ResetLocation()
{
    Location.X = DefaultLocationX;
    Location.Y = DefaultLocationY;
    Location.Width = (*this)->TWidth;
    Location.Height = (*this)->THeight;
    Location.SpeedX = 0;
    Location.SpeedY = 0;

    if(DefaultLocationHeight_Force32)
        Location.Height = 32;
}

// UpdateNPCs at npc/npc_update.cpp

// NpcHit at npc/npc_hit.cpp

num_t NPCPlayerTargetDist(const NPC_t& npc, const Player_t& player)
{
    num_t dx = npc.Location.minus_center_x(player.Location);

    if(g_config.fix_multiplayer_targeting)
    {
        num_t dy = npc.Location.minus_center_y(player.Location);
        return num_t::dist2(dx, dy);
    }
    else
        return num_t::abs(dx);
}

int NPCTargetPlayer(const NPC_t& npc)
{
    int target_plr = 0;
    num_t min_dist = 0;

    for(int B = 1; B <= numPlayers; B++)
    {
        if(!Player[B].Dead && Player[B].Section == npc.Section)
        {
            if(numPlayers == 1)
                return 1;

            num_t dist = NPCPlayerTargetDist(npc, Player[B]);
            if(min_dist == 0 || dist < min_dist)
            {
                min_dist = dist;
                target_plr = B;
            }
        }
    }

    return target_plr;
}

int NPCFaceNearestPlayer(NPC_t& npc, bool old_version)
{
    int target = NPCTargetPlayer(npc);

    num_t target_center = Player[target].Location.X + Player[target].Location.Width / 2;

    // broken version of logic that appears in a few places
    if(old_version)
    {
        if(target_center > npc.Location.X + 16)
            npc.Direction = 1;
        else
            npc.Direction = -1;
    }
    else
    {
        if(target != 0)
        {
            if(npc.Location.X + npc.Location.Width / 2 > target_center)
                npc.Direction = -1;
            else
                npc.Direction = 1;
        }
    }

    return target;
}

static void s_NPCSetSpeedTarget_FixedX_dist(NPC_t& npc, num_t dist_x, num_t dist_y, int speed_x, int speed_y_cap)
{
    // originally only applied to plant, but good to apply in all cases (and was UB in VB6)
    if(dist_x == 0)
        dist_x = -0.00001_n;

    npc.Location.SpeedX = speed_x * npc.Direction;
    npc.Location.SpeedY = speed_x * npc.Direction * dist_y.divided_by(dist_x);

    if(NPC[numNPCs].Location.SpeedY > speed_y_cap)
        NPC[numNPCs].Location.SpeedY = speed_y_cap;
    else if(NPC[numNPCs].Location.SpeedY < -speed_y_cap)
        NPC[numNPCs].Location.SpeedY = -speed_y_cap;
}

void NPCSetSpeedTarget_FixedX(NPC_t& npc, const Location_t& target, int speed_x, int speed_y_cap)
{
    num_t dist_x = target.minus_center_x(npc.Location);
    num_t dist_y = target.minus_center_y(npc.Location);

    s_NPCSetSpeedTarget_FixedX_dist(npc, dist_x, dist_y, speed_x, speed_y_cap);
}

void NPCSetSpeedTarget_FixedX(NPC_t& npc, num_t target_x, num_t target_y, int speed_x, int speed_y_cap)
{
    num_t dist_x = target_x - (npc.Location.X + npc.Location.Width / 2);
    num_t dist_y = target_y - (npc.Location.Y + npc.Location.Height / 2);

    s_NPCSetSpeedTarget_FixedX_dist(npc, dist_x, dist_y, speed_x, speed_y_cap);
}

void NPCSetSpeedTarget_FixedSpeed(NPC_t& npc, const Location_t& target, int speed)
{
    num_t dist_x = target.minus_center_x(npc.Location);
    num_t dist_y = target.minus_center_y(npc.Location);

    num_t dist = num_t::dist(dist_x, dist_y);

    npc.Location.SpeedX = speed * dist_x.divided_by(dist);
    npc.Location.SpeedY = speed * dist_y.divided_by(dist);
}

void CheckSectionNPC(int A)
{
    if(GameMenu)
        return;

    if(NPC[A].HoldingPlayer > 0)
    {
        if(NPC[A].TimeLeft < 10)
            NPC[A].TimeLeft = 10;
        NPC[A].Section = Player[NPC[A].HoldingPlayer].Section;
    }

    // disabled in 1.3.6.1 because of bugs when overlapping sections change
    // Previously, it optimized for section 0. Now, it optimizes for NPC's current section.
    int B = NPC[A].Section;
    if(B <= maxSections && g_SectionFirstOverlap[B] == B && NPC[A].Location.X >= level[B].X)
    {
        if(NPC[A].Location.X + NPC[A].Location.Width <= level[B].Width)
        {
            if(NPC[A].Location.Y >= level[B].Y)
            {
                if(NPC[A].Location.Y + NPC[A].Location.Height <= level[B].Height)
                {
                    return;
                }
            }
        }
    }

    for(int B = 0; B < numSections; B++)
    {
        if(NPC[A].Location.X >= level[B].X)
        {
            if(NPC[A].Location.X + NPC[A].Location.Width <= level[B].Width)
            {
                if(NPC[A].Location.Y >= level[B].Y)
                {
                    if(NPC[A].Location.Y + NPC[A].Location.Height <= level[B].Height)
                    {
                        NPC[A].Section = B;
                        return;
                    }
                }
            }
        }
    }
}

void Deactivate(int A)
{
    if(NPC[A].DefaultType > 0)
    {
        if(NPC[A].TurnBackWipe && NoTurnBack[NPC[A].Section])
        {
            NPC[A].Killed = 9;
            NPCQueues::Killed.push_back(A);
        }
        else
        {
            if(NPC[A].Type == NPCID_SKELETON && NPC[A].Special > 0)
            {
                NPC[A].Inert = false;
                NPC[A].Stuck = false;
            }

            // reset variables back to default
            NPC[A].Quicksand = 0;
            NPC[A].NoLavaSplash = false;
            NPC[A].Active = false;

            // prevent instantly respawning, unless the animation will be totally seamless
            if(!g_config.fix_npc_camera_logic
                || !NPC_InactiveRender(NPC[A])
                || num_t::floor(NPC[A].Location.X) != num_t::floor(NPC[A].DefaultLocationX)
                || num_t::floor(NPC[A].Location.Y) != num_t::floor(NPC[A].DefaultLocationY))
            {
                NPC[A].Reset[1] = false;
                NPC[A].Reset[2] = false;
            }

            // reset variables
            NPC[A].Type = NPC[A].DefaultType;
            NPC[A].ResetLocation();
            NPC[A].Direction = NPC[A].DefaultDirection;
            NPC[A].Stuck = NPC[A].DefaultStuck;

            // NEW: reset wings (unless the NPC is a container that bans wings)
            NPC[A].Wings = NPC[A].DefaultWings;
            if(NPC[A].Type == NPCID_ITEM_BURIED || NPC[A].Type == NPCID_ITEM_BUBBLE)
                NPC[A].Wings = WING_NONE;

            NPC[A].TimeLeft = 0;
            NPC[A].Projectile = false;
            NPC[A].Effect = NPCEFF_NORMAL;

            // counter to respawn
            if(NPC[A].RespawnDelay)
                NPC[A].Effect2 = 65 * 30;
            else
                NPC[A].Effect2 = 0;

            NPC[A].Effect3 = 0;
            NPC[A].BeltSpeed = 0;
            NPC[A].vehiclePlr = 0;
            NPC[A].vehicleYOffset = 0;
            NPC[A].Frame = 0;
            NPC[A].Killed = 0;
            NPC[A].Shadow = false;
            NPC[A].oldAddBelt = 0;

            // DefaultSpecial
            NPC[A].Special = NPC[A].DefaultSpecial;
            NPC[A].Special2 = 0; // NPC[A].DefaultSpecial2; // moved to Variant for the one type (NPCID_MAGIC_DOOR) that used it

            // for generators, these store critical variables
            if(!NPC[A].Generator)
            {
                NPC[A].Special3 = 0;
                NPC[A].Special4 = 0;
                NPC[A].Special5 = 0;
            }

            // NPC[A].Special6 = 0;
            NPC[A].SpecialX = 0;
            NPC[A].SpecialY = 0;
            if(!NPC[A]->IsACoin)
                NPC[A].Damage = 0;
            NPC[A].HoldingPlayer = 0;

            // NPC[A].Pinched1 = 0;
            // NPC[A].Pinched2 = 0;
            // NPC[A].Pinched3 = 0;
            // NPC[A].Pinched4 = 0;
            // NPC[A].Pinched = 0;
            // NPC[A].MovingPinched = 0;

            NPC[A].Pinched = PinchedInfo_t();

            // new-added logic: if CURRENTLY offscreen for all vScreens, allow it to reset
            if(g_config.fix_frame_perfect_despawn && (g_config.fix_timestop_respawn || !FreezeNPCs))
            {
                bool hit = false;

                for(int screen_i = 0; !hit && screen_i < c_screenCount; screen_i++)
                {
                    const Screen_t& screen = Screens[screen_i];

                    for(int vscreen_i = screen.active_begin(); !hit && vscreen_i < screen.active_end(); vscreen_i++)
                    {
                        int vscreen_Z = screen.vScreen_refs[vscreen_i];

                        if(vScreenCollision(vscreen_Z, NPC[A].Location))
                            hit = true;
                    }
                }

                if(!hit)
                {
                    NPC[A].Reset[1] = true;
                    NPC[A].Reset[2] = true;
                }
            }

            // NEW now that we have the new NPC Queues
            NPCQueues::update(A);
            bool changed = treeNPCUpdate(A);
            if(changed && NPC[A].tempBlock > 0)
                treeNPCSplitTempBlock(A);
        }
    }
    else if(NPCIsAnExit(NPC[A]))
        NPC[A].TimeLeft = 100;
    else
    {
        NPC[A].Killed = 9;
        NPCQueues::Killed.push_back(A);
    }
}

void Bomb(Location_t Location, int Game, int ImmunePlayer)
{
    num_t Radius = 0;
    // int i = 0;
    num_t X = 0;
    num_t Y = 0;
    num_t A = 0;
    num_t B = 0;
    num_t C = 0;

    NPC[0].Multiplier = 0;
    if(Game == 0)
    {
        NewEffect(EFFID_CHAR3_HEAVY_EXPLODE, Location);
        PlaySoundSpatial(SFX_Bullet, Location);
        Radius = 32;
    }
    if(Game == 2)
    {
        Controls::RumbleAllPlayers(150, 1.0);
        NewEffect(EFFID_BOMB_S2_EXPLODE, Location);
        PlaySoundSpatial(SFX_Fireworks, Location);
        Radius = 52;
    }
    if(Game == 3)
    {
        Controls::RumbleAllPlayers(200, 1.0);
        NewEffect(EFFID_BOMB_S3_EXPLODE_SEED, Location);
        PlaySoundSpatial(SFX_Fireworks, Location);
        Radius = 64;
    }

    X = Location.X + Location.Width / 2;
    Y = Location.Y + Location.Height / 2;

    for(int i : treeNPCQuery(newLoc(X - Radius, Y - Radius, Radius * 2, Radius * 2), SORTMODE_ID))
    {
        if(!NPC[i].Hidden && NPC[i].Active && !NPC[i].Inert && !NPC[i].Generator && !NPC[i]->IsABonus)
        {
            if(NPC[i].Type != NPCID_PLR_FIREBALL && NPC[i].Type != NPCID_CHAR3_HEAVY)
            {
                A = NPC[i].Location.X + NPC[i].Location.Width / 2 - X;
                B = NPC[i].Location.Y + NPC[i].Location.Height / 2 - Y;
                C = num_t::dist(A, B);

                if(C <= Radius + NPC[i].Location.Width / 4 + NPC[i].Location.Height / 4)
                {
                    NPC[0].Location = NPC[i].Location;
                    NPCHit(i, 3, 0);
                    if(NPCIsVeggie(NPC[i]))
                    {
                        NPC[i].Projectile = true;
                        NPC[i].Location.SpeedY = -5;
                        NPC[i].Location.SpeedX = dRand() * 4 - 2;
                    }
                }
            }
        }
    }

    for(int i : treeBlockQuery(newLoc(X - Radius, Y - Radius, Radius * 2, Radius * 2), SORTMODE_COMPAT))
    {
        if(!Block[i].Hidden && !BlockNoClipping[Block[i].Type])
        {
            A = Block[i].Location.X + Block[i].Location.Width / 2 - X;
            B = Block[i].Location.Y + Block[i].Location.Height / 2 - Y;
            C = num_t::dist(A, B);

            if(C <= Radius + Block[i].Location.Width / 4 + Block[i].Location.Height / 4)
            {
                BlockHit(i);
                BlockHitHard(i);
                if(Game == 0 && Block[i].Type == 457)
                    SafelyKillBlock(i);
            }
        }
    }

    if(Game != 0 || BattleMode)
    {
        for(int i = 1; i <= numPlayers; i++)
        {
            if(Game != 0 || i != ImmunePlayer)
            {
                A = Player[i].Location.X + Player[i].Location.Width / 2 - X;
                B = Player[i].Location.Y + Player[i].Location.Height / 2 - Y;
                C = num_t::dist(A, B);

                if(C <= Radius + Player[i].Location.Width / 4 + Player[i].Location.Height / 4)
                    PlayerHurt(i);
            }
        }
    }
}

#if 0
// dead code, removed

void DropNPC(int A, int NPCType)
{
    int B = 0;
    if(A == 1 || numPlayers == 2)
    {
        PlaySound(SFX_DropItem);
        numNPCs++;
        NPC[numNPCs].Type = NPCType;
        NPC[numNPCs].Location.Width = NPCWidth(NPCType);
        NPC[numNPCs].Location.Height = NPCHeight(NPCType);
        if(ScreenType == 5 && !vScreen[2].Visible)
        {
            if(A == 1)
                B = -40;
            if(A == 2)
                B = 40;
            NPC[numNPCs].Location.X = -vScreen[1].X + vScreen[1].Width / 2 - NPC[numNPCs].Location.Width / 2 + B;
            NPC[numNPCs].Location.Y = -vScreen[1].Y + 16 + 12;
        }
        else
        {
            NPC[numNPCs].Location.X = -vScreen[A].X + vScreen[A].Width / 2 - NPC[numNPCs].Location.Width / 2;
            NPC[numNPCs].Location.Y = -vScreen[A].Y + 16 + 12;
        }
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].Location.SpeedY = 0;
        NPC[numNPCs].Effect = NPCEFF_DROP_ITEM;
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 200;
        syncLayers_NPC(numNPCs);
    }
}
#endif

void TurnNPCsIntoCoins()
{
    // need this complex loop syntax because Active can be modified within it
    for(int A : NPCQueues::Active.may_erase)
    {
        if(NPC[A].Active && !NPC[A].Generator)
        {
            if(!NPC[A].Hidden && NPC[A].Killed == 0 && !NPCIsAnExit(NPC[A]) && !NPC[A].Inert)
            {
                if(!NPCIsYoshi(NPC[A]) && !NPCIsBoot(NPC[A]) &&
                   !NPC[A]->IsABonus && NPC[A].Type != NPCID_PLR_ICEBALL && NPC[A].Type != NPCID_PLR_FIREBALL &&
                   NPC[A].Type != NPCID_PET_FIRE && NPC[A].Type != NPCID_SPRING && !NPCIsVeggie(NPC[A]) &&
                   NPC[A].Type != NPCID_ITEM_BURIED && NPC[A].Type != NPCID_PLR_HEAVY && !NPC[A]->IsAVine &&
                   NPC[A].Type != NPCID_VEHICLE && NPC[A].Type != NPCID_YEL_PLATFORM && NPC[A].Type != NPCID_BLU_PLATFORM &&
                   NPC[A].Type != NPCID_GRN_PLATFORM && NPC[A].Type != NPCID_RED_PLATFORM && NPC[A].Type != NPCID_PLATFORM_S3 &&
                   !(NPC[A].Projectile && NPC[A].Type == NPCID_HEAVY_THROWN) &&
                   !(NPC[A].Projectile && NPC[A].Type == NPCID_BULLET) &&
                   NPC[A].Type != NPCID_CHAR3_HEAVY && NPC[A].Type != NPCID_CHAR4_HEAVY && NPC[A].Type != NPCID_SWORDBEAM &&
                   NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_METALBARREL &&
                   !(NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M) &&
                   NPC[A].Type != NPCID_ITEM_BURIED && NPC[A].Type != NPCID_FIRE_CHAIN && NPC[A].Type != NPCID_FIRE_DISK)
                {
                    NPC[A].Location.Y += 32;
                    NewEffect(EFFID_COIN_BLOCK_S3, NPC[A].Location);
                    PlaySoundSpatial(SFX_Coin, NPC[A].Location);
                    Coins += 1;
                    if(Coins >= 100)
                        Got100Coins();

                    NPC[A].Killed = 9;
                    NPC[A].Location.Height = 0;

                    NPCQueues::Killed.push_back(A);

                    if(NPC[A].Active)
                        NPCQueues::Active.erase(A);

                    NPC[A].Active = false;

                    treeNPCUpdate(A);
                    if(NPC[A].tempBlock != 0)
                        treeNPCSplitTempBlock(A);
                }
                else if(NPC[A].Type == NPCID_GOALTAPE || NPC[A].Type == NPCID_FIRE_CHAIN || NPC[A].Type == NPCID_FIRE_DISK)
                {
                    NPC[A].Active = false;
                    NPCQueues::update(A);
                }
            }
        }
        else if(NPC[A].Generator)
        {
            NPC[A].Killed = 9;
            NPC[A].Hidden = true;

            NPCQueues::Killed.push_back(A);
        }
    }
}

#if 0
// previous recursive version of the function

void SkullRide(int A, bool reEnable)
{
    Location_t loc = NPC[A].Location;
    loc.Width += 16;
    loc.X -= 8;
    if(g_config.fix_skull_raft) // Detect by height in condition skull ruft cells were on slopes
    {
        loc.Height += 30;
        loc.Y -= 15;
    }

    int spec = reEnable ? 2 : 0;

    for(int B = 1; B <= numNPCs; B++) // Recursively activate all neihbour skull-ride segments
    {
        auto &npc = NPC[B];
        if(npc.Type == NPCID_RAFT)
        {
            if(npc.Active)
            {
                if(npc.Special == spec)
                {
                    if(CheckCollision(loc, npc.Location))
                    {
                        npc.Special = 1;
                        SkullRide(B, reEnable);
                    }
                }
            }
        }
    }
}
#endif

static void s_alignRuftCell(NPC_t &me, const Location_t &alignAt)
{
    num_t w = me.Location.Width;
    me.Location.SpeedX = 0;
    me.RealSpeedX = 0;

    if(me.Direction > 0)
    {
        auto p = alignAt.X;
        do
        {
            p -= w;
        } while(num_t::abs(p - me.Location.X) >= w / 2 && p > me.Location.X);
        me.Location.X = p;
    }
    else
    {
        auto p = alignAt.X + alignAt.Width;
        while(num_t::abs(p - me.Location.X) >= w / 2 && p < me.Location.X + me.Location.Width)
        {
            p += w;
        }
        me.Location.X = p;
    }

    me.SpecialX = me.Location.X;
    treeNPCUpdate(&me);
    if(me.tempBlock != 0)
        treeNPCSplitTempBlock(&me);
}

#if 0
// previous recursive version of the function

void SkullRideDone(int A, const Location_t &alignAt)
{
    auto &me = NPC[A];

    Location_t loc = me.Location;
    loc.Width += 16;
    loc.X -= 8;
    loc.Height += 30;
    loc.Y -= 15;

    for(int B = 1; B <= numNPCs; B++) // Recursively DE-activate all neighbour skull-ride segments
    {
        auto &npc = NPC[B];
        if(npc.Type == NPCID_RAFT)
        {
            if(npc.Active)
            {
                if(npc.Special == 1.0)
                {
                    if(CheckCollision(loc , npc.Location))
                    {
                        npc.Special = 2;
                        npc.Location.SpeedX = 0.0;
                        s_alignRuftCell(npc, alignAt);
                        SkullRideDone(B, alignAt);
                    }
                }
            }
        }
    }
}
#endif


// this mimics the above functions and should match it in all cases
void SkullRide(int A, bool reEnable, const Location_t *alignAt)
{
    int spec = (alignAt) ? 1 : (reEnable ? 2 : 0);

    // allocates and guards a vector of BaseRef_t in its i_vec.
    TreeResult_Sentinel<NPCRef_t> frontier;

    // LIFO stack of NPCs to trigger, imitates recursion from old SkullRide
    std::vector<BaseRef_t>& vec = *frontier.i_vec;

    vec.push_back(A);

    while(!vec.empty())
    {
        // pop from top of stack
        const NPC_t& me = static_cast<NPCRef_t>(vec.back());
        vec.pop_back();

        // construct location query (head of original SkullRide)
        Location_t loc = me.Location;
        loc.Width += 16;
        loc.X -= 8;

        if(g_config.fix_skull_raft) // Detect by height in condition skull ruft cells were on slopes
        {
            loc.Height += 30;
            loc.Y -= 15;
        }

        // add all queried NPCs to the frontier's internal vector (but check them, and remove them if invalid)
        size_t current_frontier_size = vec.size();

        treeNPCQuery(vec, loc, SORTMODE_NONE);

        size_t unchecked = current_frontier_size;
        while(unchecked < vec.size())
        {
            NPC_t& npc = static_cast<NPCRef_t>(vec[unchecked]);

            // conditions from inside of loop in original SkullRide
            if(npc.Type == NPCID_RAFT && npc.Active && npc.Special == spec && CheckCollision(loc, npc.Location))
            {
                if(alignAt)
                {
                    npc.Special = 2;
                    npc.Location.SpeedX = 0;
                    s_alignRuftCell(npc, *alignAt);
                }
                else
                {
                    // set active flag
                    npc.Special = 1;
                }

                // previously, used recursion
                // SkullRide(B, reEnable);

                // this will preserve the NPC in the stack
                unchecked++;
                continue;
            }

            // remove the NPC from the stack if conditions fail
            vec[unchecked] = vec[vec.size() - 1];
            vec.pop_back();
        }

        // reverse sort the frontier of the stack because this is a depth-first approach
        NPCQueues::reverse_sort(
            reinterpret_cast<int16_t*>(vec.data() + current_frontier_size),
            reinterpret_cast<int16_t*>(vec.data() + vec.size())
        );
    }
}

static void s_NPCSpawnCustomThrown(NPC_t& thrower, NPC_t& new_npc)
{
    new_npc.Location.Width = new_npc->TWidth;
    new_npc.Location.Height = new_npc->THeight;
    new_npc.Variant = thrower.Variant;

    num_t floor_y = thrower.Location.Y + thrower.Location.Height - new_npc.Location.Height;
    if(new_npc.Location.Y > floor_y)
        new_npc.Location.Y = floor_y;

    new_npc.Immune = 12;

    if(new_npc->IsACoin || new_npc.Type == NPCID_SLIDE_BLOCK)
    {
        if(NPC[numNPCs]->IsACoin)
            NPC[numNPCs].Location.SpeedX /= 2;

        new_npc.Special = 1;
    }
    else if(new_npc.Type == NPCID_HOMING_BALL)
        new_npc.Special2 = 95;

    if(new_npc->IsABlock)
    {
        if(new_npc.Direction > 0)
            new_npc.Location.X = thrower.Location.X + thrower.Location.Width;
        else
            new_npc.Location.X = thrower.Location.X - new_npc.Location.Width;
    }

    if(new_npc.Type != NPCID_BULLET && new_npc.Type != NPCID_BOMB && new_npc.Type != NPCID_HOMING_BALL
        && new_npc.Type != NPCID_HOMING_BALL_GEN && new_npc.Type != NPCID_STACKER && new_npc.Type != NPCID_WALK_PLANT
        && new_npc.Type != NPCID_VINE_BUG && new_npc.Type != NPCID_BOSS_CASE && new_npc.Type != NPCID_FLY
        && !new_npc->IsACoin)
    {
        new_npc.Projectile = true;
    }

    if(thrower.Inert && !(new_npc->IsACoin || new_npc->IsABonus || new_npc->WontHurt))
        new_npc.Inert = true;
}

void NPCSpecial(int A)
{
    // double C = 0;
    // double D = 0;
    // double E = 0;
    // double F = 0;
    // int64_t fBlock = 0;
    // int64_t lBlock = 0;
    // bool straightLine = false; // SET BUT NOT USED
    bool tempBool = false;
    // bool tempBool2 = false;
    // Location_t tempLocation;
    // NPC_t tempNPC;
    auto &npc = NPC[A];

    if(npc.Type == NPCID_BOSS_CASE) // previously included many other bosses, now moved into their logic
    {
        // dont despawn
        if(npc.TimeLeft > 1)
            npc.TimeLeft = 100;
    }
    else if(npc.Type == NPCID_RED_VINE_TOP_S3 || npc.Type == NPCID_GRN_VINE_TOP_S3 || npc.Type == NPCID_GRN_VINE_TOP_S4) // Vine Maker
    {
        npc.Location.SpeedY = -2;
        Location_t tempLocation;
        tempLocation.Height = 28;
        tempLocation.Width = 30;
        tempLocation.Y = npc.Location.Y + (npc.Location.Height - tempLocation.Height) / 2;
        tempLocation.X = npc.Location.X + (npc.Location.Width - tempLocation.Width) / 2;
        tempBool = false;

        for(int i : treeNPCQuery(tempLocation, SORTMODE_NONE))
        {
            auto &n = NPC[i];
            if(n->IsAVine && !n.Hidden && CheckCollision(tempLocation, n.Location))
            {
                tempBool = true;
                break;
            }
        }

        // this block had no effect because the inner `tempBool = True` was commented in the original VB6 code
#if 0
        if(!tempBool)
        {
            // fBlock = FirstBlock[static_cast<int>(floor(static_cast<double>(tempLocation.X / 32))) - 1];
            // lBlock = LastBlock[floor((tempLocation.X + tempLocation.Width) / 32.0) + 1];

            for(Block_t* block : treeFLBlockQuery(tempLocation, false))
            {
                auto &b = *block;
                if(!b.Hidden && !BlockNoClipping[b.Type] && !BlockIsSizable[b.Type] && !BlockOnlyHitspot1[b.Type])
                {
                    if(CheckCollision(tempLocation, b.Location) && BlockSlope[b.Type] == 0)
                    {
                        // tempBool = True
                    }
                }
            }
        }
#endif
        // ============================================================

        if(!tempBool || npc.Special == 1)
        {
            numNPCs++;
            if(npc.Type == NPCID_RED_VINE_TOP_S3)
                NPC[numNPCs].Type = NPCID_RED_VINE_S3;
            else if(npc.Type == NPCID_GRN_VINE_TOP_S3)
                NPC[numNPCs].Type = NPCID_GRN_VINE_S3;
            else if(npc.Type == NPCID_GRN_VINE_TOP_S4)
                NPC[numNPCs].Type = NPCID_GRN_VINE_S4;
            NPC[numNPCs].Location.Y = num_t::vb6round(npc.Location.Y / 32) * 32;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
            NPC[numNPCs].Location.X = npc.Location.X + (npc.Location.Width - NPC[numNPCs].Location.Width) / 2;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Section = npc.Section;
            NPC[numNPCs].DefaultLocationX = NPC[numNPCs].Location.X;
            NPC[numNPCs].DefaultLocationY = NPC[numNPCs].Location.Y;
            NPC[numNPCs].DefaultLocationHeight_Force32 = true;
            NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
            NPC[numNPCs].Layer = npc.Layer;
            NPC[numNPCs].Shadow = npc.Shadow;
            syncLayers_NPC(numNPCs);
        }

        if(npc.Special == 1)
        {
            npc.Killed = 9;
            NPCQueues::Killed.push_back(A);
        }

        // driving block
    }
    else if(npc.Type == NPCID_COCKPIT)
    {
        // Previously, used Special4 (1 if being driven), Special5 (-1 for left, 1 for right), and Special6 (-1 for up, 1 for down)
        // Now, uses a bitfield in Special4

        // if(npc.Special4 > 0)
        //     npc.Special4 = 0;
        // else
        // {
        //     npc.Special5 = 0;
        //     npc.Special6 = 0;
        // }

        // if(npc.Special5 > 0)
        if(npc.Special4 & NPC_COCKPIT_RIGHT)
        {
            if(npc.Location.SpeedX < 0)
                npc.Location.SpeedX = npc.Location.SpeedX * 0.95_r;
            npc.Location.SpeedX += 0.1_n;
        }
        // else if(npc.Special5 < 0)
        else if(npc.Special4 & NPC_COCKPIT_LEFT)
        {
            if(npc.Location.SpeedX > 0)
                npc.Location.SpeedX = npc.Location.SpeedX * 0.95_r;
            npc.Location.SpeedX -= 0.1_n;
        }
        else
        {
            npc.Location.SpeedX = npc.Location.SpeedX * 0.95_r;
            if(npc.Location.SpeedX > -0.1_n && npc.Location.SpeedX < 0.1_n)
                npc.Location.SpeedX = 0;
        }

        // if(npc.Special6 > 0)
        if(npc.Special4 & NPC_COCKPIT_DOWN)
        {
            if(npc.Location.SpeedY < 0)
                npc.Location.SpeedY = npc.Location.SpeedY * 0.95_r;
            npc.Location.SpeedY += 0.1_n;
        }
        // else if(npc.Special6 < 0)
        else if(npc.Special4 & NPC_COCKPIT_UP)
        {
            if(npc.Location.SpeedY > 0)
                npc.Location.SpeedY = npc.Location.SpeedY * 0.95_r;
            npc.Location.SpeedY -= 0.1_n;
        }
        else
        {
            npc.Location.SpeedY = npc.Location.SpeedY * 0.95_r;
            if(npc.Location.SpeedY > -0.1_n && npc.Location.SpeedY < 0.1_n)
                npc.Location.SpeedY = 0;
        }

        // reset after reading
        npc.Special4 = 0;

        if(npc.Location.SpeedY > 4)
            npc.Location.SpeedY = 4;

        if(npc.Location.SpeedY < -4)
            npc.Location.SpeedY = -4;

        if(npc.Location.SpeedX > 6)
            npc.Location.SpeedX = 6;

        if(npc.Location.SpeedX < -6)
            npc.Location.SpeedX = -6;

    }
    else if(npc.Type == NPCID_CHAR3_HEAVY) // heart bomb
    {
        if(npc.Special4 != 0)
        {
            npc.Killed = 9;
            NPCQueues::Killed.push_back(A);

            int C = npc.BattleOwner;

            if(npc.CantHurtPlayer > 0)
                C = npc.CantHurtPlayer;

            Bomb(npc.Location, 0, C);

            for(int i = 1; i <= 5; i++)
            {
                NewEffect(EFFID_SPARKLE, newLoc(npc.Location.X + npc.Location.Width / 2 - 4, npc.Location.Y + npc.Location.Height / 2 - 6), 1, 0, npc.Shadow);
                Effect[numEffects].Location.SpeedX = dRand() * 6 - 3;
                Effect[numEffects].Location.SpeedY = dRand() * 6 - 3;
                Effect[numEffects].Frame = iRand(3);
            }
        }
    }
    else if(npc.Type == NPCID_VINE_BUG)
    {
        if(!npc.Projectile)
        {
            npc.Location.SpeedX = 0;
            if(npc.Location.SpeedY < 0)
                npc.Special = -1;
            else
                npc.Special = 1;

            Location_t tempLocation = npc.Location;

            if(npc.Special == -1)
                tempLocation.Y -= 1;
            else
                tempLocation.Y += tempLocation.Height + 1;

            tempLocation.Height = 1;
            tempBool = false;

            for(int i : treeNPCQuery(tempLocation, SORTMODE_NONE))
            {
                auto &n = NPC[i];
                if(n.Active && !n.Hidden && n->IsAVine && CheckCollision(tempLocation, n.Location))
                {
                    tempBool = true;
                    break;
                }
            }

            if(!tempBool)
            {
                for(BackgroundRef_t i : treeBackgroundQuery(tempLocation, SORTMODE_NONE))
                {
                    if((int)i > numBackground)
                        continue;

                    Background_t &b = i;
                    if(!b.Hidden && ((b.Type >= 174 && b.Type <= 186) || b.Type == 63) && CheckCollision(tempLocation, b.Location))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }

            if(tempBool)
            {
                if(npc.Special == 1)
                    npc.Location.SpeedY = 2;
                else
                    npc.Location.SpeedY = -1;
            }
            else
            {
                if(npc.Special == -1)
                {
                    npc.Location.SpeedY = 2;
                    npc.Special = 2;
                }
                else
                {
                    npc.Location.SpeedY = -1;
                    npc.Special = -1;
                }
            }
        }
        else
        {
            npc.Location.SpeedY += Physics.NPCGravity;
            npc.Location.SpeedX *= 0.987_r;

            if(npc.Location.SpeedX > -0.1_n && npc.Location.SpeedX < 0.1_n)
                npc.Location.SpeedX = 0;
        }
    }
    else if(npc.Type == NPCID_RANDOM_POWER)
        npc.Type = RandomBonus();
    else if(npc.Type == NPCID_SPIKY_BALL_S4) // falling spiney
    {
        if(npc.Special != 0)
        {
            npc.Type = NPCID_SPIKY_S4;
            npc.Special = 0;

            num_t min_dist = 0;

            for(int i = 1; i <= numPlayers; i++)
            {
                auto &p = Player[i];
                if(!p.Dead && p.Section == npc.Section && p.TimeToLive == 0)
                {
                    num_t horiz_dist = NPCPlayerTargetDist(npc, p);
                    if(min_dist == 0 || horiz_dist < min_dist)
                    {
                        min_dist = horiz_dist;
                        if(npc.Location.to_right_of(p.Location))
                            npc.Direction = -1;
                        else
                            npc.Direction = 1;
                    }
                }
            }

            npc.Location.SpeedX = Physics.NPCWalkingOnSpeed * npc.Direction;
        }
    }
    else if(npc.Type == NPCID_ITEM_BUBBLE) // bubble
    {
        npc.Wings = WING_NONE;

        if(npc.Special == NPCID_RANDOM_POWER)
        {
            npc.Special = RandomBonus();
            npc.DefaultSpecial = npc.Special;
        }

        npc.Location.SpeedX = 0.75_n * npc.DefaultDirection;
        if(npc.Special2 == 0)
            npc.Special2 = -1;

        npc.Location.SpeedY += 0.05_n * npc.Special2;
        if(npc.Location.SpeedY > 1)
        {
            npc.Location.SpeedY = 1;
            npc.Special2 = -1;
        }
        else if(npc.Location.SpeedY < -1)
        {
            npc.Location.SpeedY = -1;
            npc.Special2 = 1;
        }

        if(npc.Special3 > 0)
        {
            NewEffect(EFFID_BUBBLE_POP, npc.Location);
            PlaySoundSpatial(SFX_Bubble, npc.Location);
            npc.Type = NPCID(npc.Special);
            npc.Special3 = 0;
            npc.Special2 = 0;
            npc.Special = 0;
            npc.Frame = EditorNPCFrame(npc.Type, npc.Direction);
            npc.FrameCount = 0;
            npc.Location.set_width_center(npc->TWidth);
            npc.Location.set_height_center(npc->THeight);
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;
            npc.Direction = npc.DefaultDirection;
            npc.Wings = npc.DefaultWings;

            NPCQueues::update(A);

            // deferring tree update to end of the NPC physics update

            if(npc->IsACoin)
            {
                npc.Special = 1;
                npc.Location.SpeedX = dRand() - 0.5_n;
            }

            if(npc.Direction == 0)
            {
                if(iRand(2) == 0)
                    npc.Direction = 1;
                else
                    npc.Direction = -1;
            }

            npc.TurnAround = false;
            if(npc.Type == NPCID_BOMB)
                npc.Projectile = true;
        }

    }
    else if(npc.Type == NPCID_QUAD_SPITTER) // fire plant thing
    {
        if(npc.Special == 0)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 170)
            {
                npc.Special2 = 0;
                npc.Special = 1;
            }
        }
        else if(npc.Special == 1)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 70)
            {
                npc.Special2 = 0;
                npc.Special = 2;
            }
        }
        else if(npc.Special == 2)
        {
            if(npc.Special2 == 0) // spit fireballs
            {
                for(int i = 1; i <= 4; i++)
                {
                    numNPCs++;
                    NPC[numNPCs].Type = NPCID_QUAD_BALL;
                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                    NPC[numNPCs].Section = npc.Section;
                    NPC[numNPCs].TimeLeft = npc.TimeLeft;
                    NPC[numNPCs].Location.X = npc.Location.X + (npc.Location.Width - NPC[numNPCs].Location.Width) / 2;
                    NPC[numNPCs].Location.Y = npc.Location.Y - NPC[numNPCs].Location.Height;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Layer = npc.Layer;

                    if(i == 1 || i == 4)
                    {
                        NPC[numNPCs].Location.SpeedX = -4.0_n; // -2.5 * 1.6
                        NPC[numNPCs].Location.SpeedY = -2.4_n; // -1.5 * 1.6
                    }
                    else
                    {
                        NPC[numNPCs].Location.SpeedX = -1.6_n; // -1.0 * 1.6
                        NPC[numNPCs].Location.SpeedY = -3.2_n; // -2 * 1.6
                    }

                    if(i == 3 || i == 4)
                        NPC[numNPCs].Location.SpeedX = -NPC[numNPCs].Location.SpeedX;

                    // NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.6;
                    // NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.6;

                    syncLayers_NPC(numNPCs);
                }
            }

            npc.Special2 += 1;
            if(npc.Special2 >= 50)
            {
                npc.Special2 = 0;
                npc.Special = 0;
            }
        }
    }
    else if(npc.Type == NPCID_QUAD_BALL) // plant fireballs
    {
        if(npc.Special == 0)
        {
            npc.Location.SpeedY *= 0.98_r;
            npc.Location.SpeedX *= 0.98_r;
            if(npc.Location.SpeedY > -0.5_n)
            {
                npc.Location.SpeedX = 0;
                npc.Location.SpeedY = 0;
                npc.Special2 = 0;
                npc.Special = 1;
            }
        }
        else
        {
            npc.Location.SpeedY += 0.02_n;
            if(npc.Location.SpeedY > 2)
                npc.Location.SpeedY = 2;

            if(npc.Location.SpeedY > 0.25_n)
            {
                npc.Special2 += 1;
                if(npc.Special2 < 7)
                    npc.Location.SpeedX = -0.8_n;
                else if(npc.Special2 < 13)
                    npc.Location.SpeedX = 0.8_n;
                else
                {
                    npc.Special2 = 0;
                    npc.Location.SpeedX = 0;
                }
            }

        }
    }
    else if(npc.Type == NPCID_BAT) // bat thing
    {
        // Special is an activation flag
        // SpecialY (previously Special2) is the target Y coordinate
        if(npc.Special == 0)
        {
            if(!g_config.fix_bat_start_while_inactive || npc.Active)
            {
                for(int i = 1; i <= numPlayers; i++)
                {
                    auto &p = Player[i];
                    if(p.Section == npc.Section && !p.Dead && p.TimeToLive == 0)
                    {
                        Location_t tempLocation = npc.Location;
                        tempLocation.Width = 400;
                        tempLocation.Height = 800;
                        tempLocation.X -= tempLocation.Width / 2;
                        tempLocation.Y -= tempLocation.Height / 2;

                        if(CheckCollision(tempLocation, p.Location))
                        {
                            npc.Special = 1;
                            if(p.Location.X < npc.Location.X)
                                npc.Direction = -1;
                            else
                                npc.Direction = 1;
                            npc.Location.SpeedX = 0.01_n * npc.Direction;

                            if(p.Location.Y > npc.Location.Y)
                            {
                                npc.Location.SpeedY = 2.5_n;
                                npc.SpecialY = p.Location.Y - 130;
                            }
                            else
                            {
                                npc.Location.SpeedY = -2.5_n;
                                npc.SpecialY = p.Location.Y + 130;
                            }

                            if(npc.Active)
                                PlaySoundSpatial(SFX_BatFlap, npc.Location);
                        }
                    }
                }
            }
        }
        else
        {
            npc.Location.SpeedX = (3 - num_t::abs(npc.Location.SpeedY)) * npc.Direction;
            if((npc.Location.SpeedY > 0 && npc.Location.Y > npc.SpecialY) || (npc.Location.SpeedY < 0 && npc.Location.Y < npc.SpecialY))
            {
                npc.Location.SpeedY = npc.Location.SpeedY * 0.98_r;
                if(npc.Location.SpeedY > -0.1_n && npc.Location.SpeedY < 0.1_n)
                    npc.Location.SpeedY = 0;
            }
        }
    }
    else if(npc.Type == NPCID_MAGIC_BOSS_BALL) // larry magic
    {
        if(npc.Special < 2)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 30 && npc.Special != 2)
            {
                npc.Location.X += npc.Location.Width;
                npc.Location.Width = 16;
                npc.Location.set_height_floor(32);
                npc.Special = 2;
                npc.Location.X += -npc.Location.Width;
                npc.Special2 = 21;

                NPCQueues::Unchecked.push_back(A);
                // deferring tree update to end of the NPC physics update
            }
            else if(npc.Special2 >= 15 && npc.Special != 1)
            {
                npc.Location.X += npc.Location.Width;
                npc.Location.Width = 10;
                npc.Location.set_height_floor(20);
                npc.Special = 1;
                npc.Location.X += -npc.Location.Width;

                NPCQueues::Unchecked.push_back(A);
                // deferring tree update to end of the NPC physics update
            }
        }
    }
    else if(npc.Type == NPCID_MAGIC_BOSS_SHELL || npc.Type == NPCID_FIRE_BOSS_SHELL) // larry/ludwig shell
    {
        // temporarily remove any wings if present
        npc.Wings = WING_NONE;

        if(npc.Special5 == 0) // Target a Random Player
        {
            int C = 0;
            do
            {
                // COMPAT BUGFIX. Should be iRand(numPlayers) + 1
                int i = (g_config.fix_multiplayer_targeting) ? (iRand(numPlayers) + 1) : (iRand(1) * numPlayers + 1); // always 1
                if(!Player[i].Dead && Player[i].Section == npc.Section && Player[i].TimeToLive == 0)
                    npc.Special5 = i;
                C += 1;
                if(C >= 20)
                    npc.Special5 = 1;
            }
            while(npc.Special5 <= 0);
        }

        if(npc.Location.to_right_of(Player[npc.Special5].Location))
            npc.Direction = -1;
        else
            npc.Direction = 1;
        if(npc.Special == 0)
        {
            npc.Special2 += 1;
            if(npc.Special2 >= 60)
            {
                npc.Special = 1;
                npc.Special2 = 0;
            }
        }
        else if(npc.Special == 1)
        {
            if(npc.Type == NPCID_FIRE_BOSS_SHELL && num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
                npc.Location.SpeedX += 0.1_n * npc.Direction;
            else
                npc.Location.SpeedX += 0.2_n * npc.Direction;
            if(npc.Type == NPCID_FIRE_BOSS_SHELL && npc.Damage >= 5)
            {
                if(npc.Location.SpeedX > 5.5_n)
                    npc.Location.SpeedX = 5.5_n;
                else if(npc.Location.SpeedX < -5.5_n)
                    npc.Location.SpeedX = -5.5_n;
            }
            else if(npc.Type == NPCID_FIRE_BOSS_SHELL && npc.Damage >= 10)
            {
                if(npc.Location.SpeedX > 6)
                    npc.Location.SpeedX = 6;
                else if(npc.Location.SpeedX < -6)
                    npc.Location.SpeedX = -6;
            }
            else
            {
                if(npc.Location.SpeedX > 5)
                    npc.Location.SpeedX = 5;
                else if(npc.Location.SpeedX < -5)
                    npc.Location.SpeedX = -5;
            }

            if(npc.Type == NPCID_FIRE_BOSS_SHELL && num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special3 += 1;
                if((npc.Location.SpeedX < -2 && npc.Direction < 0) || (npc.Location.SpeedX > 2 && npc.Direction > 0))
                {
                    if(npc.Special3 >= 20 - npc.Damage * 2)
                    {
                        npc.Special3 = 0;
                        npc.Location.SpeedY = -3 - dRand() * 2;
                    }
                }
            }

            npc.Special2 += 1;

            if(npc.Special2 >= 300 && num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special = 2;
                npc.Special2 = 0;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedY = -5 - dRand() * 3;
            if(npc.Type == NPCID_FIRE_BOSS_SHELL)
                npc.Location.SpeedY -= 2;
            npc.Special = 3;
        }
        else if(npc.Special == 3)
        {
            if(npc.Location.SpeedX > 2.5_n)
                npc.Location.SpeedX -= 0.2_n;
            else if(npc.Location.SpeedX < -2.5_n)
                npc.Location.SpeedX += 0.2_n;
            npc.Special2 += 1;

            if(npc.Type == NPCID_FIRE_BOSS_SHELL)
            {
                npc.Special2 = 20;
                PlaySoundSpatial(SFX_Spring, npc.Location);
            }

            if(npc.Special2 == 20)
            {
                npc.Special = 0;
                npc.Special2 = npc.Direction;
                npc.Special3 = 0;
                npc.Special4 = 0;
                npc.Special5 = 0;
                // npc.Special6 = 0;
                npc.Type = NPCID(npc.Type - 1);
                npc.Location.set_width_center(npc->TWidth);
                npc.Location.set_height_floor(npc->THeight);

                // restore wings if lost
                npc.Wings = npc.DefaultWings;

                NPCQueues::Unchecked.push_back(A);
                // deferring tree update to end of the NPC physics update
            }
        }
        else
            npc.Special = 0;

    }
    else if(npc.Type == NPCID_MAGIC_BOSS) // larry koopa
    {
        // special is phase
        // special5 is targetted player
        // special3 is jump counter
        // special2 is direction

        // special4 is attack timer
        // special3 is attack count
        if(npc.Inert)
        {
            npc.Special4 = 0;
            npc.Special3 = 0;
        }

        if(npc.Special5 == 0) // Target a Random Player
        {
            int C = 0;
            do
            {
                // COMPAT BUGFIX. Should be iRand(numPlayers) + 1
                int i = (g_config.fix_multiplayer_targeting) ? (iRand(numPlayers) + 1) : (iRand(1) * numPlayers + 1); // always 1
                // should be int i = iRand(numPlayers) + 1;
                if(!Player[i].Dead && Player[i].Section == npc.Section && Player[i].TimeToLive == 0)
                    npc.Special5 = i;
                C += 1;
                if(C >= 20)
                    npc.Special5 = 1;
            }
            while(npc.Special5 <= 0);
        }

        if(npc.Location.to_right_of(Player[npc.Special5].Location))
            npc.Direction = -1;
        else
            npc.Direction = 1;

        if(npc.Special2 == 0)
            npc.Special2 = npc.Direction;

        if(npc.Special == 0)
        {
            if(npc.Special2 == -1)
                npc.Location.SpeedX = -2.5_n;
            else
                npc.Location.SpeedX = 2.5_n;

            // movement
            if(npc.Location.X < Player[npc.Special5].Location.X - 400)
                npc.Special2 = 1;
            else if(npc.Location.X > Player[npc.Special5].Location.X + 400)
                npc.Special2 = -1;

            // jumping
            if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Special3 += 1;
                if(npc.Special3 > 30 + iRand(100))
                {
                    npc.Special3 = 0;
                    npc.Location.SpeedY = -5 - dRand() * 4;
                }
            }
            else
                npc.Special3 = 0;

            // attack timer
            npc.Special4 += 1;
            if(npc.Special4 > 100 + iRand(100) && (num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity) || npc.Wings))
            {
                npc.Special = 1;
                npc.Special5 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
            }
        }
        else if(npc.Special == 1)
        {
            if(npc.Location.to_right_of(Player[npc.Special5].Location))
                npc.Direction = -1;
            else
                npc.Direction = 1;

            npc.Special2 = npc.Direction;
            npc.Location.SpeedX = 0;
            npc.Special3 += 1;

            if(npc.Special3 >= 10)
            {
                npc.Special3 = 0;
                npc.Special = 2;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedX = 0;
            if(npc.Special3 == 0 || npc.Special3 == 6 || npc.Special3 == 12) // shoot
            {

                if(npc.Special3 == 0)
                {
                    npc.SpecialX = Player[npc.Special5].Location.X + Player[npc.Special5].Location.Width / 2;
                    npc.SpecialY = Player[npc.Special5].Location.Y + Player[npc.Special5].Location.Height / 2 + 16;
                }

                if(npc.Special3 == 0)
                    PlaySoundSpatial(SFX_Transform, npc.Location);

                numNPCs++;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Section = npc.Section;
                NPC[numNPCs].Type = NPCID_MAGIC_BOSS_BALL;
                NPC[numNPCs].Location.Width = 10;
                NPC[numNPCs].Location.Height = 8;
                NPC[numNPCs].Frame = 3;
                NPC[numNPCs].Special2 = npc.Special3;

                if(NPC[numNPCs].Direction == -1)
                    NPC[numNPCs].Location.X = npc.Location.X - 20;
                else
                    NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width - NPC[numNPCs].Location.Width + 20;

                NPC[numNPCs].Location.Y = npc.Location.Y + 47;

                NPCSetSpeedTarget_FixedX(NPC[numNPCs], npc.SpecialX, npc.SpecialY, 3, 3);

                syncLayers_NPC(numNPCs);
            }

            npc.Special3 += 1;
            if(npc.Special3 >= 30)
            {
                npc.Special = 0;
                npc.Special4 = 0;
                npc.Special5 = 0;
                // npc.Special6 = 0;
                npc.SpecialX = 0;
                npc.SpecialY = 0;
            }
        }

        // ludwig koopa
    }
    else if(npc.Type == NPCID_FIRE_BOSS)
    {
        // special is phase
        // special5 is targetted player
        // special3 is jump counter
        // special2 is direction

        // special4 is attack timer
        // special3 is attack count

        // SpecialX (Special6 in SMBX 1.3) appears to be an additional attack timer

        if(npc.Inert)
        {
            npc.Special4 = 0;
            npc.Special3 = 0;
            npc.SpecialX = 0;
        }

        if(npc.Special5 == 0) // Target a Random Player
        {
            int C = 0;
            do
            {
                // COMPAT BUGFIX. Should be iRand(numPlayers) + 1
                int i = (g_config.fix_multiplayer_targeting) ? (iRand(numPlayers) + 1) : (iRand(1) * numPlayers + 1); // always 1
                if(!Player[i].Dead && Player[i].Section == npc.Section && Player[i].TimeToLive == 0)
                    npc.Special5 = i;
                C += 1;
                if(C >= 20)
                    npc.Special5 = 1;
            }
            while(npc.Special5 <= 0); // TESTME: !(npc.Special5 > 0)
        }

        if(npc.Location.to_right_of(Player[npc.Special5].Location))
            npc.Direction = -1;
        else
            npc.Direction = 1;

        if(npc.Special2 == 0)
            npc.Special2 = npc.Direction;

        if(npc.Special == 0)
        {
            if((npc.Damage < 5 && npc.SpecialX > 60) || (npc.Damage < 10 && npc.SpecialX > 80) || npc.Inert)
            {
                if(npc.Special2 == -1)
                    npc.Location.SpeedX = -1.5_n;
                else
                    npc.Location.SpeedX = 1.5_n;

                // movement
                if(npc.Location.X < Player[npc.Special5].Location.X - 400)
                    npc.Special2 = 1;
                else if(npc.Location.X > Player[npc.Special5].Location.X + 400)
                    npc.Special2 = -1;
            }
            else
            {
                npc.Location.SpeedX = npc.Location.SpeedX * 0.98_r;
                if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
                    npc.Location.SpeedX = 0;

            }

            // attack timer
            bool can_advance = (num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity) || npc.Wings);
            if(can_advance)
                npc.SpecialX += 1;

            if(npc.SpecialX == 20 || npc.SpecialX == 40 || npc.SpecialX == 60 || (npc.Damage >= 5 && npc.SpecialX == 80) || (npc.Damage >= 10 && npc.SpecialX == 100))
            {
                npc.Special = 1;
                npc.Special5 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
            }

            if(npc.Damage >= 10 && npc.Special == 0 && npc.SpecialX >= 100)
                npc.SpecialX = 200;

            if(npc.SpecialX >= 160 && can_advance)
            {
                npc.SpecialX = 0;
                npc.Special = 3;
                PlaySoundSpatial(SFX_Spring, npc.Location);
                npc.Location.SpeedY = -7 - dRand() * 2;
                // clip wings if present
                npc.Wings = WING_NONE;
            }
        }
        else if(npc.Special == 3)
        {
            if(npc.Location.SpeedY > 0) // turn into shell
            {
                npc.Special = 1;
                npc.Special2 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
                npc.Special5 = 0;
                // npc.Special6 = 0;
                npc.SpecialX = 0;
                npc.Type = NPCID_FIRE_BOSS_SHELL;
                npc.Location.set_width_center(npc->TWidth);
                npc.Location.set_height_floor(npc->THeight);

                NPCQueues::Unchecked.push_back(A);
                // deferring tree update to end of the NPC physics update
            }
            else
            {
                npc.Location.SpeedX += 0.2_n * npc.Special2;
                if(npc.Location.SpeedX > 5)
                    npc.Location.SpeedX = 5;
                else if(npc.Location.SpeedX < -5)
                    npc.Location.SpeedX = -5;
            }
        }
        else if(npc.Special == 1 || npc.Special == 2)
        {
            if(npc.Location.to_right_of(Player[npc.Special5].Location))
                npc.Direction = -1;
            else
                npc.Direction = 1;

            npc.Special2 = npc.Direction;
            npc.Location.SpeedX = 0;

            if(npc.Special3 == 20) // shoot
            {
                PlaySoundSpatial(SFX_BigFireball, npc.Location);
                numNPCs++;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Section = npc.Section;
                NPC[numNPCs].Type = NPCID_FIRE_BOSS_FIRE;
                NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                NPC[numNPCs].Frame = 0;

                if(NPC[numNPCs].Direction == -1)
                    NPC[numNPCs].Location.X = npc.Location.X - 24;
                else
                    NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width - NPC[numNPCs].Location.Width + 24;
                NPC[numNPCs].Location.Y = npc.Location.Y + 4;

                NPCSetSpeedTarget_FixedX(NPC[numNPCs], Player[npc.Special5].Location, 4, 2);

                syncLayers_NPC(numNPCs);
            }

            npc.Special3 += 1;
            if(npc.Special3 < 20)
                npc.Special = 1;
            else
                npc.Special = 2;

            if(npc.Special3 >= 40)
            {
                npc.Special = 0;
                npc.Special5 = 0;
                npc.Special3 = 0;
                npc.Special4 = 0;
            }
        }
    }
    else if(npc.Type == NPCID_SWORDBEAM) // link sword beam
    {
        npc.Special += 1;
        if(npc.Special == 40)
        {
            npc.Killed = 9;
            NPCQueues::Killed.push_back(A);
        }

        // unified sparkle code (previously there was a 1-4 loop above, and the iRand(2) code was below)
        for(int i = 1; i <= 5; i++)
        {
            if((i < 5 && npc.Special == 40)
                || (i == 5 && iRand(2) == 0))
            {
                Location_t tempLocation;
                tempLocation.Height = EffectHeight[EFFID_SPARKLE];
                tempLocation.Width = EffectWidth[EFFID_SPARKLE];
                tempLocation.SpeedX = 0;
                tempLocation.SpeedY = 0;
                tempLocation.X = npc.Location.X + dRand() * 16 - (num_t)EffectWidth[EFFID_SPARKLE] / 2 - 4; // + .Location.SpeedX
                tempLocation.Y = npc.Location.Y + dRand() * 4 - (num_t)EffectHeight[EFFID_SPARKLE] / 2 - 2;
                NewEffect(EFFID_SPARKLE, tempLocation);

                if(i < 5)
                {
                    Effect[numEffects].Location.SpeedX = npc.Location.SpeedX * 0.3_r + dRand() * 2 - 1;
                    Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5_n;
                    Effect[numEffects].Frame = iRand(3);
                }
                else
                {
                    Effect[numEffects].Location.SpeedX = npc.Location.SpeedX * 0.15_r;
                    Effect[numEffects].Location.SpeedY = npc.Location.SpeedY; // + Rnd * 2 - 1
                    Effect[numEffects].Frame = iRand(2) + 1;
                }
            }
        }
    }
    else if(npc.Type == NPCID_BOMBER_BOSS) // mouser
    {
        // don't despawn -- moved here from the top of the routine
        if(npc.TimeLeft > 1)
            npc.TimeLeft = 100;

        if(npc.Immune == 0)
            NPCFaceNearestPlayer(npc);
        else
        {
            if(iRand(10) == 0)
                npc.Direction = -npc.Direction;
            npc.Special3 = 0;
            npc.Special4 = 0;
        }

        if(npc.Immune != 0)
            npc.Location.SpeedX = 0;
        else if(npc.Special == 0)
        {
            if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity) || npc.Wings)
            {
                if(npc.Special2 == 0)
                    npc.Special2 = npc.Direction;

                npc.Location.SpeedX = 2 * npc.Special2;

                if(npc.Location.X < npc.DefaultLocationX - 64)
                    npc.Special2 = 1;
                else if(npc.Location.X > npc.DefaultLocationX + 64)
                    npc.Special2 = -1;

                npc.Special3 += 1;
                npc.Special4 += 1;

                if(npc.Special3 > 100 + iRand(200))
                {
                    npc.Special3 = 0;
                    npc.Location.SpeedX = 0;
                    npc.Location.SpeedY = -5;
                }
                else if(npc.Special4 > 20 + iRand(200))
                {
                    npc.Special4 = 0;
                    npc.Special = -10;
                }
            }
            else
                npc.Location.SpeedX = 0;
        }
        else
        {
            if(npc.Special == -1)
            {
                npc.Special = 20;
                numNPCs++;
                NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Type = NPCID_BOMB;
                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                NPC[numNPCs].Location.Y = npc.Location.Y + npc.Location.Height - 48;
                NPC[numNPCs].Location.X = npc.Location.X + (npc.Location.Width - NPC[numNPCs].Location.Width) / 2 - 12 * NPC[numNPCs].Direction;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Section = npc.Section;
                NPC[numNPCs].Location.SpeedX = (5 + dRand() * 3) * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedY = -5 - (dRand() * 3);

                syncLayers_NPC(numNPCs);
            }

            npc.Location.SpeedX = 0;

            if(npc.Special < 0)
                npc.Special += 1;
            else
                npc.Special -= 1;
        }
    }
    else if(npc.Type == NPCID_WALK_PLANT) // muncher thing
    {
        if(npc.Special == 0)
        {
            for(int i = 1; i <= numPlayers; i++)
            {
                auto &p = Player[i];
                if(!p.Dead && p.TimeToLive == 0 && p.Section == npc.Section)
                {
                    Location_t tempLocation = npc.Location;
                    tempLocation.Height = 256;
                    tempLocation.Y -= tempLocation.Height;

                    if(CheckCollision(tempLocation, p.Location))
                    {
                        npc.Special = 1;
                        npc.Location.SpeedY = -7;
                        npc.Location.SpeedX = 0;
                    }
                }
            }
        }
        else if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
            npc.Special = 0;

        if(!npc.Stuck && npc.Special == 0)
        {
            if(npc.Special2 == 0)
            {
                if(npc.Location.X < npc.DefaultLocationX - 128 && npc.Direction == -1)
                    npc.Special2 = 60;
                else if(npc.Location.X > npc.DefaultLocationX + 128 && npc.Direction == 1)
                    npc.Special2 = 60;

                npc.Location.SpeedX = 1.4_n * npc.Direction;

                if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
                    npc.Location.SpeedY = -1.5_n;
            }
            else
            {
                npc.Special2 -= 1;

                if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
                    npc.Location.SpeedX = 0;

                if(npc.Special2 == 0)
                {
                    if(npc.Location.X < npc.DefaultLocationX)
                        npc.Direction = 1;
                    else
                        npc.Direction = -1;
                }
            }
        }
    }
    // newly combined (note that Special was previously the stage counter for FIRE_DISK; now Special2 is the counter for both)
    else if(npc.Type == NPCID_FIRE_CHAIN || npc.Type == NPCID_FIRE_DISK)
    {
        num_t C;
        num_t B;
        num_t thresh;

        if(npc.Type == NPCID_FIRE_DISK)
        {
            npc.Special4 += 1;

            if(npc.Special4 == 4)
            {
                NewEffect(EFFID_FIRE_DISK_DIE, npc.Location);
                Effect[numEffects].Frame = npc.Frame;
                Effect[numEffects].Location.SpeedX = 0;
                Effect[numEffects].Location.SpeedY = 0;
                npc.Special4 = 0;
            }

            C = 0.2_n;
            B = 6.05_n;
            thresh = 0_n;
        }
        else
        {
            C = 0.03_n * npc.DefaultSpecial;
            B = 0.98_n * npc.DefaultSpecial;
            thresh = 0.001_n;
        }

        if(npc.Special2 == 0)
        {
            npc.Location.SpeedX += C;
            npc.Location.SpeedY += C * npc.DefaultDirection;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedX = -B;
                npc.Location.SpeedY = 0;
            }

            if(npc.Location.SpeedX >= -thresh)
            {
                if(npc.DefaultDirection > 0)
                    npc.Special2 = 1;
                else if(npc.DefaultDirection < 0 || npc.Type == NPCID_FIRE_CHAIN)
                    npc.Special2 = 3;

                npc.Special5 = 0;
            }
        }
        else if(npc.Special2 == 1)
        {
            npc.Location.SpeedX += C * npc.DefaultDirection;
            npc.Location.SpeedY -= C;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedY = B;
                npc.Location.SpeedX = 0;
            }

            if(npc.Location.SpeedY <= thresh)
            {
                npc.Special2 += npc.DefaultDirection;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special2 == 2)
        {
            npc.Location.SpeedX -= C;
            npc.Location.SpeedY -= C * npc.DefaultDirection;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedX = B;
                npc.Location.SpeedY = 0;
            }

            if(npc.Location.SpeedX <= thresh)
            {
                npc.Special2 += npc.DefaultDirection;
                npc.Special5 = 0;
            }
        }
        else if(npc.Special2 == 3)
        {
            npc.Location.SpeedX -= C * npc.DefaultDirection;
            npc.Location.SpeedY += C;

            if(npc.Special5 == 0)
            {
                npc.Special5 = 1;
                npc.Location.SpeedY = -B;
                npc.Location.SpeedX = 0;
            }

            if(npc.Location.SpeedY >= -thresh)
            {
                if(npc.DefaultDirection > 0)
                    npc.Special2 = 0;
                else
                    npc.Special2 += npc.DefaultDirection;

                npc.Special5 = 0;
            }
        }
    }
    else if(npc.Type == NPCID_LOCK_DOOR)
    {
        for(int i : treeNPCQuery(npc.Location, SORTMODE_NONE))
        {
            auto &n = NPC[i];
            if(n.Type == NPCID_KEY && n.Active && n.HoldingPlayer != 0 && CheckCollision(npc.Location, n.Location))
            {
                n.Killed = 9;
                NewEffect(EFFID_SMOKE_S3, n.Location);
                npc.Killed = 3;
                NPCQueues::Killed.push_back(A);
                NPCQueues::Killed.push_back(i);
            }
        }
    }
    else if(npc.Type == NPCID_BOSS_FRAGILE) // Mother Brain
    {
        // don't despawn -- moved here from the top of the routine
        if(npc.TimeLeft > 1)
            npc.TimeLeft = 100;

        if(npc.Special >= 1)
        {
            // double B = 1 - (npc.Special / 45.0);
            // double C = B * 0.5;
            // B *= 15;
            // C *= 15;
            // npc.Location.X = npc.DefaultLocationX + dRand() * B - dRand() * C;
            // npc.Location.Y = npc.DefaultLocationY + dRand() * B - dRand() * C;

            int time_left = 45 - npc.Special;

            npc.Location.X = npc.DefaultLocationX + (dRand() * 2 - dRand()) * time_left / 6;
            npc.Location.Y = npc.DefaultLocationY + (dRand() * 2 - dRand()) * time_left / 6;

            npc.Special += 1;
            if(npc.Special >= 45)
                npc.Special = 0;

            // deferring tree update to end of the NPC physics update
        }
        else
        {
            npc.ResetLocation();
            NPCQueues::Unchecked.push_back(A);
            // deferring tree update to end of the NPC physics update
        }
    }
    else if(npc.Type == NPCID_HOMING_BALL) // O thing
    {
        if(npc.Special == 0)
        {
            npc.Special2 += 1;

            if(npc.Special2 > 80 + iRand(20))
            {
                npc.Special = 1;

                int ip = NPCTargetPlayer(npc);
                NPCSetSpeedTarget_FixedSpeed(npc, Player[ip].Location, 3);
            }
        }
    }
    else if(npc.Type == NPCID_HOMING_BALL_GEN) // Metroid O shooter thing
    {
        // moved from pre-NPCSpecial code
        npc.Location.SpeedX = 0;
        npc.Location.SpeedY = 0;

        // was Special in SMBX 1.3
        npc.SpecialY += 1 + dRand();
        if(npc.SpecialY >= 200 + dRand() * 200)
        {
            npc.SpecialY = 0;
            numNPCs++;
            NPC[numNPCs].Inert = npc.Inert;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 28;
            NPC[numNPCs].Location.X = npc.Location.X + 2;
            NPC[numNPCs].Location.Y = npc.Location.Y;
            NPC[numNPCs].Section = npc.Section;
            NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
            NPC[numNPCs].Type = NPCID_HOMING_BALL;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 50;

            syncLayers_NPC(numNPCs);
        }
    }
    else if(npc.Type == NPCID_WALL_SPARK || npc.Type == NPCID_WALL_BUG || npc.Type == NPCID_WALL_TURTLE) // sparky
    {
        // was F
        int speed_mult = (npc.Type == NPCID_WALL_SPARK) ? 2 : 1;

        // special stores wall attachment side, special2 stores current direction (1 is right, -1 is left)

        if(npc.Special == 0)
        {
            if(npc.Direction == 0)
            {
                if(iRand(2) == 1)
                    npc.Direction = 1;
                else
                    npc.Direction = -1;
            }

            npc.Special = 1;
            npc.Special2 = npc.Direction;
        }

        // handle floor slopes
        if(npc.Slope > 0 && (npc.Special == 2 || npc.Special == 4))
        {
            // "right side of a wall" -> moving left
            if(npc.Special == 2)
                npc.Special2 = 1;
            // "left side of a wall" -> moving left
            else if(npc.Special == 4)
                npc.Special2 = -1;

            // you're on a floor!
            npc.Special = 1;
        }

        // timer for being unsupported while on floor
        npc.Special5 += 1;
        if(npc.Special5 >= 8 && npc.Special == 1)
        {
            npc.Special5 = 8;
            npc.Special = 0;
            npc.Location.SpeedY += Physics.NPCGravity;
            if(npc.Location.SpeedY > 8)
                npc.Location.SpeedY = 8;
        }

        // the following code was previously separated into conditions 1/2/3/4
        if(npc.Special < 1 || npc.Special > 4)
            return;

        // set speed: push NPC into wall it is currently climbing
        // FIXME: is Special2 ever anything other than -1 or 1 at this point? Would be nice to remove the abs calls.
        if(npc.Special == 1)
        {
            npc.Location.SpeedY = speed_mult * std::abs(npc.Special2);
            npc.Location.SpeedX = speed_mult * npc.Special2;
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedY = speed_mult * npc.Special2;
            npc.Location.SpeedX = std::abs(npc.Special2);
        }
        else if(npc.Special == 3)
        {
            npc.Location.SpeedY = -std::abs(npc.Special2);
            npc.Location.SpeedX = speed_mult * npc.Special2;
        }
        else if(npc.Special == 4)
        {
            npc.Location.SpeedY = speed_mult * npc.Special2;
            npc.Location.SpeedX = -std::abs(npc.Special2);
        }

        // check whether the NPC is about to turn an inner corner (eg, hit a wall when on ground)
        bool inner_corner = false;
        Location_t tempLocation;
        tempLocation.Width = 2;
        tempLocation.Height = 2;
        tempLocation.X = npc.Location.X + npc.Location.Width / 2 - 1;
        tempLocation.Y = npc.Location.Y + npc.Location.Height / 2 - 1;

        // check the point 18px to the left/right of the NPC's center (horizontal climbing)
        if(npc.Special == 1 || npc.Special == 3)
            tempLocation.X += 18 * npc.Special2;
        // check the point 18px to the top/bottom of the NPC's center (vertical climbing)
        else if(npc.Special == 2 || npc.Special == 4)
            tempLocation.Y += 18 * npc.Special2;

        for(Block_t& block : treeFLBlockQuery(tempLocation, SORTMODE_NONE))
        {
            if(!block.Hidden && !BlockNoClipping[block.Type] && !BlockIsSizable[block.Type] && !BlockOnlyHitspot1[block.Type])
            {
                if(npc.Special == 1 && BlockSlope[block.Type])
                    continue;

                if((npc.Special == 3 || npc.Special == 4) && BlockSlope2[block.Type])
                    continue;

                if(CheckCollision(tempLocation, block.Location))
                {
                    inner_corner = true;
                    break;
                }
            }
        }

        // routine for turning an inner corner
        if(inner_corner)
        {
            int old_side = npc.Special;

            // on a ceiling / floor?
            if(npc.Special == 1 || npc.Special == 3)
            {
                // going right -> hit left side of wall
                if(npc.Special2 == 1)
                    npc.Special = 2;
                // hit right side of wall
                else
                    npc.Special = 4;

                // was on floor -> now you're going up
                if(old_side == 1)
                {
                    npc.Location.SpeedY = 0;
                    npc.Special2 = -1;
                }
                // now you're going down
                else
                    npc.Special2 = 1;
            }
            // on a wall?
            else if(npc.Special == 2 || npc.Special == 4)
            {
                // going down -> hit a floor
                if(npc.Special2 == 1)
                    npc.Special = 1;
                // hit a ceiling
                else
                    npc.Special = 3;

                // was on left side of wall -> now you're going left
                if(old_side == 2)
                    npc.Special2 = -1;
                // now you're going right
                else
                    npc.Special2 = 1;
            }

            return;
        }

        // check whether the wall the NPC is pressing into is gone -> it's on an outer corner
        bool outer_corner = true;

        // check the 8px below (case 1) / above (case 3) the NPC
        if(npc.Special == 1 || npc.Special == 3)
        {
            tempLocation.Width = npc.Location.Width + ((npc.Special == 1) ? 2 : 0);
            tempLocation.Height = 8;
            tempLocation.X = npc.Location.X;
            tempLocation.Y = npc.Location.Y + ((npc.Special == 1) ? npc.Location.Height : -8);
        }
        // check the 8px to the right (case 2) / left (case 4) of the NPC
        else if(npc.Special == 2 || npc.Special == 4)
        {
            tempLocation.Width = 8;
            tempLocation.Height = npc.Location.Height;
            tempLocation.Y = npc.Location.Y;
            tempLocation.X = npc.Location.X + ((npc.Special == 2) ? npc.Location.Width : -8);
        }

        for(BlockRef_t block_p : treeFLBlockQuery(tempLocation, SORTMODE_COMPAT))
        {
            Block_t& block = *block_p;

            if(!block.Hidden && !BlockNoClipping[block.Type] && !BlockIsSizable[block.Type] && !BlockOnlyHitspot1[block.Type])
            {
                if(CheckCollision(tempLocation, block.Location))
                {
                    // this saves the last Block that the NPC was on top of
                    if(npc.Special == 1)
                        npc.Special3 = (vbint_t)block_p;
                    // this makes the climbing work when on a ceiling slope, without slamming into ceilings otherwise
                    else if(npc.Special == 3)
                    {
                        if(BlockSlope2[block.Type] != 0)
                            npc.Location.SpeedY *= speed_mult;
                    }

                    outer_corner = false;
                    break;
                }
            }
        }

        // outer corner turning logic
        if(outer_corner)
        {
            int old_side = npc.Special;

            // moving on floor / ceiling
            if(npc.Special == 1 || npc.Special == 3)
            {
                // was moving right -> now you're on right side of floor/ceiling block
                if(npc.Special2 == 1)
                {
                    // VB6 logic: turn right off of floor extra beautifully (but note that this could potentially cause the NPC to return to a location from long ago)
                    if(npc.Special == 1 && npc.Special3 > 0)
                    {
                        npc.Location.X = Block[npc.Special3].Location.X + Block[npc.Special3].Location.Width + 2;
                        npc.Location.Y += 2;
                    }

                    npc.Special = 4;
                }
                // now you're on left side of floor/ceiling block
                else
                    npc.Special = 2;

                // on floor -> moving down
                if(old_side == 1)
                    npc.Special2 = 1;
                // on ceiling -> moving up
                else
                    npc.Special2 = -1;
            }
            // moving on wall
            else if(npc.Special == 2 || npc.Special == 4)
            {
                // was moving down -> now you're on bottom of wall block
                if(npc.Special2 == 1)
                    npc.Special = 3;
                // now you're on top of wall block
                else
                    npc.Special = 1;

                // on left side of wall -> moving right
                if(old_side == 2)
                    npc.Special2 = 1;
                // on right side of wall -> moving left
                else
                    npc.Special2 = -1;
            }
        }
    }
    else if(npc.Type == NPCID_SICK_BOSS) // Wart
    {
        // don't despawn -- moved from top of this routine
        if(npc.TimeLeft > 1)
            npc.TimeLeft = 100;

        if(!npc.Wings)
            npc.Direction = npc.DefaultDirection;

        if(npc.Immune > 0 && npc.Special != 3)
            npc.Special = 2;

        if(npc.Damage >= 30)
        {
            if(npc.Special != 3)
                PlaySoundSpatial(SFX_SickBossKilled, npc.Location);
            npc.Special = 3;
        }

        if(npc.Special == 0)
        {
            npc.Special3 += 1;

            if(npc.Special3 > 160 + iRand(140))
            {
                npc.Special = 1;
                npc.Special3 = 0;
            }

            if(npc.Special2 == 1)
            {
                npc.Location.SpeedX = 1;
                if(npc.Location.X > npc.DefaultLocationX + npc.Location.Width * 1)
                    npc.Special2 = -1;
            }
            else if(npc.Special2 == -1)
            {
                npc.Location.SpeedX = -1;
                if(npc.Location.X < npc.DefaultLocationX - npc.Location.Width * 1)
                    npc.Special2 = 1;
            }
            else
                npc.Special2 = npc.Direction;
        }
        else if(npc.Special == 1)
        {
            npc.Location.SpeedX = 0;
            if(npc.Special3 == 0)
                PlaySoundSpatial(SFX_SickBossSpit, npc.Location);

            npc.Special3 += 1;
            if((npc.Special3 % 10) == 0)
            {
                numNPCs++;
                NPC[numNPCs].Inert = npc.Inert;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2 - 16 + (32 * npc.Direction);
                NPC[numNPCs].Location.Y = npc.Location.Y + 18;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Type = NPCID_SICK_BOSS_BALL;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 50;
                NPC[numNPCs].Location.SpeedY = -7;
                NPC[numNPCs].Location.SpeedY += dRand() * 6 - 3;
                NPC[numNPCs].Location.SpeedX = 7 * NPC[numNPCs].Direction;
                NPC[numNPCs].Location.SpeedX *= (140 - npc.Special3);
                NPC[numNPCs].Location.SpeedX /= 140;

                syncLayers_NPC(numNPCs);
            }

            if(npc.Special3 > 120 + iRand(40))
            {
                npc.Special = 0;
                npc.Special3 = 0;
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedX = 0;
            npc.Special4 += 1;
            if(npc.Special4 >= 120)
            {
                npc.Special4 = 0;
                npc.Special = 0;
            }
        }
        else if(npc.Special == 3)
        {
            npc.Location.SpeedX = 0;
            npc.Special4 += 1;
            if(npc.Special4 >= 120)
            {
                npc.Killed = 3;
                NPCQueues::Killed.push_back(A);
            }
        }

    }
    // Platform movement
    else if(npc.Type == NPCID_YEL_PLATFORM || npc.Type == NPCID_BLU_PLATFORM || npc.Type == NPCID_GRN_PLATFORM ||
            npc.Type == NPCID_RED_PLATFORM || npc.Type == NPCID_PLATFORM_S3 || npc.Type == NPCID_SAW)
    {
        // SpecialX (prev Special2) stores the previous X speed (ignoring platform pause)
        // SpecialY (prev Special) stores the previous Y speed (ignoring platform pause)

        // straightLine = false; // SET BUT NOT USED
        // UNUSED(straightLine);
        tempBool = false;

        for(int B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Section == npc.Section)
                tempBool = true;
        }

        int railAlgo = int(npc.Variant);

        if(npc.Type == NPCID_SAW && railAlgo != 2) // Grinder
        {
            npc.Location.X -= 24;
            npc.Location.Width = 96;
            npc.Location.Y += 8;
            npc.Location.Height = 32;

            NPCQueues::Unchecked.push_back(A);
            // deferring tree update to end of the NPC physics update
        }

        const num_t speed = 2; // The Speed

        if((npc.Direction == 1 && tempBool) || npc.Type == NPCID_SAW) // Player in same section, enabled, or, grinder
        {
            bool pausePlatforms = !AllPlayersNormal();

            // this code ran unconditionally in SMBX 1.3
            if(!g_config.fix_platforms_acceleration || !pausePlatforms) // Keep zeroed speed when player required the pause of the move effect
            {
                npc.Location.SpeedY = npc.SpecialY;
                npc.Location.SpeedX = npc.SpecialX;
            }

            tempBool = false;
            // tempBool2 = false; // "value set, but never unused"
            bool centered = false;

            Location_t tempLocation = npc.Location;

            switch(railAlgo) // The hot spot (the rail attachment point)
            {
            case 0:
            case 1: // Fixed 15x47 hot spot
                tempLocation.Y = npc.Location.Y + 15;
                if(railAlgo == 1)
                    tempLocation.Y -= 1; // Exclusive workaround for The Invasion 1 to fix the stuck at the "Clown Car Parking" level
                tempLocation.Height = 2;
                tempLocation.X = npc.Location.X + 47;
                tempLocation.Width = 2;
                break;

            case 2: // Centered hot spot
                centered = true;
                tempLocation.Y = npc.Location.Y + (npc.Location.Height / 2);
                tempLocation.Height = 2;
                tempLocation.X = npc.Location.X + (npc.Location.Width / 2) - 1;
                tempLocation.Width = 2;
                break;
            }

            num_t speed_y = 0; // D = 0;
            num_t speed_x = 0; // E = 0;
            vbint_t bgo_type = 0; // F = 0;
            // tempNPC = npc;
            Location_t oldLoc = npc.Location;

            for(int i : treeBackgroundQuery(tempLocation, SORTMODE_ID))
            {
                if(i > numBackground)
                    break;

                // Any nearest BGO touched? (rails and reverse buffers)
                if((Background[i].Type >= 70 && Background[i].Type <= 74) || Background[i].Type == 100)
                {
                    // Not hidden
                    if(!Background[i].Hidden)
                    {
                        if(CheckCollision(tempLocation, Background[i].Location))
                        {
                            if(bgo_type > 0)
                            {
                                if(Background[i].Type == npc.Special5 || Background[i].Type == 70 || Background[i].Type == 100)
                                {
                                    bgo_type = 0;
                                    speed_x = 0;
                                    speed_y = 0;
                                    // npc = tempNPC;
                                    npc.Location = oldLoc;
                                }
                            }

                            if(bgo_type == 0)
                            {
                                // Vertical rail
                                if(Background[i].Type == 72)
                                {
                                    if(npc.Location.SpeedY <= 0)
                                        npc.Location.SpeedY = -speed;
                                    else
                                        npc.Location.SpeedY = speed;

                                    npc.Location.SpeedX = 0;

                                    if(centered)
                                        speed_x = -(npc.Location.X + (npc.Location.Width / 2)) + (Background[i].Location.X + (Background[i].Location.Width / 2));
                                    else
                                        speed_x = -npc.Location.X + Background[i].Location.X - 32;
                                }
                                // Horizontal rail
                                else if(Background[i].Type == 71)
                                {
                                    if(npc.Location.SpeedX >= 0)
                                        npc.Location.SpeedX = speed;
                                    else
                                        npc.Location.SpeedX = -speed;

                                    npc.Location.SpeedY = 0;

                                    if(centered)
                                        speed_y = -(npc.Location.Y + (npc.Location.Height / 2)) + (Background[i].Location.Y + (Background[i].Location.Height / 2));
                                    else
                                        speed_y = -npc.Location.Y + Background[i].Location.Y;
                                }
                                // Diagonal rail left-bottom, right-top
                                else if(Background[i].Type == 73)
                                {
                                    if(npc.Location.SpeedY < 0)
                                        npc.Location.SpeedX = speed;
                                    else if(npc.Location.SpeedY > 0)
                                        npc.Location.SpeedX = -speed;

                                    if(npc.Location.SpeedX > 0)
                                        npc.Location.SpeedY = -speed;
                                    else if(npc.Location.SpeedX < 0)
                                        npc.Location.SpeedY = speed;
                                }
                                // Diagonal rail left-top, right-bottom
                                else if(Background[i].Type == 74)
                                {
                                    if(npc.Location.SpeedY < 0)
                                        npc.Location.SpeedX = -speed;
                                    else if(npc.Location.SpeedY > 0)
                                        npc.Location.SpeedX = speed;

                                    if(npc.Location.SpeedX > 0)
                                        npc.Location.SpeedY = speed;
                                    else if(npc.Location.SpeedX < 0)
                                        npc.Location.SpeedY = -speed;
                                }
                                // Reverse buffer
                                else if(Background[i].Type == 70 || Background[i].Type == 100)
                                {
                                    npc.Location.SpeedX = -npc.Location.SpeedX;
                                    npc.Location.SpeedY = -npc.Location.SpeedY;
                                    tempBool = true;
                                    break;
                                }

                                tempBool = true;
                                bgo_type = Background[i].Type;
                            }
                        }
                    }//Not hidden
                } // any important BGO?
            } // for BGOs

            // store current rail type in Special5
            npc.Special5 = bgo_type;

            if(!tempBool)
            {
                if(npc.Type == NPCID_PLATFORM_S3 && npc.Wet == 2)
                    npc.Location.SpeedY -= Physics.NPCGravity / 4;
                else
                    npc.Location.SpeedY += Physics.NPCGravity;
            }
            else
            {
                npc.Location.SpeedX += speed_x;
                npc.Location.SpeedY += speed_y;
            }

            // this code ran unconditionally in SMBX 1.3
            // it does not run in modern mode so that the SpecialY / SpecialX keep the speed from before the pause
            if(!g_config.fix_platforms_acceleration || !pausePlatforms)
            {
                npc.SpecialY = npc.Location.SpeedY;
                npc.SpecialX = npc.Location.SpeedX;
            }

            // NEW: add a terminal velocity for the platforms in modern mode
            if(npc.SpecialY > 16)
            {
                if(g_config.fix_platforms_acceleration)
                    npc.SpecialY = 16;

                // in either mode, try to "cancel" the NPC once it has fallen below everything
                // fixes some serious memory exhaustion bugs
                int below_section = (int)(npc.Location.Y - level[npc.Section].Height);

                // kill 4096 below section in Vanilla, and 1024 below section in Classic / Modern
                int kill_threshold = (!g_config.fix_platforms_acceleration) ? 4096 : 1024;

                if(below_section >= kill_threshold)
                {
                    bool below_all = true;

                    for(int B = 0; B < numSections; B++)
                    {
                        if(npc.Location.Y < level[B].Height)
                        {
                            if(npc.Location.X >= level[B].X && npc.Location.X + npc.Location.Width <= level[B].Width)
                            {
                                below_all = false;
                                break;
                            }
                        }
                    }

                    if(below_all)
                    {
                        npc.Effect = NPCEFF_WAITING;
                        npc.Effect2 = npc.TimeLeft + 1;
                    }
                }
            }

            // SMBX 1.3 logic: zero the actual speed of the platforms
            if(pausePlatforms) // Or zero the speed and don't change special values
            {
                npc.Location.SpeedX = 0;
                npc.Location.SpeedY = 0;
            }
        }
        else
        {
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;
        }

        Block[npc.tempBlock].Location = npc.Location;
        Block[npc.tempBlock].Location.X += npc.Location.SpeedX;

        if(npc.Location.SpeedY < 0)
            Block[npc.tempBlock].Location.Y += npc.Location.SpeedY;

        // The tempBlock has been moved, above, and the NPC hasn't.
        // So, the tempBlock goes out of sync with the NPC and joins the tree.
        if(npc.tempBlock > 0 && (npc.Location.SpeedX != 0 || npc.Location.SpeedY < 0))
            treeNPCUpdateTempBlock(&npc);

        if(npc.Type == NPCID_SAW)
        {
            if(railAlgo != 2)
            {
                npc.Location.X += 24;
                npc.Location.Width = 48;
                npc.Location.Y -= 8;
                npc.Location.Height = 48;

                NPCQueues::Unchecked.push_back(A);
                // deferring tree update to end of the NPC physics update
            }

            if(npc.Location.SpeedX == 0 && num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity))
            {
                npc.Location.SpeedX = speed * npc.Direction;
                npc.SpecialX = npc.Location.SpeedX;
            }
        }
    }
    else if(npc.Type == NPCID_VILLAIN_S1) // King Koopa
    {
        // don't despawn -- moved from top of this routine
        if(npc.TimeLeft > 1)
            npc.TimeLeft = 100;

        int target_plr = NPCFaceNearestPlayer(npc, true); // was D

        npc.Special5 = target_plr;

        if(iRand(300) >= 297 && npc.Special == 0)
            npc.Special = 1;

        npc.Special4 += 1;

        if(npc.Inert)
            npc.Special4 = 150;

        if(npc.Special4 <= 80 + iRand(40))
        {
            if((iRand(100) >= 40) && npc.Special4 % 16 == 0)
            {
                PlaySoundSpatial(SFX_HeavyToss, npc.Location);
                numNPCs++;
                NPC[numNPCs].Inert = npc.Inert;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.X = npc.Location.X + npc.Location.Width / 2 - 16;
                NPC[numNPCs].Location.Y = npc.Location.Y - 32;
                NPC[numNPCs].Direction = npc.Direction;
                NPC[numNPCs].Type = NPCID_HEAVY_THROWN;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 50;
                NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
                NPC[numNPCs].Location.SpeedY = -8;
                NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;

                syncLayers_NPC(numNPCs);
            }
        }
        else if(npc.Special4 > 300 + iRand(50))
            npc.Special4 = 0;

        if(npc.Inert)
            npc.Special = 0;

        if(npc.Special > 0)
        {
            npc.Special3 += 1;
            if(npc.Special3 < 40)
                npc.Special = 1;
            else if(npc.Special3 < 70)
            {
                if(npc.Special3 == 40)
                {
                    numNPCs++;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Direction = npc.Direction;
                    NPC[numNPCs].Section = npc.Section;
                    NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
                    NPC[numNPCs].Type = NPCID_VILLAIN_FIRE;
                    if(NPC[numNPCs].Direction == 1)
                        NPC[numNPCs].Frame = 4;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    if(NPC[numNPCs].Direction == -1)
                        NPC[numNPCs].Location.X = npc.Location.X - 40;
                    else
                        NPC[numNPCs].Location.X = npc.Location.X + 54;
                    NPC[numNPCs].Location.Y = npc.Location.Y + 19;

                    NPCSetSpeedTarget_FixedX(NPC[numNPCs], Player[npc.Special5].Location, 4, 1);

                    PlaySoundSpatial(SFX_BigFireball, npc.Location);

                    syncLayers_NPC(numNPCs);
                }
                npc.Special = 2;
            }
            else
            {
                npc.Special = 0;
                npc.Special3 = 0;
            }
        }
        else if(npc.Special == 0)
        {
            if(npc.Special2 == 0)
            {
                npc.Location.SpeedX = -0.5_n;
                if(npc.Location.X < npc.DefaultLocationX - npc.Location.Width * 1.5_rb)
                    npc.Special2 = 1;
            }
            else
            {
                npc.Location.SpeedX = 0.5_n;
                if(npc.Location.X > npc.DefaultLocationX + npc.Location.Width * 1.5_rb)
                    npc.Special2 = 0;
            }

            if(num_t::fEqual_d(npc.Location.SpeedY, Physics.NPCGravity) || npc.Slope > 0)
            {
                if(iRand(100) == 0)
                    npc.Location.SpeedY = -8;
            }
        }
    }
    else if(npc.Type == NPCID_GOALTAPE || npc.Type == NPCID_FLAG_EXIT) // SMW Exit
    {
        // Special is whether the tape is going up or down
        // SpecialY is the Y coordinate of the ground below the tape (was previously Special2)

        if(npc.Type == NPCID_FLAG_EXIT)
        {
            if(LevelMacro == LEVELMACRO_FLAG_EXIT)
            {
                if(npc.Special)
                    npc.Location.SpeedY = 0;
                else
                    npc.Location.SpeedY = 2;

                return;
            }
        }
        else if(npc.Special == 0)
            npc.Location.SpeedY = 2;
        else
            npc.Location.SpeedY = -2;

        if(npc.Location.Y <= npc.DefaultLocationY)
            npc.Special = 0;

        if(npc.SpecialY == 0)
        {
            Location_t tempLocation = npc.Location;
            tempLocation.Height = 8000;
            int C = 0;
            for(int i : treeBlockQuery(tempLocation, SORTMODE_COMPAT))
            {
                if(CheckCollision(tempLocation, Block[i].Location))
                {
                    if(C == 0)
                        C = i;
                    else
                    {
                        if(Block[i].Location.Y < Block[C].Location.Y)
                            C = i;
                    }
                }
            }
            if(C > 0)
                npc.SpecialY = Block[C].Location.Y + 4;
        }

        num_t left_boundary = npc.Location.X;
        if(npc.Type == NPCID_GOALTAPE)
            left_boundary += npc.Location.Width - 8;

        for(int i = 1; i <= numPlayers; i++)
        {
            auto &p = Player[i];

            if(p.Section == npc.Section)
            {
                if(p.Location.Y + npc.Location.Height <= npc.SpecialY)
                {
                    if(p.Location.X + p.Location.Width >= left_boundary)
                    {
                        if(p.Location.X <= npc.Location.X + 80)
                        {
                            if(npc.Type == NPCID_FLAG_EXIT)
                            {
                                // have some stricter checks for this exit
                                if(p.Dead || p.TimeToLive || p.Effect != PLREFF_NORMAL || p.CurMazeZone)
                                    continue;

                                // give tapes for flag exit
                                int score = 10;

                                int gap = num_t::floor(p.Location.Y) - num_t::floor(npc.Location.Y);
                                if(gap > 0)
                                    score -= (gap + 31) / 32;

                                if(score < 1)
                                    score = 1;

                                MoreScore(score, npc.Location);
                            }
                            // give points for goal tape if touched
                            else if(CheckCollision(p.Location, npc.Location))
                            {
                                MoreScore(num_t::vb6round((1 - (npc.Location.Y - npc.DefaultLocationY).divided_by(npc.SpecialY - npc.DefaultLocationY)) * 10) + 1, npc.Location);
                                npc.Killed = 9;
                                NPCQueues::Killed.push_back(A);
                                PlaySoundSpatial(SFX_Stone, npc.Location);
                            }

                            FreezeNPCs = false;
                            TurnNPCsIntoCoins();

                            if(g_ClonedPlayerMode)
                                Player[1] = Player[A];

                            for(int j = 1; j <= numPlayers; j++)
                            {
                                if(i == j) // And DScreenType <> 5 Then
                                    continue;
                                Player[j].Section = p.Section;
                                // yes, it's a VB6 bug
                                Player[j].Location.Y = p.Location.Y /* + p.Location.Height - p.Location.Height*/;
                                Player[j].Location.X = p.Location.X /* + (p.Location.Width - p.Location.Width) / 2*/;
                                Player[j].Location.SpeedX = 0;
                                Player[j].Location.SpeedY = 0;
                                Player[j].Effect = PLREFF_WAITING;
                                Player[j].Effect2 = -i;
                            }

                            StopMusic();

                            if(npc.Type == NPCID_FLAG_EXIT)
                            {
                                LevelMacro = LEVELMACRO_FLAG_EXIT;
                                // go towards the ground
                                npc.Location.Y += 2;
                                // wait for player to reach ground
                                LevelMacroWhich = -16;
                                PlaySound(SFX_FlagExit);
                            }
                            else
                            {
                                LevelMacro = LEVELMACRO_GOAL_TAPE_EXIT;
                                PlaySound(SFX_TapeExit);
                            }

                            // XEvents::doEvents();
                            break;
                        }
                    }
                }
            }
        }

    }
    else if(npc.Type == NPCID_CHECKER_PLATFORM)
    {
        if(npc.Special == 1)
        {
            npc.Location.SpeedY += Physics.NPCGravity / 4;
            npc.Special = 0;
        }
        else
            npc.Location.SpeedY = 0;
    }
    else if(npc.Type == NPCID_PLATFORM_S1)
        npc.Location.SpeedY = npc.Direction * 2;

    else if(npc.Type == NPCID_LAVA_MONSTER)
    {
        if(npc.Special == 0)
        {
            Location_t tempLocation = npc.Location;
            tempLocation.Height = 400;
            tempLocation.Y -= tempLocation.Height;
            tempLocation.set_width_center(600);

            for(int i = 1; i <= numPlayers; i++)
            {
                if(CheckCollision(tempLocation, Player[i].Location))
                {
                    if(npc.Location.to_right_of(Player[i].Location))
                        npc.Direction = -1;
                    else
                        npc.Direction = 1;

                    npc.Special = 1;
                    NewEffect(EFFID_LAVA_MONSTER_LOOK, npc.Location, npc.Direction);
                    break;
                }
            }
        }
        else if(npc.Special == 1)
        {
            npc.Special2 += 1;
            if(npc.Special2 == 90)
            {
                npc.Location.SpeedX = 1 * npc.Direction;
                npc.Location.SpeedY = -4.2_n;
                npc.Special = 2;
                PlaySoundSpatial(SFX_LavaMonster, npc.Location);
            }
        }
        else if(npc.Special == 2)
        {
            npc.Location.SpeedY += Physics.NPCGravity * 0.4_r;
            if(npc.Location.Y > npc.DefaultLocationY + npc.Location.Height + 48)
                Deactivate(A);
        }


        // End If
        // Sniffits
    }
    else if(npc.Type >= NPCID_RED_SPIT_GUY && npc.Type <= NPCID_GRY_SPIT_GUY)
    {
        if(npc.Projectile)
        {
            npc.Special = 0;
            npc.Special2 = 0;
        }

        tempBool = false;

        if(npc.Type < NPCID_GRY_SPIT_GUY)
        {
            npc.Special += 1;
            if(npc.Special > 120)
            {
                npc.FrameCount -= 1;

                if(npc.Special2 == 0)
                {
                    npc.Special2 = 1;
                    npc.Location.X -= 2;
                }
                else
                {
                    npc.Special2 = 0;
                    npc.Location.X += 2;
                }

                npc.Location.SpeedX = 0;

                if(npc.Special >= 150)
                {
                    tempBool = true;
                    npc.Special = 0;
                }
            }
        }
        else
        {
            npc.Special += 1;

            if(npc.Special > 160)
            {
                tempBool = true;

                if(npc.Special3 != 2)
                {
                    npc.Special3 -= 1;
                    npc.Special = 0;
                }
                else
                {
                    npc.Special = 140;
                    npc.Special3 -= 1;
                    if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity) || npc.Slope > 0)
                        npc.Special2 = 90;
                }

                if(npc.Special3 < 0)
                    npc.Special3 = 2;
            }

            if(num_t::fEqual_f(npc.Location.SpeedY, Physics.NPCGravity) || npc.Slope > 0)
            {
                npc.Special2 += 1;
                if(npc.Special2 >= 100)
                {
                    npc.Special2 = 0;
                    npc.Location.SpeedY = -3.9_n;
                    npc.Location.Y -= Physics.NPCGravity;
                    // deferring tree update to end of the NPC physics update
                }
            }
            else
                npc.FrameCount -= 1;
        }

        if(tempBool)
        {
            numNPCs++;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Section = npc.Section;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Type = NPCID_SPIT_GUY_BALL;
            NPC[numNPCs].Layer = npc.Layer;
            NPC[numNPCs].Inert = npc.Inert;
            NPC[numNPCs].Direction = npc.Direction;
            NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
            NPC[numNPCs].Location.Width = 16;
            NPC[numNPCs].Location.Height = 16;
            NPC[numNPCs].Location.X = npc.Location.X + 8 + 16 * NPC[numNPCs].Direction;
            NPC[numNPCs].Location.Y = npc.Location.Y + 13;
            syncLayers_NPC(numNPCs);
        }
    }
    // code moved from pre-NPCSpecial call
    else if(npc.Type == NPCID_SPIT_BOSS_BALL)
    {
        if(!npc.Projectile)
        {
            npc.Location.SpeedY = 0; // egg code
            if(npc.Location.SpeedX == 0)
                npc.Projectile = true;
        }
    }
    else if(npc.Type == NPCID_SLIDE_BLOCK || npc.Type == NPCID_FALL_BLOCK_RED || npc.Type == NPCID_FALL_BLOCK_BROWN)
    {
        if(npc.Special == 0)
            npc.Location.SpeedY = 0;
    }
    else if(npc.Type == NPCID_TOOTHY)
    {
        npc.Location.SpeedX = 0;
        npc.Location.SpeedY = 0;
    }
    // code moved from post-NPCSpecial call
    else if(npc.Type == NPCID_TANK_TREADS)
    {
        if(!AllPlayersNormal())
            npc.Location.SpeedX = 0;
    }
    else if(npc.Type == NPCID_ICE_CUBE)
    {
        if(npc.Projectile || npc.Wet > 0 || npc.HoldingPlayer > 0)
            npc.Special3 = 0;
        else if(npc.Special3 == 1)
        {
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;
        }

        // this part was from SpecialNPC
        if(npc.Special == 3)
            npc.BeltSpeed = 0;
    }
    else if(npc.Type == NPCID_ITEM_POD)
    {
        if(npc.Special2 == 1)
        {
            // this part was from the below section
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;

            // this part was from SpecialNPC
            npc.Killed = 1;
            NPCQueues::Killed.push_back(A);
        }
    }
    else if(npc.Type == NPCID_SIGN || npc.Type == NPCID_LIFT_SAND)
    {
        npc.Location.SpeedX = 0;
        npc.Location.SpeedY = 0;
    }
    else if(npc.Type == NPCID_ROCKET_WOOD || npc.Type == NPCID_3_LIFE)
        npc.Location.SpeedY = 0;
    else if(npc.Type == NPCID_CHECKPOINT)
    {
        npc.Projectile = false;
        npc.Location.SpeedX = 0;
        npc.Location.SpeedY = 0;
    }
    else if(npc.Type == NPCID_CYCLONE_POWER)
    {
        npc.Special += 1;

        if(npc.Special == 96 || npc.Special >= 256)
            npc.Special = 128;

        npc.Location.SpeedX /= 2;

        if(npc.Stuck)
        {
            // do nothing!
        }
        // accelerate upwards for 80 frames
        else if(npc.Special < 80)
        {
            if(npc.Special < 32)
                npc.Special2 += (32 - npc.Special);
            else if(npc.Special >= 62)
                npc.Special2 -= (84 - npc.Special) * 2;

            npc.Location.SpeedY = -0.004_n * npc.Special2;
            npc.Location.SpeedX = 0.003_n * (npc.Special2 * npc.Direction);
        }
        // pause for a few frames
        else if(npc.Special < 88)
        {
            npc.Location.SpeedX = 0;

            if(npc.Location.SpeedY < 0_n)
                npc.Location.SpeedY += 0.05_n;
        }
        // ease into movement
        else if(npc.Special < 96)
        {
            npc.Location.SpeedX = Physics.NPCMushroomSpeed * npc.Direction * (npc.Special - 80) / 32;
            npc.Location.SpeedY += 0.05_n;
        }
        // bob movement pattern
        else if(npc.Special < 192)
        {
            if(npc.Location.SpeedY < 1.5_n)
                npc.Location.SpeedY += 0.1_n;
        }
        else
        {
            if(npc.Location.SpeedY > -1.5_n)
                npc.Location.SpeedY -= 0.1_n;
        }
    }
    else if(npc.Type == NPCID_RAFT) // Skull raft
    {
        if(!AllPlayersNormal())
        {
            npc.Location.SpeedX = 0;
            npc.Location.SpeedY = 0;
        }

        // the following is all new code!

        if((npc.Special == 2 || npc.Special == 3) && (npc.SpecialX != 0))
        {
            npc.Location.X = npc.SpecialX; // Finish alignment
            npc.SpecialX = 0;
        }

        if(npc.Special == 3) // Watch for wall collisions. If one got dissappear (hidden layer, toggled switch), resume a ride
        {
            auto loc = npc.Location;
            loc.X += 1 * npc.Direction;
            loc.SpeedX += 2 * npc.Direction;

            // int64_t fBlock;// = FirstBlock[static_cast<int>(floor(static_cast<double>(loc.X / 32))) - 1];
            // int64_t lBlock;// = LastBlock[floor((loc.X + loc.Width) / 32.0) + 1];
            // blockTileGet(loc, fBlock, lBlock);
            bool stillCollide = false;

            for(BlockRef_t block : treeBlockQuery(loc, SORTMODE_NONE))
            {
                int B = block;
                if(!CheckCollision(loc, Block[B].Location))
                    continue;

                if(npc.tempBlock == B || Block[B].tempBlockNoProjClipping() ||
                   BlockOnlyHitspot1[Block[B].Type] || BlockIsSizable[Block[B].Type] ||
                   BlockNoClipping[Block[B].Type] || Block[B].Hidden)
                {
                    continue;
                }

                int hs = NPCFindCollision(loc, Block[B].Location);
                if(Block[B].tempBlockNpcType > 0)
                    hs = 0;
                if(hs == 2 || hs == 4)
                    stillCollide = true;
            }

            if(!npcHasFloor(npc) || !stillCollide)
            {
                npc.Special = 2;
                SkullRide(A, true);
            }
        }
    }
}

void SpecialNPC(int A)
{
    // int B = 0;
    // float C = 0;
    // float D = 0;
    // float E = 0;
    // float F = 0;
    // bool tempTurn = false;
    // Location_t tempLocation;
    // Location_t tempLocation2;
    // NPC_t tempNPC;

    if(NPC[A].Type == NPCID_VILLAIN_FIRE || NPC[A].Type == NPCID_QUAD_BALL || NPC[A].Type == NPCID_STATUE_FIRE ||
       NPC[A].Type == NPCID_SPIT_GUY_BALL || NPC[A].Type == NPCID_PLANT_FIREBALL || NPC[A].Type == NPCID_HEAVY_THROWN ||
       NPC[A].Type == NPCID_SICK_BOSS_BALL || NPC[A].Type == NPCID_HOMING_BALL ||
       NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_HEAVY || NPC[A].Type == NPCID_PLR_ICEBALL) // Link shield block
    {
        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_HEAVY || NPC[A].Type == NPCID_PLR_ICEBALL)
        {
            // Projectile check, previously set immediately before calling SpecialNPC
            NPC[A].Projectile = true;

            if(!BattleMode)
                return;
        }

        for(int B = 1; B <= numPlayers; B++)
        {
            if(Player[B].Character == 5 && !Player[B].Dead && Player[B].TimeToLive == 0 &&
               Player[B].Effect == PLREFF_NORMAL && Player[B].SwordPoke == 0 && !Player[B].Fairy &&
               !(NPC[A].Type == NPCID_PLR_FIREBALL && NPC[A].CantHurtPlayer == B) &&
               !(NPC[A].Type == NPCID_PLR_HEAVY && NPC[A].CantHurtPlayer == B))
            {
                Location_t tempLocation;
                if(!Player[B].Duck)
                    tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height - 52;
                else
                    tempLocation.Y = Player[B].Location.Y + Player[B].Location.Height - 28;
                tempLocation.Height = 24;
                tempLocation.Width = 6;
                if(Player[B].Direction == 1)
                    tempLocation.X = Player[B].Location.X + Player[B].Location.Width - 2;
                else
                    tempLocation.X = Player[B].Location.X - tempLocation.Width + 4;
                if(CheckCollision(NPC[A].Location, tempLocation))
                {
#if XTECH_ENABLE_WEIRD_GFX_UPDATES
                    UpdateGraphics(true);
#endif
                    PlaySoundSpatial(SFX_HeroShield, NPC[A].Location);
                    if(NPC[A].Type == NPCID_SPIT_GUY_BALL)
                    {
                        NPC[A].Killed = 3;
                        NPCQueues::Killed.push_back(A);
                    }
                    else
                    {
                        NPC[A].Killed = 9;
                        NPCQueues::Killed.push_back(A);

                        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
                            NPC[A].Killed = 3;

                        if(NPC[A].Type != NPCID_HEAVY_THROWN && NPC[A].Type != NPCID_SICK_BOSS_BALL && NPC[A].Type != NPCID_HOMING_BALL &&
                           NPC[A].Type != NPCID_PLR_HEAVY && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PLR_ICEBALL)
                        {
                            for(int Ci = 1; Ci <= 10; Ci++)
                            {
                                NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[A].Location, NPC[A].Special);
                                Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5_n + NPC[A].Location.SpeedX / 10;
                                Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5_n - NPC[A].Location.SpeedY / 10;

                                if(Effect[numEffects].Frame == 0)
                                    Effect[numEffects].Frame = -iRand(3);
                                else
                                    Effect[numEffects].Frame = 5 + iRand(3);
                            }
                        }

                        if(NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PLR_ICEBALL)
                        {
                            NPC[A].Location.X += (NPC[A].Location.Width - EffectWidth[EFFID_SMOKE_S3]) / 2;
                            NPC[A].Location.Y += (NPC[A].Location.Height - EffectHeight[EFFID_SMOKE_S3]) / 2;
                            NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                            // deferring tree update to end of the NPC physics update
                        }
                    }
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_CHAR4_HEAVY) // Toad Boomerang
    {
        // Projectile check, previously set immediately before calling SpecialNPC
        NPC[A].Projectile = true;

        // Special5 is player that threw NPC
        // Special4 is the direction that the player was facing when throwing (Special6 in SMBX 1.3)

        num_t npcHCenter = NPC[A].Location.X + NPC[A].Location.Width / 2;
        num_t npcVCenter = NPC[A].Location.Y + NPC[A].Location.Height / 2;
        num_t playerHCenter = Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2;
        num_t playerVCenter = Player[NPC[A].Special5].Location.Y + Player[NPC[A].Special5].Location.Height / 2;

        if(NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;
        if(NPC[A].Location.SpeedY > 8)
            NPC[A].Location.SpeedY = 8;
        if(NPC[A].Location.SpeedY < -8)
            NPC[A].Location.SpeedY = -8;
        if(NPC[A].Location.SpeedX > 12 + Player[NPC[A].Special5].Location.SpeedX)
            NPC[A].Location.SpeedX = 12 + Player[NPC[A].Special5].Location.SpeedX;
        if(NPC[A].Location.SpeedX < -12 + Player[NPC[A].Special5].Location.SpeedX)
            NPC[A].Location.SpeedX = -12 + Player[NPC[A].Special5].Location.SpeedX;

        if(npcHCenter > playerHCenter)
        {
            NPC[A].Location.SpeedX -= 0.2_n;
            if(NPC[A].Location.SpeedX > -4 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX -= 0.5_n;

        }
        else if(npcHCenter < playerHCenter)
        {
            NPC[A].Location.SpeedX += 0.2_n;
            if(NPC[A].Location.SpeedX > -4 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX += 0.5_n;
        }

        NPC[A].Location.SpeedX += (playerHCenter - npcHCenter) / 2000;

        if(NPC[A].Location.Y + NPC[A].Location.Height / 2 > playerVCenter)
        {
            NPC[A].Location.SpeedY -= 0.2_n;
            if(NPC[A].Location.SpeedY > 0 && NPC[A].Direction != NPC[A].Special4)
                NPC[A].Location.SpeedY += -num_t::abs(NPC[A].Location.SpeedY) / 25;
        }
        else if(NPC[A].Location.Y + NPC[A].Location.Height / 2 < playerVCenter)
        {
            NPC[A].Location.SpeedY += 0.2_n;
            if(NPC[A].Location.SpeedY < 0 && NPC[A].Direction != NPC[A].Special4)
                NPC[A].Location.SpeedY += num_t::abs(NPC[A].Location.SpeedY) / 25;
        }
        NPC[A].Location.SpeedY += (playerVCenter - NPC[A].Location.Y + NPC[A].Location.Height / 2) / 250;


        bool picked_up = false;
        bool atCenter = (npcHCenter > playerHCenter && NPC[A].Special4 == 1) ||
                        (npcHCenter < playerHCenter && NPC[A].Special4 == -1);
        if(!atCenter)
        {
            NPC[A].Special2 = 1;
            if(CheckCollision(NPC[A].Location, Player[NPC[A].Special5].Location))
            {
                NPC[A].Killed = 9;
                NPCQueues::Killed.push_back(A);
                Player[NPC[A].Special5].FrameCount = 115;
                PlaySoundSpatial(SFX_Grab2, NPC[A].Location);
                picked_up = true;
            }
        }

        // look for coins, and pick them up if needed
        for(int B : treeNPCQuery(NPC[A].Location, (picked_up) ? SORTMODE_ID : SORTMODE_NONE))
        {
            if(NPC[B].Active)
            {
                if(NPC[B]->IsACoin)
                {
                    if(CheckCollision(NPC[A].Location, NPC[B].Location))
                    {
                        NPC[B].Special = 0;
                        NPC[B].Projectile = false;

                        if(picked_up)
                        {
                            NPC[B].Location.X = playerHCenter - NPC[B].Location.Width / 2;
                            NPC[B].Location.Y = playerVCenter - NPC[B].Location.Height / 2;
                            TouchBonus(NPC[A].Special5, B);
                        }
                        else
                        {
                            NPC[B].Location.X = npcHCenter - NPC[B].Location.Width / 2;
                            NPC[B].Location.Y = npcVCenter - NPC[B].Location.Height / 2;
                        }

                        // must update now because B won't be checked later
                        treeNPCUpdate(B);
                        if(NPC[B].tempBlock > 0)
                            treeNPCSplitTempBlock(B);
                    }
                }
            }
        }

        if(NPC[A].Special2 == 1)
        {
            if(NPC[A].Location.X + NPC[A].Location.Width / 2 > playerHCenter)
            {
                if(NPC[A].Location.SpeedX > 0)
                    NPC[A].Location.SpeedX -= 0.1_n;
            }
            else if(NPC[A].Location.X + NPC[A].Location.Width / 2 < playerHCenter)
            {
                if(NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX += 0.1_n;
            }

            if(NPC[A].Location.Y + NPC[A].Location.Height / 2 > playerVCenter)
            {
                if(NPC[A].Location.SpeedY > 0)
                    NPC[A].Location.SpeedY -= 0.3_n;
            }
            else if(NPC[A].Location.Y + NPC[A].Location.Height / 2 < playerVCenter)
            {
                if(NPC[A].Location.SpeedY < 0)
                    NPC[A].Location.SpeedY += 0.1_n;
            }
        }
    }
    else if(NPC[A].Type == NPCID_ITEM_POD) // yoshi egg
    {
        if(NPC[A].Location.SpeedY > 2)
            NPC[A].Projectile = true;

        // moved to NPCSpecial
        // if(NPC[A].Special2 == 1)
        // {
        //     NPC[A].Killed = 1;
        //     NPCQueues::Killed.push_back(A);
        // }
    }
    else if(NPC[A].Type == NPCID_GEM_1 || NPC[A].Type == NPCID_GEM_5 || NPC[A].Type == NPCID_GEM_20) // Rupee
    {
        if(NPC[A].Location.SpeedX < -0.02_n)
            NPC[A].Location.SpeedX += 0.02_n;
        else if(NPC[A].Location.SpeedX > 0.02_n)
            NPC[A].Location.SpeedX -= 0.02_n;
        else
            NPC[A].Location.SpeedX = 0;
    }
    else if(NPC[A].Type == NPCID_ICE_BLOCK || NPC[A].Type == NPCID_ICE_CUBE || NPC[A].Type == NPCID_INVINCIBILITY_POWER) // Yoshi Ice
    {
        // moved to NPCSpecial
        // if(NPC[A].Type == NPCID_ICE_CUBE && NPC[A].Special == 3)
        //     NPC[A].BeltSpeed = 0;

        Location_t tempLocation;

        // two chances to make a sparkle: 7 frames out of 100 when stationary, and 1 frame out of 5 when thrown
        if(iRand(100) >= 93)
            NewEffect_IceSparkle(NPC[A], tempLocation);

        if(NPC[A].Projectile && iRand(5) == 0)
            NewEffect_IceSparkle(NPC[A], tempLocation);
    }
    else if(NPC[A].Type == NPCID_SQUID_S3 || NPC[A].Type == NPCID_SQUID_S1) // Blooper
    {
        if(NPC[A].Wet == 2 && NPC[A].Quicksand == 0)
        {
            if(NPC[A].Special == 0)
            {
                int target_plr = NPCTargetPlayer(NPC[A]);

                if(target_plr == 0)
                    target_plr = 1;

                NPC[A].Special = target_plr;
            }

            if(NPC[A].Special2 == 0 || NPC[A].Special4 == 1)
            {
                if(NPC[A].Location.Y + NPC[A].Location.Height >= Player[NPC[A].Special].Location.Y - 24 || NPC[A].Special4 == 1)
                {
                    NPC[A].Special2 = 60;
                    if(Player[NPC[A].Special].Location.to_right_of(NPC[A].Location))
                        NPC[A].Location.SpeedX = 4;
                    else
                        NPC[A].Location.SpeedX = -4;
                    if(NPC[A].Special4 == 1)
                    {
                        NPC[A].Special4 = 0;
                        NPC[A].Location.Y -= 0.1_n;
                    }
                }
            }

            if(NPC[A].Special2 > 0)
            {
                NPC[A].Special2 -= 1;
                NPC[A].Location.SpeedY = -1.75_n;
                NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Location.SpeedY = 1;
                NPC[A].Frame = 1;
            }

            if(NPC[A].Special2 == 0)
                NPC[A].Special2 = -20;

            if(NPC[A].Special2 < 0)
                NPC[A].Special2 += 1;

            if(NPC[A].Location.SpeedY >= 0)
                NPC[A].Location.SpeedX = 0;
        }
        else
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.7_r;
            if(NPC[A].Location.SpeedY < -1)
                NPC[A].Location.SpeedY = -1;
            NPC[A].Special2 = -60;
        }

        if(Player[NPC[A].Special].Dead || Player[NPC[A].Special].Section != NPC[A].Section)
            NPC[A].Special = 0;
    }
    else if(NPC[A].Type == NPCID_DOOR_MAKER)
    {
        if(NPC[A].Special3 == 1)
        {
            Location_t tempLocation = NPC[A].Location;
            tempLocation.Y -= 32;
            NewEffect(EFFID_SMOKE_S2, NPC[A].Location);
            NewEffect(EFFID_SMOKE_S2, tempLocation);
            NPC[A].Frame = 0;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
            NPC[A].Special3 = 0;
            NPC[A].Effect = NPCEFF_WAITING;
            NPC[A].Effect2 = 16;
            NPC[A].Projectile = false;
            NPC[A].Type = NPCID_MAGIC_DOOR;
            NPC[A].Wings = WING_NONE;
            PlaySoundSpatial(SFX_SpitBossBeat, NPC[A].Location);
        }
    }
    // firespitting plant
    else if(NPC[A].Type == NPCID_FIRE_PLANT)
    {
        int target_plr = NPCFaceNearestPlayer(NPC[A]);

        if(target_plr != 0)
            NPC[A].Special4 = target_plr;

        if(NPC[A].Location.X != NPC[A].DefaultLocationX)
        {
            NPC[A].Killed = 2;
            NPCQueues::Killed.push_back(A);
            NPC[A].Location.Y += -NPC[A].Location.SpeedY;
            // deferring tree update to end of the NPC physics update
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                NPC[A].Location.Y += NPC[A]->THeight + 1.5_n;
                NPC[A].Special2 = 4;
                NPC[A].Special = 70;
            }

            if(NPC[A].Special2 == 1)
            {
                NPC[A].Special += 1;
                NPC[A].Location.Y -= 1.5_n;

                // NPC[A].Special >= NPC[A]->THeight * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->THeight * 65 + 100)
                {
                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                NPC[A].Special += 1;
                if(NPC[A].Special >= 100)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
                else if(NPC[A].Special == 50)
                {
                    numNPCs++;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].Type = NPCID_PLANT_FIREBALL;
                    NPC[numNPCs].Frame = 1;
                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;

                    if(NPC[numNPCs]->TWidth == 16)
                    {
                        NPC[numNPCs].Location.X = NPC[A].Location.X + 8;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + 8;
                    }
                    else // modified fireball
                    {
                        NPC[numNPCs].Location.X = NPC[A].Location.X;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                    }

                    NPCSetSpeedTarget_FixedX(NPC[numNPCs], Player[NPC[A].Special4].Location, 3, 2);

                    NPC[numNPCs].Location.X += NPC[numNPCs].Location.SpeedX * 4;
                    NPC[numNPCs].Location.Y += NPC[numNPCs].Location.SpeedY * 4;

                    syncLayers_NPC(numNPCs);
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special += 1;
                NPC[A].Location.Y += 1.5_n;

                // NPC[A].Special >= NPC[A]->THeight * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->THeight * 65 + 100)
                    NPC[A].Special2 = 4;
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special += 1;
                if(NPC[A].Special >= 150)
                {
                    bool tempTurn = true;
                    if(!NPC[A].Inert)
                    {
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                            {
                                if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                {
                                    tempTurn = false;
                                    break;
                                }
                            }
                        }
                    }

                    if(tempTurn)
                    {
                        NPC[A].Special2 = 1;
                        NPC[A].Special = 0;
                    }
                    else
                        NPC[A].Special = 140;
                }
            }

            NPC[A].Location.Height = NPC[A]->THeight - (NPC[A].Location.Y - NPC[A].DefaultLocationY);

            if(NPC[A].Location.Height < 0)
                NPC[A].Location.Height = 0;
            // deferring tree update to end of the NPC physics update

            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }

    // jumping plant
    }
    else if(NPC[A].Type == NPCID_JUMP_PLANT)
    {
        if(NPC[A].Projectile)
        {
            NPC[A].Location.SpeedY += Physics.NPCGravity;
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98_r;
        }
        else
        {
            if(NPC[A].Special == 0) // hiding
            {
                NPC[A].Location.Y = NPC[A].DefaultLocationY + NPC[A]->THeight + 1.5_n;
                NPC[A].Location.Height = 0;
                NPC[A].Special2 -= 1;

                if(NPC[A].Special2 <= -30)
                {
                    bool tempTurn = true;

                    if(!NPC[A].Inert)
                    {
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                            {
                                if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                {
                                    tempTurn = false;
                                    break;
                                }
                            }
                        }
                    }

                    if(tempTurn)
                    {
                        NPC[A].Special = 1;
                        NPC[A].Special2 = 0;
                    }
                    else
                        NPC[A].Special2 = 1000;
                }
            }
            else if(NPC[A].Special == 1) // jumping
            {
                NPC[A].Location.Height = NPC[A]->THeight;

                if(NPC[A].Special2 == 0)
                    NPC[A].Location.SpeedY = -6;
                else if(NPC[A].Location.SpeedY < -4)
                    NPC[A].Location.SpeedY += 0.2_n;
                else if(NPC[A].Location.SpeedY < -3)
                    NPC[A].Location.SpeedY += 0.15_n;
                else if(NPC[A].Location.SpeedY < -2)
                    NPC[A].Location.SpeedY += 0.1_n;
                else if(NPC[A].Location.SpeedY < -1)
                    NPC[A].Location.SpeedY += 0.05_n;
                else
                    NPC[A].Location.SpeedY += 0.02_n;

                NPC[A].Special2 += 1;

                if(NPC[A].Location.SpeedY >= 0)
                {
                    NPC[A].Special = 2;
                    NPC[A].Special2 = 0;
                }

            }
            else if(NPC[A].Special == 2) // falling
            {
                NPC[A].Location.Height = NPC[A]->THeight;

                NPC[A].Location.SpeedY += 0.01_n;
                if(NPC[A].Location.SpeedY >= 0.75_n)
                    NPC[A].Location.SpeedY = 0.75_n;

                if(NPC[A].Location.Y + NPC[A]->THeight >= NPC[A].DefaultLocationY + NPC[A]->THeight)
                {
                    NPC[A].Location.Height = (NPC[A].DefaultLocationY + NPC[A]->THeight) - (NPC[A].Location.Y);
                    if(NPC[A].Location.Y >= NPC[A].DefaultLocationY + NPC[A]->THeight)
                    {
                        NPC[A].Location.Height = 0;
                        NPC[A].Location.Y = NPC[A].DefaultLocationY + NPC[A]->THeight;
                        NPC[A].Special = 0;
                        NPC[A].Special2 = 60;
                    }
                }
            }

            if(NPC[A].Location.Height < 0)
                NPC[A].Location.Height = 0;
            // deferring tree update to end of the NPC physics update

            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }

    // Piranha Plant code
    }
    else if(NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_BIG_PLANT || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_LONG_PLANT_UP)
    {
        if(NPC[A].Special3 > 0)
            NPC[A].Special3 -= 1;

        if(NPC[A].Location.X != NPC[A].DefaultLocationX)
        {
            NPC[A].Killed = 2;
            NPCQueues::Killed.push_back(A);

            NPC[A].Location.Y += -NPC[A].Location.SpeedY;
            // deferring tree update to end of the NPC physics update
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                NPC[A].Location.Y += NPC[A]->THeight + 1.5_n;
                NPC[A].Special2 = 4;
                NPC[A].Special = 70;
            }

            if(NPC[A].Special2 == 1)
            {
                NPC[A].Special += 1;
                NPC[A].Location.Y -= 1.5_n;

                // NPC[A].Special >= NPC[A]->THeight * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->THeight * 65 + 100)
                {
                    if(g_config.fix_plant_wobble)
                    {
                        NPC[A].Location.Y = num_t::round(NPC[A].Location.Y);
                        NPC[A].Location.Height = NPC[A]->THeight;
                    }

                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                if(NPC[A].Type != NPCID_LONG_PLANT_UP)
                    NPC[A].Special += 1;
                if(NPC[A].Special >= 50)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special += 1;
                NPC[A].Location.Y += 1.5_n;

                // NPC[A].Special >= NPC[A]->THeight * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->THeight * 65 + 100)
                {
                    NPC[A].Special2 = 4;
                    if(NPC[A].Type == NPCID_LONG_PLANT_UP)
                        NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special += 1;
                if(NPC[A].Special >= 75)
                {
                    bool tempTurn = true;
                    if(!NPC[A].Inert)
                    {
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                            {
                                if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                {
                                    tempTurn = false;
                                    break;
                                }
                            }
                        }
                    }
                    if(NPC[A].Type == NPCID_LONG_PLANT_UP)
                        tempTurn = true;
                    if(tempTurn)
                    {
                        NPC[A].Special2 = 1;
                        NPC[A].Special = 0;
                    }
                    else
                        NPC[A].Special = 140;
                }
            }

            NPC[A].Location.Height = NPC[A]->THeight - (NPC[A].Location.Y - NPC[A].DefaultLocationY);
            if(NPC[A].Location.Height < 0)
                NPC[A].Location.Height = 0;
            // deferring tree update to end of the NPC physics update

            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }
    // down piranha plant
    }
    else if(NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_LONG_PLANT_DOWN)
    {
        if(NPC[A].Special3 > 0)
            NPC[A].Special3 -= 1;

        if(NPC[A].Location.X != NPC[A].DefaultLocationX)
        {
            NPC[A].Killed = 2;
            NPCQueues::Killed.push_back(A);

            NPC[A].Location.Y -= NPC[A].Location.SpeedY;
            // deferring tree update to end of the NPC physics update
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                // .Location.Y += -NPCHeight(.Type) - 1.5
                NPC[A].Location.Height = 0;
                NPC[A].Special2 = 1;
                NPC[A].Special = 0;
            }
            else if(NPC[A].Special2 == 1)
            {
                NPC[A].Special += 1;
                // .Location.Y += 1.5
                NPC[A].Location.Height += 1.5_n;

                // NPC[A].Special >= NPC[A]->THeight * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->THeight * 65 + 100)
                {
                    if(g_config.fix_plant_wobble)
                        NPC[A].Location.Height = NPC[A]->THeight;

                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                if(NPC[A].Type != NPCID_LONG_PLANT_DOWN)
                    NPC[A].Special += 1;

                if(NPC[A].Special >= 50)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special += 1;
                // .Location.Y += -1.5
                NPC[A].Location.Height -= 1.5_n;

                // NPC[A].Special >= NPC[A]->THeight * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->THeight * 65 + 100)
                {
                    if(g_config.fix_plant_wobble)
                        NPC[A].Location.Height = 0;

                    NPC[A].Special2 = 4;
                }
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special += 1;
                if(NPC[A].Special >= 110)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Special = 0;
                }
            }
            // deferring tree update to end of the NPC physics update

            if(NPC[A].Location.Height == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;
        }
    // left/right piranha plant
    }
    else if(NPC[A].Type == NPCID_SIDE_PLANT)
    {
        NPC[A].Direction = NPC[A].DefaultDirection;
        if(NPC[A].Location.Y != NPC[A].DefaultLocationY)
        {
            NPC[A].Location.Y += -NPC[A].Location.SpeedY;
            NPCHit(A, 4);
            // deferring tree update to end of the NPC physics update
        }
        else
        {
            if(NPC[A].Special2 == 0 && !NPC[A].Inert)
            {
                if(NPC[A].Direction == 1)
                {
                    // .Location.x += -NPCWidth(.Type) - 1.5
                    NPC[A].Location.Width += -NPC[A]->TWidth - 1.5_n;
                }
                else
                    NPC[A].Location.X += NPC[A]->TWidth + 1.5_n;
                NPC[A].Special2 = 1;
                NPC[A].Special = 0;
            }
            else if(NPC[A].Special2 == 1)
            {
                NPC[A].Special += 1;

                if(NPC[A].Direction == -1)
                    NPC[A].Location.X += 1.5_n * NPC[A].Direction;
                else
                    NPC[A].Location.Width += 1.5_n * NPC[A].Direction;

                // NPC[A].Special >= NPC[A]->TWidth * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->TWidth * 65 + 100)
                {
                    if(g_config.fix_plant_wobble)
                    {
                        NPC[A].Location.Width = NPC[A]->TWidth;
                        NPCQueues::Unchecked.push_back(A);
                    }

                    NPC[A].Special2 = 2;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 2)
            {
                NPC[A].Special += 1;

                if(NPC[A].Special >= 50)
                {
                    NPC[A].Special2 = 3;
                    NPC[A].Special = 0;
                }
            }
            else if(NPC[A].Special2 == 3)
            {
                NPC[A].Special += 1;

                if(NPC[A].Direction == -1)
                    NPC[A].Location.X -= 1.5_n * NPC[A].Direction;
                else
                    NPC[A].Location.Width -= 1.5_n * NPC[A].Direction;

                // NPC[A].Special >= NPC[A]->TWidth * 0.65 + 1
                if(NPC[A].Special * 100 >= NPC[A]->TWidth * 65 + 100)
                {
                    NPC[A].Special2 = 4;

                    if(g_config.fix_plant_wobble)
                    {
                        NPC[A].Location.Width = 0;
                        NPCQueues::Unchecked.push_back(A);
                    }
                }
            }
            else if(NPC[A].Special2 == 4)
            {
                NPC[A].Special += 1;
                if(NPC[A].Special >= 110)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Special = 0;
                }
            }

            if(NPC[A].Direction == -1)
            {
                NPC[A].Location.Width = NPC[A]->TWidth - (NPC[A].Location.X - NPC[A].DefaultLocationX);
                if(NPC[A].Location.Width < 0)
                    NPC[A].Location.Width = 0;

                NPCQueues::Unchecked.push_back(A);
            }

            // deferring tree update to end of the NPC physics update

            if(NPC[A].Location.Width == 0)
                NPC[A].Immune = 100;
            else
                NPC[A].Immune = 0;

        }
    }
    // smb3 belt code
    else if(NPC[A].Type == NPCID_CONVEYOR)
    {
        NPC[A].Location.SpeedX = 0.8_n * NPC[A].DefaultDirection * BeltDirection;
        NPC[A].Location.X = NPC[A].DefaultLocationX;
        NPC[A].Location.Y = NPC[A].DefaultLocationY;
        NPC[A].Direction = NPC[A].DefaultDirection * BeltDirection;
        // deferring tree update to end of the NPC physics update
    }
    else if(NPC[A].Type == NPCID_CIVILIAN_SCARED)
    {
        if(NPC[A].Location.SpeedY == Physics.NPCGravity)
        {
            NPC[A].Special += 1;
            NPC[A].Frame = 0;
            if(NPC[A].Special >= 100)
                NPC[A].Special = 1;
            else if(NPC[A].Special >= 10)
            {
                NPC[A].Special = 0;
                NPC[A].Frame = 1;
                NPC[A].Location.Y -= 1;
                NPC[A].Location.SpeedY = -4.6_n;
                // deferring tree update to end of the NPC physics update
            }
        }
        else
        {
            if(NPC[A].Special <= 8)
            {
                NPC[A].Special += 1;
                NPC[A].Frame = 1;
            }
            else
            {
                NPC[A].Frame = 2;
                NPC[A].Special = 100;
            }

        }

        if(NPC[A].Direction == 1)
            NPC[A].Frame += 3;
    }
    // Fireball code (Podoboo)
    else if(NPC[A].Type == NPCID_LAVABUBBLE)
    {
        Location_t tempLocation;

        if(NPC[A].Location.Y > NPC[A].DefaultLocationY + NPC[A].Location.Height + 16)
            NPC[A].Location.Y = NPC[A].DefaultLocationY + NPC[A]->THeight + 16;

        NPC[A].Projectile = true;

        // If .Location.X <> .DefaultLocationX Then .Killed = 2
        if(NPC[A].Special2 == 0)
        {
            NPC[A].Location.Y = NPC[A].DefaultLocationY + NPC[A].Location.Height + 1.5_n;
            NPC[A].Special2 = 1;
            NPC[A].Special = 0;
            PlaySoundSpatial(SFX_Lava, NPC[A].Location);
            tempLocation = NPC[A].Location;
            tempLocation.Y -= 32;
            NewEffect(EFFID_LAVA_SPLASH, tempLocation);
        }
        else if(NPC[A].Special2 == 1)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -6;

            if(NPC[A].Location.Y < NPC[A].DefaultLocationY - 10)
            {
                if(NPC[A].Special % 5 == 0) {
                    NewEffect(EFFID_BIG_FIREBALL_TAIL, NPC[A].Location);
                }
            }

            if(NPC[A].Special >= 30)
            {
                NPC[A].Special2 = 2;
                NPC[A].Special = 0;
            }
        }
        else if(NPC[A].Special2 == 2)
        {
            NPC[A].Special += 1;

            if(NPC[A].Special == 61)
            {
                tempLocation = NPC[A].Location;
                tempLocation.Y += 2;
                NewEffect(EFFID_LAVA_SPLASH, tempLocation);
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
            }

            if(NPC[A].Special >= 150)
            {
                NPC[A].Special2 = 0;
                NPC[A].Special = 0;
            }
        }

        if(NPC[A].Location.Y > level[NPC[A].Section].Height + 1)
            NPC[A].Location.Y = level[NPC[A].Section].Height;
        // deferring tree update to end of the NPC physics update

        // Stop the big fireballs from getting killed from tha lava
        NPC[A].Projectile = false;
    }
    else if(NPC[A].Type == NPCID_FALL_BLOCK_RED || NPC[A].Type == NPCID_FALL_BLOCK_BROWN)
    {
        if(LevelMacro == LEVELMACRO_OFF && NPC[A].Special == 0)
        {
            if(NPC[A].Special2 == 1)
            {
                NPC[A].Special3 += 1;
                NPC[A].Special2 = 0;

                // new shake logic because wings allow this object to move horizontally
                if(NPC[A].Wings)
                {
                    NPC[A].Special4 = (NPC[A].Special4 + 1) & 3;
                    NPC[A].Location.X += (NPC[A].Special4 >= 2) ? 2 : -2;
                }
                else
                {
                    NPC[A].Location.X += NPC[A].Direction * 2;
                    if(NPC[A].Location.X >= NPC[A].DefaultLocationX + 2)
                        NPC[A].Direction = -1;
                    if(NPC[A].Location.X <= NPC[A].DefaultLocationX - 2)
                        NPC[A].Direction = 1;
                }
            }
            else
            {
                if(NPC[A].Special3 > 0)
                    NPC[A].Special3 -= 1;

                // new shake logic because wings allow this object to move horizontally
                if(NPC[A].Wings)
                    NPC[A].Special4 = 0;
                else
                    NPC[A].Location.X = NPC[A].DefaultLocationX;
            }

            if((NPC[A].Special3 >= 5 && NPC[A].Type == NPCID_FALL_BLOCK_RED) || (NPC[A].Special3 >= 30 && NPC[A].Type == NPCID_FALL_BLOCK_BROWN))
            {
                NPC[A].Special = 1;

                if(NPC[A].Wings)
                    NPC[A].Wings = WING_NONE;
                else
                    NPC[A].Location.X = NPC[A].DefaultLocationX;
            }
            // deferring tree update to end of the NPC physics update
        }
    // Big Koopa Code
    }
    else if(NPC[A].Type == NPCID_MINIBOSS)
    {
        // Special is the current state
        // SpecialY is the main counter (was previously Special2)
        if(NPC[A].Legacy)
        {
            if(NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;
            if(bgMusic[NPC[A].Section] != 6 && bgMusic[NPC[A].Section] != 15 && bgMusic[NPC[A].Section] != 21 && NPC[A].TimeLeft > 1)
            {
                bgMusic[NPC[A].Section] = 6;
                StartMusicIfOnscreen(NPC[A].Section);
            }
        }

        // temporarily remove any wings if not in normal state
        if(NPC[A].Special != 0)
            NPC[A].Wings = WING_NONE;

        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.Height != 54)
            {
                NPC[A].Location.Y += NPC[A].Location.Height - 54;
                NPC[A].Location.Height = 54;

                // restore wings if lost
                NPC[A].Wings = NPC[A].DefaultWings;
            }

            NPC[A].SpecialY += dRand() * 2;
            if(NPC[A].SpecialY >= 250 + iRand(250))
            {
                NPC[A].Special = 2;
                NPC[A].SpecialY = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].SpecialY += 1;
            if(NPC[A].SpecialY >= 10)
            {
                NPC[A].Special = 1;
                NPC[A].SpecialY = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            if(NPC[A].Location.Height != 40)
            {
                NPC[A].Location.Y += NPC[A].Location.Height - 40;
                NPC[A].Location.Height = 40;
            }

            NPC[A].SpecialY += dRand() * 2;
            if(NPC[A].SpecialY >= 100 + iRand(100))
            {
                NPC[A].Special = 3;
                NPC[A].SpecialY = 0;
            }
        }
        else if(NPC[A].Special == 3)
        {
            NPC[A].SpecialY += 1;
            if(NPC[A].SpecialY >= 10)
            {
                NPC[A].Special = 0;
                NPC[A].SpecialY = 0;
            }
        }
        else if(NPC[A].Special == 4)
        {
            if(NPC[A].Location.Height != 34)
            {
                NPC[A].Location.Y += NPC[A].Location.Height - 34;
                NPC[A].Location.Height = 34;
            }

            NPC[A].SpecialY += 1;
            if(NPC[A].SpecialY >= 100)
            {
                NPC[A].Special = 1;
                NPC[A].SpecialY = 0;
            }
        }
        // deferring tree update to end of the NPC physics update
    }
    else if(NPCIsAParaTroopa(NPC[A].Type)) // para-troopas
    {
        // do wings logic if it won't be handled elsewhere
        if(!NPC[A].Wings)
            NPCMovementLogic_Wings(A, 1);
    }
    // Jumpy bee thing
    else if(NPC[A].Type == NPCID_FLY)
    {
        if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Special += 1;
            if(NPC[A].Special == 30)
            {
                NPC[A].Special = 0;
                NPC[A].Location.Y -= 1;
                NPC[A].Location.SpeedY = -6;
                NPC[A].Location.SpeedX = 1.4_n * NPC[A].Direction;
                // deferring tree update to end of the NPC physics update
            }
        }
    }
    // Bouncy Star thing code
    else if(NPC[A].Type == NPCID_JUMPER_S3)
    {
        NPCFaceNearestPlayer(NPC[A]);

        if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
        {
            NPC[A].Special += 1;
            if(NPC[A].Special == 8)
            {
                NPC[A].Location.SpeedY = -7;
                NPC[A].Location.Y -= 1;
                NPC[A].Special = 0;
                // deferring tree update to end of the NPC physics update
            }
        }
        else
            NPC[A].Special = 0;
    }
    // bowser statue
    else if(NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4)
    {
        NPC[A].Special += 1;
        if(NPC[A].Special > 200 + iRand(200))
        {
            NPC[A].Special = 0;
            numNPCs++;
            NPC[numNPCs].Inert = NPC[A].Inert;
            NPC[numNPCs].Type = NPCID_STATUE_FIRE;
            NPC[numNPCs].Direction = NPC[A].Direction;
            NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
            NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y + 16;
            NPC[numNPCs].Location.X = NPC[A].Location.X + 24 * NPC[numNPCs].Direction;
            if(NPC[A].Type == NPCID_STATUE_S4)
            {
                NPC[numNPCs].Location.Y -= 5;
                NPC[numNPCs].Location.X = NPC[A].Location.X + 6 + 30 * NPC[numNPCs].Direction;
            }
            NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;
            if(NPC[numNPCs].Direction == 1)
                NPC[numNPCs].Frame = 4;
            NPC[numNPCs].FrameCount = iRand(8);
            PlaySoundSpatial(SFX_BigFireball, NPC[A].Location);

            syncLayers_NPC(numNPCs);
        }
    }
    // Hammer Bro
    else if(NPC[A].Type == NPCID_HEAVY_THROWER)
    {
        if(NPC[A].Projectile)
            return;

        NPCFaceNearestPlayer(NPC[A]);

        if(NPC[A].Special > 0)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedX = 0.6_n;
            if(NPC[A].Special >= 100 && NPC[A].Location.SpeedY == Physics.NPCGravity)
                NPC[A].Special = -1;
        }
        else
        {
            NPC[A].Special -= 1;
            NPC[A].Location.SpeedX = -0.6_n;
            if(NPC[A].Special <= -100 && NPC[A].Location.SpeedY == Physics.NPCGravity)
                NPC[A].Special = 1;
        }

        if(NPC[A].Location.SpeedY == Physics.NPCGravity)
        {
            NPC[A].Special2 += 1;
            if(NPC[A].Special2 >= 250)
            {
                NPC[A].Location.SpeedY = -7;
                NPC[A].Location.Y -= 1;
                NPC[A].Special2 = 0;
                // deferring tree update to end of the NPC physics update
            }
        }

        // the throw counter was previously Special3
        NPC[A].SpecialX += dRand() * 2;
        if(NPC[A].SpecialX >= 50 + dRand() * 1000)
        {
            if(NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Location.SpeedY = -3;
                NPC[A].Location.Y -= 1;
                // deferring tree update to end of the NPC physics update
            }

            PlaySoundSpatial(SFX_HeavyToss, NPC[A].Location);
            NPC[A].SpecialX = -15;
            numNPCs++;
            NPC[numNPCs].Inert = NPC[A].Inert;
            NPC[numNPCs].Location.Height = 32;
            NPC[numNPCs].Location.Width = 32;
            NPC[numNPCs].Location.X = NPC[A].Location.X;
            NPC[numNPCs].Location.Y = NPC[A].Location.Y;
            NPC[numNPCs].Direction = NPC[A].Direction;
            NPC[numNPCs].Type = NPCID_HEAVY_THROWN;
            NPC[numNPCs].Section = NPC[A].Section;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 50;
            NPC[numNPCs].Location.SpeedY = -8;
            NPC[numNPCs].Location.SpeedX = 3 * NPC[numNPCs].Direction;

            syncLayers_NPC(numNPCs);
        }
    }
    // leaf
    else if(NPC[A].Type == NPCID_LEAF_POWER) // Leaf
    {
        if(NPC[A].Stuck && !NPC[A].Projectile)
            NPC[A].Location.SpeedX = 0;
        else if(NPC[A].Stuck)
        {
            NPC[A].Location.SpeedY += Physics.NPCGravity;
            if(NPC[A].Location.SpeedY >= 8)
                NPC[A].Location.SpeedY = 8;
        }
        else if(NPC[A].Special == 0)
        {
            NPC[A].Location.SpeedY += Physics.NPCGravity;
            if(NPC[A].Projectile)
            {
                if(NPC[A].Location.SpeedY >= 2)
                {
                    NPC[A].Location.SpeedX = 1.2_n;
                    NPC[A].Special = 1;
                    NPC[A].Projectile = false;
                }

            }
            else if(NPC[A].Location.SpeedY >= 0)
                NPC[A].Special = 6;
        }
        else
        {
            if(NPC[A].Special == 1)
            {
                NPC[A].Location.SpeedY -= 0.25_n;
                NPC[A].Location.SpeedX += 0.3_n;
                if(NPC[A].Location.SpeedY <= 0)
                    NPC[A].Special = 2;
            }
            else if(NPC[A].Special == 2)
            {
                NPC[A].Location.SpeedX -= 0.3_n;
                NPC[A].Location.SpeedY -= 0.02_n;
                if(NPC[A].Location.SpeedX <= 0)
                {
                    NPC[A].Special = 3;
                    NPC[A].Location.SpeedX = 0;
                }
            }
            else if(NPC[A].Special == 3)
            {
                NPC[A].Location.SpeedY += 0.4_n;
                NPC[A].Location.SpeedX -= 0.1_n;
                if(NPC[A].Location.SpeedY >= 3)
                    NPC[A].Special = 4;
            }
            else if(NPC[A].Special == 4)
            {
                NPC[A].Location.SpeedY -= 0.25_n;
                NPC[A].Location.SpeedX -= 0.3_n;
                if(NPC[A].Location.SpeedY <= 0)
                    NPC[A].Special = 5;
            }
            else if(NPC[A].Special == 5)
            {
                NPC[A].Location.SpeedX += 0.3_n;
                NPC[A].Location.SpeedY -= 0.02_n;
                if(NPC[A].Location.SpeedX >= 0)
                {
                    NPC[A].Special = 6;
                    NPC[A].Location.SpeedX = 0;
                }
            }
            else if(NPC[A].Special == 6)
            {
                NPC[A].Location.SpeedY += 0.4_n;
                NPC[A].Location.SpeedX += 0.1_n;
                if(NPC[A].Location.SpeedY >= 3)
                    NPC[A].Special = 1;
            }
        }
    }
    else if(NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER) // combined code for spiky and item throwers
    {
        if(NPC[A].Type == NPCID_ITEM_THROWER && NPC[A].Special == 0)
            NPC[A].Special = NPC[A].Type;

        NPC[A].Projectile = false;
        if(NPC[A].TimeLeft > 1)
            NPC[A].TimeLeft = 100;
        if(NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;

        num_t min_dist = 0;
        int target_plr = 0;
        for(int B = 1; B <= numPlayers; B++)
        {
            if(NPC[A].Type == NPCID_ITEM_THROWER && Player[B].TimeToLive != 0)
                continue;

            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && B != NPC[A].CantHurtPlayer)
            {
                num_t dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                if(min_dist == 0 || dist < min_dist)
                {
                    min_dist = dist;
                    target_plr = B;
                }
            }
        }

        const auto& p = Player[target_plr];

        if(target_plr > 0)
        {
            // this direction indicator used Special (for spiky throwers) and Special6 (for item throwers) in SMBX 1.3
            // now it uses SpecialX (as a simple boolean flag)

            // speed control code: check distance to player
            num_t dx = NPC[A].Location.minus_center_x(p.Location);
            num_t speed_bound = num_t::abs(dx) / 100 + num_t::abs(p.Location.SpeedX) / 2 + 5;

            // adjust expected distance by player's speed
            dx -= p.Location.SpeedX * 15;

            // movement left
            if(NPC[A].SpecialX == 0)
            {
                if(NPC[A].Location.SpeedX >= -speed_bound)
                    NPC[A].Location.SpeedX -= 0.2_n;
                // if we are more than 50px to the left of the player's expected location
                if(dx < -50)
                    NPC[A].SpecialX = 1;
            }
            // movement right
            else
            {
                if(NPC[A].Location.SpeedX <= speed_bound)
                    NPC[A].Location.SpeedX += 0.2_n;
                // if we are more than 50px to the right of the player's expected location
                if(dx > 50)
                    NPC[A].SpecialX = 0;
            }

            // target_plr is the targeted player
            // D is the targeted vScreen

            int D;
            if(g_config.fix_npc_camera_logic)
                D = vScreenIdxByPlayer_canonical(target_plr);
            else
            {
                const Screen_t& plr_screen = ScreenByPlayer(target_plr);

                int vscreen_idx = 0;

                if(plr_screen.player_count == 2)
                {
                    if(plr_screen.Type == 5)
                    {
                        if(plr_screen.DType != 5)
                            vscreen_idx = 1;
                    }
                }

                D = plr_screen.vScreen_refs[vscreen_idx];
            }

            if(NPC[A].Location.Y + NPC[A].Location.Height > p.Location.Y - 248)
                NPC[A].Special2 = 1;
            if(NPC[A].Location.Y + NPC[A].Location.Height < p.Location.Y - 256 || NPC[A].Location.Y < -vScreen[D].Y)
                NPC[A].Special2 = 0;
            if(NPC[A].Location.Y > -vScreen[D].Y + 64)
                NPC[A].Special2 = 1;
            if(NPC[A].Location.Y < -vScreen[D].Y + 72)
                NPC[A].Special2 = 0;

            if(NPC[A].Special2 == 0)
            {
                NPC[A].Location.SpeedY += 0.05_n;
                if(NPC[A].Location.SpeedY > 2)
                    NPC[A].Location.SpeedY = 2;
            }
            else
            {
                NPC[A].Location.SpeedY -= 0.05_n;
                if(NPC[A].Location.SpeedY < -2)
                    NPC[A].Location.SpeedY = -2;
            }

            if(NPC[A].Inert)
            {
                if(NPC[A].Special3 > 1)
                    NPC[A].Special3 = 0;
            }

            // frame timing code for spiky thrower
            if(NPC[A].Type != NPCID_ITEM_THROWER)
            {
                if(NPC[A].Special3 == 0)
                {
                    NPC[A].FrameCount += 1;
                    if(NPC[A].FrameCount >= 10)
                    {
                        NPC[A].FrameCount = 0;
                        NPC[A].Frame += 1;
                        if(NPC[A].Frame >= 2)
                            NPC[A].Special3 = 1;
                    }
                }
                else if(NPC[A].Special3 == 1)
                {
                    NPC[A].FrameCount += 1;
                    if(NPC[A].FrameCount >= 10)
                    {
                        NPC[A].FrameCount = 0;
                        NPC[A].Frame -= 1;
                        if(NPC[A].Frame <= 0)
                            NPC[A].Special3 = 0;
                    }
                }
                else if(NPC[A].Special3 == 2)
                {
                    NPC[A].FrameCount += 1;
                    if(NPC[A].FrameCount >= 16)
                    {
                        NPC[A].FrameCount = 10;
                        if(NPC[A].Frame < 5)
                            NPC[A].Frame += 1;
                        if(NPC[A].Frame <= 5)
                            NPC[A].Special5 += 1;
                    }
                }
                else if(NPC[A].Special3 == 3)
                {
                    NPC[A].FrameCount += 1;
                    if(NPC[A].FrameCount >= 2)
                    {
                        NPC[A].FrameCount = 0;
                        NPC[A].Frame -= 1;
                        if(NPC[A].Frame <= 0)
                        {
                            NPC[A].Special3 = 0;
                            NPC[A].Frame = 0;
                        }
                    }
                }
            }

            // yes, it's a VB6 bug, p.Location.Width / 2 should be subtracted, not added
            if(num_t::abs(NPC[A].Location.X - p.Location.X + (NPC[A].Location.Width + p.Location.Width) / 2) < 100)
            {
                if(NPC[A].Special4 == 0)
                {
                    NPC[A].Special3 = 2;
                    NPC[A].Special4 = 100;
                }
            }
        }

        if(NPC[A].Special4 > 0)
            NPC[A].Special4 -= 1;

        // frame timing code for item thrower
        if(NPC[A].Type == NPCID_ITEM_THROWER)
        {
            NPC[A].Frame = 0;
            if(NPC[A].FrameCount < 100)
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount < 8)
                    NPC[A].Frame = 0;
                else if(NPC[A].FrameCount < 16)
                    NPC[A].Frame = 1;
                else if(NPC[A].FrameCount < 24)
                    NPC[A].Frame = 2;
                else if(NPC[A].FrameCount < 32)
                    NPC[A].Frame = 1;
                else
                {
                    NPC[A].Frame = 0;
                    NPC[A].FrameCount = 0;
                }
            }
            else
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount < 108)
                    NPC[A].Frame = 6;
                else if(NPC[A].FrameCount < 116)
                    NPC[A].Frame = 7;
                else if(NPC[A].FrameCount < 124)
                    NPC[A].Frame = 8;
                else if(NPC[A].FrameCount < 132)
                    NPC[A].Frame = 7;
                else
                {
                    NPC[A].Frame = 0;
                    NPC[A].FrameCount = 0;
                }
            }

            if(NPC[A].Direction == 1)
                NPC[A].Frame += 3;

            NPC[A].Special5 += 1;
        }

        int timer = (NPC[A].Type == NPCID_ITEM_THROWER) ? 150 : 20;

        if(NPC[A].Special5 >= timer)
        {
            NPC[A].Special5 = timer;
            Location_t tempLocation = NPC[A].Location;
            tempLocation.X -= 16;
            tempLocation.Y -= 16;
            tempLocation.Width += 32;
            tempLocation.Height += 32;

            bool do_not_throw = false;

            if(NPC[A].Location.Y + NPC[A].Location.Height > p.Location.Y)
                do_not_throw = true;
            else
            {
                for(int Ei : treeBlockQuery(tempLocation, SORTMODE_NONE))
                {
                    if(NPC[A].Type != NPCID_ITEM_THROWER)
                    {
                        if(BlockIsSizable[Block[Ei].Type] ||
                           BlockOnlyHitspot1[Block[Ei].Type])
                        {
                            continue;
                        }
                    }

                    if(CheckCollision(tempLocation, Block[Ei].Location) && !BlockNoClipping[Block[Ei].Type])
                    {
                        do_not_throw = true;
                        break;
                    }
                }
            }

            if(!do_not_throw)
            {
                if(NPC[A].Type == NPCID_ITEM_THROWER)
                    NPC[A].FrameCount = 100;
                else
                    NPC[A].FrameCount = 0;

                NPC[A].Special3 = 3;
                NPC[A].Special5 = 0;

                numNPCs++;

                if(NPC[A].Location.to_right_of(p.Location))
                    NPC[numNPCs].Direction = -1;
                else
                    NPC[numNPCs].Direction = 1;

                if(NPC[A].CantHurt > 0)
                {
                    NPC[numNPCs].CantHurt = 100;
                    NPC[numNPCs].CantHurtPlayer = NPC[A].CantHurtPlayer;
                }

                NPC[numNPCs].Active = true;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].TimeLeft = 100;

                if(NPC[A].Type == NPCID_ITEM_THROWER)
                {
                    NPC[numNPCs].Type = NPCID(NPC[A].Special);

                    if(NPC[numNPCs].Type == NPCID_RANDOM_POWER)
                        NPC[numNPCs].Type = RandomBonus();

                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                    NPC[numNPCs].Location.X = NPC[A].Location.X + (NPC[A].Location.Width - NPC[numNPCs].Location.Width) / 2;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 8;

                    NPC[numNPCs].Location.SpeedX = (1 + dRand() * 2) * NPC[numNPCs].Direction;
                    NPC[numNPCs].Location.SpeedY = -7;

                    NPC[numNPCs].Variant = NPC[A].Variant;

                    if(NPC[numNPCs]->IsACoin)
                    {
                        NPC[numNPCs].Special = 1;
                        NPC[numNPCs].Location.SpeedX /= 2;
                    }

                    // commented out in SMBX 1.3
                    // tempNPC = NPC(A)
                    // NPC(A) = NPC(numNPCs)
                    // NPC(numNPCs) = tempNPC
                }
                else
                {
                    NPC[numNPCs].Type = NPCID_SPIKY_BALL_S3;

                    NPC[numNPCs].Location.X = NPC[A].Location.X;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 8;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 32;

                    NPC[numNPCs].Location.SpeedX = (1.5_n + num_t::abs(p.Location.SpeedX) * 0.75_rb) * NPC[numNPCs].Direction;
                    NPC[numNPCs].Location.SpeedY = -8;

                    // tempNPC = NPC[A];
                    NPC_t tempNPC = NPC[A];
                    NPC[A] = NPC[numNPCs];
                    NPC[numNPCs] = tempNPC;
                    PlaySoundSpatial(SFX_HeavyToss, NPC[A].Location);

                    syncLayers_NPC(A);
                }

                syncLayers_NPC(numNPCs);
            }
        }
    }
    else if(NPC[A].Type == NPCID_HIT_CARRY_FODDER) // smw goomba
    {
        NPC[A].Special += 1;
        if(NPC[A].Special >= 400)
        {
            if(NPC[A].Slope > 0 || NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Location.SpeedY == 0)
            {
                NPC[A].Location.SpeedY = -5;
                NPC[A].Type = NPCID_CARRY_FODDER;
                NPC[A].Special = 0;
                NPC[A].Location.Y -= 1;
                // deferring tree update to end of the NPC physics update
            }
        }
    }
    else if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4) // thwomp
    {
            if(NPC[A].Special == 0)
            {
                NPC[A].Location.SpeedY = 0;
                if(!NPC[A].Wings)
                    NPC[A].Location.Y = NPC[A].DefaultLocationY;
                for(int B = 1; B <= numPlayers; B++)
                {
                    bool playerLower = Player[B].Location.Y >= NPC[A].Location.Y;
                    // When mode 1, simulate older behavior and do always fall
                    playerLower |= (NPC[A].Variant == 1);
                    if(!CanComeOut(NPC[A].Location, Player[B].Location) && playerLower)
                    {
                        NPC[A].Special = 1;
                        break;
                    }
                }
            }
            else if(NPC[A].Special == 1)
            {
                NPC[A].Location.SpeedY = 6;

                if(NPC[A].Wings)
                {
                    // remember current Y coordinate when rising back up
                    NPC[A].SpecialY = NPC[A].Location.Y;
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Wings = WING_NONE;
                }
            }
            else if(NPC[A].Special == 2)
            {
                if(NPC[A].Special2 == 0 && !NPC[A].Wings)
                {
                    PlaySoundSpatial(SFX_Stone, NPC[A].Location);
                    if(g_config.extra_screen_shake)
                        doShakeScreen(0, 4, SHAKE_SEQUENTIAL, 5, 200, NPC[A].Location);
                    Location_t tempLocation;
                    tempLocation.Width = 32;
                    tempLocation.Height = 32;
                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 16;

//                    tempLocation.X = NPC[A].Location.X;
                    tempLocation.X = (NPC[A].Location.X + NPC[A].Location.Width / 8);
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    Effect[numEffects].Location.SpeedX = -1.5_n;

//                    tempLocation.X += tempLocation.Width - EffectWidth[EFFID_SMOKE_S3];
                    tempLocation.X = (NPC[A].Location.X + NPC[A].Location.Width - EffectWidth[EFFID_SMOKE_S3]) - (NPC[A].Location.Width / 8);
                    NewEffect(EFFID_SMOKE_S3, tempLocation);
                    Effect[numEffects].Location.SpeedX = 1.5_n;
                }
                NPC[A].Location.SpeedY = 0;
                if(NPC[A].Slope > 0)
                    NPC[A].Location.Y -= 0.1_n;
                NPC[A].Special2 += 1;
                if(NPC[A].Special2 >= 100)
                {
                    NPC[A].Location.Y -= 1;
                    NPC[A].Special = 3;
                    NPC[A].Special2 = 0;
                }
            }
            else if(NPC[A].Special == 3)
            {
                NPC[A].Location.SpeedY = -2;

                num_t target_y = (NPC[A].DefaultWings) ? NPC[A].SpecialY : NPC[A].DefaultLocationY;
                if(NPC[A].Location.Y <= target_y + 1)
                {
                    if(!NPC[A].Wings)
                        NPC[A].Location.Y = target_y;

                    NPC[A].Location.SpeedY = 0;
                    NPC[A].Special = 0;
                    NPC[A].Special2 = 0;
                    NPC[A].Wings = NPC[A].DefaultWings;
                }
            }
        // End If
        // deferring tree update to end of the NPC physics update
    }
    else if(NPC[A].Type == NPCID_GHOST_S3 || NPC[A].Type == NPCID_GHOST_S4 || NPC[A].Type == NPCID_BIG_GHOST) // boo
    {
        if(BattleMode && NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;

        if(NPC[A].Projectile)
        {
            if(NPC[A].CantHurtPlayer > 0)
                NPC[A].BattleOwner = NPC[A].CantHurtPlayer;

            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95_r;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95_r;

            if(NPC[A].Location.SpeedX > -2 && NPC[A].Location.SpeedX < 2)
            {
                if(NPC[A].Location.SpeedY > -2 && NPC[A].Location.SpeedY < 2)
                    NPC[A].Projectile = false;
            }
        }

        num_t min_dist = 0;
        int target_plr = 0;
        for(int B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && B != NPC[A].CantHurtPlayer)
            {
                num_t dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                if(min_dist == 0 || dist < min_dist)
                {
                    min_dist = dist;
                    target_plr = B;
                }
            }
        }

        const auto& p = Player[target_plr];

        if(target_plr > 0)
        {
            num_t D = NPC[A].Location.X + NPC[A].Location.Width / 2;
            num_t E = p.Location.X + p.Location.Width / 2;

            // hide
            if((D <= E && p.Direction == -1) || (D >= E && p.Direction == 1) || p.SpinJump)
            {
                // set wings to override to use normal ghost speed logic
                if(NPC[A].Wings)
                    NPC[A].Wings = WING_OVERRIDE;

                NPC[A].Special = 0;

                if(NPC[A].Type == NPCID_GHOST_S3)
                {
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.9_r;
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.9_r;
                }
                else if(NPC[A].Type == NPCID_GHOST_S4)
                {
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.85_r;
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.85_r;
                }
                else if(NPC[A].Type == NPCID_BIG_GHOST)
                {
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.8_r;
                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.8_r;
                }

                if(NPC[A].Location.SpeedX < 0.1_n && NPC[A].Location.SpeedX > -0.1_n)
                    NPC[A].Location.SpeedX = 0;

                if(NPC[A].Location.SpeedY < 0.1_n && NPC[A].Location.SpeedY > -0.1_n)
                    NPC[A].Location.SpeedY = 0;
            }
            // chase
            else
            {
                // restore wings to their default value
                NPC[A].Wings = NPC[A].DefaultWings;

                NPC[A].Special = 1;
                NPC[A].Direction = p.Direction;

                num_t F = 0;
                if(NPC[A].Type == NPCID_GHOST_S3)
                    F = 0.03_n;
                else if(NPC[A].Type == NPCID_GHOST_S4)
                    F = 0.025_n;
                else if(NPC[A].Type == NPCID_BIG_GHOST)
                    F = 0.02_n;

                if(D <= E && NPC[A].Location.SpeedX < 1.5_n)
                    NPC[A].Location.SpeedX += F;
                else if(NPC[A].Location.SpeedX > -1.5_n)
                    NPC[A].Location.SpeedX += -F;

                D = NPC[A].Location.Y + NPC[A].Location.Height / 2;
                E = p.Location.Y + p.Location.Height / 2;

                if(D <= E && NPC[A].Location.SpeedY < 1.5_n)
                    NPC[A].Location.SpeedY += F;
                else if(NPC[A].Location.SpeedY > -1.5_n)
                    NPC[A].Location.SpeedY += -F;
            }
        }
        else
        {
            NPC[A].Special = 0;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
        }
    }
    else if(NPC[A].Type == NPCID_STAR_EXIT || NPC[A].Type == NPCID_STAR_COLLECT)
    {
        if(NPC[A].Projectile)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.95_r;
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95_r;
            if(NPC[A].Location.SpeedY < 1 && NPC[A].Location.SpeedY > -1)
            {
                if(NPC[A].Location.SpeedX < 1 && NPC[A].Location.SpeedX > -1)
                    NPC[A].Projectile = false;
            }
        }

        // unified sparkle timer
        NPC[A].Special4 += 1;
        if(NPC[A].Special4 >= ((NPC[A].Special == 0) ? 5 : 10))
        {
            NPC[A].Special4 = 0;
            NewEffect(EFFID_SPARKLE, newLoc(NPC[A].Location.X + dRand().times(NPC[A].Location.Width) - 2, NPC[A].Location.Y + dRand().times(NPC[A].Location.Height)));
            Effect[numEffects].Location.SpeedX = dRand() - 0.5_n;
            Effect[numEffects].Location.SpeedY = dRand() - 0.5_n;

            if(NPC[A].Special != 0)
                Effect[numEffects].Frame = 1;
        }

        if(NPC[A].Special2 == 0)
        {
            NPC[A].Location.SpeedY -= 0.04_n;
            if(NPC[A].Location.SpeedY <= -1.4_n)
                NPC[A].Special2 = 1;
        }
        else
        {
            NPC[A].Location.SpeedY += 0.04_n;
            if(NPC[A].Location.SpeedY >= 1.4_n)
                NPC[A].Special2 = 0;
        }

        if(NPC[A].Special3 == 0)
        {
            NPC[A].Location.SpeedX -= 0.03_n;
            if(NPC[A].Location.SpeedX <= -0.6_n)
                NPC[A].Special3 = 1;
        }
        else
        {
            NPC[A].Location.SpeedX += 0.03_n;
            if(NPC[A].Location.SpeedX >= 0.6_n)
                NPC[A].Special3 = 0;
        }
    }
    else if(NPC[A].Type == NPCID_SPIT_BOSS) // birdo
    {
        if(NPC[A].Projectile)
            return;

        if(NPC[A].Legacy)
        {
            if(NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;
            if(bgMusic[NPC[A].Section] != 6 && bgMusic[NPC[A].Section] != 15 && bgMusic[NPC[A].Section] != 21 && NPC[A].TimeLeft > 1)
            {
                bgMusic[NPC[A].Section] = 15;
                StartMusicIfOnscreen(NPC[A].Section);
            }
        }

        // previously Special, changed to Special5 when it became a container
        if(NPC[A].Special5 >= 0)
        {
            NPCFaceNearestPlayer(NPC[A]);

            NPC[A].Special2 += 1;
            if(NPC[A].Special2 == 125)
            {
                NPC[A].Location.Y -= 1;
                NPC[A].Location.SpeedY = -5;
                // previously never shot anything, now it shoots if contents are configured
                if(NPC[A].Inert && !NPC[A].Special)
                    NPC[A].Special2 = 0;
                // deferring tree update to end of the NPC physics update
            }
            else if(NPC[A].Special2 >= 240)
            {
                if(NPC[A].Special2 == 260)
                {
                    numNPCs++;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Direction = NPC[A].Direction;
                    NPC[numNPCs].Type = (NPC[A].Special) ? (NPCID)NPC[A].Special : NPCID_SPIT_BOSS_BALL;

                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 14 - NPC[numNPCs].Location.Height / 2;

                    if(NPC[numNPCs].Direction == 1)
                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2;
                    else
                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2 - NPC[numNPCs].Location.Width;

                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Section = NPC[A].Section;
                    NPC[numNPCs].Location.SpeedX = 4 * NPC[numNPCs].Direction;

                    if(NPC[A].Special)
                    {
                        s_NPCSpawnCustomThrown(NPC[A], NPC[numNPCs]);

                        if(NPC[numNPCs].Type == NPCID_SLIDE_BLOCK || NPC[numNPCs]->IsAShell)
                            NPC[numNPCs].Location.SpeedX = Physics.NPCShellSpeed * NPC[numNPCs].Direction;
                    }

                    PlaySoundSpatial(SFX_SpitBossSpit, NPC[A].Location);
                    syncLayers_NPC(numNPCs);
                }
                NPC[A].Special5 = 1;
                if(NPC[A].Special2 > 280)
                {
                    NPC[A].Special2 = 0;
                    NPC[A].Special5 = 0;
                }
            }
            if(NPC[A].Special5 == 0 && NPC[A].Location.SpeedY == Physics.NPCGravity)
            {
                NPC[A].Special3 += 1;
                if(NPC[A].Special3 <= 200)
                    NPC[A].Location.SpeedX = -1;
                else if(NPC[A].Special3 > 500)
                    NPC[A].Special3 = 0;
                else if(NPC[A].Special3 > 250 && NPC[A].Special3 <= 450)
                    NPC[A].Location.SpeedX = 1;
                else
                    NPC[A].Location.SpeedX = 0;
            }
            else
                NPC[A].Location.SpeedX = 0;
        }
        else
        {
            NPC[A].Special5 += 1;
            NPC[A].Location.SpeedX = 0;
        }

        if(NPC[A].Stuck)
            NPC[A].Location.SpeedX = 0;
    }
    else if(NPC[A].Type == NPCID_EXT_TURTLE)
    {
        if(NPC[A].Special > 0)
        {
            NPC[A].Special -= 1;
            NPC[A].Location.SpeedX = 0;
        }
    // beach koopa
    }
    else if(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4)
    {
        if(NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4 && NPC[A].Special > 0)
        {
            NPC[A].Special -= 1;
            NPC[A].Location.SpeedX = 0;
        }

        if(NPC[A].Projectile)
        {
            NPC[A].Location.SpeedX *= 0.96_r;
            if(NPC[A].Location.SpeedX > -0.003_n && NPC[A].Location.SpeedX < 0.003_n)
            {
                NPC[A].Projectile = false;
                NPC[A].Location.Y -= Physics.NPCGravity;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Direction = -NPC[A].Direction;
                // deferring tree update to end of the NPC physics update
            }
        }
        else
        {
            if(NPC[A].Type != NPCID_BLU_HIT_TURTLE_S4)
            {
                if(num_t::fEqual_f(NPC[A].Location.SpeedY, Physics.NPCGravity))
                {
                    Location_t tempLocation = NPC[A].Location;
                    tempLocation.Width += 32;
                    tempLocation.X -= 16;

                    for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                    {
                        if(NPC[B].Active && NPC[B].Section == NPC[A].Section && !NPC[B].Hidden && NPC[B].HoldingPlayer == 0)
                        {
                            if(NPC[B].Type >= NPCID_GRN_SHELL_S4 && NPC[B].Type <= NPCID_YEL_SHELL_S4)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location))
                                {
                                    NPC[A].Location.Y -= Physics.NPCGravity;
                                    NPC[A].Location.SpeedY = -4;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_TOOTHY) // killer plant destroys blocks
    {
        for(int B : treeBlockQuery(NPC[A].Location, SORTMODE_COMPAT))
        {
            if(CheckCollision(NPC[A].Location, Block[B].Location))
            {
                BlockHitHard(B);
            }
        }
    }
    // Projectile code
    else if(NPC[A].Type == NPCID_SLIDE_BLOCK)
    {
        if(NPC[A].Special == 1 && NPC[A].Location.SpeedX != 0)
            NPC[A].Projectile = true;
    }
    else if(NPC[A].Type == NPCID_BULLET)
    {
        if(NPC[A].CantHurt > 0)
            NPC[A].Projectile = true;
    }
    // Projectile check, previously set immediately before calling SpecialNPC
    else if(NPC[A].Type == NPCID_PET_FIRE || NPC[A].Type == NPCID_SWORDBEAM || NPC[A].Type == NPCID_TANK_TREADS)
        NPC[A].Projectile = true;
    else if(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT
        || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG
        || (NPC[A].Type >= NPCID_SHORT_WOOD && NPC[A].Type <= NPCID_SLANT_WOOD_M))
    {
        if(NPC[A].Location.SpeedY > Physics.NPCGravity * 20)
            NPC[A].Projectile = true;
        else
            NPC[A].Projectile = false;
    }
    else if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK)
    {
        if(NPC[A].Location.SpeedY > 2 || NPC[A].Location.SpeedY < -2)
            NPC[A].Projectile = true;
    }
    // NPCs selected by trait (instead of type)
    else
    {
        if(NPC[A]->IsAShell)
        {
            if(NPC[A].Location.SpeedX != 0)
                NPC[A].Projectile = true;
        }
        // moved from above
        else if(NPC[A]->IsFish)
        {
            if(NPC[A].Special == 1 && !NPC[A].Projectile)
            {
                if(NPC[A].Wet == 2)
                    NPC[A].Special5 = 0;

                int B = NPCTargetPlayer(NPC[A]);
                if(B == 0)
                    B = 1;

                if(!Player[B].WetFrame && Player[B].Location.Y + Player[B].Location.Height < NPC[A].Location.Y)
                {
                    if((NPC[A].Direction == 1 && Player[B].Location.X > NPC[A].Location.X) ||
                       (NPC[A].Direction == -1 && Player[B].Location.X < NPC[A].Location.X))
                    {
                        if(NPC[A].Location.X > Player[B].Location.X - 200 && NPC[A].Location.X + NPC[A].Location.Width < Player[B].Location.X + Player[B].Location.Width + 200)
                        {
                            if(NPC[A].Wet == 2)
                            {
                                if(NPC[A].Location.SpeedY > -3)
                                    NPC[A].Location.SpeedY -= 0.1_n;
                                NPC[A].Special3 = 1;
                            }
                        }
                        else
                            NPC[A].Special3 = 0;
                    }
                    else
                        NPC[A].Special3 = 0;

                    if(NPC[A].Special3 == 1 && NPC[A].Wet == 0)
                    {
                        NPC[A].Location.SpeedY = -(NPC[A].Location.Y - Player[B].Location.Y + Player[B].Location.Height / 2) / 20 + dRand() * 4 - 2;
                        if(NPC[A].Location.SpeedY < -9)
                            NPC[A].Location.SpeedY = -9;
                        NPC[A].Special3 = 0;
                        NPC[A].Special5 = 1;

                        // this allows the fish to go through walls after leaping
                        NPC[A].WallDeath = 10;
                    }
                }
            }
        }
    }
}

void CharStuff(int WhatNPC, bool CheckEggs)
{
    bool SMBX = false;
    bool SMB2 = false;
    bool TLOZ = false;
    int A = 0;
    int NPCStart = 0;
    int NPCStop = 0;
    if(GameMenu)
        return;

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Character == 1 || Player[A].Character == 2)
            SMBX = true;
        if(Player[A].Character == 3 || Player[A].Character == 4)
            SMB2 = true;
        if(Player[A].Character == 5)
            TLOZ = true;
    }

    if(WhatNPC == 0)
    {
        NPCStart = 1;
        NPCStop = numNPCs;
    }
    else
    {
        NPCStart = WhatNPC;
        NPCStop = WhatNPC;
    }

    if(!SMBX && SMB2 && CheckEggs) // Turn SMBX stuff into SMB2 stuff
    {
        for(A = NPCStart; A <= NPCStop; A++)
        {
            if(NPC[A].Type == NPCID_ITEM_POD && NPC[A].Special > 0 /* && CheckEggs*/) // Check Eggs
            {
                if(NPCIsYoshi((NPCID)NPC[A].Special)) // Yoshi into mushroom (Egg)
                {
                    // NPC(A).Special = 249
                    NPC[A].Special = NPCID_GRN_BOOT; // Yoshi into boot
                }
            }
        }
    }

    if(!SMBX && !SMB2 && TLOZ) // Turn SMBX stuff into Zelda stuff
    {
        for(int A : NPCQueues::Active.no_change)
        {
            if(WhatNPC != 0)
                A = WhatNPC;

            if(NPC[A].Active && !NPC[A].Generator && !NPC[A].Inert)
            {
                if(NPC[A].Type == NPCID_POWER_S3 || NPC[A].Type == NPCID_POWER_S1 || NPC[A].Type == NPCID_POWER_S4 || NPCIsBoot(NPC[A])) // turn mushrooms into hearts
                {
                    NPC[A].Frame = 0;
                    NPC[A].Type = NPCID_POWER_S5;
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.Y += NPC[A].Location.Height - NPC[A]->THeight - 1;
                    NPC[A].Location.X += (NPC[A].Location.Width - NPC[A]->TWidth) / 2;
                    NPC[A].Location.Width = 32;
                    NPC[A].Location.Height = 32;

                    NPCQueues::Unchecked.push_back(A);
                    treeNPCUpdate(A);
                }
                else if(NPC[A].Type == NPCID_COIN_S3 || NPC[A].Type == NPCID_COIN_S4 || NPC[A].Type == NPCID_COIN_S1 || NPC[A].Type == NPCID_COIN_S2 || NPC[A].Type == NPCID_COIN_5) // turn coins into rupees
                {
                    if(NPC[A].Type == NPCID_COIN_5)
                        NPC[A].Type = NPCID_GEM_5;
                    else
                        NPC[A].Type = NPCID_GEM_1;
                    NPC[A].Location.Y += NPC[A].Location.Height - NPC[A]->THeight;
                    NPC[A].Location.X += (NPC[A].Location.Width - NPC[A]->TWidth) / 2;
                    NPC[A].Location.Width = NPC[A]->TWidth;
                    NPC[A].Location.Height = NPC[A]->THeight;
                    NPC[A].Frame = 0;

                    NPCQueues::Unchecked.push_back(A);
                    treeNPCUpdate(A);
                }
            }

            if(WhatNPC != 0)
                break;
        }
    }

    if(!SMBX && !SMB2 && TLOZ && CheckEggs) // Turn SMBX stuff into Zelda stuff
    {
        for(A = NPCStart; A <= NPCStop; A++)
        {
            if(NPC[A].Type == NPCID_ITEM_POD && NPC[A].Special > 0 /* && CheckEggs*/) // Check Eggs
            {
                if(NPCIsYoshi((NPCID)NPC[A].Special) || NPCIsBoot((NPCID)NPC[A].Special)) // Yoshi / boot into mushroom (Egg)
                    NPC[A].Special = NPCID_POWER_S5;
                if(NPC[A].Special == NPCID_POWER_S3 || NPC[A].Special == NPCID_POWER_S1 || NPC[A].Special == NPCID_POWER_S4) // mushrooms into hearts (eggs)
                    NPC[A].Special = NPCID_POWER_S5;
                if(NPC[A].Special == NPCID_COIN_S3 || NPC[A].Special == NPCID_COIN_S4 || NPC[A].Special == NPCID_COIN_S1 || (!SMB2 && NPC[A].Special == NPCID_COIN_S2)) // coins into rupees (eggs)
                    NPC[A].Special = NPCID_GEM_1;
            }
        }
    }
}

NPCID RandomBonus()
{
    int B = iRand(6);

    switch(B)
    {
    default:
    case 0:
        return NPCID_POWER_S3;
    case 1:
        return NPCID_FIRE_POWER_S3;
    case 2:
        return NPCID_LEAF_POWER;
    case 3:
        return NPCID_STATUE_POWER;
    case 4:
        return NPCID_HEAVY_POWER;
    case 5:
        return NPCID_ICE_POWER_S3;
    }
}

bool npcHasFloor(const struct NPC_t &npc)
{
    bool hasFloor = false;

    if(npc.Type < 1 || npc.Type > maxNPCType)
        return false; // invalid NPC type

    if(npc->NoClipping)
        return false; // No collision with blocks

    const auto &l = npc.Location;

    auto checkLoc = l;
    checkLoc.Y = l.Y + l.Height;
    checkLoc.Height = 4;
    checkLoc.Width = l.Width / 2;
    checkLoc.X = l.X + (l.Width / 2) - (checkLoc.Width / 2);

    // Ensure that there is a floor under feet
    for(BlockRef_t sb : treeBlockQuery(checkLoc, SORTMODE_NONE))
    {
        int idx = sb;

        if(npc.tempBlock == idx)
            continue; // Skip collision check to self

        if(BlockNoClipping[sb->Type] || sb->Hidden || sb->Invis || (npc.Projectile && sb->tempBlockNoProjClipping()))
            continue;

        if(CheckCollision(checkLoc, sb->Location))
        {
            hasFloor = true;
            break;
        }
    }

    return hasFloor;
}

bool NPCNewContainerType(int Type)
{
    return Type == NPCID_SPIT_BOSS;
}

bool NPCIsContainer(const NPC_t& npc)
{
    return npc.Type == NPCID_ITEM_BURIED || npc.Type == NPCID_ITEM_POD
        || npc.Type == NPCID_ITEM_BUBBLE || npc.Type == NPCID_ITEM_THROWER
        || (NPCNewContainerType(npc.Type) && npc.Special != 0);
}

void NPCUnbury(int A, int Callsite)
{
    if(NPC[A].Generator)
    {
        NPC[A].Generator = false;
        NPCQueues::update(A);
    }

    NPC[A].Frame = 0;

    if(Callsite != 1)
        NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);

    if(Callsite != 2)
    {
        NPC[A].Type = NPCID(NPC[A].Special);
        NPC[A].Wings = NPC[A].DefaultWings;
    }

    if(Callsite != 1 && Callsite != 2 && NPC[A].Type == NPCID_RANDOM_POWER)
    {
        NPC[A].Type = RandomBonus();
        NPC[A].DefaultSpecial = NPC[A].Type;
    }

    if(Callsite != 1)
        CharStuff(A);

    NPC[A].Special = 0;

    if(NPCIsYoshi(NPC[A]))
    {
        NPC[A].Special = NPC[A].Type;
        NPC[A].Type = NPCID_ITEM_POD;
    }

    if(Callsite != 2 && Callsite != 3 && (NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT))
    {
        // those types were in the below sequence of checks at Callsite 1
    }
    else if(!(NPC[A].Type == NPCID_CANNONENEMY
        || NPC[A].Type == NPCID_CANNONITEM
        || NPC[A].Type == NPCID_SPRING
        || NPC[A].Type == NPCID_KEY
        || NPC[A].Type == NPCID_COIN_SWITCH
        // || NPC[A].Type == NPCID_TIME_SWITCH
        // || NPC[A].Type == NPCID_TNT
        || NPC[A].Type == NPCID_GRN_BOOT
        || NPC[A].Type == NPCID_RED_BOOT
        || NPC[A].Type == NPCID_BLU_BOOT
        || NPC[A].Type == NPCID_TOOTHYPIPE
        || NPCIsAnExit(NPC[A])))
    {
        if(Callsite == 1 || !BattleMode)
            NPC[A].DefaultType = NPCID_NULL;
    }

    NPC[A].Location.Height = NPC[A]->THeight;
    NPC[A].Location.Width = NPC[A]->TWidth;

    if(Callsite != 2 && NPC[A].Type == NPCID_VEGGIE_RANDOM)
    {
        int B = iRand(9);
        NPC[A].Type = NPCID(NPCID_VEGGIE_2 + B);

        if(NPC[A].Type == NPCID_VEGGIE_RANDOM)
            NPC[A].Type = NPCID_VEGGIE_1;

        NPC[A].Location.set_width_center(NPC[A]->TWidth);
        NPC[A].Location.set_height_center(NPC[A]->THeight);
    }
}

static bool s_TypeBansWings(NPCID Type)
{
    const auto& tr = NPCTraits[Type];
    if(tr.IsAVine)
        return true;

    switch(Type)
    {
    // ban plants
    case NPCID_JUMP_PLANT:
    case NPCID_FIRE_PLANT:
    case NPCID_PLANT_S3:
    case NPCID_BIG_PLANT:
    case NPCID_PLANT_S1:
    case NPCID_LONG_PLANT_UP:
    case NPCID_BOTTOM_PLANT:
    case NPCID_LONG_PLANT_DOWN:
    case NPCID_SIDE_PLANT:
    // ban these types which set location to default location
    case NPCID_LAVA_MONSTER:
    case NPCID_LAVABUBBLE:
    case NPCID_CONVEYOR:
    case NPCID_BOSS_CASE:
    case NPCID_BOSS_FRAGILE:
        return true;
    default:
        return false;
    }
}

bool NPCBansWings(const NPC_t& npc)
{
    if(s_TypeBansWings(npc.Type))
        return true;

    if(npc.Type == NPCID_ITEM_BUBBLE || npc.Type == NPCID_ITEM_BURIED)
    {
        if(s_TypeBansWings((NPCID)npc.Special))
            return true;
    }

    return false;
}
