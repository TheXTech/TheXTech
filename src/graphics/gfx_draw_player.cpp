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

#include <array>

#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "core/render.h"
#include "gfx.h"

#include "npc_id.h"
#include "npc_traits.h"

#include "graphics/gfx_keyhole.h"

//! Get left pixel at the player sprite
int pfrX(const StdPicture& tx, const Player_t& p)
{
#if !defined(THEXTECH_WIP_FEATURES) && !defined(__16M__)
    UNUSED(tx);
    return ((p.Frame * p.Direction + 49) / 10) * 100;

#else
    // FIXME: Replace this heuristic logic with a proper texture flags mechanism

    // will use internal flags (tx.flags & PLAYER_MODERN and tx.flags & PLAYER_CUSTOM) in future
    // if tx.flags & PLAYER_CUSTOM, then will use heap-allocated polymorphic CustomData_t* tx.custom_data,
    // which will include all frame bounding boxes and offsets
    if(tx.w == 1000)
        return ((p.Frame * p.Direction + 49) / 10) * 100;
    else
    {
        int col_w = (p.Character == 5) ? 64 : 48;
        int n_rows = 4;
        int n_cols = (p.Character == 5) ? 4 : 10;

        int fr = (p.Frame <= 32) ? p.Frame : p.Frame - 7;

        if(p.Character == 5)
            fr -= 1;

        int col = fr / n_rows;

        // load left-facing frame (bottom) - right to left (in order to mirror top half)
        if(fr != 0 && p.Direction < 0)
            return col_w * (n_cols - col - 1);
        // load right-facing frame (top) - left to right
        else
            return col_w * col;
    }
#endif
}

//! Get top pixel at the player sprite
int pfrY(const StdPicture& tx, const Player_t& p)
{
#if !defined(THEXTECH_WIP_FEATURES) && !defined(__16M__)
    UNUSED(tx);
    return ((p.Frame * p.Direction + 49) % 10) * 100;

#else
    // FIXME: Replace this heuristic logic with a proper texture flags mechanism
    if(tx.w == 1000)
        return ((p.Frame * p.Direction + 49) % 10) * 100;
    else
    {
        int row_h = 64;
        int n_rows = 4;

        int fr = (p.Frame <= 32) ? p.Frame : p.Frame - 7;

        if(p.Character == 5)
            fr -= 1;

        int row = fr % n_rows;

        // load left-facing frame (bottom)
        if(fr != 0 && p.Direction < 0)
            return row_h * (n_rows + row);
        // load right-facing frame (top)
        else
            return row_h * row;
    }
#endif
}

//! Get width at the player sprite
int pfrW(const StdPicture& tx, const Player_t& p)
{
    if(tx.w == 1000)
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

    if(tx.w == 1000)
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

    using plr_frame_off_arr = RangeArrI<int, 0, maxPlayerFrames, 0>;
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

    using plr_frame_off_arr = RangeArrI<int, 0, maxPlayerFrames, 0>;
    constexpr std::array<plr_frame_off_arr*, 5> char_offsetY = {&MarioFrameY, &LuigiFrameY, &PeachFrameY, &ToadFrameY, &LinkFrameY};
    int offY = (*char_offsetY[p.Character - 1])[(p.State * 100) + (p.Frame * p.Direction)];

    return offY;
}

void DrawPlayer(const int A, const int Z, XTColor color)
{
    DrawPlayer(Player[A], Z, color);
}

