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

#include "sdl_proxy/sdl_stdinc.h"

#include <algorithm>
#include <array>
#include <bitset>

#include "../globals.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../collision.h"
#include "../editor.h"
#include "../npc.h"
#include "../gfx.h"
#include "../layers.h"
#include "../main/menu_main.h"
#include "../main/speedrunner.h"
#include "../main/trees.h"
#include "../main/screen_pause.h"
#include "../main/screen_connect.h"
#include "../main/screen_quickreconnect.h"
#include "../main/screen_textentry.h"
#include "../compat.h"
#include "../config.h"
#include "../game_main.h"
#include "../main/game_globals.h"
#include "../core/render.h"
#include "../script/luna/luna.h"

#include "effect.h"
#include "npc_id.h"

#include "graphics/gfx_keyhole.h"

#include <fmt_format_ne.h>
#include <Utils/maths.h>

#include "npc/npc_queues.h"

struct ScreenShake_t
{
    double forceX = 0;
    double forceY = 0;
    double forceDecay = 1.0;
    int    type = SHAKE_RANDOM;
    double duration = 0;
    double sign = +1.0;

    bool   active = false;

    void update()
    {
        if(!active || GameMenu)
            return;

        int offsetX, offsetY;

        if(duration <= 0)
        {
            if(forceX > 0)
                forceX -= forceDecay;
            if(forceY > 0)
                forceY -= forceDecay;
        }
        else
            duration--;

        if(forceX <= 0 && forceY <= 0)
        {
            forceX = 0.0;
            forceY = 0.0;
            active = false;
        }
        // always perform this section to keep the number of random calls consistent w/legacy sources
        {
            switch(type)
            {
            default:
            case SHAKE_RANDOM:
                offsetX = iRand(forceX * 4) - forceX * 2;
                offsetY = iRand(forceY * 4) - forceY * 2;
                break;
            case SHAKE_SEQUENTIAL:
                offsetX = forceX > 0 ? (int)round(sign * forceX) : 0;
                offsetY = forceY > 0 ? (int)round(sign * forceY) : 0;
                sign *= -1;
                break;
            }

            XRender::offsetViewport(offsetX, offsetY);
        }
    }

    void setup(int i_forceX, int i_forceY, int i_type, int i_duration, double i_decay)
    {
        if(GameMenu)
            return;

        if((forceX <= 0 && forceY <= 0) || (forceDecay < i_decay))
            forceDecay = i_decay;

        // don't override random shake by sequential while random shake is active
        if((forceX <= 0 && forceY <= 0) || (type != SHAKE_RANDOM))
            type = i_type;

        if(forceX < i_forceX)
            forceX = i_forceX;
        if(forceY < i_forceY)
            forceY = i_forceY;
        if(duration < i_duration)
            duration = i_duration;

        active = true;
    }

    void clear()
    {
        forceX = 0.0;
        forceY = 0.0;
        duration = 0.0;
        active = false;
    }
};

static ScreenShake_t s_shakeScreen;

//static double s_shakeScreenX = 0;
//static double s_shakeScreenY = 0;
//static int s_shakeScreenType = SHAKE_RANDOM;
//static double s_shakeScreenDuration = 0;
//static double s_shakeScreenSign = +1.0;

void doShakeScreen(int force, int type)
{
    s_shakeScreen.setup(force, force, type, 0, 1.0);
}

void doShakeScreen(int forceX, int forceY, int type, int duration, double decay)
{
    s_shakeScreen.setup(forceX, forceY, type, duration, decay);
}

void doShakeScreenClear()
{
    s_shakeScreen.clear();
}


// this organizes all of the NPC draw conditions into one queue
class NPC_Draw_Queue_t
{
#ifndef LOW_MEM
    static constexpr int maxDrawNPCs = maxNPCs;
#else
    static constexpr int maxDrawNPCs = 512;
#endif
public:
    uint16_t BG[maxDrawNPCs];
    size_t BG_n;
    uint16_t Low[maxDrawNPCs];
    size_t Low_n;
    uint16_t Iced[maxDrawNPCs];
    size_t Iced_n;
    uint16_t Normal[maxDrawNPCs];
    size_t Normal_n;
    uint16_t Chat[20];
    size_t Chat_n;
    uint16_t Held[20];
    size_t Held_n;
    uint16_t FG[maxDrawNPCs];
    size_t FG_n;
    uint16_t Dropped[20];
    size_t Dropped_n;
    uint16_t Warning[32];
    size_t Warning_n;

    // reset the draw queue for the frame
    void reset()
    {
        BG_n = Low_n = Iced_n = Normal_n = Chat_n = Held_n = FG_n = Dropped_n = Warning_n = 0;
    }

