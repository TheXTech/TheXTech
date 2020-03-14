/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "../globals.h"
#include "../graphics.h"
#include "../collision.h"


void DrawPlayer(int A, int Z)
{
    int B = 0;
    double C = 0;
    float s = ShadowMode ? 0.f : 1.f;
    if(Player[A].Dead == false && Player[A].Immune2 == false && Player[A].TimeToLive == 0 && !(Player[A].Effect == 3 || Player[A].Effect == 5 || Player[A].Effect == 8 || Player[A].Effect == 10))
    {
        if(vScreenCollision(Z, Player[A].Location))
        {
            if(Player[A].Mount == 3 && Player[A].Fairy == false)
            {
                B = Player[A].MountType;
                // Yoshi's Tongue
                if(Player[A].MountSpecial > 0)
                {

                    C = 0;
                    if(Player[A].Direction == -1)
                        C = Player[A].YoshiTongueLength;

                    frmMain.renderTexture(vScreenX[Z] + double(Player[A].YoshiTongueX) - C - 1, vScreenY[Z] + Player[A].YoshiTongue.Y, Player[A].YoshiTongueLength + 2, 16, GFX.Tongue[2], 0, 0, s, s, s);

                    C = 1;
                    if(Player[A].Direction == 1)
                        C = 0;

                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].YoshiTongue.X))), vScreenY[Z] + Player[A].YoshiTongue.Y, 16, 16, GFX.Tongue[1], 0, 16 * C, s, s, s);

                }
                // Yoshi's Body
                frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].YoshiBX, vScreenY[Z] + Player[A].Location.Y + Player[A].YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * Player[A].YoshiBFrame, s, s, s);

                // Yoshi's Head
                frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].YoshiTX, vScreenY[Z] + Player[A].Location.Y + Player[A].YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * Player[A].YoshiTFrame, s, s, s);
            }
            if(Player[A].Fairy == true) // draw a fairy
            {
                Player[A].Frame = 1;
                if(Player[A].Immune2 == false)
                {
                    if(Player[A].Direction == 1)
                    {
                        frmMain.renderTexture(vScreenX[Z] + Player[A].Location.X - 5, vScreenY[Z] + Player[A].Location.Y - 2, 32, 32, GFXNPC[254], 0, (SpecialFrame[9] + 1) * 32, s, s, s);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + Player[A].Location.X - 5, vScreenY[Z] + Player[A].Location.Y - 2, 32, 32, GFXNPC[254], 0, (SpecialFrame[9] + 3) * 32, s, s, s);
                    }
                }
            }
            else if(Player[A].Character == 1) // draw mario
            {
                if(Player[A].Mount == 0)
                {
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, 99, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
                else if(Player[A].Mount == 1)
                {
                    if(Player[A].Duck == false)
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 26 - Player[A].MountOffsetY - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                    }
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 16, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, s, s, s);
                }
                else if(Player[A].Mount == 3)
                {
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY, 99, 99, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
            }
            else if(Player[A].Character == 2) // draw luigi
            {
                if(Player[A].Mount == 0)
                {
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, 99, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
                else if(Player[A].Mount == 1)
                {
                    if(Player[A].Duck == false)
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 26 - Player[A].MountOffsetY - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                    }
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 16, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame);
                }
                else if(Player[A].Mount == 3)
                {
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY, 99, 99, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
            }
            else if(Player[A].Character == 3) // draw peach
            {
                if(Player[A].Mount == 0)
                {
                    frmMain.renderTexture(vScreenX[Z] + Player[A].Location.X + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, 99, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
                else if(Player[A].Mount == 1)
                {
                    if(Player[A].Duck == false)
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 26 - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 2, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                    }
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 16, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, s, s, s);
                }
            }
            else if(Player[A].Character == 4) // draw Toad
            {
                if(Player[A].Mount == 0)
                {
                    frmMain.renderTexture(vScreenX[Z] + Player[A].Location.X + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, 99, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
                else if(Player[A].Mount == 1)
                {
                    if(Player[A].Duck == false)
                    {
                        if(Player[A].State == 1)
                        {
                            frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 6 + vScreenY[Z] + Player[A].Location.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 26 - Player[A].MountOffsetY - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                        }
                        else
                        {
                            frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 26 - Player[A].MountOffsetY - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                        }
                    }
                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 16, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, s, s, s);
                }
            }
            else if(Player[A].Character == 5) // draw link
            {
                if(Player[A].Mount == 0)
                {
                    frmMain.renderTexture(vScreenX[Z] + Player[A].Location.X + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], vScreenY[Z] + Player[A].Location.Y + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, 99, GFXLink[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), s, s, s);
                }
            }
        // peach/toad held npcs
            if((Player[A].Character == 3 || Player[A].Character == 4) && Player[A].HoldingNPC > 0 && Player[A].Effect != 7)
            {
                if(NPC[Player[A].HoldingNPC].Type != 263)
                {
                    if(
                        (
                            (
                                 (
                                        NPC[Player[A].HoldingNPC].HoldingPlayer > 0 &&
                                        Player[NPC[Player[A].HoldingNPC].HoldingPlayer].Effect != 3
                                  ) ||
                                 (NPC[Player[A].HoldingNPC].Type == 50 && NPC[Player[A].HoldingNPC].standingOnPlayer == 0) ||
                                 (NPC[Player[A].HoldingNPC].Type == 17 && NPC[Player[A].HoldingNPC].CantHurt > 0)
                             ) ||
                          NPC[Player[A].HoldingNPC].Effect == 5
                        ) &&
                     !(NPC[Player[A].HoldingNPC].Type == 91) &&
                     !Player[NPC[Player[A].HoldingNPC].HoldingPlayer].Dead
                    )
                    {
                        if(NPCIsYoshi[NPC[Player[A].HoldingNPC].Type] == false && NPC[Player[A].HoldingNPC].Type > 0)
                        {
                            if(NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] == 0)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[Player[A].HoldingNPC].Location.X + NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type], vScreenY[Z] + NPC[Player[A].HoldingNPC].Location.Y + NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type], NPC[Player[A].HoldingNPC].Location.Width, NPC[Player[A].HoldingNPC].Location.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], 0, NPC[Player[A].HoldingNPC].Frame * NPC[Player[A].HoldingNPC].Location.Height);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[Player[A].HoldingNPC].Location.X + (NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type] * -NPC[Player[A].HoldingNPC].Direction) - NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] / 2.0 + NPC[Player[A].HoldingNPC].Location.Width / 2.0, vScreenY[Z] + NPC[Player[A].HoldingNPC].Location.Y + NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type] - NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] + NPC[Player[A].HoldingNPC].Location.Height, NPCWidthGFX[NPC[Player[A].HoldingNPC].Type], NPCHeightGFX[NPC[Player[A].HoldingNPC].Type], GFXNPC[NPC[Player[A].HoldingNPC].Type], 0, NPC[Player[A].HoldingNPC].Frame * NPCHeightGFX[NPC[Player[A].HoldingNPC].Type]);
                            }
                        }
                    }
                }
                else
                {
                    DrawFrozenNPC(Z, Player[A].HoldingNPC);
                }
            }
            if(Player[A].Fairy == false)
            {
                if(Player[A].Mount == 3 && Player[A].YoshiBlue == true)
                {
                    if(Player[A].Direction == 1)
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].YoshiBX - 12, vScreenY[Z] + Player[A].Location.Y + Player[A].YoshiBY - 16, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, s, s, s);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].YoshiBX + 12, vScreenY[Z] + Player[A].Location.Y + Player[A].YoshiBY - 16, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, s, s, s);
                    }
                }
                if(Player[A].Mount == 1 && Player[A].MountType == 3)
                {
                    if(Player[A].Direction == 1)
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) - 24, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 40, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, s, s, s);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + 16, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 40, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, s, s, s);
                    }
                }
            }
        }
    }
}
