/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../player.h"
#include "../gfx.h"
#include "../layers.h"
#include "../main/menu_main.h"
#include "../main/speedrunner.h"
#include "../main/trees.h"
#include "../main/screen_pause.h"
#include "../main/screen_connect.h"
#include "../main/screen_quickreconnect.h"
#include "../main/screen_textentry.h"
#include "../main/cheat_code.h"
#include "../compat.h"
#include "../config.h"
#include "../game_main.h"
#include "../main/game_globals.h"
#include "main/level_medals.h"
#include "../core/render.h"
#include "../script/luna/luna.h"

#include "npc/npc_activation.h"
#include "npc/npc_queues.h"
#include "npc/section_overlap.h"

#include "effect.h"
#include "npc_id.h"
#include "eff_id.h"
#include "npc_traits.h"
#include "draw_planes.h"

#include "graphics/gfx_special_frames.h"
#include "graphics/gfx_camera.h"
#include "graphics/gfx_keyhole.h"

#ifdef THEXTECH_BUILD_GL_MODERN
#    include "core/opengl/gl_program_bank.h"
#endif

#include <fmt_format_ne.h>
#include <Utils/maths.h>

struct ScreenShake_t
{
    double forceX = 0;
    double forceY = 0;
    int    offsetX = 0;
    int    offsetY = 0;
    double forceDecay = 1.0;
    int    type = SHAKE_RANDOM;
    double duration = 0;
    double sign = +1.0;

    bool   active = false;

    void update()
    {
        if(!active || GameMenu)
            return;

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
        }
    }

    void apply()
    {
        if(!active || GameMenu)
        {
            XRender::offsetViewport(0, 0);
            return;
        }

        XRender::offsetViewport(offsetX, offsetY);
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
static bool s_forcedShakeScreen = false;

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
            if(Chat_n != sizeof(Chat) / sizeof(uint16_t))
            {
                Chat[Chat_n] = A;
                Chat_n += 1;
                g_stats.renderedNPCs += 1;
            }
        }

        if(NPC[A].Effect == NPCEFF_DROP_ITEM)
        {
            if(std::fmod(NPC[A].Effect2, 3) == 0.0)
                return;
            if(Dropped_n == sizeof(Dropped) / sizeof(uint16_t))
                return;
            Dropped[Dropped_n] = A;
            Dropped_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(
                (
                  (
                    (NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
                    (NPC[A].Type == NPCID_TOOTHY && NPC[A].standingOnPlayer == 0) ||
                    (NPC[A].Type == NPCID_BULLET && NPC[A].CantHurt > 0)
                  ) || NPC[A].Effect == NPCEFF_PET_TONGUE
                ) && NPC[A].Type != NPCID_ITEM_BURIED && !Player[NPC[A].HoldingPlayer].Dead
            )
        {
            if(Held_n == sizeof(Held) / sizeof(uint16_t))
                return;
            Held[Held_n] = A;
            Held_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A]->Foreground && NPC[A].HoldingPlayer == 0 && !NPC[A]->IsACoin)
        {
            if(FG_n == sizeof(FG) / sizeof(uint16_t))
                return;
            FG[FG_n] = A;
            FG_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Type == NPCID_ICE_CUBE && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            if(Iced_n == sizeof(Iced) / sizeof(uint16_t))
                return;
            Iced[Iced_n] = A;
            Iced_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0 &&
            (NPC[A].standingOnPlayer > 0 || NPC[A].Type == NPCID_VEHICLE || NPC[A].Type == NPCID_CANNONITEM ||
                NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_ITEM_BURIED || NPC[A].Type == NPCID_ROCKET_WOOD ||
                NPC[A].Type == NPCID_FIRE_BOSS_FIRE || NPC[A]->IsACoin))
        {
            if(Low_n == sizeof(Low) / sizeof(uint16_t))
                return;
            Low[Low_n] = A;
            Low_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Type == NPCID_SAW || NPC[A].Type == NPCID_JUMP_PLANT ||
            ((NPC[A].Effect == NPCEFF_ENCASED || NPC[A]->IsAVine ||
                    NPC[A].Type == NPCID_BOSS_FRAGILE || NPC[A].Type == NPCID_LIFT_SAND || NPC[A].Type == NPCID_FIRE_PLANT ||
                    NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_BIG_PLANT ||
                    NPC[A].Type == NPCID_LONG_PLANT_UP || NPC[A].Type == NPCID_LONG_PLANT_DOWN || NPC[A].Type == NPCID_BOTTOM_PLANT ||
                    NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Effect == NPCEFF_EMERGE_UP || NPC[A].Effect == NPCEFF_EMERGE_DOWN ||
                    NPC[A].Effect == NPCEFF_WARP || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0.0))
                && NPC[A].standingOnPlayer == 0))
        {
            if(BG_n == sizeof(BG) / sizeof(uint16_t))
                return;
            BG[BG_n] = A;
            BG_n += 1;
            g_stats.renderedNPCs += 1;
        }
        else if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            if(Normal_n == sizeof(Normal) / sizeof(uint16_t))
                return;
            Normal[Normal_n] = A;
            Normal_n += 1;
            g_stats.renderedNPCs += 1;
        }
    }

    void add_warning(uint16_t A)
    {
        if(Warning_n != sizeof(Warning) / sizeof(uint16_t))
        {
            Warning[Warning_n] = A;
            Warning_n += 1;
            g_stats.renderedNPCs += 1;
        }
    }

    // sort each queue properly
    void sort()
    {
        std::sort(&BG[0], &BG[BG_n]);
        std::sort(&Low[0], &Low[Low_n]);
        std::sort(&Iced[0], &Iced[Iced_n]);
        std::sort(&Normal[0], &Normal[Normal_n]);
        std::sort(&Chat[0], &Chat[Chat_n]);
        std::sort(&Held[0], &Held[Held_n]);
        std::sort(&FG[0], &FG[FG_n]);
        std::sort(&Dropped[0], &Dropped[Dropped_n]);
        std::sort(&Warning[0], &Warning[Warning_n]);
    }
};

NPC_Draw_Queue_t NPC_Draw_Queue[maxLocalPlayers] = {NPC_Draw_Queue_t(), NPC_Draw_Queue_t()};

constexpr int NPC_intro_length = 8;
constexpr float NPC_shade_opacity = 0.4f;
constexpr size_t NPC_intro_count_MAX = 32;

uint8_t NPC_intro_count = 0;
int16_t NPC_intro[NPC_intro_count_MAX];
// positive values represent just-activated onscreen NPCs; negative values represent conditionally active NPCs
int8_t NPC_intro_frame[NPC_intro_count_MAX];

static inline void s_RemoveIntroNPC(int i)
{
    NPC_intro_count--;
    NPC_intro[i] = NPC_intro[NPC_intro_count];
    NPC_intro_frame[i] = NPC_intro_frame[NPC_intro_count];
}

static inline void ProcessIntroNPCFrames()
{
    for(uint8_t i = 0; i < NPC_intro_count; i++)
    {
        NPC_intro_frame[i]++;

        // two termination conditions, remove the NPC from the intro list either way
        if(NPC_intro_frame[i] == NPC_intro_length || NPC_intro_frame[i] == 0)
        {
            s_RemoveIntroNPC(i);

            // don't advance the iteration counter, since a new NPC is here now
            i--;
        }
    }
}

// tints NPCS
static inline void s_get_NPC_tint(int A, XTColor& cn)
{
    const NPC_t& n = NPC[A];

    if(!LevelEditor)
    {
        if(!n.Active)
        {
            if(!NPC_InactiveRender(n))
            {
                cn = XTColorF(0.0f, 0.0f, 0.0f, 0.4f);
                return;
            }
        }
        else for(uint8_t i = 0; i < NPC_intro_count; i++)
        {
            if(NPC_intro[i] == A)
            {
                if(NPC_intro_frame[i] >= 0)
                {
                    float coord = NPC_intro_frame[i] / (float)NPC_intro_length;
                    cn = XTColorF(coord, coord, coord);
                    cn.a = XTColor::from_float(NPC_shade_opacity + coord * (1.0f - NPC_shade_opacity));
                    return;
                }
                break;
            }
        }
    }

    cn = n.Shadow ? XTColor(0, 0, 0) : XTColor();
}

// draws a warning icon for offscreen active NPC A on vScreen Z
void DrawWarningNPC(int Z, int A)
{
    XTColor cn;
    s_get_NPC_tint(A, cn);

    double scr_x, scr_y, w, h, frame_h;
    double frame_x = 0, frame_y = 0;

    // some special cases: plants that come from below
    if(NPC[A].Type == 8 || NPC[A].Type == 74 || NPC[A].Type == 93 || NPC[A].Type == 245 || NPC[A].Type == 256 || NPC[A].Type == 270)
    {
        scr_x = vScreen[Z].X + NPC[A].Location.X + NPC[A]->FrameOffsetX;
        scr_y = vScreen[Z].Y + NPC[A].Location.Y + NPC[A]->FrameOffsetY;
        w = NPC[A].Location.Width;
        h = NPC[A].Location.Height;
        frame_h = NPC[A]->THeight;
    }
    // plants from above
    else if(NPC[A].Type == 51 || NPC[A].Type == 257)
    {
        scr_x = vScreen[Z].X + NPC[A].Location.X + NPC[A]->FrameOffsetX;
        scr_y = vScreen[Z].Y + NPC[A].Location.Y + NPC[A]->FrameOffsetY,
        w = NPC[A].Location.Width;
        h = NPC[A].Location.Height;
        frame_h = NPC[A]->THeight;
        frame_y = NPC[A]->THeight - NPC[A].Location.Height;
    }
    // plants from side
    else if(NPC[A].Type == 52)
    {
        if(NPC[A].Direction == -1)
        {
            scr_x = vScreen[Z].X + NPC[A].Location.X + NPC[A]->FrameOffsetX;
            scr_y = vScreen[Z].Y + NPC[A].Location.Y + NPC[A]->FrameOffsetY;
            w = NPC[A].Location.Width;
            frame_h = h = NPC[A].Location.Height;
        }
        else
        {
            scr_x = vScreen[Z].X + NPC[A].Location.X + NPC[A]->FrameOffsetX;
            scr_y = vScreen[Z].Y + NPC[A].Location.Y + NPC[A]->FrameOffsetY;
            w = NPC[A].Location.Width;
            frame_x = NPC[A]->TWidth - NPC[A].Location.Width;
            frame_h = h = NPC[A].Location.Height;
        }
    }
    else if(NPC[A]->WidthGFX == 0)
    {
        scr_x = vScreen[Z].X + NPC[A].Location.X + NPC[A]->FrameOffsetX;
        scr_y = vScreen[Z].Y + NPC[A].Location.Y + NPC[A]->FrameOffsetY;
        w = NPC[A].Location.Width;
        frame_h = h = NPC[A].Location.Height;
    }
    else
    {
        scr_x = vScreen[Z].X + NPC[A].Location.X + (NPC[A]->FrameOffsetX * -NPC[A].Direction) - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0;
        scr_y = vScreen[Z].Y + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height;
        w = NPC[A]->WidthGFX;
        frame_h = h = NPC[A]->HeightGFX;
    }

    double left_x = -scr_x;
    double right_x = scr_x + w - vScreen[Z].Width;

    double add_x = (left_x > 0 ? left_x : (right_x > 0 ? -right_x : 0));

    double top_y = -scr_y;
    double bottom_y = scr_y + h - vScreen[Z].Height;

    double add_y = (top_y > 0 ? top_y : (bottom_y > 0 ? -bottom_y : 0));

    int total_off = (add_x > 0 ? add_x : -add_x)
        + (add_y > 0 ? add_y : -add_y);

    if(total_off >= 250)
        return;

    int a_scale = (int(cn.a) * (250 - total_off)) / 500;
    if(a_scale > 255)
        a_scale = 255;

    cn.a = uint8_t(a_scale);

    double scale = 0.25 + (250.0 - total_off) / 500.0;

    scr_x += w * (1 - scale) / 2;
    scr_y += h * (1 - scale);

    double exclam_x = 0.5;
    double exclam_y = 0.5;

    // push it onto the screen
    if(scr_x < 0)
    {
        scr_x = 0;
        exclam_x = 0.25;
    }
    else if(scr_x + w * scale > vScreen[Z].Width)
    {
        scr_x = vScreen[Z].Width - w * scale;
        exclam_x = 0.75;
    }

    if(scr_y < 0)
    {
        scr_y = 0;
        exclam_y = 0.25;
    }
    else if(scr_y + h * scale > vScreen[Z].Height)
    {
        scr_y = vScreen[Z].Height - h * scale;
        exclam_y = 0.75;
    }

    XRender::renderTextureScaleEx(scr_x,
        scr_y,
        w * scale, h * scale,
        GFXNPC[NPC[A].Type],
        frame_x, NPC[A].Frame * frame_h + frame_y,
        w, h,
        0, nullptr, X_FLIP_NONE,
        cn);

    XRender::renderTexture(scr_x + (w * scale - GFX.Chat.w) * exclam_x,
        scr_y + (h * scale - GFX.Chat.h) * exclam_y,
        GFX.Chat,
        {255, 0, 0, cn.a});
}