    // add NPC with index A to the draw queue, according to its type properties
    void add(uint16_t A)
    {
        if(NPC[A].Chat)
        {
            if(Chat_n != sizeof(Chat)/sizeof(uint16_t))
            {
                Chat[Chat_n] = A;
                Chat_n += 1;
                g_stats.renderedNPCs += 1;
            }
        }

        if(NPC[A].Effect == 2)
        {
            if(std::fmod(NPC[A].Effect2, 3) == 0.0)
                return;
            if(Dropped_n == sizeof(Dropped)/sizeof(uint16_t))
                return;
            Dropped[Dropped_n] = A;
            Dropped_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(
                (
                  (
                    (NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
                    (NPC[A].Type == 50 && NPC[A].standingOnPlayer == 0) ||
                    (NPC[A].Type == 17 && NPC[A].CantHurt > 0)
                  ) || NPC[A].Effect == 5
                ) && NPC[A].Type != 91 && !Player[NPC[A].HoldingPlayer].Dead
            )
        {
            if(Held_n == sizeof(Held)/sizeof(uint16_t))
                return;
            Held[Held_n] = A;
            Held_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Effect == 0 && NPCForeground[NPC[A].Type] && NPC[A].HoldingPlayer == 0 && !NPCIsACoin[NPC[A].Type])
        {
            if(FG_n == sizeof(FG)/sizeof(uint16_t))
                return;
            FG[FG_n] = A;
            FG_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Type == 263 && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            if(Iced_n == sizeof(Iced)/sizeof(uint16_t))
                return;
            Iced[Iced_n] = A;
            Iced_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0 &&
            (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 || NPC[A].Type == 22 ||
                NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160 ||
                NPC[A].Type == 282 || NPCIsACoin[NPC[A].Type]))
        {
            if(Low_n == sizeof(Low)/sizeof(uint16_t))
                return;
            Low[Low_n] = A;
            Low_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Type == 179 || NPC[A].Type == 270 ||
            ((NPC[A].Effect == 208 || NPCIsAVine[NPC[A].Type] ||
                    NPC[A].Type == 209 || NPC[A].Type == 159 || NPC[A].Type == 245 ||
                    NPC[A].Type == 8 || NPC[A].Type == 93 || NPC[A].Type == 74 ||
                    NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 51 ||
                    NPC[A].Type == 52 || NPC[A].Effect == 1 || NPC[A].Effect == 3 ||
                    NPC[A].Effect == 4 || (NPC[A].Type == 45 && NPC[A].Special == 0.0))
                && NPC[A].standingOnPlayer == 0))
        {
            if(BG_n == sizeof(BG)/sizeof(uint16_t))
                return;
            BG[BG_n] = A;
            BG_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            if(Normal_n == sizeof(Normal)/sizeof(uint16_t))
                return;
            Normal[Normal_n] = A;
            Normal_n += 1;
            g_stats.renderedNPCs += 1;
        }
    }
};

NPC_Draw_Queue_t NPC_Draw_Queue[2] = {NPC_Draw_Queue_t(), NPC_Draw_Queue_t()};

// code to facilitate cached values for the onscreen blocks and BGOs
// query results of last update
static std::vector<BlockRef_t> s_drawMainBlocks[2] = {std::vector<BlockRef_t>(400), std::vector<BlockRef_t>(400)};
static std::vector<BlockRef_t> s_drawLavaBlocks[2] = {std::vector<BlockRef_t>(100), std::vector<BlockRef_t>(100)};
static std::vector<BlockRef_t> s_drawSBlocks[2] = {std::vector<BlockRef_t>(40), std::vector<BlockRef_t>(40)};
static std::vector<BaseRef_t> s_drawBGOs[2] = {std::vector<BaseRef_t>(400), std::vector<BaseRef_t>(400)};

// query location of last update
static Location_t s_drawBlocks_bounds[2];
static Location_t s_drawBGOs_bounds[2];

// maximum amount of layer movement since last update
static double s_drawBlocks_invalidate_timer[2] = {0, 0};
static double s_drawBGOs_invalidate_timer[2] = {0, 0};

// global: force-invalidate the cache when the blocks themselves change
bool g_drawBlocks_valid[2] = {false, false};
bool g_drawBGOs_valid[2] = {false, false};

// global: based on layer movement speed, set in layers.cpp
double g_drawBlocks_invalidate_rate = 0;
double g_drawBGOs_invalidate_rate = 0;


// Performance-tweakable code. This is the margin away from the vScreen that is filled when onscreen blocks and BGOs are calculated.
//   Larger values result in more offscreen blocks / BGOs being checked.
//   Smaller values result in more frequent block / BGO table queries.
constexpr double i_drawBlocks_margin = 64;
constexpr double i_drawBGOs_margin = 128;

// updates the lists of blocks and BGOs to draw on vScreen Z
void s_UpdateDrawItems(int Z)
{
    int i = Z - 1;
    if(i < 0 || i >= 2)
        return;

    // based on layer movement speed
    s_drawBlocks_invalidate_timer[i] += g_drawBlocks_invalidate_rate;
    s_drawBGOs_invalidate_timer[i] += g_drawBGOs_invalidate_rate;


    // update draw blocks if needed
    if(!g_drawBlocks_valid[i]
        || -vScreenX[Z]                     < s_drawBlocks_bounds[i].X                                 + s_drawBlocks_invalidate_timer[i]
        || -vScreenX[Z] + vScreen[Z].Width  > s_drawBlocks_bounds[i].X + s_drawBlocks_bounds[i].Width  - s_drawBlocks_invalidate_timer[i]
        || -vScreenY[Z]                     < s_drawBlocks_bounds[i].Y                                 + s_drawBlocks_invalidate_timer[i]
        || -vScreenY[Z] + vScreen[Z].Height > s_drawBlocks_bounds[i].Y + s_drawBlocks_bounds[i].Height - s_drawBlocks_invalidate_timer[i])
    {
        g_drawBlocks_valid[i] = true;
        s_drawBlocks_invalidate_timer[i] = 0;

        // form query location
        s_drawBlocks_bounds[i] = newLoc(-vScreenX[Z] - i_drawBlocks_margin,
            -vScreenY[Z] - i_drawBlocks_margin,
            vScreen[Z].Width + i_drawBlocks_margin * 2,
            vScreen[Z].Height + i_drawBlocks_margin * 2);

        // make query (sort by ID as done in vanilla)
        TreeResult_Sentinel<BlockRef_t> areaBlocks = treeBlockQuery(s_drawBlocks_bounds[i], SORTMODE_ID);

        // load query results into different sets of blocks
        s_drawSBlocks[i].clear();
        s_drawMainBlocks[i].clear();
        s_drawLavaBlocks[i].clear();

        for(BlockRef_t b : areaBlocks)
        {
            if(b->Hidden)
                continue;
            if(b->Type == 0)
                continue;

            if(BlockIsSizable[b->Type])
                s_drawSBlocks[i].push_back(b);
            else if(BlockKills[b->Type])
                s_drawLavaBlocks[i].push_back(b);
            else
                s_drawMainBlocks[i].push_back(b);
        }

        // sort the Sizable blocks
        // cross-ref the dead code at sorting.cpp:qSortSBlocks
        std::stable_sort(s_drawSBlocks[i].begin(), s_drawSBlocks[i].end(),
            [](BlockRef_t a, BlockRef_t b) {
                return a->Location.Y < b->Location.Y;
            });
    }

    // update draw BGOs if needed
    if(!g_drawBGOs_valid[i]
        || -vScreenX[Z]                     < s_drawBGOs_bounds[i].X                               + s_drawBGOs_invalidate_timer[i]
        || -vScreenX[Z] + vScreen[Z].Width  > s_drawBGOs_bounds[i].X + s_drawBGOs_bounds[i].Width  - s_drawBGOs_invalidate_timer[i]
        || -vScreenY[Z]                     < s_drawBGOs_bounds[i].Y                               + s_drawBGOs_invalidate_timer[i]
        || -vScreenY[Z] + vScreen[Z].Height > s_drawBGOs_bounds[i].Y + s_drawBGOs_bounds[i].Height - s_drawBGOs_invalidate_timer[i])
    {
        g_drawBGOs_valid[i] = true;
        s_drawBGOs_invalidate_timer[i] = 0;

        // form query location
        s_drawBGOs_bounds[i] = newLoc(-vScreenX[Z] - i_drawBGOs_margin,
            -vScreenY[Z] - i_drawBGOs_margin,
            vScreen[Z].Width + i_drawBGOs_margin * 2,
            vScreen[Z].Height + i_drawBGOs_margin * 2);

        // make query (sort by ID as done in vanilla)
        s_drawBGOs[i].clear();
        treeBackgroundQuery(s_drawBGOs[i], s_drawBGOs_bounds[i], SORTMODE_ID);
    }
}

void GraphicsLazyPreLoad()
{
    // TODO: check if this is needed at caller
    SetupScreens();

    int numScreens = 1;

    if(ScreenType == 1)
        numScreens = 2;
    if(ScreenType == 4)
        numScreens = 2;
    if(ScreenType == 5)
    {
        DynamicScreen();
        if(vScreen[2].Visible)
            numScreens = 2;
        else
            numScreens = 1;
    }
    if(ScreenType == 8)
        numScreens = 1;


    if(SingleCoop == 2)
        numScreens = 1; // fine to be 1, since it would just be run for Z = 2 twice otherwise;

    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        int S = Player[Z].Section;
        int bg = Background2[S];

        switch(bg)
        {
        case 1: // Double-row background
            XRender::lazyPreLoad(GFXBackground2[1]);
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;

        case 2: // Single-row clouds background
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;

        case 3: // Double-row background
            XRender::lazyPreLoad(GFXBackground2[3]);
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;
        case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
            // All these backgrounds do use picture with the number less with 1
            XRender::lazyPreLoad(GFXBackground2[bg - 1]);
            break;
        case 22: // Double-row background
            XRender::lazyPreLoad(GFXBackground2[22]);
            XRender::lazyPreLoad(GFXBackground2[2]);
            break;

        default: // Any other normal backgrounds
            if(bg < 1 || bg > maxBackgroundType)
                break; // Don't crash it, stupid!
            XRender::lazyPreLoad(GFXBackground2[bg]);
            break;
        }

        For(A, 1, numPlayers)
        {
            Player_t &p = Player[A];
            int c = p.Character;
            int s = p.State;

            switch(c)
            {
            case 1:
                XRender::lazyPreLoad(GFXMarioBMP[s]);
                break;
            case 2:
                XRender::lazyPreLoad(GFXLuigiBMP[s]);
                break;
            case 3:
                XRender::lazyPreLoad(GFXPeachBMP[s]);
                break;
            case 4:
                XRender::lazyPreLoad(GFXToadBMP[s]);
                break;
            case 5:
                XRender::lazyPreLoad(GFXLinkBMP[s]);
                break;
            default: // Trap
                abort(); // "Please fix me up if you implemented a new playable character, see gfx_update.cpp!"
                return;
            }
        }

        // int64_t fBlock = 0;
        // int64_t lBlock = 0;
        // blockTileGet(-vScreenX[Z], vScreen[Z].Width, fBlock, lBlock);
        s_UpdateDrawItems(Z);

        for(Block_t& b : s_drawSBlocks[Z - 1])
        {
            if(vScreenCollision(Z, b.Location) && !b.Hidden && IF_INRANGE(b.Type, 1, maxBlockType))
                XRender::lazyPreLoad(GFXBlock[b.Type]);
        }

        for(Block_t& b : s_drawMainBlocks[Z - 1])
        {
            if(vScreenCollision(Z, b.Location) && !b.Hidden && IF_INRANGE(b.Type, 1, maxBlockType))
                XRender::lazyPreLoad(GFXBlock[b.Type]);
        }

        for(Block_t& b : s_drawLavaBlocks[Z - 1])
        {
            if(vScreenCollision(Z, b.Location) && !b.Hidden && IF_INRANGE(b.Type, 1, maxBlockType))
                XRender::lazyPreLoad(GFXBlock[b.Type]);
        }

        for(BackgroundRef_t bgo : s_drawBGOs[Z - 1])
        {
            Background_t& b = bgo;
            if(vScreenCollision(Z, b.Location) && !b.Hidden && IF_INRANGE(b.Type, 1, maxBackgroundType))
                XRender::lazyPreLoad(GFXBackgroundBMP[b.Type]);
        }

        for(int A = 1; A <= numNPCs; A++)
        {
            auto &n = NPC[A];
            if(vScreenCollision(Z, n.Location) && IF_INRANGE(n.Type, 0, maxNPCType))
                XRender::lazyPreLoad(GFXNPC[n.Type]);
        }
    }
}

// swappable buffer for previous frame's NoReset NPCs
static std::vector<NPCRef_t> s_NoReset_NPCs_LastFrame;

// does the classic ("onscreen") NPC activation / reset logic for vScreen Z, directly based on the many NPC loops of the original game
void ClassicNPCScreenLogic(int Z, int numScreens, bool Do_FrameSkip, NPC_Draw_Queue_t& NPC_Draw_Queue_p)
{
    // using bitset here instead of simpler set for checkNPCs because I benchmarked it to be faster -- ds-sloth
    static std::bitset<maxNPCs> NPC_present;

    // find the onscreen NPCs
    TreeResult_Sentinel<NPCRef_t> _screenNPCs = treeNPCQuery(-vScreenX[Z], -vScreenY[Z],
        -vScreenX[Z] + vScreen[Z].Width, -vScreenY[Z] + vScreen[Z].Height,
        SORTMODE_NONE);

    // combine the onscreen NPCs with the no-reset NPCs
    std::vector<BaseRef_t>& checkNPCs = *_screenNPCs.i_vec;

    // mark previous ones as checked
    for(int16_t n : checkNPCs)
        NPC_present[n] = true;

    // add the previous frame's no-reset NPCs
    for(int16_t n : s_NoReset_NPCs_LastFrame)
    {
        if(n <= numNPCs && !NPC_present[n])
        {
            checkNPCs.push_back(n);
            NPC_present[n] = true;
        }
    }

    // cleanup the checked NPCs
    for(int16_t n : checkNPCs)
        NPC_present[n] = false;

    // allocate it outside the loop; use it only when needed
    Location_t npcALoc;

    // following logic is somewhat difficult to read but includes the precise conditions
    // from each NPC check in the original UpdateGraphics to determine how to handle each NPC
    for(int A : checkNPCs)
    {
        g_stats.checkedNPCs++;

        bool has_ALoc = false;
        bool check_both_reset = false;
        bool check_long_life = false;
        bool activate_conveyer = false;
        bool kill_zero = false;
        bool set_justactivated = true;
        bool can_check = false;
        bool reset_all = true;

        if(((NPC[A].Effect == 208 || NPCIsAVine[NPC[A].Type] ||
             NPC[A].Type == 209 || NPC[A].Type == 159 || NPC[A].Type == 245 ||
             NPC[A].Type == 8 || NPC[A].Type == 93 || NPC[A].Type == 74 ||
             NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 51 ||
             NPC[A].Type == 52 || NPC[A].Effect == 1 || NPC[A].Effect == 3 ||
             NPC[A].Effect == 4 || (NPC[A].Type == 45 && NPC[A].Special == 0.0)) &&
             (NPC[A].standingOnPlayer == 0 && (!NPC[A].Generator || LevelEditor))) ||
             NPC[A].Type == 179 || NPC[A].Type == 270)
        {
            if(NPC[A].Effect != 2 && (!NPC[A].Generator || LevelEditor))
            {
                can_check = true;
            }
        }

        if(NPC[A].Effect == 0 && ((NPC[A].HoldingPlayer == 0 && (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 ||
                                   NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160 ||
                                   NPC[A].Type == 282 || NPCIsACoin[NPC[A].Type]) && (!NPC[A].Generator || LevelEditor))))
        {
            npcALoc = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0,
                                  NPC[A].Location.Y,
                                  static_cast<double>(NPCWidthGFX[NPC[A].Type]),
                                  static_cast<double>(NPCHeight[NPC[A].Type]));
            has_ALoc = true;
            can_check = true;
        }

        if(NPC[A].Type == 263 && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            npcALoc = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0,
                                  NPC[A].Location.Y,
                                  static_cast<double>(NPCWidthGFX[NPC[A].Type]),
                                  static_cast<double>(NPCHeight[NPC[A].Type]));

            has_ALoc = true;
            can_check = true;
        }