void DrawPlayer(Player_t &p, const int Z, XTColor color)
{
    double camX = vScreen[Z].CameraAddX();
    double camY = vScreen[Z].CameraAddY();

    int B = 0;
    double C = 0;
    XTColor s = (ShadowMode ? XTColor(0, 0, 0) : color);
    //auto &p = Player[A];

    bool player_door_scroll = (p.Effect == 7 && p.Effect2 >= 128);

    if(!p.Dead && !p.Immune2 && p.TimeToLive == 0 && !(p.Effect == 3 || p.Effect == 5 || p.Effect == 8 || p.Effect == 10 || player_door_scroll))
    {
        if(vScreenCollision(Z, p.Location))
        {
            if(p.Mount == 3 && !p.Fairy)
            {
                B = p.MountType;
                // Yoshi's Tongue
                if(p.MountSpecial > 0)
                {

                    C = 0;
                    if(p.Direction == -1)
                        C = p.YoshiTongueLength;

                    RenderTexturePlayer(Z, camX + double(p.YoshiTongueX) - C - 1,
                                          camY + p.YoshiTongue.Y,
                                          p.YoshiTongueLength + 2,
                                          16,
                                          GFX.Tongue[2],
                                          0, 0,
                                          s);

                    C = 1;
                    if(p.Direction == 1)
                        C = 0;

                    RenderTexturePlayer(Z, camX + p.YoshiTongue.X,
                                          camY + p.YoshiTongue.Y,
                                          16, 16,
                                          GFX.Tongue[1],
                                          0,
                                          16 * C,
                                          s);
                }

                // Yoshi's Body
                RenderTexturePlayer(Z, camX + p.Location.X + p.YoshiBX,
                                      camY + p.Location.Y + p.YoshiBY,
                                      32, 32,
                                      GFXYoshiB[B], 0, 32 * p.YoshiBFrame, s);

                // Yoshi's Head
                RenderTexturePlayer(Z, camX + p.Location.X + p.YoshiTX,
                                      camY + p.Location.Y + p.YoshiTY,
                                      32, 32,
                                      GFXYoshiT[B], 0, 32 * p.YoshiTFrame, s);
            }

            if(p.Fairy) // draw a fairy
            {
                p.Frame = 1;

                //if(!p.Immune2) // Always true because of covered condition above
                {
                    RenderTexturePlayer(Z, camX + p.Location.X - 5,
                                          camY + p.Location.Y - 2,
                                          32, 32,
                                          GFXNPC[254],
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
                using plr_pic_arr = RangeArr<StdPicture, 1, 10>;
                constexpr std::array<plr_pic_arr*, 5> char_tex = {&GFXMario, &GFXLuigi, &GFXPeach, &GFXToad, &GFXLink};

                StdPicture& tx = (*char_tex[p.Character - 1])[p.State];
                int offX = pfrOffX(tx, p);
                int offY = pfrOffY(tx, p);

                if(p.Mount == 0)
                {
                    RenderTexturePlayer(Z,
                                camX + p.Location.X + offX,
                                camY + p.Location.Y + offY,
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
                                    camX + p.Location.X + offX,
                                    small_toad_oy_corr + camY + p.Location.Y + offY,
                                    pfrW(tx, p),
                                    p.Location.Height - 26 /*- p.MountOffsetY*/ - offY + peach_h_corr,
                                    tx,
                                    pfrX(tx, p),
                                    pfrY(tx, p),
                                    s);
                    }

                    RenderTexturePlayer(Z, camX + p.Location.X + p.Location.Width / 2.0 - 16,
                                          camY + p.Location.Y + p.Location.Height - 30,
                                          32, 32,
                                          GFX.Boot[p.MountType],
                                          0,
                                          32 * p.MountFrame,
                                          s);
                }
                else if(p.Mount == 3)
                {
                    RenderTexturePlayer(Z, camX + p.Location.X + offX,
                                          camY + p.Location.Y + offY + p.MountOffsetY,
                                          pfrW(tx, p),
                                          pfrH(tx, p),
                                          tx,
                                          pfrX(tx, p),
                                          pfrY(tx, p),
                                          s);
                }
            }

        // peach/toad held npcs
            if((p.Character == 3 || p.Character == 4) && p.HoldingNPC > 0 && p.Effect != 7)
            {
                if(NPC[p.HoldingNPC].Type != NPCID_ICE_CUBE)
                {
                    if(
                        (
                            (
                                 (
                                        NPC[p.HoldingNPC].HoldingPlayer > 0 &&
                                        Player[NPC[p.HoldingNPC].HoldingPlayer].Effect != 3
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
                        if(!NPCIsYoshi(NPC[p.HoldingNPC]) && NPC[p.HoldingNPC].Type > 0)
                        {
                            if(NPC[p.HoldingNPC]->WidthGFX == 0)
                            {
                                RenderTexturePlayer(Z, camX + NPC[p.HoldingNPC].Location.X + NPC[p.HoldingNPC]->FrameOffsetX,
                                                      camY + NPC[p.HoldingNPC].Location.Y + NPC[p.HoldingNPC]->FrameOffsetY,
                                                      NPC[p.HoldingNPC].Location.Width,
                                                      NPC[p.HoldingNPC].Location.Height,
                                                      GFXNPC[NPC[p.HoldingNPC].Type],
                                                      0,
                                                      NPC[p.HoldingNPC].Frame * NPC[p.HoldingNPC].Location.Height);
                            }
                            else
                            {
                                RenderTexturePlayer(Z, camX + NPC[p.HoldingNPC].Location.X + (NPC[p.HoldingNPC]->FrameOffsetX * -NPC[p.HoldingNPC].Direction) - NPC[p.HoldingNPC]->WidthGFX / 2.0 + NPC[p.HoldingNPC].Location.Width / 2.0,
                                                      camY + NPC[p.HoldingNPC].Location.Y + NPC[p.HoldingNPC]->FrameOffsetY - NPC[p.HoldingNPC]->HeightGFX + NPC[p.HoldingNPC].Location.Height,
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
                    DrawFrozenNPC(Z, p.HoldingNPC);
                }
            }

            if(!p.Fairy)
            {
                if(p.Mount == 3 && p.YoshiBlue)
                {
                    if(p.Direction == 1)
                    {
                        RenderTexturePlayer(Z, camX + p.Location.X + p.YoshiBX - 12,
                                              camY + p.Location.Y + p.YoshiBY - 16,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                    else
                    {
                        RenderTexturePlayer(Z, camX + p.Location.X + p.YoshiBX + 12,
                                              camY + p.Location.Y + p.YoshiBY - 16,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                }
                if(p.Mount == 1 && p.MountType == 3)
                {
                    if(p.Direction == 1)
                    {
                        RenderTexturePlayer(Z, camX + p.Location.X - 24,
                                              camY + p.Location.Y + p.Location.Height - 40,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                    else
                    {
                        RenderTexturePlayer(Z, camX + p.Location.X + 16,
                                              camY + p.Location.Y + p.Location.Height - 40,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s);
                    }
                }
            }
        }
    }
}
