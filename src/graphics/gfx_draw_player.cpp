/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../globals.h"
#include "../graphics.h"
#include "../collision.h"


void DrawPlayer(const int A, const int Z)
{
    int B = 0;
    double C = 0;
    float s = ShadowMode ? 0.f : 1.f;
    auto &p = Player[A];

    if(!p.Dead && !p.Immune2 && p.TimeToLive == 0 && !(p.Effect == 3 || p.Effect == 5 || p.Effect == 8 || p.Effect == 10))
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

                    frmMain.renderTexture(vScreenX[Z] + double(p.YoshiTongueX) - C - 1,
                                          vScreenY[Z] + p.YoshiTongue.Y,
                                          p.YoshiTongueLength + 2,
                                          16,
                                          GFX.Tongue[2],
                                          0, 0,
                                          s, s, s);

                    C = 1;
                    if(p.Direction == 1)
                        C = 0;

                    frmMain.renderTexture(vScreenX[Z] + p.YoshiTongue.X,
                                          vScreenY[Z] + p.YoshiTongue.Y,
                                          16, 16,
                                          GFX.Tongue[1],
                                          0,
                                          16 * C,
                                          s, s, s);
                }

                // Yoshi's Body
                frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.YoshiBX,
                                      vScreenY[Z] + p.Location.Y + p.YoshiBY,
                                      32, 32,
                                      GFXYoshiB[B], 0, 32 * p.YoshiBFrame, s, s, s);

                // Yoshi's Head
                frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.YoshiTX,
                                      vScreenY[Z] + p.Location.Y + p.YoshiTY,
                                      32, 32,
                                      GFXYoshiT[B], 0, 32 * p.YoshiTFrame, s, s, s);
            }

            if(p.Fairy) // draw a fairy
            {
                p.Frame = 1;

                if(!p.Immune2)
                {
                    frmMain.renderTexture(vScreenX[Z] + p.Location.X - 5,
                                          vScreenY[Z] + p.Location.Y - 2,
                                          32, 32,
                                          GFXNPC[254],
                                          0,
                                          (SpecialFrame[9] + (p.Direction > 0 ? 1 : 3)) * 32,
                                          s, s, s);
//                    if(p.Direction == 1)
//                    {
//                        frmMain.renderTexture(vScreenX[Z] + p.Location.X - 5, vScreenY[Z] + p.Location.Y - 2, 32, 32, GFXNPC[254], 0, (SpecialFrame[9] + 1) * 32, s, s, s);
//                    }
//                    else
//                    {
//                        frmMain.renderTexture(vScreenX[Z] + p.Location.X - 5, vScreenY[Z] + p.Location.Y - 2, 32, 32, GFXNPC[254], 0, (SpecialFrame[9] + 3) * 32, s, s, s);
//                    }
                }
            }
            else if(p.Character == 1) // draw mario
            {
                if(p.Mount == 0)
                {
                    frmMain.renderTexture(
                                vScreenX[Z] + p.Location.X + MarioFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                vScreenY[Z] + p.Location.Y + MarioFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                100, 100,
                                GFXMario[p.State],
                                pfrX(100 + p.Frame * p.Direction),
                                pfrY(100 + p.Frame * p.Direction),
                                s, s, s);
                }
                else if(p.Mount == 1)
                {
                    if(!p.Duck)
                    {
                        frmMain.renderTexture(
                                    vScreenX[Z] + p.Location.X + MarioFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                    vScreenY[Z] + p.Location.Y + MarioFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                    100,
                                    p.Location.Height - 26 /*- p.MountOffsetY*/ - MarioFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                    GFXMario[p.State],
                                    pfrX(100 + p.Frame * p.Direction),
                                    pfrY(100 + p.Frame * p.Direction),
                                    s, s, s);
                    }

                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.Location.Width / 2.0 - 16,
                                          vScreenY[Z] + p.Location.Y + p.Location.Height - 30,
                                          32, 32,
                                          GFX.Boot[p.MountType],
                                          0,
                                          32 * p.MountFrame,
                                          s, s, s);
                }
                else if(p.Mount == 3)
                {
                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + MarioFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                          vScreenY[Z] + p.Location.Y + MarioFrameY[(p.State * 100) + (p.Frame * p.Direction)] + p.MountOffsetY,
                                          100, 100,
                                          GFXMario[p.State],
                                          pfrX(100 + p.Frame * p.Direction),
                                          pfrY(100 + p.Frame * p.Direction),
                                          s, s, s);
                }
            }
            else if(p.Character == 2) // draw luigi
            {
                if(p.Mount == 0)
                {
                    frmMain.renderTexture(
                                vScreenX[Z] + p.Location.X + LuigiFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                vScreenY[Z] + p.Location.Y + LuigiFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                100, 100,
                                GFXLuigi[p.State],
                                pfrX(100 + p.Frame * p.Direction),
                                pfrY(100 + p.Frame * p.Direction),
                                s, s, s);
                }
                else if(p.Mount == 1)
                {
                    if(!p.Duck)
                    {
                        frmMain.renderTexture(vScreenX[Z] + p.Location.X + LuigiFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                              vScreenY[Z] + p.Location.Y + LuigiFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                              100,
                                              p.Location.Height - 26 /*- p.MountOffsetY*/ - LuigiFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                              GFXLuigi[p.State],
                                              pfrX(100 + p.Frame * p.Direction),
                                              pfrY(100 + p.Frame * p.Direction),
                                              s, s, s);
                    }

                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.Location.Width / 2.0 - 16,
                                          vScreenY[Z] + p.Location.Y + p.Location.Height - 30,
                                          32, 32,
                                          GFX.Boot[p.MountType],
                                          0,
                                          32 * p.MountFrame,
                                          s, s, s);
                }
                else if(p.Mount == 3)
                {
                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + LuigiFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                          vScreenY[Z] + p.Location.Y + LuigiFrameY[(p.State * 100) + (p.Frame * p.Direction)] + p.MountOffsetY,
                                          100, 100,
                                          GFXLuigi[p.State],
                                          pfrX(100 + p.Frame * p.Direction),
                                          pfrY(100 + p.Frame * p.Direction),
                                          s, s, s);
                }
            }
            else if(p.Character == 3) // draw peach
            {
                if(p.Mount == 0)
                {
                    frmMain.renderTexture(
                                vScreenX[Z] + p.Location.X + PeachFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                vScreenY[Z] + p.Location.Y + PeachFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                100, 100,
                                GFXPeach[p.State],
                                pfrX(100 + p.Frame * p.Direction),
                                pfrY(100 + p.Frame * p.Direction),
                                s, s, s);
                }
                else if(p.Mount == 1)
                {
                    if(!p.Duck)
                    {
                        frmMain.renderTexture(vScreenX[Z] + p.Location.X + PeachFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                              vScreenY[Z] + p.Location.Y + PeachFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                              100,
                                              p.Location.Height - 26 - PeachFrameY[(p.State * 100) + (p.Frame * p.Direction)] - 2,
                                              GFXPeach[p.State],
                                              pfrX(100 + p.Frame * p.Direction),
                                              pfrY(100 + p.Frame * p.Direction),
                                              s, s, s);
                    }

                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.Location.Width / 2.0 - 16,
                                          vScreenY[Z] + p.Location.Y + p.Location.Height - 30,
                                          32, 32,
                                          GFX.Boot[p.MountType],
                                          0,
                                          32 * p.MountFrame,
                                          s, s, s);
                }
            }
            else if(p.Character == 4) // draw Toad
            {
                if(p.Mount == 0)
                {
                    frmMain.renderTexture(
                                vScreenX[Z] + p.Location.X + ToadFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                vScreenY[Z] + p.Location.Y + ToadFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                100, 100,
                                GFXToad[p.State],
                                pfrX(100 + p.Frame * p.Direction),
                                pfrY(100 + p.Frame * p.Direction),
                                s, s, s);
                }
                else if(p.Mount == 1)
                {
                    if(!p.Duck)
                    {
                        if(p.State == 1)
                        {
                            frmMain.renderTexture(vScreenX[Z] + p.Location.X + ToadFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                                  6 + vScreenY[Z] + p.Location.Y + ToadFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                                  100,
                                                  p.Location.Height - 26 /*- p.MountOffsetY*/ - ToadFrameY[(p.State * 100) + (p.Frame * p.Direction)], GFXToad[p.State],
                                                  pfrX(100 + p.Frame * p.Direction),
                                                  pfrY(100 + p.Frame * p.Direction),
                                                  s, s, s);
                        }
                        else
                        {
                            frmMain.renderTexture(vScreenX[Z] + p.Location.X + ToadFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                                  vScreenY[Z] + p.Location.Y + ToadFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                                  100,
                                                  p.Location.Height - 26 /*- p.MountOffsetY*/ - ToadFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                                  GFXToad[p.State],
                                                  pfrX(100 + p.Frame * p.Direction),
                                                  pfrY(100 + p.Frame * p.Direction),
                                                  s, s, s);
                        }
                    }

                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.Location.Width / 2.0 - 16,
                                          vScreenY[Z] + p.Location.Y + p.Location.Height - 30,
                                          32, 32,
                                          GFX.Boot[p.MountType],
                                          0,
                                          32 * p.MountFrame, s, s, s);
                }
            }
            else if(p.Character == 5) // draw link
            {
                if(p.Mount == 0)
                {
                    frmMain.renderTexture(vScreenX[Z] + p.Location.X + LinkFrameX[(p.State * 100) + (p.Frame * p.Direction)],
                                          vScreenY[Z] + p.Location.Y + LinkFrameY[(p.State * 100) + (p.Frame * p.Direction)],
                                          100, 100,
                                          GFXLink[p.State],
                                          pfrX(100 + p.Frame * p.Direction),
                                          pfrY(100 + p.Frame * p.Direction),
                                          s, s, s);
                }
            }
        // peach/toad held npcs
            if((p.Character == 3 || p.Character == 4) && p.HoldingNPC > 0 && p.Effect != 7)
            {
                if(NPC[p.HoldingNPC].Type != 263)
                {
                    if(
                        (
                            (
                                 (
                                        NPC[p.HoldingNPC].HoldingPlayer > 0 &&
                                        Player[NPC[p.HoldingNPC].HoldingPlayer].Effect != 3
                                  ) ||
                                 (NPC[p.HoldingNPC].Type == 50 && NPC[p.HoldingNPC].standingOnPlayer == 0) ||
                                 (NPC[p.HoldingNPC].Type == 17 && NPC[p.HoldingNPC].CantHurt > 0)
                             ) ||
                          NPC[p.HoldingNPC].Effect == 5
                        ) &&
                        NPC[p.HoldingNPC].Type != 91 &&
                     !Player[NPC[p.HoldingNPC].HoldingPlayer].Dead
                    )
                    {
                        if(!NPCIsYoshi[NPC[p.HoldingNPC].Type] && NPC[p.HoldingNPC].Type > 0)
                        {
                            if(NPCWidthGFX[NPC[p.HoldingNPC].Type] == 0)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[p.HoldingNPC].Location.X + NPCFrameOffsetX[NPC[p.HoldingNPC].Type],
                                                      vScreenY[Z] + NPC[p.HoldingNPC].Location.Y + NPCFrameOffsetY[NPC[p.HoldingNPC].Type],
                                                      NPC[p.HoldingNPC].Location.Width,
                                                      NPC[p.HoldingNPC].Location.Height,
                                                      GFXNPC[NPC[p.HoldingNPC].Type],
                                                      0,
                                                      NPC[p.HoldingNPC].Frame * NPC[p.HoldingNPC].Location.Height);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[p.HoldingNPC].Location.X + (NPCFrameOffsetX[NPC[p.HoldingNPC].Type] * -NPC[p.HoldingNPC].Direction) - NPCWidthGFX[NPC[p.HoldingNPC].Type] / 2.0 + NPC[p.HoldingNPC].Location.Width / 2.0,
                                                      vScreenY[Z] + NPC[p.HoldingNPC].Location.Y + NPCFrameOffsetY[NPC[p.HoldingNPC].Type] - NPCHeightGFX[NPC[p.HoldingNPC].Type] + NPC[p.HoldingNPC].Location.Height,
                                                      NPCWidthGFX[NPC[p.HoldingNPC].Type],
                                                      NPCHeightGFX[NPC[p.HoldingNPC].Type],
                                                      GFXNPC[NPC[p.HoldingNPC].Type],
                                                      0,
                                                      NPC[p.HoldingNPC].Frame * NPCHeightGFX[NPC[p.HoldingNPC].Type]);
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
                        frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.YoshiBX - 12,
                                              vScreenY[Z] + p.Location.Y + p.YoshiBY - 16,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s, s, s);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + p.Location.X + p.YoshiBX + 12,
                                              vScreenY[Z] + p.Location.Y + p.YoshiBY - 16,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s, s, s);
                    }
                }
                if(p.Mount == 1 && p.MountType == 3)
                {
                    if(p.Direction == 1)
                    {
                        frmMain.renderTexture(vScreenX[Z] + p.Location.X - 24,
                                              vScreenY[Z] + p.Location.Y + p.Location.Height - 40,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s, s, s);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + p.Location.X + 16,
                                              vScreenY[Z] + p.Location.Y + p.Location.Height - 40,
                                              32, 32, GFX.YoshiWings, 0, 0 + 32 * p.YoshiWingsFrame, s, s, s);
                    }
                }
            }
        }
    }
}