        if(NPC[A].Effect == 0)
        {
            if(!(NPC[A].HoldingPlayer > 0 || NPCIsAVine[NPC[A].Type] || NPC[A].Type == 209 || NPC[A].Type == 282 ||
                 NPC[A].Type == 270 || NPC[A].Type == 160 || NPC[A].Type == 159 || NPC[A].Type == 8 || NPC[A].Type == 245 ||
                 NPC[A].Type == 93 || NPC[A].Type == 51 || NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 256 ||
                 NPC[A].Type == 257 || NPC[A].Type == 56 || NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91) &&
               !(NPC[A].Type == 45 && NPC[A].Special == 0) && NPC[A].standingOnPlayer == 0 &&
               !NPCForeground[NPC[A].Type] && (!NPC[A].Generator || LevelEditor) &&
               NPC[A].Type != 179 && NPC[A].Type != 263)
            {
                if(!NPCIsACoin[NPC[A].Type])
                {
                    can_check = true;
                    kill_zero = true;
                    activate_conveyer = true;
                    check_both_reset = true;
                    check_long_life = true;
                }
            }
        }


        if(
            (
              (
                (NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
                (NPC[A].Type == 50 && NPC[A].standingOnPlayer == 0) ||
                (NPC[A].Type == 17 && NPC[A].CantHurt > 0)
              ) || NPC[A].Effect == 5
            ) && NPC[A].Type != 91 && !Player[NPC[A].HoldingPlayer].Dead
        )
        {
            NPC_Draw_Queue_p.add(A);
        }

        if(NPC[A].Effect == 0)
        {
            if(NPCForeground[NPC[A].Type] && NPC[A].HoldingPlayer == 0 && (!NPC[A].Generator || LevelEditor))
            {
                if(!NPCIsACoin[NPC[A].Type])
                {
                    can_check = true;
                    check_both_reset = true;
                }
            }
        }

        if(NPC[A].Generator)
        {
            if(vScreenCollision(Z, NPC[A].Location) && !NPC[A].Hidden)
                NPC[A].GeneratorActive = true;
        }

        if(NPC[A].Effect == 2)
        {
            if(std::fmod(NPC[A].Effect2, 3) != 0.0)
            {
                can_check = true;
                reset_all = false;
                set_justactivated = false;
            }
        }

        if(!can_check)
        {
            continue;
        }

        if((vScreenCollision(Z, NPC[A].Location) || (has_ALoc && vScreenCollision(Z, npcALoc))) && !NPC[A].Hidden)
        {
            if(kill_zero && NPC[A].Type == 0)
            {
                NPC[A].Killed = 9;
                KillNPC(A, 9);
            }
            else if(NPC[A].Active && !Do_FrameSkip)
            {
                NPC_Draw_Queue_p.add(A);
            }

            if((NPC[A].Reset[Z] && (!check_both_reset || NPC[A].Reset[3 - Z])) || NPC[A].Active || (activate_conveyer && NPC[A].Type == 57))
            {
                if(set_justactivated && !NPC[A].Active)
                {
                    NPC[A].JustActivated = Z;
                }
                NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                if(check_long_life && (NPCIsYoshi[NPC[A].Type] || NPCIsBoot[NPC[A].Type] || NPC[A].Type == 9 || NPC[A].Type == 14 || NPC[A].Type == 22 || NPC[A].Type == 90 || NPC[A].Type == 153 || NPC[A].Type == 169 || NPC[A].Type == 170 || NPC[A].Type == 182 || NPC[A].Type == 183 || NPC[A].Type == 184 || NPC[A].Type == 185 || NPC[A].Type == 186 || NPC[A].Type == 187 || NPC[A].Type == 188 || NPC[A].Type == 195 || NPC[A].Type == 104))
                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen * 20;

                if(!NPC[A].Active)
                {
                    NPCQueues::Active.insert(A);
                    NPC[A].Active = true;
                }
            }
            NPC[A].Reset[1] = false;
            NPC[A].Reset[2] = false;

            NPCQueues::NoReset.push_back(A);
        }
        else
        {
            NPC[A].Reset[Z] = true;
            if(reset_all)
            {
                if(numScreens == 1)
                    NPC[A].Reset[2] = true;
                if(SingleCoop == 1)
                    NPC[A].Reset[2] = true;
                else if(SingleCoop == 2)
                    NPC[A].Reset[1] = true;
            }
        }
    }
}

// This draws the graphic to the screen when in a level/game menu/outro/level editor
void UpdateGraphics(bool skipRepaint)
{
//    On Error Resume Next
    float c = ShadowMode ? 0.f : 1.f;
    int A = 0;
//    std::string timeStr;
    int Z = 0;

    if(!GameIsActive)
        return;

#ifdef USE_RENDER_BLOCKING
    // might want to put this after the logic part of UpdateGraphics,
    // once we have merged the multires code that separates logic from
    // rendering
    if(XRender::renderBlocked())
        return;
#endif

    // frame skip code
    cycleNextInc();

    bool Do_FrameSkip = FrameSkip && !TakeScreen;

    // Note: never frame skip in this case, because the legacy code doesn't update the fields used by frameSkipNeeded()
    if(!g_compatibility.fix_keyhole_framerate && LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
        Do_FrameSkip = false;

    if(Do_FrameSkip)
        Do_FrameSkip = frameSkipNeeded();

    // ALL graphics-based logic code has been moved here, separate from rendering.
    // (This code is a combination of the FrameSkip logic from before with the
    //   logic components of the full rendering code.)
    // NPC render queue formation is also here.
    SetupScreens();

    int numScreens = 1;
    if(ScreenType == 1)
        numScreens = 2;

    if(ScreenType == 4)
        numScreens = 2;

    if(ScreenType == 5)
    {
        DynamicScreen();

        if(vScreen[2].Visible)
            numScreens = 2;
        else
            numScreens = 1;
    }

    if(ScreenType == 8)
        numScreens = 1;

    if(SingleCoop == 2)
        numScreens = 2;

    g_stats.reset();

    // prepare to fill this frame's NoReset queue
    std::swap(NPCQueues::NoReset, s_NoReset_NPCs_LastFrame);
    NPCQueues::NoReset.clear();

    for(Z = 1; Z <= numScreens; Z++)
    {
        if(SingleCoop == 2)
            Z = 2;

        int S;
        if(LevelEditor)
            S = curSection;
        else
            S = Player[Z].Section;

        // update vScreen location
        if(!LevelEditor)
        {
            if(ScreenType == 2 || ScreenType == 3)
                GetvScreenAverage();
            else if(ScreenType == 5 && !vScreen[2].Visible)
                GetvScreenAverage();
            else if(ScreenType == 7)
                GetvScreenCredits();
            else
                GetvScreen(Z);
        }

        if(!Do_FrameSkip && qScreen)
        {
            if(vScreenX[1] < qScreenX[1] - 2)
                qScreenX[1] -= 2;
            else if(vScreenX[1] > qScreenX[1] + 2)
                qScreenX[1] += 2;
            if(vScreenY[1] < qScreenY[1] - 2)
                qScreenY[1] -= 2;
            else if(vScreenY[1] > qScreenY[1] + 2)
                qScreenY[1] += 2;
            if(qScreenX[1] < vScreenX[1] + 5 && qScreenX[1] > vScreenX[1] - 5 &&
               qScreenY[1] < vScreenY[1] + 5 && qScreenY[1] > vScreenY[1] - 5)
                qScreen = false;
            vScreenX[1] = qScreenX[1];
            vScreenY[1] = qScreenY[1];
        }

        // noturningback
        if(!LevelEditor && NoTurnBack[Player[Z].Section])
        {
            A = Z;
            if(numScreens > 1)
            {
                if(Player[1].Section == Player[2].Section)
                {
                    if(Z == 1)
                        GetvScreen(2);
                    if(-vScreenX[1] < -vScreenX[2])
                        A = 1;
                    else
                        A = 2;
                }
            }
            if(-vScreenX[A] > level[S].X)
            {
                LevelChop[S] += float(-vScreenX[A] - level[S].X);
                level[S].X = -vScreenX[A];
            }
        }

        // Position swap code?
        if(!GameMenu && !LevelEditor)
        {
            if(numPlayers > 2)
            {
                int C = 0;
                int D = 0;
//                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
//                    With Player(A)
                    Player_t &p = Player[A];
//                        If vScreenCollision(Z, .Location) = False And LevelMacro = 0 And .Location.Y < level(.Section).Height And .Location.Y + .Location.Height > level(.Section).Y And .TimeToLive = 0 And .Dead = False Then
                    if(!vScreenCollision(Z, p.Location) && LevelMacro == LEVELMACRO_OFF &&
                        p.Location.Y < level[p.Section].Height &&
                        p.Location.Y + p.Location.Height > level[p.Section].Y &&
                        p.TimeToLive == 0 && !p.Dead)
                    {
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Section == Player[A].Section && vScreenCollision(Z, Player[B].Location))
                            {
                                if(C == 0 || std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                {
                                    C = std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                    D = B;
                                }
                            }
                        }
                        if(C == 0)
                        {
                            for(int B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].TimeToLive == 0 && Player[B].Section == Player[A].Section)
                                {
                                    if(C == 0 || std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(Player[A].Location.X + Player[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                        D = B;
                                    }
                                }
                            }
                        }
                        Player[A].Location.X = Player[D].Location.X + Player[D].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                        Player[A].Location.Y = Player[D].Location.Y + Player[D].Location.Height - Player[A].Location.Height;
                        Player[A].Section = Player[D].Section;
                        Player[A].Location.SpeedX = Player[D].Location.SpeedX;
                        Player[A].Location.SpeedY = Player[D].Location.SpeedY;
                        Player[A].Location.SpeedY = dRand() * 12 - 6;
                        Player[A].CanJump = true;
                    }
//                    End With
//                Next A
                }
//            End If
            }
        }

        // It's time to process NPCs. We will update their active state and fill a draw queue.

        // Make sure we are in range.
        // If we later add more than two screens,
        // need to change how many NPC draw queues we have.
        SDL_assert_release(Z-1 >= 0 && Z-1 < (int)(sizeof(NPC_Draw_Queue) / sizeof(NPC_Draw_Queue_t)));
        NPC_Draw_Queue_t& NPC_Draw_Queue_p = NPC_Draw_Queue[Z-1];
        if(!Do_FrameSkip)
            NPC_Draw_Queue_p.reset();

        // we'll check the NPCs and do some logic for the game,
        if(!LevelEditor)
        {
            ClassicNPCScreenLogic(Z, numScreens, Do_FrameSkip, NPC_Draw_Queue_p);
        }
        // fill the NPC render queue for the level editor
        else if(!Do_FrameSkip)
        {
            for(A = 1; A <= numNPCs; A++)
            {
                g_stats.checkedNPCs++;

                if(NPC[A].Hidden)
                    continue;

                const Location_t loc2 = newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0,
                    NPC[A].Location.Y,
                    NPCWidthGFX[NPC[A].Type], NPCHeight[NPC[A].Type]);

                if(vScreenCollision(Z, NPC[A].Location) || vScreenCollision(Z, loc2))
                {
                    NPC_Draw_Queue_p.add(A);
                }
            }
        }
    }

    // we've now done all the logic that UpdateGraphics can do.
    if(Do_FrameSkip)
        return;

    XRender::setTargetTexture();

    frameNextInc();
    frameRenderStart();
    lunaRenderStart();

    // std::string SuperText;
    // std::string tempText;
    // int BoxY = 0;
    // bool tempBool = false;
    int B = 0;
