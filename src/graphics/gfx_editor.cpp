/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "core/render.h"
#include "globals.h"
#include "graphics.h"
#include "layers.h"
#include "npc.h"
#include "collision.h"
#include "gfx.h"
#include "config.h"
#include "npc_id.h"

#include "editor.h"
#include "editor/new_editor.h"
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

// #ifdef __3DS__
//     XRender::setLayer(2);
// #endif
    if(Z == 1)
        BlockFlash += 1;

    if(BlockFlash > 45)
        BlockFlash = 0;

    if(LevelEditor)
    {
        if(BlockFlash <= 30)
        {
            // render NPCs in blocks
            for(A = 1; A <= numBlock; A++)
            {
                if(Block[A].Special > 0 && !Block[A].Hidden)
                {
                    if(vScreenCollision(Z, Block[A].Location))
                    {
                        if(Block[A].Special > 1000)
                            C = Block[A].Special - 1000;
                        else
                            C = 10;

                        if(NPCWidthGFX[C] == 0)
                        {
                            tempLocation.X = Block[A].Location.X + Block[A].Location.Width / 2 - NPCWidth[C] / 2;
                            tempLocation.Y = Block[A].Location.Y + Block[A].Location.Height / 2 - NPCHeight[C] / 2;
                            tempLocation.Height = NPCHeight[C];
                            tempLocation.Width = NPCWidth[C];
                        }
                        else
                        {
                            tempLocation.X = Block[A].Location.X + Block[A].Location.Width / 2 - NPCWidthGFX[C] / 2;
                            tempLocation.Y = Block[A].Location.Y + Block[A].Location.Height / 2 - NPCHeightGFX[C] / 2;
                            tempLocation.Height = NPCHeightGFX[C];
                            tempLocation.Width = NPCWidthGFX[C];
                        }

                        XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[C],
                            vScreenY[Z] + tempLocation.Y + NPCFrameOffsetY[C],
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

                        XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFX.Chat, 1., 0., 0., 0.7);
                    }
                }
            }

            // render NPCs in containers
            for(A = 1; A <= numNPCs; A++)
            {
                if(!NPC[A].Hidden && (NPC[A].Type == 91 || NPC[A].Type == 96)
                    && (NPC[A].Special > 0))
                {
                    if(vScreenCollision(Z, NPC[A].Location))
                    {
                        C = NPC[A].Special;
                        if(NPCWidthGFX[C] == 0)
                        {
                            tempLocation.Height = NPCHeight[C];
                            tempLocation.Width = NPCWidth[C];
                        }
                        else
                        {
                            tempLocation.Height = NPCHeightGFX[C];
                            tempLocation.Width = NPCWidthGFX[C];
                        }
                        if(NPC[A].Type == 96)
                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - tempLocation.Height;
                        else
                            tempLocation.Y = NPC[A].Location.Y;
                        tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2 - tempLocation.Width / 2;
                        XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[C],
                            vScreenY[Z] + tempLocation.Y + NPCFrameOffsetY[C],
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

                        XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFX.Chat, 1., 0., 0., 0.7);
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

                        XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFX.Chat, 1., 1., 1., 0.7);
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
                int ch = testPlayer[A].Character;
                if(ch == 0)
                    ch = A;
                C = Physics.PlayerHeight[ch][2] - Physics.PlayerHeight[A][2];
                switch (ch)
                {
                case 1:
                    XRender::renderTexture(vScreenX[Z] + PlayerStart[A].X + MarioFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + MarioFrameY[201] - C,
                            99, 99, GFXMario[2], 500, 0);
                    break;
                case 2:
                    XRender::renderTexture(vScreenX[Z] + PlayerStart[A].X + LuigiFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + LuigiFrameY[201] - C,
                            99, 99, GFXLuigi[2], 500, 0);
                    break;
                case 3:
                    XRender::renderTexture(vScreenX[Z] + PlayerStart[A].X + PeachFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + PeachFrameY[201] - C,
                            99, 99, GFXPeach[2], 500, 0);
                    break;
                case 4:
                    XRender::renderTexture(vScreenX[Z] + PlayerStart[A].X + ToadFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + ToadFrameY[201] - C,
                            99, 99, GFXToad[2], 500, 0);
                    break;
                case 5:
                    XRender::renderTexture(vScreenX[Z] + PlayerStart[A].X + LinkFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + LinkFrameY[201] - C,
                            99, 99, GFXLink[2], 500, 0);
                    break;
                }
            }
        }

        // render warps
        for(A = 1; A <= numWarps; A++)
        {
            if(Warp[A].Direction > 0 && !Warp[A].Hidden)
            {
                bool complete = Warp[A].PlacedEnt && Warp[A].PlacedExit;

                if(Warp[A].PlacedEnt)
                {
                    if(complete)
                        XRender::renderRect(vScreenX[Z] + Warp[A].Entrance.X, vScreenY[Z] + Warp[A].Entrance.Y, 32, 32,
                            1.f, 0.f, 1.f, 1.f, false);
                    else
                        XRender::renderRect(vScreenX[Z] + Warp[A].Entrance.X, vScreenY[Z] + Warp[A].Entrance.Y, 32, 32,
                            0.7f, 0.3f, 0.f, 1.f, false);
                    SuperPrint(std::to_string(A), 1, vScreenX[Z] + Warp[A].Entrance.X + 2, vScreenY[Z] + Warp[A].Entrance.Y + 2);
                }

                if(Warp[A].PlacedExit)
                {
                    if(complete)
                        XRender::renderRect(vScreenX[Z] + Warp[A].Exit.X, vScreenY[Z] + Warp[A].Exit.Y, 32, 32,
                            1.f, 0.f, 1.f, 1.f, false);
                    else
                        XRender::renderRect(vScreenX[Z] + Warp[A].Exit.X, vScreenY[Z] + Warp[A].Exit.Y, 32, 32,
                            0.7f, 0.3f, 0.f, 1.f, false);
                    SuperPrint(std::to_string(A), 1, vScreenX[Z] + Warp[A].Exit.X + Warp[A].Exit.Width - 16 - 2,
                        vScreenY[Z] + Warp[A].Exit.Y + Warp[A].Exit.Height - 14 - 2);
                }
            }
        }
    }