// code to facilitate cached values for the onscreen blocks and BGOs
// intentionally only initialize the vectors for the first 2 screens since >2P mode is rare

// query results of last tree query
static std::vector<BlockRef_t> s_drawMainBlocks[maxLocalPlayers] = {std::vector<BlockRef_t>(400), std::vector<BlockRef_t>(400)};
static std::vector<BlockRef_t> s_drawLavaBlocks[maxLocalPlayers] = {std::vector<BlockRef_t>(100), std::vector<BlockRef_t>(100)};
static std::vector<BlockRef_t> s_drawSBlocks[maxLocalPlayers] = {std::vector<BlockRef_t>(40), std::vector<BlockRef_t>(40)};
static std::vector<BaseRef_t> s_drawBGOs[maxLocalPlayers] = {std::vector<BaseRef_t>(400), std::vector<BaseRef_t>(400)};

// query location of last tree query
static Location_t s_drawBlocks_bounds[maxLocalPlayers];
static Location_t s_drawBGOs_bounds[maxLocalPlayers];

// maximum amount of layer movement since last tree query
static double s_drawBlocks_invalidate_timer[maxLocalPlayers] = {0, 0};
static double s_drawBGOs_invalidate_timer[maxLocalPlayers] = {0, 0};

// global: force-invalidate the cache when the blocks themselves change
std::array<bool, maxLocalPlayers> g_drawBlocks_valid{};
std::array<bool, maxLocalPlayers> g_drawBGOs_valid{};

// global: based on layer movement speed, set in layers.cpp
double g_drawBlocks_invalidate_rate = 0;
double g_drawBGOs_invalidate_rate = 0;


// Performance-tweakable code. This is the margin away from the vScreen that is filled when onscreen blocks and BGOs are calculated.
//   Larger values result in more offscreen blocks / BGOs being checked.
//   Smaller values result in more frequent block / BGO table queries.
constexpr double i_drawBlocks_margin = 64;
constexpr double i_drawBGOs_margin = 128;

// updates the lists of blocks and BGOs to draw on i'th vScreen of screen
void s_UpdateDrawItems(Screen_t& screen, int i)
{
    vScreen_t& vscreen = screen.vScreen(i + 1);

    if(i < 0 || i >= maxLocalPlayers)
        return;

    // based on layer movement speed
    s_drawBlocks_invalidate_timer[i] += g_drawBlocks_invalidate_rate;
    s_drawBGOs_invalidate_timer[i] += g_drawBGOs_invalidate_rate;


    // update draw blocks if needed
    if(!g_drawBlocks_valid[i]
        || -vscreen.X                  < s_drawBlocks_bounds[i].X                                 + s_drawBlocks_invalidate_timer[i]
        || -vscreen.X + vscreen.Width  > s_drawBlocks_bounds[i].X + s_drawBlocks_bounds[i].Width  - s_drawBlocks_invalidate_timer[i]
        || -vscreen.Y                  < s_drawBlocks_bounds[i].Y                                 + s_drawBlocks_invalidate_timer[i]
        || -vscreen.Y + vscreen.Height > s_drawBlocks_bounds[i].Y + s_drawBlocks_bounds[i].Height - s_drawBlocks_invalidate_timer[i])
    {
        g_drawBlocks_valid[i] = true;
        s_drawBlocks_invalidate_timer[i] = 0;

        // form query location
        s_drawBlocks_bounds[i] = newLoc(-vscreen.X - i_drawBlocks_margin,
            -vscreen.Y - i_drawBlocks_margin,
            vscreen.Width + i_drawBlocks_margin * 2,
            vscreen.Height + i_drawBlocks_margin * 2);

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
            [](BlockRef_t a, BlockRef_t b)
            {
                return a->Location.Y < b->Location.Y;
            });
    }

    // update draw BGOs if needed
    if(!g_drawBGOs_valid[i]
        || -vscreen.X                  < s_drawBGOs_bounds[i].X                               + s_drawBGOs_invalidate_timer[i]
        || -vscreen.X + vscreen.Width  > s_drawBGOs_bounds[i].X + s_drawBGOs_bounds[i].Width  - s_drawBGOs_invalidate_timer[i]
        || -vscreen.Y                  < s_drawBGOs_bounds[i].Y                               + s_drawBGOs_invalidate_timer[i]
        || -vscreen.Y + vscreen.Height > s_drawBGOs_bounds[i].Y + s_drawBGOs_bounds[i].Height - s_drawBGOs_invalidate_timer[i])
    {
        g_drawBGOs_valid[i] = true;
        s_drawBGOs_invalidate_timer[i] = 0;

        // form query location
        s_drawBGOs_bounds[i] = newLoc(-vscreen.X - i_drawBGOs_margin,
            -vscreen.Y - i_drawBGOs_margin,
            vscreen.Width + i_drawBGOs_margin * 2,
            vscreen.Height + i_drawBGOs_margin * 2);

        // make query (sort by ID as done in vanilla)
        s_drawBGOs[i].clear();
        treeBackgroundQuery(s_drawBGOs[i], s_drawBGOs_bounds[i], (LevelEditor) ? SORTMODE_Z : SORTMODE_ID);
    }
}