//    int B2 = 0;
    // int C = 0;
    // int D = 0;
    // int E = 0;
    // double d2 = 0;
//    int e2 = 0;
//    int X = 0;
    int Y = 0;
    // int64_t fBlock = 0;
    // int64_t lBlock = 0;
    Location_t tempLocation;

    if(Score > 9999990)
        Score = 9999990;

    if(Lives > 99)
        Lives = 99;

//    If TakeScreen = True Then // Useless
//        If LevelEditor = True Or MagicHand = True Then
//            frmLevelWindow.vScreen(1).AutoRedraw = True
//        Else
//            frmMain.AutoRedraw = True
//        End If
//    End If

    // Background frames
    if(!FreezeNPCs)
    {
        BackgroundFrameCount[26]++;
        if(BackgroundFrameCount[26] >= 8)
        {
            BackgroundFrame[26]++;
            if(BackgroundFrame[26] >= 8)
                BackgroundFrame[26] = 0;
            BackgroundFrameCount[26] = 0;
        }
        BackgroundFrameCount[18]++;
        if(BackgroundFrameCount[18] >= 12)
        {
            BackgroundFrame[18]++;
            if(BackgroundFrame[18] >= 4)
                BackgroundFrame[18] = 0;
            BackgroundFrame[19] = BackgroundFrame[18];
            BackgroundFrame[20] = BackgroundFrame[18];
            BackgroundFrame[161] = BackgroundFrame[18];
            BackgroundFrameCount[18] = 0;
        }
        BackgroundFrameCount[36] += 1;
        if(BackgroundFrameCount[36] >= 2)
        {
            BackgroundFrame[36] += 1;
            if(BackgroundFrame[36] >= 4)
                BackgroundFrame[36] = 0;
            BackgroundFrameCount[36] = 0;
        }
        BackgroundFrame[68] = BackgroundFrame[36];
        BackgroundFrameCount[65] += 1;
        if(BackgroundFrameCount[65] >= 8)
        {
            BackgroundFrame[65] += 1;
            if(BackgroundFrame[65] >= 4)
                BackgroundFrame[65] = 0;
            BackgroundFrameCount[65] = 0;
        }

        BackgroundFrame[66] = BackgroundFrame[65];

        BackgroundFrame[70] = BackgroundFrame[65];
        BackgroundFrame[100] = BackgroundFrame[65];

        BackgroundFrame[134] = BackgroundFrame[65];
        BackgroundFrame[135] = BackgroundFrame[65];
        BackgroundFrame[136] = BackgroundFrame[65];
        BackgroundFrame[137] = BackgroundFrame[65];
        BackgroundFrame[138] = BackgroundFrame[65];


        BackgroundFrameCount[82] += 1;
        if(BackgroundFrameCount[82] >= 10)
        {
            BackgroundFrame[82] += 1;
            if(BackgroundFrame[82] >= 4)
                BackgroundFrame[82] = 0;
            BackgroundFrameCount[82] = 0;
        }

        BackgroundFrameCount[170] += 1;
        if(BackgroundFrameCount[170] >= 8)
        {
            BackgroundFrame[170] += 1;
            if(BackgroundFrame[170] >= 4)
                BackgroundFrame[170] = 0;
            BackgroundFrame[171] = BackgroundFrame[170];
            BackgroundFrameCount[170] = 0;
        }

        BackgroundFrameCount[125] += 1;
        if(BackgroundFrameCount[125] >= 4)
        {
            if(BackgroundFrame[125] == 0)
                BackgroundFrame[125] = 1;
            else
                BackgroundFrame[125] = 0;
            BackgroundFrameCount[125] = 0;
        }
        SpecialFrames();
    }

    BackgroundFrame[172] = BackgroundFrame[66];
    BackgroundFrameCount[158] += 1;

    if(BackgroundFrameCount[158] >= 6)
    {
        BackgroundFrameCount[158] = 0;
        BackgroundFrame[158] += 1;
        BackgroundFrame[159] += 1;
        if(BackgroundFrame[158] >= 4)
            BackgroundFrame[158] = 0;
        if(BackgroundFrame[159] >= 8)
            BackgroundFrame[159] = 0;
    }

    BackgroundFrameCount[168] += 1;
    if(BackgroundFrameCount[168] >= 8)
    {
        BackgroundFrame[168] += 1;
        if(BackgroundFrame[168] >= 8)
            BackgroundFrame[168] = 0;
        BackgroundFrameCount[168] = 0;
    }

    BackgroundFrameCount[173] += 1;
    if(BackgroundFrameCount[173] >= 8)
    {
        BackgroundFrameCount[173] = 0;
        if(BackgroundFrame[173] == 0)
            BackgroundFrame[173] = 1;
        else
            BackgroundFrame[173] = 0;
    }

    BackgroundFrameCount[187] += 1;
    if(BackgroundFrameCount[187] >= 6)
    {
        BackgroundFrame[187] += 1;
        if(BackgroundFrame[187] >= 4)
            BackgroundFrame[187] = 0;
        BackgroundFrame[188] = BackgroundFrame[187];
        BackgroundFrame[189] = BackgroundFrame[187];
        BackgroundFrame[190] = BackgroundFrame[187];
        BackgroundFrameCount[187] = 0;
    }


    if(ClearBuffer)
    {
        ClearBuffer = false;
        XRender::clearBuffer();
    }

#ifdef __3DS__
        XRender::setTargetLayer(0);
#endif

    // No logic
    // Draw the screens!
    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        int S;
        if(LevelEditor)
            S = curSection;
        else
            S = Player[Z].Section;

        // (Code to get vScreen moved into logic section above.)

#ifdef __3DS__
        if(Z != 1)
            XRender::setTargetLayer(0);
