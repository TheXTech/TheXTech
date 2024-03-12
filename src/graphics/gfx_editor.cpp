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

#include <fmt_format_ne.h>

#include "core/render.h"
#include "globals.h"
#include "graphics.h"
#include "layers.h"
#include "npc.h"
#include "collision.h"
#include "gfx.h"
#include "config.h"
#include "npc_id.h"
#include "npc_traits.h"
#include "player.h"

#include "main/trees.h"

#include "editor.h"
#include "editor/new_editor.h"
#include "editor/editor_strings.h"
#include "editor/magic_block.h"

#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif

void DrawEditorLevel(int Z)
{
    int A = 0;
    int B = 0;
    int C = 0;
    int D;
    int E;
    Location_t tempLocation;
    int S = curSection; // Level section to display

#ifdef __3DS__
    XRender::setTargetLayer(2);
#endif
    if(LevelEditor)
    {
        if((CommonFrame % 46) <= 30)
        {
            // render NPCs in blocks
            for(A = 1; A <= numBlock; A++)
            {
                if(Block[A].Special > 0 && !Block[A].Hidden)
                {
                    if(vScreenCollision(Z, Block[A].Location))
                    {
                        NPCID C = NPCID_NULL;
                        if(Block[A].Special > 1000)
                            C = NPCID(Block[A].Special - 1000);
                        else
                            C = NPCID_COIN_S3;

                        if(NPCWidthGFX(C) == 0)
                        {
                            tempLocation.X = Block[A].Location.X + Block[A].Location.Width / 2 - NPCWidth(C) / 2;
                            tempLocation.Y = Block[A].Location.Y + Block[A].Location.Height / 2 - NPCHeight(C) / 2;
                            tempLocation.Height = NPCHeight(C);
                            tempLocation.Width = NPCWidth(C);
                        }
                        else
                        {
                            tempLocation.X = Block[A].Location.X + Block[A].Location.Width / 2 - NPCWidthGFX(C) / 2;
                            tempLocation.Y = Block[A].Location.Y + Block[A].Location.Height / 2 - NPCHeightGFX(C) / 2;
                            tempLocation.Height = NPCHeightGFX(C);
                            tempLocation.Width = NPCWidthGFX(C);
                        }

                        XRender::renderTexture(vScreen[Z].X + tempLocation.X + NPCFrameOffsetX(C),
                            vScreen[Z].Y + tempLocation.Y + NPCFrameOffsetY(C),
                            tempLocation.Width, tempLocation.Height,
                            GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                    }
                }

                // new: indicate that blocks have events
                if(Block[A].TriggerHit != EVENT_NONE || Block[A].TriggerDeath != EVENT_NONE || Block[A].TriggerLast != EVENT_NONE)
                {
                    if(vScreenCollision(Z, Block[A].Location))
                    {
                        tempLocation.X = Block[A].Location.X + Block[A].Location.Width / 2 - GFX.Chat.w / 2;
                        tempLocation.Y = Block[A].Location.Y - GFX.Chat.h - 8;

                        XRender::renderTexture(vScreen[Z].X + tempLocation.X, vScreen[Z].Y + tempLocation.Y, GFX.Chat, XTColorF(1., 0., 0., 0.7f));
                    }
                }
            }

            // render NPCs in containers
            for(A = 1; A <= numNPCs; A++)
            {
                if(!NPC[A].Hidden && (NPC[A].Type == NPCID_ITEM_BURIED || NPC[A].Type == NPCID_ITEM_POD)
                    && (NPC[A].Special > 0))
                {
                    if(vScreenCollision(Z, NPC[A].Location))
                    {
                        NPCID C = NPCID(NPC[A].Special);
                        if(NPCWidthGFX(C) == 0)
                        {
                            tempLocation.Height = NPCHeight(C);
                            tempLocation.Width = NPCWidth(C);
                        }
                        else
                        {
                            tempLocation.Height = NPCHeightGFX(C);
                            tempLocation.Width = NPCWidthGFX(C);
                        }
                        if(NPC[A].Type == NPCID_ITEM_POD)
                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - tempLocation.Height;
                        else
                            tempLocation.Y = NPC[A].Location.Y;
                        tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2 - tempLocation.Width / 2;
                        XRender::renderTexture(vScreen[Z].X + tempLocation.X + NPCFrameOffsetX(C),
                            vScreen[Z].Y + tempLocation.Y + NPCFrameOffsetY(C),
                            tempLocation.Width, tempLocation.Height,
                            GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                    }
                }

                // new: indicate that NPCs have events
                if(NPC[A].TriggerActivate != EVENT_NONE || NPC[A].TriggerTalk != EVENT_NONE || NPC[A].TriggerDeath != EVENT_NONE || NPC[A].TriggerLast != EVENT_NONE)
                {
                    if(vScreenCollision(Z, NPC[A].Location))
                    {
                        if(NPC[A].Text == STRINGINDEX_NONE)
                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2 - GFX.Chat.w / 2;
                        else
                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2 - 4 - GFX.Chat.w;
                        tempLocation.Y = NPC[A].Location.Y - GFX.Chat.h - 8;

                        XRender::renderTexture(vScreen[Z].X + tempLocation.X, vScreen[Z].Y + tempLocation.Y, GFX.Chat, XTColorF(1., 0., 0., 0.7f));
                    }
                }

                // and that they can talk
                if(NPC[A].Text != STRINGINDEX_NONE)
                {
                    if(vScreenCollision(Z, NPC[A].Location))
                    {
                        if(!(NPC[A].TriggerActivate != EVENT_NONE || NPC[A].TriggerTalk != EVENT_NONE || NPC[A].TriggerDeath != EVENT_NONE || NPC[A].TriggerLast != EVENT_NONE))
                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2 - GFX.Chat.w / 2;
                        else
                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2 + 4;
                        tempLocation.Y = NPC[A].Location.Y - GFX.Chat.h - 8;

                        XRender::renderTexture(vScreen[Z].X + tempLocation.X, vScreen[Z].Y + tempLocation.Y, GFX.Chat, XTColorF(1., 1., 1., 0.7f));
                    }
                }
            }
        }

        // render player start points
        for(A = 1; A <= 2; A++)
        {
            if(!(PlayerStart[A].Width > 0)) continue;
            if(vScreenCollision(Z, PlayerStart[A]))
            {
                Player_t& p = Player[A];

                p.Character = testPlayer[A].Character;
                p.State = testPlayer[A].State;
                p.Mount = testPlayer[A].Mount;
                p.MountType = testPlayer[A].MountType;

                if(p.Character < 1 || p.Character > 5)
                    p.Character = A;
                if(p.State < 1 || p.State > 7)
                    p.State = 2;

                p.Direction = 1;
                p.Location.SpeedY = 0;
                p.Location.SpeedX = 0;
                p.Controls.Left = false;
                p.Controls.Right = false;
                p.SpinJump = false;
                p.Dead = false;
                p.Immune2 = false;
                p.Fairy = false;
                p.TimeToLive = 0;
                p.Effect = 0;
                p.MountSpecial = 0;
                p.HoldingNPC = 0;
                if(p.Duck)
                    UnDuck(p);
                PlayerFrame(p);

                if(p.MountType == 3)
                {
                    p.YoshiWingsFrameCount += 1;
                    p.YoshiWingsFrame = 0;
                    if(p.YoshiWingsFrameCount <= 12)
                        p.YoshiWingsFrame = 1;
                    else if(p.YoshiWingsFrameCount >= 24)
                        p.YoshiWingsFrameCount = 0;
                    if(p.Direction == 1)
                        p.YoshiWingsFrame += 2;
                }

                C = Physics.PlayerHeight[p.Character][p.State] - Physics.PlayerHeight[A][2];

                p.Location.X = PlayerStart[A].X;
                p.Location.Y = PlayerStart[A].Y - C;
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                SizeCheck(p);

                DrawPlayer(p, Z);
            }
        }

        // render warps
        for(A = 1; A <= numWarps; A++)
        {
            if(Warp[A].Direction > 0 && !Warp[A].Hidden)
            {
                bool complete = Warp[A].PlacedEnt && Warp[A].PlacedExit;
                XTColor color = complete ? XTColorF(1.f, 0.f, 1.f) : XTColorF(0.7f, 0.3f, 0.f);

                if(Warp[A].PlacedEnt)
                {
                    XRender::renderRect(vScreen[Z].X + Warp[A].Entrance.X, vScreen[Z].Y + Warp[A].Entrance.Y, 32, 32,
                        color, false);
                    SuperPrint(std::to_string(A), 1, vScreen[Z].X + Warp[A].Entrance.X + 2, vScreen[Z].Y + Warp[A].Entrance.Y + 2);
                }

                if(Warp[A].PlacedExit)
                {
                    XRender::renderRect(vScreen[Z].X + Warp[A].Exit.X, vScreen[Z].Y + Warp[A].Exit.Y, 32, 32,
                        color, false);
                    SuperPrint(std::to_string(A), 1, vScreen[Z].X + Warp[A].Exit.X + Warp[A].Exit.Width - 16 - 2,
                        vScreen[Z].Y + Warp[A].Exit.Y + Warp[A].Exit.Height - 14 - 2);
                }
            }
        }
    }

#ifdef __3DS__
    XRender::setTargetLayer(0);
#endif
    // render section boundary
    if(LevelEditor)
    {
        if((CommonFrame % 46) > 30 || (CommonFrame % 46) == 0)
        {
            if(vScreen[Z].X + level[S].X > 0)
            {
                XRender::renderRect(0, 0,
                                   vScreen[Z].X + level[S].X, XRender::TargetH, {0, 0, 0}, true);
            }

            if(XRender::TargetW > level[S].Width + vScreen[Z].X)
            {
                XRender::renderRect(level[S].Width + vScreen[Z].X, 0,
                                   XRender::TargetW - (level[S].Width + vScreen[Z].X), XRender::TargetH, {0, 0, 0}, true);
            }

            if(vScreen[Z].Y + level[S].Y > 0)
            {
                XRender::renderRect(0, 0,
                                   XRender::TargetW, vScreen[Z].Y + level[S].Y, {0, 0, 0}, true);
            }

            if(XRender::TargetH > level[S].Height + vScreen[Z].Y)
            {
                XRender::renderRect(0, level[S].Height + vScreen[Z].Y,
                                   XRender::TargetW, XRender::TargetH - (level[S].Height + vScreen[Z].Y), {0, 0, 0}, true);
            }
        }
    }

#ifdef __3DS__
    XRender::setTargetLayer(3);
#endif

    // In-Editor message box preview
    if(editorScreen.active && !MessageText.empty())
    {
        if(MessageTextMap.empty())
            DrawMessage(MessageText);
        else
            DrawMessage(MessageTextMap);
    }

#ifdef __3DS__
    // disable cursor rendering on main screen when editor screen is active
    if(!editorScreen.active)
#endif
    // Display the cursor
    {
        auto &e = EditorCursor;
        int curX = int(double(e.X) - vScreen[Z].TargetX());
        int curY = int(double(e.Y) - vScreen[Z].TargetY());

        if((CommonFrame % 46) < 10)
        {
            // don't draw the currently held object
        }
        else if(e.Mode == OptCursor_t::LVL_BLOCKS) // Blocks
        {
            auto &b = e.Block;
            if(BlockIsSizable[b.Type])
            {
                if(vScreenCollision(Z, b.Location))
                {
                    for(B = 0; B <= (b.Location.Height / 32) - 1; B++)
                    {
                        for(C = 0; C <= (b.Location.Width / 32) - 1; C++)
                        {
                            D = C;
                            E = B;

                            if(D != 0)
                            {
                                if(fEqual(D, (b.Location.Width / 32) - 1))
                                    D = 2;
                                else
                                {
                                    D = 1;
                                }
                            }

                            if(E != 0)
                            {
                                if(fEqual(E, (b.Location.Height / 32) - 1))
                                    E = 2;
                                else
                                    E = 1;
                            }

                            XRender::renderTexture(vScreen[Z].X + b.Location.X + C * 32,
                                                  vScreen[Z].Y + b.Location.Y + B * 32,
                                                  32, 32, GFXBlock[b.Type], D * 32, E * 32);
                        }
                    }
                }
            }
            else
            {
                if(vScreenCollision(Z, b.Location))
                {
                    XRender::renderTexture(vScreen[Z].X + b.Location.X,
                                          vScreen[Z].Y + b.Location.Y + b.ShakeOffset,
                                          b.Location.Width,
                                          b.Location.Height,
                                          GFXBlock[b.Type], 0, BlockFrame[b.Type] * 32);
                }
            }

            // render NPC inside block
            if((CommonFrame % 46) <= 30 && b.Special > 0)
            {
                if(vScreenCollision(Z, b.Location))
                {
                    NPCID C = NPCID_NULL;
                    if(b.Special > 1000)
                        C = NPCID(b.Special - 1000);
                    else
                        C = NPCID_COIN_S3;

                    if(NPCWidthGFX(C) == 0)
                    {
                        tempLocation.X = b.Location.X + b.Location.Width / 2 - NPCWidth(C) / 2;
                        tempLocation.Y = b.Location.Y + b.Location.Height / 2 - NPCHeight(C) / 2;
                        tempLocation.Height = NPCHeight(C);
                        tempLocation.Width = NPCWidth(C);
                    }
                    else
                    {
                        tempLocation.X = b.Location.X + b.Location.Width / 2 - NPCWidthGFX(C) / 2;
                        tempLocation.Y = b.Location.Y + b.Location.Height / 2 - NPCHeightGFX(C) / 2;
                        tempLocation.Height = NPCHeightGFX(C);
                        tempLocation.Width = NPCWidthGFX(C);
                    }

                    XRender::renderTexture(vScreen[Z].X + tempLocation.X + NPCFrameOffsetX(C),
                        vScreen[Z].Y + tempLocation.Y + NPCFrameOffsetY(C),
                        tempLocation.Width, tempLocation.Height,
                        GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                }
            }

            // new: indicate that blocks have events
            if(b.TriggerHit != EVENT_NONE || b.TriggerDeath != EVENT_NONE || b.TriggerLast != EVENT_NONE)
            {
                tempLocation.X = b.Location.X + b.Location.Width / 2 - GFX.Chat.w / 2;
                tempLocation.Y = b.Location.Y - GFX.Chat.h - 8;

                XRender::renderTexture(vScreen[Z].X + tempLocation.X, vScreen[Z].Y + tempLocation.Y, GFX.Chat, XTColorF(1., 0., 0., 0.7f));
            }
        }

        else if(e.Mode == OptCursor_t::LVL_SETTINGS) // Player start points
        {
            if(e.SubMode == 4 || e.SubMode == 5)
            {
                A = e.SubMode - 3;

                Player_t& p = Player[A];

                p.Character = testPlayer[A].Character;
                p.State = testPlayer[A].State;
                p.Mount = testPlayer[A].Mount;
                p.MountType = testPlayer[A].MountType;

                if(p.Character < 1 || p.Character > 5)
                    p.Character = A;
                if(p.State < 1 || p.State > 7)
                    p.State = 2;

                p.Direction = 1;
                p.Location.SpeedY = 0;
                p.Location.SpeedX = 0;
                p.Controls.Left = false;
                p.Controls.Right = false;
                p.SpinJump = false;
                p.Dead = false;
                p.Immune2 = false;
                p.Fairy = false;
                p.TimeToLive = 0;
                p.Effect = 0;
                p.MountSpecial = 0;
                p.HoldingNPC = 0;
                if(p.Duck)
                    UnDuck(p);
                PlayerFrame(p);

                if(p.MountType == 3)
                {
                    p.YoshiWingsFrameCount += 1;
                    p.YoshiWingsFrame = 0;
                    if(p.YoshiWingsFrameCount <= 12)
                        p.YoshiWingsFrame = 1;
                    else if(p.YoshiWingsFrameCount >= 24)
                        p.YoshiWingsFrameCount = 0;
                    if(p.Direction == 1)
                        p.YoshiWingsFrame += 2;
                }

                C = Physics.PlayerHeight[p.Character][p.State] - Physics.PlayerHeight[A][2];

                p.Location.X = e.Location.X;
                p.Location.Y = e.Location.Y - C;
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                SizeCheck(p);

                DrawPlayer(p, Z);
            }
        }

        else if(e.Mode == OptCursor_t::LVL_BGOS) // BGOs
        {
            auto &b = e.Background;
            if(vScreenCollision(Z, b.Location))
            {
                XRender::renderTexture(vScreen[Z].X + b.Location.X,
                                      vScreen[Z].Y + b.Location.Y,
                                      BackgroundWidth[b.Type],
                                      BackgroundHeight[b.Type],
                                      GFXBackground[b.Type], 0,
                                      BackgroundHeight[b.Type] * BackgroundFrame[b.Type]);
            }
        }

        else if(e.Mode == OptCursor_t::LVL_NPCS) // NPCs
        {
            e.NPC.Frame = NPC[0].Frame;
            e.NPC.FrameCount = NPC[0].FrameCount;
            NPC[0] = e.NPC;
            NPCFrames(0);
            e.NPC = NPC[0];

            auto &n = e.NPC;
            if(n->WidthGFX == 0)
            {
                XRender::renderTexture(vScreen[Z].X + n.Location.X + n->FrameOffsetX,
                                      vScreen[Z].Y + n.Location.Y + n->FrameOffsetY,
                                      n.Location.Width,
                                      n.Location.Height,
                                      GFXNPC[n.Type], 0, n.Frame * n.Location.Height);
            }
            else
            {
                if(n.Type == NPCID_ITEM_BUBBLE && n.Special > 0)
                {
                    if(NPCWidthGFX(n.Special) == 0)
                    {
                        tempLocation.Width = NPCWidth(n.Special);
                        tempLocation.Height = NPCHeight(n.Special);
                    }
                    else
                    {
                        tempLocation.Width = NPCWidthGFX(n.Special);
                        tempLocation.Height = NPCHeightGFX(n.Special);
                    }

                    tempLocation.X = n.Location.X + n.Location.Width / 2 - tempLocation.Width / 2;
                    tempLocation.Y = n.Location.Y + n.Location.Height / 2 - tempLocation.Height / 2;
                    B = EditorNPCFrame(NPCID(n.Special), n.Direction);

                    XRender::renderTexture(vScreen[Z].X + tempLocation.X + n->FrameOffsetX,
                                          vScreen[Z].Y + tempLocation.Y,
                                          tempLocation.Width, tempLocation.Height,
                                          GFXNPC[n.Special], 0, B * tempLocation.Height);
                }

                XRender::renderTexture(vScreen[Z].X + n.Location.X + n->FrameOffsetX - n->WidthGFX / 2 + n.Location.Width / 2,
                                      vScreen[Z].Y + n.Location.Y + n->FrameOffsetY - n->HeightGFX + n.Location.Height,
                                      n->WidthGFX, n->HeightGFX, GFXNPC[n.Type],
                                      0, n.Frame * n->HeightGFX);
            }

            // render NPC inside container
            if((CommonFrame % 46) <= 30 && (n.Type == NPCID_ITEM_BURIED || n.Type == NPCID_ITEM_POD)
                && (n.Special > 0))
            {
                if(vScreenCollision(Z, n.Location))
                {
                    NPCID C = NPCID(n.Special);
                    if(NPCWidthGFX(C) == 0)
                    {
                        tempLocation.Height = NPCHeight(C);
                        tempLocation.Width = NPCWidth(C);
                    }
                    else
                    {
                        tempLocation.Height = NPCHeightGFX(C);
                        tempLocation.Width = NPCWidthGFX(C);
                    }

                    if(n.Type == NPCID_ITEM_POD)
                        tempLocation.Y = n.Location.Y + n.Location.Height - tempLocation.Height;
                    else
                        tempLocation.Y = n.Location.Y;

                    tempLocation.X = n.Location.X + n.Location.Width / 2 - tempLocation.Width / 2;

                    XRender::renderTexture(vScreen[Z].X + tempLocation.X + NPCFrameOffsetX(C),
                        vScreen[Z].Y + tempLocation.Y + NPCFrameOffsetY(C),
                        tempLocation.Width, tempLocation.Height,
                        GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                }
            }

            // new: indicate that NPCs have events
            if(n.TriggerActivate != EVENT_NONE || n.TriggerTalk != EVENT_NONE || n.TriggerDeath != EVENT_NONE || n.TriggerLast != EVENT_NONE)
            {
                if(n.Text == STRINGINDEX_NONE)
                    tempLocation.X = n.Location.X + n.Location.Width / 2 - GFX.Chat.w / 2;
                else
                    tempLocation.X = n.Location.X + n.Location.Width / 2 - 4 - GFX.Chat.w;
                tempLocation.Y = n.Location.Y - GFX.Chat.h - 8;

                XRender::renderTexture(vScreen[Z].X + tempLocation.X, vScreen[Z].Y + tempLocation.Y, GFX.Chat, XTColorF(1., 0., 0., 0.7f));
            }

            // and that they can talk
            if(n.Text != STRINGINDEX_NONE)
            {
                if(!(n.TriggerActivate != EVENT_NONE || n.TriggerTalk != EVENT_NONE || n.TriggerDeath != EVENT_NONE || n.TriggerLast != EVENT_NONE))
                    tempLocation.X = n.Location.X + n.Location.Width / 2 - GFX.Chat.w / 2;
                else
                    tempLocation.X = n.Location.X + n.Location.Width / 2 + 4;
                tempLocation.Y = n.Location.Y - GFX.Chat.h - 8;

                XRender::renderTexture(vScreen[Z].X + tempLocation.X, vScreen[Z].Y + tempLocation.Y, GFX.Chat, XTColorF(1., 1., 1., 0.7f));
            }
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WATER) // Water
        {
            if(EditorCursor.Water.Quicksand)
                XRender::renderRect(vScreen[Z].X + EditorCursor.Location.X, vScreen[Z].Y + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                    XTColorF(1.f, 1.f, 0.f, 1.f), false);
            else
                XRender::renderRect(vScreen[Z].X + EditorCursor.Location.X, vScreen[Z].Y + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                    XTColorF(0.f, 1.f, 1.f, 1.f), false);
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
        {
            XRender::renderRect(vScreen[Z].X + EditorCursor.Location.X, vScreen[Z].Y + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                XTColorF(1.f, 0.f, 0.f, 1.f), false);
        }

#if 0
        if(g_config.editor_edge_scroll && !editorScreen.active && !MagicHand)
        {
            if(curX >= 0 && curX < 36)
                curX = 36;
            if(curY >= 0 && curY < 36)
                curY = 36;
            if(curX >= XRender::TargetW - 36)
                curX = XRender::TargetW - 36;
            if(curY >= XRender::TargetH - 36)
                curY = XRender::TargetH - 36;
        }
#endif

        if(EditorCursor.Mode == 0 || EditorCursor.Mode == 6) // Eraser
        {
            if(EditorCursor.SubMode == -1)
            {
                int frame = (CommonFrame % 46) / 20;
                if(frame > 2)
                    frame = 2;
                XRender::renderTexture(curX - 8, curY, 32, 32, GFXNPC[NPCID_AXE], 0, 32 * frame);
            }
            else
                XRender::renderTexture(curX - 2, curY, GFX.ECursor[3]);
        }

        else if(EditorCursor.Mode == 13 || EditorCursor.Mode == 14) // Selector
        {
            if(MagicBlock::enabled)
                XRender::renderTexture(curX, curY, GFX.ECursor[2], XTColorF(0.7f, 1.0f, 0.7f, 1.0f));
            else
                XRender::renderTexture(curX, curY, GFX.ECursor[2]);

        }

        // Section Resize
        else if(EditorCursor.Mode == 2 && EditorCursor.SubMode < 4)
        {
            XRender::renderTexture(curX, curY, GFX.ECursor[1]);
        }

//                Else
        else
        {
//                    If .Mode = 5 Then
            if(MagicBlock::enabled)
                XRender::renderTexture(curX, curY, GFX.ECursor[2], XTColorF(0.7f, 1.0f, 0.7f, 1.0f));
            else
                XRender::renderTexture(curX, curY, GFX.ECursor[2]);

            if(e.Layer != LAYER_NONE && e.Layer != LAYER_DEFAULT)
            {
                // there might be a tooltip in this case
                if(editorScreen.active || EditorCursor.Y < 40)
                    SuperPrint(GetL(e.Layer), 3, curX + 28 , curY + 34, XTColorF(1., 1., 1., 0.3f));
                else
                    SuperPrint(GetL(e.Layer), 3, curX + 28 , curY + 34);
            }
        }

//            End With
    }

    if(editor_section_toast > 0)
    {
        SuperPrintCenter("SECTION " + std::to_string(curSection+1), 3, vScreen[Z].Width / 2, vScreen[Z].Height - 100);
        editor_section_toast--;
    }
}

void DrawEditorLevel_UI()
{
#ifdef THEXTECH_INTERPROC_SUPPORTED
    if(!MagicHand || !IntProc::isEnabled())
#endif
    {
        editorScreen.UpdateEditorScreen(EditorScreen::CallMode::Render);
        XRender::resetViewport();
    }

#ifdef __3DS__
    if(!editorScreen.active)
    {
        editorScreen.UpdateEditorScreen(EditorScreen::CallMode::Render, true);
        XRender::resetViewport();
        XRender::setTargetLayer(3);
    }
#endif
}

void DrawEditorWorld()
{
    int Z = 1;

#ifdef __3DS__
    // disable cursor rendering on inactive screen of 3DS
    if(editorScreen.active) {}
    else
#endif

    if((CommonFrame % 46) < 10)
    {
        // don't draw the currently held object
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_TILES)
    {
        XRender::renderTexture(vScreen[Z].X + EditorCursor.Tile.Location.X,
            vScreen[Z].Y + EditorCursor.Tile.Location.Y,
            EditorCursor.Tile.Location.Width,
            EditorCursor.Tile.Location.Height,
            GFXTile[EditorCursor.Tile.Type],
            0,
            TileHeight[EditorCursor.Tile.Type] * TileFrame[EditorCursor.Tile.Type]);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES)
    {
        XRender::renderTexture(vScreen[Z].X + EditorCursor.Scene.Location.X,
            vScreen[Z].Y + EditorCursor.Scene.Location.Y,
            EditorCursor.Scene.Location.Width,
            EditorCursor.Scene.Location.Height,
            GFXScene[EditorCursor.Scene.Type],
            0,
            SceneHeight[EditorCursor.Scene.Type] * SceneFrame[EditorCursor.Scene.Type]);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_LEVELS)
    {
        if(EditorCursor.WorldLevel.Path)
        {
            XRender::renderTexture(vScreen[Z].X + EditorCursor.WorldLevel.Location.X,
                                  vScreen[Z].Y + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width,
                                  EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[0], 0, 0);
        }

        if(EditorCursor.WorldLevel.Path2)
        {
            XRender::renderTexture(vScreen[Z].X + EditorCursor.WorldLevel.Location.X - 16,
                                  vScreen[Z].Y + 8 + EditorCursor.WorldLevel.Location.Y,
                                  64, 32,
                                  GFXLevelBMP[29], 0, 0);
        }

        if(GFXLevelBig[EditorCursor.WorldLevel.Type])
        {
            XRender::renderTexture(vScreen[Z].X + EditorCursor.WorldLevel.Location.X - (GFXLevelWidth[EditorCursor.WorldLevel.Type] - 32) / 2.0,
                                  vScreen[Z].Y + EditorCursor.WorldLevel.Location.Y - GFXLevelHeight[EditorCursor.WorldLevel.Type] + 32,
                                  GFXLevelWidth[EditorCursor.WorldLevel.Type], GFXLevelHeight[EditorCursor.WorldLevel.Type],
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
        else
        {
            XRender::renderTexture(vScreen[Z].X + EditorCursor.WorldLevel.Location.X,
                                  vScreen[Z].Y + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width, EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_PATHS)
    {
        XRender::renderTexture(vScreen[Z].X + EditorCursor.WorldPath.Location.X,
            vScreen[Z].Y + EditorCursor.WorldPath.Location.Y,
            EditorCursor.WorldPath.Location.Width,
            EditorCursor.WorldPath.Location.Height,
            GFXPath[EditorCursor.WorldPath.Type],
            0, 0);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
    {
        XRender::renderRect(vScreen[Z].X + EditorCursor.WorldMusic.Location.X, vScreen[Z].Y + EditorCursor.WorldMusic.Location.Y, 32, 32,
            XTColorF(1.f, 0.f, 1.f), false);
        SuperPrint(std::to_string(EditorCursor.WorldMusic.Type), 1, vScreen[Z].X + EditorCursor.WorldMusic.Location.X + 2, vScreen[Z].Y + EditorCursor.WorldMusic.Location.Y + 2);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_AREA)
    {
        XRender::renderRect(vScreen[Z].X + EditorCursor.WorldArea.Location.X, vScreen[Z].Y + EditorCursor.WorldArea.Location.Y,
            EditorCursor.WorldArea.Location.Width, EditorCursor.WorldArea.Location.Height,
            XTColorF(1.0f, 0.8f, 0.2f), false);
    }

    double X = EditorCursor.X - vScreen[Z].TargetX();
    double Y = EditorCursor.Y - vScreen[Z].TargetY();

#if 0
    if(g_config.editor_edge_scroll && !editorScreen.active && !MagicHand)
    {
        if(X >= 0 && X < 36)
            X = 36;
        if(Y >= 0 && Y < 36)
            Y = 36;
        if(X >= XRender::TargetW - 36)
            X = XRender::TargetW - 36;
        if(Y >= XRender::TargetH - 36)
            Y = XRender::TargetH - 36;
    }
#endif

#ifdef __3DS__
    // disable cursor rendering on inactive screen of 3DS
    if(editorScreen.active) {}
    else
#endif

    if(EditorCursor.Mode == OptCursor_t::LVL_ERASER || EditorCursor.Mode == OptCursor_t::LVL_ERASER0)
    {
        if(EditorCursor.SubMode == -1)
        {
            int frame = (CommonFrame % 46) / 20;
            if(frame > 2)
                frame = 2;
            XRender::renderTexture(X - 8, Y, 32, 32, GFXNPC[NPCID_AXE], 0, 32 * frame);
        }
        else
            XRender::renderTexture(X - 2, Y, GFX.ECursor[3]);
    }
    else
    {
        XRender::renderTexture(X, Y,
            32, 32,
            GFX.ECursor[2], 0, 0);

        // show coordinates of nearby level for help making warps
        for(WorldLevel_t* t : treeWorldLevelQuery(newLoc(EditorCursor.Location.X, EditorCursor.Location.Y, 1, 1), SORTMODE_NONE))
        {
            WorldLevel_t &lvl = *t;
            if(CursorCollision(EditorCursor.Location, lvl.Location))
            {
                double at_X = lvl.Location.X + lvl.Location.Width / 2 + vScreen[Z].X;
                double at_Y = lvl.Location.Y + vScreen[Z].Y - 40;
                XRender::renderRect(at_X - 80, at_Y - 4, 160, 44, {0, 0, 0, 127}, true);
                SuperPrintCenter(fmt::format_ne("{0}: {1}", g_editorStrings.letterCoordX, static_cast<int>(lvl.Location.X)), 3, at_X, at_Y);
                SuperPrintCenter(fmt::format_ne("{0}: {1}", g_editorStrings.letterCoordY, static_cast<int>(lvl.Location.Y)), 3, at_X, at_Y + 20);
            }
        }
    }

    editorScreen.UpdateEditorScreen(EditorScreen::CallMode::Render);
    XRender::resetViewport();

#ifdef __3DS__
    if(!editorScreen.active)
    {
        editorScreen.UpdateEditorScreen(EditorScreen::CallMode::Render, true);
        XRender::resetViewport();
        XRender::setTargetLayer(3);
    }
#endif
}