// #ifdef __3DS__
//     XRender::setLayer(0);
// #endif
    // render section boundary
    if(LevelEditor)
    {
        if(BlockFlash > 30 || BlockFlash == 0)
        {
            if(vScreenX[Z] + level[S].X > 0)
            {
                XRender::renderRect(0, 0,
                                   vScreenX[Z] + level[S].X, ScreenH, 0.f, 0.f, 0.f, 1.f, true);
            }

            if(ScreenW > level[S].Width + vScreenX[Z])
            {
                XRender::renderRect(level[S].Width + vScreenX[Z], 0,
                                   ScreenW - (level[S].Width + vScreenX[Z]), ScreenH, 0.f, 0.f, 0.f, 1.f, true);
            }

            if(vScreenY[Z] + level[S].Y > 0)
            {
                XRender::renderRect(0, 0,
                                   ScreenW, vScreenY[Z] + level[S].Y, 0.f, 0.f, 0.f, 1.f, true);
            }

            if(ScreenH > level[S].Height + vScreenY[Z])
            {
                XRender::renderRect(0, level[S].Height + vScreenY[Z],
                                   ScreenW, ScreenH - (level[S].Height + vScreenY[Z]), 0.f, 0.f, 0.f, 1.f, true);
            }
        }
    }

// #ifdef __3DS__
//     XRender::setLayer(3);
// #endif

    // In-Editor message box preview
    if(editorScreen.active && !MessageText.empty())
    {
        DrawMessage(MessageText);
    }

    // Display the cursor
    {
        auto &e = EditorCursor;
        int curX = int(double(e.X) - vScreen[Z].Left);
        int curY = int(double(e.Y) - vScreen[Z].Top);

        if(BlockFlash < 10)
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

                            XRender::renderTexture(vScreenX[Z] + b.Location.X + C * 32,
                                                  vScreenY[Z] + b.Location.Y + B * 32,
                                                  32, 32, GFXBlock[b.Type], D * 32, E * 32);
                        }
                    }
                }
            }
            else
            {
                if(vScreenCollision(Z, b.Location))
                {
                    XRender::renderTexture(vScreenX[Z] + b.Location.X,
                                          vScreenY[Z] + b.Location.Y + b.ShakeY3,
                                          b.Location.Width,
                                          b.Location.Height,
                                          GFXBlock[b.Type], 0, BlockFrame[b.Type] * 32);
                }
            }

            // render NPC inside block
            if(BlockFlash <= 30 && b.Special > 0)
            {
                if(vScreenCollision(Z, b.Location))
                {
                    if(b.Special > 1000)
                        C = b.Special - 1000;
                    else
                        C = 10;

                    if(NPCWidthGFX[C] == 0)
                    {
                        tempLocation.X = b.Location.X + b.Location.Width / 2 - NPCWidth[C] / 2;
                        tempLocation.Y = b.Location.Y + b.Location.Height / 2 - NPCHeight[C] / 2;
                        tempLocation.Height = NPCHeight[C];
                        tempLocation.Width = NPCWidth[C];
                    }
                    else
                    {
                        tempLocation.X = b.Location.X + b.Location.Width / 2 - NPCWidthGFX[C] / 2;
                        tempLocation.Y = b.Location.Y + b.Location.Height / 2 - NPCHeightGFX[C] / 2;
                        tempLocation.Height = NPCHeightGFX[C];
                        tempLocation.Width = NPCWidthGFX[C];
                    }

                    XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[C],
                        vScreenY[Z] + tempLocation.Y + NPCFrameOffsetY[C],
                        tempLocation.Width, tempLocation.Height,
                        GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                }
            }

            // new: indicate that blocks have events
            if(b.TriggerHit != EVENT_NONE || b.TriggerDeath != EVENT_NONE || b.TriggerLast != EVENT_NONE)
            {
                tempLocation.X = b.Location.X + b.Location.Width / 2 - GFX.Chat.w / 2;
                tempLocation.Y = b.Location.Y - GFX.Chat.h - 8;

                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFX.Chat, 1., 0., 0., 0.7);
            }
        }

        else if(e.Mode == OptCursor_t::LVL_SETTINGS) // Player start points
        {
            if(e.SubMode == 4 || e.SubMode == 5)
            {
                A = e.SubMode - 3;

                int ch = testPlayer[A].Character;
                if(ch == 0)
                    ch = A;

                C = Physics.PlayerHeight[ch][2] - Physics.PlayerHeight[A][2];
                switch (ch)
                {
                case 1:
                    XRender::renderTexture(vScreenX[Z] + e.Location.X + MarioFrameX[201],
                            vScreenY[Z] + e.Location.Y + MarioFrameY[201] - C,
                            99, 99, GFXMario[2], 500, 0);
                    break;
                case 2:
                    XRender::renderTexture(vScreenX[Z] + e.Location.X + LuigiFrameX[201],
                            vScreenY[Z] + e.Location.Y + LuigiFrameY[201] - C,
                            99, 99, GFXLuigi[2], 500, 0);
                    break;
                case 3:
                    XRender::renderTexture(vScreenX[Z] + e.Location.X + PeachFrameX[201],
                            vScreenY[Z] + e.Location.Y + PeachFrameY[201] - C,
                            99, 99, GFXPeach[2], 500, 0);
                    break;
                case 4:
                    XRender::renderTexture(vScreenX[Z] + e.Location.X + ToadFrameX[201],
                            vScreenY[Z] + e.Location.Y + ToadFrameY[201] - C,
                            99, 99, GFXToad[2], 500, 0);
                    break;
                case 5:
                    XRender::renderTexture(vScreenX[Z] + e.Location.X + LinkFrameX[201],
                            vScreenY[Z] + e.Location.Y + LinkFrameY[201] - C,
                            99, 99, GFXLink[2], 500, 0);
                    break;
                }
            }
        }

        else if(e.Mode == OptCursor_t::LVL_BGOS) // BGOs
        {
            auto &b = e.Background;
            if(vScreenCollision(Z, b.Location))
            {
                XRender::renderTexture(vScreenX[Z] + b.Location.X,
                                      vScreenY[Z] + b.Location.Y,
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
            if(NPCWidthGFX[n.Type] == 0)
            {
                XRender::renderTexture(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type],
                                      vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type],
                                      n.Location.Width,
                                      n.Location.Height,
                                      GFXNPC[n.Type], 0, n.Frame * n.Location.Height);
            }
            else
            {
                if(n.Type == 283 && n.Special > 0)
                {
                    if(NPCWidthGFX[n.Special] == 0)
                    {
                        tempLocation.Width = NPCWidth[n.Special];
                        tempLocation.Height = NPCHeight[n.Special];
                    }
                    else
                    {
                        tempLocation.Width = NPCWidthGFX[n.Special];
                        tempLocation.Height = NPCHeightGFX[n.Special];
                    }

                    tempLocation.X = n.Location.X + n.Location.Width / 2 - tempLocation.Width / 2;
                    tempLocation.Y = n.Location.Y + n.Location.Height / 2 - tempLocation.Height / 2;
                    B = EditorNPCFrame(int(n.Special), n.Direction);

                    XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[n.Type],
                                          vScreenY[Z] + tempLocation.Y,
                                          tempLocation.Width, tempLocation.Height,
                                          GFXNPC[n.Special], 0, B * tempLocation.Height);
                }

                XRender::renderTexture(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] - NPCWidthGFX[n.Type] / 2 + n.Location.Width / 2,
                                      vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] - NPCHeightGFX[n.Type] + n.Location.Height,
                                      NPCWidthGFX[n.Type], NPCHeightGFX[n.Type], GFXNPC[n.Type],
                                      0, n.Frame * NPCHeightGFX[n.Type]);
            }

            // render NPC inside container
            if(BlockFlash <= 30 && (n.Type == 91 || n.Type == 96)
                && (n.Special > 0))
            {
                if(vScreenCollision(Z, n.Location))
                {
                    C = n.Special;
                    if(NPCWidthGFX[C] == 0)
                    {
                        tempLocation.Height = NPCHeight[C];
                        tempLocation.Width = NPCWidth[C];
                    }
                    else
                    {
                        tempLocation.Height = NPCHeightGFX[C];
                        tempLocation.Width = NPCWidthGFX[C];
                    }

                    if(n.Type == 96)
                        tempLocation.Y = n.Location.Y + n.Location.Height - tempLocation.Height;
                    else
                        tempLocation.Y = n.Location.Y;
                    tempLocation.X = n.Location.X + n.Location.Width / 2 - tempLocation.Width / 2;

                    XRender::renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[C],
                        vScreenY[Z] + tempLocation.Y + NPCFrameOffsetY[C],
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

                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFX.Chat, 1., 0., 0., 0.7);
            }

            // and that they can talk
            if(n.Text != STRINGINDEX_NONE)
            {
                if(!(n.TriggerActivate != EVENT_NONE || n.TriggerTalk != EVENT_NONE || n.TriggerDeath != EVENT_NONE || n.TriggerLast != EVENT_NONE))
                    tempLocation.X = n.Location.X + n.Location.Width / 2 - GFX.Chat.w / 2;
                else
                    tempLocation.X = n.Location.X + n.Location.Width / 2 + 4;
                tempLocation.Y = n.Location.Y - GFX.Chat.h - 8;

                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFX.Chat, 1., 1., 1., 0.7);
            }
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WATER) // Water
        {
            if(EditorCursor.Water.Quicksand)
                XRender::renderRect(vScreenX[Z] + EditorCursor.Location.X, vScreenY[Z] + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                    1.f, 1.f, 0.f, 1.f, false);
            else
                XRender::renderRect(vScreenX[Z] + EditorCursor.Location.X, vScreenY[Z] + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                    0.f, 1.f, 1.f, 1.f, false);
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
        {
            XRender::renderRect(vScreenX[Z] + EditorCursor.Location.X, vScreenY[Z] + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                1.f, 0.f, 0.f, 1.f, false);
        }

        if(g_config.editor_edge_scroll && !editorScreen.active && !MagicHand)
        {
            if(curX >= 0 && curX < 36)
                curX = 36;
            if(curY >= 0 && curY < 36)
                curY = 36;
            if(curX >= ScreenW - 36)
                curX = ScreenW - 36;
            if(curY >= ScreenH - 36)
                curY = ScreenH - 36;
        }

        if(EditorCursor.Mode == 0 || EditorCursor.Mode == 6) // Eraser
        {
            if(EditorCursor.SubMode == -1)
            {
                int frame = BlockFlash / 20;
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
                XRender::renderTexture(curX, curY, GFX.ECursor[2], 0.7f, 1.0f, 0.7f, 1.0f);
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
                XRender::renderTexture(curX, curY, GFX.ECursor[2], 0.7f, 1.0f, 0.7f, 1.0f);
            else
                XRender::renderTexture(curX, curY, GFX.ECursor[2]);

            if(e.Layer != LAYER_NONE && e.Layer != LAYER_DEFAULT)
            {
                // there might be a tooltip in this case
                if(editorScreen.active || EditorCursor.Y < 40)
                    SuperPrint(GetL(e.Layer), 3, curX + 28 , curY + 34, 1., 1., 1., 0.3);
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
}

void DrawEditorWorld()
{
    int Z = 1;

    BlockFlash += 1;

    if(BlockFlash > 45)
        BlockFlash = 0;

    if(BlockFlash < 10)
    {
        // don't draw the currently held object
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_TILES)
    {
        XRender::renderTexture(vScreenX[Z] + EditorCursor.Tile.Location.X,
            vScreenY[Z] + EditorCursor.Tile.Location.Y,
            EditorCursor.Tile.Location.Width,
            EditorCursor.Tile.Location.Height,
            GFXTile[EditorCursor.Tile.Type],
            0,
            TileHeight[EditorCursor.Tile.Type] * TileFrame[EditorCursor.Tile.Type]);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES)
    {
        XRender::renderTexture(vScreenX[Z] + EditorCursor.Scene.Location.X,
            vScreenY[Z] + EditorCursor.Scene.Location.Y,
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
            XRender::renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X,
                                  vScreenY[Z] + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width,
                                  EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[0], 0, 0);
        }

        if(EditorCursor.WorldLevel.Path2)
        {
            XRender::renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X - 16,
                                  vScreenY[Z] + 8 + EditorCursor.WorldLevel.Location.Y,
                                  64, 32,
                                  GFXLevelBMP[29], 0, 0);
        }

        if(GFXLevelBig[EditorCursor.WorldLevel.Type])
        {
            XRender::renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X - (GFXLevelWidth[EditorCursor.WorldLevel.Type] - 32) / 2.0,
                                  vScreenY[Z] + EditorCursor.WorldLevel.Location.Y - GFXLevelHeight[EditorCursor.WorldLevel.Type] + 32,
                                  GFXLevelWidth[EditorCursor.WorldLevel.Type], GFXLevelHeight[EditorCursor.WorldLevel.Type],
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
        else
        {
            XRender::renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X,
                                  vScreenY[Z] + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width, EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_PATHS)
    {
        XRender::renderTexture(vScreenX[Z] + EditorCursor.WorldPath.Location.X,
            vScreenY[Z] + EditorCursor.WorldPath.Location.Y,
            EditorCursor.WorldPath.Location.Width,
            EditorCursor.WorldPath.Location.Height,
            GFXPath[EditorCursor.WorldPath.Type],
            0, 0);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
    {
        XRender::renderRect(vScreenX[Z] + EditorCursor.WorldMusic.Location.X, vScreenY[Z] + EditorCursor.WorldMusic.Location.Y, 32, 32,
            1.f, 0.f, 1.f, 1.f, false);
        SuperPrint(std::to_string(EditorCursor.WorldMusic.Type), 1, vScreenX[Z] + EditorCursor.WorldMusic.Location.X + 2, vScreenY[Z] + EditorCursor.WorldMusic.Location.Y + 2);
    }

    double X = EditorCursor.X;
    double Y = EditorCursor.Y;
    if(g_config.editor_edge_scroll && !editorScreen.active && !MagicHand)
    {
        if(X >= 0 && X < 36)
            X = 36;
        if(Y >= 0 && Y < 36)
            Y = 36;
        if(X >= ScreenW - 36)
            X = ScreenW - 36;
        if(Y >= ScreenH - 36)
            Y = ScreenH - 36;
    }

    if(EditorCursor.Mode == OptCursor_t::LVL_ERASER || EditorCursor.Mode == OptCursor_t::LVL_ERASER0)
    {
        if(EditorCursor.SubMode == -1)
        {
            int frame = BlockFlash / 20;
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
    }

    editorScreen.UpdateEditorScreen(EditorScreen::CallMode::Render);
}