#endif

        // Note: this was guarded by an if(!LevelEditor) condition in the past
        if(Background2[S] == 0)
        {
            if(numScreens > 1)
                XRender::renderRect(vScreen[Z].Left, vScreen[Z].Top,
                                    vScreen[Z].Width, vScreen[Z].Height, 0.f, 0.f, 0.f, 1.f, true);
            else
                XRender::clearBuffer();
        }

        // Get a reference to our NPC draw queue.
        // Make sure we are in range.
        // If we later add more than two screens,
        // need to change how many NPC draw queues we have.
        SDL_assert_release(Z-1 >= 0 && Z-1 < (int)(sizeof(NPC_Draw_Queue) / sizeof(NPC_Draw_Queue_t)));
        NPC_Draw_Queue_t& NPC_Draw_Queue_p = NPC_Draw_Queue[Z-1];

        if(numScreens > 1) // To separate drawing of screens
            XRender::setViewport(vScreen[Z].Left, vScreen[Z].Top, vScreen[Z].Width, vScreen[Z].Height);

        DrawBackground(S, Z);

        // don't show background outside of the current section!
        if(LevelEditor)
        {
            if(vScreenX[Z] + level[S].X > 0) {
                XRender::renderRect(0, 0,
                                   vScreenX[Z] + level[S].X, ScreenH, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if(ScreenW > level[S].Width + vScreenX[Z]) {
                XRender::renderRect(level[S].Width + vScreenX[Z], 0,
                                   ScreenW - (level[S].Width + vScreenX[Z]), ScreenH, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if(vScreenY[Z] + level[S].Y > 0) {
                XRender::renderRect(0, 0,
                                   ScreenW, vScreenY[Z] + level[S].Y, 0.2f, 0.2f, 0.2f, 1.f, true);
            }
            if(ScreenH > level[S].Height + vScreenY[Z]) {
                XRender::renderRect(0, level[S].Height + vScreenY[Z],
                                   ScreenW, ScreenH - (level[S].Height + vScreenY[Z]), 0.2f, 0.2f, 0.2f, 1.f, true);
            }
        }


//        If GameMenu = True Then
        // if(GameMenu)
        // {
            // (Commented out in original code :thinking:)
            // Curtain
//            XRender::renderTexture(0, 0, GFX.MenuGFX[1]);
            // Game logo
//            XRender::renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70, GFX.MenuGFX[2]);
//        ElseIf LevelEditor = False Then
        if(!GameMenu && !LevelEditor)
        {
//            If numPlayers > 2 And nPlay.Online = False Then

            // moved strange many-player handling code to logic section above

//        End If
        }

#ifdef __3DS__
        XRender::setTargetLayer(1);
#endif

        // update the vectors of all the onscreen blocks and backgrounds for use at multiple places
        s_UpdateDrawItems(Z);
        const std::vector<BlockRef_t>& screenMainBlocks = s_drawMainBlocks[Z - 1];
        const std::vector<BlockRef_t>& screenLavaBlocks = s_drawLavaBlocks[Z - 1];
        const std::vector<BlockRef_t>& screenSBlocks = s_drawSBlocks[Z - 1];
        const std::vector<BaseRef_t>& screenBackgrounds = s_drawBGOs[Z - 1];

        int nextBackground = 0;

        if(LevelEditor)
        {
            for(int A : screenBackgrounds)
            {
                if(A > numBackground)
                    break;

                if(Background[A].Type == 11 || Background[A].Type == 12 || Background[A].Type == 60
                    || Background[A].Type == 61 || Background[A].Type == 75 || Background[A].Type == 76
                    || Background[A].Type == 77 || Background[A].Type == 78 || Background[A].Type == 79)
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                              vScreenY[Z] + Background[A].Location.Y,
                                              GFXBackgroundWidth[Background[A].Type],
                                              BackgroundHeight[Background[A].Type],
                                              GFXBackgroundBMP[Background[A].Type], 0,
                                              BackgroundHeight[Background[A].Type] *
                                              BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }
        else
        {
//            For A = 1 To MidBackground - 1 'First backgrounds
            for(; nextBackground < (int)screenBackgrounds.size() && (int)screenBackgrounds[nextBackground] < MidBackground; nextBackground++)  // First backgrounds
            {
                A = screenBackgrounds[nextBackground];
                g_stats.checkedBGOs++;
//                if(BackgroundHasNoMask[Background[A].Type] == false) // Useless code
//                {
//                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
//                    {
//                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
//                                              vScreenY[Z] + Background[A].Location.Y,
//                                              BackgroundWidth[Background[A].Type],
//                                              BackgroundHeight[Background[A].Type],
//                                              GFXBackgroundBMP[Background[A].Type], 0,
//                                              BackgroundHeight[Background[A].Type] *
//                                              BackgroundFrame[Background[A].Type]);
//                    }
//                }
//                else
//                {
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                          vScreenY[Z] + Background[A].Location.Y,
                                          GFXBackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type], 0,
                                          BackgroundHeight[Background[A].Type] *
                                          BackgroundFrame[Background[A].Type]);
                }
            }
        }

        tempLocation.Width = 32;
        tempLocation.Height = 32;

        for(Block_t& b : screenSBlocks) // Display sizable blocks
        {
            g_stats.checkedSzBlocks++;
            if(/*BlockIsSizable[b.Type] &&*/ (!b.Invis || LevelEditor))
            {
                double bLeftOnscreen = vScreenX[Z] + b.Location.X;
                if(bLeftOnscreen > vScreen[Z].Width)
                    continue;
                double bRightOnscreen = bLeftOnscreen + b.Location.Width;
                if(bRightOnscreen < 0)
                    continue;

                double bTopOnscreen = vScreenY[Z] + b.Location.Y;
                if(bTopOnscreen > vScreen[Z].Height)
                    continue;
                double bBottomOnscreen = bTopOnscreen + b.Location.Height;
                if(bBottomOnscreen < 0)
                    continue;

                g_stats.renderedSzBlocks++;

                bool show_left = true;
                if(bLeftOnscreen <= -32)
                {
                    show_left = false;
                    bLeftOnscreen = std::fmod(bLeftOnscreen, 32);
                }

                bool show_top = true;
                if(bTopOnscreen <= -32)
                {
                    show_top = false;
                    bTopOnscreen = std::fmod(bTopOnscreen, 32);
                }

                double colSemiLast = bRightOnscreen - 64;
                double rowSemiLast = bBottomOnscreen - 64;

                if(bRightOnscreen > vScreen[Z].Width)
                    bRightOnscreen = vScreen[Z].Width;
                if(bBottomOnscreen > vScreen[Z].Height)
                    bBottomOnscreen = vScreen[Z].Height;

                // first row source
                int src_y = show_top ? 0 : 32;

                for(double dst_y = bTopOnscreen; dst_y < bBottomOnscreen; dst_y += 32.0)
                {
                    // first col source
                    int src_x = show_left ? 0 : 32;

                    for(double dst_x = bLeftOnscreen; dst_x < bRightOnscreen; dst_x += 32.0)
                    {
                        XRender::renderTexture(dst_x, dst_y, 32, 32, GFXBlockBMP[b.Type], src_x, src_y);

                        // next col source
                        if(dst_x >= colSemiLast)
                            src_x = 64;
                        else
                            src_x = 32;
                    }

                    // next row source
                    if(dst_y >= rowSemiLast)
                        src_y = 64;
                    else
                        src_y = 32;
                }
            }
        }

        if(LevelEditor)
        {
            for(int A : screenBackgrounds)
            {
                if(A > numBackground)
                    break;

                if(!(Background[A].Type == 11 || Background[A].Type == 12 || Background[A].Type == 60
                    || Background[A].Type == 61 || Background[A].Type == 75 || Background[A].Type == 76
                    || Background[A].Type == 77 || Background[A].Type == 78 || Background[A].Type == 79) && !Foreground[Background[A].Type])
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                              vScreenY[Z] + Background[A].Location.Y,
                                              GFXBackgroundWidth[Background[A].Type],
                                              BackgroundHeight[Background[A].Type],
                                              GFXBackgroundBMP[Background[A].Type], 0,
                                              BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }
        else if(numBackground > 0)
        {
            for(; nextBackground < (int)screenBackgrounds.size() && (int)screenBackgrounds[nextBackground] <= LastBackground; nextBackground++)  // Second backgrounds
            {
                A = screenBackgrounds[nextBackground];

                g_stats.checkedBGOs++;

                if(Background[A].Hidden)
                    continue;

                double sX = vScreenX[Z] + Background[A].Location.X;
                if(sX > vScreen[Z].Width)
                    continue;

                double sY = vScreenY[Z] + Background[A].Location.Y;
                if(sY > vScreen[Z].Height)
                    continue;

                if(sX + Background[A].Location.Width >= 0 && sY + Background[A].Location.Height >= 0 /*&& !Background[A].Hidden*/)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(sX,
                                          sY,
                                          BackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type],
                                          0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }

        for(int oBackground = screenBackgrounds.size() - 1; oBackground > 0 && (int)screenBackgrounds[oBackground] > numBackground; oBackground--)  // Locked doors
        {
            A = screenBackgrounds[oBackground];

            g_stats.checkedBGOs++;
            if(vScreenCollision(Z, Background[A].Location) &&
                (Background[A].Type == 98 || Background[A].Type == 160) && !Background[A].Hidden)
            {
                g_stats.renderedBGOs++;
                XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                      vScreenY[Z] + Background[A].Location.Y,
                                      BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type],
                                      GFXBackgroundBMP[Background[A].Type],
                                      0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
            }
        }

#ifdef __3DS__
        XRender::setTargetLayer(2);
#endif

//        For A = 1 To numNPCs 'Display NPCs that should be behind blocks
        for(size_t i = 0; i < NPC_Draw_Queue_p.BG_n; i++)
        {
            A = NPC_Draw_Queue_p.BG[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    {
                        {
                            if(NPC[A].Type == 8 || NPC[A].Type == 74 || NPC[A].Type == 93 || NPC[A].Type == 245 || NPC[A].Type == 256 || NPC[A].Type == 270)
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                            }
                            else if(NPC[A].Type == 51 || NPC[A].Type == 257)
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type],
                                        vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type],
                                        NPC[A].Location.Width, NPC[A].Location.Height,
                                        GFXNPC[NPC[A].Type], 0,
                                        NPC[A].Frame * NPCHeight[NPC[A].Type] + NPCHeight[NPC[A].Type] - NPC[A].Location.Height,
                                        cn, cn, cn);
                            }
                            else if(NPC[A].Type == 52)
                            {
                                if(NPC[A].Direction == -1)
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type]);
                                }
                                else
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], NPCWidth[NPC[A].Type] - NPC[A].Location.Width, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                                }
                            }
                            else if(NPCWidthGFX[NPC[A].Type] == 0 || NPC[A].Effect == 1)
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn ,cn ,cn);
                            }
                            else
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
                    }
                }
            }
        }


