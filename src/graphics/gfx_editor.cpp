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

static inline int s_round2int(num_t d)
{
    return num_t::floor(d + 0.5_n);
}

void s_drawBlockExtra(int Z, int camX, int camY, const Block_t& b)
{
    if(b.Special > 0 && !b.Hidden)
    {
        if(vScreenCollision(Z, b.Location))
        {
            NPCID C = NPCID_NULL;
            if(b.Special > 1000)
                C = NPCID(b.Special - 1000);
            else if(b.Special == 110 && (CommonFrame & 32))
                C = NPCID_INVINCIBILITY_POWER;
            else
                C = NPCID_COIN_S3;

            int dW, dH;

            if(NPCWidthGFX(C) == 0)
            {
                dH = NPCHeight(C);
                dW = NPCWidth(C);
            }
            else
            {
                dH = NPCHeightGFX(C);
                dW = NPCWidthGFX(C);
            }

            int sX = s_round2int(b.Location.X + b.Location.Width / 2) - dW / 2;
            int sY = s_round2int(b.Location.Y + b.Location.Height / 2) - dH / 2;

            vbint_t tempDirection = -1;

            XRender::renderTextureBasic(camX + sX + NPCFrameOffsetX(C),
                camY + sY + NPCFrameOffsetY(C),
                dW, dH,
                GFXNPC[C], 0, EditorNPCFrame(C, tempDirection) * dH);
        }
    }

    // new: indicate that blocks have events
    if(b.TriggerHit != EVENT_NONE || b.TriggerDeath != EVENT_NONE || b.TriggerLast != EVENT_NONE)
    {
        if(vScreenCollision(Z, b.Location))
        {
            int sX = s_round2int(b.Location.X + b.Location.Width / 2) - GFX.Chat.w / 2;
            int sY = s_round2int(b.Location.Y) - GFX.Chat.h - 8;

            XRender::renderTextureBasic(camX + sX, camY + sY, GFX.Chat, XTColorF(1.0_n, 0.0_n, 0.0_n, 0.7_n));
        }
    }
}

void s_drawNpcExtra(int Z, int camX, int camY, const NPC_t& n)
{
    if(!n.Hidden && NPCIsContainer(n) && n.Type != NPCID_ITEM_BUBBLE && (n.Special > 0))
    {
        if(vScreenCollision(Z, n.Location))
        {
            NPCID C = NPCID(n.Special);

            int dW, dH;
            if(NPCWidthGFX(C) == 0)
            {
                dH = NPCHeight(C);
                dW = NPCWidth(C);
            }
            else
            {
                dH = NPCHeightGFX(C);
                dW = NPCWidthGFX(C);
            }

            int sY;
            if(n.Type == NPCID_ITEM_POD)
                sY = s_round2int(n.Location.Y + n.Location.Height) - dH;
            else
                sY = s_round2int(n.Location.Y);

            int sX = s_round2int(n.Location.X + n.Location.Width / 2) - dW / 2;

            vbint_t tempDirection = -1;

            XRender::renderTextureBasic(camX + sX + NPCFrameOffsetX(C),
                camY + sY + NPCFrameOffsetY(C),
                dW, dH,
                GFXNPC[C], 0, EditorNPCFrame(C, tempDirection) * dH);
        }
    }

    // new: indicate that NPCs have events
    if(n.TriggerActivate != EVENT_NONE || n.TriggerTalk != EVENT_NONE || n.TriggerDeath != EVENT_NONE || n.TriggerLast != EVENT_NONE)
    {
        if(vScreenCollision(Z, n.Location))
        {
            int sX = s_round2int(n.Location.X + n.Location.Width / 2);
            if(n.Text == STRINGINDEX_NONE)
                sX -= GFX.Chat.w / 2;
            else
                sX -= 4 + GFX.Chat.w;

            int sY = s_round2int(n.Location.Y) - GFX.Chat.h - 8;

            XRender::renderTextureBasic(camX + sX, camY + sY, GFX.Chat, XTColorF(1.0_n, 0.0_n, 0.0_n, 0.7_n));
        }
    }

    // and that they can talk
    if(n.Text != STRINGINDEX_NONE)
    {
        if(vScreenCollision(Z, n.Location))
        {
            int sX = s_round2int(n.Location.X + n.Location.Width / 2);
            if(!(n.TriggerActivate != EVENT_NONE || n.TriggerTalk != EVENT_NONE || n.TriggerDeath != EVENT_NONE || n.TriggerLast != EVENT_NONE))
                sX -= GFX.Chat.w / 2;
            else
                sX += 4;

            int sY = s_round2int(n.Location.Y) - GFX.Chat.h - 8;

            XRender::renderTextureBasic(camX + sX, camY + sY, GFX.Chat, XTColorF(1.0_n, 1.0_n, 1.0_n, 0.7_n));
        }
    }
}

