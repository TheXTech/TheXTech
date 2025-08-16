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

#include <array>

#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "core/render.h"
#include "gfx.h"

#include "npc_id.h"
#include "npc_traits.h"

#include "graphics/gfx_keyhole.h"

static inline int s_round2int(num_t d)
{
    return num_t::floor(d + 0.5_n);
}

static inline int s_round2int_plr(num_t d)
{
#ifdef PGE_MIN_PORT
    return (int)(num_t::floor(d / 2 + 0.5_n)) * 2;
#else
    return num_t::floor(d + 0.5_n);
#endif
}

//! Get left pixel at the player sprite
int pfrX(const StdPicture& tx, const Player_t& p)
{
    // FIXME: Replace this heuristic logic with a proper texture flags mechanism

    // will use internal flags (tx.flags & PLAYER_MODERN and tx.flags & PLAYER_CUSTOM) in future
    // if tx.flags & PLAYER_CUSTOM, then will use heap-allocated polymorphic CustomData_t* tx.custom_data,
    // which will include all frame bounding boxes and offsets
    if(tx.h != 512)
        return ((p.Frame * p.Direction + 49) / 10) * 100;
    else
    {
        int col_w = (p.Character == 5) ? 64 : 48;
        int n_rows = 4;
        int n_cols = (p.Character == 5) ? 4 : 10;

        int fr = p.Frame;
        int dir = p.Direction;

        if(fr < 0)
        {
            fr = -fr;
            dir = -dir;
        }

        fr = (fr <= 32) ? fr : fr - 7;

        bool is_center_frame = (fr == 0);

        if(p.Character == 5)
            fr -= 1;

        int col = fr / n_rows;

        // load left-facing frame (bottom) - right to left (in order to mirror top half)
        if(!is_center_frame && dir < 0)
            return col_w * (n_cols - col - 1);
        // load right-facing frame (top) - left to right
        else
            return col_w * col;
    }
}

//! Get top pixel at the player sprite
int pfrY(const StdPicture& tx, const Player_t& p)
{
    // FIXME: Replace this heuristic logic with a proper texture flags mechanism
    if(tx.h != 512)
        return ((p.Frame * p.Direction + 49) % 10) * 100;
    else
    {
        int row_h = 64;
        int n_rows = 4;

        int fr = p.Frame;
        int dir = p.Direction;

        if(fr < 0)
        {
            fr = -fr;
            dir = -dir;
        }

        fr = (fr <= 32) ? fr : fr - 7;

        bool is_center_frame = (fr == 0);

        if(p.Character == 5)
            fr -= 1;

        int row = fr % n_rows;

        // load left-facing frame (bottom)
        if(!is_center_frame && dir < 0)
            return row_h * (n_rows + row);
        // load right-facing frame (top)
        else
            return row_h * row;
    }
}

//! Get width at the player sprite
int pfrW(const StdPicture& tx, const Player_t& p)
{
    if(tx.h != 512)
        return 100;
    else if(p.Character == 5)
        return 64;
    else
        return 48;
}

//! Get height at the player sprite
int pfrH(const StdPicture& tx, const Player_t& p)
{
    UNUSED(p);

    if(tx.h != 512)
        return 100;
    else
        return 64;
}

//! Get x offset that should be ADDED to the player position to draw the sprite
int pfrOffX(const StdPicture& tx, const Player_t& p)
{
    UNUSED(tx);

    if((p.Character < 1) || (p.Character > 5))
        return 0;

    using plr_frame_off_arr = RangeArrI<int8_t, 0, maxPlayerFrames, 0>;
    constexpr std::array<plr_frame_off_arr*, 5> char_offsetX = {&MarioFrameX, &LuigiFrameX, &PeachFrameX, &ToadFrameX, &LinkFrameX};
    int offX = (*char_offsetX[p.Character - 1])[(p.State * 100) + (p.Frame * p.Direction)];

    return offX;
}

//! Get y offset that should be ADDED to the player position to draw the sprite
int pfrOffY(const StdPicture& tx, const Player_t& p)
{
    UNUSED(tx);

    if((p.Character < 1) || (p.Character > 5))
        return 0;

    using plr_frame_off_arr = RangeArrI<int8_t, 0, maxPlayerFrames, 0>;
    constexpr std::array<plr_frame_off_arr*, 5> char_offsetY = {&MarioFrameY, &LuigiFrameY, &PeachFrameY, &ToadFrameY, &LinkFrameY};
    int offY = (*char_offsetY[p.Character - 1])[(p.State * 100) + (p.Frame * p.Direction)];

    return offY;
}