//        For A = 1 To numPlayers 'Players behind blocks
        For(A, 1, numPlayers)
        {
            if(!Player[A].Dead && !Player[A].Immune2 && Player[A].TimeToLive == 0 && Player[A].Effect == 3)
            {
                float Y2 = 0;
                float X2 = 0;

                if(vScreenCollision(Z, Player[A].Location))
                {
                    if(Player[A].Character == 5 && Player[A].Frame > 5)
                        Player[A].Frame = 1;

                    const Player_t& p = Player[A];

                    if(p.Character < 1 || p.Character > 5)
                        continue;

                    // warp NPCs
                    if(p.HoldingNPC > 0 && p.Frame != 15)
                    {
                        const NPC_t& hNpc = NPC[p.HoldingNPC];

                        auto warpNpcLoc = newLoc(hNpc.Location.X - (NPCWidthGFX[hNpc.Type] - hNpc.Location.Width) / 2.0,
                                                 hNpc.Location.Y,
                                                 static_cast<double>(NPCWidthGFX[hNpc.Type]),
                                                 static_cast<double>(NPCHeight[hNpc.Type]));

                        if((vScreenCollision(Z, hNpc.Location) || vScreenCollision(Z, warpNpcLoc)) && !hNpc.Hidden)
                        {
                            tempLocation = hNpc.Location;
                            if(NPCHeightGFX[hNpc.Type] != 0 || NPCWidthGFX[hNpc.Type] != 0)
                            {
                                tempLocation.Height = NPCHeightGFX[hNpc.Type];
                                tempLocation.Width = NPCWidthGFX[hNpc.Type];
                                tempLocation.Y = hNpc.Location.Y + hNpc.Location.Height - NPCHeightGFX[hNpc.Type];
                                tempLocation.X = hNpc.Location.X + hNpc.Location.Width / 2.0 - NPCWidthGFX[hNpc.Type] / 2.0;
                            }
                            else
                            {
                                tempLocation.Height = NPCHeight[hNpc.Type];
                                tempLocation.Width = NPCWidth[hNpc.Type];
                            }

                            tempLocation.X += NPCFrameOffsetX[hNpc.Type];
                            tempLocation.Y += NPCFrameOffsetY[hNpc.Type];
                            Y2 = 0;
                            X2 = 0;

                            NPCWarpGFX(A, tempLocation, X2, Y2);

                            if(NPCHeightGFX[hNpc.Type] != 0 || NPCWidthGFX[hNpc.Type] != 0)
                            {
                                XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                      vScreenY[Z] + tempLocation.Y,
                                                      tempLocation.Width,
                                                      tempLocation.Height,
                                                      GFXNPC[hNpc.Type],
                                                      X2,
                                                      Y2 + hNpc.Frame * NPCHeightGFX[hNpc.Type]);
                            }
                            else
                            {
                                XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                      vScreenY[Z] + tempLocation.Y,
                                                      tempLocation.Width,
                                                      tempLocation.Height,
                                                      GFXNPC[hNpc.Type],
                                                      X2,
                                                      Y2 + hNpc.Frame * NPCHeight[hNpc.Type]);
                            }
                        }
                    }

                    if(p.Mount == 3)
                    {
                        B = p.MountType;
                        // Yoshi's Body
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += p.YoshiBX;
                        tempLocation.Y += p.YoshiBY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFXYoshiB[B],
                                              X2,
                                              Y2 + 32 * p.YoshiBFrame,
                                              c, c, c);
                        // Yoshi's Head
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += p.YoshiTX;
                        tempLocation.Y += p.YoshiTY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFXYoshiT[B],
                                              X2,
                                              Y2 + 32 * p.YoshiTFrame);
                    }

                    // DONE: Convert this mess of duplicated code into united with usage of references, pointers, and ternary expressions

                    using plr_pic_arr = RangeArr<StdPicture, 1, 10>;
                    constexpr std::array<plr_pic_arr*, 5> char_tex = {&GFXMario, &GFXLuigi, &GFXPeach, &GFXToad, &GFXLink};

                    StdPicture& tx = (*char_tex[p.Character - 1])[p.State];

                    int offX = pfrOffX(tx, p);
                    int offY = pfrOffY(tx, p);
                    int w = pfrW(tx, p);
                    int h = pfrH(tx, p);

                    if(p.Mount == 1)
                    {
                        int small_h_corr
                            = (p.State != 0)
                            ? 0
                            : (p.Character == 4)
                              ? -26
                              : -30;
                        int toad_oy_corr
                            = (p.Character != 4)
                            ? 0
                            : (p.State == 1)
                              ?  6
                              : -4;
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = p.Location.Height - offY + small_h_corr;
                        tempLocation.Width = w;
                        tempLocation.X += offX;
                        tempLocation.Y += offY + toad_oy_corr;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              tx,
                                              pfrX(tx, p) + X2,
                                              pfrY(tx, p) + Y2,
                                              c, c, c);

                        // boot GFX
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += p.Location.Width / 2.0 - 16;
                        tempLocation.Y += p.Location.Height - 30;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFX.Boot[p.MountType],
                                              X2,
                                              Y2 + 32 * p.MountFrame,
                                              c, c, c);
                    }
                    else
                    {
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = h;
                        tempLocation.Width = w;
                        tempLocation.X += offX;
                        tempLocation.Y += offY + p.MountOffsetY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                              vScreenY[Z] + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              tx,
                                              pfrX(tx, p) + X2,
                                              pfrY(tx, p) + Y2,
                                              c, c, c);
                    }
                }

                if(Player[A].HoldingNPC > 0 && Player[A].Frame == 15)
                {
                    auto &hNpc = NPC[Player[A].HoldingNPC];
                    auto hNpcLoc = newLoc(hNpc.Location.X - (NPCWidthGFX[hNpc.Type] - hNpc.Location.Width) / 2.0,
                                          hNpc.Location.Y,
                                          static_cast<double>(NPCWidthGFX[hNpc.Type]),
                                          static_cast<double>(NPCHeight[hNpc.Type]));

                    if((vScreenCollision(Z, hNpc.Location) || vScreenCollision(Z, hNpcLoc)) && !hNpc.Hidden && hNpc.Type != 263)
                    {
                        tempLocation = hNpc.Location;
                        if(NPCHeightGFX[hNpc.Type] != 0 || NPCWidthGFX[hNpc.Type] != 0)
                        {
                            tempLocation.Height = NPCHeightGFX[hNpc.Type];
                            tempLocation.Width = NPCWidthGFX[hNpc.Type];
                            tempLocation.Y = hNpc.Location.Y + hNpc.Location.Height - NPCHeightGFX[hNpc.Type];
                            tempLocation.X = hNpc.Location.X + hNpc.Location.Width / 2.0 - NPCWidthGFX[hNpc.Type] / 2.0;
                        }
                        else
                        {
                            tempLocation.Height = NPCHeight[hNpc.Type];
                            tempLocation.Width = NPCWidth[hNpc.Type];
                        }

                        tempLocation.X += NPCFrameOffsetX[hNpc.Type];
                        tempLocation.Y += NPCFrameOffsetY[hNpc.Type];
                        Y2 = 0;
                        X2 = 0;

                        NPCWarpGFX(A, tempLocation, X2, Y2);
                        if(NPCHeightGFX[hNpc.Type] != 0 || NPCWidthGFX[hNpc.Type] != 0)
                        {
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXNPC[hNpc.Type],
                                                  X2,
                                                  Y2 + hNpc.Frame * NPCHeightGFX[hNpc.Type]);
                        }
                        else
                        {
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                                  vScreenY[Z] + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXNPC[hNpc.Type],
                                                  X2,
                                                  Y2 + hNpc.Frame * NPCHeight[hNpc.Type]);
                        }
                    }
                }
            }
        }


//        if(LevelEditor)
//        {
//            fBlock = 1;
//            lBlock = numBlock;
//        }
//        else
//        {
//            //fBlock = FirstBlock[int(-vScreenX[Z] / 32) - 1];
//            //lBlock = LastBlock[int((-vScreenX[Z] + vScreen[Z].Width) / 32) + 1];
//            blockTileGet(-vScreenX[Z], vScreen[Z].Width, fBlock, lBlock);
//        }

//        For A = fBlock To lBlock 'Non-Sizable Blocks
        for(Block_t& block : screenMainBlocks)
        {
            g_stats.checkedBlocks++;

            if(/*!BlockIsSizable[block.Type] &&*/ (!block.Invis || (LevelEditor && BlockFlash <= 30)) /*&& block.Type != 0 && !BlockKills[block.Type]*/)
            {
                double sX = vScreenX[Z] + block.Location.X;
                if(sX > vScreen[Z].Width)
                    continue;

                double sY = vScreenY[Z] + block.Location.Y;
                if(sY > vScreen[Z].Height)
                    continue;

                if(sX + block.Location.Width >= 0 && sY + block.Location.Height >= 0 /*&& !block.Hidden*/)
                {
                    g_stats.renderedBlocks++;
                    // Don't show a visual difference of hit-resized block in a comparison to original state
                    bool wasShrinkResized = block.getShrinkResized();
                    double offX = wasShrinkResized ? 0.05 : 0.0;
                    double offW = wasShrinkResized ? 0.1 : 0.0;
                    XRender::renderTexture(sX - offX,
                                          sY + block.ShakeY3,
                                          block.Location.Width + offW,
                                          block.Location.Height,
                                          GFXBlock[block.Type],
                                          0,
                                          BlockFrame[block.Type] * 32);
                }
            }
        }

//'effects in back
        for(A = 1; A <= numEffects; A++)
        {
            g_stats.checkedEffects++;
            if(Effect[A].Type == 112 || Effect[A].Type == 54 || Effect[A].Type == 55 ||
               Effect[A].Type == 59 || Effect[A].Type == 77 || Effect[A].Type == 81 ||
               Effect[A].Type == 82 || Effect[A].Type == 103 || Effect[A].Type == 104 ||
               Effect[A].Type == 114 || Effect[A].Type == 123 || Effect[A].Type == 124)
            {
                g_stats.renderedEffects++;
                if(vScreenCollision(Z, Effect[A].Location))
                {
                    float cn = Effect[A].Shadow ? 0.f : 1.f;
                    XRender::renderTexture(vScreenX[Z] + Effect[A].Location.X,
                                           vScreenY[Z] + Effect[A].Location.Y,
                                           Effect[A].Location.Width,
                                           Effect[A].Location.Height,
                                           GFXEffect[Effect[A].Type], 0,
                                           Effect[A].Frame * EffectHeight[Effect[A].Type], cn, cn, cn);
                }
            }
        }


        // draw NPCs that should be behind other NPCs
        for(size_t i = 0; i < NPC_Draw_Queue_p.Low_n; i++)
        {
            A = NPC_Draw_Queue_p.Low[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    // If Not NPCIsACoin(.Type) Then
                    {
                        {
                            if(NPCWidthGFX[NPC[A].Type] == 0)
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                            }
                            else
                            {
                                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
                    }
                }
            }
        }


        // ice
        for(size_t i = 0; i < NPC_Draw_Queue_p.Iced_n; i++)
        {
            A = NPC_Draw_Queue_p.Iced[i];
            {
                {
                    DrawFrozenNPC(Z, A);
                }
            }
        }