void GraphicsLazyPreLoad()
{
    // FIXME: update to work for multiple screens
    // TODO: check if this is needed at caller
    SetupScreens(false);

    // TODO: check whether this is even safe
    DynamicScreens();

    int numScreens = Screens[0].active_end();

    if(SingleCoop == 2)
        numScreens = 1; // fine to be 1, since it would just be run for Z = 2 twice otherwise;

    CenterScreens(Screens[0]);

    For(Z, 1, numScreens)
    {
        if(SingleCoop == 2)
            Z = 2;

        // TODO: need to get vScreen?

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
        // blockTileGet(-vScreen[Z].X, vScreen[Z].Width, fBlock, lBlock);
        s_UpdateDrawItems(Screens[0], Z - 1);

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

// shared between the NPC screen logic functions, always reset to 0 between frames
static std::bitset<maxNPCs> s_NPC_present;

// does the classic ("onscreen") NPC activation / reset logic for vScreen Z, directly based on the many NPC loops of the original game
void ClassicNPCScreenLogic(int Z, int numScreens, bool fill_draw_queue, NPC_Draw_Queue_t& NPC_Draw_Queue_p)
{
    // using bitset here instead of simpler set because I benchmarked it to be faster -- ds-sloth
    std::bitset<maxNPCs>& NPC_present = s_NPC_present;

    // find the onscreen NPCs
    TreeResult_Sentinel<NPCRef_t> _screenNPCs = treeNPCQuery(-vScreen[Z].X, -vScreen[Z].Y,
        -vScreen[Z].X + vScreen[Z].Width, -vScreen[Z].Y + vScreen[Z].Height,
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

        if(((NPC[A].Effect == NPCEFF_ENCASED || NPC[A]->IsAVine ||
             NPC[A].Type == NPCID_BOSS_FRAGILE || NPC[A].Type == NPCID_LIFT_SAND || NPC[A].Type == NPCID_FIRE_PLANT ||
             NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_BIG_PLANT ||
             NPC[A].Type == NPCID_LONG_PLANT_UP || NPC[A].Type == NPCID_LONG_PLANT_DOWN || NPC[A].Type == NPCID_BOTTOM_PLANT ||
             NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Effect == NPCEFF_EMERGE_UP || NPC[A].Effect == NPCEFF_EMERGE_DOWN ||
             NPC[A].Effect == NPCEFF_WARP || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0.0)) &&
             (NPC[A].standingOnPlayer == 0 && (!NPC[A].Generator || LevelEditor))) ||
             NPC[A].Type == NPCID_SAW || NPC[A].Type == NPCID_JUMP_PLANT)
        {
            if(NPC[A].Effect != NPCEFF_DROP_ITEM && (!NPC[A].Generator || LevelEditor))
            {
                can_check = true;
            }
        }

        if(NPC[A].Effect == 0 && ((NPC[A].HoldingPlayer == 0 && (NPC[A].standingOnPlayer > 0 || NPC[A].Type == NPCID_VEHICLE ||
                                   NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_ITEM_BURIED || NPC[A].Type == NPCID_ROCKET_WOOD ||
                                   NPC[A].Type == NPCID_FIRE_BOSS_FIRE || NPC[A]->IsACoin) && (!NPC[A].Generator || LevelEditor))))
        {
            npcALoc = newLoc(NPC[A].Location.X - (NPC[A]->WidthGFX - NPC[A].Location.Width) / 2.0,
                                  NPC[A].Location.Y,
                                  static_cast<double>(NPC[A]->WidthGFX),
                                  static_cast<double>(NPC[A]->THeight));
            has_ALoc = true;
            can_check = true;
        }

        if(NPC[A].Type == NPCID_ICE_CUBE && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
        {
            npcALoc = newLoc(NPC[A].Location.X - (NPC[A]->WidthGFX - NPC[A].Location.Width) / 2.0,
                                  NPC[A].Location.Y,
                                  static_cast<double>(NPC[A]->WidthGFX),
                                  static_cast<double>(NPC[A]->THeight));

            has_ALoc = true;
            can_check = true;
        }


        if(NPC[A].Effect == 0)
        {
            if(!(NPC[A].HoldingPlayer > 0 || NPC[A]->IsAVine || NPC[A].Type == NPCID_BOSS_FRAGILE || NPC[A].Type == NPCID_FIRE_BOSS_FIRE ||
                 NPC[A].Type == NPCID_JUMP_PLANT || NPC[A].Type == NPCID_ROCKET_WOOD || NPC[A].Type == NPCID_LIFT_SAND || NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_FIRE_PLANT ||
                 NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Type == NPCID_BIG_PLANT || NPC[A].Type == NPCID_LONG_PLANT_UP ||
                 NPC[A].Type == NPCID_LONG_PLANT_DOWN || NPC[A].Type == NPCID_VEHICLE || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_ITEM_BURIED) &&
               !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0) && NPC[A].standingOnPlayer == 0 &&
               !NPC[A]->Foreground && (!NPC[A].Generator || LevelEditor) &&
               NPC[A].Type != NPCID_SAW && NPC[A].Type != NPCID_ICE_CUBE)
            {
                if(!NPC[A]->IsACoin)
                {
                    can_check = true;
                    kill_zero = true;
                    activate_conveyer = true;
                    check_both_reset = true;
                    check_long_life = true;
                }
            }
        }


        const Player_t& hp = Player[NPC[A].HoldingPlayer];
        bool hp_door_scroll = (NPC[A].HoldingPlayer > 0 && hp.Effect == 7 && hp.Effect2 >= 128);

        if(
            (
              (
                (NPC[A].HoldingPlayer > 0 && hp.Effect != 3 && !hp_door_scroll) ||
                (NPC[A].Type == NPCID_TOOTHY && NPC[A].standingOnPlayer == 0) ||
                (NPC[A].Type == NPCID_BULLET && NPC[A].CantHurt > 0)
              ) || NPC[A].Effect == NPCEFF_PET_TONGUE
            ) && NPC[A].Type != NPCID_ITEM_BURIED && !Player[NPC[A].HoldingPlayer].Dead
        )
        {
            NPC_Draw_Queue_p.add(A);
        }

        if(NPC[A].Effect == 0)
        {
            if(NPC[A]->Foreground && NPC[A].HoldingPlayer == 0 && (!NPC[A].Generator || LevelEditor))
            {
                if(!NPC[A]->IsACoin)
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

        if(NPC[A].Effect == NPCEFF_DROP_ITEM)
        {
            if(std::fmod(NPC[A].Effect2, 3) != 0.0)
            {
                can_check = true;
                reset_all = false;
                set_justactivated = false;
            }
        }

        if(!can_check)
            continue;

        if((vScreenCollision(Z, NPC[A].Location) || (has_ALoc && vScreenCollision(Z, npcALoc))) && !NPC[A].Hidden)
        {
            if(kill_zero && NPC[A].Type == 0)
            {
                NPC[A].Killed = 9;
                KillNPC(A, 9);
            }
            else if(NPC[A].Active && fill_draw_queue)
            {
                NPC_Draw_Queue_p.add(A);
            }

            if((NPC[A].Reset[Z] && (!check_both_reset || NPC[A].Reset[3 - Z])) || NPC[A].Active || (activate_conveyer && NPC[A].Type == NPCID_CONVEYOR))
            {
                if(set_justactivated && !NPC[A].Active)
                    NPC[A].JustActivated = Z;

                NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                if(check_long_life && (NPCIsYoshi(NPC[A]) || NPCIsBoot(NPC[A]) || NPC[A].Type == NPCID_POWER_S3 || NPC[A].Type == NPCID_FIRE_POWER_S3 || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_LIFE_S3 || NPC[A].Type == NPCID_POISON || NPC[A].Type == NPCID_STATUE_POWER || NPC[A].Type == NPCID_HEAVY_POWER || NPC[A].Type == NPCID_FIRE_POWER_S1 || NPC[A].Type == NPCID_FIRE_POWER_S4 || NPC[A].Type == NPCID_POWER_S1 || NPC[A].Type == NPCID_POWER_S4 || NPC[A].Type == NPCID_LIFE_S1 || NPC[A].Type == NPCID_LIFE_S4 || NPC[A].Type == NPCID_3_LIFE || NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL || NPC[A].Type == NPCID_PLATFORM_S3))
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

    if(fill_draw_queue)
        NPC_Draw_Queue_p.sort();
}

// does the modern NPC activation / reset logic for vScreen Z
void ModernNPCScreenLogic(Screen_t& screen, int vscreen_i, bool fill_draw_queue, NPC_Draw_Queue_t& NPC_Draw_Queue_p)
{
    int Z = screen.vScreen_refs[vscreen_i];

    // canonical vScreens
    int c_Z1 = 0;
    int c_Z2 = 0;

    if(!screen.is_canonical())
    {
        Screen_t& c_screen = screen.canonical_screen();

        // canonical screen might have different split than visible screen
        if(c_screen.Type == 5)
        {
            // visible shared, canonical split
            if(screen.DType == 5 && c_screen.DType != 5)
            {
                c_Z1 = c_screen.vScreen_refs[0];
                c_Z2 = c_screen.vScreen_refs[1];
            }
            // visible split, canonical shared
            else if(screen.DType != 5 && c_screen.DType == 5)
            {
                c_Z1 = c_screen.vScreen_refs[0];
            }
            // same mode, use same vScreen index
            else
            {
                c_Z1 = c_screen.vScreen_refs[vscreen_i];
            }
        }
        // same mode, use same vScreen index
        else
        {
            c_Z1 = c_screen.vScreen_refs[vscreen_i];
        }
    }

    // using bitset here instead of simpler set because I benchmarked it to be faster -- ds-sloth
    // the purpose of this logic is to avoid duplicates in the checkNPCs vector
    std::bitset<maxNPCs>& NPC_present = s_NPC_present;

    // find the onscreen NPCs; first, get query bounds that cover all 3 possible screens
    double bounds_left = -vScreen[Z].X;
    double bounds_top = -vScreen[Z].Y;
    double bounds_right = -vScreen[Z].X + vScreen[Z].Width;
    double bounds_bottom = -vScreen[Z].Y + vScreen[Z].Height;

    if(c_Z1)
    {
        bounds_left = SDL_min(bounds_left, -vScreen[c_Z1].X);
        bounds_top = SDL_min(bounds_top, -vScreen[c_Z1].Y);
        bounds_right = SDL_max(bounds_right, -vScreen[c_Z1].X + vScreen[c_Z1].Width);
        bounds_bottom = SDL_max(bounds_bottom, -vScreen[c_Z1].Y + vScreen[c_Z1].Height);
    }

    if(c_Z2)
    {
        bounds_left = SDL_min(bounds_left, -vScreen[c_Z2].X);
        bounds_top = SDL_min(bounds_top, -vScreen[c_Z2].Y);
        bounds_right = SDL_max(bounds_right, -vScreen[c_Z2].X + vScreen[c_Z2].Width);
        bounds_bottom = SDL_max(bounds_bottom, -vScreen[c_Z2].Y + vScreen[c_Z2].Height);
    }

    TreeResult_Sentinel<NPCRef_t> _screenNPCs = treeNPCQuery(bounds_left, bounds_top,
        bounds_right, bounds_bottom, SORTMODE_NONE);


    // combine the onscreen NPCs with the no-reset NPCs
    std::vector<BaseRef_t>& checkNPCs = *_screenNPCs.i_vec;

    // mark previous ones as checked
    for(int16_t n : checkNPCs)
        NPC_present[n] = true;

    // add the previous frame's no-reset NPCs (without duplicates)
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

    Location_t loc2;

    for(int A : checkNPCs)
    {
        g_stats.checkedNPCs++;

        // there are three related things we will determine:
        // - is the NPC onscreen for rendering?
        // - can we reset the NPC (respawn it to its original location)?
        // - can we activate the NPC?

        // they are normally the same thing -- onscreen -- but we need to separate them to support multiple resolutions.

        bool loc2_exists;
        if(NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0 && !NPC[A].Generator &&
                (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 || NPC[A].Type == 22
                    || NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160
                    || NPC[A].Type == 282 || NPC[A]->IsACoin || NPC[A].Type == 263))
        {
            loc2_exists = true;
            loc2 = newLoc(NPC[A].Location.X - (NPC[A]->WidthGFX - NPC[A].Location.Width) / 2.0,
                NPC[A].Location.Y,
                NPC[A]->WidthGFX,
                NPC[A]->THeight);
            // not sure why loc2 does not consider NPCHeightGFX...
        }
        else
            loc2_exists = false;

        bool render, cannot_reset, can_activate;

        if(NPC[A].Hidden)
        {
            render = cannot_reset = can_activate = false;
        }
        else
        {
            render = vScreenCollision(Z, NPC[A].Location) || (loc2_exists && vScreenCollision(Z, loc2));

            bool onscreen_canonical = false;

            // check canonical screen
            if(c_Z1)
            {
                onscreen_canonical = (vScreenCollision(c_Z1, NPC[A].Location)
                    || (loc2_exists && vScreenCollision(c_Z1, loc2)));
            }
            // fallback to Z itself if no canonical screen exists
            else
            {
                onscreen_canonical = render;
            }

            // add second canonical screen if needed
            if(c_Z2 && !onscreen_canonical)
            {
                onscreen_canonical = (vScreenCollision(c_Z2, NPC[A].Location)
                    || (loc2_exists && vScreenCollision(c_Z2, loc2)));
            }

            cannot_reset = (render || onscreen_canonical);

            // Possible situations where we need to activate as original:
            if(
                   ForcedControls
                || LevelMacro != LEVELMACRO_OFF
                || qScreen_canonical
                || NPC_MustBeCanonical(A)
            )
                can_activate = onscreen_canonical;
            else
                can_activate = render;
        }

        if(NPC[A].Generator)
        {
            NPC[A].GeneratorActive |= can_activate;
            continue;
        }

        if(NPC[A].Type == 0)
            render = false;

        // find the intro frame index for this NPC; will be NPC_intro_count if not found
        uint8_t NPC_intro_index;
        for(NPC_intro_index = 0; NPC_intro_index < NPC_intro_count; NPC_intro_index++)
        {
            if(NPC_intro[NPC_intro_index] == A)
                break;
        }

        // Handle the game's "conditional activation" state where NPC is activated *without* its events being triggered,
        //   with TimeLeft = 0 by the time we get to rendering. In this case, "resetting" means "not activating",
        //   so it's essential that we set cannot_reset to follow can_activate for this frame and the next one
        //   (when Reset[1] and Reset[2] need to become true).

        // Note that this condition, NPC[A].TimeLeft == 0 or 1, is practically never encountered when the NPC is onscreen,
        //   except in the conditional activation code.
        if((NPC[A].TimeLeft == 0 || NPC[A].TimeLeft == 1) && (!NPC[A].Reset[1] || !NPC[A].Reset[2]))
        {
            if(render && !can_activate)
            {
                // add it to the set of NPCs in intro/conditional states if not already present
                if(NPC_intro_index == NPC_intro_count && NPC_intro_count < NPC_intro_count_MAX)
                {
                    NPC_intro[NPC_intro_index] = A;
                    NPC_intro_count++;
                }

                // if it was already in the set, or if it was successfully added,
                // set its intro frame to -2 to give it 2 frames of allowing it to reset while onscreen
                if(NPC_intro_index < NPC_intro_count)
                    NPC_intro_frame[NPC_intro_index] = -2;
            }
        }

        // if the NPC is in the "conditional activation" state but didn't activate, then allow it to reset even when onscreen
        if(NPC_intro_index < NPC_intro_count && NPC_intro_frame[NPC_intro_index] < 0)
            cannot_reset = can_activate;

        // if in "poof" mode, render the smoke effect on the first frame of the intro for an active NPC, then cancel the intro
        if(NPC_intro_index < NPC_intro_count && NPC_intro_frame[NPC_intro_index] > 0 && NPC[A].Active && NPC_InactiveSmoke(NPC[A]))
        {
            Location_t tempLocation = NPC[A].Location;
            tempLocation.X += tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                tempLocation.Y += tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, tempLocation);

                // disable the NPC intro
            s_RemoveIntroNPC(NPC_intro_index);
            NPC_intro_index = NPC_intro_count;
        }

        // this section of the logic handles NPCs that are onscreen but not active yet
        if(!NPC[A].Active && render)
        {
            // Don't show a fish that hasn't jumped yet, a lava bubble that hasn't started coming out yet,
            //   a plant that hasn't emerged yet, etc. Also, don't do poofs for them, since they handle their own intros.
            if(NPC_InactiveIgnore(NPC[A]))
                render = false;
            else if(!NPC_InactiveRender(NPC[A]))
            {
                // add to queue of hidden NPCs
                if(!can_activate)
                {
                    if(NPC_intro_index == NPC_intro_count && NPC_intro_count < NPC_intro_count_MAX)
                    {
                        NPC_intro[NPC_intro_index] = A;
                        NPC_intro_frame[NPC_intro_index] = 0;
                        NPC_intro_count++;
                    }

                    // keep it hidden
                    if(NPC_intro_index < NPC_intro_count && NPC_intro_frame[NPC_intro_index] >= 0)
                        NPC_intro_frame[NPC_intro_index] = 0;
                }

                // if in "poof" mode, cancel render, but still set the intro frames as above so we can spawn the Smoke effect
                if(NPC_InactiveSmoke(NPC[A]))
                    render = false;
            }
        }

        // TODO: experiment with allowing resetting NPCs that are onscreen, cannot activate, and are not rendered
        // if(!can_activate && !render && onscreen)
        //     cannot_reset = false;

        // activate the NPC if allowed
        if(can_activate)
        {
            if(NPC[A].Type == 0) // what is this?
            {
                NPC[A].Killed = 9;
                KillNPC(A, 9);

                NPCQueues::Killed.push_back(A);
            }
            else if(!NPC[A].Active && NPC[A].Effect != 2
                && ((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Type == NPCID_CONVEYOR))
            {
                NPC[A].JustActivated = Z;

                if(!NPC[A].Active)
                {
                    NPCQueues::Active.insert(A);
                    NPC[A].Active = true;
                }
            }
        }

        // track the NPC's reset timer
        if(cannot_reset)
        {
            if(NPC[A].Type != 0
                && ((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active || NPC[A].Type == NPCID_CONVEYOR))
            {
                if(
                       NPCIsYoshi(NPC[A]) || NPCIsBoot(NPC[A]) || NPC[A].Type == NPCID_POWER_S3
                    || NPC[A].Type == NPCID_FIRE_POWER_S3 || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_LIFE_S3
                    || NPC[A].Type == NPCID_POISON || NPC[A].Type == NPCID_STATUE_POWER || NPC[A].Type == NPCID_HEAVY_POWER || NPC[A].Type == NPCID_FIRE_POWER_S1
                    || NPC[A].Type == NPCID_FIRE_POWER_S4 || NPC[A].Type == NPCID_POWER_S1 || NPC[A].Type == NPCID_POWER_S4
                    || NPC[A].Type == NPCID_LIFE_S1 || NPC[A].Type == NPCID_LIFE_S4 || NPC[A].Type == NPCID_3_LIFE || NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL
                    || NPC[A].Type == NPCID_PLATFORM_S3
                )
                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen * 20;
                else
                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
            }

            // mark no-reset if it is currently active, or it has never been offscreen since deactivating
            if((NPC[A].Active || !NPC[A].Reset[2]) && NPC[A].Reset[1])
            {
                NPC[A].Reset[1] = false;
                // only need to mark current-frame state
                // NPC[A].Reset[2] = false;

                NPCQueues::NoReset.push_back(A);
            }
        }
        else
        {
            // Reset was previously cleared here, but now it is cleared in the main UpdateGraphics function
            // NPC[A].Reset[Z] = true;
            // if(numScreens == 1)
            //     NPC[A].Reset[2] = true;
            // if(SingleCoop == 1)
            //     NPC[A].Reset[2] = true;
            // else if(SingleCoop == 2)
            //     NPC[A].Reset[1] = true;
        }

        const Player_t& hp = Player[NPC[A].HoldingPlayer];
        bool hp_door_scroll = (NPC[A].HoldingPlayer > 0 && hp.Effect == 7 && hp.Effect2 >= 128);

        if(hp_door_scroll)
            render = false;

        if(fill_draw_queue && render && ((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active || NPC[A].Type == NPCID_CONVEYOR))
        {
            NPC_Draw_Queue_p.add(A);

            // just for appearance, do the NPC's frames if it hasn't been activated yet and isn't in a shade / hidden mode
            if(!NPC[A].Active && !FreezeNPCs)
            {
                bool in_hidden_mode = NPC_intro_index < NPC_intro_count && NPC_intro_frame[NPC_intro_index] >= 0;
                if(!in_hidden_mode)
                    NPCFrames(A);
            }
        }
        else if(fill_draw_queue && g_config.small_screen_camera_features && NPC[A].Active && cannot_reset && NPC[A].JustActivated == 0 && !NPC[A].Inert && NPC[A].Type != NPCID_CONVEYOR)
        {
            if(NPC[A].Location.SpeedX != 0 || NPC[A].Location.SpeedY != 0
                || (!NPC[A]->WontHurt && !NPC[A]->IsACoin && !NPC[A]->IsABonus))
            {
                NPC_Draw_Queue_p.add_warning(A);
            }
        }
    }

    if(fill_draw_queue)
        NPC_Draw_Queue_p.sort();
}

//! all of the logic done by UpdateGraphics
void UpdateGraphicsLogic(bool Do_FrameSkip);

//! parent function for the actual drawing to screen
void UpdateGraphicsDraw(bool skipRepaint);

//! draw graphics for the current Screen_t
void UpdateGraphicsScreen(Screen_t& screen);

//! extra non-gameplay related draws (menus and information display)
void UpdateGraphicsMeta();

// This draws the graphic to the screen when in a level/game menu/outro/level editor
void UpdateGraphics(bool skipRepaint)
{
//    On Error Resume Next

    if(!GameIsActive)
        return;

    // check that we can render, and that we should not frameskip
    bool Do_FrameSkip = false;

#ifdef USE_RENDER_BLOCKING
    if(XRender::renderBlocked())
        Do_FrameSkip = true;
#endif

    // frame skip code
    cycleNextInc();

    if(FrameSkip && !TakeScreen && frameSkipNeeded())
        Do_FrameSkip = true;

    UpdateGraphicsLogic(Do_FrameSkip);

    // we've now done all the logic that UpdateGraphics can do.
    if(Do_FrameSkip)
        return;

    UpdateGraphicsDraw(skipRepaint);
}

void UpdateGraphicsLogic(bool Do_FrameSkip)
{
    // ALL graphics-based logic code has been moved here, separate from rendering.
    // (This code is a combination of the FrameSkip logic from before with the
    //   logic components of the full rendering code.)
    // NPC render queue formation is also here.

    g_stats.reset();

    SetupScreens(false);
    DynamicScreens();
    CenterScreens();

    bool continue_qScreen = false; // will qScreen continue for any visible screen?
    bool continue_qScreen_local = false; // will qScreen continue for a visible screen on the local machine? (NetPlay)
    bool continue_qScreen_canonical = false; // will qScreen continue for any canonical screen?

    // prepare to fill this frame's NoReset queue
    std::swap(NPCQueues::NoReset, s_NoReset_NPCs_LastFrame);
    NPCQueues::NoReset.clear();

    // mark the last-frame reset state of NPCs that may have Reset[0] set to false, and clear their this-frame reset state
    if(g_compatibility.modern_npc_camera_logic)
    {
        for(NPC_t& n : s_NoReset_NPCs_LastFrame)
        {
            n.Reset[2] = n.Reset[1] && n.Reset[2];
            n.Reset[1] = true;
        }
    }

    // the graphics screen logic is handled via a big loop over Screens (clients)
    for(int screen_i = 0; screen_i < c_screenCount; screen_i++)
    {
        Screen_t& screen = Screens[screen_i];

        if(!screen.Visible)
            continue;

        if(!screen.player_count && !LevelEditor)
            continue;

        // update screen's canonical vScreens
        if(!screen.is_canonical())
        {
            Screen_t& c_screen = screen.canonical_screen();

            for(int i = c_screen.active_begin() + 1; i <= c_screen.active_end(); i++)
                GetvScreenAuto(c_screen.vScreen(i));

            if(qScreen_canonical)
            {
                for(int i = c_screen.active_begin(); i < c_screen.active_end(); i++)
                {
                    int Z_i = c_screen.vScreen_refs[i];
                    continue_qScreen_canonical |= Update_qScreen(Z_i);

                    // the original code was badly written and made THIS happen (always exactly one frame of qScreen in 2P mode)
                    if(i >= 1 && !g_compatibility.modern_section_change)
                        continue_qScreen_canonical = false;
                }
            }
        }

        int numScreens = screen.active_end();

        for(int vscreen_i = screen.active_begin(); vscreen_i < screen.active_end(); vscreen_i++)
        {
            int Z = screen.vScreen_refs[vscreen_i];
            int plr_Z = screen.players[vscreen_i];

            int S;
            if(LevelEditor)
                S = curSection;
            else
                S = Player[plr_Z].Section;

            // update vScreen location
            if(!LevelEditor)
                GetvScreenAuto(vScreen[Z]);

            // moved to `graphics/gfx_screen.cpp`
            // NOTE: this logic was previously only performed on non-frameskips
            if(qScreen)
            {
                bool continue_this_qScreen = Update_qScreen(Z);
                continue_qScreen |= continue_this_qScreen;
                continue_qScreen_local |= (&screen == l_screen) && continue_this_qScreen;
            }

            // the original code was badly written and made THIS happen (always exactly one frame of qScreen in 2P mode)
            if(Z == 2 && !g_compatibility.modern_section_change)
                continue_qScreen = false;

            // noturningback
            if(!LevelEditor && NoTurnBack[Player[plr_Z].Section] && g_compatibility.allow_multires)
            {
                // goal: find canonical vScreen currently on this section that is the furthest left
                // only do anything with the last vScreen (of a Visible screen) in the section
                // (ensures all other logic has happened)

                int A = 0;
                int section = Player[plr_Z].Section;
                bool is_last = true;

                for(int screen_j = 0; screen_j < c_screenCount; screen_j++)
                {
                    Screen_t& o_screen = Screens[screen_j];

                    // active_begin() / active_end() are 0-indexed
                    for(int vscreen_j = o_screen.active_begin(); vscreen_j < o_screen.active_end(); vscreen_j++)
                    {
                        int o_p = o_screen.players[vscreen_j];
                        int o_Z = o_screen.vScreen_refs[vscreen_j];

                        if(Player[o_p].Section == section)
                        {
                            // check if this is the last vScreen (of a Visible screen) in the section
                            if(o_screen.Visible && (screen_j > screen_i || (screen_j == screen_i && vscreen_j > vscreen_i)))
                                is_last = false;

                            // only consider canonical screens
                            if(!o_screen.is_canonical())
                                break;

                            // pick screen further to left
                            if(A == 0 || -vScreen[o_Z].X < -vScreen[A].X)
                                A = o_Z;
                        }
                    }
                }

                if(is_last && A != 0 && -vScreen[A].X > level[S].X)
                {
                    LevelChop[S] += float(-vScreen[A].X - level[S].X);
                    level[S].X = -vScreen[A].X;

                    // mark that section has shrunk
                    UpdateSectionOverlaps(S, true);
                }
            }
            // Legacy noturningback
            else if(!LevelEditor && NoTurnBack[Player[plr_Z].Section])
            {
                vScreen_t& vscreen1 = screen.vScreen(1);
                vScreen_t& vscreen2 = screen.vScreen(2);
                int screen_p1 = screen.players[0];
                int screen_p2 = screen.players[1];

                // goal: find screen currently on this section that is the furthest left
                int A = vscreen_i + 1;
                if(numScreens > 1)
                {
                    if(Player[screen_p1].Section == Player[screen_p2].Section)
                    {
                        if(A == 1)
                            GetvScreen(vscreen2);

                        if(-vscreen1.X < -vscreen2.X)
                            A = 1;
                        else
                            A = 2;
                    }
                }

                if(-screen.vScreen(A).X > level[S].X)
                {
                    LevelChop[S] += float(-screen.vScreen(A).X - level[S].X);
                    level[S].X = -screen.vScreen(A).X;

                    // mark that section has shrunk
                    UpdateSectionOverlaps(S, true);
                }
            }

            // Keep all players onscreen in clone mode
            if(!GameMenu && !LevelEditor)
            {
                if(g_ClonedPlayerMode)
                {
                    int C = 0;
                    int D = 0;

                    For(A, 1, numPlayers)
                    {
                        Player_t &p = Player[A];

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
                    }
                }
            }

            // It's time to process NPCs. We will update their active state and fill a draw queue.

            // only fill draw queue if drawing will happen and this is the local screen
            bool fill_draw_queue = !Do_FrameSkip && (&screen == l_screen);

            // Make sure we are in range.
            SDL_assert_release(vscreen_i >= 0 && vscreen_i < (int)(sizeof(NPC_Draw_Queue) / sizeof(NPC_Draw_Queue_t)));
            NPC_Draw_Queue_t& NPC_Draw_Queue_p = NPC_Draw_Queue[vscreen_i];

            if(fill_draw_queue)
                NPC_Draw_Queue_p.reset();

            // we'll check the NPCs and do some logic for the game,
            if(!LevelEditor)
            {
                if(g_compatibility.modern_npc_camera_logic)
                    ModernNPCScreenLogic(screen, vscreen_i, fill_draw_queue, NPC_Draw_Queue_p);
                else if(Z <= 2)
                    ClassicNPCScreenLogic(Z, numScreens, fill_draw_queue, NPC_Draw_Queue_p);
            }
            // fill the NPC render queue for the level editor
            else if(fill_draw_queue)
            {
                for(int A = 1; A <= numNPCs; A++)
                {
                    g_stats.checkedNPCs++;

                    if(NPC[A].Hidden)
                        continue;

                    const Location_t loc2 = newLoc(NPC[A].Location.X - (NPC[A]->WidthGFX - NPC[A].Location.Width) / 2.0,
                        NPC[A].Location.Y,
                        NPC[A]->WidthGFX, NPC[A]->THeight);

                    if(vScreenCollision(Z, NPC[A].Location) || vScreenCollision(Z, loc2))
                        NPC_Draw_Queue_p.add(A);
                }
            }

            // moved from render code because it affects the game's random state
            // TODO: have a separate shakeScreen state per screen
            s_shakeScreen.update();

        } // loop over vScreens

    } // loop over Screens


    // Background frames (NOTE: frames were only updated on non-frameskip in vanilla)
    if(!FreezeNPCs)
    {
        LevelFramesNotFrozen();
        SpecialFrames();
    }

    LevelFramesAlways();
    ProcessIntroNPCFrames();

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

    // clear the last-frame reset state of NPCs
    if(g_compatibility.modern_npc_camera_logic)
    {
        for(NPC_t& n : s_NoReset_NPCs_LastFrame)
            n.Reset[2] = true;
    }

    // use screen-shake to indicate if invisible screen is currently causing qScreen
    if(g_compatibility.allow_multires)
    {
        // TODO: do loop over visible screens here once s_shakeScreen, s_forcedShakeScreen, and continue_qScreen_local are separated by screen

        // shake screen to tell player game is currently paused (will take effect next frame)
        if(!continue_qScreen_local && (continue_qScreen || continue_qScreen_canonical) && !s_shakeScreen.active)
        {
            s_forcedShakeScreen = true;
            doShakeScreen(1, 1, SHAKE_RANDOM, 0, 0.0);
        }
        // finish forced screenshake
        else if(!(continue_qScreen || continue_qScreen_canonical) && s_forcedShakeScreen)
        {
            s_forcedShakeScreen = false;
            doShakeScreen(1, 1, SHAKE_RANDOM, 0, 0.1);
        }
    }

    // update score and lives to their displayable limits
    if(Score > 9999990)
        Score = 9999990;

    if(Lives > 99)
        Lives = 99;

    if(g_100s > 9999)
        g_100s = 9999;

    if(g_100s < -9999)
        g_100s = -9999;

    // NOTE: qScreen was only updated on non-frameskip in vanilla
    qScreen = continue_qScreen;
    qScreen_canonical = continue_qScreen_canonical;
}


void UpdateGraphicsDraw(bool skipRepaint)
{
    // begin render code
    XRender::setTargetTexture();

    frameNextInc();
    frameRenderStart();
    lunaRenderStart();

    if(ClearBuffer)
    {
        ClearBuffer = false;
        XRender::clearBuffer();
    }

#ifdef __3DS__
    XRender::setTargetLayer(0);
#endif

    XRender::resetViewport();

    XRender::setDrawPlane(PLANE_GAME_BACKDROP);

    UpdateGraphicsScreen(*l_screen);

    UpdateGraphicsMeta();

    if(!skipRepaint)
        XRender::repaint();

    lunaRenderEnd();
    frameRenderEnd();

//    if(XRender::lazyLoadedBytes() > 200000) // Reset timer while loading many pictures at the same time
//        resetFrameTimer();
    XRender::lazyLoadedBytesReset();
}

void UpdateGraphicsScreen(Screen_t& screen)
{
    XTColor plr_shade = ShadowMode ? XTColor(0, 0, 0) : XTColor();
    Location_t tempLocation;

    int numScreens = screen.active_end();

    // even if not clearing buffer, black background is good, to be safe
    XRender::renderRect(0, 0, XRender::TargetW, XRender::TargetH, {0, 0, 0});
    DrawBackdrop(screen);

    // No logic
    // Draw the screens!
    for(int vscreen_i = screen.active_begin(); vscreen_i < screen.active_end(); vscreen_i++)
    {
        int Z = screen.vScreen_refs[vscreen_i];
        int plr_Z = screen.players[vscreen_i];

        int S;
        if(LevelEditor)
            S = curSection;
        else
            S = Player[plr_Z].Section;

        // (Code to get vScreen moved into logic section above.)

#ifdef __3DS__
        if(Z != 1)
            XRender::setTargetLayer(0);
#endif

        XRender::setDrawPlane(PLANE_LVL_BG);

        // Note: this was guarded by an if(!LevelEditor) condition in the past
        if(Background2[S] == 0)
        {
            XRender::renderRect(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop,
                                vScreen[Z].Width, vScreen[Z].Height, {0, 0, 0}, true);
        }

        // Get a reference to our NPC draw queue.
        // Make sure we are in range.
        // If we later add more than two screens,
        // need to change how many NPC draw queues we have.
        SDL_assert_release((Z - 1 >= 0) && ((Z - 1) < (int)(sizeof(NPC_Draw_Queue) / sizeof(NPC_Draw_Queue_t))));
        NPC_Draw_Queue_t& NPC_Draw_Queue_p = NPC_Draw_Queue[Z-1];

        // always needed now due to cases where vScreen is smaller than physical screen
        XRender::setViewport(vScreen[Z].TargetX(), vScreen[Z].TargetY(), vScreen[Z].Width, vScreen[Z].Height);

        // update viewport from screen shake
        s_shakeScreen.apply();

        // camera offsets to add to all object positions before drawing
        double camX = vScreen[Z].CameraAddX();
        double camY = vScreen[Z].CameraAddY();

        DrawBackground(S, Z);

#ifdef THEXTECH_BUILD_GL_MODERN
        if(SectionParticlesBG[S])
            XRender::renderParticleSystem(**SectionParticlesBG[S], camX, camY);
#endif

        // don't show background outside of the current section!
        if(LevelEditor)
        {
            if(camX + level[S].X > 0)
            {
                XRender::renderRect(0, 0,
                                    camX + level[S].X, screen.H, XTColorF(0.2f, 0.2f, 0.2f), true);
            }

            if(screen.W > level[S].Width + camX)
            {
                XRender::renderRect(level[S].Width + camX, 0,
                                    screen.W - (level[S].Width + camX), screen.H, XTColorF(0.2f, 0.2f, 0.2f), true);
            }

            if(camY + level[S].Y > 0)
            {
                XRender::renderRect(0, 0,
                                    screen.W, camY + level[S].Y, XTColorF(0.2f, 0.2f, 0.2f), true);
            }

            if(screen.H > level[S].Height + camY)
            {
                XRender::renderRect(0, level[S].Height + camY,
                                    screen.W, screen.H - (level[S].Height + camY), XTColorF(0.2f, 0.2f, 0.2f), true);
            }
        }


        // moved many-player (superbdemo128) handling code to logic section above

#ifdef __3DS__
        XRender::setTargetLayer(1);
#endif

        // update the vectors of all the onscreen blocks and backgrounds for use at multiple places
        s_UpdateDrawItems(screen, vscreen_i);
        const std::vector<BlockRef_t>& screenMainBlocks = s_drawMainBlocks[vscreen_i];
        const std::vector<BlockRef_t>& screenLavaBlocks = s_drawLavaBlocks[vscreen_i];
        const std::vector<BlockRef_t>& screenSBlocks = s_drawSBlocks[vscreen_i];
        const std::vector<BaseRef_t>& screenBackgrounds = s_drawBGOs[vscreen_i];

        int nextBackground = 0;

        XRender::setDrawPlane(PLANE_LVL_BGO_LOW);

        if(LevelEditor)
        {
            for(; nextBackground < (int)screenBackgrounds.size(); nextBackground++) // First backgrounds
            {
                int A = screenBackgrounds[nextBackground];

                if(A > numBackground)
                    break;

                if(Background[A].SortPriority >= Background_t::PRI_NORM_START)
                    break;

                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(camX + Background[A].Location.X,
                                          camY + Background[A].Location.Y,
                                          GFXBackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type], 0,
                                          BackgroundHeight[Background[A].Type] *
                                          BackgroundFrame[Background[A].Type]);
                }
            }
        }
        else
        {
            // For A = 1 To MidBackground - 1 'First backgrounds
            for(; nextBackground < (int)screenBackgrounds.size() && (int)screenBackgrounds[nextBackground] < MidBackground; nextBackground++)  // First backgrounds
            {
                int A = screenBackgrounds[nextBackground];
                g_stats.checkedBGOs++;

                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(camX + Background[A].Location.X,
                                          camY + Background[A].Location.Y,
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

        XRender::setDrawPlane(PLANE_LVL_SBLOCK);

        for(Block_t& b : screenSBlocks) // Display sizable blocks
        {
            g_stats.checkedSzBlocks++;
            if(/*BlockIsSizable[b.Type] &&*/ (!b.Invis || LevelEditor))
            {
                double bLeftOnscreen = camX + b.Location.X;
                if(bLeftOnscreen > vScreen[Z].Width)
                    continue;
                double bRightOnscreen = bLeftOnscreen + b.Location.Width;
                if(bRightOnscreen < 0)
                    continue;

                double bTopOnscreen = camY + b.Location.Y;
                if(bTopOnscreen > vScreen[Z].Height)
                    continue;
                double bBottomOnscreen = bTopOnscreen + b.Location.Height;
                if(bBottomOnscreen < 0)
                    continue;

                g_stats.renderedSzBlocks++;

                int left_sx = 0;
                if(bLeftOnscreen <= -32)
                {
                    left_sx = 32;
                    bLeftOnscreen = std::fmod(bLeftOnscreen, 32);

                    // go straight to right if less than 33 pixels in total
                    if(bRightOnscreen - bLeftOnscreen < 33)
                        left_sx = 64;
                }

                int top_sy = 0;
                if(bTopOnscreen <= -32)
                {
                    top_sy = 32;
                    bTopOnscreen = std::fmod(bTopOnscreen, 32);

                    // go straight to bottom if less than 33 pixels in total
                    if(bBottomOnscreen - bTopOnscreen < 33)
                        top_sy = 64;
                }

                // logically, offset should be -64. use -65 for double precision safety.
                double colSemiLast = bRightOnscreen - 65;
                double rowSemiLast = bBottomOnscreen - 65;

                if(bRightOnscreen > vScreen[Z].Width)
                    bRightOnscreen = vScreen[Z].Width;

                if(bBottomOnscreen > vScreen[Z].Height)
                    bBottomOnscreen = vScreen[Z].Height;

                // first row source
                int src_y = top_sy;

                for(double dst_y = bTopOnscreen; dst_y < bBottomOnscreen; dst_y += 32.0)
                {
                    // first col source
                    int src_x = left_sx;

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

        XRender::setDrawPlane(PLANE_LVL_BGO_NORM);

        if(LevelEditor)
        {
            for(; nextBackground < (int)screenBackgrounds.size(); nextBackground++)  // Second backgrounds
            {
                int A = screenBackgrounds[nextBackground];

                if(A > numBackground)
                    break;

                if(Background[A].SortPriority >= Background_t::PRI_FG_START)
                    break;

                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(camX + Background[A].Location.X,
                                          camY + Background[A].Location.Y,
                                          GFXBackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type], 0,
                                          BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }
        else if(numBackground > 0)
        {
            for(; nextBackground < (int)screenBackgrounds.size() && (int)screenBackgrounds[nextBackground] <= LastBackground; nextBackground++)  // Second backgrounds
            {
                int A = screenBackgrounds[nextBackground];

                g_stats.checkedBGOs++;

                if(Background[A].Hidden)
                    continue;

                double sX = camX + Background[A].Location.X;
                if(sX > vScreen[Z].Width)
                    continue;

                double sY = camY + Background[A].Location.Y;
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

        for(int oBackground = (int)screenBackgrounds.size() - 1; oBackground > 0 && (int)screenBackgrounds[oBackground] > numBackground; oBackground--)  // Locked doors
        {
            int A = screenBackgrounds[oBackground];

            g_stats.checkedBGOs++;
            if(vScreenCollision(Z, Background[A].Location) &&
                (Background[A].Type == 98 || Background[A].Type == 160) && !Background[A].Hidden)
            {
                g_stats.renderedBGOs++;
                XRender::renderTexture(camX + Background[A].Location.X,
                                      camY + Background[A].Location.Y,
                                      BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type],
                                      GFXBackgroundBMP[Background[A].Type],
                                      0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
            }
        }

#ifdef __3DS__
        XRender::setTargetLayer(2);
#endif

        XRender::setDrawPlane(PLANE_LVL_NPC_BG);

        // 'Display NPCs that should be behind blocks
        for(size_t i = 0; i < NPC_Draw_Queue_p.BG_n; i++)
        {
            int A = NPC_Draw_Queue_p.BG[i];
            XTColor cn;
            s_get_NPC_tint(A, cn);

            if(NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_BIG_PLANT || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_FIRE_PLANT || NPC[A].Type == NPCID_LONG_PLANT_UP || NPC[A].Type == NPCID_JUMP_PLANT)
            {
                XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->THeight, cn);
            }
            else if(NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_LONG_PLANT_DOWN)
            {
                XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX,
                        camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY,
                        NPC[A].Location.Width, NPC[A].Location.Height,
                        GFXNPC[NPC[A].Type], 0,
                        NPC[A].Frame * NPC[A]->THeight + NPC[A]->THeight - NPC[A].Location.Height,
                        cn);
            }
            else if(NPC[A].Type == NPCID_SIDE_PLANT)
            {
                if(NPC[A].Direction == -1)
                {
                    XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->THeight);
                }
                else
                {
                    XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], NPC[A]->TWidth - NPC[A].Location.Width, NPC[A].Frame * NPC[A]->THeight, cn);
                }
            }
            else if(NPC[A]->WidthGFX == 0 || NPC[A].Effect == NPCEFF_EMERGE_UP)
            {
                XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->THeight, cn);
            }
            else
            {
                XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height, NPC[A]->WidthGFX, NPC[A]->HeightGFX, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->HeightGFX, cn);
            }
        }


        XRender::setDrawPlane(PLANE_LVL_PLR_WARP);

        // Player warp effects 'Players behind blocks
        for(int A = 1; A <= numPlayers; A++)
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

                    if((p.Character < 1) || (p.Character > 5))
                        continue;

                    // warp NPCs
                    if(p.HoldingNPC > 0 && p.Frame != 15)
                    {
                        const NPC_t& hNpc = NPC[p.HoldingNPC];

                        auto warpNpcLoc = newLoc(hNpc.Location.X - (hNpc->WidthGFX - hNpc.Location.Width) / 2.0,
                                                 hNpc.Location.Y,
                                                 static_cast<double>(hNpc->WidthGFX),
                                                 static_cast<double>(hNpc->THeight));

                        if((vScreenCollision(Z, hNpc.Location) || vScreenCollision(Z, warpNpcLoc)) && !hNpc.Hidden)
                        {
                            tempLocation = hNpc.Location;
                            if(hNpc->HeightGFX != 0 || hNpc->WidthGFX != 0)
                            {
                                tempLocation.Height = hNpc->HeightGFX;
                                tempLocation.Width = hNpc->WidthGFX;
                                tempLocation.Y = hNpc.Location.Y + hNpc.Location.Height - hNpc->HeightGFX;
                                tempLocation.X = hNpc.Location.X + hNpc.Location.Width / 2.0 - hNpc->WidthGFX / 2.0;
                            }
                            else
                            {
                                tempLocation.Height = hNpc->THeight;
                                tempLocation.Width = hNpc->TWidth;
                            }

                            tempLocation.X += hNpc->FrameOffsetX;
                            tempLocation.Y += hNpc->FrameOffsetY;
                            Y2 = 0;
                            X2 = 0;

                            NPCWarpGFX(A, tempLocation, X2, Y2);

                            if(hNpc->HeightGFX != 0 || hNpc->WidthGFX != 0)
                            {
                                XRender::renderTexture(camX + tempLocation.X,
                                                      camY + tempLocation.Y,
                                                      tempLocation.Width,
                                                      tempLocation.Height,
                                                      GFXNPC[hNpc.Type],
                                                      X2,
                                                      Y2 + hNpc.Frame * hNpc->HeightGFX);
                            }
                            else
                            {
                                XRender::renderTexture(camX + tempLocation.X,
                                                      camY + tempLocation.Y,
                                                      tempLocation.Width,
                                                      tempLocation.Height,
                                                      GFXNPC[hNpc.Type],
                                                      X2,
                                                      Y2 + hNpc.Frame * hNpc->THeight);
                            }
                        }
                    }

                    if(p.Mount == 3)
                    {
                        int B = p.MountType;
                        // Yoshi's Body
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += p.YoshiBX;
                        tempLocation.Y += p.YoshiBY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(camX + tempLocation.X,
                                              camY + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFXYoshiB[B],
                                              X2,
                                              Y2 + 32 * p.YoshiBFrame,
                                              plr_shade);
                        // Yoshi's Head
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += p.YoshiTX;
                        tempLocation.Y += p.YoshiTY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(camX + tempLocation.X,
                                              camY + tempLocation.Y,
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
                        XRender::renderTexture(camX + tempLocation.X,
                                              camY + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              tx,
                                              pfrX(tx, p) + X2,
                                              pfrY(tx, p) + Y2,
                                              plr_shade);

                        // boot GFX
                        tempLocation = roundLoc(p.Location, 2.0);
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X += p.Location.Width / 2.0 - 16;
                        tempLocation.Y += p.Location.Height - 30;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        XRender::renderTexture(camX + tempLocation.X,
                                              camY + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              GFX.Boot[p.MountType],
                                              X2,
                                              Y2 + 32 * p.MountFrame,
                                              plr_shade);
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
                        XRender::renderTexture(camX + tempLocation.X,
                                              camY + tempLocation.Y,
                                              tempLocation.Width,
                                              tempLocation.Height,
                                              tx,
                                              pfrX(tx, p) + X2,
                                              pfrY(tx, p) + Y2,
                                              plr_shade);
                    }
                }

                if(Player[A].HoldingNPC > 0 && Player[A].Frame == 15)
                {
                    auto &hNpc = NPC[Player[A].HoldingNPC];
                    auto hNpcLoc = newLoc(hNpc.Location.X - (hNpc->WidthGFX - hNpc.Location.Width) / 2.0,
                                          hNpc.Location.Y,
                                          static_cast<double>(hNpc->WidthGFX),
                                          static_cast<double>(hNpc->THeight));

                    if((vScreenCollision(Z, hNpc.Location) || vScreenCollision(Z, hNpcLoc)) && !hNpc.Hidden && hNpc.Type != 263)
                    {
                        tempLocation = hNpc.Location;
                        if(hNpc->HeightGFX != 0 || hNpc->WidthGFX != 0)
                        {
                            tempLocation.Height = hNpc->HeightGFX;
                            tempLocation.Width = hNpc->WidthGFX;
                            tempLocation.Y = hNpc.Location.Y + hNpc.Location.Height - hNpc->HeightGFX;
                            tempLocation.X = hNpc.Location.X + hNpc.Location.Width / 2.0 - hNpc->WidthGFX / 2.0;
                        }
                        else
                        {
                            tempLocation.Height = hNpc->THeight;
                            tempLocation.Width = hNpc->TWidth;
                        }

                        tempLocation.X += hNpc->FrameOffsetX;
                        tempLocation.Y += hNpc->FrameOffsetY;
                        Y2 = 0;
                        X2 = 0;

                        NPCWarpGFX(A, tempLocation, X2, Y2);
                        if(hNpc->HeightGFX != 0 || hNpc->WidthGFX != 0)
                        {
                            XRender::renderTexture(camX + tempLocation.X,
                                                  camY + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXNPC[hNpc.Type],
                                                  X2,
                                                  Y2 + hNpc.Frame * hNpc->HeightGFX);
                        }
                        else
                        {
                            XRender::renderTexture(camX + tempLocation.X,
                                                  camY + tempLocation.Y,
                                                  tempLocation.Width,
                                                  tempLocation.Height,
                                                  GFXNPC[hNpc.Type],
                                                  X2,
                                                  Y2 + hNpc.Frame * hNpc->THeight);
                        }
                    }
                }
            }
        }


        XRender::setDrawPlane(PLANE_LVL_BLK_NORM);

        // 'Non-Sizable Blocks
        for(Block_t& block : screenMainBlocks)
        {
            g_stats.checkedBlocks++;

            if(/*!BlockIsSizable[block.Type] &&*/ (!block.Invis || (LevelEditor && (CommonFrame % 46) <= 30)) /*&& block.Type != 0 && !BlockKills[block.Type]*/)
            {
                double sX = camX + block.Location.X;
                if(sX > vScreen[Z].Width)
                    continue;

                double sY = camY + block.Location.Y;
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

        XRender::setDrawPlane(PLANE_LVL_EFF_LOW);

        //'effects in back
        for(int A = 1; A <= numEffects; A++)
        {
            g_stats.checkedEffects++;
            if(Effect[A].Type == EFFID_BOSS_FRAGILE_DIE || Effect[A].Type == EFFID_DOOR_S2_OPEN || Effect[A].Type == EFFID_DOOR_DOUBLE_S3_OPEN ||
               Effect[A].Type == EFFID_DOOR_SIDE_S3_OPEN || Effect[A].Type == EFFID_PLR_FIREBALL_TRAIL || Effect[A].Type == EFFID_COIN_SWITCH_PRESS ||
               Effect[A].Type == EFFID_SPINBLOCK || Effect[A].Type == EFFID_BIG_DOOR_OPEN || Effect[A].Type == EFFID_LAVA_MONSTER_LOOK ||
               Effect[A].Type == EFFID_WATER_SPLASH || Effect[A].Type == EFFID_TIME_SWITCH_PRESS || Effect[A].Type == EFFID_TNT_PRESS)
            {
                g_stats.renderedEffects++;
                if(vScreenCollision(Z, Effect[A].Location))
                {
                    XTColor cn = Effect[A].Shadow ? XTColor(0, 0, 0) : XTColor();
                    XRender::renderTexture(camX + Effect[A].Location.X,
                                           camY + Effect[A].Location.Y,
                                           Effect[A].Location.Width,
                                           Effect[A].Location.Height,
                                           GFXEffect[Effect[A].Type], 0,
                                           Effect[A].Frame * EffectHeight[Effect[A].Type], cn);
                }
            }
        }


        XRender::setDrawPlane(PLANE_LVL_NPC_LOW);

        // draw NPCs that should be behind other NPCs
        for(size_t i = 0; i < NPC_Draw_Queue_p.Low_n; i++)
        {
            int A = NPC_Draw_Queue_p.Low[i];

            XTColor cn;
            s_get_NPC_tint(A, cn);
            if(NPC[A].Type == NPCID_MEDAL && g_curLevelMedals.gotten(NPC[A].Variant - 1))
                cn.a *= 0.5f;

            if(NPC[A]->WidthGFX == 0)
                XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn);
            else
                XRender::renderTexture(camX + NPC[A].Location.X + (NPC[A]->FrameOffsetX * -NPC[A].Direction) - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height, NPC[A]->WidthGFX, NPC[A]->HeightGFX, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->HeightGFX, cn);
        }


        XRender::setDrawPlane(PLANE_LVL_NPC_LOW + 1);

        // ice
        for(size_t i = 0; i < NPC_Draw_Queue_p.Iced_n; i++)
        {
            int A = NPC_Draw_Queue_p.Iced[i];
            DrawFrozenNPC(Z, A);
        }


        XRender::setDrawPlane(PLANE_LVL_NPC_NORM);

        // 'Display NPCs that should be in front of blocks
        for(size_t i = 0; i < NPC_Draw_Queue_p.Normal_n; i++)
        {
            int A = NPC_Draw_Queue_p.Normal[i];

            XTColor cn;
            s_get_NPC_tint(A, cn);

            if(!NPCIsYoshi(NPC[A]))
            {
                if(NPC[A]->WidthGFX == 0)
                {
                    XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn);
                }
                else
                {
                    if(NPC[A].Type == NPCID_ITEM_BUBBLE && NPC[A].Special > 0)
                    {
                        if(NPCWidthGFX(NPC[A].Special) == 0)
                        {
                            tempLocation.Width = NPCWidth(NPC[A].Special);
                            tempLocation.Height = NPCHeight(NPC[A].Special);
                        }
                        else
                        {
                            tempLocation.Width = NPCWidthGFX(NPC[A].Special);
                            tempLocation.Height = NPCHeightGFX(NPC[A].Special);
                        }
                        tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                        tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - tempLocation.Height / 2.0;

                        int B = EditorNPCFrame((NPCID)SDL_floor(NPC[A].Special), NPC[A].Direction);
                        XRender::renderTexture(camX + tempLocation.X + NPC[A]->FrameOffsetX, camY + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[A].Special], 0, B * tempLocation.Height, cn);
                    }

                    XRender::renderTexture(camX + NPC[A].Location.X + (NPC[A]->FrameOffsetX * -NPC[A].Direction) - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height, NPC[A]->WidthGFX, NPC[A]->HeightGFX, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->HeightGFX, cn);
                }
            }
            else
            {
                int B = 1;

                if(NPC[A].Type == NPCID_PET_GREEN)
                    B = 1;
                else if(NPC[A].Type == NPCID_PET_BLUE)
                    B = 2;
                else if(NPC[A].Type == NPCID_PET_YELLOW)
                    B = 3;
                else if(NPC[A].Type == NPCID_PET_RED)
                    B = 4;
                else if(NPC[A].Type == NPCID_PET_BLACK)
                    B = 5;
                else if(NPC[A].Type == NPCID_PET_PURPLE)
                    B = 6;
                else if(NPC[A].Type == NPCID_PET_PINK)
                    B = 7;
                else if(NPC[A].Type == NPCID_PET_CYAN)
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
                XRender::renderTexture(camX + SDL_floor(NPC[A].Location.X) + YoshiBX, camY + NPC[A].Location.Y + YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * YoshiBFrame, cn);

                // Yoshi's Head
                XRender::renderTexture(camX + SDL_floor(NPC[A].Location.X) + YoshiTX, camY + NPC[A].Location.Y + YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * YoshiTFrame, cn);
            }
        }

        // npc chat bubble
        for(size_t i = 0; i < NPC_Draw_Queue_p.Chat_n; i++)
        {
            int A = NPC_Draw_Queue_p.Chat[i];

            int B = NPC[A]->HeightGFX - NPC[A].Location.Height;
            if(B < 0)
                B = 0;

            XRender::renderTexture(camX + NPC[A].Location.X + NPC[A].Location.Width / 2.0 - GFX.Chat.w / 2, camY + NPC[A].Location.Y - 30 - B, GFX.Chat);
        }


        XRender::setDrawPlane(PLANE_LVL_PLR_NORM);

        For(A, 1, numPlayers) // The clown car
        {
            if(!Player[A].Dead && !Player[A].Immune2 && Player[A].TimeToLive == 0 &&
               !(Player[A].Effect == 3 || Player[A].Effect == 5) && Player[A].Mount == 2)
            {
                const Player_t& p = Player[A];

                using plr_pic_arr = RangeArr<StdPicture, 1, 10>;
                constexpr std::array<plr_pic_arr*, 5> char_tex = {&GFXMario, &GFXLuigi, &GFXPeach, &GFXToad, &GFXLink};

                StdPicture& tx = (*char_tex[p.Character - 1])[p.State];

                int Y = 0;

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
                        camX + SDL_floor(Player[A].Location.X) + pfrOffX(tx, p) - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64,
                        camY + Player[A].Location.Y + pfrOffY(tx, p) + Player[A].MountOffsetY - Y,
                        pfrW(tx, p), // was 99, not 100, but not a big deal
                        Player[A].Location.Height - 20 - Player[A].MountOffsetY,
                        tx,
                        pfrX(tx, p),
                        pfrY(tx, p),
                        plr_shade);
                XRender::renderTexture(
                        camX + SDL_floor(Player[A].Location.X) + Player[A].Location.Width / 2.0 - 64,
                        camY + Player[A].Location.Y + Player[A].Location.Height - 128,
                        128,
                        128,
                        GFX.Mount[Player[A].Mount],
                        0,
                        128 * Player[A].MountFrame,
                        plr_shade);
            }
        }

        if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT && LevelMacroWhich != 0)
            RenderKeyhole(Z);

        // Put held NPCs on top
        for(size_t i = 0; i < NPC_Draw_Queue_p.Held_n; i++)
        {
            int A = NPC_Draw_Queue_p.Held[i];
            XTColor cn = NPC[A].Shadow ? XTColor(0, 0, 0) : XTColor();

            if(NPC[A].Type == NPCID_ICE_CUBE)
            {
                DrawFrozenNPC(Z, A);
            }
            else if(!NPCIsYoshi(NPC[A]) && NPC[A].Type > 0)
            {
                if(NPC[A]->WidthGFX == 0)
                    RenderTexturePlayer(Z, camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn);
                else
                    RenderTexturePlayer(Z, camX + NPC[A].Location.X + (NPC[A]->FrameOffsetX * -NPC[A].Direction) - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height, NPC[A]->WidthGFX, NPC[A]->HeightGFX, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->HeightGFX, cn);
            }
        }



        //'normal player draw code
        //'Players in front of blocks
        for(int A = numPlayers; A >= 1; A--)
        {
            DrawPlayer(Player[A], Z);
        }
        //'normal player end


        // foreground backgrounds
        XRender::setDrawPlane(PLANE_LVL_BGO_FG);

        if(LevelEditor)
        {
            for(; nextBackground < (int)screenBackgrounds.size(); nextBackground++)  // Foreground objects
            {
                int A = screenBackgrounds[nextBackground];

                if(A > numBackground)
                    continue;

                g_stats.checkedBGOs++;
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(camX + Background[A].Location.X,
                                          camY + Background[A].Location.Y,
                                          GFXBackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type], 0,
                                          BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }
        else
        {
            for(; nextBackground < (int)screenBackgrounds.size() && (int)screenBackgrounds[nextBackground] <= numBackground; nextBackground++)  // Foreground objects
            {
                int A = screenBackgrounds[nextBackground];

                g_stats.checkedBGOs++;

                if(Background[A].Hidden)
                    continue;

                double sX = camX + Background[A].Location.X;
                if(sX > vScreen[Z].Width)
                    continue;

                double sY = camY + Background[A].Location.Y;
                if(sY > vScreen[Z].Height)
                    continue;

                if(sX + Background[A].Location.Width >= 0 && sY + Background[A].Location.Height >= 0 /*&& !Background[A].Hidden*/)
                {
                    g_stats.renderedBGOs++;
                    XRender::renderTexture(sX, sY, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }

        XRender::setDrawPlane(PLANE_LVL_NPC_FG);

        // foreground NPCs
        for(size_t i = 0; i < NPC_Draw_Queue_p.FG_n; i++)
        {
            int A = NPC_Draw_Queue_p.FG[i];

            XTColor cn;
            s_get_NPC_tint(A, cn);

            if(NPC[A]->WidthGFX == 0)
                XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn);
            else
                XRender::renderTexture(camX + NPC[A].Location.X + (NPC[A]->FrameOffsetX * -NPC[A].Direction) - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height, NPC[A]->WidthGFX, NPC[A]->HeightGFX, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->HeightGFX, cn);
        }

        XRender::setDrawPlane(PLANE_LVL_BLK_HURTS);

        // Blocks in Front
        for(Block_t& block : screenLavaBlocks)
        {
            g_stats.checkedBlocks++;

            // screenLavaBlocks only contains deadly blocks
            // if(!BlockKills[block.Type]) continue;

            if(vScreenCollision(Z, block.Location) /*&& !block.Hidden*/)
            {
                g_stats.renderedBlocks++;
                // Don't show a visual difference of hit-resized block in a comparison to original state
                bool wasShrinkResized = block.getShrinkResized();
                double offX = wasShrinkResized ? 0.05 : 0.0;
                double offW = wasShrinkResized ? 0.1 : 0.0;
                XRender::renderTexture(camX + block.Location.X - offX,
                                      camY + block.Location.Y + block.ShakeY3,
                                      block.Location.Width + offW,
                                      block.Location.Height,
                                      GFXBlock[block.Type],
                                      0,
                                      BlockFrame[block.Type] * 32);
            }
        }

        XRender::setDrawPlane(PLANE_LVL_EFF_NORM);

        // effects on top
        For(A, 1, numEffects)
        {
            g_stats.checkedEffects++;
            auto &e = Effect[A];

            if(e.Type != EFFID_BOSS_FRAGILE_DIE && e.Type != EFFID_DOOR_S2_OPEN && e.Type != EFFID_DOOR_DOUBLE_S3_OPEN && e.Type != EFFID_DOOR_SIDE_S3_OPEN &&
               e.Type != EFFID_PLR_FIREBALL_TRAIL && e.Type != EFFID_COIN_SWITCH_PRESS && e.Type != EFFID_SPINBLOCK && e.Type != EFFID_BIG_DOOR_OPEN &&
               e.Type != EFFID_LAVA_MONSTER_LOOK && e.Type != EFFID_WATER_SPLASH && e.Type != EFFID_TIME_SWITCH_PRESS && e.Type != EFFID_TNT_PRESS)
            {
                if(vScreenCollision(Z, e.Location))
                {
                    g_stats.renderedEffects++;

                    XTColor cn = e.Shadow ? XTColor(0, 0, 0) : XTColor();
                    XRender::renderTexture(vb6Round(camX + e.Location.X),
                                           vb6Round(camY + e.Location.Y),
                                           vb6Round(e.Location.Width),
                                           vb6Round(e.Location.Height),
                                           GFXEffectBMP[e.Type], 0, e.Frame * EffectHeight[e.Type], cn);
                }
            }
        }

        XRender::setDrawPlane(PLANE_LVL_INFO);

        // NPCs warnings
        for(size_t i = 0; i < NPC_Draw_Queue_p.Warning_n; i++)
        {
            int A = NPC_Draw_Queue_p.Warning[i];
            DrawWarningNPC(Z, A);
        }

        // water
        if(LevelEditor)
        {
            for(int B : treeWaterQuery(-camX, -camY,
                -camX + vScreen[Z].Width, -camY + vScreen[Z].Height,
                SORTMODE_ID))
            {
                if(!Water[B].Hidden && vScreenCollision(Z, Water[B].Location))
                {
                    if(Water[B].Quicksand)
                        XRender::renderRect(camX + Water[B].Location.X, camY + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            {255, 255, 0}, false);
                    else
                        XRender::renderRect(camX + Water[B].Location.X, camY + Water[B].Location.Y, Water[B].Location.Width, Water[B].Location.Height,
                            {0, 255, 255}, false);
                }
            }
        }

        if(!LevelEditor) // Graphics for the main game.
        {
            // moved vScreen divider below

            // Redigit NetPlay player names were also dropped

            lunaRender(Z);

            // debug code to show logical screens
            if(g_CheatLogicScreen && !screen.is_canonical())
            {
                Screen_t& c_screen = screen.canonical_screen();

                for(int c_vscreen_Z = c_screen.active_begin() + 1; c_vscreen_Z <= c_screen.active_end(); c_vscreen_Z++)
                {
                    vScreen_t& c_vscreen = c_screen.vScreen(c_vscreen_Z);

                    XRender::renderRect(camX - c_vscreen.X, camY - c_vscreen.Y, c_vscreen.Width, c_vscreen.Height,
                        {  0,   0,   0}, false);
                    XRender::renderRect(camX - c_vscreen.X + 1, camY - c_vscreen.Y + 1, c_vscreen.Width - 2, c_vscreen.Height - 2,
                        {  0,   0,   0}, false);
                    XRender::renderRect(camX - c_vscreen.X + 2, camY - c_vscreen.Y + 2, c_vscreen.Width - 4, c_vscreen.Height - 4,
                        {255, 255, 255}, false);
                    XRender::renderRect(camX - c_vscreen.X + 3, camY - c_vscreen.Y + 3, c_vscreen.Width - 6, c_vscreen.Height - 6,
                        {255, 255, 255}, false);
                    SuperPrint(std::to_string(c_vscreen_Z), 1, camX - c_vscreen.X + 4, camY - c_vscreen.Y + 4);
                }
            }

            // 'Interface

            // moved condition past the splitFrame() call (always draw section effects)
            // if(!GameMenu && !GameOutro)
            For(A, 1, numPlayers)
            {
                if(Player[A].ShowWarp > 0)
                {
                    const auto &w = Warp[Player[A].ShowWarp];

                    int p_center_x = camX + Player[A].Location.X + (Player[A].Location.Width / 2);
                    int info_y = Player[A].Location.Y + Player[A].Location.Height - 96 + camY;

                    if(!w.noPrintStars && w.save_info().inited() && w.save_info().max_stars > 0 && Player[A].Mount != 2)
                    {
                        std::string tempString = fmt::format_ne("{0}/{1}", w.curStars, w.save_info().max_stars);
                        XRender::renderTexture(p_center_x - tempString.length() * 9 - 20, info_y,     GFX.Interface[5]);
                        XRender::renderTexture(p_center_x - tempString.length() * 9,      info_y + 1, GFX.Interface[1]);
                        SuperPrint(tempString, 3,
                                   p_center_x - tempString.length() * 9 + 18,
                                   info_y);

                        info_y -= 20;
                    }

                    if(w.save_info().inited() && w.save_info().max_medals > 0)
                    {
                        uint8_t ckpt = (InHub() && Checkpoint == FileNamePath + GetS(w.level)) ? g_curLevelMedals.got : 0;

                        DrawMedals(p_center_x, info_y, true, w.save_info().max_medals, 0, ckpt, w.save_info().medals_got, w.save_info().medals_best);
                    }
                }
            }
        }

        XRender::setDrawPlane(PLANE_LVL_SECTION_FG);

#ifdef THEXTECH_BUILD_GL_MODERN
        if(SectionParticlesFG[S])
            XRender::renderParticleSystem(**SectionParticlesFG[S], camX, camY);

        XRender::setupLighting(SectionLighting[S]);

        if(SectionEffect[S])
            XRender::renderTextureScale(0, 0, vScreen[Z].Width, vScreen[Z].Height, **SectionEffect[S]);
        else if(SectionLighting[S])
            XRender::renderLighting();
#endif

        XRender::splitFrame();

#ifdef __3DS__
        XRender::setTargetLayer(3);
#endif

        // HUD and dropped NPCs
        if(!GameMenu && !GameOutro && !LevelEditor)
        {
            XRender::setDrawPlane(PLANE_LVL_HUD);

            // draw HUD only if player has not disabled it
            if(ShowOnScreenHUD)
            {
#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
                lunaRenderHud(Z);
#endif

                // this is LunaScript's way of disabling the original SMBX HUD, so it shouldn't affect the Luna HUD
                if(!gSMBXHUDSettings.skip)
                   DrawInterface(Z, numScreens);
            }

            XRender::setDrawPlane(PLANE_LVL_HUD + 1);

            // Display NPCs that got dropped from the container
            for(size_t i = 0; i < NPC_Draw_Queue_p.Dropped_n; i++)
            {
                int A = NPC_Draw_Queue_p.Dropped[i];

                if(NPC[A]->WidthGFX == 0)
                    XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY, NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height);
                else
                    XRender::renderTexture(camX + NPC[A].Location.X + NPC[A]->FrameOffsetX - NPC[A]->WidthGFX / 2.0 + NPC[A].Location.Width / 2.0, camY + NPC[A].Location.Y + NPC[A]->FrameOffsetY - NPC[A]->HeightGFX + NPC[A].Location.Height, NPC[A]->WidthGFX, NPC[A]->HeightGFX, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A]->HeightGFX);
            }
        }

        XRender::setDrawPlane(PLANE_LVL_META);

        // Always draw for single-player
        // And don't draw when many players at the same screen
        if(screen.player_count == 1 || numScreens != 1)
            g_levelVScreenFader[Z].draw(false);

        if((LevelEditor || MagicHand))
        {
            // editor code now located in `gfx_editor.cpp`
            XRender::setDrawPlane(PLANE_LVL_INFO);
            DrawEditorLevel(Z);
        }

        XRender::setDrawPlane(PLANE_LVL_META);

        // Screen shake logic was here; moved into the logic section of the file because it affects the random state of the game

        // draw onscreen controls display
        XRender::offsetViewportIgnore(true);

        if(screen.Type == 5 && numScreens == 1)
        {
            speedRun_renderControls(screen.players[0], Z, SPEEDRUN_ALIGN_LEFT);
            speedRun_renderControls(screen.players[1], Z, SPEEDRUN_ALIGN_RIGHT);
        }
        else if(numScreens >= 2)
        {
            speedRun_renderControls(plr_Z, Z, SPEEDRUN_ALIGN_AUTO);
        }

        // indicate any small-screen camera features
        if(g_config.small_screen_camera_features && screen.H < 600
            && screen.Type != 2 && screen.Type != 3 && screen.Type != 7 && (screen.Type != 5 || screen.vScreen(2).Visible))
        {
            DrawSmallScreenCam(vScreen[Z]);
        }

        XRender::offsetViewportIgnore(false);
    } // For(Z, 2, numScreens)


    // graphics shared by all vScreens, but still on the Screen
    XRender::setViewport(screen.TargetX(), screen.TargetY(), screen.W, screen.H);
    XRender::offsetViewportIgnore(true);

    // splitscreen dividers
    if(numScreens > 1 && !SingleCoop)
    {
        XRender::setDrawPlane(PLANE_LVL_META);

        bool horiz_split = (screen.Type == ScreenTypes::TopBottom) || (screen.Type == ScreenTypes::Quad);
        horiz_split |= screen.Type == ScreenTypes::Dynamic && (screen.DType == 3 || screen.DType == 4 || screen.DType == 6);

        if(horiz_split)
            XRender::renderRect(0, (screen.H / 2) - 2, screen.W, 4, {0, 0, 0});

        bool vert_split = (screen.Type == ScreenTypes::LeftRight) || (screen.Type == ScreenTypes::Quad);
        vert_split |= screen.Type == ScreenTypes::Dynamic && (screen.DType == 1 || screen.DType == 2);

        if(vert_split)
            XRender::renderRect((screen.W / 2) - 2, 0, 4, screen.H, {0, 0, 0});
    }

    XRender::resetViewport();

    XRender::setDrawPlane(PLANE_GAME_META);

    // 1P controls indicator
    if(screen.Type != 5 && numScreens == 1)
    {
        for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
            speedRun_renderControls(screen.players[plr_i], -1, SPEEDRUN_ALIGN_AUTO);
    }

    // fix missing controls info when the vScreen didn't get rendered at all
    if(screen.Type == 5 && numScreens == 1 && screen.vScreen(1).Width == 0)
    {
        speedRun_renderControls(screen.players[0], -1);
        speedRun_renderControls(screen.players[1], -1);
    }
}

void UpdateGraphicsMeta()
{
    XRender::resetViewport();

    XRender::setDrawPlane(PLANE_GAME_MENUS);

    if(GameMenu && !GameOutro)
        mainMenuDraw();

    if(GameOutro)
        DrawCredits();

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

    if(GamePaused == PauseCode::DropAdd)
    {
        ConnectScreen::Render();
        XRender::renderTexture(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);
    }

    if(GamePaused == PauseCode::TextEntry)
        TextEntryScreen::Render();

    XRender::setDrawPlane(PLANE_GAME_META);

    speedRun_renderTimer();

    if(PrintFPS > 0 && ShowFPS)
        SuperPrint(fmt::format_ne("{0}", int(PrintFPS)), 1, XRender::TargetOverscanX + 8, 8, {0, 255, 0});

    g_stats.print();

    if(!BattleMode && !GameMenu && !GameOutro && g_config.show_episode_title)
    {
        // big screen, display at top
        if(XRender::TargetH >= 640 && g_config.show_episode_title == Config_t::EPISODE_TITLE_TOP)
        {
            int y = 20;
            float alpha = 1.0f;
            SuperPrintScreenCenter(WorldName, 3, y, XTAlphaF(alpha));
        }
        // display at bottom
        else if(g_config.show_episode_title == Config_t::EPISODE_TITLE_BOTTOM)
        {
            int y = XRender::TargetH - 60;
            float alpha = 0.75f;
            SuperPrintScreenCenter(WorldName, 3, y, XTAlphaF(alpha));
        }
    }

    DrawDeviceBattery();

    // Draw screen fader below level menu when game is paused
    // This makes sure that the level test menu is drawn above the screen fader during level tests
    if(GamePaused != PauseCode::None)
        XRender::setDrawPlane(PLANE_GAME_MENUS - 1);

    g_levelScreenFader.draw();

    XRender::offsetViewportIgnore(false);
}