void s_drawWaterBox(int camX, int camY, const Water_t& w)
{
    XRender::renderRect(camX + s_round2int(w.Location.X), camY + s_round2int(w.Location.Y), (int)(w.Location.Width), (int)(w.Location.Height),
        (w.Type == PHYSID_QUICKSAND) ? XTColor{255, 255, 0} :
        (w.Type == PHYSID_MAZE) ? XTColor{192, 192, 255} :
        XTColor{0, 255, 255},
        false);
}

void DrawEditorLevel(int Z)
{
    int A = 0;
    int B = 0;
    int C = 0;
    int S = curSection; // Level section to display

    // camera offsets to add to all object positions before drawing
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

#ifdef __3DS__
    XRender::setTargetLayer(2);
#endif
    if(LevelEditor)
    {
        if((CommonFrame % 46) <= 30)
        {
            // render NPCs in and events for blocks
            for(A = 1; A <= numBlock; A++)
                s_drawBlockExtra(Z, camX, camY, Block[A]);

            // render NPCs in containers
            for(A = 1; A <= numNPCs; A++)
                s_drawNpcExtra(Z, camX, camY, NPC[A]);
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
                p.Effect = PLREFF_NORMAL;
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

        // render water
        for(int B : treeWaterQuery(-camX, -camY,
            -camX + vScreen[Z].Width, -camY + vScreen[Z].Height,
            SORTMODE_ID))
        {
            if(!Water[B].Hidden && vScreenCollision(Z, Water[B].Location))
                s_drawWaterBox(camX, camY, Water[B]);
        }

        // render warps
        for(A = 1; A <= numWarps; A++)
        {
            if(Warp[A].Direction > 0 && !Warp[A].Hidden)
            {
                bool complete = Warp[A].PlacedEnt && Warp[A].PlacedExit;
                XTColor color = complete ? XTColorF(1.0_n, 0.0_n, 1.0_n) : XTColorF(0.7_n, 0.3_n, 0.0_n);

                if(Warp[A].PlacedEnt)
                {
                    XRender::renderRect(camX + s_round2int(Warp[A].Entrance.X), camY + s_round2int(Warp[A].Entrance.Y), 32, 32,
                        color, false);
                    SuperPrint(std::to_string(A), 1, camX + s_round2int(Warp[A].Entrance.X) + 2, camY + s_round2int(Warp[A].Entrance.Y) + 2);
                }

                if(Warp[A].PlacedExit)
                {
                    XRender::renderRect(camX + s_round2int(Warp[A].Exit.X), camY + s_round2int(Warp[A].Exit.Y), 32, 32,
                        color, false);
                    SuperPrint(std::to_string(A), 1, camX + s_round2int(Warp[A].Exit.X + Warp[A].Exit.Width) - 16 - 2,
                        camY + s_round2int(Warp[A].Exit.Y + Warp[A].Exit.Height) - 14 - 2);
                }
            }
        }

        // render event section resizes
        for(int A = 0; A < numEvents; A++)
        {
            const auto& e = Events[A];
            const IntegerLocation_t& sectPos = e.section[curSection].position;
            if(sectPos.X == EventSection_t::LESet_Nothing || sectPos.X == EventSection_t::LESet_ResetDefault)
                continue;

            XTColor ev_color = {uint8_t(64 + 128 * (A & 1)), uint8_t(64 + 64 * (A & 2)), uint8_t(64 + 32 * (A & 4))};
            DrawSimpleFrame(camX + sectPos.X, camY + sectPos.Y, sectPos.Width - sectPos.X, sectPos.Height - sectPos.Y, {0, 0, 0}, ev_color, {0, 0, 0, 0});
            SuperPrint(g_editorStrings.wordEvent, 3, camX + sectPos.X + 8, camY + sectPos.Y + 8);
            SuperPrint(e.Name, 3, camX + sectPos.X + 8, camY + sectPos.Y + 28, ev_color);
            SuperPrint(g_editorStrings.eventsCaseBounds, 3, camX + sectPos.X + 8, camY + sectPos.Y + 48);
        }
    }

#ifdef __3DS__
    XRender::setTargetLayer(2);
#endif

    // render section boundary
    if(LevelEditor)
    {
        if(camX + LevelREAL[S].X > 0)
        {
            XRender::renderRect(0, 0,
                               camX + LevelREAL[S].X, XRender::TargetH, {63, 63, 63, 192}, true);
        }

        if(XRender::TargetW > LevelREAL[S].Width + camX)
        {
            XRender::renderRect(LevelREAL[S].Width + camX, 0,
                               XRender::TargetW - (LevelREAL[S].Width + camX), XRender::TargetH, {63, 63, 63, 192}, true);
        }

        if(camY + LevelREAL[S].Y > 0)
        {
            XRender::renderRect(camX + LevelREAL[S].X, 0,
                               LevelREAL[S].Width - LevelREAL[S].X, camY + LevelREAL[S].Y, {63, 63, 63, 192}, true);
        }

        if(XRender::TargetH > LevelREAL[S].Height + camY)
        {
            XRender::renderRect(camX + LevelREAL[S].X, LevelREAL[S].Height + camY,
                               LevelREAL[S].Width - LevelREAL[S].X, XRender::TargetH - (LevelREAL[S].Height + camY), {63, 63, 63, 192}, true);
        }
    }

#ifdef __3DS__
    XRender::setTargetLayer(3);
#endif

#ifdef __3DS__
    // In-Editor message box preview (actually only useful on 3DS)
    if(editorScreen.active && !MessageText.empty())
        DrawMessage();
#endif

#ifdef __3DS__
    // disable cursor rendering on main screen when editor screen is active
    if(!editorScreen.active)
#endif
    // Display the cursor
    {
        auto &e = EditorCursor;
        int curX = s_round2int(e.X) - vScreen[Z].TargetX();
        int curY = s_round2int(e.Y) - vScreen[Z].TargetY();

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
                    XRender::renderSizableBlock(camX + s_round2int(b.Location.X), camY + s_round2int(b.Location.Y), (int)(b.Location.Width), (int)(b.Location.Height), GFXBlockBMP[b.Type]);
            }
            else
            {
                if(vScreenCollision(Z, b.Location))
                {
                    XRender::renderTextureBasic(camX + s_round2int(b.Location.X),
                                          camY + s_round2int(b.Location.Y) + b.ShakeOffset,
                                          (int)(b.Location.Width),
                                          (int)(b.Location.Height),
                                          GFXBlock[b.Type], 0, BlockFrame[b.Type] * 32);
                }
            }

            // render NPC inside block
            if((CommonFrame % 46) <= 30)
                s_drawBlockExtra(Z, camX, camY, b);
        }

        else if(e.Mode == OptCursor_t::LVL_PLAYERSTART) // Player start points
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
                p.Effect = PLREFF_NORMAL;
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
                XRender::renderTextureBasic(camX + s_round2int(b.Location.X),
                                      camY + s_round2int(b.Location.Y),
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

            int sX = camX + s_round2int(n.Location.X);
            int sY = camY + s_round2int(n.Location.Y);

            if(n->WidthGFX == 0)
            {
                XRender::renderTextureBasic(sX + n->FrameOffsetX,
                                      sY + n->FrameOffsetY,
                                      (int)n.Location.Width,
                                      (int)n.Location.Height,
                                      GFXNPC[n.Type], 0, n.Frame * (int)n.Location.Height);
            }
            else
            {
                if(n.Type == NPCID_ITEM_BUBBLE && n.Special > 0)
                {
                    int dW, dH;
                    if(NPCWidthGFX(n.Special) == 0)
                    {
                        dW = NPCWidth(n.Special);
                        dH = NPCHeight(n.Special);
                    }
                    else
                    {
                        dW = NPCWidthGFX(n.Special);
                        dH = NPCHeightGFX(n.Special);
                    }

                    int cont_sX = camX + s_round2int(n.Location.X + n.Location.Width / 2) - dW / 2;
                    int cont_sY = camY + s_round2int(n.Location.Y + n.Location.Height / 2) - dH / 2;
                    B = EditorNPCFrame(NPCID(n.Special), n.Direction);

                    XRender::renderTextureBasic(cont_sX + n->FrameOffsetX,
                                          cont_sY,
                                          dW, dH,
                                          GFXNPC[n.Special], 0, B * dH);
                }

                XRender::renderTextureBasic(camX + s_round2int(n.Location.X + n.Location.Width / 2) + n->FrameOffsetX - n->WidthGFX / 2,
                                      camY + s_round2int(n.Location.Y + n.Location.Height) + n->FrameOffsetY - n->HeightGFX,
                                      n->WidthGFX, n->HeightGFX, GFXNPC[n.Type],
                                      0, n.Frame * n->HeightGFX);
            }

            if(e.NPC.Wings)
                DrawNPCWings(e.NPC, sX, sY, XTColor());

            s_drawNpcExtra(Z, camX, camY, n);
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WATER) // Water
            s_drawWaterBox(camX, camY, EditorCursor.Water);
        else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
        {
            XRender::renderRect(camX + s_round2int(EditorCursor.Location.X), camY + s_round2int(EditorCursor.Location.Y), (int)EditorCursor.Location.Width, (int)EditorCursor.Location.Height,
                XTColorF(1.0_n, 0.0_n, 0.0_n, 1.0_n), false);
        }

        if(EditorCursor.Mode == 0 || EditorCursor.Mode == 6) // Eraser
        {
            if(EditorCursor.SubMode == -1)
            {
                int frame = (CommonFrame % 46) / 20;
                if(frame > 2)
                    frame = 2;
                XRender::renderTextureBasic(curX - 8, curY, 32, 32, GFXNPC[NPCID_AXE], 0, 32 * frame);
            }
            else
                XRender::renderTextureBasic(curX - 2, curY, GFX.ECursor[3]);
        }

        // left-right resize
        else if(EditorCursor.InteractFlags > 1 && !(EditorCursor.InteractFlags & 2) && !(EditorCursor.InteractFlags & 4))
        {
            XRender::renderTextureBasic(curX - 16, curY - 16, 32, 32, GFX.EIcons, 0, 32 * Icon::lr);
        }

        // up-down resize
        else if(EditorCursor.InteractFlags > 1 && !(EditorCursor.InteractFlags & 8) && !(EditorCursor.InteractFlags & 16))
        {
            XRender::renderTextureBasic(curX - 16, curY - 16, 32, 32, GFX.EIcons, 0, 32 * Icon::ud);
        }

        // resize
        else if(EditorCursor.InteractFlags > 1)
        {
            XRender::renderTextureBasic(curX - 16, curY - 16, 32, 32, GFX.EIcons, 0, 32 * Icon::target);
        }

        else if(EditorCursor.Mode == 13 || EditorCursor.Mode == 14) // Selector
        {
            if(MagicBlock::enabled)
                XRender::renderTextureBasic(curX, curY, GFX.ECursor[2], XTColorF(0.7_n, 1.0_n, 0.7_n, 1.0_n));
            else
                XRender::renderTextureBasic(curX, curY, GFX.ECursor[2]);

        }

        // Section Resize
        else if(EditorCursor.Mode == 2 && EditorCursor.SubMode < 4)
        {
            XRender::renderTextureBasic(curX, curY, GFX.ECursor[1]);
        }
        else
        {
            if(MagicBlock::enabled)
                XRender::renderTextureBasic(curX, curY, GFX.ECursor[2], XTColorF(0.7_n, 1.0_n, 0.7_n, 1.0_n));
            else
                XRender::renderTextureBasic(curX, curY, GFX.ECursor[2]);

            if(e.Layer != LAYER_NONE && e.Layer != LAYER_DEFAULT)
            {
                // there might be a tooltip in this case
                if(editorScreen.active || EditorCursor.Y < 40)
                    SuperPrint(GetL(e.Layer), 3, curX + 28 , curY + 34, XTColorF(1.0_n, 1.0_n, 1.0_n, 0.3_n));
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

    int camX = vScreen[Z].CameraAddX() + XRender::TargetOverscanX;
    int camY = vScreen[Z].CameraAddY();

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
        XRender::renderTextureBasic(camX + EditorCursor.Tile.Location.X,
            camY + EditorCursor.Tile.Location.Y,
            EditorCursor.Tile.Location.Width,
            EditorCursor.Tile.Location.Height,
            GFXTile[EditorCursor.Tile.Type],
            0,
            TileHeight[EditorCursor.Tile.Type] * TileFrame[EditorCursor.Tile.Type]);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_SCENES)
    {
        XRender::renderTextureBasic(camX + EditorCursor.Scene.Location.X,
            camY + EditorCursor.Scene.Location.Y,
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
            XRender::renderTextureBasic(camX + EditorCursor.WorldLevel.Location.X,
                                  camY + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width,
                                  EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[0], 0, 0);
        }

        if(EditorCursor.WorldLevel.Path2)
        {
            XRender::renderTextureBasic(camX + EditorCursor.WorldLevel.Location.X - 16,
                                  camY + 8 + EditorCursor.WorldLevel.Location.Y,
                                  64, 32,
                                  GFXLevelBMP[29], 0, 0);
        }

        if(GFXLevelBig[EditorCursor.WorldLevel.Type])
        {
            XRender::renderTextureBasic(camX + EditorCursor.WorldLevel.Location.X - (GFXLevel[EditorCursor.WorldLevel.Type].w - 32) / 2,
                                  camY + EditorCursor.WorldLevel.Location.Y - GFXLevel[EditorCursor.WorldLevel.Type].h + 32,
                                  GFXLevel[EditorCursor.WorldLevel.Type].w, GFXLevel[EditorCursor.WorldLevel.Type].h,
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
        else
        {
            XRender::renderTextureBasic(camX + EditorCursor.WorldLevel.Location.X,
                                  camY + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width, EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_PATHS)
    {
        XRender::renderTextureBasic(camX + EditorCursor.WorldPath.Location.X,
            camY + EditorCursor.WorldPath.Location.Y,
            EditorCursor.WorldPath.Location.Width,
            EditorCursor.WorldPath.Location.Height,
            GFXPath[EditorCursor.WorldPath.Type],
            0, 0);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
    {
        XRender::renderRect(camX + EditorCursor.WorldMusic.Location.X, camY + EditorCursor.WorldMusic.Location.Y, 32, 32,
            XTColorF(1.0_n, 0.0_n, 1.0_n), false);
        SuperPrint(std::to_string(EditorCursor.WorldMusic.Type), 1, camX + EditorCursor.WorldMusic.Location.X + 2, camY + EditorCursor.WorldMusic.Location.Y + 2);
    }
    else if(EditorCursor.Mode == OptCursor_t::WLD_AREA)
    {
        XRender::renderRect(camX + EditorCursor.WorldArea.Location.X, camY + EditorCursor.WorldArea.Location.Y,
            EditorCursor.WorldArea.Location.Width, EditorCursor.WorldArea.Location.Height,
            XTColorF(1.0_n, 0.8_n, 0.2_n), false);
    }

    int X = (int)EditorCursor.X - vScreen[Z].TargetX();
    int Y = (int)EditorCursor.Y - vScreen[Z].TargetY();

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

    if(EditorCursor.Mode == OptCursor_t::LVL_ERASER)
    {
        if(EditorCursor.SubMode == -1)
        {
            int frame = (CommonFrame % 46) / 20;
            if(frame > 2)
                frame = 2;
            XRender::renderTextureBasic(X - 8, Y, 32, 32, GFXNPC[NPCID_AXE], 0, 32 * frame);
        }
        else
            XRender::renderTextureBasic(X - 2, Y, GFX.ECursor[3]);
    }
    // left-right resize
    else if(EditorCursor.InteractFlags > 1 && !(EditorCursor.InteractFlags & 2) && !(EditorCursor.InteractFlags & 4))
    {
        XRender::renderTextureBasic(X - 16, Y - 16, 32, 32, GFX.EIcons, 0, 32 * Icon::lr);
    }

    // up-down resize
    else if(EditorCursor.InteractFlags > 1 && !(EditorCursor.InteractFlags & 8) && !(EditorCursor.InteractFlags & 16))
    {
        XRender::renderTextureBasic(X - 16, Y - 16, 32, 32, GFX.EIcons, 0, 32 * Icon::ud);
    }

    // resize
    else if(EditorCursor.InteractFlags > 1)
    {
        XRender::renderTextureBasic(X - 16, Y - 16, 32, 32, GFX.EIcons, 0, 32 * Icon::target);
    }
    else
    {
        XRender::renderTextureBasic(X, Y, GFX.ECursor[2]);

        // show coordinates of nearby level for help making warps
        for(WorldLevel_t* t : treeWorldLevelQuery(TinyLocation_t{(int)EditorCursor.Location.X, (int)EditorCursor.Location.Y, 1, 1}, SORTMODE_NONE))
        {
            WorldLevel_t &lvl = *t;
            if(CursorCollision(EditorCursor.Location, lvl.Location))
            {
                int at_X = lvl.Location.X + lvl.Location.Width / 2 + camX;
                int at_Y = lvl.Location.Y + camY - 40;
                XRender::renderRect(at_X - 80, at_Y - 4, 160, 44, {0, 0, 0, 127}, true);
                SuperPrintCenter(fmt::sprintf_ne("%s: %d", g_editorStrings.letterCoordX.c_str(), (int)lvl.Location.X), 3, at_X, at_Y);
                SuperPrintCenter(fmt::sprintf_ne("%s: %d", g_editorStrings.letterCoordY.c_str(), (int)lvl.Location.Y), 3, at_X, at_Y + 20);
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