//        For A = 1 To numNPCs 'Display NPCs that should be in front of blocks
        for(size_t i = 0; i < NPC_Draw_Queue_p.Normal_n; i++)
        {
            A = NPC_Draw_Queue_p.Normal[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    {
                        {
                            {
                                if(!NPCIsYoshi[NPC[A].Type])
                                {
                                    if(NPCWidthGFX[NPC[A].Type] == 0)
                                    {
                                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                    }
                                    else
                                    {
                                        if(NPC[A].Type == 283 && NPC[A].Special > 0)
                                        {
                                            if(NPCWidthGFX[NPC[A].Special] == 0)
                                            {
                                                tempLocation.Width = NPCWidth[NPC[A].Special];
                                                tempLocation.Height = NPCHeight[NPC[A].Special];
                                            }
                                            else
                                            {
                                                tempLocation.Width = NPCWidthGFX[NPC[A].Special];
                                                tempLocation.Height = NPCHeightGFX[NPC[A].Special];
                                            }
                                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - tempLocation.Height / 2.0;

                                            B = EditorNPCFrame((int)SDL_floor(NPC[A].Special), NPC[A].Direction);
                                            XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[A].Special], 0, B * tempLocation.Height);
                                        }

                                        XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                    }
                                }
                                else
                                {
                                    if(NPC[A].Type == 95)
                                        B = 1;
                                    else if(NPC[A].Type == 98)
                                        B = 2;
                                    else if(NPC[A].Type == 99)
                                        B = 3;
                                    else if(NPC[A].Type == 100)
                                        B = 4;
                                    else if(NPC[A].Type == 148)
                                        B = 5;
                                    else if(NPC[A].Type == 149)
                                        B = 6;
                                    else if(NPC[A].Type == 150)
                                        B = 7;
                                    else if(NPC[A].Type == 228)
                                        B = 8;
                                    int YoshiBX = 0;
                                    int YoshiBY = 0;
                                    int YoshiTX = 0;
                                    int YoshiTY = 0;
                                    int YoshiTFrame = 0;
                                    int YoshiBFrame = 0;
                                    YoshiBX = 0;
                                    YoshiBY = 0;
                                    YoshiTX = 20;
                                    YoshiTY = -32;
                                    YoshiBFrame = 6;
                                    YoshiTFrame = 0;
                                    if(NPC[A].Special == 0.0)
                                    {
                                        if(!FreezeNPCs)
                                            NPC[A].FrameCount += 1;
                                        if(NPC[A].FrameCount >= 70)
                                        {
                                            if(!FreezeNPCs)
                                                NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount >= 50)
                                            YoshiTFrame = 3;
                                    }
                                    else
                                    {
                                        if(!FreezeNPCs)
                                            NPC[A].FrameCount += 1;
                                        if(NPC[A].FrameCount > 8)
                                        {
                                            YoshiBFrame = 0;
                                            NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount > 6)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX -= 1;
                                            YoshiTY += 2;
                                            YoshiBY += 1;
                                        }
                                        else if(NPC[A].FrameCount > 4)
                                        {
                                            YoshiBFrame = 2;
                                            YoshiTX -= 2;
                                            YoshiTY += 4;
                                            YoshiBY += 2;
                                        }
                                        else if(NPC[A].FrameCount > 2)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX -= 1;
                                            YoshiTY += 2;
                                            YoshiBY += 1;
                                        }
                                        else
                                            YoshiBFrame = 0;
                                        if(!FreezeNPCs)
                                            NPC[A].Special2 += 1;
                                        if(NPC[A].Special2 > 30)
                                        {
                                            YoshiTFrame = 0;
                                            if(!FreezeNPCs)
                                                NPC[A].Special2 = 0;
                                        }
                                        else if(NPC[A].Special2 > 10)
                                            YoshiTFrame = 2;

                                    }
                                    if(YoshiBFrame == 6)
                                    {
                                        YoshiBY += 10;
                                        YoshiTY += 10;
                                    }
                                    if(NPC[A].Direction == 1)
                                    {
                                        YoshiTFrame += 5;
                                        YoshiBFrame += 7;
                                    }
                                    else
                                    {
                                        YoshiBX = -YoshiBX;
                                        YoshiTX = -YoshiTX;
                                    }
                                    // YoshiBX += 4
                                    // YoshiTX += 4
                                    g_stats.renderedNPCs++;
                                    // Yoshi's Body
                                    XRender::renderTexture(vScreenX[Z] + SDL_floor(NPC[A].Location.X) + YoshiBX, vScreenY[Z] + NPC[A].Location.Y + YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * YoshiBFrame, cn, cn, cn);

                                    // Yoshi's Head
                                    XRender::renderTexture(vScreenX[Z] + SDL_floor(NPC[A].Location.X) + YoshiTX, vScreenY[Z] + NPC[A].Location.Y + YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * YoshiTFrame, cn, cn, cn);
                                }
                            }
                        }
                    }
                }
            }