void DrawCycloneAccessory(int Z, const Player_t& p, int cX, int tY, XTColor c)
{
    int acc_frame = (CommonFrame / 4) % 4;
    if(acc_frame == 3)
        acc_frame = 1;

    // calculate offset for accessory
    int offsetX = -16;
    int offsetY = -32;

    int extra_offX = 0;
    int extra_offY = 0;

    if(p.Character == 5)
    {
        offsetX += 1;

        if(p.Frame == 1 || p.Frame == 2 || p.Frame == 3)
        {
            extra_offX = -4;
            extra_offY = -6;
        }
        else if(p.Frame == 4)
        {
            extra_offX = -2;
            extra_offY = -4;
        }
        else if(p.Frame == 5)
        {
            extra_offY = -6;
        }
        else if(p.Frame == 6)
        {
            extra_offX = -12;
            extra_offY = -4;
        }
        else if(p.Frame == 7)
        {
            extra_offX = 10;
            extra_offY = -2;
        }
        else if(p.Frame == 8)
        {
            extra_offX = 12;
            extra_offY = -4;
        }
        else if(p.Frame == 9)
        {
            extra_offX = 0;
            extra_offY = 0;
        }
        else if(p.Frame == 10)
        {
            extra_offX = -8;
            extra_offY = -4;
        }
        else if(p.Frame == 11)
        {
            extra_offX = 2;
            extra_offY = 0;
        }
    }
    else if(p.Frame == 30)
    {
        extra_offX = (p.Character == 2) ? -10 : -8;
        int offY = (p.Character == 2) ? -4 : 0;

        extra_offY = p.MountOffsetY + offY;
    }
    else if(p.Frame == 31)
    {
        extra_offX = -4;
        extra_offY = (p.Character == 2) ? -28 : -26;
    }
    else if(p.Frame == 7 || p.Frame == 27)
    {
        if(p.Character == 4)
            extra_offY = 2;
        else if(p.Character == 1)
            extra_offY = -4;
        else if(p.Character == 2)
            extra_offY = -6;
        else
            extra_offY = -2;
    }
    else if(p.Frame == 13)
    {
        if(p.Character == 3)
            extra_offY = 4;
    }
    else if(p.Frame == 15)
    {
        if(p.Character == 3)
            extra_offY = -2;
    }
    else if(p.Frame >= 40)
    {
        if(p.Character == 2)
            extra_offX = -2;
    }
    else if(p.Frame == 24)
    {
        extra_offX = -2;
        extra_offY = 2;
    }
    else if(p.Frame == 25 || p.Frame == 26)
    {
        acc_frame = 3 + (CommonFrame / 8) % 4;
        offsetY = -2;

        if(p.Character == 4)
            offsetY = -6;
        else if(p.Character == 3)
        {
            offsetY = -4;
            offsetX += 4;
        }
    }
    else if(p.Frame == 22 || p.Frame == 23)
    {
        if(p.Frame == 23)
            offsetY += 2;

        extra_offX = -2;

        if(p.Character == 4)
            extra_offY = -12;
        else if(p.Character == 3)
        {
            extra_offY = -22;
            extra_offX = 0;

            if(p.Direction == 1)
                offsetX += 8;
        }
        else
            extra_offY = -18;
    }
    else if(p.Frame == 16 || p.Frame == 17 || p.Frame == 18)
    {
        if(p.Character == 3)
            extra_offY = 2;
        else if(p.Character != 4)
            extra_offY = -2;
    }
    else if(p.Frame <= 10)
    {
        if(p.Character == 1 || p.Character == 2)
        {
            if(p.Frame == 4 || p.Frame == 5)
                extra_offY = -2;

            if(p.Frame == 6)
                extra_offX = 2;
            else if(!p.SpinJump)
                extra_offX = -2;
        }
    }

    if(p.Character == 2 && !(p.Mount == 1 && p.Duck))
        offsetY -= 2;

    offsetX += extra_offX * p.Direction;
    offsetY += extra_offY;

    RenderTexturePlayer(Z,
        cX + offsetX,
        tY + offsetY,
        32,
        32,
        GFX.CycloneAcc,
        0,
        32 * acc_frame,
        c);
}

using plr_pic_arr = RangeArr<StdPicture, 1, numStates>;
static constexpr std::array<plr_pic_arr*, 5> s_char_tex = {&GFXMario, &GFXLuigi, &GFXPeach, &GFXToad, &GFXLink};

void DrawPlayerRaw(int X, int Y, int Character, int State, int Frame, int Direction)
{
    Player_t p;
    p.Character = Character;
    p.State = State;
    p.Frame = Frame;
    p.Direction = Direction;

    StdPicture& tx = (*s_char_tex[p.Character - 1])[p.State];

    int offX = pfrOffX(tx, p);
    int offY = pfrOffY(tx, p);

    XRender::renderTextureBasic(X + offX,
                Y + offY,
                pfrW(tx, p),
                pfrH(tx, p),
                tx,
                pfrX(tx, p),
                pfrY(tx, p));
}