//        Next A
        }

        // npc chat bubble
        for(size_t i = 0; i < NPC_Draw_Queue_p.Chat_n; i++)
        {
            A = NPC_Draw_Queue_p.Chat[i];
            {
                B = NPCHeightGFX[NPC[A].Type] - NPC[A].Location.Height;
                if(B < 0)
                    B = 0;
                XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPC[A].Location.Width / 2.0 - GFX.Chat.w / 2, vScreenY[Z] + NPC[A].Location.Y - 30 - B, GFX.Chat.w, GFX.Chat.h, GFX.Chat, 0, 0);
            }
        }


        For(A, 1, numPlayers) // The clown car
        {
            if(!Player[A].Dead && !Player[A].Immune2 && Player[A].TimeToLive == 0 &&
               !(Player[A].Effect == 3 || Player[A].Effect == 5) && Player[A].Mount == 2)
            {
                const Player_t& p = Player[A];

                using plr_pic_arr = RangeArr<StdPicture, 1, 10>;
                constexpr std::array<plr_pic_arr*, 5> char_tex = {&GFXMario, &GFXLuigi, &GFXPeach, &GFXToad, &GFXLink};

                StdPicture& tx = (*char_tex[p.Character - 1])[p.State];

                switch(Player[A].Character)
                {
                default:
                case 1: // Mario
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 36;
                    break;

                case 2: // Luigi
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 38;
                    break;

                case 3: // Peach
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 30;
                    break;

                case 4: // Toad
                    if(Player[A].State == 1)
                        Y = 24;
                    else
                        Y = 30;
                    break;

                case 5: // Link
                    Y = 30;
                    break;
                }

                XRender::renderTexture(
                        vScreenX[Z] + SDL_floor(Player[A].Location.X) + pfrOffX(tx, p) - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64,
                        vScreenY[Z] + Player[A].Location.Y + pfrOffY(tx, p) + Player[A].MountOffsetY - Y,
                        pfrW(tx, p), // was 99, not 100, but not a big deal
                        Player[A].Location.Height - 20 - Player[A].MountOffsetY,
                        tx,
                        pfrX(tx, p),
                        pfrY(tx, p),
                        c, c, c);
                XRender::renderTexture(
                        vScreenX[Z] + SDL_floor(Player[A].Location.X) + Player[A].Location.Width / 2.0 - 64,
                        vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128,
                        128,
                        128,
                        GFX.Mount[Player[A].Mount],
                        0,
                        128 * Player[A].MountFrame,
                        c, c, c);
            }
        }

        if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT && LevelMacroWhich != 0)
        {
            RenderKeyhole(Z);
        }

        // Put held NPCs on top
        for(size_t i = 0; i < NPC_Draw_Queue_p.Held_n; i++)
        {
            A = NPC_Draw_Queue_p.Held[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                if(NPC[A].Type == 263)
                {
                    DrawFrozenNPC(Z, A);
                }
                else if(!NPCIsYoshi[NPC[A].Type] && NPC[A].Type > 0)
                {
                    if(NPCWidthGFX[NPC[A].Type] == 0)
                    {
                        RenderTexturePlayer(Z, vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                    }
                    else
                    {
                        RenderTexturePlayer(Z, vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                    }
                }
            }
        }



//'normal player draw code
//        For A = numPlayers To 1 Step -1 'Players in front of blocks
        for(int A = numPlayers; A >= 1; A--)// Players in front of blocks
        {
//            DrawPlayer A, Z
            DrawPlayer(Player[A], Z);
//        Next A
        }
//'normal player end




        if(LevelEditor)
        {
            for(int A : screenBackgrounds)
            {
                if(A > numBackground)
                    continue;

                if(Foreground[Background[A].Type])
                {
                    g_stats.checkedBGOs++;
                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                    {
                        g_stats.renderedBGOs++;
                        XRender::renderTexture(vScreenX[Z] + Background[A].Location.X,
                                              vScreenY[Z] + Background[A].Location.Y,
                                              GFXBackgroundWidth[Background[A].Type],
                                              BackgroundHeight[Background[A].Type],
                                              GFXBackgroundBMP[Background[A].Type], 0,
                                              BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }
        else
        {
            for(; nextBackground < (int)screenBackgrounds.size() && (int)screenBackgrounds[nextBackground] <= numBackground; nextBackground++)  // Foreground objects
            {
                A = screenBackgrounds[nextBackground];

                g_stats.checkedBGOs++;

                if(Background[A].Hidden)
                    continue;

                double sX = vScreenX[Z] + Background[A].Location.X;
                if(sX > vScreen[Z].Width)
                    continue;

                double sY = vScreenY[Z] + Background[A].Location.Y;
                if(sY > vScreen[Z].Height)
                    continue;

                if(sX + Background[A].Location.Width >= 0 && sY + Background[A].Location.Height >= 0 /*&& !Background[A].Hidden*/)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(sX, sY, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
//        End If
        }

        // foreground NPCs
        for(size_t i = 0; i < NPC_Draw_Queue_p.FG_n; i++)
        {
            A = NPC_Draw_Queue_p.FG[i];
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            {
                {
                    {
                        {
                            {
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                }
                                else
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Blocks in Front
        for(Block_t& block : screenLavaBlocks)
        {
            g_stats.checkedBlocks++;

            // if(BlockKills[block.Type])
            {
                if(vScreenCollision(Z, block.Location) /*&& !block.Hidden*/)
                {
                    g_stats.renderedBlocks++;
                    // Don't show a visual difference of hit-resized block in a comparison to original state
                    bool wasShrinkResized = block.getShrinkResized();
                    double offX = wasShrinkResized ? 0.05 : 0.0;
                    double offW = wasShrinkResized ? 0.1 : 0.0;
                    XRender::renderTexture(vScreenX[Z] + block.Location.X - offX,
                                          vScreenY[Z] + block.Location.Y + block.ShakeY3,
                                          block.Location.Width + offW,
                                          block.Location.Height,
                                          GFXBlock[block.Type],
                                          0,
                                          BlockFrame[block.Type] * 32);
                }
            }
        }

// effects on top
        For(A, 1, numEffects)
        {
            g_stats.checkedEffects++;
//            With Effect(A)
            auto &e = Effect[A];
//                If .Type <> 112 And .Type <> 54 And .Type <> 55 And .Type <> 59 And .Type <> 77 And .Type <> 81 And .Type <> 82 And .Type <> 103 And .Type <> 104 And .Type <> 114 And .Type <> 123 And .Type <> 124 Then
            if(e.Type != 112 && e.Type != 54 && e.Type != 55 && e.Type != 59 &&
               e.Type != 77 && e.Type != 81 && e.Type != 82 && e.Type != 103 &&
               e.Type != 104 && e.Type != 114 && e.Type != 123 && e.Type != 124)
            {
//                    If vScreenCollision(Z, .Location) Then
                if(vScreenCollision(Z, e.Location))
                {
                    g_stats.renderedEffects++;
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffectMask(.Type), 0, .Frame * EffectHeight(.Type), vbSrcAnd
//                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffect(.Type), 0, .Frame * EffectHeight(.Type), vbSrcPaint
                    float c = e.Shadow ? 0.f : 1.f;
                    XRender::renderTexture(vb6Round(vScreenX[Z] + e.Location.X),
                                           vb6Round(vScreenY[Z] + e.Location.Y),
                                           vb6Round(e.Location.Width),
                                           vb6Round(e.Location.Height),
                                           GFXEffectBMP[e.Type], 0, e.Frame * EffectHeight[e.Type], c, c, c);
//                    End If
                }
//                End If
            }
//            End With
//        Next A
        }

        // water
        if(LevelEditor)
        {
            for(int B : treeWaterQuery(-vScreenX[Z], -vScreenY[Z],
                -vScreenX[Z] + vScreen[Z].Width, -vScreenY[Z] + vScreen[Z].Height,
                SORTMODE_ID))
            {
                if(!Water[B].Hidden && vScreenCollision(Z, Water[B].Location))
                {
                    if(Water[B].Quicksand)
                        XRender::renderRect(vScreenX[Z] + Water[B].Location.X, vScreenY[Z] + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            1.f, 1.f, 0.f, 1.f, false);
                    else
                        XRender::renderRect(vScreenX[Z] + Water[B].Location.X, vScreenY[Z] + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            0.f, 1.f, 1.f, 1.f, false);
                }
            }
        }

        if(!LevelEditor) // Graphics for the main game.
        {
            if(vScreen[2].Visible)
            {
                if(int(vScreen[Z].Width) == ScreenW)
                {
                    if(vScreen[Z].Top != 0.0)
                    {
                        XRender::renderRect(0, 0, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        XRender::renderRect(0, vScreen[Z].Height - 1, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                }
                else
                {
                    if(vScreen[Z].Left != 0.0)
                    {
                        XRender::renderRect(0, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        XRender::renderRect(vScreen[Z].Width - 1, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                }
            }

        // player names
        /* Dropped */

            lunaRender(Z);

            // Always draw for single-player
            // And don't draw when many players at the same screen
            if(numPlayers == 1 || numScreens != 1)
                g_levelVScreenFader[Z].draw();

#ifdef __3DS__
        XRender::setTargetLayer(3);
#endif

    //    'Interface
    //            B = 0
            B = 0;
    //            C = 0
            // C = 0;
    //            If GameMenu = False And GameOutro = False Then
            if(!GameMenu && !GameOutro)
            {
    //                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
                    if(Player[A].ShowWarp > 0)
                    {
                        std::string tempString;
                        auto &w = Warp[Player[A].ShowWarp];
                        if(!w.noPrintStars && w.maxStars > 0 && Player[A].Mount != 2)
                        {
                            tempString = fmt::format_ne("{0}/{1}", w.curStars, w.maxStars);
                            XRender::renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z] + 1, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                            XRender::renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 - 20, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z], GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                            SuperPrint(tempString, 3,
                                       float(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 + 18),
                                       float(Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z]));
                        }
                    }
                }

                lunaRenderHud();
    //                DrawInterface Z, numScreens
                if(ShowOnScreenHUD && !gSMBXHUDSettings.skip)
                    DrawInterface(Z, numScreens);

                // Display NPCs that got dropped from the container
                for(size_t i = 0; i < NPC_Draw_Queue_p.Dropped_n; i++)
                {
                    A = NPC_Draw_Queue_p.Dropped[i];
                    {
                        {
                            {
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height);
                                }
                                else
                                {
                                    XRender::renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type]);
                                }
                            }
                        }
                    }
                }
            }

            else if(!GameOutro)
                mainMenuDraw();

            if(PrintFPS > 0)
            {
                XRender::offsetViewportIgnore(true);
                SuperPrint(fmt::format_ne("{0}", int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);

                for(int i = 0; i < 2; i++)
                {
                    int val = (i == 0 ? g_microStats.view_total : g_microStats.view_slow_frame_time);

                    int x = val < 95 ? 36 : val < 995 ? 18 : 0;
                    int y = 24 + i * 16;

                    float r, g, a;
                    if(val < 500)
                    {
                        r = 0.0f; g = 1.0f; a = 0.25f;
                    }
                    else if(val < 1000)
                    {
                        r = 1.0f; g = 1.0f; a = 0.5f;
                    }
                    else
                    {
                        r = 1.0f; g = 0.0f; a = 1.0f;
                    }

                    std::string display = (i == 0
                        ? fmt::sprintf_ne("%d%%", (val + 5) / 10)
                        : fmt::sprintf_ne("%d%% (max)", (val + 5) / 10));
                    SuperPrint(display, 3, x, y, r, g, 0.f, a);
                }
                XRender::offsetViewportIgnore(false);
            }

            g_stats.print();

            if(!BattleMode && !GameMenu && !GameOutro && g_config.show_episode_title)
            {
                int y = (ScreenH >= 640) ? 20 : ScreenH - 60;
                if(g_config.show_episode_title == Config_t::EPISODE_TITLE_TRANSPARENT)
                    SuperPrintScreenCenter(WorldName, 3, y, 1.f, 1.f, 1.f, 0.5f);
                else
                    SuperPrintScreenCenter(WorldName, 3, y, 1.f, 1.f, 1.f, 1.f);
            }

        }

//        If LevelEditor = True Or MagicHand = True Then
        if((LevelEditor || MagicHand))
        {
            XRender::offsetViewportIgnore(true);

            // editor code now located in `gfx_editor.cpp`
            DrawEditorLevel(Z);

            XRender::offsetViewportIgnore(false);
        }

        if(numScreens > 1) // for multiple screens
            XRender::setViewport(0, 0, ScreenW, ScreenH);

        if(GameOutro)
            DrawCredits();

//        If LevelEditor = True Then
//            StretchBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//        Else
        { // NOT AN EDITOR!!!
            s_shakeScreen.update();
        }

        // TODO: VERIFY THIS
        XRender::offsetViewportIgnore(true);
        if(ScreenType == 5 && numScreens == 1)
        {
            speedRun_renderControls(1, -1);
            speedRun_renderControls(2, -1);
        }
        else
            speedRun_renderControls(Z, Z);
        XRender::offsetViewportIgnore(false);

//    Next Z
    } // For(Z, 2, numScreens)

    XRender::offsetViewportIgnore(true);
    XRender::setViewport(0, 0, ScreenW, ScreenH);

    g_levelScreenFader.draw();

    speedRun_renderTimer();

    if(LevelEditor || MagicHand)
        DrawEditorLevel_UI();

    // render special screens
    if(GamePaused == PauseCode::PauseScreen)
        PauseScreen::Render();

    if(GamePaused == PauseCode::Message)
    {
        if(MessageTextMap.empty())
            DrawMessage(MessageText);
        else
            DrawMessage(MessageTextMap);
    }

    if(QuickReconnectScreen::g_active)
        QuickReconnectScreen::Render();

    if(GamePaused == PauseCode::Reconnect || GamePaused == PauseCode::DropAdd)
    {
        ConnectScreen::Render();
        XRender::renderTexture(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);
    }

    if(GamePaused == PauseCode::TextEntry)
        TextEntryScreen::Render();

    XRender::offsetViewportIgnore(false);

    if(!skipRepaint)
        XRender::repaint();

    XRender::setTargetScreen();

//    If TakeScreen = True Then ScreenShot
    if(TakeScreen)
        ScreenShot();

    // Update Coin Frames
    CoinFrame2[1] += 1;
    if(CoinFrame2[1] >= 6)
    {
        CoinFrame2[1] = 0;
        CoinFrame[1] += 1;
        if(CoinFrame[1] >= 4)
            CoinFrame[1] = 0;
    }
    CoinFrame2[2] += 1;
    if(CoinFrame2[2] >= 6)
    {
        CoinFrame2[2] = 0;
        CoinFrame[2] += 1;
        if(CoinFrame[2] >= 7)
            CoinFrame[2] = 0;
    }
    CoinFrame2[3] += 1;
    if(CoinFrame2[3] >= 7)
    {
        CoinFrame2[3] = 0;
        CoinFrame[3] += 1;
        if(CoinFrame[3] >= 4)
            CoinFrame[3] = 0;
    }
//    if(nPlay.Mode == 0)
//    {
//        if(nPlay.NPCWaitCount >= 11)
//            nPlay.NPCWaitCount = 0;
//        nPlay.NPCWaitCount += 2;
//        if(timeStr != "")
//            Netplay::sendData timeStr + LB;
//    }

    lunaRenderEnd();
    frameRenderEnd();

//    if(XRender::lazyLoadedBytes() > 200000) // Reset timer while loading many pictures at the same time
//        resetFrameTimer();
    XRender::lazyLoadedBytesReset();
}