void DrawPlayer(const int A, const int Z, XTColor color)
{
    DrawPlayer(Player[A], Z, color);
}

void DrawPlayer(Player_t &p, const int Z, XTColor color)
{
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    int B = 0;
    // double C = 0;
    XTColor s = (ShadowMode ? XTColor(64, 64, 64, color.a) : color);

    // color player during invincibility
    if(InvincibilityTime)
    {
        switch(((unsigned)CommonFrame / 4) % 4)
        {
        case 0:
            s.r = 128;
            break;
        case 1:
            s.g = 128;
            break;
        case 2:
            s.b = 128;
            break;
        case 3:
            break;
        }
    }

    //auto &p = Player[A];

    int sX = camX + s_round2int_plr(p.Location.X);
    int sY = camY + s_round2int_plr(p.Location.Y);
    int w = s_round2int(p.Location.Width);
    int h = s_round2int(p.Location.Height);

    if(!p.Immune2) // other draw conditions moved to calling site in UpdateGraphics
    {
        if(vScreenCollision(Z, p.Location))
        {
            if(p.Mount == 3 && !p.Fairy)
            {
                B = p.MountType;
                // Yoshi's Tongue
                if(p.MountSpecial > 0)
                {
                    int C = 0;
                    if(p.Direction == 1)
                        C = p.YoshiTongueLength;
                    else
                        C = -16;

                    RenderTexturePlayer(Z, camX + s_round2int_plr(p.YoshiTongue.X) - C - 1,
                                          camY + s_round2int_plr(p.YoshiTongue.Y),
                                          p.YoshiTongueLength + 2,
                                          16,
                                          GFX.Tongue[2],
                                          0, 0,
                                          s);

                    C = 1;
                    if(p.Direction == 1)
                        C = 0;

                    RenderTexturePlayer(Z, camX + s_round2int_plr(p.YoshiTongue.X),
                                          camY + s_round2int_plr(p.YoshiTongue.Y),
                                          16, 16,
                                          GFX.Tongue[1],
                                          0,
                                          16 * C,
                                          s);
                }

                // Yoshi's Body
                RenderTexturePlayer(Z, sX + p.YoshiBX,
                                      sY + p.YoshiBY,
                                      32, 32,
                                      GFXYoshiB[B], 0, 32 * p.YoshiBFrame, s);

                // Yoshi's Head
                RenderTexturePlayer(Z, sX + p.YoshiTX,
                                      sY + p.YoshiTY,
                                      32, 32,
                                      GFXYoshiT[B], 0, 32 * p.YoshiTFrame, s);
            }

            if(p.Fairy) // draw a fairy
            {
                p.Frame = 1;

                //if(!p.Immune2) // Always true because of covered condition above
                {
                    RenderTexturePlayer(Z, sX - 5,
                                          sY - 2,
                                          32, 32,
                                          GFXNPC[NPCID_FLY_POWER],
                                          0,
                                          (SpecialFrame[9] + (p.Direction > 0 ? 1 : 3)) * 32,
                                          s);
//                    if(p.Direction == 1)
//                    {
//                        RenderTexturePlayer(Z, camX + p.Location.X - 5, camY + p.Location.Y - 2, 32, 32, GFXNPC[254], 0, (SpecialFrame[9] + 1) * 32, s);
//                    }
//                    else
//                    {
//                        RenderTexturePlayer(Z, camX + p.Location.X - 5, camY + p.Location.Y - 2, 32, 32, GFXNPC[254], 0, (SpecialFrame[9] + 3) * 32, s);
//                    }
                }
            }
            else if(p.Character >= 1 && p.Character <= 5) // draw player
            {
                StdPicture& tx = (*s_char_tex[p.Character - 1])[p.State];
                int offX = pfrOffX(tx, p);
                int offY = pfrOffY(tx, p);

                if(p.Mount == 0)
                {
                    RenderTexturePlayer(Z,
                                sX + offX,
                                sY + offY,
                                pfrW(tx, p),
                                pfrH(tx, p),
                                tx,
                                pfrX(tx, p),
                                pfrY(tx, p),
                                s);
                }
                else if(p.Mount == 1)
                {
                    if(!p.Duck)
                    {
                        int small_toad_oy_corr
                            = (p.Character == 4 && p.State == 1)
                            ? 6
                            : 0;

                        int peach_h_corr
                            = (p.Character == 3)
                            ? -2
                            : 0;

                        RenderTexturePlayer(Z,
                                    sX + offX,
                                    small_toad_oy_corr + sY + offY,
                                    pfrW(tx, p),
                                    h - 26 /*- p.MountOffsetY*/ - offY + peach_h_corr,
                                    tx,
                                    pfrX(tx, p),
                                    pfrY(tx, p),
                                    s);
                    }

                    RenderTexturePlayer(Z, sX + w / 2 - 16,
                                          sY + h - 30,
                                          32, 32,
                                          GFX.Boot[p.MountType],
                                          0,
                                          32 * p.MountFrame,
                                          s);
                }
                else if(p.Mount == 3)
                {
                    RenderTexturePlayer(Z, sX + offX,
                                          sY + offY + p.MountOffsetY,
                                          pfrW(tx, p),
                                          pfrH(tx, p),
                                          tx,
                                          pfrX(tx, p),
                                          pfrY(tx, p),
                                          s);
                }

                if(p.State == PLR_STATE_CYCLONE)
                    DrawCycloneAccessory(Z, p, sX + w / 2, sY, s);
            }

        // peach/toad held npcs
            if((p.Character == 3 || p.Character == 4) && p.HoldingNPC > 0 && p.Effect != PLREFF_WARP_DOOR)
            {
                if(NPC[p.HoldingNPC].Type != NPCID_ICE_CUBE)
                {
                    if(
                        (
                            (
                                 (
                                        NPC[p.HoldingNPC].HoldingPlayer > 0 &&
                                        Player[NPC[p.HoldingNPC].HoldingPlayer].Effect != PLREFF_WARP_PIPE
                                  ) ||
                                 (NPC[p.HoldingNPC].Type == NPCID_TOOTHY && NPC[p.HoldingNPC].vehiclePlr == 0) ||
                                 (NPC[p.HoldingNPC].Type == NPCID_BULLET && NPC[p.HoldingNPC].CantHurt > 0)
                             ) ||
                          NPC[p.HoldingNPC].Effect == NPCEFF_PET_TONGUE
                        ) &&
                        NPC[p.HoldingNPC].Type != NPCID_ITEM_BURIED &&
                     !Player[NPC[p.HoldingNPC].HoldingPlayer].Dead
                    )
                    {
                        int npc_sX = camX + s_round2int_plr(NPC[p.HoldingNPC].Location.X);
                        int npc_sY = camY + s_round2int_plr(NPC[p.HoldingNPC].Location.Y);
                        int npc_w = s_round2int(NPC[p.HoldingNPC].Location.Width);
                        int npc_h = s_round2int(NPC[p.HoldingNPC].Location.Height);

                        if(!NPCIsYoshi(NPC[p.HoldingNPC]) && NPC[p.HoldingNPC].Type > 0)
                        {
                            if(NPC[p.HoldingNPC]->WidthGFX == 0)
                            {
                                RenderTexturePlayer(Z, npc_sX + NPC[p.HoldingNPC]->FrameOffsetX,
                                                      npc_sY + NPC[p.HoldingNPC]->FrameOffsetY,
                                                      npc_w,
                                                      npc_h,
                                                      GFXNPC[NPC[p.HoldingNPC].Type],
                                                      0,
                                                      NPC[p.HoldingNPC].Frame * npc_h);
                            }
                            else
                            {
                                RenderTexturePlayer(Z, npc_sX + (NPC[p.HoldingNPC]->FrameOffsetX * -NPC[p.HoldingNPC].Direction) - NPC[p.HoldingNPC]->WidthGFX / 2 + npc_w / 2,
                                                      npc_sY + NPC[p.HoldingNPC]->FrameOffsetY - NPC[p.HoldingNPC]->HeightGFX + npc_h,
                                                      NPC[p.HoldingNPC]->WidthGFX,
                                                      NPC[p.HoldingNPC]->HeightGFX,
                                                      GFXNPC[NPC[p.HoldingNPC].Type],
                                                      0,
                                                      NPC[p.HoldingNPC].Frame * NPC[p.HoldingNPC]->HeightGFX);
                            }
                        }
                    }
                }
                else
                {
                    DrawFrozenNPC(camX, camY, p.HoldingNPC);
                }
            }

            if(!p.Fairy)
            {
                if(p.Mount == 3 && p.YoshiBlue)
                {
                    if(p.Direction == 1)
                    {
                        RenderTexturePlayer(Z, sX + p.YoshiBX - 12,
                                              sY + p.YoshiBY - 16,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                    else
                    {
                        RenderTexturePlayer(Z, sX + p.YoshiBX + 12,
                                              sY + p.YoshiBY - 16,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                }
                if((p.Mount == 1 && p.MountType == 3) || p.Effect == PLREFF_COOP_WINGS)
                {
                    if(p.Direction == 1)
                    {
                        RenderTexturePlayer(Z, sX - 24,
                                              sY + h - 40,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                    else
                    {
                        RenderTexturePlayer(Z, sX + 16,
                                              sY + h - 40,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                }
            }
        }
    }
}
